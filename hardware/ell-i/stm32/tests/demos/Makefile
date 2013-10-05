#
# Make the runtime test
#

STM32_FLASH_PATH := /Volumes/Work/ST/stm32flash/
TTY := $(wildcard /dev/tty.usbserial-A*)

.SUFFIXES: .bin .lst .hex .map

all: \
	demo_led_2013_oct_9.hex

.cpp.bin: 
	(cd ../..; make all tests/demos/$(basename $@) tests/demos/$@)

.c.bin: 
	(cd ../..; make all tests/demos/$(basename $@) tests/demos/$@)

.cpp.hex: 
	(cd ../..; make all tests/demos/$(basename $@) tests/demos/$@)

.c.hex: 
	(cd ../..; make all tests/demos/$(basename $@) tests/demos/$@)

.cpp.lst: 
	(cd ../..; make all tests/demos/$(basename $@).lst)

.c.lst: 
	(cd ../..; make all tests/demos/$(basename $@).lst)

clean:
	(cd ../..; make clean)
	rm -f demo_led_2013_oct_9 demo_led_2013_oct_9.hex

flash:	all
	python ../../stm32flash.py -w ./demo_led_2013_oct_9.hex -c $(TTY)
