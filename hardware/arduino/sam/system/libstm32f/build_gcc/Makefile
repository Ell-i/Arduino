#
#  Copyright (c) 2011 Arduino.  All right reserved.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

# Makefile for compiling libchip

SUBMAKE_OPTIONS=--no-builtin-rules --no-builtin-variables --no-print-directory

#-------------------------------------------------------------------------------
# Rules
#-------------------------------------------------------------------------------

all: libstm32f_051_gcc_dbg.a libstm32f_051_gcc_rel.a ellduino_stm32f051

.PHONY: ellduino_stm32f051
ellduino_stm32f051:
	@echo ------------------------------------------------------------------------------------
	@echo --- Making $@
	@echo ---
	@$(MAKE) CHIP=__STM32F051__ $(SUBMAKE_OPTIONS) OUTPUT_BIN=../../../variants/ellduino_stm32f051 -f sam3.mk
	@echo ------------------------------------------------------------------------------------

.PHONY: libstm32f_051_gcc_dbg.a
libstm32f_051_gcc_dbg.a:
	@echo ------------------------------------------------------------------------------------
	@echo --- Making $@
	@echo ---
	@$(MAKE) CHIP=__STM32F051__ DEBUG=1 $(SUBMAKE_OPTIONS) -f stm32f.mk
	@echo ------------------------------------------------------------------------------------

.PHONY: libstmp32f_051_gcc_rel.a
libstm32f_051_gcc_rel.a:
	@echo ------------------------------------------------------------------------------------
	@echo --- Making $@
	@echo ---
	@$(MAKE) CHIP=__STM32F051__ $(SUBMAKE_OPTIONS) -f stm32f.mk
	@echo ------------------------------------------------------------------------------------

.PHONY: clean
clean:
	@echo ------------------------------------------------------------------------------------
	@echo --- Cleaning stm32f release and debug
	@$(MAKE) CHIP=__STM32F051__ $(SUBMAKE_OPTIONS) -f stm32f.mk $@
	@$(MAKE) CHIP=__STM32F051__ DEBUG=1 $(SUBMAKE_OPTIONS) -f stm32f.mk $@
	@echo ------------------------------------------------------------------------------------


