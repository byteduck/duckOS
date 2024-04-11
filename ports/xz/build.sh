#!/bin/bash
XZ_VER=5.4.0
export DOWNLOAD_URL="https://github.com/xz-mirror/xz/releases/download/v$XZ_VER/xz-$XZ_VER.tar.gz"
export DOWNLOAD_FILE="xz-$XZ_VER"
export PATCH_FILES=("config.sub.patch")
export USE_CONFIGURE="true"
export CONFIGURE_OPTIONS=("--disable-static" "--enable-shared")
export DEPENDENCIES=("zlib" "libiconv")