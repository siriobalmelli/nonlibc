{ system ? builtins.currentSystem, buildtype ? "debug", compiler ? "gcc" }:

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
	mesonBuildType = "${buildtype}"; 
	cc = "${compiler}";
	meson = pkgs.meson.overrideAttrs ( oldAttrs: rec {
		setupHook = ./mesonHook.sh;
	});
	buildPhase = "
		ninja test
		ninja install";
}
