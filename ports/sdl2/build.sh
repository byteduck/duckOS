#!/bin/bash
SDL2_VER="2.30.2"
export DOWNLOAD_URL="https://github.com/libsdl-org/SDL/releases/download/release-$SDL2_VER/SDL2-$SDL2_VER.tar.gz"
export DOWNLOAD_FILE="SDL2-$SDL2_VER"
export USE_CONFIGURE="false"
export PATCH_FILES=("sdl2.patch")
export DEPENDENCIES=("libiconv")

prebuild() {
  cmake "-DCMAKE_TOOLCHAIN_FILE=$SOURCE_DIR/cmake-build/CMakeToolchain.txt" \
        "-DPULSEAUDIO=OFF" "-DJACK=OFF" "-DPIPEWIRE=OFF" "-DSDL_LIBSAMPLERATE=OFF" \
        "$DOWNLOAD_FILE"
}