# Nonlibc - module overview

This document gives a quick overview of each module/sub-library
	included in nonlibc.

## fast, reliable 32-bit AND 64-bit hashing - [fnv.h](include/fnv.h)

If you need a non-crypto hash, odds are you want [FNV1A](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function).

This is the simplest, least-hassle implementation I know of.

Usage examples in [fnv_test.c](test/fnv_test.c).

An `fnvsum` utility is also provided for hashing files (or stdin),
	on the pattern of `md5sum`.
See the [fnvsum man page](man/fnvsum.md)

If you would like to use `fnvsum` without installing:

```bash
$ ./bootstrap.py
$ # hash stdin:
$ echo -n '' | build-release/util/fnvsum
cbf29ce484222325  -
$ # hash a file:
$ build-release/util/fnvsum /path/to/some/file
```

## straightforward integer math - [nmath.h](include/nmath.h)

Sometimes what you DON'T want is floating-point math.

This library has sane implementations for:

-	integer ceiling division (i.e.: if not evenly divisible, return +1)
-	get the next power of 2 (or current number if a power of 2)
-	return the next higher multiple of a number (or itself if evenly divisible)
-	get the position of the lowest '1' bit in an integer

Oh, and they're all inlines so your compiler can reason about local variables
	and turn out decent assembly.

Every function is shown used in [nmath_test.c](test/nmath_test.c)

## generating and parsing hexadecimal strings vis-a-vis integers and bytefields

That's done by [hx2b.h](include/hx2b.h) and [b2hx.h](include/b2hx.h).

Check out [hex2bin2hex_test.c](test/hex2bin2hex_test.c) for the full monte.

## proper RNG which isn't a hassle to set-up and use - [pcg_rand.h](include/pcg_rand.h)

This is a simple, clean, fast implementation of [PCG](http://www.pcg-random.org/).

That matters more than you may think - and anyways this API is simple and clean.

Check out the test code at [pcg_rand_test.c](test/pcg_rand_test.c).

## a very fast growing LIFO - [lifo.h](include/lifo.h)

The fastest possible way I know to push/pop pointer-sized values onto a stack,
	which grows (reallocates memory) as necessary,
	and can be kept around between function calls.

## accurately timing blocks of code

Use the `nlc_timing_start()` and `nlc_timing_stop()` macros in [nonlibc.h](include/nonlibc.h).

See e.g.: [fnv_test.c](test/fnv_test.c) for usage examples.

## Control flow; print mechanics - [zed_dbg.h](include/zed_dbg.h)

This header helps with the following annoyances:

-	forgetting to insert a '\n' at the end of an impromptu
		debug `printf` statement
-	spending time commenting/uncommenting print statements,
		no easy way to enable/disable entire categories
		or sets of prints
-	`printf`s inside conditionals increase code size
		for no good reason
-	program throws an error, but forgot to write a print there
-	program throws an error and does do a print,
		but you have to `grep` for it
		because you don't remember which source file it was in
-	kryptonite print statements full of CPP kudge
		like `__FILE__` and `__LINE__`
-	complex control flow in functions handling external state
		e.g. malloc(), open(), fork()
-	time spent making print output marginally legible so the Dev Ops people
		don't poison your coffee
-	needing to pretty-print large arrays of bytes in hex,
		for post-mortem analysis

Some usage examples are in [zed_dbg_test.c](test/zed_dbg_test.c);
	[zed_dbg.h](include/zed_dbg.h) itself is fairly well commented also.

## Zero-copy I/O - [nmem.h](include/nmem.h)

A memory allocator which handles zero-copy I/O behind-the-scenes.

An example usage is the [ncp](util/ncp.c) utility provided by this library.

`ncp` re-implements the system `cp` command using this library for zero-copy I/O;
	and is **>20%** faster than `cp` for large files.

## other odds-and-ends

-	visibility and inlining macros in [nonlibc.h](include/nonlibc.h)
-	some lock-free/atomics in [atop.h](include/atop.h)

