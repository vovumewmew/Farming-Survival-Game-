#ifndef PLAYER_CONTROL_SYSTEM_H
#define PLAYER_CONTROL_SYSTEM_H

#include <entt/entt.hpp>

namespace PlayerControlSystem
{
    // Đọc input từ bàn phím và cập nhật Velocity cho các entity có PlayerController
    void update(entt::registry& registry);
}

#endif