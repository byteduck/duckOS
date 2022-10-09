#!/bin/bash
source "$SOURCE_DIR/toolchain/toolchain-common.sh"

export DOWNLOAD_URL="$BINUTILS_URL"
export DOWNLOAD_FILE="$BINUTILS_FILE"
export PATCH_FILE="binutils.patch"
export USE_CONFIGURE="true"
export CONFIGURE_ARGS=("--target=$TARGET" "--with-sysroot=/" "--with-build-sysroot=$ROOT_DIR" "--disable-nls" "--disable-werror" "--disable-gdb")

#source ../ports.sh
#source "$SOURCE_DIR/toolchain/toolchain-common.sh"
#
#mkdir -p build
#pushd build

## Download binutils
#download-binutils
#
## Configure binutils
#msg "Configuring binutils..."
#"$BINUTILS_FILE/configure" --host="i686-pc-duckos" --target="$TARGET" --with-sysroot="/" --with-build-sysroot="$ROOT_DIR" --disable-nls --disable-werror --disable-gdb || exit 1
#
## Make binutils
#msg "Making binutils..."
#make -j "$NUM_JOBS" || exit 1
#msg "Installing binutils..."
#make DESTDIR="$ROOT_DIR" install || exit 1
#success "Installed binutils!"

#popd