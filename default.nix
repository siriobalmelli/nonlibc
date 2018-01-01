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
	builder = ./builder.sh;
	src = nix-prefetch-git {
		url = 'https://github.com/TonyTheLion/nonlibc.git';
		rev = 'refs/heads/sirio';
	}; 
	inherit meson;
}
