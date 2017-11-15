#!/bin/bash
# Compile and test all variants.
set -e

# remove existing builds
rm -rfv ./build-*

compilers="clang gcc"
build_types="debugoptimized release plain"

for cc in $compilers; do
	for type in $build_types; do
		name="build-${type}-${cc}"
		CC=$cc meson --buildtype=$type $name \
			&& pushd $name \
			&& ninja test
		popd
	done
done

# run valgrind
pushd ./build-debugoptimized-gcc \
	&& VALGRIND=1 meson test --wrap="valgrind --leak-check=full --show-leak-kinds=all"
popd

## build and test sanitizers
#sanitizers="b_sanitize=address b_sanitize=thread" # Nix's clang-4.0.1 has no tsan?
#for san in $sanitizers; do
#	type="debugoptimized"
#	name="build-${type}-${san##*=}"
#	CC=clang meson -D$san --buildtype=$type $name \
#		&& pushd $name \
#		&& VALGRIND=1 ninja test
#	popd
#done
