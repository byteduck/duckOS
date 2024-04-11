#!/bin/bash
LIBJPEG_VER=9d
export DOWNLOAD_URL="https://ijg.org/files/jpegsrc.v$LIBJPEG_VER.tar.gz"
export DOWNLOAD_FILE="jpeg-$LIBJPEG_VER"
export PATCH_FILES=("config.sub.patch")
export USE_CONFIGURE="true"
export CONFIGURE_OPTIONS=("--disable-static" "--enable-shared")