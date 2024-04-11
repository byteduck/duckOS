#!/bin/bash
LIBTIFF_VER=4.2.0
export DOWNLOAD_URL="http://download.osgeo.org/libtiff/tiff-$LIBTIFF_VER.tar.gz"
export DOWNLOAD_FILE="tiff-$LIBTIFF_VER"
export PATCH_FILES=("libtiff.patch")
export USE_CONFIGURE="true"
export DEPENDENCIES=("libjpeg" "zstd" "xz")
export CONFIGURE_OPTIONS=("--disable-static" "--enable-shared")