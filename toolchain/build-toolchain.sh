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

  msg "Configuring binutils..."
  mkdir -p "binutils-$BINUTILS_VERSION-build"
  cd "binutils-$BINUTILS_VERSION-build"
  "$CONFIGURE_SCRIPT" --prefix="$PREFIX" --target="$TARGET" --with-sysroot="$SYSROOT" --disable-nls --enable-shared || exit 1
  msg "Making binutils..."
  make -j "$NUM_JOBS" || exit 1
  msg "Installing binutils..."
  make install || exit 1
  success "Installed binutils!"
  cd ..

  popd
}

install_headers() {
  msg "Installing libc headers..."
  mkdir -p "$SYSROOT"/usr/include
  LIBC_HEADERS=$(find "$LIBC_LOC" -name '*.h' -print)
  while IFS= read -r HEADER; do
    "$INSTALL_BIN" -D "$HEADER" "$SYSROOT/usr/include/$(echo "$HEADER" | sed -e "s@$LIBC_LOC@@")"
  done <<< "$LIBC_HEADERS"
  success "Installed libc headers!"

  msg "Installing libm headers..."
  mkdir -p "$SYSROOT"/usr/include
  LIBM_HEADERS=$(find "$LIBM_LOC" -name '*.h' -print)
  while IFS= read -r HEADER; do
    "$INSTALL_BIN" -D "$HEADER" "$SYSROOT/usr/include/$(echo "$HEADER" | sed -e "s@$LIBM_LOC@@")"
  done <<< "$LIBM_HEADERS"
  success "Installed libm headers!"

  msg "Installing kernel headers..."
  mkdir -p "$SYSROOT"/usr/include/kernel
  KERNEL_HEADERS=$(find "$KERNEL_LOC" -name '*.h' -print)
  while IFS= read -r HEADER; do
    "$INSTALL_BIN" -D "$HEADER" "$SYSROOT/usr/include/kernel/$(echo "$HEADER" | sed -e "s@$KERNEL_LOC@@")"
  done <<< "$KERNEL_HEADERS"
  success "Installed kernel headers!"
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

  extra_config_args=()
  if [ "$SYS_NAME" = "Darwin" ]; then
    for library in gmp mpfr mpc; do
	  [ "$library" = "mpc" ] && brew_formula="libmpc" || brew_formula="$library"
	  extra_config_args+=("--with-$library=$(brew --prefix --installed "$brew_formula")")
	done
  fi

  msg "Configuring gcc..."
  mkdir -p "gcc-$GCC_VERSION-build"
  cd "gcc-$GCC_VERSION-build"
  "$CONFIGURE_SCRIPT" --prefix="$PREFIX" --target="$TARGET" --disable-nls --enable-languages=c,c++ --with-sysroot="$SYSROOT" --with-newlib --enable-shared "${extra_config_args[@]}" || exit 1

  msg "Making gcc..."
  make -j "$NUM_JOBS" all-gcc all-target-libgcc || exit 1
  msg "Installing gcc..."
  make install-gcc install-target-libgcc || exit 1
  success "Installed gcc!"

  install_headers

  msg "Making libstdc++..."
  make -j "$NUM_JOBS" all-target-libstdc++-v3 || exit 1
  msg "Installing libstdc++..."
  make install-gcc install-target-libstdc++-v3
  success "Installed libstdc++!"

  cd ..
  popd
}

make_toolchain_file () {
  msg "Making CMakeToolchain.txt..."
  cp "$DIR/CMakeToolchain.txt.in" "$BUILD/CMakeToolchain.txt"
  $SED_BIN -i "s/@DUCKOS_SOURCE_DIR@/$(echo "$SOURCE_DIR" | sed -r 's/\//\\\//g')/g" "$BUILD/CMakeToolchain.txt"
}

mkdir -p "$BUILD"

if [ "$1" ]; then
  if [ "$1" == "gcc" ]; then
    build_gcc
  elif [ "$1" == "binutils" ]; then
    build_binutils
  elif [ "$1" == "edited-binutils" ]; then
    build_binutils edited
  elif [ "$1" == "edited-gcc" ]; then
    build_gcc edited
  elif [ "$1" == "install-headers" ]; then
    install_headers
    exit
  elif [ "$1" == "make-toolchain-file" ]; then
    make_toolchain_file
    exit
  else
    fail "Unknown argument ${1}. Please pass either edited-binutils or edited-gcc to rebuild the respective component from the edit directory."
  fi
else
  msg "Building binutils and gcc..."
  build_binutils
  build_gcc
  make_toolchain_file
fi

success "Done! The toolchain ($TARGET) is installed at $PREFIX and the sysroot is at $SYSROOT."
msg "You can safely delete the four binutils and gcc related folders in $BUILD if you don't want the cached toolchain sources anymore."
