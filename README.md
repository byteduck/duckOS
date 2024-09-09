# duckOS
A hobby UNIX-like OS with a graphical window manager and applications for x86 PCs, with a [work-in-progress aarch64 port](https://github.com/byteduck/duckOS/pull/73).

![duckOS](https://github.com/byteduck/duckOS/workflows/duckOS/badge.svg)

![Screenshot](docs/screenshots/screenshot-2023-12-01.png)

[Demo](#try-duckos) | [Features](#features) | [Apps](#apps) | [Ports](#ports) | [How to Build](#building--running)

## On Hold

Due to my professional obligations, development of duckOS is currently on hold. Feel free to fork it and mess around with it though!

## Try duckOS

### Virtualize locally
A recent release can be downloaded from the [releases](https://github.com/byteduck/duckOS/releases) page. Alternatively, a disk image of the latest duckOS can be downloaded from the artifacts of the duckOS GitHub [workflow](https://github.com/byteduck/duckOS/actions/workflows/build-os.yml). This can then be virtualized or emulated using QEMU or your software of choice.

### Emulate in the browser
Alternatively, thanks to the [v86 project by copy](https://github.com/copy/v86), you can try a version of duckOS in your browser. _**This is a lot slower running it locally, and does not have copies of ported software like DOOM**_. [You can try it here!](https://aaron.sonin.me/duckOS)

## Features
- A window manager / compositor plus a themable view-based UI toolkit for creating apps
- A basic TCP/UDP over IP networking stack with unix sockets and an E1000 ethernet driver
- Sound support for AC97 sound cards
- Many ports of programs like DOOM, utilities like GCC, and libraries like SDL
- Dynamic linking and loading of binaries
- On-board debugging capabilities like ptrace and a sampling profiler
- A software 3D rendering library
- Runs on some real hardware
- A WIP aarch64 port that [boots past stage 1](https://github.com/byteduck/duckOS/pull/73#issuecomment-2270315545) on a raspberry pi 3b :)

## Future plans
- Finish aarch64 port
- Revamp kernel IPC system to be more efficient
- Better font rendering (Vector fonts, different sizes, etc.)
- self-host
- More s t a b i l i t y and s p e e d
- A better filesystem cache implementation that can free memory when needed and periodically flushes writes
- More kernel & userspace unit tests
- Better documentation of kernel, libraries, and applications
- Some more kernel & userspace debugging tools so I don't have to spend hours knee-deep in the qemu debugger whenever a segfault happens due to a simple bug that could've been avoided with some extra coffee in my system
- Multiprocessor (multicore) support
- Slowly transition various modules from the kernel to userspace (a la microkernel)
- Add Rust into the mix (?)

## Services

The code for these can be found in [services](services/).

- Init (/bin/init): The init system for duckOS.
- Pond (/bin/pond): The window manager / compositor for duckOS.
- Quack (/bin/quack): The sound server for duckOS.
- DHCP Client (/bin/dhcpclient): A DHCP client for assigning an IP address.

## Apps

The code for these can be found in [programs/applications](/programs/applications).

- Calculator (/apps/calculator.app): A basic calculator.
- Terminal (/apps/terminal.app): A libui-based terminal application.
- System Monitor (/apps/monitor.app): A basic system monitor showing memory and CPU utilization.
- 4 In a Row (/apps/4inarow.app): A basic four-in-a-row game. Play with two players or against the computer.
- Sandbar (/bin/sandbar): A basic "taskbar" that displays a row of buttons at the bottom of the screen to launch applications.
- Files (/apps/files.app): A rudimentary file explorer application.
- Viewer (apps/viewer.app): A basic media viewer that can view images and play sound files
- Lib3d Demo (apps/3demo.app): A demo for the lib3d library. Displays a cube by default; can be used to view obj files.
- Editor (/apps/editor.app): A basic app to edit text files.
- About (/apps/about.app): Shows some system information.

## CLI Programs

The code for these can be found in [programs/coreutils](programs/coreutils). Alongside the usual suspects, duckOS has:

- dsh (/bin/dsh): A basic shell with support for piping, redirections, and command recall.
- open (/bin/open): A utility to open files and applications from the command line using the appropriate program.
- play (/bin/play): Plays audio files.
- date (/bin/date): Shows the date and time.
- profile (/bin/profile): Profiles a running application and outputs a [FlameGraph](https://github.com/brendangregg/FlameGraph) / [SpeedScope](https://speedscope.app) compatible file.
  - You can run `scripts/debugd.py` on the host (with speedscope installed) and pass the `-r` parameter to profile to send the output directly to the host via networking and open it in speedscope.

Programs that take arguments will provide you with the correct usage when you run them without arguments.


## Libraries

- [libc](libraries/libc): The standard C library.
- [libm](libraries/libm): The math portion of the standard C library.
- [libpond](libraries/libpond): The library used for interfacing with the pond window manager / compositor.
- [libgraphics](libraries/libgraphics): A library which provides a few utilities for working with graphics such as image format loading.
- [libui](/libraries/libui): A UI framework for applications.
- [libtui](/libraries/libtui): A framework for terminal applications.
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

## Ports

Ports can be installed by running [ports.sh](ports/ports.sh) supplied with the desired port as an argument. The required dependencies will also be built and installed.

Some of the more exciting ports are:

- [DOOM](ports/doom)
- [binutils](ports/binutils)
- [gcc](ports/gcc)
- [sdl2](ports/sdl2)
  - Also, [sdl2_gfx](ports/sdl2_gfx), [sdl2_image](ports/sdl2_image), and [sdl2_ttf](ports/sdl2_ttf)

## Building / Running
- See [INSTRUCTIONS.md](INSTRUCTIONS.md) for instructions.

## Contributing
- See [CONTRIBUTING.md](CONTRIBUTING.md) for information on how to contribute to duckOS.

## Credits
- [blanham's mirror of liballoc 1.1](https://github.com/blanham/liballoc) for the kernel heap allocation implementation (it's open domain, so if you want to use it, I highly recommend it)
- [SerenityOS](http://serenityos.org) for a lot of inspiration
- [Gohufont](https://font.gohu.org/) for the font (licensed under [WTFPL](http://www.wtfpl.net/about/))

## License
- See [LICENSE.TXT](LICENSE.txt).
