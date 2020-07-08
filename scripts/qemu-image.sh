#!/bin/sh

qemu-system-x86_64 -drive file=duckOS.img,cache=directsync,format=raw,id=disk,if=ide -m 2G -machine type=pc-i440fx-3.1 -serial stdio