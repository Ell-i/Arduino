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
		    $(SYSTEM_PATH)/net
INCLUDE_CFLAGS := -I$(VARIANT_PATH) -I$(CORE_PATH) -I$(SYSTEM_PATH)/include \
		  -I$(SYSTEM_PATH)/enc28j60 \
		  -I$(SYSTEM_PATH)/net \
	          -I$(SYSTEM_PATH)  -I$(CMSIS_PATH)/Include

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
SIZE     := $(COMPILER_PATH)arm-none-eabi-size

VARIANT_CFLAGS := -mcpu=cortex-m0 -mthumb -DF_CPU=48000000L 

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
VARIANT_OBJS := startup_stm32f0xx.o stm32f051_init.o variant.o net.o
CORE_OBJS    := $(patsubst %.c,   %.o, $(CORE_C_SRCS))   \
                $(patsubst %.cpp, %.o, $(CORE_CXX_SRCS))
SYSTEM_OBJS  := system_stm32f0xx.o
ETH_OBJS     := enc28j60.o enc28j60_init.o enc28j60_packet.o enc28j60_stm32f0_spi.o
NET_OBJS     := uip.o uip_arp.o uiplib.o

OBJS         := $(addprefix $(OBJDIR)/, $(VARIANT_OBJS) $(CORE_OBJS) $(SYSTEM_OBJS))
OBJS	     += $(addprefix $(OBJDIR)/, $(ETH_OBJS) $(NET_OBJS))

$(OBJDIR)/%.o : %.c
	$(COMPILE.c) $< -o $@

$(OBJDIR)/%.o : %.cpp
	$(COMPILE.cpp) $< -o $@

$(OBJDIR)/%.o : %.s
	$(COMPILE.s) $< -o $@
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
	-Wl,--entry=Reset_Handler \
	-Wl,--warn-common -Wl,--warn-section-align \
	-Wl,--start-group ${OBJS}  -Wl,--end-group

# -Wl,--unresolved-symbols=report-all \
# -Wl,--warn-unresolved-symbols \
