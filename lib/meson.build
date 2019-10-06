lib_files = [
  'binhex.c',
  'epoll_track.c',
  'fnv.c',
  'lifo.c',
  'messenger.c',
  'nmem.c',
  'npath.c',
  'pcg_rand.c',
  'posigs.c'
  ]

if host_machine.system() == 'linux'
  lib_files += [
    'nmem_linux.c'
    ]

elif host_machine.system() == 'darwin' or host_machine.system() == 'freebsd'
  lib_files += [
    'epoll_kqueue.c',
    'nmem_bsd.c'
    ]
endif


nonlibc = both_libraries(
  meson.project_name(),
  lib_files,
  version: meson.project_version(),
  include_directories   : inc,
  dependencies          : deps,
  install               : true
  )

# Make linking work on linux systems without breaking nix
p = get_option('prefix')
if host_machine.system() == 'linux' and not p.startswith('/nix')
  meson.add_install_script('ldconfig', p + '/' + get_option('libdir'))
endif


# Force install_dir to avoid e.g. lib/x86_64-linux-gnu/pkgconfig on Ubuntu,
#+ which breaks other tools like Nix.
pkg = import('pkgconfig')
pkg.generate(
  libraries     : [nonlibc, nonlibc.get_static_lib()],
  requires      : deps,
  install_dir   : get_option('prefix') + '/lib/pkgconfig',
  name          : meson.project_name(),
  filebase      : meson.project_name(),
  version       : meson.project_version(),
  description   : description,
  url           : homepage
  )


# Export a shared dependency.
# Force dependers to have an rpath entry pointing to where we KNOW we'll install.
# This fixes both Nix and non-nix projects (providing 'prefix' is set correctly),
# and does so OPAQUELY (depender doesn't have to add an 'install_rpath' directive).
rpath = get_option('prefix') + '/' + get_option('libdir')
nonlibc_dep = declare_dependency(link_with : nonlibc,
					include_directories : inc,
					dependencies : deps,
					link_args : '-Wl,-rpath,' + rpath)
