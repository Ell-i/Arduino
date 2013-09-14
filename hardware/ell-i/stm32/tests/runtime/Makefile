#
# Make the runtime test
#

STM32_FLASH_PATH := /Volumes/Work/ST/stm32flash/
TTY := $(wildcard /dev/tty.usbserial-A*)

.SUFFIXES: .bin .lst

all:  test_basic_led.bin test_thread_switch.bin test_thread_switch_reg.bin test_thread_switch_reg2.bin

.cpp.bin: 
	(cd ../..; make all tests/runtime/$(basename $@) tests/runtime/$@)

.c.bin: 
	(cd ../..; make all tests/runtime/$(basename $@) tests/runtime/$@)

.cpp.lst: 
	(cd ../..; make all tests/runtime/$(basename $@).lst)

.c.lst: 
	(cd ../..; make all tests/runtime/$(basename $@).lst)

clean:
	(cd ../..; make clean)
	rm -f test_basic_led test_basic_led.bin
	rm -f test_thread_switch test_thread_switch.bin
	rm -f test_thread_switch_reg test_thread_switch_reg.bin
	rm -f test_thread_switch_reg2 test_thread_switch_reg2.bin

flash:	all
	$(STM32_FLASH_PATH)stm32flash -w ./test_basic_led.bin $(TTY)
	sleep 1
	$(STM32_FLASH_PATH)stm32flash -w ./test_thread_switch.bin $(TTY)
