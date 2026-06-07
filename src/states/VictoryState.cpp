#include "VictoryState.h"
#include "GameStateManager.h"
#include "MainMenuState.h"

#include "../renderer/Renderer.h"
#include "../ui/UIRenderer.h"

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
        m_ctx.stateManager.PopAll(
            std::make_unique<MainMenuState>(m_ctx));
    }

    // Continue button: center x=540, y=1000-1100
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

        if (tapX >= 340 && tapX <= 740 && tapY >= 1000 && tapY <= 1100)
            m_ctx.stateManager.PopAll(std::make_unique<MainMenuState>(m_ctx));
    }
}

void VictoryState::Update(float deltaTime)
{
    (void)deltaTime;
}

void VictoryState::Render()
{
    // 150 coins per stage as placeholder — real loot system calculates this
    m_ctx.uiRenderer.DrawVictoryOverlay(150);
}