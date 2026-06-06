#include "Inventory.h"

#include <algorithm>

uint32_t Inventory::s_nextInstanceId = 1;

// ==============================================================================
// Materials
// ==============================================================================

void Inventory::AddMaterial(uint32_t itemId, const std::string &name, int quantity)
{
    m_materials[itemId].name = name;
    m_materials[itemId].quantity += quantity;
}

bool Inventory::RemoveMaterial(uint32_t itemId, int quantity)
{
    auto it = m_materials.find(itemId);
    if (it == m_materials.end())
        return false;
    if (it->second.quantity < quantity)
        return false;

    it->second.quantity -= quantity;
    if (it->second.quantity == 0)
        m_materials.erase(it);

    return true;
}

int Inventory::GetMaterialCount(uint32_t itemId) const
{
    auto it = m_materials.find(itemId);
    return (it != m_materials.end()) ? it->second.quantity : 0;
}

bool Inventory::HasMaterial(uint32_t itemId, int quantity) const
{
    return GetMaterialCount(itemId) >= quantity;
}

// ==============================================================================
// Gear and potions
// ==============================================================================

void Inventory::AddItem(const Item &item)
{
    // Give this instance a unique ID
    Item copy = item;
    copy.id = s_nextInstanceId++;
    m_items[copy.id] = copy;
}

bool Inventory::RemoveItem(uint32_t instanceId)
{
    return m_items.erase(instanceId) > 0;
}

const Item *Inventory::GetItem(uint32_t instanceId) const
{
    auto it = m_items.find(instanceId);
    return (it != m_items.end()) ? &it->second : nullptr;
}

std::vector<const Item *> Inventory::GetItemsBySlot(ItemSlot slot) const
{
    std::vector<const Item *> result;
    for (auto &[id, item] : m_items)
    {
        if (item.slot == slot)
            result.push_back(&item);
    }
    return result;
}

// ==============================================================================
// Equipped gear
// ==============================================================================

uint32_t Inventory::EquipItem(ItemSlot slot, uint32_t instanceId)
{
    int key = static_cast<int>(slot);
    uint32_t previous = 0;

    auto it = m_equipped.find(key);
    if (it != m_equipped.end())
        previous = it->second;

    m_equipped[key] = instanceId;
    return previous;
}

uint32_t Inventory::UnequipItem(ItemSlot slot)
{
    int key = static_cast<int>(slot);
    auto it = m_equipped.find(key);
    if (it == m_equipped.end())
        return 0;

    uint32_t id = it->second;
    m_equipped.erase(it);
    return id;
}

const Item *Inventory::GetEquippedItem(ItemSlot slot) const
{
    auto it = m_equipped.find(static_cast<int>(slot));
    if (it == m_equipped.end())
        return nullptr;
    return GetItem(it->second);
}

int Inventory::GetTotalAttackBonus() const
{
    int total = 0;
    for (auto &[key, id] : m_equipped)
    {
        const Item *item = GetItem(id);
        if (item)
            total += item->attackBonus;
    }
    return total;
}

int Inventory::GetTotalDefenseBonus() const
{
    int total = 0;
    for (auto &[key, id] : m_equipped)
    {
        const Item *item = GetItem(id);
        if (item)
            total += item->defenseBonus;
    }
    return total;
}

int Inventory::GetTotalHPBonus() const
{
    int total = 0;
    for (auto &[key, id] : m_equipped)
    {
        const Item *item = GetItem(id);
        if (item)
            total += item->hpBonus;
    }
    return total;
}

// ==============================================================================
// Potions
// ==============================================================================

bool Inventory::EquipPotion(uint32_t instanceId)
{
    for (int i = 0; i < MAX_EQUIPPED_POTIONS; ++i)
    {
        if (m_equippedPotions[i] == 0)
        {
            m_equippedPotions[i] = instanceId;
            return true;
        }
    }
    return false; // All slots full
}

bool Inventory::UnequipPotion(int slot)
{
    if (slot < 0 || slot >= MAX_EQUIPPED_POTIONS)
        return false;
    m_equippedPotions[slot] = 0;
    return true;
}

const Item *Inventory::GetEquippedPotion(int slot) const
{
    if (slot < 0 || slot >= MAX_EQUIPPED_POTIONS)
        return nullptr;
    if (m_equippedPotions[slot] == 0)
        return nullptr;
    return GetItem(m_equippedPotions[slot]);
}

int Inventory::GetEquippedPotionCount() const
{
    int count = 0;
    for (int i = 0; i < MAX_EQUIPPED_POTIONS; ++i)
        if (m_equippedPotions[i] != 0)
            count++;
    return count;
}