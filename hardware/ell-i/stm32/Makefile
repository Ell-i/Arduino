#
# Copyright (c) 2013 Pekka Nikander.  All rights reserved.  See NOTICE for licencing.
#
# As simple as possible Makefile to compile the Ell-i Arduino IDE extension
#

#
# Use the Arduino compiler
#
COMPILER_PATH := ../../../build/macosx/work/Arduino.app/Contents/Resources/Java/hardware/tools/g++_arm_none_eabi/bin/

#
# Build target
#
CHIP    := stm32f051
VARIANT := ellduino_$(CHIP)

#
# Location of the source and destination directories
#

BUILD_PATH   := build
CORE_PATH    := cores/arduino
SYSTEM_PATH  := system/libstm32
VARIANT_PATH := variants/$(VARIANT)
CMSIS_PATH   := ../../arduino/sam/system/CMSIS/CMSIS

VPATH          :=   $(VARIANT_PATH)   $(CORE_PATH)   $(SYSTEM_PATH)/source  \
		    $(SYSTEM_PATH)/enc28j60 \
		    $(SYSTEM_PATH)/contiki/sys \
		    $(SYSTEM_PATH)/contiki/lib \
		    $(SYSTEM_PATH)/contiki/net \
		    $(SYSTEM_PATH)/contiki/webserver \
		    $(SYSTEM_PATH)/contiki/apps/er-coap-13 \
		    $(SYSTEM_PATH)/contiki/apps/erbium \

INCLUDE_CFLAGS := -I$(VARIANT_PATH) -I$(CORE_PATH) -I$(SYSTEM_PATH)/include \
		  -I$(SYSTEM_PATH)/enc28j60 \
		  -I$(SYSTEM_PATH)/contiki/net \
		  -I$(SYSTEM_PATH)/contiki/sys \
		  -I$(SYSTEM_PATH)/contiki \
		  -I$(SYSTEM_PATH)/contiki/apps/erbium \
		  -I$(SYSTEM_PATH)/contiki/apps/er-coap-13 \
	          -I$(SYSTEM_PATH)  -I$(CMSIS_PATH)/Include \

OBJDIR         := $(BUILD_PATH)

TARGET         := $(VARIANT_PATH)/lib$(CHIP)_gcc_rel.a

#
# Compiler definitions.  Should match with ones in platform.txt
#

CC       := $(COMPILER_PATH)arm-none-eabi-gcc
AS       := $(COMPILER_PATH)arm-none-eabi-as
CXX      := $(COMPILER_PATH)arm-none-eabi-g++
AR       := $(COMPILER_PATH)arm-none-eabi-ar
OBJCOPY  := $(COMPILER_PATH)arm-none-eabi-objcopy
OBJDUMP  := $(COMPILER_PATH)arm-none-eabi-objdump
SIZE     := $(COMPILER_PATH)arm-none-eabi-size

VARIANT_CFLAGS := -mcpu=cortex-m0 -mthumb -DF_CPU=48000000L 
#if 1
VARIANT_CFLAGS += -DREST=coap_rest_implementation \
	-DCOAP_MAX_OPEN_TRANSACTIONS=2 \
	-DUIP_CONF_BUFFER_SIZE=256 \
	-DWITH_COAP=13
#endif

BASE_CFLAGS   := -g -Os -w \
	-ffunction-sections -fdata-sections \
	--param max-inline-insns-single=500 \
	-Dprintf=iprintf \
	-DARDUINO=153 -DARDUINO_ELLI_STM32F051 -DARDUINO_ARCH_STM32 \
	$(VARIANT_CFLAGS) \
	$(INCLUDE_CFLAGS)

CFLAGS   := $(BASE_CFLAGS) -std=gnu99
CXXFLAGS := $(BASE_CFLAGS) \
	-fno-rtti -fno-exceptions -std=gnu99 # -std=gnu++0x

#
# Dynamic source file list
# 
CORE_C_SRCS    := $(notdir $(wildcard $(CORE_PATH)/*.c))
CORE_CXX_SRCS  := $(notdir $(wildcard $(CORE_PATH)/*.cpp))

#
# Object files
#
VARIANT_OBJS := startup_stm32f0xx.o \
	        stm32f051_init.o \
	        variant.o \
	        net.o \
	        HardwareSerial_irqHandler.o
CORE_OBJS    := $(patsubst %.c,   %.o, $(CORE_C_SRCS))   \
                $(patsubst %.cpp, %.o, $(CORE_CXX_SRCS))
SYSTEM_OBJS  := system_stm32f0xx.o \
		newlib_syscalls.o \
		thread_switch.o \
		thread_idle.o \
		thread_exit.o \
		serial_helper.o
ETH_OBJS     := enc28j60.o enc28j60_init.o enc28j60_packet.o enc28j60_stm32f0_spi.o
TCP_OBJS     := autostart.o process.o timer.o tcpip.o
NET_OBJS     := uip.o uip_arp.o uiplib.o
COAP_OBJS    := process.o etimer.o erbium.o list.o memb.o timer.o stimer.o \
		tcpip.o random.o uip-udp-packet.o \
		er-coap-13.o er-coap-13-engine.o er-coap-13-observing.o  \
		er-coap-13-separate.o er-coap-13-transactions.o \
		er-example-server.o autostart.o
 DHCP_OBJS    := dhcpc.o 
# WEBSERV_OBJS := psock.o memb.o httpd.o http-strings.o httpd-cgi.o httpd-fs.o \
#                 webserver-nogui.o

OBJS         := $(addprefix $(OBJDIR)/, $(VARIANT_OBJS) $(CORE_OBJS) $(SYSTEM_OBJS))
OBJS	     += $(addprefix $(OBJDIR)/, $(ETH_OBJS) $(NET_OBJS))
# OBJS         += $(addprefix $(OBJDIR)/, $(TCP_OBJS))
OBJS         += $(addprefix $(OBJDIR)/, $(DHCP_OBJS))
# OBJS         += $(addprefix $(OBJDIR)/, $(WEBSERV_OBJS))
OBJS         += $(addprefix $(OBJDIR)/, $(COAP_OBJS))


$(OBJDIR)/%.o : %.c
	$(COMPILE.c) $< -o $@

$(OBJDIR)/%.o : %.cpp
	$(COMPILE.cpp) $< -o $@

$(OBJDIR)/%.o : %.s
	$(COMPILE.s) $< -o $@

.SUFFIXES: .bin .hex .lst .map

%.bin: %
	$(OBJCOPY) -O binary $< $@

%.hex: %
	$(OBJCOPY) -O ihex $< $@

%.lst: %
	$(OBJDUMP) -S $< > $@

#
# Targets
#
all:	$(BUILD_PATH) $(TARGET)

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)

$(TARGET): $(OBJS)
	$(AR) -rs $@ $(OBJS)

clean:
	$(RM) $(TARGET) $(OBJS)
	@if [ -d $(BUILD_PATH) ]; then echo rmdir $(BUILD_PATH); rmdir $(BUILD_PATH); fi

#
# For testing
#

LDFLAGS  := \
	-Tvariants/ellduino_stm32f051/linker_scripts/gcc/flash.ld \
	-lm -lgcc -mthumb \
	-Wl,--check-sections -Wl,--gc-sections \
	-Wl,-Map=test.map \
	-Wl,--entry=Reset_Handler \
	-Wl,--warn-common -Wl,--warn-section-align \
	-Wl,--start-group ${OBJS}  -Wl,--end-group \

# -Wl,--unresolved-symbols=report-all \
# -Wl,--warn-unresolved-symbols \
