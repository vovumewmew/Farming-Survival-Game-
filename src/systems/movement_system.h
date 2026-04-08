#ifndef MOVEMENT_SYSTEM_H
#define MOVEMENT_SYSTEM_H

#include <entt/entt.hpp>
#include "../components/transform.h"
#include "../components/velocity.h"

namespace MovementSystem
{
    void update(entt::registry& registry, float delta_time);
}

#endif
