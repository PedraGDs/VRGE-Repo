
#ifndef UTIL_INTRINSICS_H
#define UTIL_INTRINSICS_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__GNUC__) && !defined(__clang__)
    #include <math.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
    static inline float floorf ( float x ) { return __builtin_floorf(x); }
    static inline float ceilf ( float x )  { return __builtin_ceilf(x);  }
    static inline float roundf ( float x ) { return __builtin_roundf(x); }
    static inline float sqrtf ( float x )  { return __builtin_sqrtf(x);  }
#else
    static inline float floorf ( float x ) { return floor(x); }
    static inline float ceilf ( float x )  { return ceil(x);  }
    static inline float roundf ( float x ) { return round(x); }
    static inline float sqrtf ( float x )  { return sqrt(x);  }
#endif

#ifdef __cplusplus
}
#endif

#endif // UTIL_INTRINSICS_H