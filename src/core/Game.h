#pragma once

#include "Engine.h"
#include "AssetManager.h"
#include "../renderer/Renderer.h"
#include "../audio/AudioManager.h"
#include "../events/EventBus.h"

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
    // The four Layer 1 systems
    Engine m_engine;
    Renderer m_renderer;
    AssetManager m_assets;
    AudioManager m_audio;

    EventBus m_eventBus;

    bool m_running = false;

    // Delta time — how many seconds passed since last frame.
    // Used to make movement/animation frame-rate independent.
    // Example: moving 200 pixels/second = position += 200 * deltaTime each frame.
    float m_deltaTime = 0.0f;
    Uint32 m_lastTick = 0;

    // Loop steps — called every frame
    void HandleEvents();
    void Update(float deltaTime);
    void Render();

    // Calculates delta time from SDL tick counter
    float CalculateDeltaTime();
};