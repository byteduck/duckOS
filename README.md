# duckOS
### A hobby operating system

This is based off of my previous hobby OS, codeOS2. I'm in the process of rewriting most of the previous code from scratch, though.

### What's working
- Booting via grub multiboot off of a hard drive
- Typing commands into the kernel shell
- Reading files off of the hard drive
- Reading devices as files (`/dev/hda`, `/dev/zero`, `/dev/random`, etc)
- Basic multitasking (But no executables)

### Known Issues / Limitations
- It kinda sucks
- Only works with ext2 filesystems with a block size of 1024  (I think)
- Uses BIOS interrupts (ATA PIO) to read/write to disk instead of AHCI/IDE (This is pretty slow)

### Building / Running
- See [INSTRUCTIONS.md](INSTRUCTIONS.md) for instructions.

### Credits
- [blanham's mirror of liballoc 1.1](https://github.com/blanham/liballoc) for the kernel heap allocation implementation (it's open domain, so if you want to use it, I highly recommend it)
- [SRombaut's implementation of shared_ptr](https://github.com/SRombauts/shared_ptr/)
- [SerenityOS](http://serenityos.org) for a lot of inspiration