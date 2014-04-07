################################################################################
#	rules.mk for TUNA unit tests.
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

d := test/unit

tests := $(d)/000_import.py \
	$(d)/001_log.py \
	$(d)/002_buffer.py \
	$(d)/003_bufhold.py \
	$(d)/004_tol.py \
	$(d)/005_window.py \
	$(d)/006_fft.py \
	$(d)/007_env_estimate.py \
	$(d)/008_onset_threshold.py

run_tests := $(tests:$(d)/%.py=run-u%.py)

run-u%.py: $(d)/%.py libtuna/libtuna.so swig/python/libtuna.py swig/python/_libtuna.so
	$(Q)$(PYTHON) $<

# Set paths when running python scripts
run-u%.py: export PYTHONPATH := swig/python:test
run-u%.py: export LD_LIBRARY_PATH := libtuna

CHECK_DEPS += $(run_tests)
