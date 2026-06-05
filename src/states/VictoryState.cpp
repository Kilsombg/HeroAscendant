#include "VictoryState.h"
#include "GameStateManager.h"
#include "MainMenuState.h"

#include "../renderer/Renderer.h"

#include <iostream>

VictoryState::VictoryState(StateContext ctx, int stageIndex)
    : m_ctx(ctx), m_stageIndex(stageIndex)
{
}

void VictoryState::OnEnter()
{
    std::cout << "[VictoryState] Stage " << m_stageIndex << " cleared!\n";
}

void VictoryState::OnExit()
{
    std::cout << "[VictoryState] Exited.\n";
}

void VictoryState::HandleInput(const SDL_Event &event)
{
    if (event.type == SDL_KEYDOWN &&
        event.key.keysym.sym == SDLK_RETURN)
    {
        // Return to main menu hub
        m_ctx.stateManager.PopAll(
            std::make_unique<MainMenuState>(m_ctx));
    }
}

void VictoryState::Update(float deltaTime)
{
    (void)deltaTime;
}

void VictoryState::Render()
{
    // Golden semi-transparent overlay
    m_ctx.renderer.DrawRect(0, 0, 1080, 1920, Color{80, 60, 0, 180}, true);
}