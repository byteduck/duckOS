#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LIBC_LOC=$(realpath "$DIR"/../libraries/libc)
LIBM_LOC=$(realpath "$DIR"/../libraries/libm)
KERNEL_LOC=$(realpath "$DIR"/../kernel)
TARGET="i686-pc-duckos"
PREFIX="$DIR/tools"
BUILD="$DIR/../cmake-build"
EDIT="$DIR/edit"
SYSROOT="$DIR/../cmake-build/root"
NUM_JOBS=$(( $(nproc) / 2 ))

source "$DIR/../scripts/duckos.sh"

GNU_MIRROR="https://mirrors.ocf.berkeley.edu/gnu"

BINUTILS_VERSION="2.34"
BINUTILS_FILE="binutils-$BINUTILS_VERSION"
BINUTILS_URL="$GNU_MIRROR/binutils/$BINUTILS_FILE.tar.gz"

GCC_VERSION="9.3.0"
GCC_FILE="gcc-$GCC_VERSION"
GCC_URL="$GNU_MIRROR/gcc/gcc-$GCC_VERSION/$GCC_FILE.tar.gz"

SYS_NAME="$(uname -s)"
INSTALL_BIN="install"

if [ "$SYS_NAME" = "Darwin" ]; then
  INSTALL_BIN="ginstall"
fi

download-binutils () {
  if [ ! -d "$BINUTILS_FILE" ]; then
    msg "Downloading binutils $BINUTILS_VERSION..."
    curl "$BINUTILS_URL" > "$BINUTILS_FILE.tar.gz" || exit 1
    msg "Extracting binutils..."
    tar -xzf "$BINUTILS_FILE.tar.gz" || exit 1
    rm "$BINUTILS_FILE.tar.gz"

    cd "$BINUTILS_FILE" || exit 1
    msg "Patching binutils..."
    if [ "$1" == "use-git" ]; then
      git init . > /dev/null || exit 1
      git add -A > /dev/null || exit 1
      git commit -m "First commit" > /dev/null || exit 1
      git apply "$DIR/binutils-$BINUTILS_VERSION.patch" > /dev/null || exit 1
    else
      patch -p1 < "$DIR/binutils-$BINUTILS_VERSION.patch" > /dev/null || exit 1
    fi
    success "Pathed binutils!"
    cd ..
  else
    msg "Already downloaded binutils."
  fi
}

download-gcc () {
  if [ ! -d "$GCC_FILE" ]; then
    msg "Downloading gcc $GCC_VERSION..."
    curl "$GCC_URL" > "$GCC_FILE.tar.gz" || exit 1
    msg "Extracting gcc..."
    tar -xzf "$GCC_FILE.tar.gz" || exit 1
    rm "$GCC_FILE.tar.gz"

    cd "$GCC_FILE" || exit 1
    msg "Patching gcc..."
    if [ "$1" == "use-git" ]; then
      git init . > /dev/null || exit 1
      git add -A > /dev/null || exit 1
      git commit -m "First commit" > /dev/null || exit 1
      git apply "$DIR/gcc-$GCC_VERSION.patch" > /dev/null || exit 1
    else
      patch -p1 < "$DIR/gcc-$GCC_VERSION.patch" > /dev/null || exit 1
    fi
    success "Patched gcc!"
    cd ..
  else
    msg "Already downloaded gcc."
  fi
}
