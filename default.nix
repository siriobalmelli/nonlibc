{
  nixpkgs ? import (builtins.fetchGit {
    url = "https://siriobalmelli@github.com/siriobalmelli-foss/nixpkgs.git";
    ref = "refs/tags/sirio-2022-08-24";
    }) { }
}:

with nixpkgs;

stdenv.mkDerivation rec {
  name = "nonlibc";
  version = with builtins; replaceStrings ["\n" " "] ["" ""] (readFile ./VERSION);

  meta = with lib; {
    description = "Collection of standard-not-standard utilities for the discerning C programmer";
    homepage = https://siriobalmelli.github.io/nonlibc;
    license = licenses.lgpl21Plus;
    platforms = platforms.unix;
    maintainers = with maintainers; [ siriobalmelli ];
  };

  nativeBuildInputs = [
    gcc
    meson
    ninja
    pandoc
    pkgconfig
    python3
  ];

  propagatedBuildInputs = [
    liburcu
  ];

  # just work with the current directory (aka: Git repo), no fancy tarness
  src = nix-gitignore.gitignoreSource [] ./.;

  patchPhase = ''
    patchShebangs util/test_fnvsum.py
    patchShebangs util/test_ncp.py

    # see include/nlc_linuxversion.h for the gory details
    if [ -e /usr/include/linux/version.h ]; then
      cp -fv /usr/include/linux/version.h include/nlc_linuxversion.h
    fi
  '';

  # don't harden away position-dependent speedups for static builds
  hardeningDisable = [ "pic" "pie" ];
  mesonBuildType = "release";

  doCheck = true;
  checkTarget = "test";
}
