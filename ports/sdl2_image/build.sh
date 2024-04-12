#!/bin/bash
SDL2_IMAGE_VER="2.8.2"
export DOWNLOAD_URL="https://github.com/libsdl-org/SDL_image/releases/download/release-$SDL2_IMAGE_VER/SDL2_image-$SDL2_IMAGE_VER.tar.gz"
export DOWNLOAD_FILE="SDL2_image-$SDL2_IMAGE_VER"
export USE_CONFIGURE="true"
export DEPENDENCIES=("sdl2" "libpng" "libtiff" "libjpeg")
export PATCH_FILES=("config.sub.patch" "makefile.in.patch")
export CONFIGURE_ARGS=("--with-sdl-prefix=$ROOT_DIR/usr/local" "--enable-webp=false" "--enable-webp-shared=false" "--disable-static" "--enable-shared")