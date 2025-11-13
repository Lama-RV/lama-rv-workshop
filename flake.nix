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
    in rec {
      devShell = pkgs.mkShell {
        name = "lama-rv";
        src = ./.;
        packages = with pkgs; [
          qemu-user
          lamac
          gcc
          clang-tools
          cmake
          gdb
          glog

          pkgsRV.stdenv.cc
        ];
      };

      packages = {
        docker-minimal = pkgs.dockerTools.buildNixShellImage {
          drv = devShell;
          name = "ghcr.io/khaser/lama-rv";
          tag = "latest";
        };

        # TODO: docker-with-ssh-access desc
      };

    });
}

