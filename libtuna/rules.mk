################################################################################
#	rules.mk for libtuna.
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
OBJS_$(d) := $(d)/buffer.o \
	$(d)/bufhold.o \
	$(d)/bufq.o \
	$(d)/csv.o \
	$(d)/fft.o \
	$(d)/input_alsa.o \
	$(d)/input_sndfile.o \
	$(d)/input_zero.o \
	$(d)/log.o \
	$(d)/output_null.o \
	$(d)/output_sndfile.o \
	$(d)/slab.o \
	$(d)/time_slice.o \
	$(d)/timespec.o \
	$(d)/tol.o \
	$(d)/window.o \

# Only include ADS1672 input module if it was enabled by 'configure'
ifdef enable-ads1672
OBJS_$(d) += $(d)/input_ads1672.o
endif

DEPS_$(d) := $(OBJS_$(d):%.o=%.d)

TGTS_$(d) := $(d)/libtuna.a

TARGETS_LIB += $(TGTS_$(d))

INTERMEDIATES += $(DEPS_$(d)) $(OBJS_$(d))

INSTALL_DEPS += install-$(d)

# Rules for this directory
$(TGTS_$(d)): $(SRCDIR)/$(d)/rules.mk $(OBJS_$(d))

$(OBJS_$(d)): CFLAGS_TGT := -I$(SRCDIR)/$(d)

.PHONY: install-$(d)
install-$(d): $(TGTS_$(d))
	@echo INSTALL $^
	$(Q)$(INSTALL) -m 0755 -d $(DESTDIR)$(libdir)
	$(Q)$(INSTALL) -m 0644 $^ $(DESTDIR)$(libdir)

# Include dependencies
-include $(DEPS_$(d))

# Make everything depend on this rules file
$(OBJS_$(d)): $(d)/rules.mk

# Pop directory stack
d := $(dirstack_$(sp))
sp := $(basename $(sp))
