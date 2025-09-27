
#pragma once

#include "Vector3f.h"
#include "util/Constants.h"

typedef struct Triangle3d
{
    Vector3f VecA, VecB, VecC, Normal;

#ifdef __cplusplus

    constexpr inline Triangle3d ( const Vector3f& vecA, const Vector3f& vecB, const Vector3f& vecC, const Vector3f& normal) noexcept {
        this->VecA = vecA;
        this->VecB = vecB;
        this->VecC = vecC;
        this->Normal = normal;
    }

    constexpr inline Vector3f GetCenter ( ) const noexcept {
        return (VecA + VecB + VecC) * ONE_THIRD;
    }

    inline float GetArea ( ) const noexcept {
        return (this->VecB - this->VecA).Cross(this->VecC - this->VecA).Magnitude() * ONE_HALF;
    }

#endif
} Triangle3d;
