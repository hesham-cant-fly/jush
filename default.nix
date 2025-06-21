{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    clang
    cmake
    gnumake
    pkg-config
  ];

  buildInputs = with pkgs; [
    ncurses
    readline
    readline.dev
  ];

  PKG_CONFIG_PATH = with pkgs; "${readline.dev}/lib/pkgconfig";

  CC = "clang";
  CXX = "clang++";
}

