##
#   project configuration
##
conf_data = configuration_data()

##
#   headers
##
headers = [ 'nonlibc.h',

    'b2hx.h',
    'binhex.h',
    'hx2b.h',

    'fnv.h',
    'lifo.h',
    'messenger.h',
    'nlc_urand.h',
    'nmath.h',
    'nmem.h',
    'npath.h',
    'pcg_rand.h',
    'posigs.h',
    'zed_dbg.h'
]

# O/S-specific
if host_machine.system() == 'linux'
    headers += 'epoll_track.h'
endif

# We assume that we will be statically linked if we're a subproject;
#+  ergo: don't pollute the system with our headers
if not meson.is_subproject()
    install_headers(headers)
endif
