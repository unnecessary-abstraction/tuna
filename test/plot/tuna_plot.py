#! /usr/bin/env python
################################################################################
#   common.py: Common functions for graph plotting
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

import libtuna
import matplotlib.pyplot as plt
import numpy as np

def init():
    plt.rc('font', family='serif', serif='Palatino')
    plt.rc('text', usetex='True')
    plt.rc('figure', autolayout='True')

def exit():
    pass
