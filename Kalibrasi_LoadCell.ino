#include "HX711.h"
#include <LiquidCrystal_I2C.h>

// SIX SEVENNNNNNNNNN
// Ini kalibrasi Load Cell
HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int buttonCalibrate = 7;
const int buttonTare = 9;

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
}

void loop() {
  lcd.setCursor(0,0);
  lcd.print("Berat: ");
  lcd.print(scale.get_units(), 1);
  lcd.print(" g   ");
  
  // Tombol kalibrasi: tekan saat ada beban diketahui
  if (digitalRead(buttonCalibrate) == LOW) {
    lcd.clear();
    lcd.print("Masukkan berat");
    lcd.setCursor(0,1);
    lcd.print("(gram):");
    
    // Input manual ke Serial Monitor
    while(!Serial.available());
    float knownWeight = Serial.parseFloat();
    
    float currentReading = scale.get_units(10);
    float calibrationFactor = currentReading / knownWeight;
    
    scale.set_scale(calibrationFactor);
    lcd.clear();
    lcd.print("Kalibrasi OK!");
    delay(2000);
  }
  
  delay(500);
}