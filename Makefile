################################################################################
#	Makefile for TUNA.
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

TUNA_OBJECTS := analysis.o buffer.o bufq.o fft.o input_alsa.o input_sndfile.o \
	log.o null.o output_csv.o output_sndfile.o slab.o time_slice.o \
	timespec.o tol.o tuna.o window.o zero.o
OUTPUT_CSV_TEST_OBJECTS := output_csv_test.o output_csv.o timespec.o
ALL_OBJECTS := $(TUNA_OBJECTS) $(OUTPUT_CSV_TEST_OBJECTS)
ALL_SOURCES := $(ALL_OBJECTS:%.o=%.c)
ALL_DEPS := $(ALL_SOURCES:%.c=.deps/%.P)
ALL_TARGETS := tuna output_csv_test

# Programs used
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)gcc
RM := rm -f
INSTALL := install
FIXDEPS := python $(SOURCE_ROOT)scripts/fixdeps.py

# Paths
SOURCE_DIR := $(SOURCE_ROOT)src
INCLUDE_DIR := $(SOURCE_ROOT)include
VPATH := $(SOURCE_DIR)
PREFIX := $(DESTDIR)/usr/local
BINDIR := $(PREFIX)/bin

# Flags
CFLAGS := -Wall -Wextra -pthread $(EXTRA_CFLAGS)
LDFLAGS := -pthread $(EXTRA_LDFLAGS)
LDLIBRARIES := -lfftw3 -lm -lsndfile -lrt -lasound

# Flags to generate dependencies
DEPFLAGS := -MD

# Support for high or low verbosity
VERBOSITY := 0
ifeq ($(VERBOSITY),0)
  Q := @
else
  Q :=
endif

all: tuna

test: test_cmds

test_cmds: output_csv_test

%.o: %.c
	@echo CC $@
	$(Q)mkdir -p .deps
	$(Q)$(CC) $(CFLAGS) $(DEPFLAGS) -I $(INCLUDE_DIR) -I $(SOURCE_DIR) -o $@ -c $<
	$(Q)$(FIXDEPS) $*.d .deps/$*.P
	$(Q)$(RM) $*.d

tuna: $(TUNA_OBJECTS)
	@echo LD $@
	$(Q)$(LD) $(LDFLAGS) -o $@ $(TUNA_OBJECTS) $(LDLIBRARIES)

output_csv_test: $(OUTPUT_CSV_TEST_OBJECTS)
	@echo LD $@
	$(Q)$(LD) $(LDFLAGS) -o $@ $(OUTPUT_CSV_TEST_OBJECTS) $(LDLIBRARIES)

clean-intermediates:
	@echo CLEAN INTERMEDIATES
	$(Q)$(RM) $(ALL_OBJECTS) $(ALL_DEPS) *.d

clean: clean-intermediates
	@echo CLEAN
	$(Q)$(RM) $(ALL_TARGETS)

dev-clean: clean
	@echo DEVCLEAN
	$(Q)$(RM) ChangeLog

install: tuna
	@echo INSTALL tuna
	$(Q)$(INSTALL) -D tuna $(BINDIR)/tuna

ChangeLog:
	@echo GIT LOG > Changelog
	$(Q)git log > ChangeLog

.PHONY: dev-clean clean clean-intermediates all install

# Include dependencies
-include $(ALL_DEPS)
