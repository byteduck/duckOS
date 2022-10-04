#!/bin/sh

set -e

# Check for KVM Support
if [ -z "$USE_KVM" ]; then
	USE_KVM="0"
	if [ -e /dev/kvm ] && [ -r /dev/kvm ] && [ -w /dev/kvm ]; then
		USE_KVM="1"
	fi
fi

if [ -z "$DUCKOS_IMAGE" ]; then
	DUCKOS_IMAGE="duckOS.img"
fi

# Determine what acceleration we should use
if [ -z "$DUCKOS_QEMU_ACCEL" ]; then
	if command -v wslpath >/dev/null; then
		DUCKOS_QEMU_ACCEL="-accel whpx,kernel-irqchip=off -accel tcg"
	else
		if [ "$USE_KVM" -ne "0" ]; then
			DUCKOS_QEMU_ACCEL="-enable-kvm"
		else
			DUCKOS_QEMU_ACCEL=""
		fi
	fi
fi

# Find which qemu binary we should use
if command -v wslpath >/dev/null; then
	# We're on windows, use windows QEMU
	if [ -z "$DUCKOS_WIN_QEMU_INSTALL_DIR" ]; then
		DUCKOS_WIN_QEMU_INSTALL_DIR="C:\\Program Files\\qemu"
	fi
	USE_KVM="0"
	DUCKOS_QEMU="$(wslpath "${DUCKOS_WIN_QEMU_INSTALL_DIR}")/qemu-system-x86_64.exe"
	DUCKOS_IMAGE="$(wslpath -w "$DUCKOS_IMAGE")"
else
	DUCKOS_QEMU="qemu-system-x86_64"
fi

DUCKOS_QEMU_DISPLAY=""

if "$DUCKOS_QEMU" --display help | grep -iq sdl; then
	DUCKOS_QEMU_DISPLAY="--display sdl"
elif "$DUCKOS_QEMU" --display help | grep -iq cocoa; then
	DUCKOS_QEMU_DISPLAY="--display cocoa"
fi

if [ -z "$DUCKOS_KERNEL_ARGS" ]; then
  DUCKOS_KERNEL_ARGS="$@"
fi

# Run!
DUCKOS_QEMU_ARGS="
	-s
	-kernel kernel/duckk32
	-drive file=$DUCKOS_IMAGE,cache=directsync,format=raw,id=disk,if=ide
	-m 512M
	-serial stdio
	-device ac97
	$DUCKOS_QEMU_DISPLAY
	$DUCKOS_QEMU_ACCEL"

"$DUCKOS_QEMU" $DUCKOS_QEMU_ARGS -append "$DUCKOS_KERNEL_ARGS"
