
#pragma once

typedef struct Rect2d {
    int xPos, yPos, width, height;

#ifdef __cplusplus
    constexpr inline Rect2d() noexcept: xPos(0), yPos(0), width(0), height(0) { }
    constexpr inline Rect2d ( int x, int y, int w, int h) noexcept: xPos(x), yPos(y), width(w), height(h){}
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