tests = [
	'atop_test.c',
	'fnv_test.c',
	'binhex_test.c',
	'lifo_test.c',
	'messenger_test.c',
	'nlc_urand_test.c',
	'nmath_test.c',
	'npath_test.c',
	'pcg_rand_test.c',
	'zed_dbg_test.c'
]

if host_machine.system() == 'linux'
  tests += [
	'epoll_track_test.c',
	'epoll_track_test_types.c'
  ]
endif

# pthread dependency only for tests, not for the library itself
test_deps = deps + [dependency('threads', static: dep_static)]

foreach t : tests
	name = t.split('.')[0]
	name_spaced = ' '.join(name.split('_'))

	a_test = executable(name + '_shared', t,
			include_directories : inc,
			dependencies : test_deps,
			link_with : nonlibc)
	test(name_spaced + ' (shared)', a_test, timeout : 45)

	test_static = executable(name + '_static', t,
			include_directories : inc,
			dependencies : test_deps,
			link_with : nonlibc_static)
	test(name_spaced + ' (static)', test_static, timeout : 45)
endforeach
