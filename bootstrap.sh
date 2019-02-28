#!/bin/bash
# Compile and test all variants.
#+	... if you're missing tools (or running a blank/vanilla system),
#+	see https://github.com/siriobalmelli/toolbench
FAILS=

# remove existing builds
rm -rfv ./build-*

compilers="clang gcc"
build_types="debug debugoptimized release plain"

for cc in $compilers; do
	if ! which $cc; then break; fi

	for type in $build_types; do
		name="build-${type}-${cc}"
		CC=$cc meson --buildtype=$type $name \
			&& pushd $name \
			&& ninja test
		# log if build didn't succeed
		poop=$?
		if (( $poop )); then
			FAILS="${FAILS}$name\n"
		fi
		# out regardless
		popd >/dev/null 2>&1
	done
done


# Index codebase
cscope -b -q -U -I ./include -s ./src -s ./util -s ./test

# the use of a "core" suite:
# - limits the valgrind run to the "shared" tests only (avoiding static builds)
# - avoids application/util tests which are written e.g. in Python and would
#   break valgrind hard
pushd ./build-debugoptimized-gcc \
	&& meson test --suite="core" \
		--wrap="valgrind \
			--leak-check=full --show-leak-kinds=all \
			--track-origins=yes --error-exitcode=1"
popd

## build and test sanitizers
# TODO: wait for clang 4.0.1 bugfix in nix repos, or go with 5.0.0?
#sanitizers="b_sanitize=address b_sanitize=thread" # Nix's clang-4.0.1 has no tsan?
#for san in $sanitizers; do
#	type="debugoptimized"
#	name="build-${type}-${san##*=}"
#	CC=clang meson -D$san --buildtype=$type $name \
#		&& pushd $name \
#		&& VALGRIND=1 ninja test
#	popd
#done

if [[ $FAILS ]]; then
	# flush current stdout (if user is watching all logs)
	echo -ne "\n\n"
	# output fail list
	echo -e "FAILS:\n$FAILS" >&2
	exit 1
else
	exit 0
fi
