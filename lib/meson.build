lib_files = [ 'binhex.c', 'fnv.c', 'nmem.c', 'npath.c', 'pcg_rand.c', 'lifo.c', 'posigs.c' ]

if host_machine.system() == 'linux'
  lib_files += 'epoll_track.c'
  lib_files += 'nmem_linux.c'
elif host_machine.system() == 'darwin' or host_machine.system() == 'freebsd'
  lib_files += 'nmem_bsd.c'
endif

# TODO: clean up using both_libraries() instead
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

# Force install_dir to avoid e.g. lib/x86_64-linux-gnu/pkgconfig on Ubuntu,
#+ which breaks other tools like Nix.
pkg = import('pkgconfig')
pkg.generate(	libraries : [ nonlibc_static, nonlibc ],
		requires : deps,
		install_dir : get_option('prefix') + '/lib/pkgconfig',
		name : meson.project_name(),
		filebase : meson.project_name(),
		version : meson.project_version(),
		description : description,
		url : homepage
)


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
