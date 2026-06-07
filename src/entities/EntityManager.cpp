#include "EntityManager.h"

#include "../systems/CraftingSystem.h"

#include <algorithm>
#include <iostream>

EntityManager::EntityManager(EventBus &eventBus, AssetManager &assets)
    : m_eventBus(eventBus), m_assets(assets)
{
}

// ==============================================================================
// Factory methods
// ==============================================================================

uint32_t EntityManager::CreateHero(float x, float y)
{
    auto entity = std::make_unique<Entity>("Hero");

    // --- Transform ---
    // Hero sprite size: 64x64 virtual pixels (pixel art, scaled up)
    entity->AddComponent<TransformComponent>(x, y, 64, 64);

    // --- Health ---
    // Hero starts with 200 HP. StatsComponent HP upgrades add to this.
    auto *health = entity->AddComponent<HealthComponent>(200, m_eventBus);
    health->SetIsHero(true); // Fire HeroTookDamage event on damage
    health->SetBattleContext(0, m_stageIndex);

    // --- Combat ---
    // Base attack: 15 damage, 1 auto-attack per second, 5% crit chance
    entity->AddComponent<CombatComponent>(15, 1.0f, 0.05f);

    // --- Render ---
    // "hero_idle" is the asset ID. Sprite sheet: 64x64 frames, 4 idle frames
    // Assets must be loaded before CreateHero is called (in BattleState::OnEnter)
    entity->AddComponent<RenderComponent>(
        "hero_idle", // textureId
        64, 64,      // frameW, frameH
        4,           // totalFrames in idle animation
        0.15f,       // frameInterval — ~6.7 fps animation
        false        // facingLeft — hero faces right
    );

    // --- Stats ---
    // StatsComponent is hero-only — enemies don't level up or hold coins
    entity->AddComponent<StatsComponent>(m_eventBus);

    uint32_t id = entity->GetId();
    m_heroId = id;

    AddEntity(std::move(entity));
    return id;
}

uint32_t EntityManager::CreateEnemy(EnemyType type, float x, float y, int floorIndex)
{
    // Each enemy type has base stats. Floor index scales them up.
    // moveSpeed removed — enemies are stationary, hero walks to them.
    struct EnemyData
    {
        const char *name;
        const char *textureId;
        int baseHP;
        int baseDamage;
        float attackRange;    // Distance hero must be within to trigger combat
        float attackInterval; // Seconds between enemy attacks
        int frameW, frameH;
        int totalFrames;
        bool isBoss;
    };

    // Base stat table — floor scaling applied below
    // name,        textureId,       HP,  dmg,  range,  interval, fw,  fh,  frames, boss
    static const EnemyData enemyData[] = {
        {"Goblin", "goblin_idle", 60, 8, 80.0f, 1.5f, 48, 48, 4, false},
        {"Skeleton", "skeleton_idle", 80, 12, 80.0f, 1.2f, 48, 64, 4, false},
        {"Orc", "orc_idle", 120, 18, 90.0f, 1.0f, 64, 64, 4, false},
        {"Dragon", "dragon_idle", 500, 35, 200.0f, 0.8f, 128, 128, 6, true},
    };

    const EnemyData &data = enemyData[static_cast<int>(type)];

    auto entity = std::make_unique<Entity>(data.name);

    // --- Transform ---
    entity->AddComponent<TransformComponent>(x, y, data.frameW, data.frameH);

    // --- Health (scaled by floor) ---
    int scaledHP = ScaleHP(data.baseHP, floorIndex);
    auto *health = entity->AddComponent<HealthComponent>(scaledHP, m_eventBus);
    health->SetBattleContext(floorIndex, m_stageIndex);

    // --- Combat (scaled by floor) ---
    int scaledDmg = ScaleDamage(data.baseDamage, floorIndex);
    entity->AddComponent<CombatComponent>(
        scaledDmg,
        1.0f / data.attackInterval, // Convert interval to attacks/second
        0.05f                       // 5% crit chance for all enemies
    );

    // --- Render ---
    // Enemies always face left initially — BattleState flips them per floor
    // direction when the hero approaches from the other side.
    entity->AddComponent<RenderComponent>(
        data.textureId,
        data.frameW, data.frameH,
        data.totalFrames,
        0.15f,
        true // facingLeft default — adjusted by BattleState per floor direction
    );

    // --- AI ---
    // All enemies use Stationary — they never move, just attack when hero arrives.
    entity->AddComponent<AIComponent>(
        AIBehaviour::Stationary,
        data.attackRange,
        data.attackInterval);

    entity->AddComponent<EnemyTypeComponent>(type);

    uint32_t id = entity->GetId();
    AddEntity(std::move(entity));
    return id;
}

// ==============================================================================
// Per-frame
// ==============================================================================

void EntityManager::Update(float deltaTime)
{
    for (auto &entity : m_entities)
    {
        if (entity->IsAlive())
        {
            entity->Update(deltaTime);
        }
    }
}

void EntityManager::RemoveDeadEntities()
{
    // Erase-remove idiom — removes all entities where IsAlive() is false.
    // unique_ptr destructor calls entity destructor, which calls OnDetach()
    // on all components — clean shutdown.
    m_entities.erase(
        std::remove_if(m_entities.begin(), m_entities.end(),
                       [](const std::unique_ptr<Entity> &e)
                       {
                           return !e->IsAlive();
                       }),
        m_entities.end());
}

void EntityManager::Render(Renderer &renderer)
{
    for (auto &entity : m_entities)
    {
        if (!entity->IsAlive())
            continue;

        auto *render = entity->GetComponent<RenderComponent>();
        auto *transform = entity->GetComponent<TransformComponent>();

        if (!render || !render->IsVisible() || !transform)
            continue;

        // Get the texture from AssetManager by ID
        SDL_Texture *texture = m_assets.GetTexture(render->GetTextureId());
        if (!texture)
            continue;

        // DrawTextureRegion draws one frame from the sprite sheet
        renderer.DrawTextureRegion(
            texture,
            render->GetSrcX(),                   // srcX — frame column * frameW
            render->GetSrcY(),                   // srcY — animation row * frameH
            render->GetFrameW(),                 // srcW
            render->GetFrameH(),                 // srcH
            static_cast<int>(transform->GetX()), // dstX
            static_cast<int>(transform->GetY()), // dstY
            transform->GetW(),                   // dstW (draw size — may differ from frame size)
            transform->GetH(),                   // dstH
            0.0,                                 // angle
            render->IsFacingLeft()               // flipH
        );
    }
}

// ==============================================================================
// Lookup
// ==============================================================================

Entity *EntityManager::GetEntityById(uint32_t id) const
{
    for (auto &entity : m_entities)
    {
        if (entity->GetId() == id)
            return entity.get();
    }
    return nullptr;
}

Entity *EntityManager::GetHero() const
{
    return GetEntityById(m_heroId);
}

std::vector<Entity *> EntityManager::GetLivingEnemies() const
{
    std::vector<Entity *> result;
    for (auto &entity : m_entities)
    {
        // Hero has StatsComponent, enemies don't — use this to distinguish.
        // AIComponent is enemy-only.
        if (entity->IsAlive() && entity->HasComponent<AIComponent>())
        {
            result.push_back(entity.get());
        }
    }
    return result;
}

int EntityManager::GetLivingEnemyCount() const
{
    int count = 0;
    for (auto &entity : m_entities)
    {
        if (entity->IsAlive() && entity->HasComponent<AIComponent>())
            count++;
    }
    return count;
}

void EntityManager::Clear()
{
    // OnDetach is called via Entity destructor for each component
    m_entities.clear();
    m_heroId = 0;
}

const LootTable &EntityManager::GetLootTable(EnemyType type)
{
    // Static loot tables — defined once, referenced by all enemies of that type.

    static const LootTable goblinLoot = {
        10, 5, // 10 ± 5 coins
        {
            {CraftingSystem::MAT_BONE, "Bone", 1, 2, 0.6f},
            {CraftingSystem::MAT_LEATHER, "Leather", 1, 1, 0.3f},
        }};
    static const LootTable skeletonLoot = {
        15, 5, {
                   {CraftingSystem::MAT_BONE, "Bone", 2, 3, 0.8f},
                   {CraftingSystem::MAT_IRON_ORE, "Iron Ore", 1, 1, 0.2f},
               }};
    static const LootTable orcLoot = {
        25, 8, {
                   {CraftingSystem::MAT_IRON_ORE, "Iron Ore", 1, 2, 0.5f},
                   {CraftingSystem::MAT_LEATHER, "Leather", 1, 2, 0.4f},
                   {CraftingSystem::MAT_MAGIC_CRYSTAL, "Magic Crystal", 1, 1, 0.1f},
               }};
    static const LootTable dragonLoot = {
        200, 50, {
                     {CraftingSystem::MAT_DRAGON_SCALE, "Dragon Scale", 2, 4, 0.9f},
                     {CraftingSystem::MAT_MAGIC_CRYSTAL, "Magic Crystal", 1, 2, 0.6f},
                     {CraftingSystem::MAT_IRON_ORE, "Iron Ore", 3, 5, 0.8f},
                 }};

    switch (type)
    {
    case EnemyType::Goblin:
        return goblinLoot;
    case EnemyType::Skeleton:
        return skeletonLoot;
    case EnemyType::Orc:
        return orcLoot;
    case EnemyType::Dragon:
        return dragonLoot;
    default:
        return goblinLoot;
    }
}

// ==============================================================================
// Private helpers
// ==============================================================================

Entity *EntityManager::AddEntity(std::unique_ptr<Entity> entity)
{
    Entity *raw = entity.get();
    m_entities.push_back(std::move(entity));
    return raw;
}

int EntityManager::ScaleHP(int baseHP, int floorIndex) const
{
    // Each floor adds 15% HP. Floor 0 = base, floor 3 = boss floor ~145% HP.
    float multiplier = 1.0f + (floorIndex * 0.15f);
    return static_cast<int>(baseHP * multiplier);
}

int EntityManager::ScaleDamage(int baseDmg, int floorIndex) const
{
    // Each floor adds 10% damage.
    float multiplier = 1.0f + (floorIndex * 0.10f);
    return static_cast<int>(baseDmg * multiplier);
}