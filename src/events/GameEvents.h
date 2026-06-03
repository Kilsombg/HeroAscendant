#pragma once

// ==============================================================================
// GameEvents.h — All event types and their data payloads.
//
// Every event that can be fired is defined here.
//
// Structure:
//   - EventType enum  — unique ID for each event
//   - Event structs   — data carried with each event
//
// How to add a new event:
//   1. Add a value to EventType enum
//   2. Add a struct with the data it carries (can be empty if no data needed)
//   3. Add it to the EventData variant at the bottom
//
// Design note:
//   Events carry data as value types (int, float, bool) not pointers.
//   This is intentional — events may be processed after the source object
//   is gone. Copying the data is safe; keeping a pointer is not.
//
// ==============================================================================

#include <cstdint>
#include <variant>
#include <string>

// ==============================================================================
// EventType — unique identifier for each event in the game.
// Using enum class (C++11+) prevents accidental integer comparisons.
//
// ==============================================================================
enum class EventType : uint32_t
{
    // --- Combat ---
    EnemyDied,      // An enemy was killed
    HeroTookDamage, // Hero received damage
    HeroDied,       // Hero health reached 0
    BossDied,       // Boss was killed
    HeroAttacked,   // Hero performed an attack (click or auto)
    CriticalHit,    // An attack was a critical hit

    // --- Floor & Stage ---
    FloorCleared, // All enemies on current floor killed
    StageCleared, // Boss killed — stage complete
    FloorChanged, // Player moved to next floor
    BossSpawned,  // Boss appeared on final floor

    // --- Progression ---
    HeroLeveledUp, // Hero gained a level
    StatUpgraded,  // A stat was purchased with coins
    CoinGained,    // Coins were added to player wallet
    CoinSpent,     // Coins were deducted

    // --- Inventory & Crafting ---
    ItemCrafted,    // Player finished crafting an item
    ItemEquipped,   // Player equipped an item
    ItemUnequipped, // Player unequipped an item
    PotionUsed,     // Player consumed a potion

    // --- Dungeon ---
    DungeonEntered,     // Player entered a dungeon
    DungeonCleared,     // Dungeon boss defeated
    DungeonAttemptUsed, // One daily dungeon attempt consumed

    // --- Quests ---
    QuestProgressUpdated, // A quest counter changed
    QuestCompleted,       // A quest reached its goal
    QuestRewardClaimed,   // Player collected quest reward

    // --- UI ---
    ShowNotification, // Show a popup message on screen
    ShowDamageNumber, // Show floating damage number in battle

    // --- App lifecycle (cross-platform) ---
    AppPaused,  // App went to background (Android home button)
    AppResumed, // App came back to foreground

    COUNT // Always last — lets you know how many events exist
};

// ==============================================================================
// Event data structs — the payload carried with each event.
// Keep these small. Only include what listeners actually need.
// ==============================================================================

struct EnemyDiedEvent
{
    uint32_t enemyId; // Which enemy died (entity ID)
    int coinsDropped; // Coins this enemy drops
    int floorIndex;   // Which floor it died on
    float posX, posY; // Where it died (for loot/effects spawning)
};

struct HeroTookDamageEvent
{
    int damage;       // Amount of damage taken
    int remainingHP;  // Hero HP after damage
    int maxHP;        // Hero max HP (for UI health bar percentage)
    bool wasCritical; // Was it a critical hit?
};

struct HeroDiedEvent
{
    int floorIndex; // Which floor the hero died on
    int stageIndex; // Which stage
};

struct BossDiedEvent
{
    uint32_t bossId;  // Which boss died
    int stageIndex;   // Which stage
    float posX, posY; // Where it died (for loot/effects spawning)
};

struct HeroAttackedEvent
{
    int damage; // Damage dealt
    bool wasCritical;
    float targetPosX; // For showing damage numbers at right position
    float targetPosY;
};

struct CriticalHitEvent
{
    int damage;
    float posX, posY;
};

struct FloorClearedEvent
{
    int floorIndex;    // Floor that was cleared (0-based)
    int totalFloors;   // Total floors in this stage
    bool isFinalFloor; // Was this the floor before the boss?
};

struct StageCllearedEvent
{
    int stageIndex;
    int coinsRewarded;
};

struct FloorChangedEvent
{
    int newFloorIndex;
    int totalFloors;
};

struct BossSpawnedEvent
{
    uint32_t bossId;
    int bossMaxHP;
};

struct HeroLeveledUpEvent
{
    int newLevel;
    int previousLevel;
};

struct StatUpgradedEvent
{
    // Which stat was upgraded — using a string keeps it flexible
    // without needing another enum right now
    std::string statName; // "strength", "defense", "agility", "hp"
    int newValue;
    int costPaid;
};

struct CoinGainedEvent
{
    int amount;
    int newTotal;
};

struct CoinSpentEvent
{
    int amount;
    int newTotal;
};

struct ItemCraftedEvent
{
    uint32_t itemId;
    std::string itemName;
    std::string rarity; // "common", "rare", "epic", "legendary"
};

struct ItemEquippedEvent
{
    uint32_t itemId;
    std::string slot; // "weapon", "armor", "accessory"
};

struct ItemUnequippedEvent
{
    uint32_t itemId;
    std::string slot;
};

struct PotionUsedEvent
{
    uint32_t itemId;
    std::string effectType; // "heal", "attack_boost", "defense_boost"
    int effectValue;
};

struct DungeonEnteredEvent
{
    int dungeonId;
    int attemptsRemaining;
};

struct DungeonClearedEvent
{
    int dungeonId;
    int coinsRewarded;
};

struct DungeonAttemptUsedEvent
{
    int dungeonId;
    int attemptsRemaining;
};

struct QuestProgressUpdatedEvent
{
    uint32_t questId;
    int current;
    int goal;
};

struct QuestCompletedEvent
{
    uint32_t questId;
    std::string questName;
};

struct QuestRewardClaimedEvent
{
    uint32_t questId;
    int coinsRewarded;
};

struct ShowNotificationEvent
{
    std::string message;
    float durationSeconds = 2.0f; // How long to show it
};

struct ShowDamageNumberEvent
{
    int damage;
    float worldX, worldY; // Where to show it on screen
    bool isCritical;
    bool isHeroTakingDamage; // true = red number, false = white/yellow
};

struct AppPausedEvent
{
}; // No data needed

struct AppResumedEvent
{
};

// ==============================================================================
// EventData — a type-safe union of all possible event payloads.
//
// std::variant (C++17) holds exactly ONE of its listed types at a time.
// It is type-safe — accessing the wrong type throws std::bad_variant_access.
//
// This is the actual object passed through the EventBus.
// Listeners use std::get<SpecificEventType>(event.data) to read the payload.
//
// Why variant instead of inheritance/void*?
//   - No heap allocation (variant is stack-allocated)
//   - Type-safe — compiler catches wrong type access
//   - No virtual dispatch overhead
//
// ==============================================================================
using EventData = std::variant<
    EnemyDiedEvent,
    HeroTookDamageEvent,
    HeroDiedEvent,
    BossDiedEvent,
    HeroAttackedEvent,
    CriticalHitEvent,
    FloorClearedEvent,
    StageCllearedEvent,
    FloorChangedEvent,
    BossSpawnedEvent,
    HeroLeveledUpEvent,
    StatUpgradedEvent,
    CoinGainedEvent,
    CoinSpentEvent,
    ItemCraftedEvent,
    ItemEquippedEvent,
    ItemUnequippedEvent,
    PotionUsedEvent,
    DungeonEnteredEvent,
    DungeonClearedEvent,
    DungeonAttemptUsedEvent,
    QuestProgressUpdatedEvent,
    QuestCompletedEvent,
    QuestRewardClaimedEvent,
    ShowNotificationEvent,
    ShowDamageNumberEvent,
    AppPausedEvent,
    AppResumedEvent>;

// ==============================================================================
// Event — the complete object that travels through the EventBus.
//
// ==============================================================================
struct Event
{
    EventType type; // What kind of event
    EventData data; // The payload

    // Constructor — EventBus::Fire() uses this
    Event(EventType t, EventData d)
        : type(t), data(std::move(d)) {}
};