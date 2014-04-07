#! /usr/bin/env python
################################################################################
#   004_tol.py: Tests for third octave analysis code
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
from math import floor

import tempfile
import os

class tunaTOLTests(tunaTestCase):
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

    def test_tol_00_init(self):
        sample_rate = 400000
        analysis_length = sample_rate//2

        tol = libtuna.tol_init(sample_rate, analysis_length, 0.4, 3)
        self.assertIsNotNone(tol)

        libtuna.tol_exit(tol)
        self.assertNoErrors()

    def test_tol_01_coefficient_values(self):
        sample_rate = 400000
        analysis_length = sample_rate//2

        tol = libtuna.tol_init(sample_rate, analysis_length, 0.4, 3)
        self.assertIsNotNone(tol)

        n_tol = libtuna.tol_get_num_levels(tol)
        self.assertEqual(n_tol, libtuna.MAX_THIRD_OCTAVE_LEVELS)

        for i in range(n_tol):
            w = np.empty([analysis_length,], dtype=np.float32)
            self.assertSuccess(libtuna.tol_get_coeffs(tol, i, w))

            # All coeffs must be between 0 and 1 inclusive.
            self.assertGreaterEqual(min(w), 0)
            self.assertLessEqual(max(w), 1)

        libtuna.tol_exit(tol)
        self.assertNoErrors()

    def test_tol_02_coefficient_sum(self):
        sample_rate = 400000
        analysis_length = sample_rate//2

        tol = libtuna.tol_init(sample_rate, analysis_length, 0.4, 3)
        self.assertIsNotNone(tol)

        n_tol = libtuna.tol_get_num_levels(tol)
        self.assertEqual(n_tol, libtuna.MAX_THIRD_OCTAVE_LEVELS)

        w_sum = np.zeros([analysis_length,], dtype=np.float32)
        for i in range(n_tol):
            w = np.empty([analysis_length,], dtype=np.float32)
            self.assertSuccess(libtuna.tol_get_coeffs(tol, i, w))

            w_sum += w

        # We can only compare upto the midpoint of the final third octave level
        for i in range(160000):
            self.assertAlmostEqual(w_sum[i], 1.0, places=6)

        libtuna.tol_exit(tol)
        self.assertNoErrors()

    def test_tol_03_single_band_bleed(self):
        # Place a single sample at the centre of a third-octave band and ensure
        # that the third octave level analysis shows a value in the correct band
        sample_rate = 400000
        analysis_length = sample_rate//2

        tol = libtuna.tol_init(sample_rate, analysis_length, 0.4, 3)
        self.assertIsNotNone(tol)

        n_tol = libtuna.tol_get_num_levels(tol)
        self.assertEqual(n_tol, libtuna.MAX_THIRD_OCTAVE_LEVELS)

        w = np.zeros([analysis_length,], dtype=np.float32)
        for i in range(n_tol):
            results = np.zeros([n_tol + 1,], dtype=np.float32)
            index = round(libtuna.tol_get_band_centre(i))
            w[index] = 1

            libtuna.tol_calculate(tol, w, results)
            for j in range(n_tol):
                if i == j:
                    self.assertEqual(results[j], 1, msg="at band %d, value in band %d" % (j, i))
                # Allow some slip for bands with non-integer centre frequencies
                elif ((libtuna.tol_get_band_centre(i) % 1) != 0) and (abs(i - j) == 1):
                    pass
                else:
                    self.assertEqual(results[j], 0, msg="at band %d, value in band %d" % (j, i))

            w[index] = 0

        libtuna.tol_exit(tol)
        self.assertNoErrors()

    def test_tol_04_mid_band_bleed(self):
        # Place a single sample at the midpoint between adjacent third-octave
        # band centres and ensure that the third octave level analysis shows a
        # value in each of the adjacent bands only.
        sample_rate = 400000
        analysis_length = sample_rate//2

        tol = libtuna.tol_init(sample_rate, analysis_length, 0.4, 3)
        self.assertIsNotNone(tol)

        n_tol = libtuna.tol_get_num_levels(tol)
        self.assertEqual(n_tol, libtuna.MAX_THIRD_OCTAVE_LEVELS)

        w = np.zeros([analysis_length,], dtype=np.float32)
        for i in range(n_tol - 1):
            results = np.zeros([n_tol + 1,], dtype=np.float32)
            index = round(libtuna.tol_get_band_edge(i))
            w[index] = 1

            libtuna.tol_calculate(tol, w, results)
            for j in range(n_tol):
                # We can only really check that values are non-zero in each of
                # the adjacent bands, we shouldn't assume the value will be
                # around 0.5 in each band
                if (i == j) or (i + 1 == j):
                    self.assertNotEqual(results[j], 0, msg="at band %d, value in band %d" % (j, i))
                else:
                    self.assertEqual(results[j], 0, msg="at band %d, value in band %d" % (j, i))

            # Ensure the sum of the adjacent third-octave levels is around 1
            r_sum = results[i] + results[i + 1]
            self.assertAlmostEqual(r_sum, 1.0, places=6)

            w[index] = 0

        libtuna.tol_exit(tol)
        self.assertNoErrors()

if __name__ == '__main__':
    unittest.main(testRunner=tunaTestRunner())
