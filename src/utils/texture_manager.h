#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>

#include <string>
#include <unordered_map>

class TextureManager
{
public:
    static SDL_Texture* LoadTexture(const char* fileName, SDL_Renderer* renderer);
    static void clean();

private:
    static std::unordered_map<std::string, SDL_Texture*> textureMap;
};

#endif
