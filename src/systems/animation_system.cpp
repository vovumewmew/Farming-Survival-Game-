#include "animation_system.h"

#include "../components/animation.h"
#include "../components/player.h"
#include "../components/sprite.h"
#include "../components/velocity.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kPlayerSpeed = 180.0f;
    constexpr float kCellSize = 48.0f;
    constexpr float kFrameInsetX = 16.0f;
    constexpr float kFrameInsetY = 16.0f;
    constexpr float kFrameWidth = 16.0f;
    constexpr float kFrameHeight = 16.0f;
    constexpr int kFrameCount = 4;
    constexpr float kWalkFrameDurationMin = 0.05f;
    constexpr float kWalkFrameDurationMax = 0.2f;
    constexpr float kIdleFrameDuration = 0.22f;
    constexpr int kIdleFrameSequence[] = {0, 0, 1, 0};
    constexpr int kIdleFrameSequenceCount = static_cast<int>(sizeof(kIdleFrameSequence) / sizeof(kIdleFrameSequence[0]));
    constexpr float kIdleCycleSpeed = 1.38f;
    constexpr float kIdleBounceDown = 0.45f;
    constexpr float kIdleBounceDownSecondary = 0.11f;
    constexpr float kIdleSwayX = 0.18f;
    constexpr float kIdleSwayXSecondary = 0.07f;
    constexpr float kMoveSwayX = 0.8f;
    constexpr float kMoveBobY = 0.5f;
    constexpr float kOffsetResponse = 12.5f;
    constexpr float kTwoPi = 6.2831853f;

    int rowFromFacing(FacingDirection facing)
    {
        switch(facing)
        {
            case FacingDirection::UP:
                return 1;
            case FacingDirection::LEFT:
                return 2;
            case FacingDirection::RIGHT:
                return 3;
            case FacingDirection::DOWN:
            default:
                return 0;
        }
    }

    float smoothToward(float current, float target, float response, float deltaTime)
    {
        const float blend = 1.0f - std::exp(-response * deltaTime);
        return current + (target - current) * blend;
    }
}

void AnimationSystem::update(entt::registry& registry, float delta_time)
{
    auto view = registry.view<Velocity, Sprite, Animation>();
    for(auto entity : view)
    {
        auto& vel = view.get<Velocity>(entity);
        auto& sprite = view.get<Sprite>(entity);
        auto& animation = view.get<Animation>(entity);

        const bool moving = (std::fabs(vel.dx) > 0.001f) || (std::fabs(vel.dy) > 0.001f);
        animation.isMoving = moving;
        const float speed = std::sqrt((vel.dx * vel.dx) + (vel.dy * vel.dy));
        const float speedNorm = std::clamp(speed / kPlayerSpeed, 0.0f, 1.0f);

        if(moving)
        {
            if(std::fabs(vel.dx) > std::fabs(vel.dy))
                animation.facing = (vel.dx < 0.0f) ? FacingDirection::LEFT : FacingDirection::RIGHT;
            else
                animation.facing = (vel.dy < 0.0f) ? FacingDirection::UP : FacingDirection::DOWN;
        }

        float targetOffsetX = 0.0f;
        float targetOffsetY = 0.0f;

        if(animation.isMoving)
        {
            if(!animation.wasMoving) {
                animation.frameIndex = 0; animation.timer = 0.0f; animation.idleFrameCursor = 0;
            }

            const float walkFrameDuration = kWalkFrameDurationMax + (kWalkFrameDurationMin - kWalkFrameDurationMax) * speedNorm;
            animation.timer += delta_time;
            while(animation.timer >= walkFrameDuration) {
                animation.frameIndex = (animation.frameIndex + 1) % kFrameCount;
                animation.timer -= walkFrameDuration;
            }
            const float frameProgress = animation.timer / walkFrameDuration;
            const float stepAngle = ((static_cast<float>(animation.frameIndex) + frameProgress) / kFrameCount) * kTwoPi;
            targetOffsetX = std::sin(stepAngle) * kMoveSwayX;
            targetOffsetY = std::fabs(std::sin(stepAngle)) * kMoveBobY;
        }
        else
        {
            if(animation.wasMoving) { animation.timer = 0.0f; animation.idleFrameCursor = 0; }
            animation.timer += delta_time;
            while(animation.timer >= kIdleFrameDuration) {
                animation.idleFrameCursor = (animation.idleFrameCursor + 1) % kIdleFrameSequenceCount;
                animation.timer -= kIdleFrameDuration;
            }
            animation.frameIndex = kIdleFrameSequence[animation.idleFrameCursor];
            animation.idleBobTime += delta_time;
            const float phase = animation.idleBobTime * kIdleCycleSpeed * kTwoPi;
            targetOffsetY = (std::pow(0.5f - 0.5f * std::cos(phase), 1.45f) * kIdleBounceDown) + ((0.5f - 0.5f * std::cos((phase * 2.0f) + 0.65f)) * kIdleBounceDownSecondary);
            targetOffsetX = (std::sin(phase + 0.9f) * kIdleSwayX) + (std::sin((phase * 2.0f) + 1.8f) * kIdleSwayXSecondary);
        }

        animation.idleOffsetX = smoothToward(animation.idleOffsetX, targetOffsetX, kOffsetResponse, delta_time);
        animation.idleOffsetY = smoothToward(animation.idleOffsetY, targetOffsetY, kOffsetResponse, delta_time);
        animation.wasMoving = animation.isMoving;

        sprite.texture = animation.isMoving ? sprite.walkTexture : sprite.idleTexture;
        sprite.srcRect = { static_cast<float>(animation.frameIndex) * kCellSize + kFrameInsetX, static_cast<float>(rowFromFacing(animation.facing)) * kCellSize + kFrameInsetY, kFrameWidth, kFrameHeight };
    }
}