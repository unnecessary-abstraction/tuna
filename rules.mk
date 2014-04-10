################################################################################
#	Top-level rules.mk for TUNA.
#
#	Copyright (C) 2013, 2014 Paul Barker, Loughborough University
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
INCLUDE_ALL := -I$(SRCDIR)/include -I$(SRCDIR)/include/tuna
CFLAGS_ALL :=
LDFLAGS_ALL := 

# Required libraries
LDLIBRARIES_ALL := 

# Initialise empty variables which each directory's `rules.mk` will append to.
TARGETS_BIN :=
TARGETS_LIB :=
TARGETS_TEST :=
TARGETS_DOCS :=
TARGETS_EXTRA :=
INTERMEDIATES :=
INSTALL_DEPS :=
CHECK_DEPS :=

# Set VPATH incase we're building outside the source tree
VPATH := $(SRCDIR)

# One rule to bring them all and in the darkness bind them
.PHONY: all
all: targets

# Pull in subdirectories
include $(SRCDIR)/bin/rules.mk
include $(SRCDIR)/libtuna/rules.mk
include $(SRCDIR)/test/rules.mk
include $(SRCDIR)/include/rules.mk
include $(SRCDIR)/swig/rules.mk

ifdef enable-docs
include $(SRCDIR)/doxy/rules.mk
endif

# Combined list of targets
TARGETS_ALL := $(TARGETS_BIN) $(TARGETS_LIB) $(TARGETS_TEST) $(TARGETS_DOCS)

.PHONY: targets
targets: $(TARGETS_ALL)

# Compiler rule
%.o: %.c
	@echo CC $@
	$(Q)$(CC) $(CFLAGS) $(CFLAGS_ALL) $(CFLAGS_TGT) $(INCLUDE_ALL) $(INCLUDE_TGT) $(DEPFLAGS) -o $@ -c $<
	$(Q)$(PYTHON) $(SRCDIR)/scripts/fixdeps.py $*.d $*.d.tmp
	$(Q)mv $*.d.tmp $*.d

%.lo: %.c
	@echo CC $@
	$(Q)$(CC) $(CFLAGS) $(CFLAGS_ALL) $(CFLAGS_TGT) -fPIC $(INCLUDE_ALL) $(INCLUDE_TGT) $(DEPFLAGS) -o $@ -c $<
	$(Q)$(PYTHON) $(SRCDIR)/scripts/fixdeps.py $*.d $*.d.tmp
	$(Q)mv $*.d.tmp $*.d

%.s: %.c
	@echo CC -S $@
	$(Q)$(CC) $(CFLAGS) $(CFLAGS_ALL) $(CFLAGS_TGT) $(INCLUDE_ALL) $(INCLUDE_TGT) -o $@ -S $<

# Linker rule
%: %.o
	@echo CCLD $@
	$(Q)$(CCLD) $(LDFLAGS) $(LDFLAGS_ALL) $(LDFLAGS_TGT) -o $@ $(filter %.o,$^) $(LDLIBRARIES_TGT) $(LDLIBRARIES_ALL) $(LDLIBRARIES)

%.so:
	@echo CCLD $@
	$(Q)$(CCLD) $(LDFLAGS) $(LDFLAGS_ALL) $(LDFLAGS_TGT) -shared -o $@ $(filter %.lo,$^) $(LDLIBRARIES_TGT) $(LDLIBRARIES_ALL) $(LDLIBRARIES)

# Archiver rule
%.a:
	@echo AR $@
	$(Q)$(AR) rcs $@ $(filter %.lo,$^)

# Clean rules
.PHONY: clean clean-intermediates
clean: clean-intermediates
	@echo CLEAN
	$(Q)rm -rf $(TARGETS_ALL) $(TARGETS_EXTRA)

clean-intermediates:
	@echo CLEAN INTERMEDIATES
	$(Q)rm -rf $(INTERMEDIATES)
	$(Q)find -name __pycache__ -prune -exec rm -rf {} \;

# Install rules
.PHONY: install 
install: $(INSTALL_DEPS)

# Check rules
.PHONY: check
check: $(CHECK_DEPS)

# Ensure everything is rebuilt if top-level rules or config change
TOPLEVEL_DEPS := $(SRCDIR)/rules.mk $(SRCDIR)/Makefile unconfig.mk

$(INTERMEDIATES): $(TOPLEVEL_DEPS)

$(TARGETS_ALL) : $(TOPLEVEL_DEPS)
