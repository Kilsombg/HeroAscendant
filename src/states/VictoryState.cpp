#include "VictoryState.h"
#include "GameStateManager.h"
#include "MainMenuState.h"

#include "../renderer/Renderer.h"
#include "../ui/UIRenderer.h"

#include <iostream>

VictoryState::VictoryState(StateContext ctx, int stageIndex, int coinsEarned)
    : m_ctx(ctx), m_stageIndex(stageIndex), m_coinsEarned(coinsEarned)
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
        m_ctx.stateManager.PopAll(
            std::make_unique<MainMenuState>(m_ctx));
    }

    // Continue button: center x=540, y=1000-1100
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

void VictoryState::Update(float deltaTime)
{
    (void)deltaTime;
}

void VictoryState::Render()
{
    m_ctx.uiRenderer.DrawVictoryOverlay(m_coinsEarned);
}