#include "render_system.h"

void RenderSystem::update(entt::registry& registry, SDL_Renderer* renderer)
{
    auto view = registry.view<Transform, Sprite>();
    for(auto entity : view)
    {
        auto& transform = view.get<Transform>(entity);
        auto& sprite = view.get<Sprite>(entity);

        if(sprite.texture)
        {
            SDL_RenderTextureRotated(renderer, sprite.texture, &sprite.srcRect, &transform.rect, 0.0, nullptr, sprite.flip);
        }
    }
}
