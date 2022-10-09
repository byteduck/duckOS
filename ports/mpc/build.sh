#!/bin/bash
export DOWNLOAD_URL="https://ftp.gnu.org/gnu/mpc/mpc-1.2.1.tar.gz"
export DOWNLOAD_FILE="mpc-1.2.1"
export PATCH_FILE="mpc.patch"
export USE_CONFIGURE="true"
export CONFIGURE_OPTIONS=("--target=i686-pc-duckos" "--with-sysroot=/")