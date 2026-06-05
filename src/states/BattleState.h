#pragma once

#include "IGameState.h"
#include "../events/GameEvents.h"
#include "../events/EventBus.h"
#include "../entities/EntityManager.h"

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
    int m_floorIndex = 0;                      // Current floor (0-based)
    static constexpr int FLOORS_PER_STAGE = 4; // 3 normal + 1 boss

    // EntityManager lives here — created in OnEnter, destroyed in OnExit
    std::unique_ptr<EntityManager> m_entityManager;

    // Cached hero ID for fast lookup each frame
    uint32_t m_heroId = 0;
    HeroMoveState m_heroMoveState = HeroMoveState::Moving;
    uint32_t m_currentTargetId = 0; // Entity ID of enemy hero is walking toward

    // Hero movement speed in virtual pixels per second
    static constexpr float HERO_MOVE_SPEED = 220.0f;

    // Floor management
    void SpawnFloor();      // Spawns enemies for m_floorIndex
    void AdvanceFloor();    // Move to next floor or spawn boss
    void CheckFloorClear(); // Called after each enemy death

    // Floor direction — true = hero moves left→right on this floor
    // Flips on every AdvanceFloor()
    bool m_movingRight = true;

    // Timer after floor clear before advancing — brief pause for effect
    float m_floorClearTimer = 0.0f;
    static constexpr float FLOOR_CLEAR_DELAY = 0.6f; // seconds

    // Event listener IDs — stored so we can unsubscribe in OnExit()
    ListenerID m_enemyDiedListenerId = 0;
    ListenerID m_heroDiedListenerId = 0;
    ListenerID m_bossDiedListenerId = 0;

    // Private helpers
    void OnEnemyDied(const EnemyDiedEvent &data);
    void OnHeroDied(const HeroDiedEvent &data);
    void OnBossDied(const BossDiedEvent &data);

    void HandleAttackInput(); // Called on click/tap

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
    // Helpers
    // -----------------------------------------------------------------------

    // HeroStartX / EnemyStartX based on current floor direction
    float HeroStartX() const;
    float EnemyStartX(int index, int total) const;

    // Flip enemy render component to face the correct direction
    void SetEnemyFacing(Entity *enemy) const;
};