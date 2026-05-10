#include "HX711.h"

#define LOADCELL_DT   4
#define LOADCELL_SCK  5
#define BUZZER_PIN    8

// ===== KONFIGURASI =====
float calibration_factor = 452.99;   // dari hasil kalibrasi
float batas_habis = 165;              // mL, alarm mulai berbunyi
#define JUMLAH_SAMPEL 5              // <-- Semakin kecil semakin cepat

HX711 timbangan;

float volume_awal = 0;
float volume_sekarang = 0;
bool alarm_aktif = false;

// Variabel non-blocking buzzer
unsigned long buzzerTimer = 0;
bool buzzerState = LOW;
int buzzerPattern = 0;   // 0 = mati, 1 = bip pendek (hampir habis), 2 = panjang (habis)
unsigned long buzzerInterval = 0;

// Buffer perintah Serial (untuk tunggu "OK")
#define CMD_SIZE 10
char cmd[CMD_SIZE];
byte cmdIdx = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(F("i2DROP - Fast Response"));

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Inisialisasi HX711 yang benar
  timbangan.begin(LOADCELL_DT, LOADCELL_SCK);
  timbangan.set_scale();                // skala 1
  timbangan.tare();                     // nolkan mentah
  timbangan.set_scale(calibration_factor);
  
  Serial.println(F("Pastikan timbangan KOSONG, lalu ketik OK"));
  tungguPerintah("OK");
  
  Serial.println(F("Tare ulang..."));
  timbangan.tare();
  
  Serial.println(F("Gantung kantong infus PENUH, lalu ketik OK"));
  tungguPerintah("OK");
  
  // Baca volume awal (rata-rata 10 sampel untuk akurasi awal)
  volume_awal = timbangan.get_units(10);
  if (volume_awal < 0) volume_awal = 0;
  volume_sekarang = volume_awal;
  
  Serial.print(F("Volume awal: "));
  Serial.print(volume_awal);
  Serial.println(F(" mL"));
  
  if (volume_awal < batas_habis + 5) {
    Serial.println(F("WARNING: Infus sudah hampir habis!"));
  }
}

void loop() {
  // 1. Baca berat SEKARANG dengan sampel sedikit → lebih cepat
  volume_sekarang = timbangan.get_units(JUMLAH_SAMPEL);
  if (volume_sekarang < 0) volume_sekarang = 0;

  // 2. Tampilkan sisa setiap 1 detik (non-blocking)
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 1000) {
    Serial.print(F("Sisa: "));
    Serial.print(volume_sekarang, 1);    // 1 desimal
    Serial.println(F(" mL"));
    lastPrint = millis();
  }

  // 3. Tentukan mode alarm berdasarkan volume
  if (volume_sekarang <= 0) {
    // Infus habis total
    if (buzzerPattern != 2) {
      Serial.println(F("INFUS HABIS!"));
      buzzerPattern = 2;  // bunyi panjang
      alarm_aktif = true;
    }
  }
  else if (volume_sekarang <= batas_habis) {
    // Hampir habis
    if (!alarm_aktif) {
      Serial.println(F("INFUS HAMPIR HABIS!"));
      alarm_aktif = true;
    }
    buzzerPattern = 1;  // bip pendek
  }
  else {
    // Di atas batas → matikan semua
    buzzerPattern = 0;
    alarm_aktif = false;
  }

  // 4. Update buzzer non-blocking
  updateBuzzer();

  // Sedikit jeda agar tidak memboroskan CPU (tidak pengaruh ke respons)
  delay(1);
}

// ===== FUNGSI PEMBANTU =====
void updateBuzzer() {
  unsigned long now = millis();
  
  if (buzzerPattern == 0) {
    digitalWrite(BUZZER_PIN, LOW);
    return;
  }
  
  // Tentukan interval dan durasi bunyi sesuai pola
  if (buzzerPattern == 1) {   // Hampir habis: bip pendek 200ms, jeda 300ms
    if (!buzzerState) {
      buzzerInterval = 200;
    } else {
      buzzerInterval = 300;
    }
  } 
  else if (buzzerPattern == 2) { // Habis: bunyi panjang 1000ms, jeda 1000ms
    if (!buzzerState) {
      buzzerInterval = 1000;
    } else {
      buzzerInterval = 1000;
    }
  }
  
  // Periksa apakah waktunya mengganti state
  if (now - buzzerTimer >= buzzerInterval) {
    buzzerState = !buzzerState;
    digitalWrite(BUZZER_PIN, buzzerState);
    buzzerTimer = now;
  }
}

void tungguPerintah(const char* target) {
  while (true) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        cmd[cmdIdx] = '\0';
        if (strcasecmp(cmd, target) == 0) {
          cmdIdx = 0;
          break;
        }
        cmdIdx = 0;
      } else {
        if (cmdIdx < CMD_SIZE - 1) 
          cmd[cmdIdx++] = c;
      }
    }
  }
}