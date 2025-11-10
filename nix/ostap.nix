{ stdenv, buildDunePackage, fetchFromGitHub, ocaml, ocamlPackages, GT }:
buildDunePackage rec {
  pname = "ostap";
  version = "0.6";

  src = fetchFromGitHub {
    owner = "PLTools";
    repo = "ostap";
    rev = "b429ebbf001c9b253fd46e4a42f66698c581665e";
    sha256 = "sha256-Vo35Vn3/f7vwL/ZBG1BUeFf/cCT5SMu99XrZTy7kmCE=";
  };

  propagatedBuildInputs = with ocamlPackages; [ 
    camlp5
    re
    findlib
    re
    GT
    dune-configurator 
  ];

  nativeBuildInputs = with ocamlPackages; [
    camlp5
  ];
}
