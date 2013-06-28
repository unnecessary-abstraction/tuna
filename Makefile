################################################################################
#	Makefile for TUNA.
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

# The build system for this project was written after reading "Recursive Make
# Considered Harmful" by Peter Miller [1] and follows the implementation notes
# linked to on that page [2], with a few tweaks of my own.
#
# [1]: http://miller.emu.id.au/pmiller/books/rmch/
# [2]: http://evbergen.home.xs4all.nl/nonrecursive-make.html

# Include `config.mk`, then `devconfig.mk` if present.
# Therefore, settings in `devconfig.mk` override those in `config.mk`.
include config.mk
-include devconfig.mk

# Support for high or low verbosity
ifeq ($(VERBOSITY),0)
  Q := @
else
  Q :=
endif

# Include top-level rules
include $(SRCDIR)/rules.mk
