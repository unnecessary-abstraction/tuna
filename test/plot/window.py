#! /usr/bin/env python
################################################################################
#   window.py: Plot graphs for functions in <tuna/window.h>
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

plt.rc('font', family='serif', serif='Palatino')
plt.rc('text', usetex='True')
plt.rc('figure', autolayout='True')

def plot_window_sine():
    length = 1000
    w = np.empty([length,], dtype=np.float32)
    r = libtuna.window_init_sine(w)

    fig, ax = plt.subplots(figsize=(8,6), dpi=1200)
    ax.plot(w)
    ax.set_title("Sine window with 1000 samples")
    ax.set_xlabel("Sample number")
    ax.set_ylabel("Window coefficient")
    ax.set_xlim(0, 1000)
    ax.set_ylim(0, 2.1)
    fig.savefig("test/plot/window.pdf")

plot_window_sine()
