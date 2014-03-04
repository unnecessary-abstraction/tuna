#! /usr/bin/env python
################################################################################
#   000_run.py: Initial run test for TUNA
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
import tuna

class tunaRunTests(tunaTestCase):
    def test_run(self):
        # Throw away output
        tuna.stdout = open("/dev/null")
        r = tuna.run("-V")
        self.assertEqual(r, 0)

if __name__ == '__main__':
    unittest.main(testRunner=tunaTestRunner())
