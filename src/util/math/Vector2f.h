
#pragma once

#ifdef __cplusplus
#include "util/intrinsics.h"
#include <iostream>
#include <string>
#endif

typedef struct Vector2f
{
    float X, Y;

#ifdef __cplusplus
    constexpr inline Vector2f ( ) noexcept : X(0), Y(0) { }
    constexpr inline Vector2f ( float x, float y ) noexcept : X(x), Y(y) { }

    constexpr inline float Dot (const Vector2f& other) const noexcept {
        return this->X * other.X + this->Y * other.Y;
    }
    
    constexpr inline float Cross (const Vector2f& other) const noexcept {
        return this->X * other.Y - this->Y * other.X;
    }

    constexpr inline Vector2f Lerp (const Vector2f& other, const float& delta) const noexcept {
        return Vector2f(
            this->X + (other.X - this->X) * delta,
            this->Y + (other.Y - this->Y) * delta
        );
    }

    inline Vector2f Floor ( ) const noexcept {
        return Vector2f( floorf(this->X), floorf(this->Y) );
    }

    inline Vector2f Ceil ( ) const noexcept {
        return Vector2f( ceilf(this->X), ceilf(this->Y) );
    }

    inline Vector2f Round ( ) const noexcept {
        return Vector2f( roundf(this->X), roundf(this->Y) );
    }

    constexpr inline bool RoughlyEquals ( const Vector2f& other, float threshold ) const noexcept {
        float dx = other.X - this->X; float dy = other.Y - this->Y;
        return ( dx * dx + dy * dy ) < threshold;
    }

    constexpr inline bool RoughlyEquals ( const Vector2f& other ) const noexcept {
        return this->RoughlyEquals(other, 0.001F);
    }

    constexpr inline float SqrMagnitude ( ) const noexcept {
        return this->X * this->X + this->Y * this->Y;
    }

    inline float Magnitude ( ) const noexcept {
        return sqrtf(this->SqrMagnitude());
    }

    inline float Distance ( const Vector2f& other ) const noexcept {
        float dx = this->X - other.X; float dy = this->Y - other.Y;
        return sqrtf( dx * dx + dy * dy );
    }

    inline Vector2f Normalized ( ) const noexcept {
        float invMag = 1 / this->Magnitude();
        return Vector2f( this->X * invMag, this->Y * invMag );
    }

    inline Vector2f Unit ( ) const noexcept {
        return this->Normalized();
    }

    inline std::string toString ( ) const noexcept {
        return "( " + 
            std::to_string(this->X) + ", " + 
            std::to_string(this->Y) + " )";
    }

#endif // cplusplus
} Vector2f;

#ifdef __cplusplus
constexpr inline Vector2f operator- ( const Vector2f& val ) noexcept {
    return Vector2f(-val.X, -val.Y);
}

constexpr inline Vector2f operator+ ( const Vector2f& valA, const Vector2f& valB ) noexcept {
    return Vector2f(valA.X + valB.X, valA.Y + valB.Y);
}

constexpr inline Vector2f operator- ( const Vector2f& valA, const Vector2f& valB ) noexcept {
    return Vector2f(valA.X - valB.X, valA.Y - valB.Y);
}

constexpr inline Vector2f operator* ( const Vector2f& val, float scale ) noexcept {
    return Vector2f(val.X * scale, val.Y * scale);
}

constexpr inline Vector2f operator/ ( const Vector2f& val, float scale ) noexcept {
    return val * ( 1.0F / scale );
}

constexpr inline Vector2f operator+= ( Vector2f& valA, const Vector2f& ValB ) noexcept {
    valA.X += ValB.X; valA.Y += ValB.Y;
    return valA;
}

constexpr inline Vector2f operator-= ( Vector2f& valA, const Vector2f& ValB ) noexcept {
    valA.X -= ValB.X; valA.Y -= ValB.Y;
    return valA;
}

constexpr inline Vector2f operator*= ( Vector2f& val, float scale ) noexcept {
    val.X *= scale; val.Y *= scale;
    return val;
}

constexpr inline Vector2f operator/= ( Vector2f& val, float scale ) noexcept {
    return val *= (1.0F / scale);
}

constexpr inline bool operator== ( Vector2f& valA, Vector2f& valB ) noexcept {
    return valA.X == valB.X && valA.Y == valB.Y;
}

constexpr inline bool operator!= ( Vector2f& valA, Vector2f& valB ) noexcept {
    return valA.X != valB.X || valA.Y != valB.Y;
}

std::ostream& operator<<(std::ostream& os, const Vector2f& val) {
    return os << val.toString();
}

#endif // cplusplus