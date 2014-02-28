#! /usr/bin/env python
################################################################################
#   000_import.py: Basic test of `import libtuna`
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
#
# The import test is kept on its own so that all future tests can simply do
# 'import libtuna' and expect it to succeed.

from tuna_test import *
import unittest

class tunaImportTest(tunaTestCase):
    def test_import(self):
        try:
            import libtuna
            self.assertIsNotNone(libtuna)
        except ImportError as err:
            self.fail(str(err))

if __name__ == '__main__':
    unittest.main(testRunner=tunaTestRunner())
