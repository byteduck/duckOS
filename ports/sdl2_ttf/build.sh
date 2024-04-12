#!/bin/bash
SDL2_TTF_VER="2.22.0"
export DOWNLOAD_URL="https://github.com/libsdl-org/SDL_ttf/releases/download/release-$SDL2_TTF_VER/SDL2_ttf-$SDL2_TTF_VER.tar.gz"
export DOWNLOAD_FILE="SDL2_ttf-$SDL2_TTF_VER"
export USE_CONFIGURE="true"
export DEPENDENCIES=("sdl2" "freetype")
export CONFIG_SUB_PATHS=("config.sub")
export CONFIGURE_ARGS=("--with-sdl-prefix=$ROOT_DIR/usr/local" "--with-x='no'" "--disable-static" "--enable-shared")