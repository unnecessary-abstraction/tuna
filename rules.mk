################################################################################
#	Top-level rules.mk for TUNA.
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

# Required flags which cannot be configured by the user
CFLAGS_ALL := -pthread -I$(SRCDIR)/include -I$(SRCDIR)/include/tuna
LDFLAGS_ALL := -pthread

# Required libraries
LDLIBRARIES_ALL := -lfftw3 -lm -lsndfile -lrt -lasound

# Initialise empty variables which each directory's `rules.mk` will append to.
TARGETS_BIN :=
TARGETS_LIB :=
TARGETS_TEST :=
INTERMEDIATES :=

# Set VPATH incase we're building outside the source tree
VPATH := $(SRCDIR)

# One rule to bring them all and in the darkness bind them
.PHONY: all
all: targets

# Pull in subdirectories
dir := cmd
include $(SRCDIR)/$(dir)/rules.mk

dir := libtuna
include $(SRCDIR)/$(dir)/rules.mk

dir := test
include $(SRCDIR)/$(dir)/rules.mk

# Combined list of targets
TARGETS_ALL := $(TARGETS_BIN) $(TARGETS_LIB) $(TARGETS_TEST)

.PHONY: targets
targets: $(TARGETS_ALL)

# Compiler rule
%.o: %.c
	@echo CC $@
	$(Q)$(CC) $(CFLAGS) $(CFLAGS_ALL) $(CFLAGS_TGT) $(DEPFLAGS) -o $@ -c $<
	$(Q)$(PYTHON) $(SRCDIR)/scripts/fixdeps.py $*.d $*.d.tmp
	$(Q)$(MV) $*.d.tmp $*.d

# Linker rule
%: %.o
	@echo LD $@
	$(Q)$(LD) $(LDFLAGS) $(LDFLAGS_ALL) $(LDFLAGS_TGT) -o $@ $(filter %.o,$^) $(LDLIBRARIES_ALL) $(LDLIBRARIES_TGT) $(LDLIBRARIES)

# Archiver rule
%.a:
	@echo AR $@
	$(Q)$(AR) rcs $@ $(filter %.o,$^)

# Clean rules
.PHONY: clean clean-intermediates
clean: clean-intermediates
	@echo CLEAN
	$(Q)$(RM) $(TARGETS_ALL)

clean-intermediates:
	@echo CLEAN INTERMEDIATES
	$(Q)$(RM) $(INTERMEDIATES)


# Ensure everything is rebuilt if top-level rules or config change
TOPLEVEL_DEPS := $(SRCDIR)/rules.mk $(SRCDIR)/Makefile config.mk devconfig.mk

$(INTERMEDIATES): $(TOPLEVEL_DEPS)

$(TARGETS_BIN): $(TOPLEVEL_DEPS)

$(TARGETS_LIB): $(TOPLEVEL_DEPS)

$(TARGETS_TEST): $(TOPLEVEL_DEPS)

# Create an empty devconfig.mk if one doesn't exist so that the previous rule doesn't bork
devconfig.mk:
	$(Q)touch devconfig.mk