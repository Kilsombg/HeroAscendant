#include "CraftingSystem.h"
#include "../core/RNG.h"

#include <cstdlib>
#include <iostream>

CraftingSystem::CraftingSystem(EventBus &eventBus)
    : m_eventBus(eventBus)
{
}

// ==============================================================================
// Recipe registration
// ==============================================================================

void CraftingSystem::InitRecipes()
{
    m_recipes.clear();
    RegisterWeaponRecipes();
    RegisterArmorRecipes();
    RegisterAccessoryRecipes();
    RegisterPotionRecipes();

    std::cout << "[CraftingSystem] " << m_recipes.size()
              << " recipes registered.\n";
}

void CraftingSystem::RegisterWeaponRecipes()
{
    // Iron Sword — basic weapon, available from floor 1
    {
        Recipe r;
        r.id = 2001;
        r.name = "Iron Sword";
        r.requirements = {
            {MAT_IRON_ORE, "Iron Ore", 3},
            {MAT_BONE, "Bone", 1}};
        r.output = ItemBuilder("Iron Sword", ItemSlot::Weapon)
                       .SetRarity(ItemRarity::Common)
                       .SetAttackBonus(20)
                       .SetDescription("A reliable sword forged from iron ore.")
                       .Build();
        m_recipes.push_back(r);
    }

    // Dragon Blade — late game weapon, requires dragon scales
    {
        Recipe r;
        r.id = 2002;
        r.name = "Dragon Blade";
        r.requirements = {
            {MAT_IRON_ORE, "Iron Ore", 5},
            {MAT_DRAGON_SCALE, "Dragon Scale", 3},
            {MAT_MAGIC_CRYSTAL, "Magic Crystal", 1}};
        r.output = ItemBuilder("Dragon Blade", ItemSlot::Weapon)
                       .SetRarity(ItemRarity::Epic)
                       .SetAttackBonus(60)
                       .SetDescription("A blade infused with dragon energy.")
                       .Build();
        m_recipes.push_back(r);
    }
}

void CraftingSystem::RegisterArmorRecipes()
{
    // Leather Armor — early armor
    {
        Recipe r;
        r.id = 2101;
        r.name = "Leather Armor";
        r.requirements = {
            {MAT_LEATHER, "Leather", 4},
            {MAT_BONE, "Bone", 2}};
        r.output = ItemBuilder("Leather Armor", ItemSlot::Armor)
                       .SetRarity(ItemRarity::Common)
                       .SetDefenseBonus(15)
                       .SetHPBonus(20)
                       .SetDescription("Light armor made from toughened leather.")
                       .Build();
        m_recipes.push_back(r);
    }

    // Dragon Scale Armor — best armor in the game
    {
        Recipe r;
        r.id = 2102;
        r.name = "Dragon Scale Armor";
        r.requirements = {
            {MAT_DRAGON_SCALE, "Dragon Scale", 5},
            {MAT_MAGIC_CRYSTAL, "Magic Crystal", 2},
            {MAT_LEATHER, "Leather", 3}};
        r.output = ItemBuilder("Dragon Scale Armor", ItemSlot::Armor)
                       .SetRarity(ItemRarity::Legendary)
                       .SetDefenseBonus(50)
                       .SetHPBonus(100)
                       .SetDescription("Armor forged from the scales of a fallen dragon.")
                       .Build();
        m_recipes.push_back(r);
    }
}

void CraftingSystem::RegisterAccessoryRecipes()
{
    // Bone Amulet — small HP boost
    {
        Recipe r;
        r.id = 2201;
        r.name = "Bone Amulet";
        r.requirements = {
            {MAT_BONE, "Bone", 5}};
        r.output = ItemBuilder("Bone Amulet", ItemSlot::Accessory)
                       .SetRarity(ItemRarity::Common)
                       .SetHPBonus(30)
                       .SetDescription("An amulet carved from bone.")
                       .Build();
        m_recipes.push_back(r);
    }

    // Crystal Ring — attack boost accessory
    {
        Recipe r;
        r.id = 2202;
        r.name = "Crystal Ring";
        r.requirements = {
            {MAT_MAGIC_CRYSTAL, "Magic Crystal", 3},
            {MAT_IRON_ORE, "Iron Ore", 2}};
        r.output = ItemBuilder("Crystal Ring", ItemSlot::Accessory)
                       .SetRarity(ItemRarity::Rare)
                       .SetAttackBonus(15)
                       .SetDescription("A ring set with a glowing magic crystal.")
                       .Build();
        m_recipes.push_back(r);
    }
}

void CraftingSystem::RegisterPotionRecipes()
{
    // Health Potion — restores 50 HP
    {
        Recipe r;
        r.id = 2301;
        r.name = "Health Potion";
        r.requirements = {
            {MAT_HERB, "Herb", 3}};
        r.output = ItemBuilder("Health Potion", ItemSlot::Potion)
                       .SetRarity(ItemRarity::Common)
                       .SetPotionHeal(50)
                       .SetDescription("Restores 50 HP when used.")
                       .Build();
        m_recipes.push_back(r);
    }

    // Greater Health Potion — restores 150 HP
    {
        Recipe r;
        r.id = 2302;
        r.name = "Greater Health Potion";
        r.requirements = {
            {MAT_HERB, "Herb", 5},
            {MAT_MAGIC_CRYSTAL, "Magic Crystal", 1}};
        r.output = ItemBuilder("Greater Health Potion", ItemSlot::Potion)
                       .SetRarity(ItemRarity::Rare)
                       .SetPotionHeal(150)
                       .SetDescription("Restores 150 HP when used.")
                       .Build();
        m_recipes.push_back(r);
    }

    // Attack Potion — boosts attack by 50% for 30 seconds
    {
        Recipe r;
        r.id = 2303;
        r.name = "Attack Potion";
        r.requirements = {
            {MAT_HERB, "Herb", 2},
            {MAT_DRAGON_SCALE, "Dragon Scale", 1}};
        r.output = ItemBuilder("Attack Potion", ItemSlot::Potion)
                       .SetRarity(ItemRarity::Rare)
                       .SetPotionAttackBoost(1.5f, 30.0f)
                       .SetDescription("Boosts attack damage by 50% for 30 seconds.")
                       .Build();
        m_recipes.push_back(r);
    }
}

// ==============================================================================
// Queries
// ==============================================================================

std::vector<const Recipe *> CraftingSystem::GetRecipesBySlot(ItemSlot slot) const
{
    std::vector<const Recipe *> result;
    for (const auto &recipe : m_recipes)
    {
        if (recipe.output.slot == slot)
            result.push_back(&recipe);
    }
    return result;
}

bool CraftingSystem::CanCraft(uint32_t recipeId, const Inventory &inventory) const
{
    const Recipe *recipe = FindRecipe(recipeId);
    if (!recipe)
        return false;

    for (const auto &req : recipe->requirements)
    {
        if (!inventory.HasMaterial(req.materialId, req.quantity))
            return false;
    }
    return true;
}

// ==============================================================================
// Crafting
// ==============================================================================

bool CraftingSystem::Craft(uint32_t recipeId, Inventory &inventory)
{
    if (!CanCraft(recipeId, inventory))
    {
        std::cout << "[CraftingSystem] Cannot craft recipe " << recipeId
                  << " — missing materials.\n";
        return false;
    }

    const Recipe *recipe = FindRecipe(recipeId);

    // Consume all required materials
    for (const auto &req : recipe->requirements)
    {
        inventory.RemoveMaterial(req.materialId, req.quantity);
    }

    // Build the output item.
    // Re-use ItemBuilder to apply a rarity roll on top of the base output.
    // This means each craft of the same recipe can produce slightly different
    // quality results — same as most mobile RPGs.
    ItemRarity rolledRarity = RollRarity();

    Item output = ItemBuilder(recipe->output.name, recipe->output.slot)
                      .SetRarity(rolledRarity)
                      .SetDescription(recipe->output.description)
                      .SetAttackBonus(recipe->output.attackBonus)
                      .SetDefenseBonus(recipe->output.defenseBonus)
                      .SetHPBonus(recipe->output.hpBonus)
                      .SetPotionHeal(recipe->output.potionHealAmount)
                      .Build();

    // Add to inventory
    inventory.AddItem(output);

    // Determine rarity string for the event
    std::string rarityStr = "common";
    switch (rolledRarity)
    {
    case ItemRarity::Rare:
        rarityStr = "rare";
        break;
    case ItemRarity::Epic:
        rarityStr = "epic";
        break;
    case ItemRarity::Legendary:
        rarityStr = "legendary";
        break;
    default:
        break;
    }

    m_eventBus.Fire(EventType::ItemCrafted, ItemCraftedEvent{
                                                output.id,   // itemId
                                                output.name, // itemName
                                                rarityStr    // rarity
                                            });

    std::cout << "[CraftingSystem] Crafted: " << output.name
              << " (" << rarityStr << ")\n";

    return true;
}

// ==============================================================================
// Private
// ==============================================================================

const Recipe *CraftingSystem::FindRecipe(uint32_t id) const
{
    for (const auto &recipe : m_recipes)
    {
        if (recipe.id == id)
            return &recipe;
    }
    return nullptr;
}

ItemRarity CraftingSystem::RollRarity() const
{
    // Roll 0.0 to 1.0
    float roll = RNG::Get().NextFloat();

    if (roll < 0.03f)
        return ItemRarity::Legendary; // 3%
    if (roll < 0.15f)
        return ItemRarity::Epic; // 12%
    if (roll < 0.40f)
        return ItemRarity::Rare; // 25%
    return ItemRarity::Common;   // 60%
}