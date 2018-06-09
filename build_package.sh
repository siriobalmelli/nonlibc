#!/bin/bash

# create the package
 fpm -s dir -t deb -n nonlibc -v 0.2.2 \
	 src/libnonlibc.a=/usr/lib/x86_64-linux-gnu/libnonlibc.a \
	 src/libnonlibc.so=/usr/lib/x86_64-linux-gnu/libnonlibc.so \
	 util/fnvsum=/usr/local/bin/fnvsum \
	 util/ncp=/usr/local/bin/ncp
