#include "BattleState.h"
#include "GameStateManager.h"
#include "PauseMenuState.h"
#include "DeathState.h"
#include "VictoryState.h"

#include "../events/EventBus.h"
#include "../renderer/Renderer.h"

#include <iostream>

BattleState::BattleState(StateContext ctx, int stageId)
    : m_ctx(ctx), m_stageId(stageId)
{
}

// ==============================================================================
// Lifecycle
// ==============================================================================

void BattleState::OnEnter()
{
    std::cout << "[BattleState] Entered stage " << m_stageId << ".\n";

    // Subscribe to events that BattleState needs to react to.
    // listens and triggers state transitions.

    m_enemyDiedListenerId = m_ctx.eventBus.Subscribe(EventType::EnemyDied,
                                                     [this](const Event &e)
                                                     {
                                                         OnEnemyDied(std::get<EnemyDiedEvent>(e.data));
                                                     });

    m_heroDiedListenerId = m_ctx.eventBus.Subscribe(EventType::HeroDied,
                                                    [this](const Event &e)
                                                    {
                                                        OnHeroDied(std::get<HeroDiedEvent>(e.data));
                                                    });

    m_bossDiedListenerId = m_ctx.eventBus.Subscribe(EventType::BossDied,
                                                    [this](const Event &e)
                                                    {
                                                        OnBossDied(std::get<BossDiedEvent>(e.data));
                                                    });
}

void BattleState::OnExit()
{
    std::cout << "[BattleState] Exited.\n";

    // Always unsubscribe in OnExit — not the destructor.
    // The state may be destroyed after the EventBus in shutdown order.
    m_ctx.eventBus.Unsubscribe(EventType::EnemyDied, m_enemyDiedListenerId);
    m_ctx.eventBus.Unsubscribe(EventType::HeroDied, m_heroDiedListenerId);
    m_ctx.eventBus.Unsubscribe(EventType::BossDied, m_bossDiedListenerId);
}

void BattleState::OnPause()
{
    // Called when PauseMenuState or DeathState is pushed on top.
    // Stop the auto-attack timer so hero doesn't attack while paused.
    std::cout << "[BattleState] Paused.\n";
    m_autoAttackTimer = 0.0f;
}

void BattleState::OnResume()
{
    // Called when returning from pause menu.
    std::cout << "[BattleState] Resumed.\n";
}

// ==============================================================================
// Per-frame
// ==============================================================================

void BattleState::HandleInput(const SDL_Event &event)
{
    // Pause button — keyboard P for now, replaced by UI button later
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p)
    {
        m_ctx.stateManager.Push(
            std::make_unique<PauseMenuState>(m_ctx));
        return;
    }

    // Attack input — left mouse click or screen tap
    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        HandleAttackInput();
    }

    // Cross-platform touch input
    // SDL_FINGERDOWN fires on Android/iOS instead of SDL_MOUSEBUTTONDOWN
    if (event.type == SDL_FINGERDOWN)
    {
        HandleAttackInput();
    }
}

void BattleState::Update(float deltaTime)
{
    // Auto-attack timer — hero attacks automatically every interval
    m_autoAttackTimer += deltaTime;
    if (m_autoAttackTimer >= m_autoAttackInterval)
    {
        m_autoAttackTimer = 0.0f;

        std::cout << "[BattleState] Auto-attack fired.\n";
    }
}

void BattleState::Render()
{
    // Background
    m_ctx.renderer.Clear(Color{20, 10, 10, 255}); // Dark red — battle atmosphere
}

// ==============================================================================
// Event handlers
// ==============================================================================

void BattleState::OnEnemyDied(const EnemyDiedEvent &data)
{
    std::cout << "[BattleState] Enemy " << data.enemyId
              << " died on floor " << data.floorIndex << ".\n";

    // For now, simulate floor cleared after first enemy kill
    m_ctx.eventBus.FireDeferred(EventType::FloorCleared, FloorClearedEvent{
                                                             m_floorIndex,       // floorIndex
                                                             4,                  // totalFloors
                                                             (m_floorIndex == 3) // isFinalFloor
                                                         });
}

void BattleState::OnHeroDied(const HeroDiedEvent &data)
{
    std::cout << "[BattleState] Hero died on floor " << data.floorIndex << ".\n";

    // Push death screen ON TOP of battle — battle stays visible behind it
    m_ctx.stateManager.Push(
        std::make_unique<DeathState>(m_ctx, data.stageIndex, data.floorIndex));
}

void BattleState::OnBossDied(const BossDiedEvent &data)
{
    std::cout << "[BattleState] Boss died. Stage " << data.stageIndex << " cleared.\n";

    m_ctx.stateManager.Push(
        std::make_unique<VictoryState>(m_ctx, data.stageIndex));
}

// ==============================================================================
// Private helpers
// ==============================================================================

void BattleState::HandleAttackInput()
{
    std::cout << "[BattleState] Player attack input.\n";

    // Temporary: fire the event directly for testing
    m_ctx.eventBus.Fire(EventType::HeroAttacked, HeroAttackedEvent{
                                                     10,     // damage
                                                     false,  // wasCritical
                                                     540.0f, // targetPosX
                                                     960.0f  // targetPosY
                                                 });
}