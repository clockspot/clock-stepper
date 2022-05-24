// Arduino + RTC + stepper motor for intermittently driven clock displays - https://github.com/clockspot/clock-stepper
// Sketch by Luke McKenzie (luke@theclockspot.com)
// Inspired by / adapted from shiura https://www.thingiverse.com/thing:5140134

#include "configs/default.h"

#include <Wire.h> //Arduino - GNU LPGL - for I2C access to DS3231
#include <DS3231.h> //Andrew Wickert/NorthernWidget - The Unlicense - install in your Arduino IDE

#define RTC_INT_PIN 2
#define BTN_INT_PIN 3

//RTC objects
DS3231 ds3231;
RTClib rtc;
DateTime tod;
byte rtcSecLast = 61;
byte rtcMinLast = 61;

// sequence of stepper motor control
const int seq[8][4] = {
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
  //motor
  pinMode(port[0], OUTPUT);
  pinMode(port[1], OUTPUT);
  pinMode(port[2], OUTPUT);
  pinMode(port[3], OUTPUT);
  //interrupt pins
  pinMode(BTN_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_INT_PIN), handleBtn, FALLING);
  //rtc
  Wire.begin();
  rtcTakeSnap();
  rtcSecLast = rtcGetSecond();
  rtcMinLast = rtcGetMinute();
}

void loop() {
  //wait for serial input
  checkSerialInput();
  checkRTC();
}

void checkRTC() {
  rtcTakeSnap();
  if(rtcSecLast != rtcGetSecond()) {
    rtcSecLast = rtcGetSecond();
    printRTCTime();
  }
  if(rtcMinLast != rtcGetMinute()) {
    rtcMinLast = rtcGetMinute();
    advance();
  }
}

void rtcTakeSnap() { tod = rtc.now(); }
byte rtcGetSecond() { return tod.second(); }
byte rtcGetMinute() { return tod.minute(); }
void rtcSync() {
  ds3231.setMinute(0);
  ds3231.setSecond(0);
  rtcSecLast = 0;
  rtcMinLast = 0;
  advance();
}

void printRTCTime() {
  Serial.print(F(":"));
  if(rtcGetMinute()<10) Serial.print(F("0"));
  Serial.print(rtcGetMinute());
  Serial.print(F(":"));
  if(rtcGetSecond()<10) Serial.print(F("0"));
  Serial.print(rtcGetSecond());
  Serial.println();
}

byte stepperPosCur = 0;

void advance() {
  if(stepperPosCur==STEPPER_POS_PER_REV) stepperPosCur = 0;
  //Find how far we need to go
  unsigned long prev = (unsigned long)STEPPER_STEPS * (stepperPosCur) / STEPPER_POS_PER_REV;
  unsigned long next = (unsigned long)STEPPER_STEPS * (stepperPosCur+1) / STEPPER_POS_PER_REV;
  stepperPosCur++;
  rotate(next-prev);
}

void rotate(int step) {
  static int phase = 0;
  int i, j;
  int delta = (step > 0) ? 1 : 7;

  step = (step > 0) ? step : -step;
  for(j = 0; j < step; j++) {
    phase = (phase + delta) % 8;
    for(i = 0; i < 4; i++) {
      digitalWrite(port[i], seq[phase][i]);
    }
    delay(STEPPER_DELAY);
  }
  // power cut
  for(i = 0; i < 4; i++) {
    digitalWrite(port[i], LOW);
  }
}

//Serial input for control
int incomingByte = 0;
void checkSerialInput(){
  if(Serial.available()>0){
    incomingByte = Serial.read();
    switch(incomingByte){
      case 97: //a
        advance(); break;
      // case 119: //w
      //   networkStartWiFi(); break;
      // case 100: //d
      //   networkDisconnectWiFi(); break;
      case 115: //s
        rtcSync(); break;
      default: break;
    }
  }
}

unsigned long btnLast = 0;
void handleBtn() {
  //Debounce
  if((unsigned long)(millis()-btnLast)<BTN_DEBOUNCE_DUR) return;
  btnLast = millis();
  //Serial.println(F("Went low!"));
  rtcSync();
}