#! /usr/bin/env python
################################################################################
#   003_bufhold.py: Tests for buffer holding code
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

class tunaBufholdTests(tunaTestCase):
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

    def test_bufhold_00_init(self):
        bh = libtuna.bufhold_init()
        self.assertIsNotNone(bh)

        libtuna.bufhold_exit(bh)

    def test_bufhold_01_single(self):
        bh = libtuna.bufhold_init()
        self.assertIsNotNone(bh)

        frames_in = 100
        ptr, frames_out = libtuna.buffer_acquire(frames_in)
        self.assertIsNotNone(ptr)
        self.assertGreaterEqual(frames_out, frames_in)
        self.assertEqual(libtuna.buffer_refcount(ptr), 1)
        frames = frames_out
        del frames_in
        del frames_out

        self.assertSuccess(libtuna.bufhold_add(bh, ptr, frames))

        # Ensure that bufhold_add called buffer_addref
        self.assertEqual(libtuna.buffer_refcount(ptr), 2)

        # By definition, bufhold_newest called immediately after bufhold_add
        # will return the held_buffer object setup in that call
        held = libtuna.bufhold_newest(bh)
        self.assertIsNotNone(held)
        self.assertEqual(libtuna.bufhold_data(held), ptr)
        self.assertEqual(libtuna.bufhold_count(held), frames)

        # There is only one held buffer in the list
        self.assertEqual(libtuna.bufhold_oldest(bh), held)
        self.assertIsNone(libtuna.bufhold_next(held))
        self.assertIsNone(libtuna.bufhold_prev(held))

        # Now remove our held buffer and ensure that the list is now empty
        libtuna.bufhold_release(bh, held)
        self.assertIsNone(libtuna.bufhold_newest(bh))
        self.assertIsNone(libtuna.bufhold_oldest(bh))

        # Ensure that bufhold_release called buffer_release
        self.assertEqual(libtuna.buffer_refcount(ptr), 1)

        libtuna.bufhold_exit(bh)
        self.assertEqual(libtuna.buffer_release(ptr), 1)
        self.assertNoErrors()

    def test_bufhold_02_multiple(self):
        bh = libtuna.bufhold_init()
        self.assertIsNotNone(bh)

        # Allocate 3 buffers
        frames_in = 100
        ptr_1, frames_1 = libtuna.buffer_acquire(frames_in)
        self.assertIsNotNone(ptr_1)
        self.assertGreaterEqual(frames_1, frames_in)

        frames_in = 200
        ptr_2, frames_2 = libtuna.buffer_acquire(frames_in)
        self.assertIsNotNone(ptr_2)
        self.assertGreaterEqual(frames_2, frames_in)

        frames_in = 300
        ptr_3, frames_3 = libtuna.buffer_acquire(frames_in)
        self.assertIsNotNone(ptr_3)
        self.assertGreaterEqual(frames_3, frames_in)
        del frames_in

        # By definition, bufhold_newest called immediately after bufhold_add
        # will return the held_buffer object setup in that call
        self.assertSuccess(libtuna.bufhold_add(bh, ptr_1, frames_1))
        held_1 = libtuna.bufhold_newest(bh)
        self.assertIsNotNone(held_1)
        self.assertEqual(libtuna.bufhold_data(held_1), ptr_1)
        self.assertEqual(libtuna.bufhold_count(held_1), frames_1)

        # There is only one held buffer in the list
        self.assertEqual(libtuna.bufhold_oldest(bh), held_1)
        self.assertIsNone(libtuna.bufhold_next(held_1))
        self.assertIsNone(libtuna.bufhold_prev(held_1))

        # By definition, bufhold_newest called immediately after bufhold_add
        # will return the held_buffer object setup in that call
        self.assertSuccess(libtuna.bufhold_add(bh, ptr_2, frames_2))
        held_2 = libtuna.bufhold_newest(bh)
        self.assertIsNotNone(held_2)
        self.assertEqual(libtuna.bufhold_data(held_2), ptr_2)
        self.assertEqual(libtuna.bufhold_count(held_2), frames_2)

        # There are now two held buffers in the list
        self.assertEqual(libtuna.bufhold_oldest(bh), held_1)
        self.assertEqual(libtuna.bufhold_next(held_1), held_2)
        self.assertIsNone(libtuna.bufhold_next(held_2))
        self.assertIsNone(libtuna.bufhold_prev(held_1))
        self.assertEqual(libtuna.bufhold_prev(held_2), held_1)

        # By definition, bufhold_newest called immediately after bufhold_add
        # will return the held_buffer object setup in that call
        self.assertSuccess(libtuna.bufhold_add(bh, ptr_3, frames_3))
        held_3 = libtuna.bufhold_newest(bh)
        self.assertIsNotNone(held_3)
        self.assertEqual(libtuna.bufhold_data(held_3), ptr_3)
        self.assertEqual(libtuna.bufhold_count(held_3), frames_3)

        # There are now three held buffers in the list
        self.assertEqual(libtuna.bufhold_oldest(bh), held_1)
        self.assertEqual(libtuna.bufhold_next(held_1), held_2)
        self.assertEqual(libtuna.bufhold_next(held_2), held_3)
        self.assertIsNone(libtuna.bufhold_next(held_3))
        self.assertIsNone(libtuna.bufhold_prev(held_1))
        self.assertEqual(libtuna.bufhold_prev(held_2), held_1)
        self.assertEqual(libtuna.bufhold_prev(held_3), held_2)

        # Release the middle buffer and reassess the whole list
        libtuna.bufhold_release(bh, held_2)
        self.assertEqual(libtuna.bufhold_newest(bh), held_3)
        self.assertEqual(libtuna.bufhold_oldest(bh), held_1)
        self.assertEqual(libtuna.bufhold_next(held_1), held_3)
        self.assertIsNone(libtuna.bufhold_next(held_3))
        self.assertIsNone(libtuna.bufhold_prev(held_1))
        self.assertEqual(libtuna.bufhold_prev(held_3), held_1)

        # Release the oldest buffer and reassess
        libtuna.bufhold_release(bh, held_1)
        self.assertEqual(libtuna.bufhold_newest(bh), held_3)
        self.assertEqual(libtuna.bufhold_oldest(bh), held_3)
        self.assertIsNone(libtuna.bufhold_next(held_3))
        self.assertIsNone(libtuna.bufhold_prev(held_3))

        # Re-add a buffer so we again have a pair, then remove the newest
        # instead of the oldest.
        self.assertSuccess(libtuna.bufhold_add(bh, ptr_1, frames_1))
        held_1 = libtuna.bufhold_newest(bh)
        self.assertIsNotNone(held_1)
        self.assertEqual(libtuna.bufhold_newest(bh), held_1)
        self.assertEqual(libtuna.bufhold_oldest(bh), held_3)
        self.assertEqual(libtuna.bufhold_next(held_3), held_1)
        self.assertIsNone(libtuna.bufhold_next(held_1))
        self.assertIsNone(libtuna.bufhold_prev(held_3))
        self.assertEqual(libtuna.bufhold_prev(held_1), held_3)

        libtuna.bufhold_release(bh, held_1)
        self.assertEqual(libtuna.bufhold_newest(bh), held_3)
        self.assertEqual(libtuna.bufhold_oldest(bh), held_3)
        self.assertIsNone(libtuna.bufhold_next(held_3))
        self.assertIsNone(libtuna.bufhold_prev(held_3))

        # Now try releasing all the buffers in one go and check we have an empty
        # list
        libtuna.bufhold_release_all(bh)
        self.assertIsNone(libtuna.bufhold_newest(bh))
        self.assertIsNone(libtuna.bufhold_oldest(bh))

        libtuna.bufhold_exit(bh)
        self.assertEqual(libtuna.buffer_release(ptr_1), 1)
        self.assertEqual(libtuna.buffer_release(ptr_2), 1)
        self.assertEqual(libtuna.buffer_release(ptr_3), 1)
        self.assertNoErrors()

    def test_bufhold_03_advance(self):
        bh = libtuna.bufhold_init()
        self.assertIsNotNone(bh)

        # Allocate 2 equal length buffers
        frames_in = 100
        ptr_1, frames_1 = libtuna.buffer_acquire(frames_in)
        self.assertIsNotNone(ptr_1)
        self.assertGreaterEqual(frames_1, frames_in)
        ptr_2, frames_2 = libtuna.buffer_acquire(frames_in)
        self.assertIsNotNone(ptr_2)
        self.assertGreaterEqual(frames_2, frames_in)

        # Treat all buffers as if they have the originally specified length
        frames = frames_in
        del frames_in

        # Add them to a bufhold queue
        self.assertSuccess(libtuna.bufhold_add(bh, ptr_1, frames))
        held_1 = libtuna.bufhold_newest(bh)
        self.assertIsNotNone(held_1)
        self.assertSuccess(libtuna.bufhold_add(bh, ptr_2, frames))
        held_2 = libtuna.bufhold_newest(bh)
        self.assertIsNotNone(held_2)

        # Advance the oldest buffer by half its length
        self.assertEqual(libtuna.bufhold_advance(bh, held_1, 50), 50)

        # We can't assume things about pointer arithmetic or mess with that from
        # Python but we can say that the data pointer of the first held buffer
        # should have changed, even if we can't say what it should have changed
        # to.
        self.assertNotEqual(libtuna.bufhold_data(held_1), ptr_1)
        self.assertEqual(libtuna.bufhold_count(held_1), 50)

        # Advance the oldest buffer by half its original length again and ensure
        # it is now freed. We don't need to check prev/next pointers as the last
        # test case verified their behaviour, we just need to check that
        # bufhold_oldest gives the new correct result.
        self.assertEqual(libtuna.bufhold_advance(bh, held_1, 50), 0)
        self.assertEqual(libtuna.bufhold_oldest(bh), held_2)
        self.assertEqual(libtuna.buffer_refcount(ptr_1), 1)

        # Now advance the oldest buffer by its whole length in one go and ensure
        # it is released, leaving the bufhold queue empty.
        self.assertEqual(libtuna.bufhold_advance(bh, held_2, frames), 0)
        self.assertIsNone(libtuna.bufhold_oldest(bh))
        self.assertIsNone(libtuna.bufhold_newest(bh))
        self.assertEqual(libtuna.buffer_refcount(ptr_2), 1)

        libtuna.bufhold_exit(bh)
        self.assertEqual(libtuna.buffer_release(ptr_1), 1)
        self.assertEqual(libtuna.buffer_release(ptr_2), 1)
        self.assertNoErrors()

if __name__ == '__main__':
    unittest.main(testRunner=tunaTestRunner())
