{
  nixpkgs ? <nixpkgs>,
  nonlibcSrc ? { outPath = ./.; rev = 0;},
  officialRelease ? false,
  systems ? [ "i686-linux" "x86_64-linux" ] 
}:
let 
   pkgs = import nixpkgs { };
   jobs = {
   	nonlibc = pkgs.callPackage ./default.nix {}; 
   };
in jobs

