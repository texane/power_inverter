#!/usr/bin/env sh

# /home/texane/repo/arduino/arduino-1.0.5/hardware/tools/avrdude \
# -C/home/texane/repo/arduino/arduino-1.0.5/hardware/tools/avrdude.conf \
# -v -v -v -v -patmega328p -cstk500v1 -P/dev/ttyACM0 -b115200 -D -Uflash:w:main.hex:i

/home/texane/repo/arduino/arduino-0022/hardware/tools/avrdude -C/home/texane/repo/arduino/arduino-0022/hardware/tools/avrdude.conf -v -v -v -v -patmega328p -cstk500v1 -P/dev/ttyACM0 -b115200 -D -Uflash:w:main.hex:i