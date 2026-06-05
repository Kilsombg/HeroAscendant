#pragma once

#include "IGameState.h"

// ==============================================================================
// DeathState
//
// Pushed on top of BattleState when hero dies.
// Battle is visible frozen behind this overlay.
//
// Player choices:
//   Resurrect → Pop (returns to BattleState, which resets the floor)
//   Quit      → PopAll → MainMenuState
//
// IsTransparent() = true — shows frozen battle behind death screen.
//
// ==============================================================================
class DeathState : public IGameState
{
public:
    DeathState(StateContext ctx, int stageIndex, int floorIndex);

    void OnEnter() override;
    void OnExit() override;

    void HandleInput(const SDL_Event &event) override;
    void Update(float deltaTime) override;
    void Render() override;

    bool IsTransparent() const override { return true; }

private:
    StateContext m_ctx;
    int m_stageIndex = 0;
    int m_floorIndex = 0;
};