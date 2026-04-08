#include "movement_system.h"

void MovementSystem::update(entt::registry& registry, float delta_time)
{
    auto view = registry.view<Transform, Velocity>();

    for(auto entity : view)
    {
        auto& transform = view.get<Transform>(entity);
        auto& vel = view.get<Velocity>(entity);

        transform.rect.x += vel.dx * delta_time;
        transform.rect.y += vel.dy * delta_time;

        
    }
}