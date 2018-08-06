#	utils
# Any utilities directly provided by this library
fnvsum = executable('fnvsum', 'fnvsum.c',
		include_directories : inc,
		dependencies : nonlibc_dep_shared,
		install : true)

test('fnvsum test',
	find_program('test_fnvsum.py'),
	args : fnvsum.full_path())


ncp = executable('ncp', 'ncp.c',
		include_directories : inc,
		dependencies: nonlibc_dep_shared,
		install : true)

test('ncp test',
	find_program('test_ncp.py'),
	args : ncp.full_path())
