#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <SDL3/SDL.h>

struct PlayerController
{
    float speed = 180.0f;
    
    SDL_Scancode up = SDL_SCANCODE_W;
    SDL_Scancode down = SDL_SCANCODE_S;
    SDL_Scancode left = SDL_SCANCODE_A;
    SDL_Scancode right = SDL_SCANCODE_D;
    
    SDL_Scancode altUp = SDL_SCANCODE_UP;
    SDL_Scancode altDown = SDL_SCANCODE_DOWN;
    SDL_Scancode altLeft = SDL_SCANCODE_LEFT;
    SDL_Scancode altRight = SDL_SCANCODE_RIGHT;
};

#endif