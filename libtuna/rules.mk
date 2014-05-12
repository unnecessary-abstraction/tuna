################################################################################
#	rules.mk for libtuna.
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

d := libtuna

# Library version info
lib_name := libtuna.so
lib_major := 0
lib_minor := 1
lib_revision := 0
lib_version := $(lib_major).$(lib_minor).$(lib_revision)
lib_soname := $(lib_name).$(lib_major)
lib_fullname := $(lib_name).$(lib_version)

# Targets and intermediates in this directory
srcs := $(d)/analysis.c \
	$(d)/buffer.c \
	$(d)/bufhold.c \
	$(d)/bufq.c \
	$(d)/cbuf.c \
	$(d)/consumer.c \
	$(d)/counter.c \
	$(d)/csv.c \
	$(d)/dat.c \
	$(d)/env_estimate.c \
	$(d)/fft.c \
	$(d)/input_alsa.c \
	$(d)/input_sndfile.c \
	$(d)/input_zero.c \
	$(d)/log.c \
	$(d)/onset_threshold.c \
	$(d)/offset_threshold.c \
	$(d)/output_null.c \
	$(d)/output_sndfile.c \
	$(d)/producer.c \
	$(d)/pulse.c \
	$(d)/time_slice.c \
	$(d)/timespec.c \
	$(d)/tol.c \
	$(d)/window.c

# Only include ADS1672 input module if it was enabled by 'configure'
ifdef enable-ads1672
srcs += $(d)/input_ads1672.c
endif

objs_shared := $(srcs:%.c=%.lo)
objs_static := $(srcs:%.c=%.o)
objs_all := $(objs_shared) $(objs_static)
deps := $(srcs:%.c=%.d)

tgts := $(d)/$(lib_name) $(d)/$(lib_soname) $(d)/$(lib_fullname) $(d)/libtuna.a

TARGETS_LIB += $(tgts)

INTERMEDIATES += $(deps) $(objs_all)

INSTALL_DEPS += install-libtuna

# Rules for this directory
$(d)/libtuna.a: $(SRCDIR)/$(d)/rules.mk
ifdef enable-shared
$(d)/libtuna.a: $(objs_shared)
else
$(d)/libtuna.a: $(objs_static)
endif

$(tgts): LDLIBRARIES_TGT :=

$(objs_all): INCLUDE_TGT := -I$(SRCDIR)/$(d)

.PHONY: install-libtuna
install-libtuna: $(d)/libtuna.a
	@echo INSTALL $^
	$(Q)$(INSTALL) -m 0755 -d $(DESTDIR)$(libdir)
	$(Q)$(INSTALL) -m 0644 $^ $(DESTDIR)$(libdir)

# Include dependencies
-include $(deps)

# Make everything depend on this rules file
$(objs_all): $(SRCDIR)/$(d)/rules.mk

################################################################################
# Shared library support
ifdef enable-shared
$(d)/$(lib_fullname): $(SRCDIR)/$(d)/rules.mk $(objs_shared)
$(d)/$(lib_fullname): LDFLAGS_TGT := -Wl,-soname,$(lib_soname)
$(d)/$(lib_soname): $(d)/$(lib_fullname)
	@echo LN $@
	$(Q)ln -sf $(lib_fullname) $@
$(d)/$(lib_name): $(d)/$(lib_fullname)
	@echo LN $@
	$(Q)ln -sf $(lib_fullname) $@

# The generic linker rule doesn't work here as the library ends in a version
# number rather than just '.so'
$(d)/$(lib_fullname):
	@echo CCLD $@
	$(Q)$(CCLD) $(LDFLAGS) $(LDFLAGS_ALL) $(LDFLAGS_TGT) -shared -o $@ $(filter %.lo,$^) $(LDLIBRARIES_TGT) $(LDLIBRARIES_ALL) $(LDLIBRARIES)

.PHONY: install-libtuna-shared
install-libtuna-shared: $(d)/$(lib_fullname)
	@echo INSTALL $^
	$(Q)$(INSTALL) -m 0755 -d $(DESTDIR)$(libdir)
	$(Q)$(INSTALL) -m 0644 $^ $(DESTDIR)$(libdir)
	$(Q)ln -sf $(lib_fullname) $(DESTDIR)$(libdir)/$(lib_soname)
	$(Q)ln -sf $(lib_fullname) $(DESTDIR)$(libdir)/$(lib_name)
INSTALL_DEPS += install-libtuna-shared
endif
