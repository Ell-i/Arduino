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

VPATH          :=   $(VARIANT_PATH)   $(CORE_PATH)   $(SYSTEM_PATH)/source
INCLUDE_CFLAGS := -I$(VARIANT_PATH) -I$(CORE_PATH) -I$(SYSTEM_PATH)/include \
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
	-nostdlib \
	--param max-inline-insns-single=500 \
	-Dprintf=iprintf \
	-DARDUINO \
	$(VARIANT_CFLAGS) \
	$(INCLUDE_CFLAGS)

CFLAGS   := $(BASE_CFLAGS) -std=gnu99
CXXFLAGS := $(BASE_CFLAGS) \
	-fno-rtti -fno-exceptions -std=gnu++0x

#
# Dynamic source file list
# 
CORE_C_SRCS    := $(notdir $(wildcard $(CORE_PATH)/*.c))
CORE_CXX_SRCS  := $(notdir $(wildcard $(CORE_PATH)/*.cpp))

#
# Object files
#
VARIANT_OBJS := startup_stm32f0xx.o stm32f051_init.o variant.o
CORE_OBJS    := $(patsubst %.c,   %.o, $(CORE_C_SRCS))   \
                $(patsubst %.cpp, %.o, $(CORE_CXX_SRCS))
SYSTEM_OBJS  := system_stm32f0xx.o

OBJS         := $(addprefix $(OBJDIR)/, $(VARIANT_OBJS) $(CORE_OBJS) $(SYSTEM_OBJS))

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
