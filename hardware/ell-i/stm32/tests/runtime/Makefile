#
# Make the runtime test
#

STM32_FLASH_PATH := /Volumes/Work/ST/stm32flash/
TTY := $(wildcard /dev/tty.usbserial-A*)

all:
	(cd ../..; make tests/runtime/runtime_test.bin)

clean:
	(cd ../..; make clean)
	rm -f runtime_test

flash:	all
	$(STM32_FLASH_PATH)stm32flash -w ./runtime_test.bin $(TTY)

