# duckOS Build / Run instructions

## Prerequisites

### Dependencies
- A Linux environment (WSL will work, but WSL2 is preferred because it is much faster than WSL1)
- GCC
- G++
- Make
- CMake
- Bison
- Flex
- GMP
- MPFR
- MPC
- Texinfo
- ISL
- CLooG
- QEmu (For emulating)

### Installing dependencies
- Ubuntu/Debian: `apt install build-essential cmake bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo qemu-system-i386 qemu-utils nasm`
- Arch: `pacman -S base-devel cmake gmp libmpc mpfr qemu qemu-arch-extra nasm`

## Building the toolchain
1. Open the `toolchain` directory in your terminal and run `build-toolchain.sh`. (You will need an internet connection as it downloads the needed binutils/gcc releases from the GNU ftp site and newlib from the newlib ftp site.)
2. Make a cup of coffee or tea and wait. It will take a while to compile.

### Editing the toolchain
If you'd like to edit the toolchain, you can run the `edit-toolchain.sh` script to download patch binutils/gcc/newlib and setup a git repository for each one. Then, use the `gen-patches.sh` script to generate patch files for each one.

**DO NOT** git commit in the repositories created by `edit-toolchain` or else `gen-patches` won't work properly.

To build something from the `edit` directory, pass the `edited-[thing]` to the `build-toolchain.sh` script. (ex: `build-toolchain.sh edited-newlib` to build newlib from the edit directory)

## Configuring cmake
1. Make sure you've built the toolchain first.
2. Make a build folder somewhere outside of this project where you'd like to build the kernel.
3. From that directory, run `cmake duckos_dir`, substituting `duckos_dir` for the directory in which the duckOS repository is located on your computer.

## Building and running duckOS
1. Either run the following commands as root,  or use `sudo su` to open a root shell in the build directory you made earlier.
2. In your build directory, run `make` and then `make install` to build the kernel & programs.
3. Run `make image` to make the root filesystem image.
3. Run `make qemu-image` to run qemu with the image you just made.
5. Enjoy!
