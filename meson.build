# NOTE this metadata DUPLICATED in 'default.nix'. We must MANUALLY sync both because:
#   - The Meson build setup must stand alone adn *not* depend on Nix
#   - Sirio sees no way to have Nix parse these variables when building the derivation
project('nonlibc', 'c',
	version : '0.2.3',
	license : 'GPL2',
	default_options : [ 'c_std=gnu11' ])
description = 'Collection of standard-not-standard utilities for the discerning C programmer'

#sanity
assert(meson.project_name() == 'nonlibc', 'Incorrect project name')
if (build_machine.cpu_family() == 'x86_64')
	assert(meson.version() >= '0.44.0', 'unfortuately, rpath issues in previous versions')
endif


#
#	build options by type
#
# debug : all the pedantic things!
# debugoptimized : daily worker build; basically 'release' but with symbols
if get_option('buildtype') == 'debug'
  add_project_arguments([ '-march=native', '-mtune=native', '-fno-omit-frame-pointer',
			'-DZ_LOG_LVL=(Z_inf | Z_in2 | Z_wrn | Z_wr2 | Z_err )' ],
			language : 'c')

elif get_option('buildtype') == 'debugoptimized'
  add_project_arguments([ '-march=native', '-mtune=native', '-fno-omit-frame-pointer' ],
			language : 'c')

# release : optimize this! ship this!
elif get_option('buildtype') == 'release'
  add_project_arguments([ '-DNDEBUG', '-march=native', '-mtune=native', '-Ofast' ],
			language : 'c')

# plain : NO flags;
#+	use this to cross check the effect of flags on the other build types.
elif get_option('buildtype') == 'plain'
  add_project_arguments([ '-DNDEBUG' ],
			language : 'c')
endif


# deps
# deps are either statically or dynamically incorporated;
#+	see meson_options.txt for the default value, which can be overridden
#+	with a -Ddep_type=["shared"||"static"] flag when running meson.
dep_static = false
_dep = get_option('dep_type')
if _dep == 'static'
	dep_static = true
endif
# insert deps here:
urcu = dependency('liburcu', static: dep_static, required: true)

deps = [ urcu ]


inc = include_directories('include')
subdir('include')
subdir('lib')
subdir('test')
subdir('util')
subdir('man')
