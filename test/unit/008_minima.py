#! /usr/bin/env python
################################################################################
#   008_minima.py: Tests for the minima tracking
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
        windowlen = 10
        m = libtuna.minima_init(windowlen)
        self.assertIsNotNone(m)

        libtuna.minima_exit(m)
        self.assertNoErrors()

    def test_01_zeros(self):
        windowlen = 10
        siglen = 20

        m = libtuna.minima_init(windowlen)
        self.assertIsNotNone(m)

        for i in range(siglen):
            v = libtuna.minima_next(m, 0)
            self.assertEqual(v, 0)

        libtuna.minima_exit(m)
        self.assertNoErrors()

    def test_02_ramp_up(self):
        windowlen = 10
        siglen = 20

        signal = range(siglen)
        expected_output = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
                9, 10)
        expected_ages = (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9,
                9, 9)

        m = libtuna.minima_init(windowlen)
        self.assertIsNotNone(m)

        for i in range(siglen):
            v = libtuna.minima_next(m, signal[i])
            self.assertEqual(v, expected_output[i])

            v = libtuna.minima_current(m)
            self.assertEqual(v, expected_output[i])

            age = libtuna.minima_current_age(m)
            self.assertEqual(age, expected_ages[i])

        libtuna.minima_exit(m)
        self.assertNoErrors()

    def test_03_ramp_down(self):
        windowlen = 10
        siglen = 20

        signal = range(siglen, 0, -1)
        expected_output = (20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7,
                6, 5, 4, 3, 2, 1)
        expected_ages = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0)

        m = libtuna.minima_init(windowlen)
        self.assertIsNotNone(m)

        for i in range(siglen):
            v = libtuna.minima_next(m, signal[i])
            self.assertEqual(v, expected_output[i])

            v = libtuna.minima_current(m)
            self.assertEqual(v, expected_output[i])

            age = libtuna.minima_current_age(m)
            self.assertEqual(age, expected_ages[i])

        libtuna.minima_exit(m)
        self.assertNoErrors()

if __name__ == '__main__':
    unittest.main(testRunner=tunaTestRunner())
