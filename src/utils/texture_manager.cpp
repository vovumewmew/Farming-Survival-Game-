#include "texture_manager.h"
#include <iostream>

std::unordered_map<std::string, SDL_Texture*> TextureManager::textureMap;

SDL_Texture* TextureManager::LoadTexture(const char* fileName, SDL_Renderer* renderer)
{
    std::string key = fileName;

    if(textureMap.find(key) != textureMap.end())
    {
        return textureMap[key];
    }

    SDL_Texture* tex = IMG_LoadTexture(renderer, fileName);
    if(!tex)
    {
        std::cerr << "Loi load anh: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    textureMap[key] = tex;
    return tex;
}

void TextureManager::clean()
{
    for(auto const& [key, val] : textureMap)
    {
        SDL_DestroyTexture(val);
    }
    textureMap.clear();
    std::cout << "Da don dep tat ca cac Texture trong kho" << std::endl;
}
