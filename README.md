# duckOS

![duckOS](https://github.com/byteduck/duckOS/workflows/duckOS/badge.svg)

A hobby UNIX-like OS with a graphical window manager and applications for x86-based computers.

### Try duckOS

A disk image of duckOS can be downloaded from the artifacts of the duckOS GitHub [workflow](https://github.com/byteduck/duckOS/actions/workflows/build-os.yml). This can then be virtualized or emulated using QEMU or your software of choice.

Alternatively, thanks to the [v86 project](https://github.com/copy/v86) by [copy](https://copy.sh), you can try a version of duckOS in your browser. This is a lot slower running it locally, and the build will usually be a bit out-of-date. You can try it [here](https://aaron.sonin.me/duckOS)!

### Screenshots
![Screenshot](docs/screenshots/screenshot-2023-12-01.png)

### What's working
- Booting off of the primary master IDE (PATA) hard drive on both emulators and real hardware (tested on a Dell Optiplex 320 with a Pentium D)
- PATA DMA or PIO access (force PIO by using the `use_pio` grub kernel argument)
- A virtual filesystem with device files (`/dev/hda`, `/dev/zero`, `/dev/random`, `/dev/fb`, `/dev/tty`, etc)
  - The root filesystem is ext2, and is writeable
- Disk caching
- Dynamic linking with shared libraries
- A Bochs/Qemu/VirtualBox/Multiboot video driver (640x480x32bpp)
- A window manager / compositor called pond
- Various GUI applications
- Ports of binutils and gcc
- More!

### Future plans
#### Stuff that may happen soon™.
- ~~Revamp virtual memory system so it's not as error-prone and easier to debug (named regions, fix race conditions, faster allocation, etc.)~~ **Done!**
- Revamp kernel IPC system to be more efficient (Replace SocketFS with something like Mach or MINIX's grant-based IPC)
- Better font rendering (Vector fonts, different sizes, etc.)
- ~~Finish porting GCC~~, self-host
- More s t a b i l i t y and s p e e d
- A better filesystem cache implementation that can free memory when needed and periodically flushes writes
- More kernel & userspace unit tests
- Better documentation of kernel, libraries, and applications
- Some more kernel & userspace debugging tools so I don't have to spend hours knee-deep in the qemu debugger whenever a segfault happens due to a simple bug that could've been avoided with some extra coffee in my system

### Lofty goals
#### Maybe someday.
- Multiprocessor (multicore) support
- x64 / ARM support (?)
- Slowly transition various modules from the kernel to userspace (ie microkernel)
- Add Rust into the mix (?)
- Network support
 
### Services

The code for these can be found in [services](services/).

- Init (/bin/init): The init system for duckOS.
- Pond (/bin/pond): The window manager / compositor for duckOS.
- Quack (/bin/quack): The sound server for duckOS.

### GUI Programs

The code for these can be found in [programs](/programs)

- Calculator (/apps/calculator.app): A basic calculator.
- Terminal (/apps/terminal.app): A libui-based terminal application.
- System Monitor (/apps/monitor.app): A basic system monitor showing memory and CPU utilization.
- 4 In a Row (/apps/4inarow.app): A basic four-in-a-row game. Play with two players or against the computer.
- Sandbar (/bin/sandbar): A basic "taskbar" that displays a row of buttons at the bottom of the screen to launch applications.
- Files (/apps/files.app): A rudimentary file explorer application.
- Viewer (apps/viewer.app): A basic image viewer.
- Lib3d Demo (apps/3demo.app): A demo for the lib3d library. Displays a cube by default; can be used to view obj files.
- Editor (/apps/editor.app): A basic app to edit text files. 

### CLI Programs

The code for these can be found in [programs](programs/).

- ls (/bin/ls): Lists the entries in the current or given directory.
- cat (/bin/cat): Writes the contents of a file to stdout.
- cp (/bin/cp): Copies a file.
- mv (/bin/mv): Moves a file.
- pwd (/bin/pwd): Prints the current working directory.
- mkdir (/bin/mkdir): Creates a new directory.
- echo (/bin/echo): Prints the arguments given to it separated by spaces to stdout.
- rm (/bin/rm): Removes a file.
- ln (/bin/ln): Creates a hard or symbolic link to a file or directory.
- rmdir (/bin/rmdir): Removes a directory.
- touch (/bin/touch): Updates the access/modification times of a file or creates it if it doesn't exist.
- truncate (/bin/truncate): Resizes a file.
- chmod (/bin/chmod): Changes the mode of a file.
- chown (/bin/chown): Changes the owner of a file.
- free (/bin/free): Shows the amount of total, used, and free memory (use the -h flag for human-readable numbers).
- ps (/bin/ps): Shows the currently running processes.
- dsh (/bin/dsh): A basic userspace shell with support for pipes (`|`) and redirections (`>`/`>>`).
  - There is only support for one redirection at a time right now.
- open (/bin/open): A utility to open files and applications from the command line.
- play (/bin/play): Plays audio files.
- profile (/bin/profile): Profiles a running application and outputs a [FlameGraph](https://github.com/brendangregg/FlameGraph) / [SpeedScope](https://speedscope.app) compatible file.

Programs that take arguments will provide you with the correct usage when you run them without arguments.


### Libraries

- [libc](libraries/libc): The standard C library.
- [libm](libraries/libm): The math portion of the standard C library.
- [libpond](libraries/libpond): The library used for interfacing with the pond window manager / compositor.
- [libgraphics](libraries/libgraphics): A library which provides a few utilities for working with graphics such as image format loading.
- [libui](/libraries/libui): A UI framework for applications.
- [libterm](/libraries/libterm): A framework for handling terminals.
- [libduck](/libraries/libduck): A library containing commonly used classes and utilities, such as argument and configuration file parsing.
- [libriver](/libraries/libriver): An IPC library not dissimilar to D-Bus, which provides a framework for remote function calls and data passing.
- [libapp](/libraries/libapp): A library providing methods to retrieve information about installed and running applications.
- [libsys](/libraries/libsys): Provides higher-level C++ abstractions for retrieving system information, namely from procFS.
- [libsound](/libraries/libsound): Provides a framework for audio applications and interfacing with the sound server, Quack.
- [lib3d](/libraries/lib3d): Provides basic software 3D rendering functionality.
- [libmatrix](/libraries/libmatrix): Provides matrix math utilities.
- [libexec](/libraries/libexec): Provides ELF support.
- [libdebug](/libraries/libdebug): Provides debugging functionality.

### Ports

Ports can be installed by running [ports.sh](ports/ports.sh) supplied with the desired port as an argument. The required dependencies will also be built and installed.

- [DOOM](ports/doom)
- [binutils](ports/binutils)
- [gcc](ports/gcc)

### Building / Running
- See [INSTRUCTIONS.md](INSTRUCTIONS.md) for instructions.

### Contributing
- See [CONTRIBUTING.md](CONTRIBUTING.md) for information on how to contribute to duckOS.

### Credits
- [blanham's mirror of liballoc 1.1](https://github.com/blanham/liballoc) for the kernel heap allocation implementation (it's open domain, so if you want to use it, I highly recommend it)
- [SerenityOS](http://serenityos.org) for a lot of inspiration
- [Gohufont](https://font.gohu.org/) for the font (licensed under [WTFPL](http://www.wtfpl.net/about/))

### License
- See [LICENSE.TXT](LICENSE.txt).
