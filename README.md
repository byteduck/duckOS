# duckOS
### A hobby operating system

This is just a little hobby OS that aims to be at least partially POSIX/UNIX compliant.

If some of this code looks familiar, it's because it's based off of my previous hobby OS, [codeOS2](https://github.com/byteduck/codeOS2). Most of the code is new/rewritten at this point though.

### What's working
- Booting off of an IDE (PATA) hard drive on both emulators and real hardware (tested on a Dell Optiplex 320 with a Pentium D)
- A virtual filesystem with device files (`/dev/hda`, `/dev/zero`, `/dev/random`, `/dev/fb`, `/dev/tty`, etc)
  - The root filesystem is ext2
- A semicomplete newlib-based C standard library for programs (see [INSTRUCTIONS.md](INSTRUCTIONS.md))
- Preemptive Multitasking (although there isn't anything that really takes advantage of `fork` yet besides the the userspace shell `dsh`)
- A Bochs/Qemu/VirtualBox video driver (640x480x32bpp)
- Multiboot framebuffer support (requests 640x480x32bpp from the bootloader, and assumes it is such which may be problematic)
  
### Programs

The code for these can be found in [programs](programs/).

- mirror (/bin/mirror): A demo program that prompts you to type something and then prints it back to you.
- dummy (/bin/dummy): A demo program that does nothing and exits.
- fork (/bin/fork): A demo program that forks and then prints whether it is the child or the parent process.
- ls (/bin/ls): Lists the entries in the current or given directory.
- cat (/bin/cat): Writes the contents of a file to stdout.
- pwd (/bin/pwd): Prints the current working directory.
- dsh (/bin/dsh): A basic userspace shell.

### Ports
- DOOM ([duckos-doom](https://github.com/byteduck/duckos-doom)): A port of DOOM.

### Known Issues / Limitations
- Only works with ext2 filesystems with a block size of 1024  (I think, I need to test this)
- No ext2 filesystem write support (files can only be read)
- Uses ATA PIO mode to read/write to disk instead of DMA, which can be pretty slow, especially on real hardware
- File locking uses spinlocks, which wastes CPU time

### Building / Running
- See [INSTRUCTIONS.md](INSTRUCTIONS.md) for instructions.

### Credits
- [blanham's mirror of liballoc 1.1](https://github.com/blanham/liballoc) for the kernel heap allocation implementation (it's open domain, so if you want to use it, I highly recommend it)
- [SRombaut's implementation of shared_ptr](https://github.com/SRombauts/shared_ptr/)
- [SerenityOS](http://serenityos.org) for a lot of inspiration

### License
- See [LICENSE.TXT](LICENSE.txt).
