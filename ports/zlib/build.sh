#!/bin/bash
ZLIB_VER=1.3.1
export DOWNLOAD_URL="https://github.com/madler/zlib/releases/download/v$ZLIB_VER/zlib-$ZLIB_VER.tar.gz"
export DOWNLOAD_FILE="zlib-$ZLIB_VER"

prebuild() {
  "$DOWNLOAD_FILE/configure" --uname=duckOS
}