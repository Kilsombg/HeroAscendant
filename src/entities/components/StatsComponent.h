#pragma once

#include "IComponent.h"
#include "../../events/EventBus.h"
#include "../../events/GameEvents.h"

// ==============================================================================
// StatsComponent
//
// Tracks the hero's progression stats: level, coins, and upgradeable attributes.
//
// Stat upgrades:
//   The player spends coins to increase individual stats.
//   Each stat has a current level
//   and a cost that increases with each upgrade.
//
//   When a stat is upgraded:
//     1. Coins are deducted
//     2. The stat value increases
//     3. The linked component is updated
//     4. StatUpgraded and CoinSpent events are fired
//
// ==============================================================================
class StatsComponent : public IComponent
{
public:
    StatsComponent(EventBus &eventBus)
        : m_eventBus(eventBus)
    {
    }

    // -----------------------------------------------------------------------
    // Coins
    // -----------------------------------------------------------------------

    int GetCoins() const { return m_coins; }

    void AddCoins(int amount)
    {
        m_coins += amount;
        m_eventBus.Fire(EventType::CoinGained, CoinGainedEvent{
                                                   amount, // amount
                                                   m_coins // newTotal
                                               });
    }

    bool SpendCoins(int amount)
    {
        if (m_coins < amount)
            return false; // Not enough coins
        m_coins -= amount;
        m_eventBus.Fire(EventType::CoinSpent, CoinSpentEvent{
                                                  amount, // amount
                                                  m_coins // newTotal
                                              });
        return true;
    }

    bool CanAfford(int amount) const { return m_coins >= amount; }

    // -----------------------------------------------------------------------
    // Hero level and XP
    // -----------------------------------------------------------------------

    int GetLevel() const { return m_level; }
    int GetXP() const { return m_xp; }
    int GetXPToNextLevel() const { return m_xpToNextLevel; }

    void AddXP(int amount)
    {
        m_xp += amount;
        while (m_xp >= m_xpToNextLevel)
        {
            m_xp -= m_xpToNextLevel;
            LevelUp();
        }
    }

    // -----------------------------------------------------------------------
    // Upgradeable stats — each has a level and a coin cost
    // -----------------------------------------------------------------------

    // Strength — increases attack damage
    int GetStrengthLevel() const { return m_strengthLevel; }
    int GetStrengthUpgradeCost() const;
    bool UpgradeStrength(); // Returns false if can't afford

    // Defense — reduces incoming damage
    int GetDefenseLevel() const { return m_defenseLevel; }
    int GetDefenseValue() const { return m_defenseLevel * 2; } // 2 defense per level
    int GetDefenseUpgradeCost() const;
    bool UpgradeDefense();

    // Agility — increases auto-attack speed and dodge chance
    int GetAgilityLevel() const { return m_agilityLevel; }
    int GetAgilityUpgradeCost() const;
    bool UpgradeAgility();

    // MaxHP upgrade — directly increases hero's max HP
    int GetHPLevel() const { return m_hpLevel; }
    int GetHPUpgradeCost() const;
    bool UpgradeHP();

private:
    EventBus &m_eventBus;

    // Economy
    int m_coins = 0;

    // Hero level
    int m_level = 1;
    int m_xp = 0;
    int m_xpToNextLevel = 100; // Increases with each level

    // Stat upgrade levels — each starts at 0 (base value, no upgrades yet)
    int m_strengthLevel = 0;
    int m_defenseLevel = 0;
    int m_agilityLevel = 0;
    int m_hpLevel = 0;

    // Cost formula: baseCost * (currentLevel + 1)
    static constexpr int STRENGTH_BASE_COST = 50;
    static constexpr int DEFENSE_BASE_COST = 50;
    static constexpr int AGILITY_BASE_COST = 75;
    static constexpr int HP_BASE_COST = 60;

    void LevelUp();

    // Generic upgrade helper — reduces duplication across stat upgrades
    bool TryUpgradeStat(int &statLevel, int baseCost, const std::string &statName);
};