#include "render_system.h"
#include <cmath>

void RenderSystem::update(entt::registry& registry, SDL_Renderer* renderer)
{
    auto view = registry.view<Transform, Sprite>();
    for(auto entity : view)
    {
        auto& transform = view.get<Transform>(entity);
        auto& sprite = view.get<Sprite>(entity);
        SDL_FRect drawRect = transform.rect;

        if(auto* animation = registry.try_get<Animation>(entity))
        {
            drawRect.x += animation->idleOffsetX;
            drawRect.y += animation->idleOffsetY;
        }

        // Snap to pixel grid to avoid diagonal/strip artifacts on pixel art.
        drawRect.x = std::round(drawRect.x);
        drawRect.y = std::round(drawRect.y);

        if(sprite.texture)
        {
            SDL_RenderTextureRotated(renderer, sprite.texture, &sprite.srcRect, &drawRect, 0.0, nullptr, sprite.flip);
        }
    }
}
