#include "PauseMenuState.h"
#include "GameStateManager.h"
#include "MainMenuState.h"

#include "../renderer/Renderer.h"

#include <iostream>

PauseMenuState::PauseMenuState(StateContext ctx)
    : m_ctx(ctx)
{
}

void PauseMenuState::OnEnter()
{
    std::cout << "[PauseMenuState] Entered.\n";
}

void PauseMenuState::OnExit()
{
    std::cout << "[PauseMenuState] Exited.\n";
}

void PauseMenuState::HandleInput(const SDL_Event &event)
{
    if (event.type == SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_p:
        case SDLK_ESCAPE:
            // Resume — pop pause menu, BattleState gets OnResume()
            m_ctx.stateManager.Pop();
            break;

        case SDLK_m:
            // Quit to main menu — clears entire stack
            m_ctx.stateManager.PopAll(
                std::make_unique<MainMenuState>(m_ctx));
            break;

        default:
            break;
        }
    }
}

void PauseMenuState::Update(float deltaTime)
{
    (void)deltaTime;
    // Pause menu has no logic to update
}

void PauseMenuState::Render()
{
    // Draw a semi-transparent dark overlay over the battle.
    m_ctx.renderer.SetDrawAlpha(180); // ~70% opaque dark overlay
    m_ctx.renderer.DrawRect(0, 0, 1080, 1920, Color{0, 0, 0, 180}, true);
}