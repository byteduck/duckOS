# duckOS
### A hobby operating system

This is just a little hobby OS that aims to be at least partially POSIX/UNIX compliant.

If some of this code looks familiar, it's because it's based off of my previous hobby OS, [codeOS2](https://github.com/byteduck/codeOS2). Most of the code is new/rewritten at this point though.

### What's working
- Booting off of the primary master IDE (PATA) hard drive on both emulators and real hardware (tested on a Dell Optiplex 320 with a Pentium D)
- A virtual filesystem with device files (`/dev/hda`, `/dev/zero`, `/dev/random`, `/dev/fb`, `/dev/tty`, etc)
  - The root filesystem is ext2, and is writeable
- A semicomplete newlib-based C standard library for programs (see [INSTRUCTIONS.md](INSTRUCTIONS.md))
- Preemptive Multitasking (although there isn't anything that really takes advantage of `fork` yet besides the the userspace shell `dsh`)
- A Bochs/Qemu/VirtualBox video driver (640x480x32bpp)
- Multiboot framebuffer support (requests 640x480x32bpp from the bootloader, and assumes it is such which may be problematic)
  
### Programs

The code for these can be found in [programs](programs/).

- fork (/bin/fork): A demo program that forks and then prints whether it is the child or the parent process.
- ls (/bin/ls): Lists the entries in the current or given directory.
- cat (/bin/cat): Writes the contents of a file to stdout.
- pwd (/bin/pwd): Prints the current working directory.
- mkdir (/bin/mkdir): Creates a new directory.
- rm (/bin/rm): Removes a file.
- rmdir (/bin/rmdir): Removes a directory.
- touch (/bin/touch): Creates a file.
- truncate (/bin/truncate): Resizes a file.
- dsh (/bin/dsh): A basic userspace shell.

Programs that take arguments will provide you with the correct usage when you run them without arguments.

### Ports
- DOOM ([duckos-doom](https://github.com/byteduck/duckos-doom)): A port of DOOM. A little buggy at the moment, but playable. I'm waiting to  fix the bugs until I have a proper window manager since there's no sense in dedicating time to making it work in a graphical TTY.

### Kernel shell commands
Type `help` for a list of commands that can be used in the builtin kernel shell.

### Known Issues / Limitations
- Framebuffer scrolling is slow on real hardware. I'm not focusing on fixing this right now until I start working on a window manager.
- Must be booted off of the master drive on the primary PATA controller as of now.
- PATA DMA doesn't seem to work on real hardware at the moment, so the `use_pio` grub commandline option should be specified if you're crazy enough to test this on real hardware ;)
- Binaries are statically linked, so they're larger and take up more memory than usual
- Ext2 triply indirect block pointers cannot be read/written, so there may be issues writing and reading large files

### Building / Running
- See [INSTRUCTIONS.md](INSTRUCTIONS.md) for instructions.

### Credits
- [blanham's mirror of liballoc 1.1](https://github.com/blanham/liballoc) for the kernel heap allocation implementation (it's open domain, so if you want to use it, I highly recommend it)
- [SRombaut's implementation of shared_ptr](https://github.com/SRombauts/shared_ptr/)
- [SerenityOS](http://serenityos.org) for a lot of inspiration

### License
- See [LICENSE.TXT](LICENSE.txt).
