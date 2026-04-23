#include "game.h"

#include "../components/animation.h"
#include "../components/player.h"
#include "../components/player_controller.h"
#include "../components/sprite.h"
#include "../components/transform.h"
#include "../components/velocity.h"
#include "../components/tilled.h"
#include "../systems/movement_system.h"
#include "../systems/render_system.h"
#include "../systems/animation_system.h"
#include "../systems/player_control_system.h"
#include "../utils/texture_manager.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kFrameWidth = 16.0f;
    constexpr float kFrameHeight = 16.0f;
    constexpr float kPlayerScale = 2.0f;
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
    registry.emplace<PlayerController>(playerEntity);
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
}

void Game::update()
{
    const Uint64 current_time = SDL_GetTicks();
    const float delta_time = (current_time - last_time) / 1000.0f;
    last_time = current_time;

    PlayerControlSystem::update(registry);
    MovementSystem::update(registry, delta_time);

    int winWidth = 0;
    int winHeight = 0;
    SDL_GetWindowSize(window, &winWidth, &winHeight);

    auto playerView = registry.view<Player, Transform>();
    for(auto entity : playerView)
    {
        auto& transform = playerView.get<Transform>(entity);

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
    }

    AnimationSystem::update(registry, delta_time);
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
