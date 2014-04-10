#! /usr/bin/env python
################################################################################
#   001_log.py: Tests for logging code
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

import tempfile
import os

class tunaLogTests(tunaTestCase):
    def test_log_file(self):
        (h, path) = tempfile.mkstemp()
        f = os.fdopen(h)
        self.assertSuccess(libtuna.log_init(path, __file__))
        libtuna.log_exit()

        # Logfile should have 2 output lines, the first containing the string
        # 'started' and the second containing the string 'finished'.
        f.seek(0)
        self.assertIn('started', f.readline())
        self.assertIn('finished', f.readline())

        # Check for EOF and remove the temp file
        self.assertEqual('', f.readline())
        f.close()
        os.unlink(path)

    def test_log_levels(self):
        (h, path) = tempfile.mkstemp()
        f = os.fdopen(h)
        self.assertSuccess(libtuna.log_init(path, __file__))

        # Output one message at each log level except fatal (as that would kill
        # the script).
        self.assertSuccess(libtuna.log_print(libtuna.LOG_MESSAGE, "Test"))
        self.assertSuccess(libtuna.log_print(libtuna.LOG_WARNING, "Test2"))
        self.assertSuccess(libtuna.log_print(libtuna.LOG_ERROR, "Test3"))

        libtuna.log_exit()

        # Logfile should have 5 output lines, containing the strings 'started',
        # 'MESSAGE', 'WARNING', 'ERROR' and 'finished', respectively.
        f.seek(0)
        self.assertIn('started', f.readline())
        self.assertTrue(f.readline().startswith('MESSAGE:'))
        self.assertTrue(f.readline().startswith('WARNING:'))
        self.assertTrue(f.readline().startswith('ERROR:'))
        self.assertIn('finished', f.readline())

        # Check for EOF and remove the temp file
        self.assertEqual('', f.readline())
        f.close()
        os.unlink(path)

if __name__ == '__main__':
    unittest.main(testRunner=tunaTestRunner())
