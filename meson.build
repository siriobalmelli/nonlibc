project('nonlibc', 'c',
	version : '0.2.2',
	license : 'GPL2',
	default_options : [ 'c_std=gnu11' ])
description = 'Collection of standard-not-standard utilities for the discerning C programmer'

#sanity
assert(meson.project_name() == 'nonlibc', 'Incorrect project name')
assert(meson.version() >= '0.44.0', 'unfortuately, rpath issues in previous versions')


#
#	build options by type
#
# debug : all the pedantic things!
# debugoptimized : daily worker build; basically 'release' but with symbols
if get_option('buildtype') == 'debugoptimized'
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
_dep = get_option('dep_type')
if _dep == 'shared'
	dep_shared = true
elif _dep == 'static'
	dep_shared = false
else
	error('-Ddep_type expects "shared" or "static"; got ' + _dep)
endif
# insert deps here:

deps = [ ]


inc = include_directories('include')
subdir('include')
subdir('src')
subdir('test')
subdir('util')
subdir('man')
