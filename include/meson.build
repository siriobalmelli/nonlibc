##
#   project configuration
##
conf_data = configuration_data()

##
#   headers
##
headers = [
  'nonlibc.h',
  'ndebug.h',

  'epoll_track.h',
  'nlc_epoll.h',

  'b2hx.h',
  'binhex.h',
  'hx2b.h',

  'fnv.h',
  'lifo.h',
  'messenger.h',
  'nlc_endian.h',
  'nlc_urand.h',
  'nmath.h',
  'nmem.h',
  'npath.h',
  'nstring.h',
  'pcg_rand.h',
  'posigs.h'
  ]

# O/S-specific
if host_machine.system() == 'darwin' or host_machine.system() == 'freebsd'
  headers += [
    'epoll_kqueue.h'
    ]
elif host_machine.system() == 'linux'
  headers += [
    'nlc_linuxversion.h'
  ]
endif

# We assume that we will be statically linked if we're a subproject;
#+  ergo: don't pollute the system with our headers
if not meson.is_subproject()
  install_headers(headers)
endif
