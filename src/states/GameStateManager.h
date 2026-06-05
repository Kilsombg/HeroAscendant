#pragma once

#include "IGameState.h"

#include <memory>
#include <vector>

// ==============================================================================
// GameStateManager
//
// Manages a stack of IGameState objects.
// The top of the stack is the "active" state that receives input and updates.
// Multiple states can render if lower states are transparent.
//
// Stack visualization:
//
//   Normal battle:          Paused:              Death screen:
//   ┌─────────────┐         ┌─────────────┐      ┌─────────────┐
//   │ BattleState │ ← top   │ PauseState  │ top  │  DeathState │ top
//   └─────────────┘         ├─────────────┤      ├─────────────┤
//                           │ BattleState │      │ BattleState │
//                           └─────────────┘      └─────────────┘
//
//   BattleState.IsTransparent() = false → only battle renders
//   PauseState.IsTransparent()  = true  → pause + battle both render
//   DeathState.IsTransparent()  = true  → death screen + battle both render
//
// Deferred operations:
//   Push/Pop/Replace don't happen immediately when called during Update()
//   or Render() — that would invalidate the stack mid-iteration.
//   Instead they are queued as PendingOp structs and applied at the
//   start of the NEXT frame in ApplyPendingChanges().
//
//   Why PendingOp struct instead of std::function?
//   std::function requires its stored callable to be copy-constructible.
//   Lambdas that capture unique_ptr cannot be copied (deleted copy ctor).
//   The PendingOp struct owns the state via unique_ptr and carries an enum
//   for the operation type — no lambdas, no copy requirement.
//
// ==============================================================================

class GameStateManager
{
public:
    GameStateManager();
    ~GameStateManager();

    // Disable copy
    GameStateManager(const GameStateManager &) = delete;
    GameStateManager &operator=(const GameStateManager &) = delete;

    // -----------------------------------------------------------------------
    // Stack operations — all deferred to next frame
    // -----------------------------------------------------------------------

    // Push — adds a new state on top of the stack.
    // Current top state gets OnPause() called.
    // New state gets OnEnter() called.
    // Use for: opening pause menu, death screen, victory screen
    void Push(std::unique_ptr<IGameState> state);

    // Pop — removes the top state.
    // Top state gets OnExit() called, then destroyed.
    // New top state gets OnResume() called.
    // Use for: closing pause menu, dismissing death screen
    void Pop();

    // Replace — removes top state and pushes a new one.
    // Top state gets OnExit() called. New state gets OnEnter() called.
    // Use for: MainMenu → Battle, Battle → MainMenu after stage clear
    void Replace(std::unique_ptr<IGameState> state);

    // PopAll — clears the entire stack and pushes one state.
    // Use for: returning to main menu from deep in the game.
    void PopAll(std::unique_ptr<IGameState> state);

    // -----------------------------------------------------------------------
    // Per-frame — called by Game every frame
    // -----------------------------------------------------------------------

    // ApplyPendingChanges — processes queued Push/Pop/Replace operations.
    // Game calls this at the START of each frame before HandleInput.
    void ApplyPendingChanges();

    // HandleInput — passes event to the TOP state only.
    void HandleInput(const SDL_Event &event);

    // Update — updates the TOP state only.
    // Paused states below don't update (battle freezes under pause menu).
    void Update(float deltaTime);

    // Render — renders from BOTTOM to TOP respecting IsTransparent().
    void Render();

    // -----------------------------------------------------------------------
    // Utility
    // -----------------------------------------------------------------------

    bool IsEmpty() const { return m_states.empty(); }

    IGameState *GetCurrentState() const;

private:
    // The state stack. Back = top (active state).
    std::vector<std::unique_ptr<IGameState>> m_states;

    // Operation type for the pending queue
    enum class OpType
    {
        Push,
        Pop,
        Replace,
        PopAll
    };

    // Pending operation — owns the incoming state (may be nullptr for Pop)
    // This replaces std::function to avoid the move-only lambda problem.
    struct PendingOp
    {
        OpType type;
        std::unique_ptr<IGameState> state; // nullptr for Pop

        // Must be explicitly moveable since unique_ptr is move-only
        PendingOp(OpType t, std::unique_ptr<IGameState> s)
            : type(t), state(std::move(s)) {}

        // Move only — no copying
        PendingOp(PendingOp &&) = default;
        PendingOp &operator=(PendingOp &&) = default;
        PendingOp(const PendingOp &) = delete;
        PendingOp &operator=(const PendingOp &) = delete;
    };

    std::vector<PendingOp> m_pendingOps;

    // Internal helpers — perform the operation immediately.
    // Only called from ApplyPendingChanges().
    void DoPush(std::unique_ptr<IGameState> state);
    void DoPop();
    void DoReplace(std::unique_ptr<IGameState> state);
    void DoPopAll(std::unique_ptr<IGameState> state);
};