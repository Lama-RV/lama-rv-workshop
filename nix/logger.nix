{ stdenv, fetchFromGitHub, ocaml, ocamlPackages, opam-installer }:
stdenv.mkDerivation rec {
  name = "logger-p5";
  version = "dev";

  src = fetchFromGitHub {
    owner = "dboulytchev";
    repo = "logger";
    rev = "7ad8762dd522f9621dd0bdf6d6c8a534464d6934";
    sha256 = "sha256-uspwU4J/jRhEM7Iy/jce83VITpYqNX9ZY3BePH3QzEE=";
  };

  nativeBuildInputs = [ ocaml ocamlPackages.findlib opam-installer ];
  buildInputs = with ocamlPackages; [
    cmdliner
    ocamlbuild
    opam-file-format
    pcre2
    camlp5
  ];
  buildPhase = "make -j -f Makefile.ob";
  installPhase = ''
    mkdir -p $out/lib/ocaml/${ocaml.version}/site-lib
    BUNDLEDIR=$out make -f Makefile.ob install
  '';

  preInstall = "export PREFIX=$out";
}
