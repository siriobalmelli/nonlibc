# utils
# Any utilities directly provided by this library
fnvsum = executable('fnvsum', 'fnvsum.c',
    include_directories : inc,
    dependencies : nonlibc_dep,
    install : true
    )

# TODO: fix breakage when library is wrapped
test('fnvsum',
    find_program('test_fnvsum.py'),
    args    : fnvsum.full_path(),
    suite   : 'utils'
    )


ncp = executable('ncp', 'ncp.c',
    include_directories : inc,
    dependencies        : nonlibc_dep,
    install             : true
    )

# TODO: fix breakage when library is wrapped
test('ncp',
    find_program('test_ncp.py'),
    args    : ncp.full_path(),
    suite   : 'utils'
    )
