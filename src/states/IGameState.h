#pragma once

#include <SDL2/SDL.h>

// Forward declarations — keeps compile times fast, avoids circular deps
class Renderer;
class AssetManager;
class AudioManager;
class EventBus;
class GameStateManager;
class Inventory;
class CraftingSystem;
class QuestSystem;
class SaveSystem;
class UIRenderer;

// ==============================================================================
// StateContext — everything a state needs to do its job.
//
// ==============================================================================
struct StateContext
{
    Renderer &renderer;
    AssetManager &assets;
    AudioManager &audio;

    EventBus &eventBus;

    GameStateManager &stateManager;

    Inventory &inventory;
    CraftingSystem &craftingSystem;
    QuestSystem &questSystem;
    SaveSystem &saveSystem;

    UIRenderer &uiRenderer;
};

// ==============================================================================
// IGameState
//
// Abstract base class that every game state inherits from.
//
// Lifecycle:
//   OnEnter()  — called once when state becomes active.
//                Load assets, subscribe to events, start music here.
//   OnExit()   — called once when state is removed from the stack.
//                Unload assets, unsubscribe events, stop music here.
//   OnPause()  — called when another state is pushed ON TOP of this one.
//   OnResume() — called when the state on top is popped.
//
// Per-frame:
//   HandleInput(event) — process one SDL_Event
//   Update(dt)         — advance logic
//   Render()           — draw this state
//
// IsTransparent():
//   true  = state below also renders (overlays: pause, death, victory)
//   false = only this state renders (full-screen: battle, main menu)
// ==============================================================================
class IGameState
{
public:
    virtual ~IGameState() = default;

    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual void OnPause() {}  // Optional — default does nothing
    virtual void OnResume() {} // Optional — default does nothing

    virtual void HandleInput(const SDL_Event &event) = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;

    virtual bool IsTransparent() const { return false; }
};