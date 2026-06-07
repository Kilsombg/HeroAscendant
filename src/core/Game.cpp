#include "Game.h"
#include "../states/MainMenuState.h"

#include <iostream>

Game::Game()
    : m_craftingSystem(m_eventBus), m_questSystem(m_eventBus), m_uiRenderer(m_renderer, m_assets)
{
}

Game::~Game() = default;

int Game::Run()
{
    // --- Initialize ---
    // Window starts at a sensible desktop size. On Android SDL2 ignores
    // these and uses the device's full screen resolution instead.
    if (!m_engine.Init("Hero Ascendant", 540, 960))
    {
        std::cerr << "[Game] Engine initialization failed.\n";
        return 1;
    }

    // Renderer and AssetManager need the SDL_Renderer from Engine.
    m_renderer.Init(m_engine.GetSDLRenderer());
    m_assets.Init(m_engine.GetSDLRenderer());

    // --- System init ---
    InitSystems();

    // --- Load or start new game ---
    LoadOrStartNewGame();

    // Push the first state — MainMenu is the entry point of the game.
    // All state transitions happen through GameStateManager from here.
    m_stateManager.Push(std::make_unique<MainMenuState>(MakeContext()));
    m_stateManager.ApplyPendingChanges(); // Apply immediately so first frame works

    m_running = true;
    m_lastTick = SDL_GetTicks();

    // --- Main Loop ---
    // The structure is always: events → update → render.
    // This order ensures input is processed, state is updated, then drawn.
    while (m_running)
    {
        // Stop if state manager runs out of states
        // (e.g. last state popped without pushing a new one)
        if (m_stateManager.IsEmpty())
        {
            m_running = false;
            break;
        }

        float dt = CalculateDeltaTime();
        HandleEvents();
        Update(dt);
        Render();
    }

    // --- Shutdown ---
    // Destroy assets before renderer (textures need renderer to be alive first
    // but can be destroyed before it).
    ShutdownSystems();
    m_assets.Shutdown();
    m_engine.Shutdown();
    return 0;
}

void Game::InitSystems()
{
    // Load fonts — UIRenderer::GetFont() returns nullptr until these are loaded.
    // Font file must exist at assets/fonts/pixel_font.ttf
    // If not found, text won't draw but game won't crash.
    TTF_Font *small = m_assets.LoadFont("ui_small", "AvelineEleganzaRegular.otf", 30);
    TTF_Font *large = m_assets.LoadFont("ui_large", "AvelineEleganzaRegular.otf", 52);

    if (!small || !large)
        std::cerr << "[Game] WARNING: Font failed to load. "
                  << "Check assets/fonts/pixel_font.ttf exists next to the .exe\n";
    else
        std::cout << "[Game] Fonts loaded successfully.\n";

    m_craftingSystem.InitRecipes();
    m_questSystem.Init();

    std::cout << "[Game] All systems initialized.\n";
}

void Game::ShutdownSystems()
{
    m_questSystem.Shutdown();
}

void Game::LoadOrStartNewGame()
{
    GameSave save;
    if (m_saveSystem.Load(save))
        std::cout << "[Game] Save loaded — hero level " << save.heroLevel << ".\n";
    else
        std::cout << "[Game] No save found — starting new game.\n";
}

void Game::HandleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            m_running = false;
            break;

        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                m_running = false;
            break;

        case SDL_APP_WILLENTERBACKGROUND:
            m_eventBus.Fire(EventType::AppPaused, AppPausedEvent{});
            m_audio.PauseMusic();
            break;

        case SDL_APP_DIDENTERFOREGROUND:
            m_eventBus.Fire(EventType::AppResumed, AppResumedEvent{});
            m_audio.ResumeMusic();
            break;

        default:
            break;
        }

        m_stateManager.HandleInput(event);
    }
}

void Game::Update(float deltaTime)
{
    m_stateManager.ApplyPendingChanges();
    m_eventBus.Dispatch();
    m_questSystem.Update();
    m_stateManager.Update(deltaTime);
}

void Game::Render()
{
    m_stateManager.Render();
    m_renderer.Present();
}

float Game::CalculateDeltaTime()
{
    // SDL_GetTicks() returns milliseconds since SDL_Init.
    // We subtract last frame's tick to get elapsed milliseconds,
    // then divide by 1000 to convert to seconds.
    //
    // Why seconds?
    //   Moving at 200 pixels/second is resolution-independent.
    //   Moving at 3 pixels/frame depends on running at exactly 60fps.
    //   Delta time makes speed consistent at any frame rate.
    Uint32 currentTick = SDL_GetTicks();
    Uint32 elapsed = currentTick - m_lastTick;
    m_lastTick = currentTick;

    // Cap delta time to 100ms (0.1 seconds).
    // If the game lags or is paused in a debugger, deltaTime could be
    // huge, causing objects to jump wildly. 100ms cap prevents this.
    float dt = elapsed / 1000.0f;
    if (dt > 0.1f)
        dt = 0.1f;

    return dt;
}

StateContext Game::MakeContext()
{
    return StateContext{
        m_renderer,
        m_assets,
        m_audio,
        m_eventBus,
        m_stateManager,
        m_inventory,
        m_craftingSystem,
        m_questSystem,
        m_saveSystem,
        m_uiRenderer};
}