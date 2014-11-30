################################################################################
#	unconfig: Because we hate autotools
#
#	Copyright (C) 2013 Paul Barker, Loughborough University
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
################################################################################

import datetime
import os
import subprocess
import sys
from pkg_resources import parse_version

_packages = {}
_vars = {}

################################################################################
# Variables
################################################################################
def var_set(name, value):
	_vars[name] = value.strip()

def var_weak_set(name, value):
	if name not in _vars:
		_vars[name] = value.strip()

def var_get(name):
	if name in _vars:
		return _vars[name]
	else:
		return None

def var_append(name, value):
	if name in _vars and _vars[name]:
		_vars[name] = " ".join([_vars[name], value.strip()])
	else:
		_vars[name] = value.strip()

def var_remove(name):
	if name in _vars:
		del _vars[name]

################################################################################
# Packages and pkg-config
################################################################################
def run_pkgconfig(name, v):
	exe = "pkg-config"
	b = subprocess.check_output([exe, name, v])
	return b.decode("utf-8").strip()

def pkgconfig(name):
	pkg = {}
	pkg["NAME"] = name
	try:
		pkg["CFLAGS"] = run_pkgconfig(name, "--cflags")
		pkg["LDLIBRARIES"] = run_pkgconfig(name, "--libs")
		pkg["VERSION"] = run_pkgconfig(name, "--modversion")
		return pkg
	except Exception:
		return None

def pkg_atleast(pkg, minver):
	return parse_version(pkg["VERSION"]) >= parse_version(minver)

def configure_pkg(name, minver=None):
	print("Searching for pkg: %s" % name)
	pkg = pkgconfig(name)
	if pkg:
		print("Found: %s %s" % (pkg["NAME"], pkg["VERSION"]))
		if minver:
			print("Checking version >= %s", minver)
			if pkg_atleast(pkg, minver):
				print("Version is good")
			else:
				print("Version is bad")
				return False
	else:
		print("Not found: %s" % name)
		return False

	_packages[name] = pkg
	return True

def finalize_pkgs():
	# Combine CFLAGS and LDLIBRARIES for all packages
	for p in _packages.values():
		if p["CFLAGS"]:
			var_append("CFLAGS", p["CFLAGS"])
		if p["LDLIBRARIES"]:
			var_append("LDLIBRARIES", p["LDLIBRARIES"])

def configure_numpy():
	print("Finding numpy include directory")
	try:
		import numpy
		path = numpy.get_include()
		print("Found: %s" % path)
		var_append("CFLAGS", "-I%s" % path)
		return True
	except:
		print("Not found!")
		return False

################################################################################
# Install directories
################################################################################

def get_sitedir(prefix, exec_prefix):
	# Python is too clever and recommends we use distutils and a setup.py
	# file to install our extensions. However, that would require firkling
	# about to get the flags and options that we've determined correctly
	# passed to setup.py and it would require a lot of messing with our
	# dependency tracking. Basically, it's a load of nonsense.
	#
	# To install python packages by hand, we just need to know where to put
	# them. This will vary system to system, but we can query the 'site'
	# module to get this info. We will need to temporarily modify the
	# prefixes to those given by our arguments to ensure things are put in
	# the right place.
	import site
	tmp_prefixes = site.PREFIXES
	site.PREFIXES = [prefix, exec_prefix]
	site_packages_dir = site.getsitepackages()[0]
	site.PREFIXES = tmp_prefixes
	return site_packages_dir

def configure_install_dirs(default_prefix="/usr/local"):
	# DESTDIR is expected to be handled in the Makefile (or other user of
	# the config outputs) and is not set here. We just set prefix and
	# bindir, libdir, etc based on prefix.
	#
	# This is probably the only place GNU standards are useful. See 
	# http://www.gnu.org/prep/standards/html_node/Directory-Variables.html
	#
	# Note that exec_prefix is not yet supported
	var_weak_set("prefix", default_prefix)
	prefix = var_get("prefix")

	var_weak_set("bindir", os.path.join(prefix, "bin"))
	var_weak_set("libdir", os.path.join(prefix, "lib"))
	var_weak_set("includedir", os.path.join(prefix, "include"))
	var_weak_set("docdir", os.path.join(prefix, "share/doc"))
	var_weak_set("python_sitedir", get_sitedir(prefix, prefix))

################################################################################
# Toolchain
################################################################################
def default_parse_version(b):
	return b.decode("utf-8").splitlines()[0]

def check_tool(tool, args, parse_version):
	try:
		b = subprocess.check_output(tool + " " + args, shell=True)
		s = parse_version(b)
		print("Found: %s" % s)
		return tool
	except Exception:
		print("Not found: %s" % tool)
		return None

def configure_tool(key, default_name, args="--version", use_tool_prefix=False,
		parse_version=default_parse_version):
	tool = var_get(key)
	if not tool:
		# Build tool name ourself
		if use_tool_prefix:
			prefix = var_get("TOOL_PREFIX")
			if not prefix:
				prefix = ""
			tool = prefix + default_name
		else:
			tool = default_name
	print("Searching for %s: %s" % (key, tool))
	value = check_tool(tool, args, parse_version)
	if value:
		var_set(key, value)
	return value

def configure_cc(default_name="gcc"):
	return configure_tool("CC", default_name, use_tool_prefix=True)

def configure_ld(default_name="ld"):
	return configure_tool("LD", default_name, use_tool_prefix=True)

def configure_ccld(default_name="gcc"):
	return configure_tool("CCLD", default_name, use_tool_prefix=True)

def configure_ar(default_name="ar"):
	return configure_tool("AR", default_name, use_tool_prefix=True)

def configure_doxygen(default_name="doxygen"):
	return configure_tool("DOXYGEN", default_name)

def swig_parse_version(b):
	return b.decode("utf-8").splitlines()[1]

def configure_swig(default_name="swig"):
	return configure_tool("SWIG", default_name, args="-version",
			parse_version=swig_parse_version)

################################################################################
# Configuration management
################################################################################
def init(name, version):
	var_set("PROJECT_NAME", name)
	var_set("PROJECT_VERSION", version)
	print("Configuring %s %s" % (name, version))

def finalize():
	print("Configuration Successful\n")
	finalize_pkgs()

def fail():
	print("Configuration failed!")
	sys.exit(-1)

def require(v):
	if not v:
		print("Requirements not met")
		fail()

################################################################################
# Command line arguments and environment variables
################################################################################
def parse_cmdline(argv=sys.argv):
	# Skip first argument as that will be the name of the script
	argv = argv[1:]
	for a in argv:
		# Expect "a=b" format, if we just get "a" then assume b is 1.
		# ie. "a" is an alias for "a=1"
		pair = a.split('=', 1)
		if len(pair) == 1:
			pair.append('1')
		key = pair[0]
		value = pair[1]
		# Strip leading dashes off key so "--prefix" aliases to "prefix"
		key = key.lstrip('-')
                # Alias 'disable-x' to remove any instance of 'enable-x'
		if key.startswith('disable-'):
			key = 'enable-' + key[8:]
			var_remove(key)
		else:
			var_set(key, value)

def read_env(key):
	value = os.environ.get(key)
	# Use weak set so that these values from the environment may be easily
	# overridden
	if value:
		var_weak_set(key, value)

def read_envs(keylist):
	for key in keylist:
		read_env(key)

################################################################################
# Support for out-of-tree builds
################################################################################
def find_srcdir():
	path = os.path.dirname(sys.argv[0])
	var_set("SRCDIR", path)

def is_out_of_tree():
	srcdir = os.path.realpath(var_get("SRCDIR"))
	builddir = os.path.realpath(".")
	return srcdir != builddir

def link_makefile(filename="Makefile"):
	if is_out_of_tree():
		srcdir = var_get("SRCDIR")
		path = os.path.join(srcdir, filename)
		# Warning: That srcdir check better be good!
		if os.path.exists(filename):
			os.unlink(filename)
		os.symlink(path, filename)

################################################################################
# Output
################################################################################

# Output to terminal
def dump_config():
	for (k, v) in sorted(_vars.items()):
		print("%s = %s" % (k, v))

def write_config_makefile(filename="unconfig.mk"):
	f = open(filename, "w")
	tm = datetime.datetime.now()
	f.write("# Autogenerated by unconfig at %s\n" % tm.isoformat(' '))
	f.write("# Modification to this file may be overwritten!\n\n")
	for (k, v) in sorted(_vars.items()):
		f.write("%s := %s\n" % (k, v))
	f.write("\n# End\n")
