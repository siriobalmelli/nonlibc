with import <nixpkgs> {};

stdenv.mkDerivation rec {
	name = "env";
	env = buildEnv { name = name; paths = nativeBuildInputs; };
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
}
