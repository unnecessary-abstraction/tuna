#! /usr/bin/env python
################################################################################
#   001_zero_to_null.py: Test zero and null modules
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

class tunaZeroNullTests(tunaTestCase):
    def test_run(self):
        # Throw away output
        #tuna.stdout = open("/dev/null")

        # Process 64k samples from input_zero to output_null
        r = tuna.run("-i zero -o null -c 64000")

        self.assertEqual(r, 0)

if __name__ == '__main__':
    unittest.main(testRunner=tunaTestRunner())
