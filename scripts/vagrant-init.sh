#!/bin/bash

cd /vagrant

# Needed for running scripts correctly.
sudo apt-get install dos2unix -y
find scripts/*.sh -type f -exec dos2unix {} \;
find toolchain/*.sh -type f -exec dos2unix {} \;

sudo locale-gen UTF-8

sudo apt-get install software-properties-common -y
sudo add-apt-repository ppa:george-edison55/cmake-3.x -y
sudo apt update -y
sudo apt install build-essential cmake bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo qemu-system-i386 qemu-utils nasm gawk grub2-common grub-pc rsync -y

export CMAKE_ASM_NASM_COMPILER=nasm

export IS_VAGRANT=1

cd toolchain
chmod +x ./build-toolchain.sh
source "build-toolchain.sh"

cd ../
mkdir -p cmake-build
cd cmake-build
cmake .. -DCMAKE_TOOLCHAIN_FILE=toolchain/CMakeToolchain.txt