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
objs := $(d)/analysis.lo \
	$(d)/buffer.lo \
	$(d)/bufhold.lo \
	$(d)/bufq.lo \
	$(d)/cbuf.lo \
	$(d)/consumer.lo \
	$(d)/counter.lo \
	$(d)/csv.lo \
	$(d)/dat.lo \
	$(d)/env_estimate.lo \
	$(d)/fft.lo \
	$(d)/input_alsa.lo \
	$(d)/input_sndfile.lo \
	$(d)/input_zero.lo \
	$(d)/log.lo \
	$(d)/onset_threshold.lo \
	$(d)/offset_threshold.lo \
	$(d)/output_null.lo \
	$(d)/output_sndfile.lo \
	$(d)/producer.lo \
	$(d)/pulse.lo \
	$(d)/time_slice.lo \
	$(d)/timespec.lo \
	$(d)/tol.lo \
	$(d)/window.lo

# Only include ADS1672 input module if it was enabled by 'configure'
ifdef enable-ads1672
objs += $(d)/input_ads1672.lo
endif

deps := $(objs:%.lo=%.d)

tgts := $(d)/$(lib_name) $(d)/$(lib_soname) $(d)/$(lib_fullname) $(d)/libtuna.a

TARGETS_LIB += $(tgts)

INTERMEDIATES += $(deps) $(objs)

INSTALL_DEPS += install-libtuna

# Rules for this directory
$(d)/$(lib_fullname) $(d)/libtuna.a: $(SRCDIR)/$(d)/rules.mk $(objs)
$(tgts): LDLIBRARIES_TGT :=
$(d)/$(lib_fullname): LDFLAGS_TGT := -Wl,-soname,$(lib_soname)
$(d)/$(lib_soname): $(d)/$(lib_fullname)
	@echo LN $@
	$(Q)ln -sf `basename $<` $@
$(d)/$(lib_name): $(d)/$(lib_soname)
	@echo LN $@
	$(Q)ln -sf `basename $<` $@

# The generic linker rule doesn't work here as the library ends in a version
# number rather than just '.so'
$(d)/$(lib_fullname):
	@echo CCLD $@
	$(Q)$(CCLD) $(LDFLAGS) $(LDFLAGS_ALL) $(LDFLAGS_TGT) -shared -o $@ $(filter %.lo,$^) $(LDLIBRARIES_TGT) $(LDLIBRARIES_ALL) $(LDLIBRARIES)


$(objs): INCLUDE_TGT := -I$(SRCDIR)/$(d)

.PHONY: install-libtuna
install-libtuna: $(tgts)
	@echo INSTALL $^
	$(Q)$(INSTALL) -m 0755 -d $(DESTDIR)$(libdir)
	$(Q)$(INSTALL) -m 0644 $^ $(DESTDIR)$(libdir)

# Include dependencies
-include $(deps)

# Make everything depend on this rules file
$(objs): $(SRCDIR)/$(d)/rules.mk
