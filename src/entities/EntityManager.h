#pragma once

#include "Entity.h"
#include "components/TransformComponent.h"
#include "components/HealthComponent.h"
#include "components/CombatComponent.h"
#include "components/RenderComponent.h"
#include "components/StatsComponent.h"
#include "components/AIComponent.h"
#include "components/EnemyTypeComponent.h"

#include "../events/EventBus.h"
#include "../renderer/Renderer.h"
#include "../core/AssetManager.h"

#include <memory>
#include <vector>
#include <cstdint>

enum class EnemyType
{
    Goblin,
    Skeleton,
    Orc,
    Dragon, // Boss
};

struct LootEntry
{
    uint32_t materialId;
    std::string materialName;
    int minQuantity;
    int maxQuantity;
    float dropChance; // 0.0 to 1.0
};

struct LootTable
{
    int baseCoins;    // Flat coin reward per kill
    int coinVariance; // +/- variance on coin reward
    std::vector<LootEntry> materials;
};

// ==============================================================================
// EntityManager
//
// Owns ALL entities in the game. Updates them each frame,
// and removes dead ones automatically.
//
// Entity lookup:
//   Systems that need to act on a specific entity (e.g. "deal damage to
//   entity 42") call GetEntityById(42). The EventBus carries entity IDs
//   in event payloads for exactly this purpose.
//
// Dead entity removal:
//   Entities call MarkDead() on themselves (via HealthComponent).
//   EntityManager::RemoveDeadEntities() sweeps them out once per frame
//   at the END of Update(), after all systems have finished processing.
//   This prevents iterator invalidation mid-frame.
//
// ==============================================================================

class EntityManager
{
public:
    EntityManager(EventBus &eventBus, AssetManager &assets);
    ~EntityManager() = default;

    // Disable copy
    EntityManager(const EntityManager &) = delete;
    EntityManager &operator=(const EntityManager &) = delete;

    // -----------------------------------------------------------------------
    // Factory methods — create entities with correct components attached
    // -----------------------------------------------------------------------

    // CreateHero — creates the player character.
    // Returns the hero's entity ID. Store it for quick access.
    // Only one hero should exist at a time.
    uint32_t CreateHero(float x, float y);

    // CreateEnemy — creates an enemy entity of the given type.
    // floorIndex affects stat scaling (enemies get stronger per floor).
    // Returns the enemy's entity ID.
    uint32_t CreateEnemy(EnemyType type, float x, float y, int floorIndex = 0);

    // -----------------------------------------------------------------------
    // Per-frame
    // -----------------------------------------------------------------------

    // Update — calls Update() on all living entities.
    // Call this from BattleState::Update().
    void Update(float deltaTime);

    // RemoveDeadEntities — destroys entities that have been marked dead.
    // Call this AFTER Update() and AFTER all systems have processed events.
    void RemoveDeadEntities();

    // Render — draws all entities with a RenderComponent.
    // Call this from BattleState::Render().
    void Render(Renderer &renderer);

    // -----------------------------------------------------------------------
    // Lookup
    // -----------------------------------------------------------------------

    // GetEntityById — find entity by its unique ID.
    // Returns nullptr if not found (may have already been removed).
    Entity *GetEntityById(uint32_t id) const;

    // GetHero — direct access to the hero entity.
    // Returns nullptr if hero hasn't been created or has died.
    Entity *GetHero() const;

    // GetEnemies — all currently living enemy entities.
    // Used by BattleState to check if floor is cleared.
    std::vector<Entity *> GetLivingEnemies() const;

    // GetEnemyCount — how many enemies are alive right now
    int GetLivingEnemyCount() const;

    // Returns the loot table for a given enemy type
    static const LootTable &GetLootTable(EnemyType type);

    void SetBattleContext(int stageIndex) { m_stageIndex = stageIndex; }

    // -----------------------------------------------------------------------
    // Cleanup
    // -----------------------------------------------------------------------

    // Clear — removes ALL entities. Call on state exit.
    void Clear();

private:
    EventBus &m_eventBus;
    AssetManager &m_assets;

    // All entities. unique_ptr = EntityManager owns them.
    std::vector<std::unique_ptr<Entity>> m_entities;

    // Hero ID cached for fast GetHero() lookup
    uint32_t m_heroId = 0;

    int m_stageIndex = 0;

    // -----------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------

    // AddEntity — takes ownership, returns raw pointer for further setup
    Entity *AddEntity(std::unique_ptr<Entity> entity);

    // Stat scaling — enemies get harder on later floors
    int ScaleHP(int baseHP, int floorIndex) const;
    int ScaleDamage(int baseDmg, int floorIndex) const;
};