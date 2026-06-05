#pragma once

#include "IGameState.h"

// ==============================================================================
// VictoryState
//
// Pushed on top of BattleState when the boss dies.
// Shows rewards, loot, stage clear message.
//
// Transitions:
//   Continue → PopAll → MainMenuState (return to hub)
//
// IsTransparent() = true — battle visible behind victory screen.
//
// ==============================================================================
class VictoryState : public IGameState
{
public:
    VictoryState(StateContext ctx, int stageIndex);

    void OnEnter() override;
    void OnExit() override;

    void HandleInput(const SDL_Event &event) override;
    void Update(float deltaTime) override;
    void Render() override;

    bool IsTransparent() const override { return true; }

private:
    StateContext m_ctx;
    int m_stageIndex = 0;
};