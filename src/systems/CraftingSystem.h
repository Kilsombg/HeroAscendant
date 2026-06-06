#pragma once

#include "Inventory.h"
#include "../events/EventBus.h"
#include "../events/GameEvents.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// ==============================================================================
// Recipe — defines what materials are needed and what item is produced
//
// Each recipe has a unique ID and a list of required material stacks.
// CraftingSystem checks the Inventory has all required materials before
// allowing crafting to proceed.
//
// ==============================================================================
struct MaterialRequirement
{
    uint32_t materialId;
    std::string materialName;
    int quantity;
};

struct Recipe
{
    uint32_t id;
    std::string name;
    std::vector<MaterialRequirement> requirements;
    Item output; // The item produced
};

// ==============================================================================
// ItemBuilder
//
// Constructs an Item step by step. Used by CraftingSystem to build
// the output item for a recipe, and can be used anywhere else an item
// needs to be created with optional properties.
//
// Usage:
//   Item sword = ItemBuilder("Iron Sword", ItemSlot::Weapon)
//       .SetRarity(ItemRarity::Rare)
//       .SetAttackBonus(25)
//       .SetDefenseBonus(0)
//       .SetDescription("A sword forged from iron ore.")
//       .Build();
//
// ==============================================================================
class ItemBuilder
{
public:
    ItemBuilder(const std::string &name, ItemSlot slot)
    {
        m_item.name = name;
        m_item.slot = slot;
    }

    ItemBuilder &SetRarity(ItemRarity rarity)
    {
        m_item.rarity = rarity;
        return *this;
    }

    ItemBuilder &SetDescription(const std::string &desc)
    {
        m_item.description = desc;
        return *this;
    }

    ItemBuilder &SetAttackBonus(int bonus)
    {
        m_item.attackBonus = bonus;
        return *this;
    }

    ItemBuilder &SetDefenseBonus(int bonus)
    {
        m_item.defenseBonus = bonus;
        return *this;
    }

    ItemBuilder &SetHPBonus(int bonus)
    {
        m_item.hpBonus = bonus;
        return *this;
    }

    ItemBuilder &SetPotionHeal(int amount)
    {
        m_item.potionHealAmount = amount;
        return *this;
    }

    ItemBuilder &SetPotionAttackBoost(float multiplier, float duration)
    {
        m_item.potionAttackBoost = multiplier;
        m_item.potionDuration = duration;
        return *this;
    }

    // Build — returns the finished Item.
    // After calling Build() this builder should not be reused.
    Item Build() const { return m_item; }

private:
    Item m_item;
};

// ==============================================================================
// CraftingSystem — Builder Pattern (Creational)
//
// Manages the recipe database and crafting process.
//
// Flow:
//   1. CraftingSystem is initialized with a set of recipes.
//   2. UI calls CanCraft(recipeId, inventory) to check availability.
//   3. Player confirms → UI calls Craft(recipeId, inventory).
//   4. CraftingSystem consumes materials, builds output item via ItemBuilder,
//      adds it to inventory, fires ItemCrafted event.
//
// ==============================================================================
class CraftingSystem
{
public:
    CraftingSystem(EventBus &eventBus);

    // InitRecipes — registers all crafting recipes.
    // Call once at game start or when entering MainMenuState.
    void InitRecipes();

    // -----------------------------------------------------------------------
    // Queries
    // -----------------------------------------------------------------------

    // GetAllRecipes — returns all known recipes (for UI display)
    const std::vector<Recipe> &GetAllRecipes() const { return m_recipes; }

    // GetRecipesBySlot — filter recipes by output item type
    std::vector<const Recipe *> GetRecipesBySlot(ItemSlot slot) const;

    // CanCraft — returns true if inventory has all required materials
    bool CanCraft(uint32_t recipeId, const Inventory &inventory) const;

    // -----------------------------------------------------------------------
    // Crafting
    // -----------------------------------------------------------------------

    // Craft — consumes materials and adds the crafted item to inventory.
    // Returns true on success, false if materials are missing.
    // Fires ItemCrafted event on success.
    bool Craft(uint32_t recipeId, Inventory &inventory);

private:
    EventBus &m_eventBus;
    std::vector<Recipe> m_recipes;

    // Helper — find recipe by ID
    const Recipe *FindRecipe(uint32_t id) const;

    // RollRarity — random rarity based on floor or recipe tier
    // Common=60%, Rare=25%, Epic=12%, Legendary=3%
    ItemRarity RollRarity() const;

    // Recipe registration helpers — each group defined separately for clarity

    void RegisterWeaponRecipes();
    void RegisterArmorRecipes();
    void RegisterAccessoryRecipes();
    void RegisterPotionRecipes();

    // Shared material IDs — defined as constants so recipe definitions
    // and enemy loot tables use the same IDs
public:
    // These are public so EnemyFactory and LootSystem can reference them
    static constexpr uint32_t MAT_IRON_ORE = 1001;
    static constexpr uint32_t MAT_BONE = 1002;
    static constexpr uint32_t MAT_DRAGON_SCALE = 1003;
    static constexpr uint32_t MAT_MAGIC_CRYSTAL = 1004;
    static constexpr uint32_t MAT_LEATHER = 1005;
    static constexpr uint32_t MAT_HERB = 1006;
};