{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    qemu_full
    xorriso
    coreboot-toolchain.x64
  ];
}
