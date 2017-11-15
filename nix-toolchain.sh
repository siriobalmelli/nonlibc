#!/bin/bash
# Set up nix toolchain; run bootstrap
set -e

# make sure there is nix
if ! type nix-env; then
	bash <(curl https://nixos.org/nix/install)
	
	# re-source files as opposed to running nix.sh
	#+	because on macOS it's nix-daemon.sh or some such
	#+	and I still have no clue what I'm doing with Nix :P
	re_source="/etc/bashrc $HOME/.bashrc $HOME/.bash_profile"
	for f in $re_source; do
		if [[ -e "$f" ]]; then source "$f"; fi
	done

fi

nix-shell --pure --run "./bootstrap.sh"
