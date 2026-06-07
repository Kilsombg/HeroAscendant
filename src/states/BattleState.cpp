#include "BattleState.h"
#include "GameStateManager.h"
#include "PauseMenuState.h"
#include "DeathState.h"
#include "VictoryState.h"

#include "../entities/EntityManager.h"
#include "../entities/Entity.h"
#include "../entities/components/TransformComponent.h"
#include "../entities/components/HealthComponent.h"
#include "../entities/components/CombatComponent.h"
#include "../entities/components/AIComponent.h"
#include "../entities/components/RenderComponent.h"
#include "../entities/components/StatsComponent.h"
#include "../renderer/Renderer.h"
#include "../ui/UIRenderer.h"

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cmath>

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

    m_entityManager = std::make_unique<EntityManager>(m_ctx.eventBus, m_ctx.assets);

    m_enemyDiedListenerId = m_ctx.eventBus.Subscribe(EventType::EnemyDied,
                                                     [this](const Event &e)
                                                     { OnEnemyDied(std::get<EnemyDiedEvent>(e.data)); });

    m_heroDiedListenerId = m_ctx.eventBus.Subscribe(EventType::HeroDied,
                                                    [this](const Event &e)
                                                    { OnHeroDied(std::get<HeroDiedEvent>(e.data)); });

    m_bossDiedListenerId = m_ctx.eventBus.Subscribe(EventType::BossDied,
                                                    [this](const Event &e)
                                                    { OnBossDied(std::get<BossDiedEvent>(e.data)); });

    // Floor 0 always starts moving left→right
    m_floorIndex = 0;
    m_movingRight = true;

    // Spawn hero at starting edge for floor 0
    m_heroId = m_entityManager->CreateHero(HeroStartX(), 928.0f);

    SpawnFloor();
}

void BattleState::OnExit()
{
    std::cout << "[BattleState] Exited.\n";

    m_ctx.eventBus.Unsubscribe(EventType::EnemyDied, m_enemyDiedListenerId);
    m_ctx.eventBus.Unsubscribe(EventType::HeroDied, m_heroDiedListenerId);
    m_ctx.eventBus.Unsubscribe(EventType::BossDied, m_bossDiedListenerId);

    m_entityManager.reset();
}

void BattleState::OnPause() { std::cout << "[BattleState] Paused.\n"; }
void BattleState::OnResume() { std::cout << "[BattleState] Resumed.\n"; }

// ==============================================================================
// Per-frame
// ==============================================================================

void BattleState::HandleInput(const SDL_Event &event)
{
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p)
    {
        m_ctx.stateManager.Push(std::make_unique<PauseMenuState>(m_ctx));
        return;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        HandleAttackInput();
    }

    if (event.type == SDL_FINGERDOWN)
    {
        HandleAttackInput();
    }
}

void BattleState::Update(float deltaTime)
{
    if (!m_entityManager)
        return;

    // Floor clear delay — brief pause before spawning next floor
    if (m_floorClearTimer > 0.0f)
    {
        m_floorClearTimer -= deltaTime;
        if (m_floorClearTimer <= 0.0f)
        {
            m_floorClearTimer = 0.0f;
            AdvanceFloor();
        }
        // Don't update combat during the floor transition pause
        m_entityManager->Update(deltaTime);
        m_entityManager->RemoveDeadEntities();
        return;
    }

    m_entityManager->Update(deltaTime);
    UpdateCombat(deltaTime);
    m_entityManager->RemoveDeadEntities();

    if (m_damageNumbers)
        m_damageNumbers->Update(deltaTime);
}

void BattleState::Render()
{
    m_ctx.renderer.Clear(Color{20, 10, 10, 255});

    if (m_entityManager)
        m_entityManager->Render(m_ctx.renderer);

    // TODO: HP bars, floor counter, attack button
}

// ==============================================================================
// Combat / movement update
// ==============================================================================

void BattleState::UpdateCombat(float deltaTime)
{
    if (!m_entityManager)
        return;

    Entity *hero = m_entityManager->GetHero();
    if (!hero || !hero->IsAlive())
        return;

    // Find which enemy we are currently targeting
    Entity *target = nullptr;
    if (m_currentTargetId != 0)
        target = m_entityManager->GetEntityById(m_currentTargetId);

    // If current target is gone, find a new one
    if (!target || !target->IsAlive())
    {
        target = FindNextTarget();
        m_currentTargetId = target ? target->GetId() : 0;

        // If no target found and hero was in combat, all enemies are dead
        if (!target && m_heroMoveState == HeroMoveState::InCombat)
        {
            m_heroMoveState = HeroMoveState::Exiting;
        }
    }

    switch (m_heroMoveState)
    {
    case HeroMoveState::Moving:
        if (target)
            UpdateHeroMovement(deltaTime, hero, target);
        break;

    case HeroMoveState::InCombat:
        // Hero is standing still fighting — run attack logic
        if (target)
            UpdateEnemyAttacks(hero);

        // Hero auto-attack
        {
            auto *heroCombat = hero->GetComponent<CombatComponent>();
            if (heroCombat && heroCombat->IsAutoAttackReady() && target)
            {
                heroCombat->ResetAutoAttackTimer();

                auto *targetHealth = target->GetComponent<HealthComponent>();
                if (targetHealth)
                {
                    bool isCrit = (static_cast<float>(rand()) / RAND_MAX) < heroCombat->GetCritChance();
                    int damage = heroCombat->GetAttackDamage();
                    if (isCrit)
                        damage = static_cast<int>(damage * heroCombat->GetCritMultiplier());

                    targetHealth->TakeDamage(damage, isCrit);

                    auto *tf = target->GetComponent<TransformComponent>();
                    if (tf)
                    {
                        m_ctx.eventBus.Fire(EventType::ShowDamageNumber,
                                            ShowDamageNumberEvent{
                                                damage,
                                                tf->GetCenterX(), // worldX
                                                tf->GetCenterY(), // worldY
                                                isCrit,           // isCritical
                                                false             // isHeroTakingDamage
                                            });
                    }
                }
            }
        }
        break;

    case HeroMoveState::Exiting:
        // All enemies on this floor are dead.
        // Hero walks to the far edge and we start the floor clear timer.
        {
            auto *heroTransform = hero->GetComponent<TransformComponent>();
            auto *heroRender = hero->GetComponent<RenderComponent>();
            if (heroTransform)
            {
                // Target exit X: far edge of screen
                float exitX = m_movingRight
                                  ? 1080.0f + 10.0f // Off right edge
                                  : -100.0f;        // Off left edge

                float dx = m_movingRight ? 1.0f : -1.0f;
                heroTransform->Translate(HERO_MOVE_SPEED * deltaTime * dx, 0.0f);

                if (heroRender)
                    heroRender->SetFacingLeft(!m_movingRight);

                // Has hero reached the exit?
                bool reachedExit = m_movingRight
                                       ? heroTransform->GetX() >= exitX
                                       : heroTransform->GetX() <= exitX;

                if (reachedExit)
                {
                    // Start delay before spawning next floor
                    m_floorClearTimer = FLOOR_CLEAR_DELAY;
                }
            }
        }
        break;
    }
}

void BattleState::UpdateHeroMovement(float deltaTime, Entity *hero, Entity *target)
{
    auto *heroTransform = hero->GetComponent<TransformComponent>();
    auto *targetTransform = target->GetComponent<TransformComponent>();
    auto *heroRender = hero->GetComponent<RenderComponent>();
    auto *targetAI = target->GetComponent<AIComponent>();

    if (!heroTransform || !targetTransform || !targetAI)
        return;

    float attackRange = targetAI->GetAttackRange();
    float heroX = heroTransform->GetCenterX();
    float targetX = targetTransform->GetCenterX();
    float distance = std::abs(heroX - targetX);

    if (distance <= attackRange)
    {
        // Hero arrived — switch to combat
        m_heroMoveState = HeroMoveState::InCombat;
        targetAI->SetState(AIState::Attacking);
        std::cout << "[BattleState] Hero reached enemy: combat started.\n";
        return;
    }

    // Hero is still walking — move in the current floor direction
    float direction = m_movingRight ? 1.0f : -1.0f;
    heroTransform->Translate(HERO_MOVE_SPEED * deltaTime * direction, 0.0f);

    // Update hero facing direction to match movement
    if (heroRender)
        heroRender->SetFacingLeft(!m_movingRight);
}

void BattleState::UpdateEnemyAttacks(Entity *hero)
{
    // Only the current target enemy attacks the hero.
    // Other enemies wait — hero isn't in range of them yet.
    Entity *target = m_entityManager->GetEntityById(m_currentTargetId);
    if (!target || !target->IsAlive())
        return;

    auto *ai = target->GetComponent<AIComponent>();
    auto *combat = target->GetComponent<CombatComponent>();
    auto *heroHealth = hero->GetComponent<HealthComponent>();

    if (!ai || !combat || !heroHealth)
        return;
    if (!ai->IsAttackReady())
        return;

    ai->ResetAttackTimer();

    int rawDamage = combat->GetAttackDamage();
    auto *heroStats = hero->GetComponent<StatsComponent>();
    if (heroStats)
        rawDamage = std::max(1, rawDamage - heroStats->GetDefenseValue());

    heroHealth->TakeDamage(rawDamage, false);
}

// ==============================================================================
// Floor management
// ==============================================================================

void BattleState::SpawnFloor()
{
    if (!m_entityManager)
        return;

    bool isBossFloor = (m_floorIndex == FLOORS_PER_STAGE - 1);

    if (isBossFloor)
    {
        std::cout << "[BattleState] Spawning boss floor.\n";

        // Boss spawns on the far side of the direction hero is travelling
        float bossX = m_movingRight ? 900.0f : 180.0f;
        uint32_t bossId = m_entityManager->CreateEnemy(
            EnemyType::Dragon, bossX, 896.0f, m_floorIndex);

        // Boss faces hero
        Entity *boss = m_entityManager->GetEntityById(bossId);
        if (boss)
            SetEnemyFacing(boss);

        m_ctx.eventBus.Fire(EventType::BossSpawned, BossSpawnedEvent{
                                                        bossId, // bossId
                                                        500     // bossMaxHP
                                                    });
    }
    else
    {
        int enemyCount = 2 + m_floorIndex; // 2, 3, or 4 enemies
        float spacing = 140.0f;

        std::cout << "[BattleState] Spawning floor " << m_floorIndex
                  << " (" << enemyCount << " enemies).\n";

        for (int i = 0; i < enemyCount; ++i)
        {
            // Enemies are positioned ahead of the hero in movement direction.
            // Moving right: enemies start at right side, spaced toward right.
            //   Enemy 0 at x=700, enemy 1 at x=840, enemy 2 at x=980
            // Moving left:  enemies start at left side, spaced toward left.
            //   Enemy 0 at x=380, enemy 1 at x=240, enemy 2 at x=100
            float enemyX = EnemyStartX(i, enemyCount);

            EnemyType type = (i % 2 == 0) ? EnemyType::Goblin : EnemyType::Skeleton;
            if (m_floorIndex >= 2)
                type = EnemyType::Orc; // Harder enemies later

            uint32_t enemyId = m_entityManager->CreateEnemy(
                type, enemyX, 928.0f, m_floorIndex);

            // Set facing direction: enemies face the hero (opposite to movement dir)
            Entity *enemy = m_entityManager->GetEntityById(enemyId);
            if (enemy)
                SetEnemyFacing(enemy);
        }
    }

    // Hero starts moving toward the first target
    m_heroMoveState = HeroMoveState::Moving;
    m_currentTargetId = 0; // FindNextTarget() will locate it next Update()

    // Reposition hero at the starting edge of this floor
    Entity *hero = m_entityManager->GetHero();
    if (hero)
    {
        auto *tf = hero->GetComponent<TransformComponent>();
        if (tf)
            tf->SetPosition(HeroStartX(), 928.0f);
    }
}

void BattleState::CheckFloorClear()
{
    if (!m_entityManager)
        return;
    if (m_entityManager->GetLivingEnemyCount() > 0)
        return;

    bool isFinalFloor = (m_floorIndex == FLOORS_PER_STAGE - 1);

    m_ctx.eventBus.Fire(EventType::FloorCleared, FloorClearedEvent{
                                                     m_floorIndex,     // floorIndex
                                                     FLOORS_PER_STAGE, // totalFloors
                                                     isFinalFloor      // isFinalFloor
                                                 });

    if (!isFinalFloor)
    {
        // Hero exits the floor — state machine will call AdvanceFloor()
        // once the hero reaches the edge
        m_heroMoveState = HeroMoveState::Exiting;
    }
    // If final floor, boss died — VictoryState pushed by OnBossDied
}

void BattleState::AdvanceFloor()
{
    m_floorIndex++;

    // Alternate direction each floor
    m_movingRight = !m_movingRight;

    std::cout << "[BattleState] Advancing to floor " << m_floorIndex
              << ": moving " << (m_movingRight ? "right" : "left") << ".\n";

    m_ctx.eventBus.Fire(EventType::FloorChanged, FloorChangedEvent{
                                                     m_floorIndex,    // newFloorIndex
                                                     FLOORS_PER_STAGE // totalFloors
                                                 });

    SpawnFloor();
}

// ==============================================================================
// Helpers
// ==============================================================================

float BattleState::HeroStartX() const
{
    // Hero spawns just inside the edge they'll be walking from
    return m_movingRight ? 60.0f   // Left edge, moving right
                         : 956.0f; // Right edge, moving left (1080 - heroWidth)
}

float BattleState::EnemyStartX(int index, int total) const
{
    (void)total;
    float spacing = 140.0f;
    if (m_movingRight)
    {
        // Hero walks right — enemies are on the right side
        // First enemy closest to hero at x=700, next at x=840 etc.
        return 700.0f + (index * spacing);
    }
    else
    {
        // Hero walks left — enemies are on the left side
        // First enemy at x=380, next at x=240 etc.
        return 380.0f - (index * spacing);
    }
}

void BattleState::SetEnemyFacing(Entity *enemy) const
{
    auto *render = enemy->GetComponent<RenderComponent>();
    if (!render)
        return;

    // Enemy always faces the hero.
    // If hero moves right (hero is on left), enemy faces left (toward hero).
    // If hero moves left (hero is on right), enemy faces right (toward hero).
    render->SetFacingLeft(m_movingRight);
}

Entity *BattleState::FindNextTarget() const
{
    if (!m_entityManager)
        return nullptr;

    auto enemies = m_entityManager->GetLivingEnemies();
    if (enemies.empty())
        return nullptr;

    Entity *hero = m_entityManager->GetHero();
    if (!hero)
        return enemies[0];

    auto *heroTransform = hero->GetComponent<TransformComponent>();
    if (!heroTransform)
        return enemies[0];

    float heroX = heroTransform->GetCenterX();

    // Find the closest enemy in the movement direction.
    // Moving right: find the enemy with the smallest x that is still > heroX.
    // Moving left:  find the enemy with the largest x that is still < heroX.
    Entity *nearest = nullptr;
    float nearestDist = 999999.0f;

    for (Entity *enemy : enemies)
    {
        auto *tf = enemy->GetComponent<TransformComponent>();
        if (!tf)
            continue;

        float dist = std::abs(tf->GetCenterX() - heroX);

        if (m_movingRight && tf->GetCenterX() > heroX && dist < nearestDist)
        {
            nearest = enemy;
            nearestDist = dist;
        }
        else if (!m_movingRight && tf->GetCenterX() < heroX && dist < nearestDist)
        {
            nearest = enemy;
            nearestDist = dist;
        }
    }

    // Fallback: if no enemy is "ahead" (should not happen in normal play),
    // just return the absolute nearest
    if (!nearest && !enemies.empty())
        nearest = enemies[0];

    return nearest;
}

// ==============================================================================
// Input
// ==============================================================================

void BattleState::HandleAttackInput()
{
    // Player can only attack when in combat range
    if (m_heroMoveState != HeroMoveState::InCombat)
        return;

    Entity *hero = m_entityManager ? m_entityManager->GetHero() : nullptr;
    if (!hero)
        return;

    auto *heroCombat = hero->GetComponent<CombatComponent>();
    if (!heroCombat)
        return;

    Entity *target = m_entityManager->GetEntityById(m_currentTargetId);
    if (!target || !target->IsAlive())
        return;

    auto *targetHealth = target->GetComponent<HealthComponent>();
    if (!targetHealth)
        return;

    bool isCrit = (static_cast<float>(rand()) / RAND_MAX) < heroCombat->GetCritChance();
    int damage = heroCombat->GetAttackDamage();
    if (isCrit)
        damage = static_cast<int>(damage * heroCombat->GetCritMultiplier());

    targetHealth->TakeDamage(damage, isCrit);

    auto *tf = target->GetComponent<TransformComponent>();
    m_ctx.eventBus.Fire(EventType::HeroAttacked, HeroAttackedEvent{
                                                     damage,                         // damage
                                                     isCrit,                         // wasCritical
                                                     tf ? tf->GetCenterX() : 540.0f, // targetPosX
                                                     tf ? tf->GetCenterY() : 960.0f  // targetPosY
                                                 });
}

// ==============================================================================
// Event callbacks
// ==============================================================================

void BattleState::OnEnemyDied(const EnemyDiedEvent &data)
{
    std::cout << "[BattleState] Enemy " << data.enemyId << " died.\n";

    // Reward hero
    Entity *hero = m_entityManager ? m_entityManager->GetHero() : nullptr;
    if (hero)
    {
        auto *stats = hero->GetComponent<StatsComponent>();
        if (stats)
        {
            stats->AddXP(25);
            stats->AddCoins(15);
        }
    }

    // Clear current target — UpdateCombat will pick the next one
    if (m_currentTargetId == data.enemyId)
    {
        m_currentTargetId = 0;
        // Stay in Moving state — hero will walk to the next enemy
        if (m_heroMoveState == HeroMoveState::InCombat)
            m_heroMoveState = HeroMoveState::Moving;
    }

    CheckFloorClear();
}

void BattleState::OnHeroDied(const HeroDiedEvent & /*data*/)
{
    std::cout << "[BattleState] Hero died.\n";
    m_ctx.stateManager.Push(
        std::make_unique<DeathState>(m_ctx, m_stageId, m_floorIndex));
}

void BattleState::OnBossDied(const BossDiedEvent &data)
{
    std::cout << "[BattleState] Boss defeated. Stage cleared!\n";
    m_ctx.stateManager.Push(
        std::make_unique<VictoryState>(m_ctx, data.stageIndex));
}