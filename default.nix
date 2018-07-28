{	# deps
	system ? builtins.currentSystem,
	nixpkgs ? import <nixpkgs> { inherit system; },
	# options
	buildtype ? "release",
	compiler ? "clang",
	dep_type ? "shared",
	mesonFlags ? ""
}:

with import <nixpkgs> { inherit system; };

stdenv.mkDerivation rec {
	name = "nonlibc";
	outputs = [ "out" ];

	# build-only deps
	# TODO: I would ASSUME that 'compiler' is automatically added as a dependency;
	#+	alas, this is not so: `nix-shell --argstr compiler gcc --pure`
	#+	yields an environment where clang is the only available compiler.
	# WARNING: do not try to include 'fpm' here:
	#+	- 'ar' on macOS does not support '-D' flag which fpm requires
	#+	- adding binutils to the install solves 'ar' but now gives:
	#+		ld: warning: ignoring ... not the architecture being linked (x86_64)
	#_	... ON an x86_64 machine (logical!)
	nativeBuildInputs = [
		cscope
		meson
		ninja
		pandoc
		pkgconfig
		python3
		which
	];

	# runtime deps
	buildInputs = [
		liburcu
	];

	# just work with the current directory (aka: Git repo), no fancy tarness
	src = ./.;

	# Override the setupHook in the meson nix der. - we will config ourselves thanks
	meson = pkgs.meson.overrideAttrs ( oldAttrs: rec { setupHook = ""; });

	# don't harden away position-dependent speedups for static builds
	hardeningDisable = [ "pic" "pie" ];

	# Allow YouCompleteMe and other tooling to see into the byzantine
	#+	labyrinth of library includes.
	# TODO: this is a total hack: do the string manipulation in Nix and
	#+	just export CPATH.
	# TODO: once cleaned up, back-port to the derivations for nonlibc and memorywell and ffec
	shellHook=''export CPATH=$(echo $NIX_CFLAGS_COMPILE | sed "s/ \?-isystem /:/g")'';

	# build
	mFlags = mesonFlags
		+ " --buildtype=${buildtype}"
		+ " -Ddep_type=${dep_type}";
	configurePhase = ''
		echo "pkgconfig: $PKG_CONFIG_PATH"
		echo "flags: $mFlags"
		echo "prefix: $out"
		CC=${compiler} meson --prefix=$out build $mFlags
		cd build
	'';

	buildPhase = "ninja";
	doCheck = true;
	checkPhase = "ninja test";
	installPhase = "ninja install";
}
