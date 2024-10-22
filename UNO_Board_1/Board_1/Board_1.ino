const int segmentPins[] = { 14, 15, 16, 17, 18, 19, 8, 7 };  // A, B, C, D, E, F, G, DP
const int digitPins[] = { 9, 10, 11, 12 };                   // 1000, 100, 10, 1

const bool numberTable[10][7] = {
  { 1, 1, 1, 1, 1, 1, 0 },  // 0
  { 0, 1, 1, 0, 0, 0, 0 },  // 1
  { 1, 1, 0, 1, 1, 0, 1 },  // 2
  { 1, 1, 1, 1, 0, 0, 1 },  // 3
  { 0, 1, 1, 0, 0, 1, 1 },  // 4
  { 1, 0, 1, 1, 0, 1, 1 },  // 5
  { 1, 0, 1, 1, 1, 1, 1 },  // 6
  { 1, 1, 1, 0, 0, 0, 0 },  // 7
  { 1, 1, 1, 1, 1, 1, 1 },  // 8
  { 1, 1, 1, 1, 0, 1, 1 }   // 9
};

int Volume = 0;
bool interruptFlag = false;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 8; i++) pinMode(segmentPins[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(digitPins[i], OUTPUT);

  pinMode(3, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(3), handleInterrupt, FALLING);
  
  Serial.println("R");                  // ส่ง "R" ไปยังบอร์ดที่ 2 เพื่อแสดง State บน LCD
}

void displayNumber(int num) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], numberTable[num][i]);
  }
}

void selectDigit(int digit) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], i == digit);
  }
}

void printNumber(int number) {
  for (int i = 0; i < 4; i++) {
    int digitValue = number % 10;            
    number /= 10;                            // เลื่อนหลักในการแสดงผล
    selectDigit(i);                          // เลือกหลักที่อยู่ ณ ปัจจุบัน
    displayNumber(digitValue);               // เรียกฟังก์ชันสำหรับแสดงตัวเลขที่หลักนั้น ๆ
    delay(5);                                // ดีเลย์เพื่อให้ทำการมัลติเพลกซ์
  }
}

// Interrupt service routine (ISR)
void handleInterrupt() {
  interruptFlag = true;
}

void loop() {
  if (Serial.available() > 0) {       // เช็คว่ามีการส่งค่าปริมาตรน้ำจากบอร์ดที่ 3 หรือไ่ม่

    Volume = Serial.parseInt();       // นำค่าที่ได้รับมาจากบอร์ดที่ 3 เก็บในตัวแปร Volume
    Serial.println("W");              // ส่ง "W" ไปยังบอร์ดที่ 2 เพื่อแสดง State บน LCD
    
    // เริ่มนับถอยหลัง
    for (int countdown = Volume; countdown > 0; countdown--) {
                                      // เช็คว่ามี interrupt เกิดขึ้นหรือไม่
      if (interruptFlag) {
                                      // ถ้ามี interrupt, ให้แสดงเลข 0000 และออกจาก loop ทันที
        printNumber(0);               // แสดง 0000
        interruptFlag = false;        // รีเซ็ต flag
        return;
      }
      delay(15);
      printNumber(countdown);         // แสดงตัวเลขที่นับถอยหลัง
    }
    printNumber(0);                   // แสดง 0000 เมื่อการนับถอยหลังสิ้นสุด
    Serial.println("D");
    Serial.println("R");
  }
  else {
    printNumber(0);                   // แสดง 0000 เมื่อไม่ได้รับค่าจาก UART
  }
}

