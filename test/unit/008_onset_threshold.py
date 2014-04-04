#! /usr/bin/env python
################################################################################
#   008_onset_threshold.py: Tests for the onset_threshold tracking
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

class tunaMinimaTests(tunaTestCase):
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

    def test_00_init(self):
        Tw = 0.1
        sample_rate = 100
        ratio = 2
        onset = libtuna.onset_threshold_init(Tw, sample_rate, ratio)
        self.assertIsNotNone(onset)

        libtuna.onset_threshold_exit(onset)
        self.assertNoErrors()

    def test_01_zeros(self):
        Tw = 0.1
        sample_rate = 100
        ratio = 2
        siglen = 20

        onset = libtuna.onset_threshold_init(Tw, sample_rate, ratio)
        self.assertIsNotNone(onset)

        threshold = 0
        for i in range(siglen):
            threshold = libtuna.onset_threshold_next(onset, 0, threshold)
            self.assertEqual(threshold, 0)

        libtuna.onset_threshold_exit(onset)
        self.assertNoErrors()

    def test_02_ramp_up(self):
        Tw = 0.1
        sample_rate = 100
        ratio = 2
        siglen = 20

        signal = range(siglen)
        expected_minima = np.array((0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4,
            5, 6, 7, 8, 9, 10))
        expected_ages = np.array((0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 9, 9, 9, 9,
            9, 9, 9, 9, 9))
        expected_thresholds = ratio * expected_minima

        onset = libtuna.onset_threshold_init(Tw, sample_rate, ratio)
        self.assertIsNotNone(onset)

        threshold = 0
        for i in range(siglen):
            threshold = libtuna.onset_threshold_next(onset, signal[i], threshold)
            self.assertEqual(threshold, expected_thresholds[i], "next @ %d" % (i))

            v = libtuna.onset_threshold_current(onset)
            self.assertEqual(v, expected_thresholds[i], "current @ %d" % (i))

            v = libtuna.onset_threshold_current_minimum(onset)
            self.assertEqual(v, expected_minima[i], "minimum @ %d" % (i))

            age = libtuna.onset_threshold_age(onset)
            self.assertEqual(age, expected_ages[i], "age @ %d" % (i))

        libtuna.onset_threshold_exit(onset)
        self.assertNoErrors()

    def test_03_ramp_down(self):
        Tw = 0.1
        sample_rate = 100
        ratio = 2
        siglen = 20

        signal = range(siglen, 0, -1)
        expected_minima = np.array((20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10,
            9, 8, 7, 6, 5, 4, 3, 2, 1))
        expected_ages = np.array((0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0))
        expected_thresholds = ratio * expected_minima

        onset = libtuna.onset_threshold_init(Tw, sample_rate, ratio)
        self.assertIsNotNone(onset)

        threshold = 0
        for i in range(siglen):
            threshold = libtuna.onset_threshold_next(onset, signal[i], threshold)
            self.assertEqual(threshold, expected_thresholds[i], "next @ %d" % (i))

            v = libtuna.onset_threshold_current(onset)
            self.assertEqual(v, expected_thresholds[i], "current @ %d" % (i))

            v = libtuna.onset_threshold_current_minimum(onset)
            self.assertEqual(v, expected_minima[i], "minimum @ %d" % (i))

            age = libtuna.onset_threshold_age(onset)
            self.assertEqual(age, expected_ages[i], "age @ %d" % (i))

        libtuna.onset_threshold_exit(onset)
        self.assertNoErrors()

if __name__ == '__main__':
    unittest.main(testRunner=tunaTestRunner())
