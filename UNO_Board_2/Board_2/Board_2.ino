#include <Wire.h>                   
#include <LiquidCrystal_I2C.h>

// ที่อยู่ของ LCD I2C 
#define LCD_ADDRESS 0x27

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);  // 16x2 LCD

volatile bool interruptFlag = false;        // Flag สำหรับตรวจสอบ Interrupt
volatile bool watercheckFlag = false;       // Flag สำหรับตรวจสอบค่าน้ำในที่พักน้ำ
volatile bool isRefilling = false;          // Flag สำหรับตรวจสอบการเติมน้ำ

int solenoidPin = 11;     // Output pin for Valv
int pumpPin = 10;         // Output pin for pump
int waterSensorPin = A0;  // Water level sensor pin

void setup() {
  Serial.begin(9600);  
  lcd.init();          
  lcd.backlight();     

  pinMode(solenoidPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(waterSensorPin, INPUT);

  digitalWrite(pumpPin, HIGH);      // ปิดการทำงานของ Relay ที่เชื่อมต่อกับปั๊ม
  digitalWrite(solenoidPin, HIGH);  // ปิดการทำงานของ Relay ที่เชื่อมต่อกับวาล์ว
  
  pinMode(2, INPUT_PULLUP);         
  attachInterrupt(digitalPinToInterrupt(2), handleInterrupt, FALLING);  
}

// Function to display "Waiting" status
void displayWaiting() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Status: Waiting");
}

// Function to display "Done" status
void displayDone() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Water Refilled");
  lcd.setCursor(0, 1);
  lcd.print("Done!");
}

// Function to display "Ready" status
void displayReady() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Status: Ready");
  lcd.setCursor(0, 1);
  lcd.print("Place Bottle");
}

// Function ตรวจสอบน้ำในที่พักน้ำ
void waterCheck() {
  if (digitalRead(waterSensorPin) == HIGH) {  // เซนเซอร์อ่านค่าเป็น 1 มีน้ำ
    digitalWrite(pumpPin, HIGH);              // ปั๊มหยุดทำงาน
    isRefilling = false;                      
  } else {                                    // เซนเซอร์อ่านค่าเป็น 0 ไม่มีน้ำ
    waterRefill();             
    isRefilling = true;
  }
  watercheckFlag = false;                     // reset Flag
}

// Function เปิดให้ปั๊มทำงาน
void waterRefill() {
  digitalWrite(pumpPin, LOW);
}

// Interrupt service routine (ISR)
void handleInterrupt() {
  interruptFlag = true;  // Set flag เมื่อเกิด Interrupt
}

void loop() {
  if (interruptFlag) {
    digitalWrite(solenoidPin, HIGH); 
    interruptFlag = false;
    displayReady();
  }

  if (watercheckFlag) {
    waterCheck();           
  }

  if (isRefilling) {
    waterCheck();       // ตรวจสอบระดับน้ำตลอดขณะกำลังเติมน้ำ
  }

  if (Serial.available() > 0) {         // ตรวจสอบว่ามีค่าที่ส่งมาจากบอร์ดที่ 1 หรือไม่
    char receivedChar = Serial.read();  // อ่านค่าที่ได้รับมาและเก็บไว้ใน receivedChar
    
    // การทำงานในกรณีต่าง ๆ เมื่อได้รับค่า 3 กรณี "R", "W" หรือ "D"
    if (receivedChar == 'W') {
      digitalWrite(pumpPin, HIGH);      
      digitalWrite(solenoidPin, LOW);   
      displayWaiting();  
      isRefilling = false;
    } 
    
    else if (receivedChar == 'D') {
      digitalWrite(solenoidPin, HIGH); 
      displayDone();  
      delay(1000);    
    } 
    
    else if (receivedChar == 'R') {
      displayReady();  
      watercheckFlag = true;  
    }
  }
}
