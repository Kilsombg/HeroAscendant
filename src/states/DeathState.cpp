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
        int tapX = 0, tapY = 0;
        bool isTap = false;

        if (event.type == SDL_FINGERDOWN)
        {
            tapX = static_cast<int>(event.tfinger.x * 1080);
            tapY = static_cast<int>(event.tfinger.y * 1920);
            isTap = true;
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN &&
                 event.button.button == SDL_BUTTON_LEFT)
        {
            tapX = event.button.x;
            tapY = event.button.y;
            isTap = true;
        }

        if (!isTap)
            return;

        int btnW = 400, btnH = 100;
        int btnX = (1080 - btnW) / 2;

        if (UIRenderer::HitTest(tapX, tapY, btnX, 900, btnW, btnH))
            m_ctx.stateManager.Pop();

        if (UIRenderer::HitTest(tapX, tapY, btnX, 1040, btnW, btnH))
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