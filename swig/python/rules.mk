################################################################################
#	rules.mk for libtuna python bindings.
#
#	Copyright (C) 2014 Paul Barker, Loughborough University
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

# Push directory stack
sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

OBJS_$(d) := $(d)/libtuna_wrap.lo

DEPS_$(d) := $(OBJS_$(d):%.lo=%.d) $(d)/libtuna.d

TGTS_$(d) := $(d)/_libtuna.so $(d)/libtuna.py

TARGETS_LIB += $(TGTS_$(d))

INTERMEDIATES += $(OBJS_$(d)) $(DEPS_$(d)) $(d)/libtuna_wrap.c

INSTALL_DEPS += install-$(d)

# Rules for this directory
$(TGTS_$(d)): $(SRCDIR)/$(d)/rules.mk libtuna/libtuna.so $(OBJS_$(d))
$(TGTS_$(d)): LDLIBRARIES_TGT := -Llibtuna -ltuna

$(OBJS_$(d)): INCLUDE_TGT := -I$(SRCDIR)/$(d)

.PHONY: install-$(d)
install-$(d): $(TGTS_$(d))
	@echo INSTALL $^
	$(Q)$(INSTALL) -m 0755 -d $(DESTDIR)$(python_sitedir)
	$(Q)$(INSTALL) -m 0644 $^ $(DESTDIR)$(python_sitedir)

$(d)/libtuna_wrap.c: $(SWIGFILE)
	@echo SWIG $<
	$(Q)$(SWIG) -python -py3 -O -MD -MP -MF swig/python/libtuna.d -o $@ $(INCLUDE_ALL) $(INCLUDE_TGT) $<

# Include dependencies
-include $(DEPS_$(d))

# Make everything depend on this rules file
$(OBJS_$(d)) $(d)/libtuna_wrap.c: $(SRCDIR)/$(d)/rules.mk

# Pop directory stack
d := $(dirstack_$(sp))
sp := $(basename $(sp))
