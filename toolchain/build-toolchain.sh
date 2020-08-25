#!/bin/bash
set -e

source "./toolchain-common.sh"

build_binutils () {
  pushd .
  cd "$BUILD"

  if [ "$1" == "edited" ]; then
    CONFIGURE_SCRIPT="$EDIT/$BINUTILS_FILE/configure"
    cd "$EDIT"
  else
    CONFIGURE_SCRIPT="$BUILD/$BINUTILS_FILE/configure"
    download-binutils
  fi

  printf "Configuring binutils...\n"
  mkdir -p "binutils-$BINUTILS_VERSION-build"
  cd "binutils-$BINUTILS_VERSION-build"
  "$CONFIGURE_SCRIPT" --prefix="$PREFIX" --target="$TARGET" --with-sysroot="$SYSROOT" --disable-nls --enable-shared || exit 1
  printf "Making binutils...\n"
  make -j "$NUM_JOBS" || exit 1
  printf "Installing binutils...\n"
  make install || exit 1
  printf "binutils installed!\n"
  cd ..

  popd
}

build_gcc () {
  pushd .
  cd "$BUILD"

  if [ "$1" == "edited" ]; then
    CONFIGURE_SCRIPT="$EDIT/$GCC_FILE/configure"
    cd "$EDIT"
  else
    CONFIGURE_SCRIPT="$BUILD/$GCC_FILE/configure"
    download-gcc
  fi

  printf "Configuring gcc...\n"
  mkdir -p "gcc-$GCC_VERSION-build"
  cd "gcc-$GCC_VERSION-build"
  "$CONFIGURE_SCRIPT" --prefix="$PREFIX" --target="$TARGET" --disable-nls --enable-languages=c,c++ --with-sysroot="$SYSROOT" --with-newlib --enable-shared || exit 1
  printf "Making gcc...\n"
  make -j "$NUM_JOBS" all-gcc all-target-libgcc || exit 1
  printf "Installing gcc...\n"
  make install-gcc install-target-libgcc || exit 1
  printf "gcc installed!\n"
  printf "Making libstdc++...\n"
  make -j "$NUM_JOBS" all-target-libstdc++-v3 || exit 1
  printf "Installing libstdc++...\n"
  make install-gcc install-target-libstdc++-v3
  printf "libstdc++ installed!\n"
  cd ..

  popd
}

mkdir -p "$BUILD"

if [ "$1" ]; then
  if [ "$1" == "edited-binutils" ]; then
    build_binutils edited
  elif [ "$1" == "edited-gcc" ]; then
    build_gcc edited
  else
    printf "Unknown argument %s. Please pass either edited-binutils or edited-gcc to rebuild the respective repo from the edit directory.\n" "$1"
    exit
  fi
  BUILT_SOMETHING="YES"
else
  if [ ! -d "$PREFIX" ]; then
    printf "Building binutils and gcc...\n"
    build_binutils
    build_gcc
    BUILT_SOMETHING="YES"
  fi
fi

if [ ! "$BUILT_SOMETHING" ]; then
  printf "There's nothing to build.\n"
  printf "To rebuild binutils & gcc, delete %s.\n" "$PREFIX"
  exit 1
else
  printf "Done! The toolchain (%s) is installed at %s and the sysroot is at %s.\n" "$TARGET" "$PREFIX" "$SYSROOT"
  printf "You can safely delete %s if you don't want the cached toolchain sources anymore.\n" "$BUILD"
fi
