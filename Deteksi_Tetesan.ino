// Deteksi tetesan

#include <Wire.h>              // Library untuk komunikasi I2C
#include <LiquidCrystal_I2C.h> // Library untuk LCD I2C

// Deklarasi LCD: (alamat I2C, jumlah kolom, jumlah baris)
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int dropSensor = 2; // Pin sensor tetesan
volatile int dropCount = 0;
unsigned long lastTime = 0;
float flowRate = 0; // tetes per menit
int dropFactor = 20; // faktor tetes (20 tetes/ml untuk infus standar)

void setup() {
  // ... kode sebelumnya ...
  pinMode(dropSensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(dropSensor), hitungTetesan, FALLING);
}

void hitungTetesan() {
  dropCount++;
}

void loop() {
  // Hitung flow rate setiap detik
  if (millis() - lastTime >= 1000) {
    flowRate = dropCount * 60.0; // tetes per menit
    float mlPerHour = (flowRate / dropFactor) * 60.0; // konversi ke ml/jam
    
    lcd.setCursor(0,1);
    lcd.print("Flow: ");
    lcd.print(flowRate, 0);
    lcd.print(" tpm  ");
    
    // Reset counter
    dropCount = 0;
    lastTime = millis();
  }
}