
#pragma once

#define COMPILER_UNKNOWN     0
#define COMPILER_GCC         1
#define COMPILER_CLANG       2
#define COMPILER_VCPP6       3
#define COMPILER_SGI_CC      4
#define COMPILER_SUN_CC      5
#define COMPILER_INTEL_CC    6
#define COMPILER_DOTNET7     7
#define COMPILER_DOTNET2003  8
#define COMPILER_DOTNET2005  9
#define COMPILER_DOTNET2008 10
#define COMPILER_DOTNET2010 11
#define COMPILER_DOTNET2012 12
#define COMPILER_DOTNET2013 13
#define COMPILER_DOTNET2015 14
#define COMPILER_DOTNET2017 15

#ifdef __GNUC__
    #define COMPILER COMPILER_GCC
#else 
    #ifdef sgi
        #define COMPILER COMPILER_SGI_CC
    #endif

    #ifdef sun
        #define COMPILER COMPILER_SUN_CC
    #endif
#endif 

#ifdef __clang__  
    #undef COMPILER
    #define COMPILER COMPILER_CLANG
#endif 

#if defined(_WIN32) && defined(_MSC_VER)
    #define COMPILER_IS_MSVC

    #ifdef _WIN64
        #define COMPILER_IS_MSVC_64
    #endif

    #if (_MSC_VER >= 1910)
        #define COMPILER COMPILER_DOTNET2017
    #elif (_MSC_VER >= 1900)
        #define COMPILER COMPILER_DOTNET2015
    #elif (_MSC_VER >= 1800)
        #define COMPILER COMPILER_DOTNET2013
    #elif (_MSC_VER >= 1700)
        #define COMPILER COMPILER_DOTNET2012
    #elif (_MSC_VER >= 1600)
        #define COMPILER COMPILER_DOTNET2010
    #elif (_MSC_VER >= 1500)
        #define COMPILER COMPILER_DOTNET2008
    #elif (_MSC_VER >= 1400)
        #define COMPILER COMPILER_DOTNET2005
    #elif (_MSC_VER >= 1310)
        #define COMPILER COMPILER_DOTNET2003
    #elif (_MSC_VER >= 1300)
        #define COMPILER COMPILER_DOTNET7
    #elif (_MSC_VER >= 1200)
        #define COMPILER COMPILER_VCPP6
    #endif
#endif // _WIN32

#if defined(__INTEL_COMPILER)
  #define COMPILER COMPILER_INTEL_CC
#endif 

#ifndef COMPILER
    #define COMPILER COMPILER_UNKNOWN
#endif