
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
#endif

#ifdef sgi
    #define OPERATING_SYSTEM OS_IRIX
#endif 

#ifdef __CYGWIN__
    #define OPERATING_SYSTEM OS_CYGWIN
#endif 

#ifdef __APPLE__
    #define OPERATING_SYSTEM OS_DARWIN
#endif

#ifdef __QNX__
    #define OPERATING_SYSTEM OS_QNX
#endif 

#ifdef _WIN32 
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0500
    #endif
    
    #define OPERATING_SYSTEM OS_WINDOWS
#endif

#ifdef __INTEL_COMPILER
    #define OPERATING_SYSTEM OS_WINDOWS
#endif

#ifndef OPERATING_SYSTEM
    #define OPERATING_SYSTEM OS_UNKNOWN
#endif