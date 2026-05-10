#include "HX711.h"

// Pin load cell
#define DT_PIN  4
#define SCK_PIN 5

HX711 scale;

float calibration_factor = 420.0; // Akan diperbarui saat kalibrasi
float known_weight = 0.0;         // Berat beban standar (input user)

// Buffer perintah dari Serial
#define CMD_BUFFER_SIZE 20
char cmdBuffer[CMD_BUFFER_SIZE];
byte cmdIndex = 0;

void setup() {
  Serial.begin(115200);
  scale.begin(DT_PIN, SCK_PIN);

  Serial.println(F("=== Sistem Kalibrasi Load Cell ==="));
  Serial.println(F("Ketik 'MULAI' untuk Tare (nol-kan)"));
}

void loop() {
  // Baca perintah per karakter, akhiri dengan newline
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (cmdIndex > 0) {
        cmdBuffer[cmdIndex] = '\0'; // tutup string
        processCommand(cmdBuffer);
        cmdIndex = 0;               // reset indeks buffer
      }
    } else {
      if (cmdIndex < CMD_BUFFER_SIZE - 1) {
        cmdBuffer[cmdIndex++] = c;
      }
    }
  }
}

void processCommand(char* cmd) {
  // Ubah ke huruf besar untuk perbandingan
  for (byte i = 0; cmd[i] != '\0'; i++) {
    cmd[i] = toupper(cmd[i]);
  }

  // Cek perintah "MULAI"
  if (strcmp(cmd, "MULAI") == 0) {
    Serial.println(F("\nKosongkan timbangan... Sedang Tare..."));
    scale.set_scale();      // pastikan skala = 1 (default)
    scale.tare();           // reset offset ke nol
    Serial.println(F("Tare selesai."));
    Serial.println(F("Letakkan beban standar, lalu masukkan beratnya (contoh: 100):"));
  }
  // Cek perintah "CEK"
  else if (strcmp(cmd, "CEK") == 0) {
    Serial.println(F("Membaca berat (rata-rata 10 kali):"));
    float units = scale.get_units(10);
    Serial.print(F("Berat: "));
    Serial.print(units);
    Serial.println(F(" unit."));
    Serial.println(F("-------------------------"));
  }
  // Jika bukan perintah di atas, coba parse sebagai angka berat
  else {
    float weight = atof(cmd);  // atof mengembalikan 0.0 jika gagal
    if (weight > 0.0) {
      known_weight = weight;
      Serial.print(F("Kalibrasi untuk beban: "));
      Serial.print(known_weight);
      Serial.println(F(" unit."));

      // Baca nilai mentah (10x rata-rata) setelah tare
      long reading = scale.get_units(10);
      calibration_factor = reading / known_weight;

      scale.set_scale(calibration_factor);
      Serial.print(F("Faktor Kalibrasi Baru: "));
      Serial.println(calibration_factor);
      Serial.println(F("\nTimbangan siap. Ketik 'CEK' untuk melihat hasil."));
    }
  }
}