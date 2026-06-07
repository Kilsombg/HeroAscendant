#pragma once

#include "Engine.h"
#include "AssetManager.h"
#include "../renderer/Renderer.h"
#include "../audio/AudioManager.h"
#include "../events/EventBus.h"
#include "../states/GameStateManager.h"
#include "../systems/Inventory.h"
#include "../systems/CraftingSystem.h"
#include "../systems/QuestSystem.h"
#include "../systems/SaveSystem.h"
#include "../ui/UIRenderer.h"

// ==============================================================================
// Game — owns the main loop and all top-level systems.
//
// This is the root of the game. It creates Engine, Renderer, AssetManager,
// and AudioManager, then runs the loop.
//
// Lifetime:
//   main.cpp creates one Game instance, calls Run(), then it's done.
//   When Run() returns, Game's destructor cleans up everything.
//
// ==============================================================================
class Game
{
public:
    Game();
    ~Game();

    // Run initializes everything, runs the loop until quit, then shuts down.
    // Returns 0 on clean exit, 1 on initialization failure.
    int Run();

private:
    // SDL2 foundation
    Engine m_engine;
    Renderer m_renderer;
    AssetManager m_assets;
    AudioManager m_audio;

    // Event system
    // Must be declared before any system that subscribes to events
    EventBus m_eventBus;

    // State Management
    GameStateManager m_stateManager;

    // Progression systems

    Inventory m_inventory;
    CraftingSystem m_craftingSystem;
    QuestSystem m_questSystem;
    SaveSystem m_saveSystem;

    // UI
    UIRenderer m_uiRenderer;

    bool m_running = false;
    Uint32 m_lastTick = 0;

    // Loop steps — called every frame

    void HandleEvents();
    void Update(float deltaTime);
    void Render();

    // Calculates delta time from SDL tick counter
    float CalculateDeltaTime();

    // Startup / shutdown helpers

    void InitSystems();
    void ShutdownSystems();
    void LoadOrStartNewGame();

    // Builds the StateContext that gets passed to every state
    StateContext MakeContext();
};