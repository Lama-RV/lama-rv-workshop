{
  pkgs,
}:
let
  buildDunePackage = pkgs.ocamlPackages.buildDunePackage;
  logger-p5 = pkgs.callPackage ./logger.nix {};
  posix-uname = (pkgs.ocamlPackages.callPackage ./posix-uname.nix {});
  GT = (pkgs.callPackage ./GT.nix { inherit buildDunePackage logger-p5; } );
  ostap = (pkgs.callPackage ./ostap.nix { inherit buildDunePackage GT; });
in
buildDunePackage rec {
  pname = "Lama";
  version = "1.3.0";

  minimalOCamlVersion = "4.10";

  src = pkgs.fetchFromGitHub {
    owner = "PLTools";
    repo = "Lama";
    rev = "399629f379a7a93cdeb22bebec2581f5fca12920";
    hash = "sha256-Y9xT+DesXjwobs9PSR6Ta08SIuuhGckP8XL9fqqaSfc=";
  };

  buildInputs = [
    GT
    ostap
    posix-uname
    pkgs.ocamlPackages.pcre2
  ];

  nativeBuildInputs = with pkgs; [
    gcc_multi
    pkgs.ocamlPackages.camlp5
  ];

  patchPhase = ''
    sed -i \
        -e "s#opam var share#echo $out/share#" \
        -e "s#git rev-parse --abbrev-ref HEAD#echo ${version}#" \
        -e "s#git rev-parse --short HEAD#echo ${pkgs.lib.strings.substring 0 8 src.rev}#" \
        -e "s#git rev-parse --verify HEAD#echo ${src.rev}#" \
        -e "s#git show --no-patch --no-notes --pretty='%cd'#echo 1970-01-01 03:00:00 +0300#" \
        src/dune
    find ./ -name Makefile -exec sed -i -e "s#/bin/bash#${pkgs.bash}/bin/bash#" {} \;
  '';

}
