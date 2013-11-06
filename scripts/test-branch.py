#! /usr/bin/env python
#
#	test-branch.py: 'make check' on each commit on a branch
#
#	Copyright (C) 2013 Paul Barker
#
#	This program is free software; you can redistribute it and/or modify it
#	under the terms of the GNU General Public License as published by the
#	Free Software Foundation; either version 2, or (at your option) any
#	later version.
#
#	This program is distributed in the hope that it will be useful, but
#	WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#	General Public License for more details.

import subprocess as sp
import os

def rsh_s(cmd):
	data = sp.check_output(cmd, shell=True)
	s = data.decode("utf-8")
	return s

def sh(cmd):
	return sp.call(cmd, shell=True)

def rsh(cmd):
	return sp.check_call(cmd, shell=True)

def read_env(key, default_value):
	value = os.environ.get(key)
	if value:
		return value
	else:
		return default_value

def list_commits(c_from, c_to):
	s = rsh_s("git rev-list %s..%s" % (c_from, c_to))
	commits = s.strip().split("\n")
	commits.reverse()
	return commits

def do_checkout(commit):
	os.chdir(src_dir)
	rsh("git checkout %s" % commit)

def do_test():
	os.chdir(src_dir)
	rsh("./configure %s" % TUNA_CONFIGURE_OPTIONS)
	rsh("make")
	#rsh("make check")
	rsh("make DESTDIR=%s install" % install_dir)
	print("\n")

TUNA_MASTER = read_env("TUNA_MASTER", "master")
TUNA_BRANCH = read_env("TUNA_BRANCH", "HEAD")
TUNA_TEST_DIR = read_env("TUNA_TEST_DIR", "/tmp/tuna-test")
TUNA_CONFIGURE_OPTIONS = read_env("TUNA_CONFIGURE_OPTIONS", "")

repo_dir = os.getcwd()
commits = list_commits(TUNA_MASTER, TUNA_BRANCH)

for c in commits:
	print("========== %s ==========\n" % (c))

	commit_dir = os.path.join(TUNA_TEST_DIR, c)
	src_dir = os.path.join(commit_dir, "tuna")
	install_dir = os.path.join(commit_dir, "install")

	os.makedirs(src_dir, exist_ok=True)
	os.makedirs(install_dir, exist_ok=True)

	rsh("git clone %s %s" % (repo_dir, src_dir))
	do_checkout(c)
	do_test()
