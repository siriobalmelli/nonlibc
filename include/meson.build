##
#	project configuration
##
conf_data = configuration_data()

# TODO: not necessarily true on all systems
conf_data.set('NLC_CACHE_LINE', 64)

conf = configure_file(input : 'nonlibc.h.in',
	      output: 'nonlibc.h',
	      configuration : conf_data)

##
#	headers
##
headers = [ conf,
  'binhex.h', 'b2hx.h', 'hx2b.h', 
  'fnv.h', 'lifo.h',
  'nmath.h', 'nmem.h', 'nlc_urand.h', 'npath.h',
  'pcg_rand.h', 'posigs.h', 'zed_dbg.h'
]
# O/S-specific
if host_machine.system() == 'linux'
  headers += 'epoll_track.h'
endif

# We assume that we will be statically linked if we're a subproject;
#+	ergo: don't pollute the system with our headers
if not meson.is_subproject()
	install_headers(headers)
endif
