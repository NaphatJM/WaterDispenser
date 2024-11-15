#include <avr/io.h>
#include <Arduino.h>

//---------------------------------------------------------------------------------------------
//R for Right & L for Left
#define echoPinLowR 13
#define trigPinLowR 12

#define echoPinMidR 11
#define trigPinMidR 10

#define echoPinTopR 9
#define trigPinTopR 8

#define echoPinLowL 7
#define trigPinLowL 6

#define echoPinMidL 5
#define trigPinMidL 4

#define echoPinTopL 3
#define trigPinTopL 2
//---------------------------------------------------------------------------------------------

bool volumeCalculated = false;  // flag ตรวจสอบว่าผ่านการคำนวณแล้วรึยัง
float Sound_Convert = 29.1545 * 2; //สำหรับการใช้หาระยะของเซนเซอร์
bool bottlePresent = false; //flag ตรวจสอบว่ายังมีขวดวางอยู่หรือไม่
enum State {IDLE, LOWER, MID, TOP}; //flag ระดับความสูงของขวด

float Volume;
float distanceTL, distanceML, distanceLL, distanceTR, distanceMR, distanceLR;
char SensorLV;


//---------------------------------------------------------------------------------------------

void setup() 
{
  DDRD = 0b01010110;
  DDRB = 0b00010101;
  Serial.begin(9600);

  SensorLV = IDLE;
  volumeCalculated = false;
  bottlePresent = false;

  //initial ค่าความกว้างของเลเซอร์
  distanceTL = 29;
  distanceML = 29;
}

//---------------------------------------------------------------------------------------------

void call_formular()
{
  float radiusBottom = ((29 - distanceLL) - distanceLR) / 2;
  float radiusMid = ((29 - distanceML) - distanceMR) / 2;
  float radiusTop = ((29 - distanceTL) - distanceTR) / 2;
  float radiusEffective = radiusBottom;

  int TALL; 

  // เลือกที่จะใช้ radiusBottom ในทุก เงื่อนไขความสูง เพื่อความแม่นยำ
  if (SensorLV == TOP) 
  {
    radiusEffective = radiusBottom;  
    TALL = 24;  
  } 
  else if (SensorLV == MID) 
  {
    radiusEffective = radiusBottom;
    TALL = 16;
  } 
  else if (SensorLV == LOWER) 
  {
    radiusEffective = radiusBottom;
    TALL = 7;
  }

  if (SensorLV == IDLE)
  {
    return;
  }

  Volume = 3.14159 * radiusEffective * radiusEffective * TALL; //สูตรปริมาตรทรงกระบอก

  if (!volumeCalculated) {
    int new_Volume = Volume * 0.9;
    // ที่ส่งสองรอบเพราะ hardware ของจริง ส่งครั้งเดียวแล้วไม่ขึ้น 
    // Serial.println(new_Volume);
    Serial.println(new_Volume);
    volumeCalculated = true;
  }
}


//---------------------------------------------------------------------------------------------

float trigger(int pintrig, int pinecho)  
{
  float dis, total = 0;
  const int numReadings = 5;

  for (int i = 0; i < numReadings; i++) {
    long duration;

    // เป็นคำสั่งตามใน document
    digitalWrite(pintrig, LOW);
    delayMicroseconds(2);
    digitalWrite(pintrig, HIGH);
    delayMicroseconds(10);
    digitalWrite(pintrig, LOW);

    duration = pulseIn(pinecho, HIGH);
    dis = duration / Sound_Convert; 
    total += dis;

    delay(10);
  }

  return total / numReadings; //คืนค่าเป็นค่าเฉลี่ยของระยะห่าง
}
//---------------------------------------------------------------------------------------------

unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 50;

//---------------------------------------------------------------------------------------------

void call_trig()  //ฟังก์ชันที่ใช้ switch case กำหนดการทำงานของโปรแกรม
{
  unsigned long currentTime = millis();
  if (currentTime - lastCheckTime >= checkInterval) {
    lastCheckTime = currentTime;

    switch (SensorLV) {
      case LOWER: 
        call_LOWER(); 
        break;
      case MID: 
        call_MID(); 
        break;
      case TOP: 
        call_TOP(); 
        break;
      default: 
        call_DEFAULT();
    }
  }
}
//---------------------------------------------------------------------------------------------

void call_DEFAULT()
{
  float sumL = 0;
  float sumR = 0;
  for (int i = 0; i < 3; i++) {
    sumL += trigger(trigPinLowL, echoPinLowL);
    sumR += trigger(trigPinLowR, echoPinLowR);
    delay(10);
  }
  
  distanceLL = sumL / 3;
  distanceLR = sumR / 3;
  
  if (distanceLL <= 20 || distanceLR <= 20) { //ตรวจสอบว่ามีขวดหรือไม่
    SensorLV = LOWER;
    bottlePresent = true;
    call_trig();
  } else {
    // Serial.println("There's no bottle here."); //ไม่มีขวด
  }
}

//---------------------------------------------------------------------------------------------

void call_LOWER()
{
  delay(1000);
  distanceLL = trigger(trigPinLowL,echoPinLowL);
  distanceLR = trigger(trigPinLowR,echoPinLowR);

  
  distanceML = trigger(trigPinMidL, echoPinMidL);
  distanceMR = trigger(trigPinMidR, echoPinMidR);
  delay(100);
    if (distanceML <= 20 || distanceMR <= 20) //ตรวจสอบว่าสูงถือระดับกลางหรือไม่
    {
      SensorLV = MID;
      call_trig();  
    }
    else
    {

    }
}

//---------------------------------------------------------------------------------------------

void call_MID()
{
  delay(1000);
  distanceML = trigger(trigPinMidL, echoPinMidL);
  distanceMR = trigger(trigPinMidR, echoPinMidR);


  distanceTL = trigger(trigPinTopL, echoPinTopL); 
  distanceTR = trigger(trigPinTopR, echoPinTopR); 
  delay(100);
  if (distanceTL <= 20 || distanceTR <= 20) //ตรวจสอบว่าสูงถือระดับสูงหรือไม่
  {
    SensorLV = TOP;
    call_trig(); 
  }
  else
  {

  }
}

//---------------------------------------------------------------------------------------------

void call_TOP()
{
  distanceTL = trigger(trigPinTopL, echoPinTopL);
  distanceTR = trigger(trigPinTopR, echoPinTopR);
}

//---------------------------------------------------------------------------------------------

int idleCounter = 0;  // Tracks how many times no object is detected

//---------------------------------------------------------------------------------------------

void check_bottle_present()  //เช็คว่ายังมีขวดอยู่อีกหรือไม่
{
  float sumL = 0;
  float sumR = 0;
  for (int i = 0; i < 3; i++) {
    sumL += trigger(trigPinLowL, echoPinLowL);
    sumR += trigger(trigPinLowR, echoPinLowR);
    delay(10);
  }
  
  distanceLL = sumL / 3;
  distanceLR = sumR / 3;

  if (distanceLL > 20 && distanceLR > 20) { //กรณีไม่เจอขวด
    idleCounter++;
    if (idleCounter >= 1) {  // If 3 consecutive checks show no bottle
      SensorLV = IDLE;
      bottlePresent = false;
      idleCounter = 0;  // Reset counter
    }
  } 
  else {
    idleCounter = 0;  // Reset counter if a bottle is detected
  }
}

//---------------------------------------------------------------------------------------------

void loop() 
{
  call_trig();
  
  if (!volumeCalculated && bottlePresent) 
  {
    call_formular();  // Calculate the volume
  } 
  else if (volumeCalculated) // Once volume is calculated, check if the bottle is still present
  {
    check_bottle_present();
    
    // Only reset when the bottle is removed
    if (!bottlePresent) {
      volumeCalculated = false; // Allow recalculation when bottle is removed and placed again
      SensorLV = IDLE;           // Reset the sensor state to IDLE
    }
  }
}

//---------------------------------------------------------------------------------------------