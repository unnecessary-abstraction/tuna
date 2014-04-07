################################################################################
#	rules.mk for tuna headers.
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

d := include

# Targets and rules in this directory
hdrs := $(wildcard $(SRCDIR)/$(d)/tuna/*.h)
inl_hdrs := $(wildcard $(SRCDIR)/$(d)/tuna_inl/*.inl)

TUNA_HEADERS := $(hdrs) $(inl_hdrs)

INSTALL_DEPS += install-hdrs install-inl-hdrs

.PHONY: install-hdrs install-inl-hdrs
install-hdrs: $(hdrs)
	@echo INSTALL $^
	$(Q)$(INSTALL) -m 0755 -d $(DESTDIR)$(includedir)/tuna
	$(Q)$(INSTALL) -m 0644 $^ $(DESTDIR)$(includedir)/tuna

install-inl-hdrs: $(inl_hdrs)
	@echo INSTALL $^
	$(Q)$(INSTALL) -m 0755 -d $(DESTDIR)$(includedir)/tuna_inl
	$(Q)$(INSTALL) -m 0644 $^ $(DESTDIR)$(includedir)/tuna_inl
