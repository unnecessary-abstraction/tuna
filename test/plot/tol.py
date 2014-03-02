#! /usr/bin/env python
################################################################################
#   tol.py: Plot graphs for functions in <tuna/tol.h>
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
import os
import math

sample_rate = 400000

tol = libtuna.tol_init(sample_rate, sample_rate, 0.4, 3)
if not tol:
    print("ERROR: Failed to create tol object")
    os.exit(-1)

n_tol = libtuna.tol_get_num_levels(tol)
coeff_len = int(math.floor(sample_rate / 2))

plt.rc('font', family='serif', serif='Palatino')
plt.rc('text', usetex='True')
plt.rc('figure', autolayout='True')

def plot_coeffs():
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

def plot_white_noise():
    band_centres = np.empty([libtuna.MAX_THIRD_OCTAVE_LEVELS, ], dtype=np.float32)
    for i in range(libtuna.MAX_THIRD_OCTAVE_LEVELS):
        band_centres[i] = libtuna.tol_get_band_centre(i)

    fig, ax = plt.subplots(figsize=(8,4), dpi=1200)

    # A flat spectrum should give linearly increasing third octave levels
    w = np.ones([coeff_len, ], dtype=np.float32)
    results = np.zeros([n_tol + 1,], dtype=np.float32)
    libtuna.tol_calculate(tol, w, results)

    ax.scatter(band_centres, results[:-1])
    ax.set_title("Third-octave level analysis of white noise")
    ax.set_xlabel("Frequency (Hz)")
    ax.set_ylabel("Third-octave level")
    ax.set_xlim(0, 200000)
    ax.set_ylim(0, 40000)
    fig.savefig("test/plot/tol-white-noise.pdf")

def plot_pink_noise():
    band_centres = np.empty([libtuna.MAX_THIRD_OCTAVE_LEVELS, ], dtype=np.float32)
    for i in range(libtuna.MAX_THIRD_OCTAVE_LEVELS):
        band_centres[i] = libtuna.tol_get_band_centre(i)

    fig, ax = plt.subplots(figsize=(8,4), dpi=1200)

    # A 1/f spectrum should give flat third octave levels
    w = np.empty([coeff_len, ], dtype=np.float32)
    w[0:9] = 0
    for i in range(9, len(w)):
        w[i] = 1/i

    results = np.zeros([n_tol + 1,], dtype=np.float32)
    libtuna.tol_calculate(tol, w, results)

    ax.scatter(band_centres, results[:-1])
    ax.plot(w)
    ax.set_title("Third-octave level analysis of white noise")
    ax.set_xlabel("Frequency (Hz)")
    ax.set_ylabel("Third-octave level")
    ax.set_xlim(1, 200000)
    #ax.set_ylim(0, 40000)
    ax.set_xscale('log')
    ax.set_yscale('log')
    fig.savefig("test/plot/tol-pink-noise.pdf")

plot_coeffs()
plot_white_noise()
plot_pink_noise()

libtuna.tol_exit(tol)
