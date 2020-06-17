#!/bin/bash
set -e

printf "Building duckOS toolchain...\n"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TARGET="i686-pc-duckos"
PREFIX="$DIR/tools"
BUILD="$DIR/build"
SYSROOT="$DIR/sysroot"
NUM_JOBS=$(nproc)

BINUTILS_VERSION="2.34"
GCC_VERSION="9.3.0"

BINUTILS_FILE="binutils-$BINUTILS_VERSION"
GCC_FILE="gcc-$GCC_VERSION"

LIBC_FILE="duckOS-newlib"

BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/$BINUTILS_FILE.tar.gz"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/$GCC_FILE.tar.gz"
LIBC_URL="https://github.com/byteduck/duckOS-newlib"

mkdir -p "$BUILD"

if [ -d "$PREFIX" ]; then
  printf "Toolchain already built. Delete %s to build them again. Continuing in 3..." "$PREFIX"
  sleep 1
  printf "2..."
  sleep 1
  printf "1..."
  sleep 1
  printf "\n"
else

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
  "../$BINUTILS_FILE/configure" --prefix="$PREFIX" --target="$TARGET" --with-sysroot="$SYSROOT" --disable-nls --enable-shared || exit 1
  printf "Making binutils...\n"
  make -j "$NUM_JOBS" || exit 1
  printf "Installing binutils...\n"
  make install || exit 1
  printf "binutils installed!\n"
  cd ..

  printf "Configuring gcc...\n"
  mkdir -p "gcc-$GCC_VERSION-build"
  cd "gcc-$GCC_VERSION-build"
  "../$GCC_FILE/configure" --prefix="$PREFIX" --target="$TARGET" --disable-nls --enable-languages=c,c++ --with-sysroot="$SYSROOT" --with-newlib --enable-shared || exit 1
  printf "Making gcc...\n"
  make -j "$NUM_JOBS" all-gcc all-target-libgcc || exit 1
  printf "Installing gcc...\n"
  make install-gcc install-target-libgcc || exit 1
  printf "gcc installed!\n"
  cd ..

fi

if [ -d "$SYSROOT/usr" ]; then
  printf "libc already built. Delete %s to build it again.\n" "$SYSROOT"
  exit
else
  cd "$BUILD"

  if [ ! -d "$LIBC_FILE" ]; then
    printf "Downloading libc...\n"
    git clone "$LIBC_URL" > /dev/null
    printf "Libc downloaded!\n"
  fi

  export PATH="$PREFIX/bin":$PATH
  mkdir -p "$SYSROOT"
  printf "Configuring libc...\n"
  mkdir -p libc-build
  cd libc-build
  "../$LIBC_FILE/configure" --prefix="/usr" --target="i686-pc-duckos" || exit 1
  printf "Making libc...\n"
  make -j "$NUM_JOBS" all || exit 1
  printf "Installing libc...\n"
  make DESTDIR="${SYSROOT}" install || exit 1
  mv "${SYSROOT}/usr/i686-pc-duckos/"* "${SYSROOT}/usr" || exit 1
  rmdir "${SYSROOT}/usr/i686-pc-duckos" || exit 1
fi

printf "Done! The toolchain (%s) is installed at %s and the sysroot is at %s.\n" "$TARGET" "$PREFIX" "$SYSROOT"
