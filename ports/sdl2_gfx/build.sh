#!/bin/bash
SDL2_GFX_VER="1.0.4"
export DOWNLOAD_URL="https://downloads.sourceforge.net/project/sdl2gfx/SDL2_gfx-$SDL2_GFX_VER.tar.gz"
export DOWNLOAD_FILE="SDL2_gfx-$SDL2_GFX_VER"
export USE_CONFIGURE="true"
export DEPENDENCIES=("sdl2")
export CONFIG_SUB_PATHS=("config.sub")
export CONFIGURE_ARGS=("--with-sdl-prefix=$ROOT_DIR/usr/local" "--disable-static" "--enable-shared")