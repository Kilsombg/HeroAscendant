#pragma once

#include "IGameState.h"
#include "../events/GameEvents.h"

// ==============================================================================
// MainMenuState
//
// The first state the game starts in.
// Handles: character display, navigation to Battle/Dungeon/Crafting/Quests
//
// ==============================================================================
class MainMenuState : public IGameState
{
public:
    explicit MainMenuState(StateContext ctx);

    void OnEnter() override;
    void OnExit() override;
    void OnResume() override;

    void HandleInput(const SDL_Event &event) override;
    void Update(float deltaTime) override;
    void Render() override;

    // MainMenu is the bottom of the stack — nothing renders below it
    bool IsTransparent() const override { return false; }

private:
    StateContext m_ctx;
};