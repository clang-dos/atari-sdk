#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, clang-dos
"""Write include/osbind.h, the GEMDOS, BIOS and XBIOS bindings.

Each call is a macro that builds its argument block (the function number
and the arguments, as 16-bit words and 32-bit longs) and passes it to
__tos_trap1, __tos_trap13 or __tos_trap14, which copy it to the stack and
call the trap. The tables below are the whole interface, written from the
public descriptions of the Atari TOS calls.
"""
import sys

# name, trap, function number, return type, [(size, name, type)], comment
# size is 'w' (16-bit word) or 'l' (32-bit long); a fixed value is given
# as ('w', '0').
W, L = 'w', 'l'
GEMDOS = [
    ('Pterm0', 0x00, 'void', [], 'end the program with status 0'),
    ('Cconin', 0x01, 'long', [], 'read a key from the console, with echo'),
    ('Cconout', 0x02, 'void', [(W, 'c')], 'write a character to the console'),
    ('Cauxin', 0x03, 'short', [], 'read a character from the serial port'),
    ('Cauxout', 0x04, 'void', [(W, 'c')], 'write a character to the serial port'),
    ('Cprnout', 0x05, 'short', [(W, 'c')], 'write a character to the printer'),
    ('Crawio', 0x06, 'long', [(W, 'c')], 'raw console input (c = 0xff) or output'),
    ('Crawcin', 0x07, 'long', [], 'read a key, no echo, no Ctrl-C check'),
    ('Cnecin', 0x08, 'long', [], 'read a key, no echo'),
    ('Cconws', 0x09, 'short', [(L, 's')], 'write a string to the console'),
    ('Cconrs', 0x0a, 'void', [(L, 'buf')], 'read an edited line from the console'),
    ('Cconis', 0x0b, 'short', [], 'is a key waiting?'),
    ('Dsetdrv', 0x0e, 'long', [(W, 'drv')], 'set the current drive; returns the drive map'),
    ('Cconos', 0x10, 'short', [], 'can the console take a character?'),
    ('Cprnos', 0x11, 'short', [], 'can the printer take a character?'),
    ('Cauxis', 0x12, 'short', [], 'is a serial character waiting?'),
    ('Cauxos', 0x13, 'short', [], 'can the serial port take a character?'),
    ('Maddalt', 0x14, 'long', [(L, 'start'), (L, 'size')], 'add alternate RAM (TOS 2.06+)'),
    ('Srealloc', 0x15, 'long', [(L, 'size')], 'reserve screen memory (TOS 4)'),
    ('Dgetdrv', 0x19, 'short', [], 'the current drive (0 = A:)'),
    ('Fsetdta', 0x1a, 'void', [(L, 'dta')], 'set the disk transfer area'),
    ('Super', 0x20, 'long', [(L, 'stack')], 'enter or leave supervisor mode'),
    ('Tgetdate', 0x2a, 'unsigned short', [], 'the date: (year-1980)<<9 | month<<5 | day'),
    ('Tsetdate', 0x2b, 'short', [(W, 'date')], 'set the date'),
    ('Tgettime', 0x2c, 'unsigned short', [], 'the time: hour<<11 | minute<<5 | second/2'),
    ('Tsettime', 0x2d, 'short', [(W, 'time')], 'set the time'),
    ('Fgetdta', 0x2f, 'void *', [], 'the disk transfer area'),
    ('Sversion', 0x30, 'unsigned short', [], 'the GEMDOS version, bytes swapped'),
    ('Ptermres', 0x31, 'void', [(L, 'keep'), (W, 'code')], 'end the program, keeping memory'),
    ('Dfree', 0x36, 'short', [(L, 'info'), (W, 'drv')], 'free space on a drive (0 = current)'),
    ('Dcreate', 0x39, 'short', [(L, 'path')], 'make a directory'),
    ('Ddelete', 0x3a, 'short', [(L, 'path')], 'delete an empty directory'),
    ('Dsetpath', 0x3b, 'short', [(L, 'path')], 'set the current directory'),
    ('Fcreate', 0x3c, 'long', [(L, 'name'), (W, 'attr')], 'create or truncate a file'),
    ('Fopen', 0x3d, 'long', [(L, 'name'), (W, 'mode')], 'open a file: 0 read, 1 write, 2 both'),
    ('Fclose', 0x3e, 'short', [(W, 'handle')], 'close a file'),
    ('Fread', 0x3f, 'long', [(W, 'handle'), (L, 'count'), (L, 'buf')], 'read from a file'),
    ('Fwrite', 0x40, 'long', [(W, 'handle'), (L, 'count'), (L, 'buf')], 'write to a file'),
    ('Fdelete', 0x41, 'short', [(L, 'name')], 'delete a file'),
    ('Fseek', 0x42, 'long', [(L, 'offset'), (W, 'handle'), (W, 'mode')], 'move in a file: 0 start, 1 here, 2 end'),
    ('Fattrib', 0x43, 'short', [(L, 'name'), (W, 'wflag'), (W, 'attr')], 'get (wflag 0) or set file attributes'),
    ('Mxalloc', 0x44, 'void *', [(L, 'size'), (W, 'mode')], 'allocate ST or TT RAM (TOS 2.06+)'),
    ('Fdup', 0x45, 'short', [(W, 'handle')], 'duplicate a standard handle'),
    ('Fforce', 0x46, 'short', [(W, 'stdh'), (W, 'nonstdh')], 'redirect a standard handle'),
    ('Dgetpath', 0x47, 'short', [(L, 'buf'), (W, 'drv')], 'the current directory of a drive'),
    ('Malloc', 0x48, 'void *', [(L, 'size')], 'allocate memory; -1 gives the largest free block'),
    ('Mfree', 0x49, 'short', [(L, 'block')], 'free memory'),
    ('Mshrink', 0x4a, 'short', [(W, '0'), (L, 'block'), (L, 'size')], 'shrink a memory block'),
    ('Pexec', 0x4b, 'long', [(W, 'mode'), (L, 'name'), (L, 'cmdline'), (L, 'env')], 'load and run a program'),
    ('Pterm', 0x4c, 'void', [(W, 'code')], 'end the program'),
    ('Fsfirst', 0x4e, 'short', [(L, 'pattern'), (W, 'attr')], 'find the first matching file'),
    ('Fsnext', 0x4f, 'short', [], 'find the next matching file'),
    ('Frename', 0x56, 'short', [(W, '0'), (L, 'oldname'), (L, 'newname')], 'rename a file'),
    ('Fdatime', 0x57, 'short', [(L, 'timeptr'), (W, 'handle'), (W, 'wflag')], 'get (wflag 0) or set a file date and time'),
]
BIOS = [
    ('Getmpb', 0, 'void', [(L, 'mpb')], 'the memory parameter block'),
    ('Bconstat', 1, 'short', [(W, 'dev')], 'is a character waiting on a device?'),
    ('Bconin', 2, 'long', [(W, 'dev')], 'read a character from a device'),
    ('Bconout', 3, 'long', [(W, 'dev'), (W, 'c')], 'write a character to a device'),
    ('Rwabs', 4, 'long', [(W, 'rwflag'), (L, 'buf'), (W, 'count'), (W, 'recno'), (W, 'dev')], 'read or write disk sectors'),
    ('Setexc', 5, 'void *', [(W, 'vecnum'), (L, 'vec')], 'set an exception vector (vec -1 reads it)'),
    ('Tickcal', 6, 'long', [], 'milliseconds per system timer tick'),
    ('Getbpb', 7, 'void *', [(W, 'dev')], 'the BIOS parameter block of a drive'),
    ('Bcostat', 8, 'long', [(W, 'dev')], 'can a device take a character?'),
    ('Mediach', 9, 'long', [(W, 'dev')], 'has the disk in a drive changed?'),
    ('Drvmap', 10, 'unsigned long', [], 'bit map of the drives'),
    ('Kbshift', 11, 'long', [(W, 'mode')], 'the shift key state (mode -1 reads it)'),
]
XBIOS = [
    ('Initmous', 0, 'void', [(W, 'type'), (L, 'param'), (L, 'vec')], 'set up the mouse'),
    ('Ssbrk', 1, 'void *', [(W, 'count')], 'reserve memory at the top of RAM (boot only)'),
    ('Physbase', 2, 'void *', [], 'the screen the video hardware shows'),
    ('Logbase', 3, 'void *', [], 'the screen the system draws on'),
    ('Getrez', 4, 'short', [], 'the screen resolution: 0 low, 1 medium, 2 high'),
    ('Setscreen', 5, 'void', [(L, 'log'), (L, 'phys'), (W, 'rez')], 'set the screens and resolution (-1 keeps)'),
    ('Setpalette', 6, 'void', [(L, 'palette')], 'set all 16 colours at the next VBL'),
    ('Setcolor', 7, 'short', [(W, 'index'), (W, 'color')], 'set one colour (-1 reads it)'),
    ('Floprd', 8, 'short', [(L, 'buf'), (L, 'filler'), (W, 'dev'), (W, 'sect'), (W, 'track'), (W, 'side'), (W, 'count')], 'read floppy sectors'),
    ('Flopwr', 9, 'short', [(L, 'buf'), (L, 'filler'), (W, 'dev'), (W, 'sect'), (W, 'track'), (W, 'side'), (W, 'count')], 'write floppy sectors'),
    ('Flopfmt', 10, 'short', [(L, 'buf'), (L, 'filler'), (W, 'dev'), (W, 'spt'), (W, 'track'), (W, 'side'), (W, 'interleave'), (L, 'magic'), (W, 'virgin')], 'format a floppy track'),
    ('Midiws', 12, 'void', [(W, 'count'), (L, 'buf')], 'write count+1 bytes to MIDI'),
    ('Mfpint', 13, 'void', [(W, 'number'), (L, 'vec')], 'set an MFP interrupt vector'),
    ('Iorec', 14, 'void *', [(W, 'dev')], 'the input buffer record of a device'),
    ('Rsconf', 15, 'long', [(W, 'baud'), (W, 'ctrl'), (W, 'ucr'), (W, 'rsr'), (W, 'tsr'), (W, 'scr')], 'set up the serial port (-1 keeps)'),
    ('Keytbl', 16, 'void *', [(L, 'unshift'), (L, 'shift'), (L, 'capslock')], 'set the keyboard tables (-1 keeps)'),
    ('Random', 17, 'long', [], 'a 24-bit pseudo-random number'),
    ('Protobt', 18, 'void', [(L, 'buf'), (L, 'serialno'), (W, 'disktype'), (W, 'execflag')], 'make a boot sector'),
    ('Flopver', 19, 'short', [(L, 'buf'), (L, 'filler'), (W, 'dev'), (W, 'sect'), (W, 'track'), (W, 'side'), (W, 'count')], 'verify floppy sectors'),
    ('Scrdmp', 20, 'void', [], 'print the screen'),
    ('Cursconf', 21, 'short', [(W, 'func'), (W, 'rate')], 'set up the text cursor'),
    ('Settime', 22, 'void', [(L, 'time')], 'set the keyboard clock (DOS date << 16 | time)'),
    ('Gettime', 23, 'unsigned long', [], 'the keyboard clock (DOS date << 16 | time)'),
    ('Bioskeys', 24, 'void', [], 'restore the default keyboard tables'),
    ('Ikbdws', 25, 'void', [(W, 'count'), (L, 'buf')], 'write count+1 bytes to the keyboard processor'),
    ('Jdisint', 26, 'void', [(W, 'number')], 'disable an MFP interrupt'),
    ('Jenabint', 27, 'void', [(W, 'number')], 'enable an MFP interrupt'),
    ('Giaccess', 28, 'char', [(W, 'data'), (W, 'reg')], 'read or write a sound chip register'),
    ('Offgibit', 29, 'void', [(W, 'mask')], 'clear bits of sound chip port A'),
    ('Ongibit', 30, 'void', [(W, 'mask')], 'set bits of sound chip port A'),
    ('Xbtimer', 31, 'void', [(W, 'timer'), (W, 'ctrl'), (W, 'data'), (L, 'vec')], 'set up an MFP timer'),
    ('Dosound', 32, 'void', [(L, 'list')], 'play a sound chip command list'),
    ('Setprt', 33, 'short', [(W, 'config')], 'set up the printer (-1 reads it)'),
    ('Kbdvbase', 34, 'void *', [], 'the keyboard vector table'),
    ('Kbrate', 35, 'short', [(W, 'initial'), (W, 'repeat')], 'set the key repeat rate (-1 keeps)'),
    ('Prtblk', 36, 'short', [(L, 'params')], 'print a block of the screen'),
    ('Vsync', 37, 'void', [], 'wait for the next vertical blank'),
    ('Supexec', 38, 'long', [(L, 'func')], 'call a function in supervisor mode'),
    ('Puntaes', 39, 'void', [], 'throw the AES away (reboots)'),
    ('Floprate', 41, 'short', [(W, 'dev'), (W, 'rate')], 'set the floppy step rate (TOS 1.4+)'),
    ('Blitmode', 64, 'short', [(W, 'mode')], 'use the blitter (-1 reads the mode)'),
    # TT
    ('EsetShift', 80, 'short', [(W, 'mode')], 'TT: set the video mode'),
    ('EgetShift', 81, 'short', [], 'TT: the video mode'),
    ('EsetBank', 82, 'short', [(W, 'bank')], 'TT: set the palette bank'),
    ('EsetColor', 83, 'short', [(W, 'index'), (W, 'color')], 'TT: set a colour'),
    ('EsetPalette', 84, 'void', [(W, 'index'), (W, 'count'), (L, 'palette')], 'TT: set colours'),
    ('EgetPalette', 85, 'void', [(W, 'index'), (W, 'count'), (L, 'palette')], 'TT: read colours'),
    ('EsetGray', 86, 'short', [(W, 'mode')], 'TT: grey mode'),
    ('EsetSmear', 87, 'short', [(W, 'mode')], 'TT: smear mode'),
    # Falcon
    ('VsetMode', 88, 'short', [(W, 'mode')], 'Falcon: set the video mode (-1 reads it)'),
    ('VgetMonitor', 89, 'short', [], 'Falcon: the monitor type'),
    ('VsetSync', 90, 'void', [(W, 'flag')], 'Falcon: external sync'),
    ('VgetSize', 91, 'long', [(W, 'mode')], 'Falcon: screen size in bytes of a mode'),
    ('VsetRGB', 93, 'void', [(W, 'index'), (W, 'count'), (L, 'rgb')], 'Falcon: set colours'),
    ('VgetRGB', 94, 'void', [(W, 'index'), (W, 'count'), (L, 'rgb')], 'Falcon: read colours'),
    ('VsetMask', 150, 'void', [(L, 'ormask'), (L, 'andmask'), (W, 'overlay')], 'Falcon: overlay mask'),
]


def macro(name, trap, num, ret, args, comment):
    params = [a[1] for a in args if not a[1].isdigit()]
    fields = ['short __f;']
    inits = [str(num)]
    for i, (size, an) in enumerate(args):
        t = 'short' if size == W else 'long'
        fields.append('%s __a%d;' % (t, i))
        inits.append(an if an.isdigit() else '(%s)(%s)' % (t, an))
    call = '__tos_trap%d(&__b, sizeof(__b))' % trap
    if ret == 'void':
        body = '(void)%s' % call
    else:
        body = '(%s)%s' % (ret, call)
    lines = ['/* %s */' % comment,
             '#define %s(%s) \\' % (name, ', '.join(params)),
             '    __extension__({ \\',
             '        struct __attribute__((__packed__, __aligned__(2))) { %s } __b = { %s }; \\'
             % (' '.join(fields), ', '.join(inits)),
             '        %s; \\' % body,
             '    })']
    return '\n'.join(lines)


HEAD = r'''/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, clang-dos
 *
 * osbind.h: Atari TOS GEMDOS (trap #1), BIOS (trap #13) and XBIOS
 * (trap #14) calls. Generated by tools/gen-osbind.py; edit that instead.
 *
 * Each call pushes the function number and its arguments, in the sizes
 * TOS expects, calls the trap and pops them. TOS keeps d3-d7 and a3-a6.
 * Pointer arguments may be given as pointers or as longs. Calls added
 * after TOS 1.0 are marked with the TOS (or machine) they need.
 */
#ifndef _OSBIND_H
#define _OSBIND_H

/* Copy an argument block to the stack, call the trap and pop it. size is
 * always even. */
#define __TOS_TRAP_FN(n)                                                    \
    static __inline__ __attribute__((__always_inline__)) long __tos_trap##n( \
        const void *__blk, unsigned long __size)                            \
    {                                                                       \
        register long __r __asm__("d0");                                    \
        __asm__ __volatile__("suba.l %2,%%sp\n\t"                           \
                             "move.l %%sp,%%a1\n\t"                         \
                             "move.l %1,%%a0\n\t"                           \
                             "move.l %2,%%d1\n"                             \
                             "1:\tmove.w (%%a0)+,(%%a1)+\n\t"               \
                             "sub.l #2,%%d1\n\t"                            \
                             "bne 1b\n\t"                                   \
                             "trap #" #n "\n\t"                             \
                             "adda.l %2,%%sp"                               \
                             : "=&r"(__r)                                   \
                             : "r"(__blk), "r"(__size)                      \
                             : "d1", "d2", "a0", "a1", "a2", "cc", "memory"); \
        return __r;                                                         \
    }
__TOS_TRAP_FN(1)
__TOS_TRAP_FN(13)
__TOS_TRAP_FN(14)
#undef __TOS_TRAP_FN

/* BIOS devices. */
#define _PRT 0 /* printer */
#define _AUX 1 /* serial port */
#define _CON 2 /* console (VT52 screen and keyboard) */
#define _MIDI 3
#define _IKBD 4 /* keyboard processor */
#define _RAWCON 5 /* screen without VT52 escapes */

/* Fsfirst() attributes. */
#define FA_READONLY 0x01
#define FA_HIDDEN 0x02
#define FA_SYSTEM 0x04
#define FA_VOLUME 0x08
#define FA_SUBDIR 0x10
#define FA_ARCHIVE 0x20

/* Fopen() modes. */
#define S_READ 0
#define S_WRITE 1
#define S_READWRITE 2

/* Pexec() modes. */
#define PE_LOADGO 0
#define PE_LOAD 3
#define PE_GO 4
#define PE_BASEPAGE 5
#define PE_GOTHENFREE 6

/* Mxalloc() modes. */
#define MX_STRAM 0
#define MX_TTRAM 1
#define MX_PREFSTRAM 2
#define MX_PREFTTRAM 3

/* The disk transfer area of Fsfirst() and Fsnext(). */
typedef struct {
    char d_reserved[21];
    unsigned char d_attrib;
    unsigned short d_time;
    unsigned short d_date;
    unsigned long d_length;
    char d_fname[14];
} _DTA;

/* Dfree(). */
typedef struct {
    unsigned long b_free;
    unsigned long b_total;
    unsigned long b_secsiz;
    unsigned long b_clsiz;
} _DISKINFO;

/* Fdatime(). */
typedef struct {
    unsigned short time;
    unsigned short date;
} _DOSTIME;

/* The basepage of a program. _base points to the program's own. */
typedef struct _basepage {
    void *p_lowtpa;
    void *p_hitpa;
    void *p_tbase;
    long p_tlen;
    void *p_dbase;
    long p_dlen;
    void *p_bbase;
    long p_blen;
    _DTA *p_dta;
    struct _basepage *p_parent;
    long p_reserved;
    char *p_env;
    char p_undef[80];
    char p_cmdlin[128];
} BASEPAGE;
extern BASEPAGE *_base;

/* Iorec(). */
typedef struct {
    void *ibuf;
    short ibufsiz;
    volatile short ibufhd;
    volatile short ibuftl;
    short ibuflow;
    short ibufhi;
} _IOREC;

/* Kbdvbase(). */
typedef struct {
    void (*midivec)(void);
    void (*vkbderr)(void);
    void (*vmiderr)(void);
    void (*statvec)(void *);
    void (*mousevec)(void *);
    void (*clockvec)(void *);
    void (*joyvec)(void *);
    void (*midisys)(void);
    void (*ikbdsys)(void);
} _KBDVECS;

'''

out = [HEAD]
for title, trap, table in (('GEMDOS', 1, GEMDOS), ('BIOS', 13, BIOS),
                           ('XBIOS', 14, XBIOS)):
    out.append('/* %s (trap #%d) */\n' % (title, trap))
    for name, num, ret, args, comment in table:
        out.append(macro(name, trap, num, ret, args, comment) + '\n')
out.append('#endif /* _OSBIND_H */\n')
path = sys.argv[1] if len(sys.argv) > 1 else 'include/osbind.h'
with open(path, 'w') as f:
    f.write('\n'.join(out))
