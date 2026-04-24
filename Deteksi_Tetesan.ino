#include <Wire.h>              // Library untuk komunikasi I2C
#include <LiquidCrystal_I2C.h> // Library untuk LCD I2C
#include "HX711.h"

// SIX SEVENNNNNNNNNN
// Ini kalibrasi Load Cell
HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int buttonCalibrate = 7;
const int buttonTare = 9;

// Deteksi tetesan
// Deklarasi LCD: (alamat I2C, jumlah kolom, jumlah baris)

const int dropSensor = 2; // Pin sensor tetesan
volatile int dropCount = 0;
unsigned long lastTime = 0;
float flowRate = 0; // tetes per menit
int dropFactor = 20; // faktor tetes (20 tetes/ml untuk infus standar)

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  
  pinMode(buttonCalibrate, INPUT_PULLUP);
  pinMode(buttonTare, INPUT_PULLUP);
  
  scale.begin(3, 4); // DT pin 3, SCK pin 4
  
  lcd.print("Kalibrasi Load");
  lcd.setCursor(0,1);
  lcd.print("Cell...");
  delay(2000);
  
  scale.tare(); // Zeroing tanpa beban
  lcd.clear();
  lcd.print("Tare Done!");
  delay(1000);
  lcd.clear();

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