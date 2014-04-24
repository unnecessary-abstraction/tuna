#! /usr/bin/env python
################################################################################
#   fft-sine.py: Plot graph of fft of a pure sine wave
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
import matplotlib.pyplot as plt
import numpy as np
import math
import tuna_plot

tuna_plot.init()

sample_rate = 8000
length = sample_rate

fft = libtuna.fft_init(length)
data = libtuna.fft_get_data(fft)

for i in range(length):
    data[i] = math.sin(math.pi * i / 4)

libtuna.fft_transform(fft)

fig, ax = plt.subplots(figsize=(8,6), dpi=1200)
ax.plot(data[:length//2])
ax.set_title("FFT of 1 kHz sine wave")
ax.set_xlabel("Frequency (Hz)")
ax.set_ylabel("Power Spectral Density")
fig.savefig("test/plot/fft-sine.pdf")

tuna_plot.exit()
