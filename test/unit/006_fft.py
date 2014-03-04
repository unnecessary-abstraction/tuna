#! /usr/bin/env python
################################################################################
#   006_fft.py: Tests for the fft code
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

from tuna_test import *
import unittest
import libtuna
import numpy as np
import math

import tempfile
import os

class tunaFFTTests(tunaTestCase):
    def setUpLogging(self):
        h, self.log_path = tempfile.mkstemp()
        self.assertSuccess(libtuna.log_init(self.log_path, __file__))
        self.log_f = os.fdopen(h)

    def tearDownLogging(self):
        if hasattr(self, 'log_f'):
            libtuna.log_exit()
            self.log_f.close()
            os.unlink(self.log_path)

    def assertNoErrors(self):
        if hasattr(self, 'log_f'):
            libtuna.log_sync()
            self.log_f.seek(0)
            for line in self.log_f:
                self.assertNotIn('ERROR', line)
                self.assertNotIn('FATAL', line)

    def setUp(self):
        self.setUpLogging()

    def tearDown(self):
        self.tearDownLogging()

    def test_00_fft_init(self):
        sample_rate = 8000
        length = sample_rate

        # Ensure we can create and fft context
        fft = libtuna.fft_init(length)
        self.assertIsNotNone(fft)

        # Check length and data pointer
        self.assertEqual(length, libtuna.fft_get_length(fft))
        self.assertIsNotNone(libtuna.fft_get_data(fft))

        # Close down the fft context
        libtuna.fft_exit(fft)

        self.assertNoErrors()

    def test_01_fft_zeros(self):
        sample_rate = 8000
        length = sample_rate

        fft = libtuna.fft_init(length)
        self.assertIsNotNone(fft)

        # Fill the fft data buffer with zeros and transform
        data = libtuna.fft_get_data(fft)
        self.assertIsNotNone(data)

        for i in range(length):
            data[i] = 0

        self.assertSuccess(libtuna.fft_transform(fft))

        # Ensure the output data is all zeros
        for i in range(length//2):
            self.assertEqual(data[i], 0)

        self.assertNoErrors()

    def test_02_fft_dc(self):
        sample_rate = 8000
        length = sample_rate

        fft = libtuna.fft_init(length)
        self.assertIsNotNone(fft)

        # Fill the fft data buffer with zeros and transform
        data = libtuna.fft_get_data(fft)
        self.assertIsNotNone(data)

        for i in range(length):
            data[i] = 1

        self.assertSuccess(libtuna.fft_transform(fft))

        # Ensure the output data is all zeros except for the dc term
        self.assertEqual(data[0], 1)
        for i in range(1, length//2):
            self.assertEqual(data[i], 0)

        self.assertNoErrors()

    def test_03_fft_sine(self):
        sample_rate = 8000
        length = sample_rate

        fft = libtuna.fft_init(length)
        self.assertIsNotNone(fft)

        # Fill the fft data buffer with zeros and transform
        data = libtuna.fft_get_data(fft)
        self.assertIsNotNone(data)

        for i in range(length):
            data[i] = math.sin(math.pi * i / 4)

        self.assertSuccess(libtuna.fft_transform(fft))

        # Ensure the output data is all zeros except for the 1 kHz term
        for i in range(1000):
            self.assertAlmostEqual(data[i], 0, msg="data[%d]" % i)
        self.assertEqual(data[1000], 0.5, msg="data[%d]" % 1000)
        for i in range(1001, length//2):
            self.assertAlmostEqual(data[i], 0, msg="data[%d]" % i)

        self.assertNoErrors()

if __name__ == '__main__':
    unittest.main(testRunner=tunaTestRunner())
