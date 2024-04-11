#!/bin/bash
ZSTD_VER=1.5.0
export DOWNLOAD_URL="https://github.com/facebook/zstd/releases/download/v$ZSTD_VER/zstd-$ZSTD_VER.tar.gz"
export DOWNLOAD_FILE="zstd-$ZSTD_VER"
export PATCH_FILES=("pthreadfix.patch")

prebuild() {
  cd "$DOWNLOAD_FILE" || return 1
}