#!/bin/bash
set -e

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source "$SCRIPTDIR/duckos.sh"

detach_and_unmount() {
  msg "Cleaning up..."
  if [ ! -z "$MOUNTED" ]; then
    umount mnt/ || fail "Couldn't unmount."
    rmdir mnt || fail "Couldn't rmdir mnt."
  fi

  if [ ! -z "$dev" ]; then
    if [ "$SYSTEM" = "Darwin" ]; then
      hdiutil detach "$dev" || fail "Couldn't detach image."
    else
      losetup -d "${dev}" || fail "Couldn't clean up loopback device."
    fi
  fi
}

ON_FAIL=detach_and_unmount

if [ "$(id -u)" != 0 ]; then
	exec sudo -E -- "$0" "$@" || fail "Please run as root"
else
  : "${SUDO_UID:=0}" "${SUDO_GID:=0}"
fi

SYSTEM="$(uname -s)"
DU_COMMAND="du"
IMAGE_NAME="duckOS.img"
IMAGE_EXTRASIZE="100000"

if [ "$SYSTEM" = "Darwin" ]; then
    export PATH="/usr/local/opt/e2fsprogs/bin:/usr/local/opt/e2fsprogs/sbin:/opt/homebrew/opt/e2fsprogs/bin:/opt/homebrew/opt/e2fsprogs/sbin:$PATH"
    DU_COMMAND="gdu"
fi

if [ $# -eq 0 ]; then
  # Calculate size of image
  USER_SIZE=0
  if [ -d "${SOURCE_DIR}/user" ]; then
    USER_SIZE=$("$DU_COMMAND" -sk "${SOURCE_DIR}/user" | cut -f1)
  fi
  IMAGE_SIZE=$(($("$DU_COMMAND" -sk root | cut -f1) + IMAGE_EXTRASIZE + USER_SIZE))

  if [ -f "$IMAGE_NAME" ]; then
    USE_EXISTING=1
    msg "Using existing image..."
  else
    msg "Creating image ($IMAGE_SIZE K)..."
    qemu-img create -q -f raw "$IMAGE_NAME" "$IMAGE_SIZE"K || fail "Couldn't create image"
    chown "$SUDO_UID":"$SUDO_GID" "$IMAGE_NAME"
  fi

  if [ "$SYSTEM" = "Darwin" ]; then
    msg "Attaching image..."
    dev_arr=($(hdiutil attach -nomount "$IMAGE_NAME"))
    dev=${dev_arr[0]}
    part="s1"
    if [ -z "$dev" ]; then
      fail "Couldn't attach image."
    fi
  else
    msg "Making loopback device..."
    dev=$(losetup --find --partscan --show "$IMAGE_NAME")
    part="p1"
    if [ -z "$dev" ]; then
     fail "Couldn't create loopback device."
    fi
  fi

  msg "Loopback device mounted at ${dev}"
else
  dev="$1"
  part="1"
  msg "Using $1..."
fi

if [ ! "$USE_EXISTING" ]; then
  msg "Creating partition table..."
  if [ "$SYSTEM" = "Darwin" ]; then
    diskutil partitionDisk $(basename "$dev") 1 MBR fuse-ext2 duckOS 100% || fail "Couldn't partition image."
  else
    parted -s "${dev}" mklabel msdos mkpart primary ext2 32k 100% -a minimal set 1 boot on || fail "Couldn't partition image."
  fi

  msg "Creating ext2 filesystem..."
  yes | mke2fs -q -I 128 -b 1024 "${dev}${part}" || fail "Couldn't create filesystem."
fi

msg "Mounting filesystem on ${dev}${part} to mnt ..."
mkdir -p mnt/
if [ "$SYSTEM" = "Darwin" ]; then
  fuse-ext2 "${dev}${part}" mnt -o rw+,allow_other,uid="$SUDO_UID",gid="$SUDO_GID" || fail "Couldn't mount."
  MOUNTED="1"
else
  mount "${dev}${part}" mnt/ || fail "Couldn't mount."
  MOUNTED="1"
fi

if [ ! "$USE_EXISTING" ]; then
  if [ "$SYSTEM" = "Darwin" ]; then
    warn "IMPORTANT: GRUB command line tools are not present on macOS. GRUB will not be installed."
  else
    msg "Installing grub..."
    GRUB_COMMAND="grub2-install"
    GRUB_EXTRAARGS="--force"
    if ! type "$GRUB_COMMAND" &> /dev/null; then
        GRUB_COMMAND="grub-install"
        GRUB_EXTRAARGS=""
    fi
    "$GRUB_COMMAND" --boot-directory=mnt/boot --target=i386-pc --modules="ext2 part_msdos" "${dev}" $GRUB_EXTRAARGS || fail "Couldn't install grub."
    if [[ -d "mnt/boot/grub2" ]]; then
        cp "${SOURCE_DIR}/scripts/grub.cfg" mnt/boot/grub2/grub.cfg || fail "Couldn't copy grub.cfg."
    elif [[ -d "mnt/boot/grub" ]]; then
        cp "${SOURCE_DIR}/scripts/grub.cfg" mnt/boot/grub/grub.cfg || fail "Couldn't copy grub.cfg."
    else
        fail "Couldn't find GRUB installation directory."
    fi
  fi
fi

msg "Setting up base filesystem..."
bash "${SOURCE_DIR}/scripts/base-system.sh" mnt/ || fail "Couldn't make base filesystem."

if [ $# -eq 0 ]; then
  detach_and_unmount
  success "Done! Saved to $IMAGE_NAME."
else
  success "Done! Image created on $1."
fi
