################################################################################
#   tuna_test.py: Support classes for TUNA testsuite
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

import unittest
import sys

class tunaTestCase(unittest.TestCase):
    def __str__(self):
        return "%s: %s" % (str(sys.argv[0]), self._testMethodName)

    def assertSuccess(self, cond):
        self.assertGreaterEqual(cond, 0)

    def assertFailure(self, cond):
        self.assertLess(cond, 0)

class tunaTestResult(unittest.TestResult):
    def __init__(self, runner):
        super(tunaTestResult, self).__init__()
        self.runner = runner

    def addError(self, test, err):
        super(tunaTestResult, self).addError(test, err)
        self.runner.write("ERROR: %s: %s\n" % (str(test), str(err[1])))

    def addSuccess(self, test):
        super(tunaTestResult, self).addSuccess(test)
        self.runner.write("PASS: %s\n" % str(test))

    def addFailure(self, test, err):
        super(tunaTestResult, self).addFailure(test, err)
        self.runner.write("FAIL: %s: %s\n" % (str(test), str(err[1])))

    def addSkip(self, test, reason):
        super(tunaTestResult, self).addSkip(test, reason)
        self.runner.write("SKIP: %s: %s\n" % (str(test), str(reason)))

    def addExpectedFailure(self, test, err):
        super(tunaTestResult, self).addExpectedFailure(test, err)
        self.runner.write("XFAIL: %s: %s\n" % (str(test), str(err[1])))

    def addUnexpectedSuccess(self, test):
        super(tunaTestResult, self).addUnexpectedSuccess(test)
        self.runner.write("XPASS: %s\n" % str(test))

class tunaTestRunner:
    def __init__(self, stream=sys.stderr):
        self.stream = stream

    def write(self, message):
        self.stream.write(message)

    def run(self, test):
        result = tunaTestResult(self)
        test(result)
        return result
