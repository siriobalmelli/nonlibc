with import <nixpkgs> {};

{ stdenv, cscope, pandoc, gcc, clang, clang-tools, meson, ninja, which, valgrind,
python3, sudo }:

stdenv.mkDerivation rec {
	name = "nonlibc";
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
	buildPhase = "
		ninja
		ninja install";
	inherit meson;
}
