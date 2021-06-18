#!/bin/bash
set -e

if [ $# -eq 0 ]; then
  echo "Please provide a directory to create the filesystem in."
  exit 1
fi

FS_DIR="$1"

echo "Copying base and kernel to filesystem..."
if [ -f "${SOURCE_DIR}/user/"* ]; then
  cp -R "${SOURCE_DIR}/user/"* "$FS_DIR" || (echo "Couldn't copy user folder." && exit 1)
else
  echo "No user folder, or empty user folder."
fi
cp -R "${SOURCE_DIR}/base/"* "$FS_DIR" || (echo "Couldn't copy base." && exit 1)
cp -R "root/"* "$FS_DIR"/ || (echo "Couldn't copy root." && exit 1)
echo "Copied."

echo "Setting up root filesystem..."
echo "Setting up devices..."
if [ -d "$FS_DIR"/dev ]; then
  rm -rf "${FS_DIR:?}"/dev/*
fi
mkdir -p "$FS_DIR"/dev
mknod "$FS_DIR"/dev/tty0 c 4 0
mknod "$FS_DIR"/dev/hda b 3 0
mknod "$FS_DIR"/dev/random c 1 8
mknod "$FS_DIR"/dev/null c 1 3
mknod "$FS_DIR"/dev/zero c 1 5
mknod "$FS_DIR"/dev/klog c 1 16
mknod "$FS_DIR"/dev/fb0 b 29 0
mkdir -p "$FS_DIR"/dev/input
mknod "$FS_DIR"/dev/input/keyboard c 13 0
mknod "$FS_DIR"/dev/input/mouse c 13 1
mknod "$FS_DIR"/dev/ptmx c 5 2
mkdir -p "$FS_DIR"/dev/pts
echo "Done setting up devices!"
echo "Setting up directories..."
chmod -R g+rX,o+rX "$FS_DIR"/

echo "Setting up /proc/..."
mkdir -p "$FS_DIR"/proc
chmod 555 "$FS_DIR"/proc

echo "Setting up /sock/..."
mkdir -p "$FS_DIR"/sock
chmod 777 "$FS_DIR"/sock

echo "Setting up /etc/..."
chown -R 0:0 "$FS_DIR"/etc

echo "Done setting up directories!"
echo "Done setting up root filesystem!"