
#pragma once

#ifdef __cplusplus
#include "util/intrinsics.h"
#include <iostream>
#include <string>
#endif

typedef struct Vector3f
{
    float X, Y, Z;

#ifdef __cplusplus
    constexpr inline Vector3f ( ) noexcept : X(0), Y(0), Z(0) { }
    constexpr inline Vector3f ( float x, float y, float z ) noexcept : X(x), Y(y), Z(z) { }

    constexpr inline float Dot (const Vector3f& other) const noexcept {
        return this->X * other.X + this->Y * other.Y + this->Z * other.Z;
    }
    
    constexpr inline Vector3f Cross (const Vector3f& other) const noexcept {
        return Vector3f(
            this->Y * other.Z - this->Z * other.Y,
            this->Z * other.X - this->X * other.Z,
            this->X * other.Y - this->Y * other.X
        );
    }

    constexpr inline Vector3f Lerp (const Vector3f& other, const float& delta) const noexcept {
        return Vector3f(
            this->X + (other.X - this->X) * delta,
            this->Y + (other.Y - this->Y) * delta,
            this->Z + (other.Z - this->Z) * delta
        );
    }

    inline Vector3f Floor ( ) const noexcept {
        return Vector3f( floorf(this->X), floorf(this->Y), floorf(this->Z) );
    }

    inline Vector3f Ceil ( ) const noexcept {
        return Vector3f( ceilf(this->X), ceilf(this->Y), ceilf(this->Z) );
    }

    inline Vector3f Round ( ) const noexcept {
        return Vector3f( roundf(this->X), roundf(this->Y), roundf(this->Z) );
    }

    constexpr inline bool RoughlyEquals ( const Vector3f& other, float threshold ) const noexcept {
        float dx = other.X - this->X;
        float dy = other.Y - this->Y;
        float dz = other.Z - this->Z;

        return (dx * dx + dy * dy + dz * dz) < threshold;
    }

    constexpr inline bool RoughlyEquals ( const Vector3f& other ) const noexcept {
        return this->RoughlyEquals(other, 0.001F);
    }

    constexpr inline float SqrMagnitude ( ) const noexcept {
        return this->X * this->X + this->Y * this->Y + this->Z * this->Z;
    }

    inline float Magnitude ( ) const noexcept {
        return sqrtf(this->SqrMagnitude());
    }

    inline float Distance ( const Vector3f& other ) const noexcept {
        float dx = this->X - other.X;
        float dy = this->Y - other.Y;
        float dz = this->Z - other.Z;
    
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }

    inline Vector3f Normalized ( ) const noexcept {
        float invMag = 1 / this->Magnitude();
        return Vector3f( this->X * invMag, this->Y * invMag, this->Z * invMag );
    }

    inline Vector3f Unit ( ) const noexcept {
        return this->Normalized();
    }

    inline std::string toString ( ) const noexcept {
        return "( " + 
            std::to_string(this->X) + ", " + 
            std::to_string(this->Y) + ", " + 
            std::to_string(this->Z) + " )";
    }

#endif // cplusplus
} Vector3f;

#ifdef __cplusplus
constexpr inline Vector3f operator- ( const Vector3f& val ) noexcept {
    return Vector3f(-val.X, -val.Y, -val.Z);
}

constexpr inline Vector3f operator+ ( const Vector3f& valA, const Vector3f& valB ) noexcept {
    return Vector3f(valA.X + valB.X, valA.Y + valB.Y, valA.Z + valB.Z);
}

constexpr inline Vector3f operator- ( const Vector3f& valA, const Vector3f& valB ) noexcept {
    return Vector3f(valA.X - valB.X, valA.Y - valB.Y, valA.Z - valB.Z);
}

constexpr inline Vector3f operator* ( const Vector3f& val, float scale ) noexcept {
    return Vector3f(val.X * scale, val.Y * scale, val.Z * scale);
}

constexpr inline Vector3f operator/ ( const Vector3f& val, float scale ) noexcept {
    return val * ( 1.0F / scale );
}

constexpr inline Vector3f operator+= ( Vector3f& valA, const Vector3f& ValB ) noexcept {
    valA.X += ValB.X;
    valA.Y += ValB.Y;
    valA.Z += ValB.Z;

    return valA;
}

constexpr inline Vector3f operator-= ( Vector3f& valA, const Vector3f& ValB ) noexcept {
    valA.X -= ValB.X;
    valA.Y -= ValB.Y;
    valA.Z -= ValB.Z;

    return valA;
}

constexpr inline Vector3f operator*= ( Vector3f& val, float scale ) noexcept {
    val.X *= scale;
    val.Y *= scale;
    val.Z *= scale;

    return val;
}

constexpr inline Vector3f operator/= ( Vector3f& val, float scale ) noexcept {
    return val *= (1.0F / scale);
}

constexpr inline bool operator== ( Vector3f& valA, Vector3f& valB ) noexcept {
    return valA.X == valB.X && valA.Y == valB.Y && valA.Z == valB.Z;
}

constexpr inline bool operator!= ( Vector3f& valA, Vector3f& valB ) noexcept {
    return valA.X != valB.X || valA.Y != valB.Y || valA.Z != valB.Z;
}

inline std::ostream& operator<<(std::ostream& os, const Vector3f& val) {
    return os << val.toString();
}

#endif // cplusplus