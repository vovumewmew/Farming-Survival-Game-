#ifndef ANIMATION_H
#define ANIMATION_H

enum class FacingDirection
{
    DOWN = 0,
    LEFT = 1,
    RIGHT = 2,   
    UP = 3
};

struct Animation
{
    int frameIndex = 0;
    float timer = 0.0f;
    FacingDirection facing = FacingDirection::DOWN;
    bool isMoving = false;
};

#endif
