{ stdenv, buildDunePackage, fetchFromGitHub, ocaml, ocamlPackages, gnum4, logger-p5 }:
buildDunePackage rec {
  pname = "GT";
  version = "0.5.3";

  src = fetchFromGitHub {
    owner = "PLTools";
    repo = "GT";
    rev = "16389d2eee05e3b4f13fb8aec0491bc053e4fefd";
    sha256 = "sha256-gsRhk1jF943Ijj0rKyuyTnkPNgOkqASYR/Zno0Air54=";
  };

  nativeBuildInputs = with ocamlPackages; [
    camlp5
    pcre2
    gnum4
    odoc
    odig
  ];

  propagatedBuildInputs = with ocamlPackages; [
    camlp5
    pcre2
    ppxlib
    ocamlgraph
    ocamlbuild
    ppx_inline_test
    dune-configurator
    logger-p5
    bisect_ppx
  ];
}
