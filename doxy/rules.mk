################################################################################
#	rules.mk for doxygen documentation.
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

d := doxy

DOXYFILE := $(d)/doxygen.conf

tgts := $(d)/html $(d)/libtuna.pdf

TARGETS_DOCS += $(tgts)

INTERMEDIATES += $(d)/doxygen.log $(d)/latex.log $(d)/latex

# Rules for this directory
#
# Use doxygen.log as a marker of whether doxygen has been executed. Also pick up
# $(TUNA_HEADERS) as a dependency list, defined in include/rules.mk
$(d)/doxygen.log: $(DOXYFILE) $(TUNA_HEADERS)
	@echo DOXYGEN $<
	@doxygen $< > $@

$(d)/libtuna.pdf: $(d)/doxygen.log
	@echo MAKE doxy/latex
	@$(MAKE) -C doxy/latex > doxy/latex.log 2>&1
	@cp doxy/latex/refman.pdf $@

.PHONY: install-docs
install-docs: $(tgts)
	@echo INSTALL $^
	$(Q)$(INSTALL) -m 0755 -d $(DESTDIR)$(docdir)/tuna
	$(Q)$(INSTALL) -m 0644 doxy/libtuna.pdf $(DESTDIR)$(docdir)/tuna
	$(Q)cp -dpr --no-preserve=ownership doxy/html $(DESTDIR)$(docdir)/tuna/html

INSTALL_DEPS += install-docs

# Make everything depend on this rules file
$(tgts): $(SRCDIR)/$(d)/rules.mk
