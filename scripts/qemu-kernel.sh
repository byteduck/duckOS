#!/bin/sh
qemu-system-x86_64 -s -kernel kernel/duckk32 -append "" -drive file=duckOS.img,cache=directsync,format=raw,id=disk,if=ide -m 512M -enable-kvm -serial stdio