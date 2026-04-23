#ifndef ANIMATION_SYSTEM_H
#define ANIMATION_SYSTEM_H

#include <entt/entt.hpp>

namespace AnimationSystem
{
    // Hàm cập nhật trạng thái Animation cho tất cả các entity có Component liên quan
    void update(entt::registry& registry, float delta_time);
}

#endif