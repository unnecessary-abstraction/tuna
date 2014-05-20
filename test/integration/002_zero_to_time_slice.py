#! /usr/bin/env python
################################################################################
#   002_zero_to_time_slice.py: Test zero and time_slice modules
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

import csv

class tunaZeroTimeSliceTests(tunaTestCase):
    def test_00_run(self):
        # Process 64k samples from input_zero to time_slice
        r = tuna.run("-i zero -o time_slice:/dev/null -c 64000")
        self.assertEqual(r, 0)

    def test_01_400kHz(self):
        filename = "results-tunaZeroTimeSliceTests-test_01_400kHz.csv"
        # Process 2 s of data at a sampling rate of 400 kHz from input_zero to
        # time_slice
        r = tuna.run("-i zero -o time_slice:%s -c 800000 -r 400000" % filename)
        self.assertEqual(r, 0)

        # Open CSV file
        f = open(filename, 'r', newline='')
        self.assertIsNotNone(f)
        csvf = csv.reader(f, delimiter=',')

        # Line 1 should contain the start record
        line1 = next(csvf)
        self.assertEqual(len(line1), 1)
        self.assertTrue(line1[0].startswith("START"))

        # Lines 2-4 should contain 49 zeros each
        count = 0
        for line in csvf:
            count += 1
            self.assertEqual(len(line), 50)
            # Trim off final empty element
            line = line[:49]
            for elem in line:
                self.assertEqual(float(elem), 0)

        # We should see 5 time slices
        self.assertEqual(count, 5)

        # Close CSV file
        f.close()

    def test_02_8kHz(self):
        filename = "results-tunaZeroTimeSliceTests-test_01_8kHz.csv"
        # Process 15 s of data at a sampling rate of 8 kHz from input_zero to
        # time_slice
        r = tuna.run("-i zero -o time_slice:%s -c 120000 -r 8000" % filename)
        self.assertEqual(r, 0)

        # Open CSV file
        f = open(filename, 'r', newline='')
        self.assertIsNotNone(f)
        csvf = csv.reader(f, delimiter=',')

        # Line 1 should contain the start record
        line1 = next(csvf)
        self.assertEqual(len(line1), 1)
        self.assertTrue(line1[0].startswith("START"))

        # Lines 2-4 should contain 31 zeros each
        count = 0
        for line in csvf:
            count += 1
            self.assertEqual(len(line), 32)
            # Trim off final empty element
            line = line[:31]
            for elem in line:
                self.assertEqual(float(elem), 0)

        # We should see 57 time slices
        self.assertEqual(count, 57)

        # Close CSV file
        f.close()

if __name__ == '__main__':
    unittest.main(testRunner=tunaTestRunner())
