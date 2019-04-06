# NOTE this metadata DUPLICATED in 'default.nix'. We must MANUALLY sync both because:
#   - The Meson build setup must stand alone and *not* depend on Nix
#   - Sirio sees no way to have Nix parse these variables when building the derivation
project('nonlibc', 'c',
  version : '0.4.1',
  license : 'LGPL-2.1+',
  default_options : [
    'b_coverage=true',
    'c_std=gnu11'
    ])
description = 'Collection of standard-not-standard utilities for the discerning C programmer'
homepage = 'https://siriobalmelli.github.io/nonlibc/'


# whole-project build flags (most of these are hardening flags)
add_project_arguments([
    '-D_FORTIFY_SOURCE=2',
    '-fasynchronous-unwind-tables',
    '-fexceptions',
    '-fstack-protector-strong',
    '-pipe',
    '-Wall',
    '-Werror=format-security',
    '-Werror=implicit-function-declaration'
    ],
  language : 'c')

# _all_ variables visible in gdb and core dumps
if get_option('buildtype') == 'debug'
  add_project_arguments([
    '-DDEBUG',
    '-fno-omit-frame-pointer',
    '-grecord-gcc-switches',
    '-O0',
    '-Wno-cpp'
    ],
    language : 'c')

elif get_option('buildtype') == 'debugoptimized'
  add_project_arguments([
    '-fno-omit-frame-pointer',
    '-grecord-gcc-switches',
    '-march=native',
    '-mtune=native',
    '-O2'
    ],
  language : 'c')

elif get_option('buildtype') == 'release'
  add_project_arguments([
    '-DNDEBUG',
    '-march=native',
    '-mtune=native',
    '-Ofast'
    ],
  language : 'c')

# use this to cross check the effect of flags on the other build types.
elif get_option('buildtype') == 'plain'
  add_project_arguments([
    '-DNDEBUG',
    '-march=core2'
    ],
  language : 'c')
endif


# insert deps here:
liburcu_dep = dependency('liburcu-bp')  # bulletproof RCU: we have no control over thread announcements

# All deps in a single arg. Use THIS ONE in compile calls
deps = [liburcu_dep]


inc = include_directories('include')
subdir('include')
subdir('lib')
subdir('test')
subdir('util')
subdir('man')
