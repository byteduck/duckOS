#!/bin/bash
LIBPNG_VER=1.6.40
export DOWNLOAD_URL="https://download.sourceforge.net/libpng/libpng-$LIBPNG_VER.tar.gz"
export DOWNLOAD_FILE="libpng-$LIBPNG_VER"
export PATCH_FILES=("config.sub.patch" "libtool-configure.patch")
export USE_CONFIGURE="true"
export CONFIGURE_OPTIONS=("--disable-static" "--enable-shared")
export DEPENDENCIES=("zlib")