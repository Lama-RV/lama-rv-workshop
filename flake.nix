{
  description = "Lama RISC-V backend flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-25.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem ( system:
    let
      pkgs = import nixpkgs { inherit system; };
      lamac = import ./nix/lamac.nix { inherit pkgs; };
      pkgsRV = pkgs.pkgsCross.riscv64;
    in {
      devShell = pkgs.mkShell {
        name = "lama-rv";

        packages = with pkgs; [
          qemu
          lamac
          gcc
          clang-tools
          cmake
          gdb
          fmt_11
          glog

          pkgsRV.stdenv.cc
        ];
      };
    });
}

