#include "MainMenuState.h"
#include "GameStateManager.h"
#include "BattleState.h"

#include "../renderer/Renderer.h"

#include <iostream>

MainMenuState::MainMenuState(StateContext ctx)
    : m_ctx(ctx)
{
}

void MainMenuState::OnEnter()
{
    std::cout << "[MainMenuState] Entered.\n";
}

void MainMenuState::OnExit()
{
    std::cout << "[MainMenuState] Exited.\n";
}

void MainMenuState::OnResume()
{
    std::cout << "[MainMenuState] Resumed.\n";
    // Called when returning from a sub-state (e.g. back from battle)
}

void MainMenuState::HandleInput(const SDL_Event &event)
{
    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.sym == SDLK_RETURN)
        {
            // ENTER key starts a battle — temporary keyboard shortcut for testing
            std::cout << "[MainMenuState] Starting battle (press Enter).\n";
            m_ctx.stateManager.Replace(
                std::make_unique<BattleState>(m_ctx, /*stageId=*/0));
        }
    }
}

void MainMenuState::Update(float deltaTime)
{
    (void)deltaTime;
}

void MainMenuState::Render()
{
    // Temporary: solid dark background with a label
    m_ctx.renderer.Clear(Color{15, 15, 35, 255}); // Deep dark blue
}