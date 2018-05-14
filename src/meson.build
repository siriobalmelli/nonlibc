lib_files = [ 'b2hx.c', 'hx2b.c', 'epoll_track.c', 'fnv.c',
		'npath.c', 'pcg_rand.c', 'lifo.c', 'posigs.c' ]

lib_files += 'nmem.c'
if host_machine.system() == 'linux'
  lib_files += 'nmem_linux.c'
elif host_machine.system() == 'darwin' or host_machine.system() == 'bsd'
  lib_files += 'nmem_bsd.c'
endif

nonlibc = shared_library(meson.project_name(),
			lib_files,
			include_directories : inc,
			dependencies : deps,
			install : true)
# Make linking work on linux systems without breaking nix
p = get_option('prefix')
if host_machine.system() == 'linux' and not p.startswith('/nix')
	meson.add_install_script('ldconfig', p + '/' + get_option('libdir'))
endif

nonlibc_static = static_library(meson.project_name(),
			lib_files,
			include_directories : inc,
			dependencies : deps,
			install : true)

# don't set anything here, rely on variables declared in toplevel file
pkg = import('pkgconfig')
pkg.generate(	libraries : [ nonlibc_static, nonlibc ],
		name : meson.project_name(),
		version : meson.project_version(),
		filebase : meson.project_name(),
		description : description)


#   Export a static dependency (straightforward)
nonlibc_dep_static = declare_dependency(link_with : nonlibc_static,
					include_directories : inc,
					dependencies : deps)

#   Export a shared dependency (a royal pain).
#
# Force dependers to have an rpath entry pointing to where we KNOW we'll install.
# This fixes both Nix and non-nix projects (providing 'prefix' is set correctly),
#+  and does OPAQUELY (depender doesn't have to add an 'install_rpath' directive).
rpath = get_option('prefix') + '/' + get_option('libdir')
nonlibc_dep_shared = declare_dependency(link_with : nonlibc,
					include_directories : inc,
					dependencies : deps,
					link_args : '-Wl,-rpath,' + rpath)
