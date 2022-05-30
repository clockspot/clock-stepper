# Clock Stepper

An Arduino sketch for use with DS3231 RTC and 28BYJ-48 stepper motor to drive intermittently-driven mechanical time displays, maintained by [Luke](https://theclockspot.com) and inspired by [shiura](https://www.thingiverse.com/thing:5140134).

Once per minute, the RTC will trigger the Arduino to advance the stepper motor.

The pushbutton will zero the RTC and trigger a manual advance – unless it is pressed while the motor is advancing, which will teach the Arduino where to stop the motor between advances (this recalibration may be required after a series of power losses, if the motor is stopping in the middle of the display change).

For power efficiency (or at least, an attempt at it), the Arduino spends most of its time in low-power sleep, and these activities are triggered via interrupt pins.

## Hardware wiring/mods

* Arduino to RTC:
  * A4 → SDA
  * A5 → SCL
  * D2 → SQW
  * 3V3 → VIN
  * GND → GND
* Arduino to stepper driver:
  * D4–D7 → IN1–IN4 (order can be reversed and corrected for in config file)
  * 5V → VIN (or use a separate supply)
  * GND → GND (or use a separate supply)
  * _Note: D8–D11 may be used to control a second stepper driver in future_
* Voltage supply → Arduino VIN/GND
  * _Note: For more efficient battery operation, may need to supply power in a way that bypasses the Arduino voltage regulator – TBD_
* Pushbutton → Arduino D3/GND
* For low-power purposes, remove power LEDs from Arduino and RTC (if applicable) and the A–D LEDs on stepper driver board

## Compiling

Add these libraries to your Arduino IDE:
* `<DS3231.h>` by Andrew Wickert/NorthernWidget
* `<LowPower.h>` by LowPowerLab

Make any necessary changes in `configs/default.h` (or make your own config file and reference it in `clock-stepper.ino`). In particular, change `STEPPER_POS_PER_REV` to reflect the number of minutes per stepper revolution; and if the stepper revolves in the wrong direction, change the order of `STEPPER_PINS`.

Before compiling and uploading, you will need to select the correct board, port, and (for AVR) processor in the IDE’s Tools menu.

* If your Arduino does not appear as a port option, you may have a clone that requires [drivers for the CH340 chipset](https://sparks.gogo.co.nz/ch340.html).
* If upload fails for an ATMega328P Arduino (e.g. classic Nano), try selecting/unselecting “Old Bootloader” in the processor menu.

## TODO
* Internal representation of display via sensing switches
* Drive hour / date motors
* Set RTC via IoT and wait to sync display