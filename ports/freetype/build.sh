#!/bin/bash
FREETYPE_VER="2.13.2"
export DOWNLOAD_URL="https://download.savannah.gnu.org/releases/freetype/freetype-$FREETYPE_VER.tar.gz"
export DOWNLOAD_FILE="freetype-$FREETYPE_VER"
export USE_CONFIGURE="true"
export CONFIG_SUB_PATHS=("builds/unix/config.sub")
export DEPENDENCIES=()