#!/bin/bash


PORTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SOURCE_DIR="$PORTS_DIR/.."
ROOT_DIR="$SOURCE_DIR/build/i686/root"
PORTS_D_DIR="$ROOT_DIR/usr/ports.d"
PATH="$PATH:$SOURCE_DIR/toolchain/tools/i686/bin"
NUM_JOBS=$(( $(nproc) / 2 ))

SED_BIN="sed"
if [ "$SYS_NAME" = "Darwin" ]; then
  INSTALL_BIN="ginstall"
  SED_BIN="gsed"
fi

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
export PKG_CONFIG_DIR=""
export PKG_CONFIG_SYSROOT_DIR="$ROOT_DIR"
export PKG_CONFIG_LIBDIR="$ROOT_DIR/usr/local/lib/pkgconfig"

apply_config_subs() {
  for CONFIGSUB in "${CONFIG_SUB_PATHS[@]}"; do
    msg "Patching config.sub $CONFIGSUB"
    "$SED_BIN" -i '/Each alternative MUST end in/a duckos\* | \\' "$CONFIGSUB" || fail "Failed to automatically patch $CONFIGSUB"
    "$SED_BIN" -i '/sysv\* is not here because/a -duckos\* | \\' "$CONFIGSUB" || fail "Failed to automatically patch $CONFIGSUB"
  done
}

download_extract_patch() {
  if [ ! -d "$DOWNLOAD_FILE" ]; then
    msg "Downloading $DOWNLOAD_URL"
    curl -L "$DOWNLOAD_URL" > "$DOWNLOAD_FILE.tar.gz" || return 1
    msg "Extracting $DOWNLOAD_FILE.tar.gz..."
    tar -xf "$DOWNLOAD_FILE.tar.gz" || return 1
    rm "$DOWNLOAD_FILE.tar.gz"
    pushd "$DOWNLOAD_FILE" || return 1
    apply_config_subs || return 1
    if [ -n "$PATCH_FILES" ]; then
      for PATCH in "${PATCH_FILES[@]}"; do
        msg "Applying patch $PATCH..."
        patch -p1 < "$PORT_DIR/$PATCH" > /dev/null || fail "Failed to apply patch!"
      done
      success "Downloaded and patched $DOWNLOAD_FILE!"
    else
      success "Downloaded $DOWNLOAD_FILE!"
    fi
    popd || return 1
  else
    msg "$DOWNLOAD_FILE already downloaded!"
  fi
}

git_clone_patch() {
  if [ ! -d ".git" ]; then
    msg "Cloning $GIT_URL"
    git clone "$GIT_URL" . || return 1
    if [ -n "$GIT_BRANCH" ]; then
      git switch "$GIT_BRANCH" || return 1
    fi
    apply_config_subs || return 1
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
  export CONFIGURE_PATH=""
  export CONFIG_SUB_PATHS=()
  source "$PORT_DIR/build.sh"

  install_dependencies || return 1

  mkdir -p "$PORTS_DIR/build/$DUCKOS_PORT_NAME"
  pushd "$PORTS_DIR/build/$DUCKOS_PORT_NAME" || return 1
  if [ -n "$DOWNLOAD_URL" ]; then
    download_extract_patch || return 1
  elif [ -n "$GIT_URL" ]; then
    git_clone_patch || return 1
  fi
  if [[ $(type -t prebuild) == function ]]; then
    msg "Executing prebuild steps for $DUCKOS_PORT_NAME..."
    prebuild
  fi
  if [ "$USE_CONFIGURE" = "true" ]; then
    msg "Configuring port $DUCKOS_PORT_NAME..."
    if [ -z "$CONFIGURE_PATH" ]; then
      CONFIGURE_PATH="$DOWNLOAD_FILE/configure"
    fi
    "$CONFIGURE_PATH" --host="i686-pc-duckos" "${CONFIGURE_ARGS[@]}" || return 1
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
