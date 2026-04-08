#include "game.h"

#include "../components/animation.h"
#include "../components/player.h"
#include "../components/sprite.h"
#include "../components/transform.h"
#include "../components/velocity.h"
#include "../systems/movement_system.h"
#include "../systems/render_system.h"
#include "../utils/texture_manager.h"

#include <cmath>

namespace
{
    constexpr float kPlayerSpeed = 180.0f;
    constexpr float kFrameWidth = 20.0f;
    constexpr float kFrameHeight = 20.0f;
    constexpr int kFrameCount = 4;
    constexpr float kFrameDuration = 0.1f;
    constexpr float kPlayerScale = 2.0f;

    int rowFromFacing(FacingDirection facing)
    {
        switch(facing)
        {
            case FacingDirection::UP:
                return 4;
            case FacingDirection::LEFT:
            case FacingDirection::RIGHT:
                return 2;
            case FacingDirection::DOWN:
            default:
                return 0;
        }
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

    SDL_Texture* idleTex = TextureManager::LoadTexture("assets/Character/16x16/16x16 Idle-Sheet.png", renderer);
    SDL_Texture* walkTex = TextureManager::LoadTexture("assets/Character/16x16/16x16 Walk-Sheet.png", renderer);
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
    registry.emplace<Animation>(playerEntity, 0, 0.0f, FacingDirection::DOWN, false);

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

        if(animation.isMoving)
        {
            animation.timer += delta_time;
            if(animation.timer >= kFrameDuration)
            {
                animation.frameIndex = (animation.frameIndex + 1) % kFrameCount;
                animation.timer = 0.0f;
            }
        }
        else
        {
            animation.frameIndex = 0;
            animation.timer = 0.0f;
        }

        sprite.texture = animation.isMoving ? sprite.walkTexture : sprite.idleTexture;
        sprite.flip = (animation.facing == FacingDirection::LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        sprite.srcRect.x = static_cast<float>(animation.frameIndex) * kFrameWidth;
        sprite.srcRect.y = static_cast<float>(rowFromFacing(animation.facing)) * kFrameHeight;
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
