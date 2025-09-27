
#pragma once

#include <stdint.h>

#ifndef __cplusplus
#include <stdalign.h>
#endif

typedef struct alignas(4) ColorRGB
{
    uint8_t alpha, red, green, blue;

#ifdef __cplusplus
    constexpr inline ColorRGB ( ) noexcept: red(0), green(0), blue(0), alpha(255) { }

    // Takes ARGB as input and not RGBa
    constexpr inline ColorRGB ( uint32_t argb ) noexcept: 
        alpha (argb >> 24),
        red   ((argb >> 16) & 0xFF), // Unpack then normalize
        green ((argb >> 8) & 0xFF),  
        blue  (argb & 0xFF)
    { } 
    
    constexpr inline uint32_t toRGBA ( ) const noexcept {
        return (this->red << 24) | (this->blue << 16) | (this->green << 8) | this->alpha;
    }

    constexpr inline uint32_t toARGB ( ) const noexcept {
        return (this->alpha << 24) | (this->red << 16) | (this->blue << 8) | this->green;
    }

#endif
} ColorRGB;
