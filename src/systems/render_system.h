#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <entt/entt.hpp>
#include <SDL3/SDL.h>
#include "../components/transform.h"
#include "../components/sprite.h"
#include "../components/animation.h"

namespace RenderSystem
{
    void update(entt::registry& registry, SDL_Renderer* renderer);
}

#endif
