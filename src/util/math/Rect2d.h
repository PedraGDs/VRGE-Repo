
#pragma once

#ifdef __cplusplus
#include <algorithm>
#endif

typedef struct Rect2d {
    int xPos, yPos, width, height;

#ifdef __cplusplus
    constexpr inline Rect2d() noexcept: xPos(0), yPos(0), width(0), height(0) { }
    constexpr inline Rect2d ( int x, int y, int w, int h) noexcept: xPos(x), yPos(y), width(w), height(h){}
    
    constexpr inline int getArea ( ) const noexcept {
        return this->width * this->height;
    }

    constexpr inline Rect2d getIntersection ( const Rect2d& other ) const noexcept {

        int xStart = std::max(this->xPos, other.xPos);
        int yStart = std::max(this->yPos, other.yPos);

        int xEnd = std::min(this->xPos + this->width, other.yPos + other.width);
        int yEnd = std::min(this->yPos + this->height, other.yPos + other.height);

        if ( xStart < xEnd && yStart < yEnd ) {
            return Rect2d(xStart, yStart, xEnd - xStart, yEnd - yStart);
        }

        return Rect2d();

    }

    constexpr inline int getIntersectionArea ( const Rect2d other ) const noexcept {
        return this->getIntersection(other).getArea();
    }

#endif
} Rect2d;


#ifdef __cplusplus

constexpr inline bool operator== ( Rect2d& valA, Rect2d& valB ) noexcept {
    return (valA.xPos == valB.xPos) && (valA.yPos == valB.yPos) && 
         (valA.width == valB.width) && (valA.height == valB.height);
}

constexpr inline bool operator!= ( Rect2d& valA, Rect2d& valB ) noexcept {
    return (valA.xPos != valB.xPos) || (valA.yPos != valB.yPos) && 
         (valA.width != valB.width) || (valA.height != valB.height);
}

#endif