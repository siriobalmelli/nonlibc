##
# nonlibc makefile
#
# 2022 Sirio Balmelli
##
PROJECT := nonlibc
VERSION := $(shell cat ./VERSION)

# errors suppressed for use on non-RPM platforms
ARCH := $(shell rpm --eval '%{_arch}' 2>/dev/null)
FEDORA := fc$(shell rpm --eval '%{fedora}' 2>/dev/null)
TARBALL := build-release/meson-dist/$(PROJECT)-$(VERSION).tar.xz
RPM := rpmbuild/RPMS/$(ARCH)/$(PROJECT)-$(VERSION)-1.$(FEDORA).$(ARCH).rpm
SRPM := rpmbuild/SRPMS/$(PROJECT)-$(VERSION)-1.$(FEDORA).src.rpm

##
# In addition to these phony targets,
# any valid meson --buildtype is a valid target,
# see pattern rules section below.
##
.PHONY: default help all debug check test valgrind clean install uninstall dist

# if no arguments are given, builds 'release'
default: release

help:
	@printf "\
make help	::	prints this help message	\n\
\n\
make		::	builds 'build-release'		\n\
make install	::	installs 'build-release'	\n\
make uninstall	::	uninstalls 'build-release'	\n\
\n\
make clean	::	removes all build artifacts	\n\
make all	::	makes 'release' and 'debug'	\n\
\n\
make release	::	builds 'build-release'		\n\
make debug	::	builds 'build-debugoptimized'	\n\
make check	::	tests 'release' and 'debug'	\n\
make valgrind	::	tests 'debug' under valgrind	\n\
\n\
make dist	::	build RPMs			\n\
"

all: release debugoptimized

debug: debugoptimized

check: check-release check-debugoptimized

test: check

# The use of a "core" suite:
# - Limits the valgrind run to the "shared" tests only (avoiding static builds).
# - Avoids application/util tests which are written e.g. in Python and would break valgrind hard.
valgrind: build-debugoptimized
	meson test -C $< \
		--suite="core" \
		--wrap="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1"

clean:
	rm -rf build-*
	rm -rf rpmbuild
	rm -rf dist
	rm -fv vgcore*
	rm -fv result
	rm -fv test.yaml

install: build-release
	ninja -C $< -v install

uninstall: build-release
	ninja -C $< -v uninstall

dist: dist_checks $(TARBALL) $(RPM) $(SRPM)
	rm -rf dist
	mkdir -p dist
	cp -f $(TARBALL) dist/
	cp -f $(RPM) dist/
	cp -f $(SRPM) dist/

# safety checks to be able to dist
dist_checks:
	# builds only tested on Fedora at the moment
	grep -q 'Fedora release' /etc/redhat-release
	# current VERSION value _must_ be a valid git tag
	git log -1 "v$(VERSION)"
	# tip of master should match the tag in git
	diff \
		<(git show-ref --dereference "v$(VERSION)" | tail -n 1 | cut -d ' ' -f 1) \
		<(git show-ref $$(git branch --show-current) | head -n 1 | cut -d ' ' -f 1)

$(TARBALL): build-release
	meson dist -C $<

##
# refs:
# https://www.redhat.com/sysadmin/create-rpm-package
# https://docs.fedoraproject.org/en-US/packaging-guidelines/Meson/
# http://ftp.rpm.org/max-rpm/
##
$(RPM) $(SRPM): $(TARBALL)
	for d in BUILD RPMS SOURCES SPECS SRPMS; do mkdir -p rpmbuild/$$d; done
	cp -f $< rpmbuild/SOURCES/
	cp -f packaging/$(PROJECT).spec rpmbuild/SPECS/
	VERSION=$(VERSION) rpmlint \
		rpmbuild/SPECS/$(PROJECT).spec
	VERSION=$(VERSION) QA_RPATHS=0x0001 rpmbuild \
		  --define "_topdir `realpath ./rpmbuild`" \
		  -ba rpmbuild/SPECS/$(PROJECT).spec

##
# pattern rules
##
# 'build-release' -> 'meson setup --buildtype=release build-release'
build-%:
	meson setup --buildtype=$* $(@)

# 'release' -> 'ninja -C build-release'
%: build-%
	ninja -C $<

# check-release -> 'meson test -C build-release'
check-%: build-% | %
	meson test -C $<
