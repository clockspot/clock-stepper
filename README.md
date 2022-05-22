# Clock Stepper

An Arduino sketch for use with DS3231 RTC and 28BYJ-48 stepper motor to drive intermittently-driven clock displays, maintained by [Luke](https://theclockspot.com) and inspired by [shiura](https://www.thingiverse.com/thing:5140134).

## Compiling

Libraries required:
* EEPROM (Arduino) for AVR Arduinos (e.g. classic Nano)
* Wire (Arduino) and [DS3231](https://github.com/NorthernWidget/DS3231) if using DS3231 RTC (via I2C)

Before compiling and uploading, you will need to select the correct board, port, and (for AVR) processor in the IDE’s Tools menu.

* If your Arduino does not appear as a port option, you may have a clone that requires [drivers for the CH340 chipset](https://sparks.gogo.co.nz/ch340.html).
* If upload fails for an ATMega328P Arduino (e.g. classic Nano), try selecting/unselecting “Old Bootloader” in the processor menu.

## TODO
* Drive stepper motor at RTC minute change
* Manual advance/sync using button interrupt
* Low-power mode with RTC interrupt
* Internal representation of display via sensing switches
* Drive hour / date motors
* Set RTC via IoT and wait to sync display