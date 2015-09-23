#!/bin/sh

avrdude -C/usr/share/arduino/hardware/tools/avrdude.conf -v -patmega328p -carduino -P/dev/$1 -b 57600 -D -Uflash:w:$2/winch_controller.hex:i

