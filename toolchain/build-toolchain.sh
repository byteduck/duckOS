#!/bin/bash
set -e

printf "Building duckOS toolchain...\n"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TARGET="i686-pc-duckos"
PREFIX="$DIR/tools"
BUILD="$DIR/build"
NUM_JOBS=$(nproc)

BINUTILS_VERSION="2.34"
GCC_VERSION="9.3.0"

BINUTILS_FILE="binutils-$BINUTILS_VERSION"
GCC_FILE="gcc-$GCC_VERSION"

BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/$BINUTILS_FILE.tar.gz"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/$GCC_FILE.tar.gz"

if [ -d "$PREFIX/bin" ]; then
  printf "Toolchain already built. Delete %s if you'd like to rebuild it.\n" "$PREFIX"
  exit
fi

mkdir -p "$BUILD"
cd "$BUILD"

if [ ! -d "$BINUTILS_FILE" ]; then
  printf "Downloading binutils %s...\n" "$BINUTILS_VERSION"
  curl "$BINUTILS_URL" > "$BINUTILS_FILE.tar.gz"
  printf "Extracting binutils...\n"
  tar -xzf "$BINUTILS_FILE.tar.gz"
  rm "$BINUTILS_FILE.tar.gz"

  cd "$BINUTILS_FILE"

  printf "Patching binutils...\n"
  patch -p1 < "$DIR/binutils-$BINUTILS_VERSION.patch" > /dev/null
  printf "binutils patched!\n"

  cd ..
fi

if [ ! -d "$GCC_FILE" ]; then
  printf "Downloading gcc %s...\n" "$GCC_VERSION"
  curl "$GCC_URL" > "$GCC_FILE.tar.gz"
  printf "Extracting gcc...\n"
  tar -xzf "$GCC_FILE.tar.gz"
  rm "$GCC_FILE.tar.gz"

  cd "$GCC_FILE"

  printf "Patching gcc...\n"
  patch -p1 < "$DIR/gcc-$GCC_VERSION.patch" > /dev/null
  printf "gcc patched!\n"

  cd ..
fi

printf "Configuring binutils...\n"
mkdir -p "binutils-$BINUTILS_VERSION-build"
cd "binutils-$BINUTILS_VERSION-build"
"../$BINUTILS_FILE/configure" --prefix="$PREFIX" --target="$TARGET" --with-sysroot --disable-nls
printf "Making binutils...\n"
make -j "$NUM_JOBS" || exit 1
printf "Installing binutils...\n"
make install || exit 1
printf "binutils installed!\n"
cd ..

printf "Configuring gcc...\n"
mkdir -p "gcc-$GCC_VERSION-build"
cd "gcc-$GCC_VERSION-build"
"../$GCC_FILE/configure" --prefix="$PREFIX" --target="$TARGET" --disable-nls --enable-languages=c,c++ --without-headers
printf "Making gcc...\n"
make -j "$NUM_JOBS" all-gcc all-target-libgcc || exit 1
printf "Installing gcc...\n"
make install-gcc install-target-libgcc || exit 1
printf "gcc installed!\n"

printf "Done! The toolchain (%s) is installed at %s.\n" "$TARGET" "$PREFIX"
