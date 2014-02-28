#! /usr/bin/env python
################################################################################
#   002_buffer.py: Tests for buffer management code
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

class tunaBufferTests(tunaTestCase):
    def test_buffer(self):
        (h, logpath) = tempfile.mkstemp()
        self.assertSuccess(libtuna.log_init(logpath, __file__))
        os.close(h)

        frames_in = 1000
        ptr, frames_out = libtuna.buffer_acquire(frames_in)
        self.assertIsNotNone(ptr)
        self.assertGreaterEqual(frames_out, frames_in)
        self.assertEqual(libtuna.buffer_refcount(ptr), 1)

        libtuna.buffer_addref(ptr)
        self.assertEqual(libtuna.buffer_refcount(ptr), 2)

        self.assertEqual(libtuna.buffer_release(ptr), 0)
        self.assertEqual(libtuna.buffer_refcount(ptr), 1)

        self.assertEqual(libtuna.buffer_release(ptr), 1)

        libtuna.log_exit()
        os.unlink(logpath)

if __name__ == '__main__':
    unittest.main(testRunner=tunaTestRunner())
