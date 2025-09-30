
#pragma once

#define OS_UNKNOWN 0
#define OS_WINDOWS 1
#define OS_LINUX   2
#define OS_IRIX    3
#define OS_SOLARIS 4
#define OS_CYGWIN  5
#define OS_DARWIN  6
#define OS_QNX     7

#if (defined(linux) || defined(__linux__))
    #define OPERATING_SYSTEM OS_LINUX
#endif

#ifdef sun
    #define OPERATING_SYSTEM OS_SOLARIS
#endif // sun

#ifdef sgi
    #define OPERATING_SYSTEM OS_IRIX
#endif // sgi

#ifdef __CYGWIN__
    #define OPERATING_SYSTEM OS_CYGWIN
#endif // __CYGWIN__

#ifdef __APPLE__
    #define OPERATING_SYSTEM OS_DARWIN
#endif // __APPLE__

#ifdef __QNX__
    #define OPERATING_SYSTEM OS_QNX
#endif // __QNX__

#ifdef _WIN32 
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0500
    #endif
    
    #define OPERATING_SYSTEM OS_WINDOWS
#endif // _WIN32

#ifdef __INTEL_COMPILER
    #define OPERATING_SYSTEM OS_WINDOWS
#endif

#ifndef OPERATING_SYSTEM
    #define OPERATING_SYSTEM OS_UNKNOWN
#endif