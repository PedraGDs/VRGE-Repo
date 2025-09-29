
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__GNUC__) && !defined(__clang__)
    #include <math.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
    inline float floorf ( float x ) { return __builtin_floorf(x); }
    inline float ceilf ( float x )  { return __builtin_ceilf(x);  }
    inline float roundf ( float x ) { return __builtin_roundf(x); }
    inline float sqrtf ( float x )  { return __builtin_sqrtf(x);  }
#else
    inline float floorf ( float x ) { return floor(x); }
    inline float ceilf ( float x )  { return ceil(x);  }
    inline float roundf ( float x ) { return round(x); }
    inline float sqrtf ( float x )  { return sqrt(x);  }
#endif

#ifdef __cplusplus
}
#endif
