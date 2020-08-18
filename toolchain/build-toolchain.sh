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
  # Note to self: Use --enable-default-pie to enable dynamic linking by default
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

build_newlib () {
  pushd .
  cd "$BUILD"

  export PATH="$PREFIX/bin":$PATH

  if [ "$1" == "edited" ]; then
    CONFIGURE_SCRIPT="$EDIT/$NEWLIB_FILE/configure"
    cd "$EDIT"
  else
    CONFIGURE_SCRIPT="$BUILD/$NEWLIB_FILE/configure"
    download-newlib
  fi

  mkdir -p "$SYSROOT"
  printf "Configuring newlib...\n"
  mkdir -p "$BUILD/newlib-$NEWLIB_VERSION-build"
  cd "$BUILD/newlib-$NEWLIB_VERSION-build"
  "$CONFIGURE_SCRIPT" --prefix="/usr" --target="i686-pc-duckos" || exit 1
  printf "Making newlib...\n"
  make -j "$NUM_JOBS" all || exit 1
  printf "Installing newlib...\n"
  if [ -d "${SYSROOT}/usr" ]; then
    rm -rf "${SYSROOT:?}/usr" # Don't wanna accidentally delete /usr :)
  fi
  make DESTDIR="${SYSROOT}" install || exit 1
  mv "${SYSROOT}/usr/i686-pc-duckos/"* "${SYSROOT}/usr" || exit 1
  rmdir "${SYSROOT}/usr/i686-pc-duckos" || exit 1

  popd
}

mkdir -p "$BUILD"

if [ "$1" ]; then
  if [ "$1" == "edited-binutils" ]; then
    build_binutils edited
  elif [ "$1" == "edited-gcc" ]; then
    build_gcc edited
  elif [ "$1" == "edited-newlib" ]; then
    build_newlib edited
  else
    printf "Unknown argument %s. Please pass either edited-binutils, edited-gcc, or edited-newlib to rebuild the respective repo from the edit directory.\n" "$1"
    exit
  fi
  BUILT_SOMETHING="YES"
else
  if [ ! -d "$PREFIX" ]; then
    printf "Building binutils...\n"
    build_binutils
    build_gcc
    BUILT_SOMETHING="YES"
  fi

  if [ ! -d "$SYSROOT/usr" ]; then
    printf "Building newlib...\n"
    build_newlib
    BUILT_SOMETHING="YES"
  fi
fi

if [ ! "$BUILT_SOMETHING" ]; then
  printf "There's nothing to build.\n"
  printf "To rebuild binutils & gcc, delete %s.\n" "$PREFIX"
  printf "To rebuild libc (newlib), delete %s.\n" "$SYSROOT"
  exit 1
else
  printf "Done! The toolchain (%s) is installed at %s and the sysroot is at %s.\n" "$TARGET" "$PREFIX" "$SYSROOT"
  printf "You can safely delete %s if you don't want the cached toolchain sources anymore.\n" "$BUILD"
fi
