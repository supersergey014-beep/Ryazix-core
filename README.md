# Ryazix OS Kernel

Ryazix is a freestanding, 32-bit x86 monolithic kernel written in C and GNU
Assembly. The repository contains the kernel bootstrap, CPU and interrupt
setup, basic memory management, round-robin kernel tasking, a small VFS, a tar
initrd reader, and an interactive text-mode shell.

## Overview

The kernel boots through a Multiboot 1 loader and enters at `_start`. It runs
in protected mode, uses the VGA text buffer for console output, and currently
maps the first 4 MiB of physical memory identity-to-identity. The implementation
is an educational kernel foundation rather than a production-ready operating
system.

## Architecture & Features

- **Phase 1 - Boot:** Multiboot 1 header, 16 KiB bootstrap stack, and linker
	layout based at 1 MiB.
- **Phase 2 - CPU architecture:** Five-entry GDT, 256-entry IDT, exception
	handlers, and normalized interrupt frames.
- **Phase 3 - Hardware:** Remapped 8259 PIC, 100 Hz PIT timer, and PS/2 keyboard
	input through IRQ handlers.
- **Phase 4 - Memory:** Bitmap physical frame allocator, x86 paging with an
	identity map of the first 4 MiB, and a coalescing 64 KiB kernel heap.
- **Phase 5 - Tasking:** Round-robin kernel tasks, timer-driven preemption,
	context switching, and `INT 0x80` write/exit system calls.
- **Phase 6 - Files and shell:** VFS file/directory nodes, a read-only ustar
	initrd parser, and shell commands: `help`, `clear`, `version`, `ls`, `cat`,
	and `echo`.

The current PMM uses the contiguous memory size reported by Multiboot rather
than parsing the full memory map. The initrd reader expects the first Multiboot
module to contain a ustar archive. The VMM currently maps only the first 4 MiB.

## Building & Running

Required tools are GCC with 32-bit code generation, GNU binutils, GNU Make, and
QEMU for emulation.

Build the kernel image:

```sh
make
```

Run it in QEMU:

```sh
make qemu
```

An optional ustar initrd can be passed as a Multiboot module:

```sh
make qemu INITRD=initrd.tar
```

Remove generated object files and the kernel image with:

```sh
make clean
```

## License

Ryazix is distributed under the GNU General Public License, version 3. See
[`LICENSE`](LICENSE) for the complete license text.