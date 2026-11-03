# atari-sdk

An SDK for Atari ST, STe, TT and Falcon programs (TOS 1.0 to 4.x and
EmuTOS, every 68k CPU) built with clang from
[clang-dos/llvm](https://github.com/clang-dos/llvm)
(`--target=m68k-atari-tos`). It has:

- the C library: [picolibc](https://github.com/clang-dos/picolibc) with a
  TOS layer (startup code, `malloc`, files, directories, time), and the
  compiler-rt builtins, all built for the 68000;
- `<osbind.h>`: every GEMDOS, BIOS and XBIOS call, including the TT and
  Falcon video calls;
- `<aes.h>`, `<vdi.h>` and `<gem.h>`: the GEM AES and VDI, as functions in
  `libgem.a`, with gemlib's names and arguments;
- `<natfeats.h>`: Native Features, to print to the host from a program
  running in Hatari or ARAnyM;
- examples, and scripts to run programs on EmuTOS in Hatari.

Everything here was written for this project from the public descriptions
of the TOS calls and is under the BSD license, so the whole SDK can be
redistributed. Nothing from Atari's TOS, its ROMs or its development kits
is used. Tests run on [EmuTOS](https://emutos.sourceforge.io), the free
(GPL) TOS replacement, so no Atari ROM is needed either.

## Using it

The clang-dos toolchain package has the sysroot in `m68k-atari-tos/`,
next to `bin/`. Then:

    clang --target=m68k-atari-tos -O2 hello.c -o hello.tos
    clang --target=m68k-atari-tos -O2 gemhello.c -o gemhello.prg
    clang --target=m68k-atari-tos -mcpu=68030 -O2 game.c -o game.prg

The driver links `crt0.o`, `libc.a`, `libgem.a` and the builtins with
`ld.lld --emit-relocs` and the sysroot's `tos.ld`, and converts the result
to a TOS program with `llvm-elf2tos`. The output is a TOS executable
whatever its name; the extension tells the desktop how to start it:
`.PRG` for GEM programs, `.TOS` for text programs, `.TTP` for text
programs that ask for arguments. Code is for the 68000 unless `-mcpu=`
says otherwise; the libraries are 68000 code, which runs on every CPU.

The ABI is that of gcc for the Atari (`m68k-atari-mint`): `int` is 32
bits, everything of 2 bytes or more is aligned to 2, values and pointers
are returned in `d0`, the struct-return pointer is passed in `a1`, and
`a6` is the frame pointer. `long double` is the same as `double` for now.
There is no FPU code generation yet: floating point is done in software,
which works on every machine.

### C programs

- `main(argc, argv, envp)` gets the command line, split at spaces with
  `"` and `'` quoting, or the full arguments when the caller uses the
  `ARGV` environment convention. `argv[0]` is empty unless `ARGV` gives
  it. `environ` is the GEMDOS environment.
- The program keeps its text, data, bss and a 32K stack, and gives the
  rest of memory back to GEMDOS. Define `long _stksize = 65536;` for
  another stack size (0 or less keeps all of memory, with the stack at its
  top).
- `malloc` works from one block, taken from GEMDOS the first time it
  needs memory: all of the largest free block but 64K, or
  `long _heapsize = ...;` bytes. A program that starts other programs
  should set `_heapsize`, to leave them room.
- `printf`, files (`fopen`, `open`, `stat`, `rename`, `mkdir`, `chdir`,
  `getcwd`, ...) and time (`time`, `clock_gettime`, `clock`, `nanosleep`)
  work as in POSIX. Paths are GEMDOS paths (`C:\DIR\FILE.TXT`); `/` is
  taken as `\`. Files are binary: there is no CR LF translation, except
  that writing `\n` to the console writes `\r\n`.
- `stdout` is GEMDOS handle 1 and `stdin` handle 0; `stderr` is the
  console (handle -1), since GEMDOS handle 2 is the serial port.
- Time is GEMDOS local time, taken as UTC, to 2 seconds, with the 200 Hz
  timer for the fraction. `clock()` and `CLOCK_MONOTONIC` use the 200 Hz
  timer.

### OS calls

`#include <osbind.h>` and call `Cconws("Hello\r\n")`, `Fopen()`,
`Physbase()`, `Supexec()` and so on, as with other Atari C compilers. The
calls are macros that push the arguments in the sizes TOS expects and call
`trap #1` (GEMDOS), `#13` (BIOS) or `#14` (XBIOS). `<mint/osbind.h>` is
the same header. Low memory (system variables, the cookie jar) can only be
read in supervisor mode: use `Supexec()`, as `examples/sysinfo.c` does.

### GEM

`#include <gem.h>` (or `<aes.h>` and `<vdi.h>`) for windows, menus,
dialogs, alerts and VDI graphics; see `examples/gemhello.c`. The functions
are in `libgem.a`, which the driver links when it is in the sysroot.

## Running programs

`tools/hatari-run.sh` runs a program on EmuTOS in
[Hatari](https://hatari.tuxfamily.org), headless, and prints what it
writes to the console:

    EMUTOS_DIR=~/emutos tools/hatari-run.sh -m st -w "Hello" hello.tos
    tools/hatari-run.sh -m falcon -w "alert" -s shot.png gemhello.prg

`EMUTOS_DIR` is a directory with EmuTOS 1.4 ROM images (`etos192us.img`,
`etos256us.img`, `etos512us.img`), from the `emutos-192k`, `-256k` and
`-512k` downloads at https://emutos.sourceforge.io. The program is copied
to drive C: (a host directory) and started by the desktop at boot.

`tests/run-tests.sh` builds the examples at several optimization levels
for the 68000 and 68030 and runs them on an emulated ST, STe, TT and
Falcon.

## Building the sysroot

    BIN=/path/to/clang-dos/bin LLVM_SRC=llvm PICOLIBC_SRC=picolibc \
        ./build-sysroot.sh m68k-atari-tos

`tools/gen-osbind.py` writes `include/osbind.h` from its tables.

## What's not there yet

- FPU code (68881/68882 and the 68040/68060 FPUs).
- Desk accessories (`.ACC`): the startup code handles programs only.
- MiNT: this is plain TOS. MiNT runs TOS programs, but the C library
  doesn't use MiNT's calls (`Fopen` long names work through GEMDOS).
- C++ (libc++).
