// Status Volume dan Alarm

#include "HX711.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

HX711 scale; // Declare HX711
LiquidCrystal_I2C lcd(0x27, 16, 2); // Declare LCD

// Declare Pin HX711
const int DT_PIN = 3;    // Data pin
const int SCK_PIN = 4;   // Clock pin

// Declare pin LED and Buzzer
const int ledHijau = 4;
const int ledKuning = 5;
const int ledMerah = 6;
const int buzzer = 8;

// Declare pin button
const int resetButton = 7;
const int calibrateButton = 9;

// Volume n status variable
float emptyBagWeight = 0;    // Empty bottle (tare result)
float fullWeight = 500.0;    // Full weight of liquid (g) (e.g. 500ml = 500g)
float warningWeight = 50.0;  // Warning Barrier (leftover ~50ml)

// Alarm Variable
unsigned long lastBeep = 0;
String status = "";  // ← TAMBAHAN: pindahin dari dalam loop

void setup() {
  Serial.begin(9600);
  
  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Infusion Monitor");
  delay(2000);
  lcd.clear();
  
  // Inisialisasi HX711
  scale.begin(DT_PIN, SCK_PIN);
  scale.tare();
  
  // Setup pin mode
  pinMode(ledHijau, OUTPUT);
  pinMode(ledKuning, OUTPUT);
  pinMode(ledMerah, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(resetButton, INPUT_PULLUP);
  pinMode(calibrateButton, INPUT_PULLUP);
  
  // Matikan semua output
  digitalWrite(ledHijau, LOW);
  digitalWrite(ledKuning, LOW);
  digitalWrite(ledMerah, LOW);
  digitalWrite(buzzer, LOW);
  
  lcd.print("Ready!");
  delay(1000);
  lcd.clear();
}

void loop() {
  float currentWeight = scale.get_units(); // Total weight (bottle inc liquid)
  float fluidVolume = currentWeight - emptyBagWeight; // liquid volume 1ly
  float remainingHours = fluidVolume / 20.0;  // Anggeplah 20 ml/h
  int percentage = (fluidVolume / fullWeight) * 100;
  
  if (percentage > 75) {
    status = "FULL";
    digitalWrite(ledHijau, HIGH);
    digitalWrite(ledKuning, LOW);
    digitalWrite(ledMerah, LOW);
    digitalWrite(buzzer, LOW);
  } 
  else if (percentage > 15) {
    status = "HALF";
    digitalWrite(ledHijau, LOW);
    digitalWrite(ledKuning, HIGH);
    digitalWrite(ledMerah, LOW);
    digitalWrite(buzzer, LOW);
  } 
  else {
    status = "WARNING"; // <15% or <150ml
    digitalWrite(ledHijau, LOW);
    digitalWrite(ledKuning, LOW);
    digitalWrite(ledMerah, HIGH);
    
    // Buzzer beep intermiten
    if (millis() - lastBeep > 500) {
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
      lastBeep = millis();
    }
  }
  
  // LCD showing
  lcd.setCursor(0,0);
  lcd.print("Vol:");
  lcd.print(fluidVolume, 0);
  lcd.print(" ml ");
  lcd.print(status);
  
  // Estimasi sisa waktu (h)
  lcd.setCursor(0,1);
  lcd.print("Sisa: ");
  lcd.print(remainingHours, 1);
  lcd.print(" jam  ");
  
  delay(1000);
}