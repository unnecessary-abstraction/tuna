#! /usr/bin/env python
################################################################################
#   tuna.py: Wrapper for running main TUNA program from Python scripts
#
#   Copyright (C) 2014 Paul Barker
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2, or (at your option) any
#   later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
################################################################################

import subprocess
import sys

exe_path = "bin/tuna"
stdin = sys.stdin
stdout = sys.stdout
stderr = sys.stderr

def run(args):
    cmd = "%s %s" % (exe_path, args)
    r = subprocess.call(cmd, shell=True, stdin=stdin, stdout=stdout,
            stderr=stderr)
    return r
