#! /usr/bin/env python
################################################################################
#   tol-coeffs.py: Plot graph of third-octave level coefficients
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

import matplotlib
matplotlib.use('PDF')

import libtuna
import tuna_plot
import matplotlib.pyplot as plt
import numpy as np
import os
import math

sample_rate = 400000

tuna_plot.init()
tol = libtuna.tol_init(sample_rate, sample_rate//2, 0.4, 3)
if not tol:
    print("ERROR: Failed to create tol object")
    os.exit(-1)

n_tol = libtuna.tol_get_num_levels(tol)
coeff_len = int(math.floor(sample_rate / 2))

fig, ax = plt.subplots(figsize=(8,4), dpi=1200)

w = np.empty([coeff_len, ], dtype=np.float32)
for i in range(n_tol):
    r = libtuna.tol_get_coeffs(tol, i, w)
    if r < 0:
        print("ERROR: Failed to extract third octave levels")
        os.exit(-1)

    ax.plot(w)

ax.set_title("Third-octave level coefficients")
ax.set_xlabel("Frequency (Hz)")
ax.set_ylabel("Coefficient")
ax.set_xlim(1, 1000000)
ax.set_ylim(0, 1.1)
ax.set_xscale('log')
fig.savefig("test/plot/tol-coeffs.pdf")

libtuna.tol_exit(tol)
tuna_plot.exit()
