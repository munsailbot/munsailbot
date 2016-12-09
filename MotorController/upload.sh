#!/bin/sh

avrdude -C/usr/share/arduino/hardware/tools/avrdude.conf -v -patmega2560 -carduino -P/dev/$1 -D -Uflash:w:.build/motor_controller.hex:i

