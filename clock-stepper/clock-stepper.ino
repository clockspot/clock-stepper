// Arduino + RTC + stepper motor for intermittently driven clock displays - https://github.com/clockspot/clock-stepper
// Sketch by Luke McKenzie (luke@theclockspot.com)
// Inspired by / adapted from:
// https://www.thingiverse.com/thing:5140134 by shiura
// https://gist.github.com/deveth0/796afa5d35a6c9d79e30008938d42e4e

#include "configs/default.h"

#include <Wire.h> //for I2C access to DS3231

//Install via Arduino IDE Library Manager:
#include <DS3231.h> //DS3231 by Andrew Wickert/NorthernWidget
#include <LowPower.h> //LowPower_LowPowerLab by LowPowerLab

//RTC objects
DS3231 ds3231;

// sequence of stepper motor control
const int STEPPER_SEQ[8][4] = {
  {  LOW, HIGH, HIGH,  LOW},
  {  LOW,  LOW, HIGH,  LOW},
  {  LOW,  LOW, HIGH, HIGH},
  {  LOW,  LOW,  LOW, HIGH},
  { HIGH,  LOW,  LOW, HIGH},
  { HIGH,  LOW,  LOW,  LOW},
  { HIGH, HIGH,  LOW,  LOW},
  {  LOW, HIGH,  LOW,  LOW}
};

void setup() {
  Serial.begin(9600);
  Serial.println(F("Hello world"));
  //motor pins
  for(int i=0; i<4; i++) pinMode(STEPPER_PINS[i], OUTPUT);
  //interrupt pins - these will wake up the Arduino
  pinMode(BTN_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_INT_PIN), handleBtnISR, FALLING);
  pinMode(RTC_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RTC_INT_PIN), handleRTCISR, FALLING);
  //rtc
  Wire.begin();
  rtcArmMinuteSignal();
}

//Interrupt flags for loop() to pick up
volatile bool setRTC = false; //set by btn ISR (sometimes)
volatile bool newMinute = false; //set by rtc ISR

void loop() {
  Serial.println(F("Loop:"));
  if(setRTC) { setRTC = false; rtcSet(); }
  if(newMinute) { newMinute = false; advance(); }
  //checkSerialInput();

  //After doing the things, go asleep again
  Serial.println(F("Sleep."));
  delay(500); //why
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void rtcSet() {
  Serial.println(F("rtc set!"));
  ds3231.setHour(0);
  ds3231.setMinute(0);
  ds3231.setSecond(0);
  //advance(); //not necessary as it will trip the rtc alarm
}

void rtcArmMinuteSignal() {
  // Serial.println(F("Arming"));
  //teach it to alarm every minute
  //TODO learn enough about bitwise ops to simplify this
  byte ALRM1_SET = 0b1110; //ALRM1_MATCH_SEC
  byte ALRM2_SET = 0b111; //ALRM2_ONCE_PER_MIN
  int alarmBits = ALRM2_SET;
  alarmBits <<= 4;
  alarmBits |= ALRM1_SET;
  ds3231.setA1Time(0,0,0,0,alarmBits,false,false,false);
  ds3231.turnOnAlarm(1);
  // Serial.println(F("Alarm 1:"));
  // Serial.println(ds3231.checkAlarmEnabled(1));
}

volatile byte stepperPosCur = 0;
volatile int substepsRemain = 0;
byte stepperSeqCur = 0;
void advance() {
  ds3231.checkIfAlarm(1); //"cancels" this instance of alarm
  //Rather than using a library like Stepper.h, which uses whole steps,
  //this code (adapted via shiura) controls more finely via substeps
  Serial.println(F("Advance!"));
  if(stepperPosCur==STEPPER_POS_PER_REV) stepperPosCur = 0;
  unsigned long substepCur =
    (unsigned long)STEPPER_SUBSTEPS * (stepperPosCur) / STEPPER_POS_PER_REV;
  unsigned long substepNext =
    (unsigned long)STEPPER_SUBSTEPS * (stepperPosCur+1) / STEPPER_POS_PER_REV;
  substepsRemain = substepNext - substepCur;
  stepperPosCur++;
  int i;
  while(substepsRemain) {
    substepsRemain--;
    stepperSeqCur = (stepperSeqCur+1)%8;
    for(i=0; i<4; i++) digitalWrite(STEPPER_PINS[i], STEPPER_SEQ[stepperSeqCur][i]);
    delay(STEPPER_DELAY);
  }
  for(i=0; i<4; i++) digitalWrite(STEPPER_PINS[i], LOW); //power cut
}

// //Serial input for control
// int incomingByte = 0;
// void checkSerialInput(){
//   if(Serial.available()>0){
//     incomingByte = Serial.read();
//     switch(incomingByte){
//       case 97: //a
//         advance(); break;
//       // case 119: //w
//       //   networkStartWiFi(); break;
//       // case 100: //d
//       //   networkDisconnectWiFi(); break;
//       case 115: //s
//         rtcSync(); break;
//       default: break;
//     }
//   }
// }

//ISRs
volatile unsigned long btnLast = 0;
void handleBtnISR() {
  //Debounce
  if((unsigned long)(millis()-btnLast)<BTN_DEBOUNCE_DUR) return;
  btnLast = millis();
  //if advancing, immediately cancel it
  if(substepsRemain) {
    stepperPosCur = 0;
    substepsRemain = 0;
  } else {
    setRTC = true; //for loop() to handle
  }
}

void handleRTCISR() {
  newMinute = true; //for loop() to handle
}