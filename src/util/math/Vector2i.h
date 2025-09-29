
#pragma once

#ifdef __cplusplus
#include "util/intrinsics.h"
#include "Vector2f.h"
#include <iostream>
#include <string>
#endif

typedef struct Vector2i
{
    int X, Y;

#ifdef __cplusplus
    constexpr inline Vector2i ( ) noexcept : X(0), Y(0) { }
    constexpr inline Vector2i ( int x, int y ) noexcept : X(x), Y(y) { }

    constexpr inline int Dot (const Vector2i& other) const noexcept {
        return this->X * other.X + this->Y * other.Y;
    }
    
    constexpr inline int Cross (const Vector2i& other) const noexcept {
        return this->X * other.Y - this->Y * other.X;
    }

    constexpr inline Vector2i Lerp (const Vector2i& other, const float& delta) const noexcept {
        return Vector2i(
            this->X + (other.X - this->X) * delta,
            this->Y + (other.Y - this->Y) * delta
        );
    }

    constexpr inline bool RoughlyEquals ( const Vector2i& other, float threshold ) const noexcept {
        float dx = other.X - this->X; float dy = other.Y - this->Y;
        return ( dx * dx + dy * dy ) < threshold;
    }

    constexpr inline bool RoughlyEquals ( const Vector2i& other ) const noexcept {
        return this->RoughlyEquals(other, 0.001F);
    }

    constexpr inline int SqrMagnitude ( ) const noexcept {
        return this->X * this->X + this->Y * this->Y;
    }

    inline float Magnitude ( ) const noexcept {
        return sqrtf(this->SqrMagnitude());
    }

    inline float Distance ( const Vector2i& other ) const noexcept {
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
} Vector2i;

#ifdef __cplusplus
constexpr inline Vector2i operator- ( const Vector2i& val ) noexcept {
    return Vector2i(-val.X, -val.Y);
}

constexpr inline Vector2i operator+ ( const Vector2i& valA, const Vector2i& valB ) noexcept {
    return Vector2i(valA.X + valB.X, valA.Y + valB.Y);
}

constexpr inline Vector2i operator- ( const Vector2i& valA, const Vector2i& valB ) noexcept {
    return Vector2i(valA.X - valB.X, valA.Y - valB.Y);
}

constexpr inline Vector2i operator* ( const Vector2i& val, float scale ) noexcept {
    return Vector2i(val.X * scale, val.Y * scale);
}

constexpr inline Vector2i operator/ ( const Vector2i& val, float scale ) noexcept {
    return val * ( 1.0F / scale );
}

constexpr inline Vector2i operator+= ( Vector2i& valA, const Vector2i& ValB ) noexcept {
    valA.X += ValB.X; valA.Y += ValB.Y;
    return valA;
}

constexpr inline Vector2i operator-= ( Vector2i& valA, const Vector2i& ValB ) noexcept {
    valA.X -= ValB.X; valA.Y -= ValB.Y;
    return valA;
}

constexpr inline Vector2i operator*= ( Vector2i& val, float scale ) noexcept {
    val.X *= scale; val.Y *= scale;
    return val;
}

constexpr inline Vector2i operator/= ( Vector2i& val, float scale ) noexcept {
    return val *= (1.0F / scale);
}

constexpr inline bool operator== ( Vector2i& valA, Vector2i& valB ) noexcept {
    return valA.X == valB.X && valA.Y == valB.Y;
}

constexpr inline bool operator!= ( Vector2i& valA, Vector2i& valB ) noexcept {
    return valA.X != valB.X || valA.Y != valB.Y;
}

inline std::ostream& operator<<(std::ostream& os, const Vector2i& val) {
    return os << val.toString();
}

#endif // cplusplus