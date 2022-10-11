# duckOS Build / Run instructions

## Prerequisites

### Dependencies
- A Linux environment (WSL will work, but WSL2 is preferred because it is much faster than WSL1)
- GCC
- Nasm
- Make
- CMake
- Bison
- Flex
- GMP
- MPFR
- MPC
- Texinfo
- Grub
- Gawk
- QEmu (For emulating)
- Libtool

### Installing dependencies
- Ubuntu/Debian: `apt install build-essential cmake bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo qemu-system-i386 qemu-utils nasm gawk grub2-common grub-pc rsync`
- Arch: `pacman -S base-devel cmake gmp libmpc mpfr qemu qemu-arch-extra qemu-ui-gtk nasm grub rsync texinfo`
- Fedora: `dnf install @development-tools grub2-tools-extra cmake gmp-devel mpfr-devel libmpc-devel qemu qemu-system-x86 nasm rsync texinfo` and `dnf group install "C Development Tools and Libraries"`
- macOS: Install Xcode and launch it to install the developer tools. Then, run `brew install coreutils e2fsprogs qemu bash gcc@11 cmake genext2fs nasm rsync`
  - You must also install [macFUSE](https://osxfuse.github.io) and `fuse-ext2`
    - The latest version of FUSE-ext2 can be built by running `toolchain/build-ext2-fuse.sh`. Alternatively, an older binary version is available [here](https://github.com/gpz500/fuse-ext2/releases)
  - If you are on an Apple Silicon Mac, you may need to set the `CPATH=/opt/homebrew/include` and `LIBRARY_PATH=/opt/homebrew/lib` environment variables to get the toolchain to build.
## Building the toolchain
1. Open the `toolchain` directory in your terminal and run `build-toolchain.sh`. (You will need an internet connection as it downloads the needed binutils/gcc releases from the GNU ftp site.)
2. Make a cup of coffee or tea and wait. It will take a while to compile.
3. Once it's done, the toolchain will be in `toolchain/tools`, and the sysroot in `cmake-build/root`.

### Editing the toolchain
If you'd like to edit the c library, you can run `build-toolchain.sh libc` to recompile libc and libstdc++. If you just want to compile libc and not libstdc++, you can run `make libc` in the `cmake-build` folder.

If you'd like to edit gcc or binutils, you can run the `edit-toolchain.sh` script to download patch binutils and gcc and setup a git repository for each one. Then, use the `gen-patches.sh` script to generate patch files for each one.

**DO NOT** git commit in the repositories created by `edit-toolchain` or else `gen-patches` won't work properly.

To build something from the `edit` directory, pass the `edited-[thing]` to the `build-toolchain.sh` script. (ex: `build-toolchain.sh edited-gcc` to build gcc from the edit directory)

## Configuring cmake
1. Make sure you've built the toolchain first.
2. Go to the `cmake-build` directory.
3. From that directory, run `cmake .. -DCMAKE_TOOLCHAIN_FILE=toolchain/CMakeToolchain.txt`.

## Building and running duckOS
1. In the `cmake-build` directory, run `make install` to build the kernel & programs.
2. Run `make image` to make the disk image.
4. Run `make qemu` to run qemu with the image you just made.
5. Enjoy!

## Running kernel unit tests
To run kernel unit tests, run `make install` and `make image` as usual, and then use `make tests` to run tests. Instead of running init, the kernel will run unit tests after booting.

Alternatively, supply the `kernel-tests` kernel argument to run tests.
