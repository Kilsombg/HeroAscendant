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
        int tapX = 0, tapY = 0;
        bool isTap = false;

        if (event.type == SDL_FINGERDOWN)
        {
            tapX = static_cast<int>(event.tfinger.x * 1920); // landscape width
            tapY = static_cast<int>(event.tfinger.y * 1080);
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

        int btnW = 400, btnH = 90;
        int btnX = (1920 - btnW) / 2; // 760

        if (UIRenderer::HitTest(tapX, tapY, btnX, 420, btnW, btnH))
            m_ctx.stateManager.Pop();

        if (UIRenderer::HitTest(tapX, tapY, btnX, 540, btnW, btnH))
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