#!/bin/sh

source $stdenv/setup

PATH=$meson/bin:$python/bin:$PATH

tar -zxvf $src

cd nonlibc
meson --prefix=$out build
cd build
ninja
ninja install
#cp build-release-gcc/src/libnonlibc.so res/
#cp -r ./build $out

#DESTDIR=$out ninja install
