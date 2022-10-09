#!/bin/bash
export DOWNLOAD_URL="https://www.mpfr.org/mpfr-current/mpfr-4.1.0.tar.xz"
export DOWNLOAD_FILE="mpfr-4.1.0"
export PATCH_FILE="mpfr.patch"
export USE_CONFIGURE="true"
export CONFIGURE_OPTIONS=("--target=i686-pc-duckos" "--with-sysroot=/")