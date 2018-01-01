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
	src = fetchgit {
	  url = "https://github.com/TonyTheLion/nonlibc.git";
	  rev = "e0ae6b9da45e355f5454174f630b097775c1c5a2";
	  sha256 = "1pwkiq2vs0lzkd4xww2wj39q7r0cj88gvdghy3hzh4246n9za7fl";
	  fetchSubmodules = true;
	}; 
	inherit meson;
}
