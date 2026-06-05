#pragma once

#include "IGameState.h"
#include "../events/GameEvents.h"
#include "../events/EventBus.h"

// ==============================================================================
// BattleState
//
// The main gameplay state. Handles floor-by-floor combat.
//
// Responsibilities:
//   - Spawn enemies for the current floor
//   - Handle click/tap to deal damage
//   - Manage auto-attack timer
//   - Track floor progress and trigger FloorCleared event
//   - Spawn boss on final floor
//
// Transitions:
//   Push  → PauseMenuState   when player taps pause
//   Push  → DeathState       when hero HP reaches 0
//   Push  → VictoryState     when boss dies
//   Replace→ MainMenuState   when player quits to menu
//
// ==============================================================================
class BattleState : public IGameState
{
public:
    // stageId identifies which stage's floors/enemies to load
    BattleState(StateContext ctx, int stageId);

    void OnEnter() override;
    void OnExit() override;
    void OnPause() override;
    void OnResume() override;

    void HandleInput(const SDL_Event &event) override;
    void Update(float deltaTime) override;
    void Render() override;

    bool IsTransparent() const override { return false; }

private:
    StateContext m_ctx;
    int m_stageId = 0;
    int m_floorIndex = 0; // Current floor (0-based)

    // Auto-attack timer
    // Hero attacks automatically every m_autoAttackInterval seconds
    float m_autoAttackTimer = 0.0f;
    float m_autoAttackInterval = 1.5f; // seconds between auto-attacks

    // Event listener IDs — stored so we can unsubscribe in OnExit()
    ListenerID m_enemyDiedListenerId = 0;
    ListenerID m_heroDiedListenerId = 0;
    ListenerID m_bossDiedListenerId = 0;

    // Private helpers
    void OnEnemyDied(const EnemyDiedEvent &data);
    void OnHeroDied(const HeroDiedEvent &data);
    void OnBossDied(const BossDiedEvent &data);

    void HandleAttackInput(); // Called on click/tap
};