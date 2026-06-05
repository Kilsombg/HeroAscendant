#pragma once

#include <SDL2/SDL.h>

// Forward declarations — states need these but we don't include full headers
// here to avoid circular dependencies and keep compile times fast.
class Renderer;
class AssetManager;
class AudioManager;
class EventBus;
class GameStateManager;

// ==============================================================================
// StateContext — everything a state needs to do its job.
//
// Instead of passing N separate parameters to every state function,
// we bundle them into one struct. States store a reference to this.
//
// ==============================================================================
struct StateContext
{
    Renderer &renderer;
    AssetManager &assets;
    AudioManager &audio;
    EventBus &eventBus;
    GameStateManager &stateManager;
};

// ==============================================================================
// IGameState
//
// Abstract base class that every game state inherits from.
// Each state implements its own version of these four methods.
//
// Lifecycle:
//   OnEnter()  — called once when this state becomes active
//                Load assets, subscribe to events, start music here.
//
//   OnExit()   — called once when this state is removed from the stack
//                Unload assets, unsubscribe events, stop music here.
//
//   OnPause()  — called when another state is pushed ON TOP of this one
//                (e.g. BattleState gets paused when PauseMenuState is pushed)
//                Stop updating logic but keep state alive.
//
//   OnResume() — called when the state on top is popped and this becomes
//                active again. Restart logic, resume music etc.
//
// Every frame (only for the TOP state, or ALL states for rendering):
//   HandleInput(event) — process one SDL_Event
//   Update(dt)         — advance logic by deltaTime seconds
//   Render(renderer)   — draw this state
//
// IsTransparent():
//   Returns true if the state below should also be rendered.
//   PauseMenuState returns true  → battle visible behind pause menu
//   BattleState returns false    → nothing rendered below it
//
// ==============================================================================
class IGameState
{
public:
    virtual ~IGameState() = default;

    // Lifecycle — called by GameStateManager at the right moments
    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual void OnPause() {}  // Optional — default does nothing
    virtual void OnResume() {} // Optional — default does nothing

    // Per-frame — called by GameStateManager every frame
    virtual void HandleInput(const SDL_Event &event) = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;

    // Stack rendering — does the state below this one also get rendered?
    // Override and return true for overlay states (pause, death, victory)
    virtual bool IsTransparent() const { return false; }
};