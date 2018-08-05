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
	version = "0.2.3";
	description = "Collection of standard-not-standard utilities for the discerning C programmer";
	license = "GPL2";
	homepage = "https://siriobalmelli.github.io/nonlibc/";
	maintainers = [ "https://github.com/siriobalmelli" ];

	outputs = [ "out" ];

	# build-only deps
	# TODO: would be nice to replace 'clang' with the value of 'compiler' arg
	nativeBuildInputs = [
		clang
		meson
		ninja
		pandoc
		pkgconfig

		dpkg
		fpm
		rpm
		zip
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
	# Build packages _outside_ $out then move them in: fpm seems to ignore
	#+	the '-x' flag that we need to avoid packaging packages in packages
	installPhase = ''
		ninja install
		mkdir pkgs
		for pk in "deb" "rpm" "tar" "zip"; do
			if ! fpm -f -t $pk -s dir -p pkgs/ -n $name -v $version \
				--license "$license" --description "$description" \
				--maintainer "$maintainers" --url "$homepage" \
				"$out/=/"
			then
				echo "ERROR (non-fatal): could not build $pk package" >&2
			fi
		done
		mkdir -p $out/var/cache
		mv -fv pkgs/* $out/var/cache/
	'';
}
