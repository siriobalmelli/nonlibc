#	utils
# Any utilities directly provided by this library
fnvsum = executable('fnvsum', 'fnvsum.c',
		include_directories : inc,
		dependencies : nonlibc_dep_shared,
		install : true)

test_fnvsum = find_program('test_fnvsum.py', required : false)
if test_fnvsum.found()
	test('fnvsum test', test_fnvsum, args : fnvsum.full_path())
else
	warning('could not find test script "test_fnvsum.py"')
endif


ncp = executable('ncp', 'ncp.c',
		include_directories : inc,
		dependencies: nonlibc_dep_shared,
		install : true)

test('ncp test',
	find_program('test_ncp.py'),
	args : ncp.full_path())
