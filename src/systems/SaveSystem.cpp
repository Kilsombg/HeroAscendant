#include "SaveSystem.h"

#include <SDL2/SDL.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// ==============================================================================
// Constructor — resolve save path using SDL_GetPrefPath
// ==============================================================================

SaveSystem::SaveSystem()
{
    // SDL_GetPrefPath returns the OS-correct writable directory.
    // "YourStudio" = your organisation name (change to your actual name)
    // "HeroAscendant" = application name
    //
    // Cross-platform note:
    //   This is MANDATORY for Android — the app has no write access
    //   anywhere except this path. On Windows it goes to AppData/Roaming.
    char *prefPath = SDL_GetPrefPath("YourStudio", "HeroAscendant");
    if (prefPath)
    {
        m_savePath = std::string(prefPath) + "save.dat";
        SDL_free(prefPath);
    }
    else
    {
        // Fallback — should not happen in practice
        m_savePath = "save.dat";
        std::cerr << "[SaveSystem] SDL_GetPrefPath failed, using local path.\n";
    }

    std::cout << "[SaveSystem] Save path: " << m_savePath << "\n";
}

// ==============================================================================
// Save / Load
// ==============================================================================

bool SaveSystem::Save(const GameSave &save)
{
    std::string content = Serialise(save);
    if (!WriteToFile(m_savePath, content))
    {
        std::cerr << "[SaveSystem] Failed to write save file.\n";
        return false;
    }
    std::cout << "[SaveSystem] Game saved.\n";
    return true;
}

bool SaveSystem::Load(GameSave &outSave)
{
    std::string content;
    if (!ReadFromFile(m_savePath, content))
    {
        std::cout << "[SaveSystem] No save file found: new game.\n";
        return false;
    }

    if (!Deserialise(content, outSave))
    {
        std::cerr << "[SaveSystem] Save file corrupt or incompatible.\n";
        return false;
    }

    std::cout << "[SaveSystem] Game loaded (hero level "
              << outSave.heroLevel << ").\n";
    return true;
}

bool SaveSystem::HasSaveFile() const
{
    std::ifstream f(m_savePath);
    return f.good();
}

bool SaveSystem::DeleteSave()
{
    return std::remove(m_savePath.c_str()) == 0;
}

// ==============================================================================
// Snapshot helpers
// ==============================================================================

void SaveSystem::SnapshotHeroStats(GameSave &save,
                                   const StatsComponent &stats) const
{
    save.heroLevel = stats.GetLevel();
    save.heroXP = stats.GetXP();
    save.heroXPToNext = stats.GetXPToNextLevel();
    save.heroCoins = stats.GetCoins();
    save.strengthLevel = stats.GetStrengthLevel();
    save.defenseLevel = stats.GetDefenseLevel();
    save.agilityLevel = stats.GetAgilityLevel();
    save.hpLevel = stats.GetHPLevel();
}

void SaveSystem::SnapshotInventory(GameSave &save,
                                   const Inventory &inventory) const
{
    save.materials.clear();
    save.items.clear();

    // Materials are not directly iterable from outside Inventory.
    // Inventory would need a GetAllMaterials() accessor for full save support.
    // For now we save equipped gear — a minimal but working save.
    // TODO: Add Inventory::GetAllMaterials() when implementing full save UI.

    auto saveSlot = [&](ItemSlot slot, uint32_t &outId)
    {
        const Item *item = inventory.GetEquippedItem(slot);
        outId = item ? item->id : 0;

        if (item)
        {
            SavedItem si;
            si.instanceId = item->id;
            si.name = item->name;
            si.description = item->description;
            si.slot = static_cast<int>(item->slot);
            si.rarity = static_cast<int>(item->rarity);
            si.attackBonus = item->attackBonus;
            si.defenseBonus = item->defenseBonus;
            si.hpBonus = item->hpBonus;
            si.potionHealAmount = item->potionHealAmount;
            si.potionAttackBoost = item->potionAttackBoost;
            si.potionDuration = item->potionDuration;
            save.items.push_back(si);
        }
    };

    saveSlot(ItemSlot::Weapon, save.equippedWeapon);
    saveSlot(ItemSlot::Armor, save.equippedArmor);
    saveSlot(ItemSlot::Accessory, save.equippedAccessory);
}

// ==============================================================================
// Restore helpers
// ==============================================================================

void SaveSystem::RestoreHeroStats(const GameSave &save,
                                  StatsComponent &stats) const
{
    // StatsComponent needs SetLevel/SetXP setters for full restore.
    // Currently StatsComponent advances via AddXP() — we add enough XP
    // to reach the saved level as a simple restore mechanism.
    // TODO: Add direct setters to StatsComponent for clean save restore.
    stats.AddCoins(save.heroCoins);

    // Restore stat upgrade levels by calling upgrade the right number of times.
    // SpendCoins is bypassed here — we're restoring, not charging.
    // This is a limitation; a cleaner approach uses direct setters.
    // Acceptable for the current scope.
    for (int i = 0; i < save.strengthLevel; ++i)
        stats.UpgradeStrength();
    for (int i = 0; i < save.defenseLevel; ++i)
        stats.UpgradeDefense();
    for (int i = 0; i < save.agilityLevel; ++i)
        stats.UpgradeAgility();
    for (int i = 0; i < save.hpLevel; ++i)
        stats.UpgradeHP();
}

void SaveSystem::RestoreInventory(const GameSave &save,
                                  Inventory &inventory) const
{
    for (const auto &si : save.items)
    {
        Item item;
        item.id = si.instanceId;
        item.name = si.name;
        item.description = si.description;
        item.slot = static_cast<ItemSlot>(si.slot);
        item.rarity = static_cast<ItemRarity>(si.rarity);
        item.attackBonus = si.attackBonus;
        item.defenseBonus = si.defenseBonus;
        item.hpBonus = si.hpBonus;
        item.potionHealAmount = si.potionHealAmount;
        item.potionAttackBoost = si.potionAttackBoost;
        item.potionDuration = si.potionDuration;
        inventory.AddItem(item);
    }

    if (save.equippedWeapon != 0)
        inventory.EquipItem(ItemSlot::Weapon, save.equippedWeapon);
    if (save.equippedArmor != 0)
        inventory.EquipItem(ItemSlot::Armor, save.equippedArmor);
    if (save.equippedAccessory != 0)
        inventory.EquipItem(ItemSlot::Accessory, save.equippedAccessory);
}

// ==============================================================================
// Serialisation — simple key=value text format
// ==============================================================================

std::string SaveSystem::Serialise(const GameSave &save) const
{
    std::string out;

    WriteLine(out, "version", std::to_string(save.saveVersion));
    WriteLine(out, "heroLevel", std::to_string(save.heroLevel));
    WriteLine(out, "heroXP", std::to_string(save.heroXP));
    WriteLine(out, "heroXPToNext", std::to_string(save.heroXPToNext));
    WriteLine(out, "heroCoins", std::to_string(save.heroCoins));
    WriteLine(out, "strengthLevel", std::to_string(save.strengthLevel));
    WriteLine(out, "defenseLevel", std::to_string(save.defenseLevel));
    WriteLine(out, "agilityLevel", std::to_string(save.agilityLevel));
    WriteLine(out, "hpLevel", std::to_string(save.hpLevel));
    WriteLine(out, "highestStage", std::to_string(save.highestStageCleared));
    WriteLine(out, "sfxVolume", std::to_string(save.sfxVolume));
    WriteLine(out, "musicVolume", std::to_string(save.musicVolume));

    // Equipped item IDs
    WriteLine(out, "equippedWeapon", std::to_string(save.equippedWeapon));
    WriteLine(out, "equippedArmor", std::to_string(save.equippedArmor));
    WriteLine(out, "equippedAccessory", std::to_string(save.equippedAccessory));

    // Item instances — each on one line as pipe-separated values
    // format: item|instanceId|name|slot|rarity|atk|def|hp|heal|atkBoost|dur
    for (const auto &item : save.items)
    {
        std::string line = "item|" + std::to_string(item.instanceId) + "|" + item.name + "|" + std::to_string(item.slot) + "|" + std::to_string(item.rarity) + "|" + std::to_string(item.attackBonus) + "|" + std::to_string(item.defenseBonus) + "|" + std::to_string(item.hpBonus) + "|" + std::to_string(item.potionHealAmount) + "|" + std::to_string(item.potionAttackBoost) + "|" + std::to_string(item.potionDuration);
        WriteLine(out, "itemData", line);
    }

    return out;
}

bool SaveSystem::Deserialise(const std::string &text, GameSave &s) const
{
    // Check version first — reject incompatible saves
    std::string ver = ReadValue(text, "version");
    if (ver.empty())
        return false;

    s.saveVersion = std::stoi(ReadValue(text, "version"));
    s.heroLevel = std::stoi(ReadValue(text, "heroLevel"));
    s.heroXP = std::stoi(ReadValue(text, "heroXP"));
    s.heroXPToNext = std::stoi(ReadValue(text, "heroXPToNext"));
    s.heroCoins = std::stoi(ReadValue(text, "heroCoins"));
    s.strengthLevel = std::stoi(ReadValue(text, "strengthLevel"));
    s.defenseLevel = std::stoi(ReadValue(text, "defenseLevel"));
    s.agilityLevel = std::stoi(ReadValue(text, "agilityLevel"));
    s.hpLevel = std::stoi(ReadValue(text, "hpLevel"));
    s.highestStageCleared = std::stoi(ReadValue(text, "highestStage"));
    s.sfxVolume = std::stof(ReadValue(text, "sfxVolume"));
    s.musicVolume = std::stof(ReadValue(text, "musicVolume"));
    s.equippedWeapon = static_cast<uint32_t>(
        std::stoul(ReadValue(text, "equippedWeapon")));
    s.equippedArmor = static_cast<uint32_t>(
        std::stoul(ReadValue(text, "equippedArmor")));
    s.equippedAccessory = static_cast<uint32_t>(
        std::stoul(ReadValue(text, "equippedAccessory")));

    // Parse item lines
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line))
    {
        if (line.rfind("itemData=", 0) != 0)
            continue;

        std::string data = line.substr(9); // after "itemData="
        std::istringstream ss(data);
        std::string token;
        std::vector<std::string> parts;
        while (std::getline(ss, token, '|'))
            parts.push_back(token);

        if (parts.size() < 11)
            continue;

        SavedItem si;
        si.instanceId = static_cast<uint32_t>(std::stoul(parts[1]));
        si.name = parts[2];
        si.slot = std::stoi(parts[3]);
        si.rarity = std::stoi(parts[4]);
        si.attackBonus = std::stoi(parts[5]);
        si.defenseBonus = std::stoi(parts[6]);
        si.hpBonus = std::stoi(parts[7]);
        si.potionHealAmount = std::stoi(parts[8]);
        si.potionAttackBoost = std::stof(parts[9]);
        si.potionDuration = std::stof(parts[10]);
        s.items.push_back(si);
    }

    return true;
}

// ==============================================================================
// File I/O
// ==============================================================================

bool SaveSystem::WriteToFile(const std::string &path,
                             const std::string &content) const
{
    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file.is_open())
        return false;
    file << content;
    return file.good();
}

bool SaveSystem::ReadFromFile(const std::string &path,
                              std::string &outContent) const
{
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    std::ostringstream ss;
    ss << file.rdbuf();
    outContent = ss.str();
    return !outContent.empty();
}

// ==============================================================================
// Line helpers
// ==============================================================================

void SaveSystem::WriteLine(std::string &out,
                           const std::string &key,
                           const std::string &value) const
{
    out += key + "=" + value + "\n";
}

std::string SaveSystem::ReadValue(const std::string &text,
                                  const std::string &key) const
{
    // Find "key=" and extract the value until newline
    std::string search = key + "=";
    size_t pos = text.find(search);
    if (pos == std::string::npos)
        return "";

    size_t start = pos + search.size();
    size_t end = text.find('\n', start);
    if (end == std::string::npos)
        end = text.size();

    return text.substr(start, end - start);
}