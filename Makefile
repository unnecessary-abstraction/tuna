################################################################################
#	Makefile for UARA.
#
#	Copyright (C) 2013 Paul Barker, Loughborough University
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
################################################################################

DSP_OBJECTS := fft.o impulse.o third_octave_levels.o time_slice.o
UARA_OBJECTS := output.o output_csv.o timespec.o uara.o $(DSP_OBJECTS)
OUTPUT_CSV_TEST_OBJECTS := output_csv_test.o output_csv.o output.o
ALL_OBJECTS := $(UARA_OBJECTS) $(OUTPUT_CSV_TEST_OBJECTS)
ALL_SOURCES := $(ALL_OBJECTS:%.o=%.c)
ALL_DEPS := $(ALL_SOURCES:%.c=.deps/%.P)
ALL_TARGETS := uara output_csv_test

# Programs used
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)gcc
RM := rm -f
INSTALL := install
FIXDEPS := python $(SOURCE_ROOT)scripts/fixdeps.py

# Paths
SOURCE_DIR := $(SOURCE_ROOT)src/
INCLUDE_DIR := $(SOURCE_ROOT)include/
VPATH := $(SOURCE_DIR) $(SOURCE_DIR)/dsp
PREFIX := $(DESTDIR)/usr/local
BINDIR := $(PREFIX)/bin

# Flags
CFLAGS := -Wall -Wextra $(EXTRA_CFLAGS)
LDFLAGS := $(EXTRA_LDFLAGS)
LDLIBRARIES := -lfftw3 -lm

# Flags to generate dependencies
DEPFLAGS := -MD

# Support for high or low verbosity
VERBOSITY := 0
ifeq ($(VERBOSITY),0)
  Q := @
else
  Q :=
endif

all: uara

.deps:
	$(Q)mkdir -p .deps

%.o: %.c .deps
	@echo CC $@
	$(Q)$(CC) $(CFLAGS) $(DEPFLAGS) -I $(INCLUDE_DIR) -I $(SOURCE_DIR) -o $@ -c $<
	$(Q)$(FIXDEPS) $*.d .deps/$*.P
	$(Q)$(RM) $*.d

uara: $(UARA_OBJECTS)
	@echo LD $@
	$(Q)$(LD) $(LDFLAGS) -o $@ $(UARA_OBJECTS) $(LDLIBRARIES)

output_csv_test: $(OUTPUT_CSV_TEST_OBJECTS)
	@echo LD $@
	$(Q)$(LD) $(LDFLAGS) -o $@ $(OUTPUT_CSV_TEST_OBJECTS) $(LDLIBRARIES)

clean-intermediates:
	@echo RM $(ALL_OBJECTS) $(ALL_DEPS) *.d
	$(Q)$(RM) $(ALL_OBJECTS) $(ALL_DEPS) *.d

clean: clean-intermediates
	@echo RM $(ALL_TARGETS)
	$(Q)$(RM) $(ALL_TARGETS)

dev-clean: clean
	@echo RM ChangeLog
	$(Q)$(RM) ChangeLog

install: uara
	@echo INSTALL uara
	$(Q)$(INSTALL) -D uara $(BINDIR)/uara

ChangeLog:
	hg log --style changelog > ChangeLog

.PHONY: dev-clean clean clean-intermediates all install

# Include dependencies
-include $(ALL_DEPS)
