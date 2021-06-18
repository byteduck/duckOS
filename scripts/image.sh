#!/bin/bash
set -e

if [ "$(id -u)" != 0 ]; then
	echo "Please run as root"
	exit 1
fi

if [ $# -eq 0 ]; then
  echo "Creating a 100MB image..."
  dd if=/dev/zero of=duckOS.img count=204800 bs=512 status=none
  echo "Made blank image."

  echo "Making loopback device..."
  dev=$(losetup --find --partscan --show duckOS.img)
  part="p1"
  if [ -z "$dev" ]; then
    echo "Couldn't mount loopback."
    exit 1
  fi
  echo "Loopback device mounted at ${dev}"
else
  dev="$1"
  part="1"
  echo "Using $1..."
fi

echo "Creating partition table..."
parted -s "${dev}" mklabel msdos mkpart primary ext2 32k 100% -a minimal set 1 boot on || (echo "Couldn't partition image" && exit 1)
echo "Created partition table."

echo "Creating ext2 filesystem..."
mke2fs -q -I 128 -b 1024 "${dev}${part}" || (echo "Couldn't create filesystem." && exit 1)
echo "Created filesystem."

echo "Mounting filesystem to mnt/ ..."
mkdir -p mnt/
mount "${dev}${part}" mnt/ || (echo "Couldn't mount." && exit 1)
echo "Mounted."

echo "Installing grub..."
grub-install --boot-directory=mnt/boot --target=i386-pc --modules="ext2 part_msdos" "${dev}" || (echo "Couldn't install grub." && exit 1)
cp "${SOURCE_DIR}/scripts/grub.cfg" mnt/boot/grub || (echo "Couldn't copy grub.cfg." && exit 1)
echo "Installed grub!"

echo "Setting up base filesystem..."
bash "${SOURCE_DIR}/scripts/base-system.sh" mnt/ || (echo "Couldn't make base filesystem." && exit 1)

if [ $# -eq 0 ]; then
  echo "Unmounting and cleaning up loopback device..."
  umount mnt/ || (echo "Couldn't unmount." && exit 1)
  rmdir mnt || (echo "Couldn't rmdir mnt." && exit 1)
  losetup -d "${dev}" || (echo "Couldn't clean up loopback device." && exit 1)
  echo "Done! Saved to duckOS.img."
else
  echo "Done! Image created on $1."
fi