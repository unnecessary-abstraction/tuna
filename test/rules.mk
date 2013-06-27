################################################################################
#	rules.mk for TUNA tests.
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

# Push directory stack
sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

# Targets and intermediates in this directory
OBJS_output_csv_test := $(d)/output_csv_test.o

OBJS_$(d) := $(OBJS_output_csv_test)

DEPS_$(d) := $(OBJS_$(d):%.o=%.d)

TGTS_$(d) := $(d)/output_csv_test

TARGETS_TEST += $(TGTS_$(d))

INTERMEDIATES += $(DEPS_$(d)) $(OBJS_$(d))

# Rules for this directory
$(TGTS_$(d)): LDLIBRARIES_TGT := libtuna/libtuna.a
$(TGTS_$(d)): $(SRCDIR)/$(d)/rules.mk libtuna/libtuna.a

$(d)/output_csv_test: $(OBJS_output_csv_test)

$(OBJS_$(d)): CFLAGS_TGT := -I$(SRCDIR)/$(d)

# Include dependencies
-include $(DEPS_$(d))

# Pop directory stack
d := $(dirstack_$(sp))
sp := $(basename $(sp))
