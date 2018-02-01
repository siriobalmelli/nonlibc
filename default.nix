{		system ? builtins.currentSystem,
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
	nativeBuildInputs = [
		(lowPrio gcc)
		clang
		clang-tools
		cscope
		meson
		ninja
		pandoc
		pkgconfig
		python3
		valgrind
		which
	];

	# runtime deps
	buildInputs = [];

	# just work with the current directory (aka: Git repo), no fancy tarness
	src = ./.;

	# Override the setupHook in the meson nix der. - we will config ourselves thanks
	meson = pkgs.meson.overrideAttrs ( oldAttrs: rec { setupHook = ""; });

	# don't harden away position-dependent speedups for static builds
	hardeningDisable = [ "pic" "pie" ];

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
