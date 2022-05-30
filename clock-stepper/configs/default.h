#ifndef CONFIG
#define CONFIG

#define RTC_INT_PIN 2
#define BTN_INT_PIN 3

#define STEPPER_SUBSTEPS 4096
#define STEPPER_POS_PER_REV 12 //e.g. if advancing every 60 seconds, makes 1 revolution per 12 mins //TODO support non-integer
#define STEPPER_DELAY 5 //mils between steps

// ports used to control the stepper motor
// if your motor rotate to the opposite direction, 
// change the order as {7, 6, 5, 4};
const int STEPPER_PINS[4] = {4, 5, 6, 7};

#define BTN_DEBOUNCE_DUR 200

#endif