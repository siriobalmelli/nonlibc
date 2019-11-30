{
  # options
  buildtype ? "release",
  compiler ? "clang",
  mesonFlags ? "",

  # deps
  system ? builtins.currentSystem,
  nixpkgs ? import (builtins.fetchGit {
    url = "https://siriobalmelli@github.com/siriobalmelli-foss/nixpkgs.git";
    ref = "master";
    }) {}
}:

with nixpkgs;

stdenv.mkDerivation rec {
  name = "nonlibc";
  version = "0.4.2";

  meta = with stdenv.lib; {
    description = "Collection of standard-not-standard utilities for the discerning C programmer";
    homepage = https://siriobalmelli.github.io/nonlibc;
    license = licenses.lgpl21Plus;
    platforms = platforms.unix;
    maintainers = [ "https://github.com/siriobalmelli" ];
  };

  buildInputs = [
    clang
    gcc
    meson
    ninja
    pandoc
    pkgconfig
    python3
  ] ++ lib.optional lib.inNixShell [
    cscope
    gdb
    lldb
    valgrind
    which
  ];

  propagatedBuildInputs = [
    liburcu
  ];

  # TODO: split "packages" and "site" into separate outputs?
  outputs = [ "out" ];

  # just work with the current directory (aka: Git repo), no fancy tarness
  src = nix-gitignore.gitignoreSource [] ./.;

  # Override the setupHook in the meson nix der. - will config ourselves thanks
  meson = pkgs.meson.overrideAttrs ( oldAttrs: rec { setupHook = ""; });

  # don't harden away position-dependent speedups for static builds
  hardeningDisable = [ "pic" "pie" ];

  patchPhase = ''
    patchShebangs util/test_fnvsum.py
    patchShebangs util/test_ncp.py

    # see include/nlc_linuxversion.h for the gory details
    if [ -e /usr/include/linux/version.h ]; then
      cp -fv /usr/include/linux/version.h include/nlc_linuxversion.h
    fi
  '';

  # build
  mFlags = mesonFlags
    + " --buildtype=${buildtype}";
  configurePhase = ''
      CC=${compiler} meson --prefix=$out build $mFlags
      cd build
  '';

  buildPhase = ''
      ninja
  '';
  doCheck = true;
  checkPhase = ''
      ninja test
  '';
  installPhase = ''
      ninja install
  '';
}
