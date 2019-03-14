---
title: TODO
order: 0
---

# BUGS

-	ncp failing on 32-bit ARM systems

# TODO

These are in order by priority:

-	move compiled elements to separate libraries, become header-only ?
-	provide users with a single (generated) 'nonlibc' include
-	faux-namespace ... give symbols a 'nc_' prefix?
-	man pages
-	overloaded size_t functions for nmath
-	add library to WrapDB; submit to Meson site for inclusion
-	integrate code coverage testing
-	code optional replacements for required libc calls (e.g.: on ARM) ?
-	*view* any source files linked in the documentation (with syntax highlight),
		don't try and download it (facepalm)
-	Move option parsing to Argp ... which likely means meson-wrapping it
		since it doesn't come standard with LibC on all systems?!
-	Test functions for `posigs`
-	Handle generated headers in YCM clang completer?
-	compilation: ThinLTO
- look at [systemd](https://github.com/systemd/systemd) for code quality, Meson usage, packaging, etc
- also look at <https://github.com/BytePackager/packagecore> with example <https://github.com/jarun/nnn/blob/master/packagecore.yaml>

## TODO: lifo

-	slow on Darwin ;(
		(lack of mremap getting me down)

## TODO: bootstrap.py

-	document
-	building on windows

## TODO: man pages

-	nmem(3)
-	fnv(3)
-	n_dirname(3); n_basename(3); n_join(3)
