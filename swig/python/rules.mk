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

d := swig/python

objs := $(d)/libtuna_wrap.lo

deps := $(objs:%.lo=%.d) $(d)/libtuna.d

tgts := $(d)/_libtuna.so $(d)/libtuna.py

TARGETS_LIB += $(tgts)

INTERMEDIATES += $(objs) $(deps) $(d)/libtuna_wrap.c

INSTALL_DEPS += install-python-bindings

# Rules for this directory
$(tgts): $(SRCDIR)/$(d)/rules.mk libtuna/libtuna.so $(objs)
$(tgts): LDLIBRARIES_TGT := -Llibtuna -ltuna

$(objs): INCLUDE_TGT := -I$(SRCDIR)/$(d)

.PHONY: install-python-bindings
install-python-bindings: $(tgts)
	@echo INSTALL $^
	$(Q)$(INSTALL) -m 0755 -d $(DESTDIR)$(python_sitedir)
	$(Q)$(INSTALL) -m 0644 $^ $(DESTDIR)$(python_sitedir)

$(d)/libtuna_wrap.c: $(d)/libtuna.i
	@echo SWIG $<
	$(Q)$(SWIG) -python -py3 -O -MD -MP -MF swig/python/libtuna.d -I$(SRCDIR) -o $@ $(INCLUDE_ALL) $(INCLUDE_TGT) $<

# Include dependencies
-include $(deps)

# Make everything depend on this rules file
$(objs) $(d)/libtuna_wrap.c: $(SRCDIR)/$(d)/rules.mk
