#!/bin/bash
source "$SOURCE_DIR/toolchain/toolchain-common.sh"

export DOWNLOAD_URL="$GCC_URL"
export DOWNLOAD_FILE="$GCC_FILE"
export PATCH_FILE="gcc.patch"
export USE_CONFIGURE="true"
export CONFIGURE_ARGS=("--target=$TARGET" "--with-sysroot=/" "--with-build-sysroot=$ROOT_DIR" "--disable-nls" "--enable-languages=c,c++" "--with-newlib" "--enable-shared" "--enable-host-shared" "--disable-lto")