#
# Make the runtime test
#

STM32_FLASH_PATH := /Volumes/Work/ST/stm32flash/
TTY := $(wildcard /dev/tty.usbserial-A*)

.SUFFIXES: .bin .lst .hex .map

all:  test_basic_led.hex \
	test_thread_switch.hex \
	test_thread_switch_reg.hex \
	test_thread_switch_reg2.hex \
	test_serial.hex \
	test_printf.hex \
	test_adc.hex \

.cpp.bin: 
	(cd ../..; make all tests/runtime/$(basename $@) tests/runtime/$@)

.c.bin: 
	(cd ../..; make all tests/runtime/$(basename $@) tests/runtime/$@)

.cpp.hex: 
	(cd ../..; make all tests/runtime/$(basename $@) tests/runtime/$@)

.c.hex: 
	(cd ../..; make all tests/runtime/$(basename $@) tests/runtime/$@)

.cpp.lst: 
	(cd ../..; make all tests/runtime/$(basename $@).lst)

.c.lst: 
	(cd ../..; make all tests/runtime/$(basename $@).lst)

clean:
	(cd ../..; make clean)
	rm -f test_basic_led test_basic_led.hex
	rm -f test_thread_switch test_thread_switch.hex
	rm -f test_thread_switch_reg test_thread_switch_reg.hex
	rm -f test_thread_switch_reg2 test_thread_switch_reg2.hex
	rm -f test_serial test_serial.hex
	rm -f test_printf test_printf.hex
	rm -f test_adc test_adc.hex

flash:	all
	$(STM32_FLASH_PATH)stm32flash -w ./test_basic_led.hex $(TTY)
	sleep 1
	$(STM32_FLASH_PATH)stm32flash -w ./test_thread_switch.hex $(TTY)
