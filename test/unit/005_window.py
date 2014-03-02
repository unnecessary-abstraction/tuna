#! /usr/bin/env python
################################################################################
#   005_window.py: Tests for the windowing code
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

import tempfile
import os

class tunaWindowTests(tunaTestCase):
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

    def test_window_sine(self):
        length = 512
        w = np.empty([length,], dtype=np.float32)
        libtuna.window_init_sine(w)

        # First and last values could be zero, the rest shouldn't be
        self.assertGreaterEqual(w[0], 0)
        w_sum = w[0] * w[0]
        for i in range(1, length-1):
            self.assertGreater(w[i], 0)
            w_sum += w[i] * w[i]
        self.assertGreaterEqual(w[length-1], 0)
        w_sum += w[length-1] * w[length-1]

        # The sum of all coefficients should be approximately equal to the
        # length of the coefficient array (as the values are un-normalised)
        self.assertAlmostEqual(w_sum, length, places=3)

        self.assertNoErrors()

if __name__ == '__main__':
    unittest.main(testRunner=tunaTestRunner())
