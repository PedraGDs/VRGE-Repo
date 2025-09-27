
#pragma once

#ifdef __cplusplus
#include "stdint.h"
#include <algorithm>
#include "util/Constants.h"
#endif

struct Color4f
{
    float red, green, blue, alpha;

#ifdef __cplusplus

    constexpr inline Color4f ( ) noexcept: red(0), green(0), blue(0), alpha(1) { }
    
    constexpr inline Color4f ( float r, float g, float b, float a ) noexcept: 
        red(r), green(g), blue(b), alpha(a) { }

    // Takes ARGB as input and not RGBa
    constexpr inline Color4f ( uint32_t argb ) noexcept: 
        red   (((argb >> 16) & 0xFF) * ONE_255), // Unpack then normalize
        green (((argb >> 8) & 0xFF) * ONE_255),  
        blue  ((argb & 0xFF) * ONE_255),
        alpha ((argb >> 24) * ONE_255)
    { } 

    constexpr inline uint8_t getURed ( ) const noexcept {
        return static_cast<uint8_t>(std::clamp( this->red * 255.0f, 0.0f, 255.0f));
    }

    constexpr inline uint8_t getUGreen ( ) const noexcept {
        return static_cast<uint8_t>(std::clamp( this->green * 255.0f, 0.0f, 255.0f));
    }

    constexpr inline uint8_t getUBlue ( ) const noexcept {
        return static_cast<uint8_t>(std::clamp( this->blue * 255.0f, 0.0f, 255.0f));
    }

    constexpr inline uint8_t getUAlpha ( ) const noexcept {
        return static_cast<uint8_t>(std::clamp( this->alpha * 255.0f, 0.0f, 255.0f));
    }

    constexpr inline uint32_t toRGBA ( ) const noexcept {
        return (getURed() << 24) | (getUGreen() << 16) | (getUBlue() << 8) | getUAlpha();
    }

    constexpr inline uint32_t toARGB ( ) const noexcept {
        return (getUAlpha() << 24) | (getURed() << 16) | (getUGreen() << 8) | getUBlue();
    }

#endif // cplusplus
};
