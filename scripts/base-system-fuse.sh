#!/bin/bash
set -e

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source "$SCRIPTDIR/duckos.sh"

if [ $# -eq 0 ]; then
  fail "Please provide a directory to create the filesystem in."
fi

FS_DIR="$1"

msg "Copying base and kernel to filesystem..."
if [ -d "${SOURCE_DIR}/user" ]; then
  rsync -auH --inplace "${SOURCE_DIR}/user/"* "$FS_DIR" || (echo "Couldn't copy user folder." && exit 1)
else
  warn "No user folder, or empty user folder."
fi
rsync -auH --inplace "${SOURCE_DIR}/base/"* "$FS_DIR" || (echo "Couldn't copy base." && exit 1)
rsync -auH --inplace "root/"* "$FS_DIR"/ || (echo "Couldn't copy root." && exit 1)

msg "Setting up root filesystem..."
msg "Setting up devices..."
if [ -d "$FS_DIR"/dev ]; then
  rm -rf "${FS_DIR:?}"/dev/*
fi
mkdir -p "$FS_DIR"/dev
mkdir -p "$FS_DIR"/dev/input
mkdir -p "$FS_DIR"/dev/pts
msg "Setting up directories..."
chmod -R g+rX,o+rX "$FS_DIR"/

msg "Setting up /proc/..."
mkdir -p "$FS_DIR"/proc
chmod 555 "$FS_DIR"/proc

msg "Setting up /sock/..."
mkdir -p "$FS_DIR"/sock
chmod 777 "$FS_DIR"/sock

msg "Setting up /etc/..."
chown 0:0 "$FS_DIR"/etc/*
chown 0:0 "$FS_DIR"/etc/init/*
chown 0:0 "$FS_DIR"/etc/init/services/*

success "Done setting up root filesystem!"
