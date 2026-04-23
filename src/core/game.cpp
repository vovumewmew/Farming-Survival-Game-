#include "game.h"

#include "../components/animation.h"
#include "../components/player.h"
#include "../components/sprite.h"
#include "../components/transform.h"
#include "../components/velocity.h"
#include "../components/tilled.h"
#include "../systems/movement_system.h"
#include "../systems/render_system.h"
#include "../utils/texture_manager.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kPlayerSpeed = 180.0f;
    // Basic Charakter Spritesheet: 192x192, grid 4x4 => each cell 48x48.
    // Visible cat sprite is 16x16 inside each cell, anchored at (16,16).
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
    constexpr float kPlayerScale = 2.0f;
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

using namespace std;

Game::Game()
    : isRunning(false), window(nullptr), renderer(nullptr)
{
}

Game::~Game()
{
}

bool Game::init(const char* title, int width, int height)
{
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        cerr << "Loi khi tao SDL3: " << SDL_GetError() << endl;
        return false;
    }

    window = SDL_CreateWindow(title, width, height, 0);
    if(!window)
    {
        cerr << "Loi khi tao cua so: " << SDL_GetError() << endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if(!renderer)
    {
        cerr << "Loi khi tao Renderer: " << SDL_GetError() << endl;
        return false;
    }

    SDL_Texture* idleTex = TextureManager::LoadTexture("assets/Sprout Lands - Sprites - Basic pack/Characters/Basic Charakter Spritesheet.png", renderer);
    SDL_Texture* walkTex = TextureManager::LoadTexture("assets/Sprout Lands - Sprites - Basic pack/Characters/Basic Charakter Spritesheet.png", renderer);
    if(!idleTex || !walkTex)
    {
        return false;
    }

    SDL_SetTextureScaleMode(idleTex, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(walkTex, SDL_SCALEMODE_NEAREST);

    auto playerEntity = registry.create();
    const float drawW = kFrameWidth * kPlayerScale;
    const float drawH = kFrameHeight * kPlayerScale;

    registry.emplace<Player>(playerEntity);
    registry.emplace<Transform>(
        playerEntity,
        SDL_FRect{
            (static_cast<float>(width) - drawW) / 2.0f,
            (static_cast<float>(height) - drawH) / 2.0f,
            drawW,
            drawH
        }
    );
    registry.emplace<Velocity>(playerEntity, 0.0f, 0.0f);
    registry.emplace<Sprite>(
        playerEntity,
        idleTex,
        idleTex,
        walkTex,
        SDL_FRect{0.0f, 0.0f, kFrameWidth, kFrameHeight},
        SDL_FLIP_NONE
    );
    registry.emplace<Animation>(playerEntity);

    SDL_Texture* dirtTex = TextureManager::LoadTexture("assets/Sprout Lands - Sprites - Basic pack/Tilesets/Tilled_Dirt.png", renderer);
    if(!dirtTex)
    {
        return false;
    }
    SDL_SetTextureScaleMode(dirtTex, SDL_SCALEMODE_NEAREST);

    constexpr float tileSize = 32.0f;
    const float startX = (width - tileSize * 3.0f) / 2.0f;
    const float startY = (height - tileSize * 3.0f) / 2.0f;

    for(int row = 0; row < 3; row++)
    {
        for(int col = 0; col < 3; col++)
        {
            auto dirtEntity = registry.create();

            const float x = startX + col * tileSize;
            const float y = startY + row * tileSize;

            registry.emplace<Tilled>(dirtEntity, false);
            registry.emplace<Transform>(dirtEntity, SDL_FRect{x, y, tileSize, tileSize});
            registry.emplace<Sprite>(
                dirtEntity,
                dirtTex,
                dirtTex,
                dirtTex,
                SDL_FRect{0.0f, 0.0f, 16.0f, 16.0f},
                SDL_FLIP_NONE
            );
        }
    }

    last_time = SDL_GetTicks();
    isRunning = true;
    return true;
}

void Game::handleEvents()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_EVENT_QUIT)
        {
            isRunning = false;
        }
        else if(event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
        {
            isRunning = false;
        }
    }

    const bool* keys = SDL_GetKeyboardState(nullptr);
    float dx = 0.0f;
    float dy = 0.0f;

    if(keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
    {
        dy -= 1.0f;
    }
    if(keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
    {
        dy += 1.0f;
    }
    if(keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
    {
        dx -= 1.0f;
    }
    if(keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
    {
        dx += 1.0f;
    }

    if(dx != 0.0f && dy != 0.0f)
    {
        const float invSqrt2 = 0.70710678f;
        dx *= invSqrt2;
        dy *= invSqrt2;
    }

    auto velocityView = registry.view<Player, Velocity>();
    for(auto entity : velocityView)
    {
        auto& vel = velocityView.get<Velocity>(entity);
        vel.dx = dx * kPlayerSpeed;
        vel.dy = dy * kPlayerSpeed;
    }
}

void Game::update()
{
    const Uint64 current_time = SDL_GetTicks();
    const float delta_time = (current_time - last_time) / 1000.0f;
    last_time = current_time;

    MovementSystem::update(registry, delta_time);

    int winWidth = 0;
    int winHeight = 0;
    SDL_GetWindowSize(window, &winWidth, &winHeight);

    auto playerView = registry.view<Player, Transform, Velocity, Sprite, Animation>();
    for(auto entity : playerView)
    {
        auto& transform = playerView.get<Transform>(entity);
        auto& vel = playerView.get<Velocity>(entity);
        auto& sprite = playerView.get<Sprite>(entity);
        auto& animation = playerView.get<Animation>(entity);

        if(transform.rect.x < 0.0f)
        {
            transform.rect.x = 0.0f;
        }
        if(transform.rect.y < 0.0f)
        {
            transform.rect.y = 0.0f;
        }
        if(transform.rect.x + transform.rect.w > static_cast<float>(winWidth))
        {
            transform.rect.x = static_cast<float>(winWidth) - transform.rect.w;
        }
        if(transform.rect.y + transform.rect.h > static_cast<float>(winHeight))
        {
            transform.rect.y = static_cast<float>(winHeight) - transform.rect.h;
        }

        const bool moving = (fabs(vel.dx) > 0.001f) || (fabs(vel.dy) > 0.001f);
        animation.isMoving = moving;
        const float speed = std::sqrt((vel.dx * vel.dx) + (vel.dy * vel.dy));
        const float speedNorm = std::clamp(speed / kPlayerSpeed, 0.0f, 1.0f);

        if(moving)
        {
            if(fabs(vel.dx) > fabs(vel.dy))
            {
                animation.facing = (vel.dx < 0.0f) ? FacingDirection::LEFT : FacingDirection::RIGHT;
            }
            else
            {
                animation.facing = (vel.dy < 0.0f) ? FacingDirection::UP : FacingDirection::DOWN;
            }
        }

        float targetOffsetX = 0.0f;
        float targetOffsetY = 0.0f;

        if(animation.isMoving)
        {
            if(!animation.wasMoving)
            {
                animation.frameIndex = 0;
                animation.timer = 0.0f;
                animation.idleFrameCursor = 0;
            }

            const float walkFrameDuration =
                kWalkFrameDurationMax + (kWalkFrameDurationMin - kWalkFrameDurationMax) * speedNorm;

            animation.timer += delta_time;
            while(animation.timer >= walkFrameDuration)
            {
                animation.frameIndex = (animation.frameIndex + 1) % kFrameCount;
                animation.timer -= walkFrameDuration;
            }

            const float frameProgress = animation.timer / walkFrameDuration;
            const float stepPhase = (static_cast<float>(animation.frameIndex) + frameProgress) / kFrameCount;
            const float stepAngle = stepPhase * 6.2831853f;

            // Layered move motion: crisp side swing + soft step bob.
            targetOffsetX = std::sin(stepAngle) * kMoveSwayX;
            targetOffsetY = std::fabs(std::sin(stepAngle)) * kMoveBobY;
        }
        else
        {
            if(animation.wasMoving)
            {
                animation.timer = 0.0f;
                animation.idleFrameCursor = 0;
            }

            animation.timer += delta_time;
            while(animation.timer >= kIdleFrameDuration)
            {
                animation.idleFrameCursor = (animation.idleFrameCursor + 1) % kIdleFrameSequenceCount;
                animation.timer -= kIdleFrameDuration;
            }
            animation.frameIndex = kIdleFrameSequence[animation.idleFrameCursor];

            animation.idleBobTime += delta_time;
            const float phase = animation.idleBobTime * kIdleCycleSpeed * kTwoPi;
            const float bounce = 0.5f - 0.5f * std::cos(phase); // 0..1
            const float bounceSoft = std::pow(bounce, 1.45f);
            const float bounceSecondary = 0.5f - 0.5f * std::cos((phase * 2.0f) + 0.65f);
            const float sway = std::sin(phase + 0.9f);
            const float swaySecondary = std::sin((phase * 2.0f) + 1.8f);

            // Mostly-downward motion to feel like a soft knee bend, not floating.
            targetOffsetY = (bounceSoft * kIdleBounceDown) + (bounceSecondary * kIdleBounceDownSecondary);
            targetOffsetX = (sway * kIdleSwayX) + (swaySecondary * kIdleSwayXSecondary);
        }

        animation.idleOffsetX = smoothToward(animation.idleOffsetX, targetOffsetX, kOffsetResponse, delta_time);
        animation.idleOffsetY = smoothToward(animation.idleOffsetY, targetOffsetY, kOffsetResponse, delta_time);
        animation.wasMoving = animation.isMoving;

        sprite.texture = animation.isMoving ? sprite.walkTexture : sprite.idleTexture;
        sprite.flip = SDL_FLIP_NONE;
        sprite.srcRect.x = static_cast<float>(animation.frameIndex) * kCellSize + kFrameInsetX;
        sprite.srcRect.y = static_cast<float>(rowFromFacing(animation.facing)) * kCellSize + kFrameInsetY;
        sprite.srcRect.w = kFrameWidth;
        sprite.srcRect.h = kFrameHeight;
    }
}

void Game::render()
{
    SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
    SDL_RenderClear(renderer);

    RenderSystem::update(registry, renderer);

    SDL_RenderPresent(renderer);
}

void Game::clean()
{
    TextureManager::clean();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    cout << "Da don dep bo nho va thoat game!" << endl;
}
