{
  # options
  buildtype ? "release",
  compiler ? "clang",
  dep_type ? "shared",
  mesonFlags ? "",

  # deps
  system ? builtins.currentSystem,
  nixpkgs ? import <nixpkgs> { inherit system; }
}:

with nixpkgs;

stdenv.mkDerivation rec {
  name = "nonlibc";
  version = "0.3.3";

  meta = with stdenv.lib; {
    description = "Collection of standard-not-standard utilities for the discerning C programmer";
    homepage = https://siriobalmelli.github.io/nonlibc/;
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
    dpkg
    fpm
    rpm
    which
    zip
  ];
  propagatedBuildInputs = [
    liburcu
  ];

  # TODO: split "packages" and "site" into separate outputs?
  outputs = [ "out" ];

  # just work with the current directory (aka: Git repo), no fancy tarness
  src = ./.;

  # Override the setupHook in the meson nix der. - will config ourselves thanks
  meson = pkgs.meson.overrideAttrs ( oldAttrs: rec { setupHook = ""; });

  # don't harden away position-dependent speedups for static builds
  hardeningDisable = [ "pic" "pie" ];

  patchPhase = ''
    patchShebangs util/test_fnvsum.py
    patchShebangs util/test_ncp.py
  '';

  # build
  mFlags = mesonFlags
    + " --buildtype=${buildtype}"
    + " -Ddep_type=${dep_type}";
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

  # Build packages outside $out then move them in: fpm seems to ignore
  #+	the '-x' flag that we need to avoid packaging packages inside packages
  # NOTE also the `''` escape so that Nix will leave `${` alone (facepalms internally)
  postFixup = ''
      # manual dependency listing
      declare -A DEPS=( \
          ["deb"]="-d liburcu-dev" \
          ["rpm"]="-d liburcu" \
          ["tar"]="" \
          ["zip"]="")

      mkdir temp
      for pk in "deb" "rpm" "tar" "zip"; do
          if ! fpm -f -t $pk -s dir -p temp/ -n $name -v $version \
              --description "${meta.description}" \
              --license "${meta.license.spdxId}" \
              --url "${meta.homepage}" \
              --maintainer "${builtins.head meta.maintainers}" \
              -x "nix-support" \
              ''${DEPS[$pk]} \
              "$out/=/"
          then
              echo "ERROR (non-fatal): could not build $pk package" >&2
          fi
      done
      mkdir -p $out/var/cache/packages
      mv -fv temp/* $out/var/cache/packages/
  '';
}
