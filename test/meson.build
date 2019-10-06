tests = [
  'atop_test.c',
  'fnv_test.c',
  'binhex_test.c',
  'lifo_test.c',
  'mg_test.c',
  'mgrp_test.c',
  'ndebug_test.c',
  'nlc_urand_test.c',
  'nmath_test.c',
  'npath_test.c',
  'nstring_test.c',
  'pcg_rand_test.c',
  'epoll_track_test.c',
  'epoll_track_test_types.c',
  'epoll_track_test_destructor.c'
  ]

if host_machine.system() == 'linux'
  tests += [
    'epoll_track_test.c',
    'epoll_track_test_types.c',
    'epoll_track_test_destructor.c'
    ]
endif


# pthread dependency only for tests, not for nonlibc itself
test_deps = deps + [dependency('threads')]


foreach t : tests
  name = t.split('.')[0]
  name_format = name.split('_')[0]

  a_test = executable(name, t,
    include_directories : inc,
    dependencies        : test_deps,
    link_with           : nonlibc
    )
  # the 'core' test suite is what gets checked with e.g. Valgrind
  test(name_format,
    a_test,
    suite   : 'core',
    timeout : 45
    )

  test_static = executable(name + '_static', t,
    include_directories : inc,
    dependencies        : test_deps,
    link_with           : nonlibc.get_static_lib()
    )
  test(name_format,
    test_static,
    suite   : 'static',
    timeout : 45
    )
endforeach
