# Building nonlibc

## Build with [Nix](https://nixos.org/nix/)

Nix takes care of all the dependencies; from the project top-level directory:

```bash
nix-build
```

If including `nonlibc` as a dependency for another nix package,
	extend `NIX_PATH` to include the parent directory of this repo
	using the `-I` flag.

Here is an example from the [memorywell](https://siriobalmelli.github.io/memorywell/)
	library, which depends on `nonlibc`:

```bash
cd ~
git clone https://github.com/siriobalmelli/nonlibc.git
git clone https://github.com/siriobalmelli/memorywell.git
cd memorywell
nix-build -I ~
```

## Build with [Meson](http://mesonbuild.com/index.html)

You will need to have a few dependencies installed on your system:

- [python3](https://www.python.org/)
- [meson](http://mesonbuild.com/Getting-meson.html)
- [ninja](https://ninja-build.org/)
- [pandoc](http://pandoc.org/)
- [cscope](http://cscope.sourceforge.net/)
- a suitable C compiler
	([gcc](https://gcc.gnu.org/) and [clang](https://clang.llvm.org/) are tested)

Run `./bootstrap.sh` - chances are things will work automagically and
	you'll find yourself with everything built and all tests running.

### I'm on Windows || It doesn't work

Don't despair; if you can get the dependencies above to install,
	things might still work with some manual twiddling:

```bash
meson --buildtype debugoptimized build-debug \
&& cd build-debug \
&& ninja test
```

... and if it still won't work, drop me a line at <sirio.bm@gmail.com>.

## I want to link my program against this library

Here's a few scenarios to get you acquainted:

### Using [Nix](https://nixos.org/nix/)

In your derivation, add a `nonlibc` dependency.

Here is an example from the `memorywell` library:

```nix
{ 	# deps
	system ? builtins.currentSystem,
	nixpkgs ? import <nixpkgs> { inherit system; },
	nonlibc ? import <nonlibc> { inherit system;
								inherit buildtype;
								inherit compiler;
								inherit lib_type;
								inherit dep_type;
								inherit mesonFlags;
	},
	# options
	buildtype ? "release",
	compiler ? "clang",
	lib_type ? "shared",
	dep_type ? "shared",
	mesonFlags ? ""
}:

with nixpkgs;

stdenv.mkDerivation rec {
	name = "memorywell";
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
	buildInputs = [
		nonlibc
	];

	# just work with the current directory (aka: Git repo), no fancy tarness
	src = ./.;

	# Override the setupHook in the meson nix derviation,
	# so that meson doesn't automatically get invoked from there.
	meson = pkgs.meson.overrideAttrs ( oldAttrs: rec {
		setupHook = "";
	});

	# build
	mFlags = mesonFlags
		+ " --buildtype=${buildtype}"
		+ " -Dlib_type=${lib_type}"
		+ " -Ddep_type=${dep_type}";

	configurePhase = ''
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
```

### Using [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)

```bash
./bootstrap.sh \
&& cd build-release \
&& ninja test \
&& sudo ninja install
```

To verify things are kosher, check if you get some useful output from:

```bash
pkg-config --modversion nonlibc
```

Et voilà - if your build system uses pkg-config it will see `nonlibc`
	and link against it.

If you're using [Meson](http://mesonbuild.com/index.html) to build your project,
	insert the following stanza in your `meson.build` file:

```meson
nonlibc = dependency('nonlibc')
```

... and then add the `dependencies : nonlibc` stanza to the `executable` declaration(s)
		in your `meson.build` file(s); e.g.:

```meson
executable('demo', 'test.c', dependencies : nonlibc)
```

## I want to statically include this library in my program

This is a very good idea:

-	The build system won't have to build
		[position-independent code](https://en.wikipedia.org/wiki/Position-independent_code)
		which will give a measurable speedup.
-	Compiler can discard unused objects and reduce memory footprint.

If you also build your project with [Meson](http://mesonbuild.com/index.html),
	I recommend you symlink the `nonlibc` repo into the `subprojects` directory:

```bash
pushd ~
git clone https://github.com/siriobalmelli/nonlibc.git
popd
mkdir -p subprojects
ln -s ~/nonlibc subprojects/
```

Add the following line to your toplevel `meson.build` file:

```meson
nonlibc = dependency('nonlibc', fallback : ['nonlibc', 'nonlibc_dep'])
```

-	Add the `dependencies : nonlibc` stanza to the `executable` declaration(s)
		in your `meson.build` file(s); e.g.:

```meson
executable('demo', 'test.c', dependencies : nonlibc)
```

## Cross-platform

Happily builds and tests cleanly on a 32-bit ARM machine running Debian.

We still depend on a bunch of `libc` symbols at link-time, though
	(oh, the irony).

## Cross-compilation

Cross compilation is tentatively tested for ARM so far,
	using [cross_arm.txt](toolchain/cross_arm.txt):

```bash
meson --cross-file=toolchain/cross_arm.txt --buildtype=plain build-arm
cd build-arm
ninja
```
