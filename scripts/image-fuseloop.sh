#!/bin/bash
set -e

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source "$SCRIPTDIR/duckos.sh"

FUSELOOP=fuseloop

detach_and_unmount() {
  msg "Cleaning up..."
  if [ ! -z "$MOUNTED" ]; then
    fusermount -u mnt/ || fail "Couldn't unmount."
    rmdir mnt || fail "Couldn't rmdir mnt."
    fusermount -u "$part"
  fi

  if [ ! -z "$dev" ]; then
    fusermount -u "${dev}" || fail "Couldn't clean up loopback device."
  fi
}

ON_FAIL=detach_and_unmount

SYSTEM="$(uname -s)"
DU_COMMAND="du"
IMAGE_NAME="duckOS.img"
IMAGE_EXTRASIZE="100000"

# Calculate size of image
USER_SIZE=0
if [ -d "${SOURCE_DIR}/user" ]; then
  USER_SIZE=$("$DU_COMMAND" -sk "${SOURCE_DIR}/user" | cut -f1)
fi
IMAGE_SIZE=$(($("$DU_COMMAND" -sk root | cut -f1) + IMAGE_EXTRASIZE + USER_SIZE))
IMAGE_SIZE=$(expr '(' '(' "$IMAGE_SIZE" + 1023 ')' / 1024 ')' '*' 1024)

msg "Creating image ($IMAGE_SIZE K)..."
qemu-img create -q -f raw "$IMAGE_NAME" "$IMAGE_SIZE"k || fail "Couldn't create image"
chown "$SUDO_UID":"$SUDO_GID" "$IMAGE_NAME"

msg "Making loopback device..."
touch dev
dev="dev"
$FUSELOOP $IMAGE_NAME dev
touch p1
part="p1"
if [ -z "$dev" ]; then
 fail "Couldn't create loopback device."
fi

msg "Loopback device mounted at ${dev}"

msg "Creating partition table..."
/sbin/parted -s "${dev}" mklabel msdos mkpart primary ext2 32k 100% -a minimal set 1 boot on || fail "Couldn't partition image."

sectorsize=512
line=`/sbin/fdisk -c -u -l -C 300 -H 64 -S 32 -b 512 $IMAGE_NAME|grep $IMAGE_NAME|grep Linux|sed 's/\*//'`
set -- $line
offs=$(($2*$sectorsize))
size=$((($3-$2)*$sectorsize))

$FUSELOOP -O "$offs" -S "$size" $IMAGE_NAME "$part"

msg "Creating ext2 filesystem..."
yes | /sbin/mke2fs -q -I 128 -b 1024 "${part}" || fail "Couldn't create filesystem."

msg "Mounting filesystem on ${part} to mnt ..."
mkdir -p mnt/
fuseext2 -o rw+ "${part}" mnt/ || fail "Couldn't mount."
MOUNTED="1"

msg "Installing grub..."
GRUB_COMMAND="/usr/sbin/grub2-install"
GRUB_EXTRAARGS="--force"
if ! type "$GRUB_COMMAND" &> /dev/null; then
    GRUB_COMMAND="/usr/sbin/grub-install"
fi
"$GRUB_COMMAND" --boot-directory=mnt/boot --target=i386-pc --modules="ext2 part_msdos" "${dev}" $GRUB_EXTRAARGS || fail "Couldn't install grub."
if [[ -d "mnt/boot/grub2" ]]; then
    cp "${SOURCE_DIR}/scripts/grub.cfg" mnt/boot/grub2/grub.cfg || fail "Couldn't copy grub.cfg."
elif [[ -d "mnt/boot/grub" ]]; then
    cp "${SOURCE_DIR}/scripts/grub.cfg" mnt/boot/grub/grub.cfg || fail "Couldn't copy grub.cfg."
else
    fail "Couldn't find GRUB installation directory."
fi

msg "Setting up base filesystem..."
bash "${SOURCE_DIR}/scripts/base-system-fuse.sh" mnt/ || fail "Couldn't make base filesystem."

fusermount -u mnt/ || fail "Couldn't unmount."
rmdir mnt || fail "Couldn't rmdir mnt."

#detach_and_unmount

msg "Setting up base filesystem block devices..."
#$FUSELOOP -O "$offs" -S "$size" $IMAGE_NAME "$part"

/sbin/debugfs -w $part << EOF
cd dev
mknod tty0 c 4 0
mknod hda b 3 0
mknod random c 1 8
mknod null c 1 3
mknod zero c 1 5
mknod klog c 1 16
mknod fb0 b 29 0
mknod ptmx c 5 2
mknod snd0 c 69 2
cd input
mknod keyboard c 13 0
mknod mouse c 13 1
quit
EOF

fusermount -u "$part"
unset MOUNTED

detach_and_unmount

success "Done! Saved to $IMAGE_NAME."
