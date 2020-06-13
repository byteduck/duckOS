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
- Ubuntu/Debian: `apt install build-essential cmake bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo libcloog-isl-dev libisl-dev qemu-system-i386 qemu-utils`
- Arch: `pacman -Syu base-devel cmake gmp libmpc mpfr base-devel qemu qemu-arch-extra`

## Building the toolchain
1. Open the `toolchain` directory in your terminal and run `build-toolchain.sh`. (You will need an internet connection as it downloads the needed binutils/gcc releases from the GNU ftp site)
2. Make a cup of coffee or tea and wait. It will take a while to compile.

## Configuring cmake
1. Make sure you've built the toolchain first.
2. Make a build folder somewhere outside of this project where you'd like to build the kernel.
3. From that directory, run `cmake duckos_dir`, substituting `duckos_dir` for the directory in which the duckOS repository is located on your computer.

## Building and running duckOS
1. Either run the following commands as root,  or use `sudo su` to open a root shell in the build directory you made earlier.
2. In your build directory, run `make duckk32` to build the kernel.
3. Run `make image` to make the root filesystem image.
3. Run `make qemu-image` to run qemu with the image you just made.
5. Enjoy!