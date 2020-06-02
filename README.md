# duckOS
### A hobby operating system

This is based off of my previous hobby OS, codeOS2. I didn't fork it, however, as the git repository had a few large unnecessary files in it.

### What's working
- Booting via grub multiboot2 off of a hard drive
- Typing commands into the terminal shell
- Basic multitasking

### Known Issues / Limitations
- It kinda sucks
- Executables can only be in a raw format as of now, meaning no ELF files
- Only works with ext2 filesystems with a block size of 1024
- Uses BIOS interrupts to read/write to disk instead of AHCI/IDE