#!/bin/bash
# Set up nix toolchain; run bootstrap
set -e

# make sure there is nix
if ! type nix-env; then
	bash <(curl https://nixos.org/nix/install)

	# don't bother re-sourcing blah blah (it's different on macOS);
	#+	just fire a new shell, wherein we expect nix to work.
	bash -l -c 'nix-shell --pure --run "./bootstrap.sh"'
else
	nix-shell --pure --run "./bootstrap.sh"
fi
