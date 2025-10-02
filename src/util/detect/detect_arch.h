#pragma once

// This header file is from MESA, made by José Fonseca.

#if defined(__i386__) /* gcc */ || defined(_M_IX86) /* msvc */ || defined(_X86_) || defined(__386__) || defined(i386) || defined(__i386) /* Sun cc */
    #define DETECT_ARCH_X86 1
#endif

#if defined(__x86_64__) /* gcc */ || \
    ((defined(_M_X64) || defined(_M_AMD64)) && !defined(_M_ARM64EC)) /* msvc */ || \
    defined(__x86_64) /* Sun cc */
#define DETECT_ARCH_X86_64 1
#endif

#if DETECT_ARCH_X86 || DETECT_ARCH_X86_64
    #if DETECT_CC_GCC && !defined(__SSE2__)
        /* #warning SSE2 support requires -msse -msse2 compiler options */
    #else
        #define DETECT_ARCH_SSE 1
    #endif
#endif

#if defined(__ppc__) || defined(__ppc64__) || defined(__PPC__) || defined(__PPC64__)
    #define DETECT_ARCH_PPC 1
    #if defined(__ppc64__) || defined(__PPC64__)
        #define DETECT_ARCH_PPC_64 1
    #endif
#endif

#if defined(__s390x__)
    #define DETECT_ARCH_S390 1
#endif

#if defined(__arm__)
    #define DETECT_ARCH_ARM 1
#endif

#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    #define DETECT_ARCH_AARCH64 1
#endif

#if defined(_M_ARM64EC)
    #define DETECT_ARCH_ARM64EC 1
#endif

#if defined(__mips64) && defined(__LP64__)
    #define DETECT_ARCH_MIPS64 1
#endif

#if defined(__mips__)
    #define DETECT_ARCH_MIPS 1
#endif

#if defined(__hppa__)
    #define DETECT_ARCH_HPPA 1
#endif

#if defined(__riscv)
    #define DETECT_ARCH_RISCV 1
    #if __riscv_xlen == 64
        #define DETECT_ARCH_RISCV64 1
    #elif __riscv_xlen == 32
        #define DETECT_ARCH_RISCV32 1
    #else
        #error "detect_arch: unknown target riscv xlen"
    #endif
#endif

#if defined(__loongarch__)
    #ifdef __loongarch_lp64
        #define DETECT_ARCH_LOONGARCH64 1
    #else
        #error "detect_arch: unknown target loongarch base ABI type"
    #endif
#endif

#if defined(__sparc__) || defined(__sparc)
    #define DETECT_ARCH_SPARC64 1
#endif

#ifndef DETECT_ARCH_X86
    #define DETECT_ARCH_X86 0
#endif

#ifndef DETECT_ARCH_X86_64
    #define DETECT_ARCH_X86_64 0
#endif

#ifndef DETECT_ARCH_SSE
    #define DETECT_ARCH_SSE 0
#endif

#ifndef DETECT_ARCH_PPC
    #define DETECT_ARCH_PPC 0
#endif

#ifndef DETECT_ARCH_PPC_64
    #define DETECT_ARCH_PPC_64 0
#endif

#ifndef DETECT_ARCH_S390
    #define DETECT_ARCH_S390 0
#endif

#ifndef DETECT_ARCH_ARM
    #define DETECT_ARCH_ARM 0
#endif

#ifndef DETECT_ARCH_AARCH64
    #define DETECT_ARCH_AARCH64 0
#endif

#ifndef DETECT_ARCH_ARM64EC
    #define DETECT_ARCH_ARM64EC 0
#endif

#ifndef DETECT_ARCH_MIPS64
    #define DETECT_ARCH_MIPS64 0
#endif

#ifndef DETECT_ARCH_MIPS
    #define DETECT_ARCH_MIPS 0
#endif

#ifndef DETECT_ARCH_HPPA
    #define DETECT_ARCH_HPPA 0
#endif

#ifndef DETECT_ARCH_RISCV
    #define DETECT_ARCH_RISCV 0
#endif

#ifndef DETECT_ARCH_RISCV32
    #define DETECT_ARCH_RISCV32 0
#endif

#ifndef DETECT_ARCH_RISCV64
    #define DETECT_ARCH_RISCV64 0
#endif

#ifndef DETECT_ARCH_LOONGARCH64
    #define DETECT_ARCH_LOONGARCH64 0
#endif

#ifndef DETECT_ARCH_SPARC64
    #define DETECT_ARCH_SPARC64 0
#endif

#if DETECT_ARCH_X86_64 || DETECT_ARCH_PPC_64 || DETECT_ARCH_AARCH64 || DETECT_ARCH_RISCV64 || DETECT_ARCH_S390 || DETECT_ARCH_LOONGARCH64
    #define DETECT_ARCH_64_BIT 1
    #define DETECT_ARCH_32_BIT 0
#else
    #define DETECT_ARCH_64_BIT 0
    #define DETECT_ARCH_32_BIT 1
#endif