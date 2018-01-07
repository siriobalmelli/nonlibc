{ system ? builtins.currentSystem, buildtype ? "release", compiler ? "gcc" }:

with import <nixpkgs> { inherit system; };

stdenv.mkDerivation rec {
	name = "nonlibc-${buildtype}-${compiler}";
	env = buildEnv { name = name; paths = nativeBuildInputs; };
	outputs = [ "out" ];
	nativeBuildInputs = [
		cscope
		pandoc
		(lowPrio gcc)
		clang
		clang-tools
		ninja
		meson
		which
		valgrind
		python3
	];
	src = ./.;
	# Override the setupHook in the meson nix derviation,
	# so that meson doesn't automatically get invoked from there.
	meson = pkgs.meson.overrideAttrs ( oldAttrs: rec {
		setupHook = "";
	});
	configurePhase = ''
        	mesonFlags="--prefix=$out $mesonFlags"
    		mesonFlags="--buildtype=${buildtype} $mesonFlags"
		echo $mesonFlags
    		CC=${compiler} meson build $mesonFlags
		cd build
		''; 

	buildPhase = '' 
		ninja test
		ninja install
		'';
}
