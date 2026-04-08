#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <entt/entt.hpp>
#include <iostream>

class Game
{
    public:
        Game();
        ~Game();

        bool init(const char* tittle, int width, int height);
        void handleEvents();
        void update();
        void render();
        void clean();

        bool running() const {return isRunning;}
        
        entt::registry& getRegistry() { return registry; }
        SDL_Renderer* getRenderer() { return renderer; }
    private:
        bool isRunning;
        SDL_Window* window;
        SDL_Renderer* renderer;
        Uint64 last_time = 0;

        entt::registry registry;
};

#endif
