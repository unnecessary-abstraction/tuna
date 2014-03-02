################################################################################
#	rules.mk for TUNA test graph plots.
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

PLOTS_$(d) := $(d)/window.pdf \
	$(d)/tol-coeffs.pdf \
	$(d)/tol-white-noise.pdf \
	$(d)/tol-pink-noise.pdf

$(d)/%.pdf: $(d)/%.py libtuna/libtuna.so swig/python/_libtuna.so swig/python/libtuna.py
	@echo PLOT $@
	$(Q)$(PYTHON) $<

# Set paths when running python scripts
$(d)/%.pdf: export PYTHONPATH := swig/python
$(d)/%.pdf: export LD_LIBRARY_PATH := libtuna

# There's probably nowhere better to define this
.PHONY: plots
plots: $(PLOTS_$(d))

# Ensure plots are removed during 'make clean'
TARGETS_EXTRA += $(PLOTS_$(d))

# Pop directory stack
d := $(dirstack_$(sp))
sp := $(basename $(sp))
