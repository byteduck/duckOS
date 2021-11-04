#!/bin/bash
set -e

fail () {
  echo $1

  if [ $? -eq 0 ]; then
    exit 1
  else
    exit $?
  fi
}

if [ "$(id -u)" != 0 ]; then
	exec sudo -E -- "$0" "$@" || fail "Please run as root"
else
    : "${SUDO_UID:=0}" "${SUDO_GID:=0}"
fi

SYSTEM="$(uname -s)"
DU_COMMAND="du"
IMAGE_NAME="duckOS.img"
IMAGE_EXTRASIZE="10000"

if [ "$SYSTEM" = "Darwin" ]; then
    export PATH="/usr/local/opt/e2fsprogs/bin:/usr/local/opt/e2fsprogs/sbin:$PATH"
    DU_COMMAND="gdu"
fi

if [ $# -eq 0 ]; then
  # Calculate size of image
  IMAGE_SIZE=$(($("$DU_COMMAND" -sk root | cut -f1) + IMAGE_EXTRASIZE))

  if [ -f "$IMAGE_NAME" ]; then
    USE_EXISTING=1
    echo "Using existing image..."
  else
    echo "Creating image ($IMAGE_SIZE bytes)..."
    qemu-img create -q -f raw "$IMAGE_NAME" "$IMAGE_SIZE"K || fail "Couldn't create image"
    chown "$SUDO_UID":"$SUDO_GID" "$IMAGE_NAME"
  fi

  if [ "$SYSTEM" = "Darwin" ]; then
    echo "Attaching image..."
    dev_arr=($(hdiutil attach -nomount "$IMAGE_NAME"))
    dev=${dev_arr[0]}
    part="s1"
    if [ -z "$dev" ]; then
      fail "Couldn't attach image."
    fi
  else
    echo "Making loopback device..."
    dev=$(losetup --find --partscan --show "$IMAGE_NAME")
    part="p1"
    if [ -z "$dev" ]; then
     fail "Couldn't create loopback device."
    fi
  fi

  echo "Loopback device mounted at ${dev}"
else
  dev="$1"
  part="1"
  echo "Using $1..."
fi

if [ ! "$USE_EXISTING" ]; then
  echo "Creating partition table..."
  if [ "$SYSTEM" = "Darwin" ]; then
    echo ":NJO"
    diskutil partitionDisk $(basename "$dev") 1 MBR fuse-ext2 duckOS 100% || fail "Couldn't partition image."
  else
    parted -s "${dev}" mklabel msdos mkpart primary ext2 32k 100% -a minimal set 1 boot on || fail "Couldn't partition image."
  fi

  echo "Creating ext2 filesystem..."
  yes | mke2fs -q -I 128 -b 1024 "${dev}${part}" || fail "Couldn't create filesystem."
fi

echo "Mounting filesystem on ${dev}${part} to mnt ..."
mkdir -p mnt/
if [ "$SYSTEM" = "Darwin" ]; then
  fuse-ext2 "${dev}${part}" mnt -o rw+,allow_other,uid="$SUDO_UID",gid="$SUDO_GID" || fail "Couldn't mount."
else
  mount "${dev}${part}" mnt/ || fail "Couldn't mount."
fi

if [ "$SYSTEM" = "Darwin" ]; then
  echo "IMPORTANT: GRUB command line tools are not present on macOS. GRUB will not be installed."
else
  echo "Installing grub..."
  grub-install --boot-directory=mnt/boot --target=i386-pc --modules="ext2 part_msdos" "${dev}" || fail "Couldn't install grub."
  cp "${SOURCE_DIR}/scripts/grub.cfg" mnt/boot/grub || fail "Couldn't copy grub.cfg."
fi

echo "Setting up base filesystem..."
bash "${SOURCE_DIR}/scripts/base-system.sh" mnt/ || fail "Couldn't make base filesystem."

if [ $# -eq 0 ]; then
  echo "Unmounting and cleaning up..."
  umount mnt/ || fail "Couldn't unmount."
  rmdir mnt || fail "Couldn't rmdir mnt."
  if [ "$SYSTEM" = "Darwin" ]; then
    hdiutil detach "$dev" || fail "Couldn't detach image."
  else
    losetup -d "${dev}" || fail "Couldn't clean up loopback device."
  fi
  echo "Done! Saved to $IMAGE_NAME."
else
  echo "Done! Image created on $1."
fi