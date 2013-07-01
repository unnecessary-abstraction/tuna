# To allow building outside the source tree, set SRCDIR to the root of the
# source tree.
SRCDIR := .

# We can even build somewhere that isn't the current directory.
BUILDDIR := .

# To simplify cross-compilation, you can just set CROSS_COMPILE instead of
# setting the variables for each tool. Look at the next section to see how this
# is used.
CROSS_COMPILE :=

# Set the command line for each tool
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar
RM := rm -f
MV := mv
INSTALL := install
PYTHON := python2

# We have DESTDIR and PREFIX and these act like autotools.
DESTDIR :=
PREFIX := /usr/local

# If DESTDIR and PREFIX don't provide enough control, the individial target
# directories can be overridden here.
BINDIR = $(DESTDIR)$(PREFIX)/bin
LIBDIR = $(DESTDIR)$(PREFIX)/lib
INCLUDEDIR = $(DESTDIR)$(PREFIX)/include

# Common flags to use throughout the build
CFLAGS := -Wall -Wextra
LDFLAGS :=

# Flags to make CC produce dependencies
DEPFLAGS := -MD

# Additional libraries to link against, beyond those explicitly required
LDLIBRARIES :=

# Build verbosity: 0 for a quiet build, 1 to print each command line
VERBOSITY := 0
