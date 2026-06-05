#include "DeathState.h"
#include "GameStateManager.h"
#include "MainMenuState.h"

#include "../renderer/Renderer.h"

#include <iostream>

DeathState::DeathState(StateContext ctx, int stageIndex, int floorIndex)
    : m_ctx(ctx), m_stageIndex(stageIndex), m_floorIndex(floorIndex)
{
}

void DeathState::OnEnter()
{
    std::cout << "[DeathState] Hero died on stage "
              << m_stageIndex << " floor " << m_floorIndex << ".\n";
}

void DeathState::OnExit()
{
    std::cout << "[DeathState] Exited.\n";
}

void DeathState::HandleInput(const SDL_Event &event)
{
    if (event.type == SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_r:
            // Resurrect — pop death screen, BattleState resumes
            std::cout << "[DeathState] Resurrecting.\n";
            m_ctx.stateManager.Pop();
            break;

        case SDLK_m:
            // Quit to menu
            m_ctx.stateManager.PopAll(
                std::make_unique<MainMenuState>(m_ctx));
            break;

        default:
            break;
        }
    }
}

void DeathState::Update(float deltaTime)
{
    (void)deltaTime;
}

void DeathState::Render()
{
    // Dark red semi-transparent overlay over the frozen battle
    m_ctx.renderer.DrawRect(0, 0, 1080, 1920, Color{60, 0, 0, 200}, true);
}