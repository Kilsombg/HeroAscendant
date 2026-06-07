#pragma once

#include <string>
#include <unordered_map>
#include <vector>

// ==============================================================================
// ItemRarity
//
// ==============================================================================
enum class ItemRarity
{
    Common,
    Rare,
    Epic,
    Legendary
};

// ==============================================================================
// ItemSlot — where an equippable item goes on the hero
//
// ==============================================================================
enum class ItemSlot
{
    Weapon,
    Armor,
    Accessory,
    Potion,   // Consumable — not a gear slot, but stored the same way
    Material, // Crafting ingredient
};

// ==============================================================================
// Item — a single item definition
//
// Items are identified by a uint32_t id. The Inventory stores counts of
// material items (you can have 10 iron ore) and single copies of gear.
//
// Stats on gear are flat bonuses added on top of base stats when equipped.
//
// ==============================================================================
struct Item
{
    uint32_t id = 0;
    std::string name;
    std::string description;
    ItemSlot slot = ItemSlot::Material;
    ItemRarity rarity = ItemRarity::Common;

    // Stat bonuses when equipped (gear only — zero for materials/potions)

    int attackBonus = 0;
    int defenseBonus = 0;
    int hpBonus = 0;

    // Potion effect (potion only)

    int potionHealAmount = 0;
    float potionAttackBoost = 0.0f; // Multiplier added to attack
    float potionDuration = 0.0f;    // Seconds the effect lasts
};

// ==============================================================================
// Inventory
//
// Holds all items the hero owns.
//
// Two storage concepts:
//   Stack items  — materials with a quantity (iron ore x5, bone x3)
//   Unique items — gear and potions, stored individually with an instance ID
//
// ==============================================================================
class Inventory
{
public:
    // -----------------------------------------------------------------------
    // Materials — stackable resources
    // -----------------------------------------------------------------------

    // AddMaterial — adds `quantity` of a material item
    void AddMaterial(uint32_t itemId, const std::string &name, int quantity = 1);

    // RemoveMaterial — removes quantity. Returns false if not enough.
    bool RemoveMaterial(uint32_t itemId, int quantity = 1);

    // GetMaterialCount — how many of this material do we have?
    int GetMaterialCount(uint32_t itemId) const;

    bool HasMaterial(uint32_t itemId, int quantity = 1) const;

    // GetAllMaterials — returns the full material map for serialisation.
    // The map is keyed by item ID and stores name + quantity.
    // Const ref — caller must not modify the map directly.
    struct MaterialStack
    {
        std::string name;
        int quantity = 0;
    }; // already defined privately
    // We expose it publicly here via a typedef so SaveSystem can use the type:
    using MaterialMap = std::unordered_map<uint32_t, MaterialStack>;
    const MaterialMap &GetAllMaterials() const { return m_materials; }

    // -----------------------------------------------------------------------
    // Gear and potions — unique instances
    // -----------------------------------------------------------------------

    // AddItem — adds a crafted or dropped item
    void AddItem(const Item &item);

    // RemoveItem — removes by instance ID (for equipping, consuming)
    bool RemoveItem(uint32_t instanceId);

    // GetItem — find item by instance ID. Returns nullptr if not found.
    const Item *GetItem(uint32_t instanceId) const;

    // GetItemsBySlot — all items in a given slot (e.g. all weapons)
    std::vector<const Item *> GetItemsBySlot(ItemSlot slot) const;

    // -----------------------------------------------------------------------
    // Equipped gear
    // -----------------------------------------------------------------------

    // EquipItem — sets the equipped item for a slot. Returns old item ID (0 if none).
    uint32_t EquipItem(ItemSlot slot, uint32_t instanceId);

    // UnequipItem — clears the slot. Returns the unequipped item ID.
    uint32_t UnequipItem(ItemSlot slot);

    // GetEquippedItem — returns nullptr if slot is empty
    const Item *GetEquippedItem(ItemSlot slot) const;

    // GetTotalAttackBonus — sum of all equipped gear attack bonuses
    int GetTotalAttackBonus() const;
    int GetTotalDefenseBonus() const;
    int GetTotalHPBonus() const;

    // -----------------------------------------------------------------------
    // Potions
    // -----------------------------------------------------------------------

    // EquipPotion — player can equip up to MAX_EQUIPPED_POTIONS before a battle
    static constexpr int MAX_EQUIPPED_POTIONS = 3;

    bool EquipPotion(uint32_t instanceId);
    bool UnequipPotion(int slot); // slot = 0, 1, or 2
    const Item *GetEquippedPotion(int slot) const;
    int GetEquippedPotionCount() const;

private:
    MaterialMap m_materials;

    // Gear/potion instances: instanceId → Item
    std::unordered_map<uint32_t, Item> m_items;

    // Equipped slots: one item per gear slot
    std::unordered_map<int, uint32_t> m_equipped; // slot enum → instanceId

    // Equipped potions for battle
    uint32_t m_equippedPotions[MAX_EQUIPPED_POTIONS] = {0, 0, 0};

    // Instance ID counter for crafted items
    static uint32_t s_nextInstanceId;
};