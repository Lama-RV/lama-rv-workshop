{ buildDunePackage, posix-base, unix-errno }:

buildDunePackage {
  pname = "posix-uname";

  inherit (posix-base) version src;

  minimalOCamlVersion = "4.12";

  propagatedBuildInputs = [ posix-base unix-errno ];

}
