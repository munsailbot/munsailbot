#!/bin/sh

<<<<<<< HEAD
avrdude -C/usr/share/arduino/hardware/tools/avrdude.conf -v -patmega2560 -carduino -P/dev/$1 -D -Uflash:w:.build/motor_controller.hex:i
=======
avrdude -C/usr/share/arduino/hardware/tools/avrdude.conf -v -patmega328p -carduino -P/dev/$1 -b 57600 -D -Uflash:w:$2/motor_controller.hex:i

>>>>>>> refs/remotes/munsailbot/master
