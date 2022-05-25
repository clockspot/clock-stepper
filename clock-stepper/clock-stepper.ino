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
  attachInterrupt(digitalPinToInterrupt(BTN_INT_PIN), handleBtnISR, FALLING);
  pinMode(RTC_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RTC_INT_PIN), handleRTCISR, FALLING);
  //rtc
  Wire.begin();
  rtcArmMinuteSignal();
  // rtcTakeSnap();
  // rtcSecLast = rtcGetSecond();
  // rtcMinLast = rtcGetMinute();
}

volatile bool btnISR = false; //goes true after btn ISR event
volatile bool rtcISR = false; //goes true after rtc ISR event
void loop() {
  if(btnISR) { btnISR = false; rtcSync(); }
  if(rtcISR) { rtcISR = false; minuteSignal(); }
  checkSerialInput();
  //checkRTC();
}

void checkRTC() {
  rtcTakeSnap();
  if(rtcSecLast != rtcGetSecond()) {
    rtcSecLast = rtcGetSecond();
    printRTCTime();
  }
  // if(rtcMinLast != rtcGetMinute()) {
  //   rtcMinLast = rtcGetMinute();
  //   advance();
  // }
}

void rtcTakeSnap() { tod = rtc.now(); }
byte rtcGetSecond() { return tod.second(); }
byte rtcGetMinute() { return tod.minute(); }

void rtcSync() {
  ds3231.setHour(0);
  ds3231.setMinute(0);
  ds3231.setSecond(0);
  //rtcSecLast = 0;
  //rtcMinLast = 0;
  //advance(); //not necessary as it will trip the alarm
}

void rtcArmMinuteSignal() {
  //teach it to alarm every minute
  byte ALRM1_SET = 0b1110; //ALRM1_MATCH_SEC
  byte ALRM2_SET = 0b111; //ALRM2_ONCE_PER_MIN
  //int alarmBits = 0b1111110;
  int alarmBits = ALRM2_SET;
  alarmBits <<= 4;
  alarmBits |= ALRM1_SET;
  ds3231.setA1Time(0,0,0,0,alarmBits,false,false,false);
  ds3231.turnOnAlarm(1);
  // Serial.println(F("Bits:"));
  // Serial.println(alarmBits,BIN);
  // Serial.println(F("Alarm 1:"));
  // Serial.println(ds3231.checkAlarmEnabled(1));
  // Serial.println(F("Alarm 2:"));
  // Serial.println(ds3231.checkAlarmEnabled(2));
}

void minuteSignal() {
  //from ISR via loop
  Serial.println(F("new minute from RTC!"));
  ds3231.checkIfAlarm(1);
  // Serial.println(isAlarm,DEC);
  // isAlarm = ds3231.checkIfAlarm(1);
  // Serial.println(isAlarm,DEC);
  advance();
}

void printRTCTime() {
  if(rtcGetSecond()%10>0) return;
  Serial.print(F(":"));
  if(rtcGetMinute()<10) Serial.print(F("0"));
  Serial.print(rtcGetMinute());
  Serial.print(F(":"));
  if(rtcGetSecond()<10) Serial.print(F("0"));
  Serial.print(rtcGetSecond());
  Serial.println();
  // bool isAlarm = ds3231.checkIfAlarm(1);
  // Serial.println(isAlarm,DEC);
  // isAlarm = ds3231.checkIfAlarm(1);
  // Serial.println(isAlarm,DEC);
}

void handleRTCISR() {
  rtcISR = true;
}

byte stepperPosCur = 0;
volatile byte stepsRemain = 0;

void advance() {
  if(stepsRemain!=0) { //if we are moving, or btn ISR said stop, stop
    Serial.println(F("told to stop"));
    stepsRemain = 0;
  } else { //if we are stopped, move
    if(stepperPosCur==STEPPER_POS_PER_REV) stepperPosCur = 0;
    //Find how far we need to go
    unsigned long prev = (unsigned long)STEPPER_STEPS * (stepperPosCur) / STEPPER_POS_PER_REV;
    unsigned long next = (unsigned long)STEPPER_STEPS * (stepperPosCur+1) / STEPPER_POS_PER_REV;
    stepperPosCur++;
    rotate(next-prev);
  }
}

void rotate(int step) {
  static int phase = 0;
  int i, j;
  int delta = (step > 0) ? 1 : 7;

  int stepsRemain = (step > 0) ? step : -step;
  while(stepsRemain>0) {
    phase = (phase + delta) % 8;
    for(i = 0; i < 4; i++) {
      digitalWrite(port[i], seq[phase][i]);
    }
    delay(STEPPER_DELAY);
    stepsRemain--;
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

volatile unsigned long btnLast = 0;
void handleBtnISR() {
  //Debounce
  if((unsigned long)(millis()-btnLast)<BTN_DEBOUNCE_DUR) return;
  btnLast = millis();
  //if the motor is turning, cancel it
  if(stepsRemain!=0) stepsRemain = -1; //TODO fix this
  btnISR = true; //can't actually call delays/serials/etc from within ISR
  //TODO NEXT: make duplicate presses actually stop the motion and not double up
}