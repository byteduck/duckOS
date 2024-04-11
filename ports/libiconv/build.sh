#!/bin/bash
export DOWNLOAD_URL="https://ftp.gnu.org/gnu/libiconv/libiconv-1.17.tar.gz"
export DOWNLOAD_FILE="libiconv-1.17"
export PATCH_FILES=("libiconv.patch")
export USE_CONFIGURE="true"
export CONFIGURE_OPTIONS=("--enable-shared" "--disable-nls")
