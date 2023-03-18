# Kernel
## About
This page describes the different aspects of the duckOS kernel, including how the duckOS kernel is organized and how the duckOS kernel works.
## Organization
The duckOS kernel is organized by the most relevant code being in the root of the kernel directory, and the others being in subdirectories like `api`, `asm`, `device`, `filesystem`, `font8x8`, `interrupt`, `kstd`, `memory`, `pci`, `syscall`, `tasking`, `terminal`, `tests` and `time`.
*So, what do these different subdirectories contain?*
* The `api` directory contains some header files similar to the header files that the average standard C library contains.
* The `asm` directory however, contains assembly code which are specific to the x86 architecture.
* The `device` directory contains different drivers for hardware that isn't necessarily architecture-specific.
* The `filesystem` directory contains code for supporting different filesystems, aswell as other filesystem-specific stuff, like the implementation of index nodes, pipes, and file descriptors.
* The `font8x8` directory contains fonts embedded as arrays of bytes.
* The `interrupt` directory contains interrupt-related code
* The `kstd` directory (I'm not sure about this one!) contains code that can serve as utility for the kernel (string manipulation, C++-related support for the kernel, etc.)
* The `memory` directory contains code related to memory management like paging, global descriptor tables, and memory management.
* The `pci` directory contains code for communicating with PCI devices.
* The `syscall` directory contains code for defining syscalls.
* The `tasking` directory has code for process-related stuff like loading ELF executables, multithreading, etc.
* The `terminal` directory contains code for terminal-related stuff (obviously)
* I'm not sure exactly what the `tests` directory currently does, I still need to read the code there.
* The `time` directory contains code for managing time-related hardware, and counting time.
