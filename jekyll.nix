{
  nixpkgs ? import (builtins.fetchGit {
    url = "https://siriobalmelli@github.com/siriobalmelli-foss/nixpkgs.git";
    ref = "refs/tags/sirio-2022-08-24";
    }) { },
  }:

with nixpkgs;

let
  rubyEnv = ruby.withPackages (p: with p; [
    github-pages
    jekyll
    jekyll-theme-minimal
  ]);

in
  stdenv.mkDerivation rec {
    name = "nonlibcDocumentation";

    buildInputs = [
      rubyEnv
    ];

    # work with the current directory. ignore all files not in version control
    src = if lib.inNixShell then null else nix-gitignore.gitignoreSource [] ./.;

    buildPhase = ''
        bundle exec jekyll build -d "$out"
    '';
    dontInstall = true;

    # scripts and executables served by Jekyll must run on systems that download them
    # which would be broken if their #! were patched to /nix/store/[derivation]
    dontStrip = true;
    dontMoveSbin = true;
    dontPatchELF = true;
    dontPatchShebangs = true;

    shellHook = ''
        rm -rf .bundle
        echo "# to build and serve locally, run:"
        echo "jekyll serve --watch"
    '';
}
