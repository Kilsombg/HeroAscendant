#pragma once

#include "Inventory.h"
#include "../entities/components/StatsComponent.h"

#include <string>
#include <vector>
#include <cstdint>

// ==============================================================================
// GameSave — Memento Pattern (Behavioral)
//
// This is the Memento. A plain data snapshot of everything that needs
// to persist between sessions. No methods, no logic — just data.
//
// Why a flat struct instead of serialising objects directly?
//   Objects have internal state, pointers, and EventBus references that
//   can't be serialised. The Memento strips all of that out and stores
//   only the values that matter for restoring state.
//
// What needs saving:
//   Hero stats      — level, XP, coins, stat upgrade levels
//   Inventory       — materials, gear instances, equipped items
//   Quest progress  — current counts and completion flags
//   Stage progress  — highest stage reached
//   Settings        — volume, etc.
// ==============================================================================

// Saved inventory material
struct SavedMaterial
{
    uint32_t id;
    std::string name;
    int quantity;
};

// Saved item instance
struct SavedItem
{
    uint32_t instanceId;
    std::string name;
    std::string description;
    int slot;   // ItemSlot cast to int
    int rarity; // ItemRarity cast to int
    int attackBonus;
    int defenseBonus;
    int hpBonus;
    int potionHealAmount;
    float potionAttackBoost;
    float potionDuration;
};

// Saved quest state
struct SavedQuest
{
    uint32_t id;
    int currentCount;
    bool isCompleted;
    bool isRewarded;
};

// The full save file contents
struct GameSave
{
    // Version — lets us handle old save files gracefully
    int saveVersion = 1;

    // Hero stats
    int heroLevel = 1;
    int heroXP = 0;
    int heroXPToNext = 100;
    int heroCoins = 0;
    int strengthLevel = 0;
    int defenseLevel = 0;
    int agilityLevel = 0;
    int hpLevel = 0;

    // Stage progress — highest stage the player has cleared
    int highestStageCleared = -1; // -1 = none cleared yet

    // Inventory
    std::vector<SavedMaterial> materials;
    std::vector<SavedItem> items;
    uint32_t equippedWeapon = 0;
    uint32_t equippedArmor = 0;
    uint32_t equippedAccessory = 0;

    // Quest progress
    std::vector<SavedQuest> dailyQuestProgress;
    std::vector<SavedQuest> weeklyQuestProgress;

    // Settings
    float sfxVolume = 1.0f;
    float musicVolume = 0.7f;
};

// ==============================================================================
// SaveSystem — Memento Pattern (Behavioral)
//
// Creates GameSave snapshots from live game state (Save),
// and restores live game state from snapshots (Load).
//
// Storage:
//   Uses SDL_GetPrefPath() for the save file location — guaranteed to be
//   in the correct writable directory on every platform.
//
//   Windows: C:\Users\<name>\AppData\Roaming\YourStudio\HeroAscendant\
//   Android: /data/data/com.yourstudio.heroascendant/files/
//
// Format:
//   Simple key=value text format — no external library needed.
//   Not human-readable JSON, but reliable and debuggable.
//
// Cross-platform note:
//   SDL_GetPrefPath is the ONLY correct way to get a writable path
//   on Android. Hardcoding any path will fail on Android.
// ==============================================================================
class SaveSystem
{
public:
    SaveSystem();
    ~SaveSystem() = default;

    // -----------------------------------------------------------------------
    // Save / Load
    // -----------------------------------------------------------------------

    // Save — writes a GameSave to disk.
    // Returns true on success.
    bool Save(const GameSave &save);

    // Load — reads the save file and fills a GameSave.
    // Returns true if a valid save file was found and loaded.
    // Returns false if no save exists (new game) — caller uses defaults.
    bool Load(GameSave &outSave);

    // HasSaveFile — quick check before committing to a Load
    bool HasSaveFile() const;

    // DeleteSave — used by "New Game" option
    bool DeleteSave();

    // -----------------------------------------------------------------------
    // Snapshot helpers — build GameSave from live state
    // -----------------------------------------------------------------------

    // SnapshotHeroStats — fills hero stat fields from StatsComponent
    void SnapshotHeroStats(GameSave &save, const StatsComponent &stats) const;

    // SnapshotInventory — fills inventory fields from Inventory
    void SnapshotInventory(GameSave &save, const Inventory &inventory) const;

    // -----------------------------------------------------------------------
    // Restore helpers — apply GameSave back to live state
    // -----------------------------------------------------------------------

    // RestoreHeroStats — applies saved stats to a StatsComponent
    void RestoreHeroStats(const GameSave &save, StatsComponent &stats) const;

    // RestoreInventory — rebuilds Inventory from save data
    void RestoreInventory(const GameSave &save, Inventory &inventory) const;

private:
    std::string m_savePath; // Full path to save file, set in constructor

    // File I/O helpers
    bool WriteToFile(const std::string &path, const std::string &content) const;
    bool ReadFromFile(const std::string &path, std::string &outContent) const;

    // Serialisation helpers — convert GameSave to/from text
    std::string Serialise(const GameSave &save) const;
    bool Deserialise(const std::string &text, GameSave &outSave) const;

    // Line parsing helpers
    void WriteLine(std::string &out, const std::string &key,
                   const std::string &value) const;
    std::string ReadValue(const std::string &text,
                          const std::string &key) const;
};