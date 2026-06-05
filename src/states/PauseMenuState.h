#pragma once

#include "IGameState.h"

// ==============================================================================
// PauseMenuState
//
// Pushed on top of BattleState when player pauses.
// Battle is frozen and visible behind this overlay.
//
// IsTransparent() = true — BattleState renders underneath.
//
// Transitions:
//   Pop → returns to BattleState (resume)
//   GameStateManager::PopAll → MainMenuState (quit to menu)
//
// ==============================================================================
class PauseMenuState : public IGameState
{
public:
    explicit PauseMenuState(StateContext ctx);

    void OnEnter() override;
    void OnExit() override;

    void HandleInput(const SDL_Event &event) override;
    void Update(float deltaTime) override;
    void Render() override;

    // Transparent — battle renders underneath the pause overlay
    bool IsTransparent() const override { return true; }

private:
    StateContext m_ctx;
};