---
title: NonlibC
order: 100
---

# nonlibc [![Build Status](https://travis-ci.org/siriobalmelli/nonlibc.svg?branch=master)](https://travis-ci.org/siriobalmelli/nonlibc)

Collection of standard-not-standard utilities for the discerning C programmer.

- [on github](https://github.com/siriobalmelli/nonlibc)
- [as a web page](https://siriobalmelli.github.io/nonlibc/)

## Yeah, but what does it do

The functions in this library solve or alleviate a bunch of commonplace
    problems/annoyances related to writing programs in C.

This is stuff not addressed in `libc` or `GLib`
    (or for which code therein might be overkill or disliked by the author).

The focus is on solving problems with a healthy dose of minimalism
    and performance (rarely orthogonal qualities, as it were).

This is not a hodgepodge - the code here is reliable
    and being used in the real world.

A quick rundown of the contents is available [here](docs/overview).

A good place to see implementation details is in the [test dir](test).

## [Building nonlibc](docs/building.md)

TLDR - use either:

1. `nix-build`
1. `meson`

## [Hacking Tips](docs/hacking_tips.md)

Covers the coding style and rationale.

## [docs about the docs](docs/documentation.md)

... covers how this documentation is structured and how to hack on it.

## [TODO list](docs/TODO.md)

All pending things go there.
