#pragma once

#include "IGameState.h"
#include "../events/GameEvents.h"
#include "../events/EventBus.h"
#include "../entities/EntityManager.h"
#include "../ui/DamageNumberSystem.h"

enum class HeroMoveState
{
    Moving,   // Walking toward target enemy
    InCombat, // At attack range, fighting
    Exiting,  // All enemies dead, walking to exit edge
};

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
    int m_floorIndex = 0;                      // Current floor (0-based)
    static constexpr int FLOORS_PER_STAGE = 4; // 3 normal + 1 boss

    ListenerID m_appPausedListenerId = 0;
    void OnAppPaused();

    // EntityManager lives here — created in OnEnter, destroyed in OnExit
    std::unique_ptr<EntityManager> m_entityManager;

    int m_enemiesKilledThisFloor = 0; // track for resurrection respawn point

    uint32_t m_heroId = 0;
    HeroMoveState m_heroMoveState = HeroMoveState::Moving;
    uint32_t m_currentTargetId = 0; // Entity ID of enemy hero is walking toward

    // Hero movement speed in virtual pixels per second
    static constexpr float HERO_MOVE_SPEED = 220.0f;

    // Floor direction — true = hero moves left→right on this floor
    // Flips on every AdvanceFloor()
    bool m_movingRight = true;

    // Timer after floor clear before advancing — brief pause for effect
    float m_floorClearTimer = 0.0f;
    static constexpr float FLOOR_CLEAR_DELAY = 0.6f; // seconds

    // Event listener IDs
    ListenerID m_enemyDiedListenerId = 0;
    ListenerID m_heroDiedListenerId = 0;
    ListenerID m_bossDiedListenerId = 0;

    // Damage number visual effect system — owned by BattleState
    std::unique_ptr<DamageNumberSystem> m_damageNumbers;

    // -----------------------------------------------------------------------
    // Floor management
    // -----------------------------------------------------------------------
    void SpawnFloor();
    void AdvanceFloor();
    void CheckFloorClear();

    // -----------------------------------------------------------------------
    // Combat / movement update
    // -----------------------------------------------------------------------
    void UpdateCombat(float deltaTime);
    void UpdateHeroMovement(float deltaTime, Entity *hero, Entity *target);
    void UpdateEnemyAttacks(Entity *hero);

    // Returns the nearest living enemy in the current movement direction.
    // "Nearest" means closest to hero along the X axis.
    Entity *FindNextTarget() const;

    // -----------------------------------------------------------------------
    // Input
    // -----------------------------------------------------------------------
    void HandleAttackInput();

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------

    // HeroStartX / EnemyStartX based on current floor direction
    float HeroStartX() const;
    float EnemyStartX(int index, int total) const;

    // Flip enemy render component to face the correct direction
    void SetEnemyFacing(Entity *enemy) const;

    int CalculateStageReward() const;

    // -----------------------------------------------------------------------
    // Event callbacks
    // -----------------------------------------------------------------------
    void OnEnemyDied(const EnemyDiedEvent &data);
    void OnHeroDied(const HeroDiedEvent &data);
    void OnBossDied(const BossDiedEvent &data);
};