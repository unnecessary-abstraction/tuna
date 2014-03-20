#! /usr/bin/env python
################################################################################
#   007_env_estimate.py: Tests for the envelope estimation code
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

class tunaEnvEstimateTests(tunaTestCase):
    Tc = 0.1
    sample_rate = 48000

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
        env = libtuna.env_estimate_init(self.Tc, self.sample_rate)
        self.assertIsNotNone(env)

        libtuna.env_estimate_exit(env)
        self.assertNoErrors()

    def test_01_zeros(self):
        env = libtuna.env_estimate_init(self.Tc, self.sample_rate)
        self.assertIsNotNone(env)

        # Put ten zeros into the envelope estimate and ensure we always get zero
        # out
        for i in range(10):
            x_in = 0
            x_out = libtuna.env_estimate_next(env, x_in)
            self.assertEqual(x_out, 0)

        libtuna.env_estimate_exit(env)
        self.assertNoErrors()

    def test_02_ramp_up(self):
        env = libtuna.env_estimate_init(self.Tc, self.sample_rate)
        self.assertIsNotNone(env)

        # Input a ramp up function and ensure that the output is equal to the
        # sample energy (input squared) for each sample
        for i in range(10):
            x_in = i
            x_out = libtuna.env_estimate_next(env, x_in)
            self.assertEqual(x_out, x_in)

        libtuna.env_estimate_exit(env)
        self.assertNoErrors()

    def test_03_ramp_down(self):
        env = libtuna.env_estimate_init(self.Tc, self.sample_rate)
        self.assertIsNotNone(env)

        # Input a ramp down function and ensure that the output is equal to or
        # greater than the sample energy for each sample (as the envelope decay
        # may be slower than the ramp down decay)
        for i in range(10):
            x_in = 10 - i
            x_out = libtuna.env_estimate_next(env, x_in)
            self.assertGreaterEqual(x_out, x_in)

        libtuna.env_estimate_exit(env)
        self.assertNoErrors()

if __name__ == '__main__':
    unittest.main(testRunner=tunaTestRunner())
