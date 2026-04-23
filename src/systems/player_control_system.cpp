#include "player_control_system.h"

#include "../components/player_controller.h"
#include "../components/velocity.h"
#include <SDL3/SDL.h>

void PlayerControlSystem::update(entt::registry& registry)
{
    const bool* keys = SDL_GetKeyboardState(nullptr);
    
    auto view = registry.view<PlayerController, Velocity>();
    for(auto entity : view)
    {
        auto& controller = view.get<PlayerController>(entity);
        auto& vel = view.get<Velocity>(entity);

        float dx = 0.0f;
        float dy = 0.0f;

        if(keys[controller.up] || keys[controller.altUp])       dy -= 1.0f;
        if(keys[controller.down] || keys[controller.altDown])   dy += 1.0f;
        if(keys[controller.left] || keys[controller.altLeft])   dx -= 1.0f;
        if(keys[controller.right] || keys[controller.altRight])  dx += 1.0f;

        if(dx != 0.0f && dy != 0.0f) {
            const float invSqrt2 = 0.70710678f;
            dx *= invSqrt2;  dy *= invSqrt2;
        }

        vel.dx = dx * controller.speed;
        vel.dy = dy * controller.speed;
    }
}