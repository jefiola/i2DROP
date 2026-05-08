#include "HX711.h"

#define LOADCELL_DT   A1
#define LOADCELL_SCK  A0
#define TCRT_PIN      7
#define BUZZER_PIN    8

float batas_habis = 15;        // Alarm bunyi kalo ≤ 15 mL
float berat_awal = 0;
float volume_sekarang = 0;

// Variabel tetesan
int lastState = HIGH;
unsigned long lastDropTime = 0;
int tetesPerMenit = 0;
int totalTetes = 0;

// Variabel alarm
bool alarm_aktif = false;       // declare

// Kalibrasi
float calibration_factor = -500;  // Ganti pake nilai kalibrasi

HX711 timbangan;

void setup() {
  Serial.begin(9600);
  Serial.println("i2DROP");

  pinMode(TCRT_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Inisialisasi HX711
  timbangan.begin(LOADCELL_DT, LOADCELL_SCK);
  timbangan.set_scale(calibration_factor);
  timbangan.tare();  // Reset zero (tanpa beban)

  Serial.println("Letakkan kantong infus PENUH, lalu tekan tombol reset Arduino...");
  delay(3000);
  
  // Baca berat awal
  berat_awal = timbangan.get_units(10);
  volume_sekarang = berat_awal;  // asumsi 1 gram = 1 mL
  
  Serial.print("Berat awal infus: ");
  Serial.print(berat_awal);
  Serial.println(" gram");
  Serial.print("Volume awal: ");
  Serial.print(volume_sekarang);
  Serial.println(" mL");

  if (berat_awal < batas_habis + 5) {
    Serial.println("Infus hampir habis sejak awal!");
  }
}

void loop() {
  // Baca vol
  volume_sekarang = timbangan.get_units(5);
  if (volume_sekarang < 0) volume_sekarang = 0;

  Serial.print("Sisa volume: ");
  Serial.print(volume_sekarang);
  Serial.println(" mL");

  if (volume_sekarang <= batas_habis && volume_sekarang > 0) {
    if (!alarm_aktif) {
      Serial.println("WALAO EH, INFUS HAMPIR HABIS !!!");
      alarm_aktif = true;
    }
    // Buzzer
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    delay(500);
  } 
  else if (volume_sekarang <= 0) {
    Serial.println("INFUSNYA HABIS CUI !!!");
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    delay(1000);
  }
  else {
    digitalWrite(BUZZER_PIN, LOW);
    alarm_aktif = false;
  }

  int sensorState = digitalRead(TCRT_PIN);
  
  // TCRT5000 output LOW saat mendeteksi tetes
  if (sensorState == LOW && lastState == HIGH) {
    totalTetes++;
    unsigned long now = millis();
    if (lastDropTime != 0) {
      int interval = now - lastDropTime;
      if (interval > 0) {
        tetesPerMenit = 60000 / interval;
      }
    }
    lastDropTime = now;
    
    Serial.print("Tetes terdeteksi #");
    Serial.print(totalTetes);
    Serial.print(" | Laju: ");
    Serial.print(tetesPerMenit);
    Serial.println(" tetes/menit");
  }
  lastState = sensorState;

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 10000) {
    Serial.print("=== RINGKASAN === Sisa: ");
    Serial.print(volume_sekarang);
    Serial.print(" mL | Total tetes: ");
    Serial.print(totalTetes);
    Serial.print(" | Laju: ");
    Serial.print(tetesPerMenit);
    Serial.println(" tpm");
    lastPrint = millis();
  }

  delay(200);
}