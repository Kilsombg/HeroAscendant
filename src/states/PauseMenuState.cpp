#include "PauseMenuState.h"
#include "GameStateManager.h"
#include "MainMenuState.h"

#include "../renderer/Renderer.h"
#include "../ui/UIRenderer.h"

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

    // Touch/mouse — check against button regions
    // Resume button: center x=540, y=900, w=400, h=100
    // Quit button:   center x=540, y=1040, w=400, h=100
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_FINGERDOWN)
    {
        int tapX, tapY;
        if (event.type == SDL_FINGERDOWN)
        {
            tapX = static_cast<int>(event.tfinger.x * 1080);
            tapY = static_cast<int>(event.tfinger.y * 1920);
        }
        else
        {
            tapX = event.button.x;
            tapY = event.button.y;
        }

        // Resume button region
        if (tapX >= 340 && tapX <= 740 && tapY >= 900 && tapY <= 1000)
            m_ctx.stateManager.Pop();

        // Quit button region
        if (tapX >= 340 && tapX <= 740 && tapY >= 1040 && tapY <= 1140)
            m_ctx.stateManager.PopAll(std::make_unique<MainMenuState>(m_ctx));
    }
}

void PauseMenuState::Update(float deltaTime)
{
    (void)deltaTime;
}

void PauseMenuState::Render()
{
    // BattleState renders first (IsTransparent=true), then we draw overlay
    m_ctx.uiRenderer.DrawPauseOverlay();
}