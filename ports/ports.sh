#!/bin/bash


PORTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SOURCE_DIR="$PORTS_DIR/.."
ROOT_DIR="$SOURCE_DIR/cmake-build/root"
PORTS_D_DIR="$ROOT_DIR/usr/ports.d"
PATH="$PATH:$SOURCE_DIR/toolchain/tools/bin"
NUM_JOBS=$(( $(nproc) / 2 ))

export DUCKOS_PORTS_SCRIPT="${PORTS_DIR}/ports.sh"

source "$SOURCE_DIR/scripts/duckos.sh"

# Setup environment
export CXX="i686-pc-duckos-g++"
export CC="i686-pc-duckos-gcc"
export RANLIB="i686-pc-duckos-ranlib"
export AR="i686-pc-duckos-ar"
export CXXFILT="i686-pc-duckos-c++filt"
export READELF="i686-pc-duckos-readelf"
export STRIP="$i686-pc-duckos-strip"
export OBJCOPY="i686-pc-duckos-objcopy"

download_extract_patch() {
  if [ ! -d "$DOWNLOAD_FILE" ]; then
    msg "Downloading $DOWNLOAD_URL"
    curl "$DOWNLOAD_URL" > "$DOWNLOAD_FILE.tar.gz" || return 1
    msg "Extracting $DOWNLOAD_FILE.tar.gz..."
    tar -xf "$DOWNLOAD_FILE.tar.gz" || return 1
    rm "$DOWNLOAD_FILE.tar.gz"
    pushd "$DOWNLOAD_FILE"
    if [ -n "$PATCH_FILE" ]; then
      msg "Applying patch $PATCH_FILE..."
      patch -p1 < "$PORT_DIR/$PATCH_FILE" > /dev/null || return 1
      success "Downloaded and patched $DOWNLOAD_FILE!"
    else
      success "Downloaded $DOWNLOAD_FILE!"
    fi
    popd
  else
    msg "$DOWNLOAD_FILE already downloaded!"
  fi
}

git_clone_patch() {
  if [ ! -d ".git" ]; then
    msg "Cloning $GIT_URL"
    git clone "$GIT_URL" .
    if [ -n "$PATCH_FILE" ]; then
      msg "Applying patch $PATCH_FILE..."
      patch -p1 < "$PORT_DIR/$PATCH_FILE" > /dev/null || return 1
      success "Cloned and patched $GIT_URL!"
    else
      success "Cloned $GIT_URL!"
    fi
  else
    msg "$GIT_URL already downloaded!"
  fi
}

install_dependencies() {
  msg "Installing dependencies for $DUCKOS_PORT_NAME..."
  for DEPENDENCY in "${DEPENDENCIES[@]}"; do
    echo "CHECK $PORTS_D_DIR/$DEPENDENCY"
    if [ ! -f "$PORTS_D_DIR/$DEPENDENCY" ]; then
      ("$PORTS_DIR/ports.sh" $DEPENDENCY) || fail "Could not build dependency $DEPENDENCY!"
    fi
  done
  success "Installed dependencies for $DUCKOS_PORT_NAME!"
}

build_port() {
  export DUCKOS_PORT_NAME="$1"
  export PORT_DIR="$PORTS_DIR/$DUCKOS_PORT_NAME"
  msg "Building port $DUCKOS_PORT_NAME..."
  
  # Default args
  export DOWNLOAD_URL=""
  export DOWNLOAD_FILE="$DUCKOS_PORT_NAME"
  export PATCH_FILE=""
  export USE_CONFIGURE=""
  export CONFIGURE_ARGS=()
  export MAKE_ARGS=()
  export INSTALL_ARGS=("install")
  export DEPENDENCIES=()
  export GIT_URL=""
  source "$PORT_DIR/build.sh"

  install_dependencies

  mkdir -p "$PORTS_DIR/build/$DUCKOS_PORT_NAME"
  pushd "$PORTS_DIR/build/$DUCKOS_PORT_NAME" || return 1
  if [ -n "$DOWNLOAD_URL" ]; then
    download_extract_patch
  elif [ -n "$GIT_URL" ]; then
    git_clone_patch
  fi
  if [ "$USE_CONFIGURE" = "true" ]; then
    msg "Configuring port $DUCKOS_PORT_NAME..."
    "$DOWNLOAD_FILE/configure" --host="i686-pc-duckos" "${CONFIGURE_ARGS[@]}" || return 1
  fi
  msg "Building port $DUCKOS_PORT_NAME..."
  make "-j$NUM_JOBS" "${MAKE_ARGS[@]}" || return 1
  msg "Installing port $DUCKOS_PORT_NAME..."
  make DESTDIR="$ROOT_DIR" "${INSTALL_ARGS[@]}" || return 1
  popd || return 1
  mkdir -p "$PORTS_D_DIR"
  touch "$PORTS_D_DIR/$DUCKOS_PORT_NAME"
  success "Built port $DUCKOS_PORT_NAME!"
}

if [ ! -z "$1" ]; then
  build_port "$1" || fail "Could not build port $1!"
fi
