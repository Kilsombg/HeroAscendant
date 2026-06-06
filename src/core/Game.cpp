#include "Game.h"
#include "../states/MainMenuState.h"

#include <iostream>

Game::Game()
    : m_craftingSystem(m_eventBus), m_questSystem(m_eventBus)
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
        std::cerr << "[Game] Engine initialization failed. Exiting.\n";
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

// ==============================================================================
// Startup / shutdown helpers
// ==============================================================================

void Game::InitSystems()
{
    // CraftingSystem registers all recipes
    m_craftingSystem.InitRecipes();

    // QuestSystem subscribes to EventBus and registers quests
    m_questSystem.Init();

    std::cout << "[Game] All systems initialized.\n";
}

void Game::ShutdownSystems()
{
    // QuestSystem unsubscribes its EventBus listeners
    m_questSystem.Shutdown();
}

void Game::LoadOrStartNewGame()
{
    GameSave save;
    if (m_saveSystem.Load(save))
    {
        // Restore state from save file.
        // Hero entity doesn't exist yet — stats are stored in the save
        // and applied to the hero entity when MainMenuState creates it.
        std::cout << "[Game] Save loaded — hero level " << save.heroLevel << ".\n";
    }
    else
    {
        std::cout << "[Game] No save found: starting new game.\n";
    }
}

// ==============================================================================
// Per-frame
// ==============================================================================

void Game::HandleEvents()
{
    SDL_Event event;

    // SDL_PollEvent returns 1 while there are events in the queue.
    // We process ALL pending events before moving on to Update.
    // If we only processed one event per frame, input would feel delayed.
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            // SDL_QUIT is fired when the user closes the window,
            // or on Android when the OS is terminating the app.
            m_running = false;
            break;

        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                m_running = false;
            break;

        // Cross-platform note:
        //   SDL_APP_WILLENTERBACKGROUND fires on Android when the user
        //   switches to another app. We should pause music here.
        //   This doesn't exist on Windows (will be ignored).
        case SDL_APP_WILLENTERBACKGROUND:
            // Fire through EventBus so any system can react to app pause
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

        // Forward every event to the active state AFTER global handling.
        // State gets the event even if we handled it above — states may
        // also want to react to SDL_QUIT (e.g. auto-save before exit).
        m_stateManager.HandleInput(event);
    }
}

void Game::Update(float deltaTime)
{
    // 1. Apply any Push/Pop/Replace operations queued last frame
    m_stateManager.ApplyPendingChanges();

    // 2. Dispatch deferred events at the START of each frame,
    // before any system runs its Update(). This ensures events
    // fired last frame are delivered before new logic executes.
    m_eventBus.Dispatch();

    // 3. Update quest system (checks daily/weekly reset timestamps)
    m_questSystem.Update();

    // 3. Update the active state
    m_stateManager.Update(deltaTime);
}

void Game::Render()
{
    // 1. Clear the screen
    m_renderer.Clear(Color{10, 10, 40, 255}); // Dark blue background

    // 2. GameStateManager renders bottom-to-top respecting IsTransparent()
    m_stateManager.Render();

    // 3. Present (swap buffers)
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
    // Bundle references to all systems a state might need.
    // Passing references (not pointers) means states can't store nullptr.
    return StateContext{
        m_renderer,
        m_assets,
        m_audio,
        m_eventBus,
        m_stateManager,
        m_inventory,
        m_craftingSystem,
        m_questSystem,
        m_saveSystem};
}