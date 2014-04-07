################################################################################
#	rules.mk for TUNA binaries.
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

d := bin

# Targets and intermediates in this directory
objs_tuna := $(d)/tuna.o

objs := $(objs_tuna)

deps := $(objs:%.o=%.d)

tgts := $(d)/tuna

TARGETS_BIN += $(tgts)

INTERMEDIATES += $(deps) $(objs)

INSTALL_DEPS += install-bin

# Rules for this directory
$(tgts): LDLIBRARIES_TGT := -Llibtuna -ltuna
$(tgts): $(SRCDIR)/$(d)/rules.mk libtuna/libtuna.so

$(objs): INCLUDE_TGT := -I$(SRCDIR)/$(d)

$(d)/tuna: $(OBJS_tuna)

.PHONY: install-bin
install-bin: $(tgts)
	@echo INSTALL $^
	$(Q)$(INSTALL) -m 0755 -d $(DESTDIR)$(bindir)
	$(Q)$(INSTALL) -m 0755 $^ $(DESTDIR)$(bindir)

# Include dependencies
-include $(deps)

# Make everything depend on this rules file
$(objs): $(SRCDIR)/$(d)/rules.mk
