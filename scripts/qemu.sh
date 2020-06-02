#!/bin/sh

qemu-system-x86_64 -drive file=duckOS.img,cache=directsync,format=raw -device ich9-ahci