
#pragma once

typedef struct Rect2d {
    int xPos, yPos, width, height;

#ifdef __cplusplus
    constexpr inline Rect2d() noexcept: xPos(0), yPos(0), width(0), height(0) { }
    constexpr inline Rect2d ( int x, int y, int w, int h) noexcept: xPos(x), yPos(y), width(w), height(h){}
    
    constexpr inline int getIntersectionArea ( const Rect2d other ) {
        int x = (this->xPos + this->width < other.xPos + other.width 
            ? this->xPos + this->width : other.xPos + other.width) -
            (this->xPos > other.xPos ? this->xPos : other.xPos);

        int y = (this->yPos + this->height < other.yPos + other.height 
            ? this->yPos + this->height : other.yPos + other.height) -
            (this->yPos > other.yPos ? this->yPos : other.yPos);

        if ( x < 0 ) { return 0; }
        if ( y < 0 ) { return 0; }

        return x * y;

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