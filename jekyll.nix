with import <nixpkgs> { };

let jekyll_env = bundlerEnv rec {
    name = "jekyll_env";
    ruby = ruby_2_5;
    gemfile = ./Gemfile;
    lockfile = ./Gemfile.lock;
    gemset = ./gemset.nix;
  };
in
  stdenv.mkDerivation rec {
    name = "jekyll_env";
    buildInputs = [ jekyll_env ];

    src = if lib.inNixShell then null else nix-gitignore.gitignoreSource [] ./.;

    # build will fail because template will attempt to look up github links
    installPhase = ''
      bundle exec jekyll build
    '';

    shellHook = ''
      exec ${jekyll_env}/bin/jekyll serve --watch
    '';
}
