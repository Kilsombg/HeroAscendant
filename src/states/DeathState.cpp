#include "DeathState.h"
#include "GameStateManager.h"
#include "MainMenuState.h"

#include "../renderer/Renderer.h"
#include "../ui/UIRenderer.h"

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

    // Touch/mouse — Resurrect button: y=920-1020, Quit button: y=1060-1160
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

        if (tapX >= 340 && tapX <= 740 && tapY >= 920 && tapY <= 1020)
            m_ctx.stateManager.Pop();

        if (tapX >= 340 && tapX <= 740 && tapY >= 1060 && tapY <= 1160)
            m_ctx.stateManager.PopAll(std::make_unique<MainMenuState>(m_ctx));
    }
}

void DeathState::Update(float deltaTime)
{
    (void)deltaTime;
}

void DeathState::Render()
{
    m_ctx.uiRenderer.DrawDeathOverlay();
}