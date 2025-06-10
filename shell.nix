{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    clang
    cmake
    gnumake
    pkg-config
  ];

  buildInputs = with pkgs;[
    readline.dev
    readline
    ncurses
  ];

  CC = "clang";
  CXX = "clang++";

  READLINE_INCLUDE_PATH = "${pkgs.readline.dev}/include";
  READLINE_LINK_PATH = "${pkgs.readline}/lib";
}
