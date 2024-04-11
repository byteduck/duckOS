#!/bin/bash
source "$SOURCE_DIR/toolchain/toolchain-common.sh"

export DOWNLOAD_URL="$GCC_URL"
export DOWNLOAD_FILE="$GCC_FILE"
export PATCH_FILES=("gcc.patch")
export USE_CONFIGURE="true"
export CONFIGURE_ARGS=("--target=$TARGET" "--with-sysroot=/" "--with-build-sysroot=$ROOT_DIR" "--disable-nls" "--enable-languages=c,c++" "--with-newlib" "--enable-shared" "--disable-lto")
export MAKE_ARGS=("all-gcc" "all-target-libgcc" "all-target-libstdc++-v3")
export INSTALL_ARGS=("install-gcc" "install-target-libgcc" "install-target-libstdc++-v3")
export DEPENDENCIES=("gmp" "mpfr" "mpc" "binutils")
