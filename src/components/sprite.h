#ifndef SPRITE_H
#define SPRITE_H

#include <SDL3/SDL.h>

struct Sprite
{
    SDL_Texture* texture;
    SDL_Texture* idleTexture;
    SDL_Texture* walkTexture;
    SDL_FRect srcRect;
    SDL_FlipMode flip;
};

#endif
