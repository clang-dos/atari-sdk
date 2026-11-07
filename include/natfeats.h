/*
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (c) 2026, clang-dos
 *
 * natfeats.h: Native Features, the interface emulators (Hatari, ARAnyM)
 * give programs to reach the host: print to the host's terminal, stop the
 * emulator, and so on. A program can't tell otherwise whether an emulator
 * is there, so nf_init() checks first: on a real machine, or with the
 * feature off, it returns 0 and the other calls do nothing.
 *
 *     if (nf_init())
 *         nf_print("Hello, host!\n");
 *
 * nf_init() and nf_print() can be called in user mode. In Hatari, turn
 * them on with --natfeats yes.
 */
#ifndef _NATFEATS_H
#define _NATFEATS_H

#include <osbind.h>

/* The two NatFeats instructions are illegal opcodes, 0x7300 (look up a
 * feature by name) and 0x7301 (call it), which take their arguments on
 * the stack like a C function. Without an emulator they raise an illegal
 * instruction exception; __nf_illegal makes them return 0 then. */
__asm__(".pushsection .text.__nf_illegal,\"axG\",@progbits,__nf_illegal,comdat\n"
        ".weak __nf_illegal\n"
        "__nf_illegal:\n"
        "\tadd.l #2,(2,%sp)\n" /* skip the 2-byte opcode */
        "\tmoveq #0,%d0\n"
        "\trte\n"
        ".popsection");
extern char __nf_illegal[];

static long __nf_id_value;
static const char *__nf_arg;
static long __nf_call_id;

static __inline__ long __nf_id(const char *name)
{
    register long __r __asm__("d0");
    __asm__ __volatile__("move.l %1,-(%%sp)\n\t"
                         "lea (-4,%%sp),%%sp\n\t"
                         ".short 0x7300\n\t"
                         "lea (8,%%sp),%%sp"
                         : "=r"(__r)
                         : "r"(name)
                         : "d1", "a0", "a1", "cc", "memory");
    return __r;
}

static __inline__ long __nf_call1(long id, long arg)
{
    register long __r __asm__("d0");
    __asm__ __volatile__("move.l %2,-(%%sp)\n\t"
                         "move.l %1,-(%%sp)\n\t"
                         "lea (-4,%%sp),%%sp\n\t"
                         ".short 0x7301\n\t"
                         "lea (12,%%sp),%%sp"
                         : "=r"(__r)
                         : "r"(id), "r"(arg)
                         : "d1", "a0", "a1", "cc", "memory");
    return __r;
}

/* In supervisor mode: look up __nf_arg with the illegal instruction
 * vector pointing at __nf_illegal. */
static long __nf_lookup(void)
{
    void **vec = (void **)0x10;
    void *old = *vec;
    *vec = __nf_illegal;
    long id = __nf_id("NF_NAME") ? __nf_id(__nf_arg) : 0;
    *vec = old;
    return id;
}

static long __nf_call_super(void)
{
    return __nf_call1(__nf_call_id, (long)__nf_arg);
}

/* The id of a feature, or 0 when there is none (or no emulator). */
static __inline__ long nf_find(const char *name)
{
    __nf_arg = name;
    return Supexec(__nf_lookup);
}

/* Is there an emulator with NatFeats? */
static __inline__ int nf_init(void)
{
    if (!__nf_id_value)
        __nf_id_value = nf_find("NF_STDERR");
    return __nf_id_value != 0;
}

/* Call a feature with one argument, in supervisor mode. */
static __inline__ long nf_call(long id, long arg)
{
    __nf_call_id = id;
    __nf_arg = (const char *)arg;
    return id ? Supexec(__nf_call_super) : 0;
}

/* Print to the host's standard error. */
static __inline__ void nf_print(const char *s)
{
    if (nf_init())
        nf_call(__nf_id_value, (long)s);
}

/* Stop the emulator (Hatari: with the given exit status). */
static __inline__ void nf_exit(int status)
{
    long id = nf_find("NF_EXIT");
    if (id)
        nf_call(id, status);
}

#endif /* _NATFEATS_H */
