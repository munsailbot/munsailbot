#!/bin/sh

avrdude -C/usr/share/arduino/hardware/tools/avrdude.conf -v -patmega2560 -cwiring -P/dev/$1 -D -Uflash:w:./bin/sailbot/$2/RTuinOS_sailbot.hex:i

