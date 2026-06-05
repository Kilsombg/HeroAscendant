#include "StatsComponent.h"
#include "../Entity.h"
#include "HealthComponent.h"
#include "CombatComponent.h"

// ==============================================================================
// Cost formula
// ==============================================================================

int StatsComponent::GetStrengthUpgradeCost() const
{
    return STRENGTH_BASE_COST * (m_strengthLevel + 1);
}

int StatsComponent::GetDefenseUpgradeCost() const
{
    return DEFENSE_BASE_COST * (m_defenseLevel + 1);
}

int StatsComponent::GetAgilityUpgradeCost() const
{
    return AGILITY_BASE_COST * (m_agilityLevel + 1);
}

int StatsComponent::GetHPUpgradeCost() const
{
    return HP_BASE_COST * (m_hpLevel + 1);
}

// ==============================================================================
// Stat upgrades
// ==============================================================================

bool StatsComponent::UpgradeStrength()
{
    if (!TryUpgradeStat(m_strengthLevel, STRENGTH_BASE_COST, "strength"))
        return false;

    // Apply the upgrade to CombatComponent
    // StatsComponent looks up its sibling component through the owner entity
    if (m_owner)
    {
        auto *combat = m_owner->GetComponent<CombatComponent>();
        if (combat)
        {
            // Each strength level adds 5 attack damage
            combat->IncreaseAttackDamage(5);
        }
    }
    return true;
}

bool StatsComponent::UpgradeDefense()
{
    return TryUpgradeStat(m_defenseLevel, DEFENSE_BASE_COST, "defense");
    // Defense value is calculated from level in GetDefenseValue()
    // CombatSystem reads GetDefenseValue() when calculating incoming damage
}

bool StatsComponent::UpgradeAgility()
{
    if (!TryUpgradeStat(m_agilityLevel, AGILITY_BASE_COST, "agility"))
        return false;

    // Each agility level increases auto-attack speed by 0.1 attacks/sec
    if (m_owner)
    {
        auto *combat = m_owner->GetComponent<CombatComponent>();
        if (combat)
        {
            combat->SetAutoAttackSpeed(combat->GetAutoAttackSpeed() + 0.1f);
        }
    }
    return true;
}

bool StatsComponent::UpgradeHP()
{
    if (!TryUpgradeStat(m_hpLevel, HP_BASE_COST, "hp"))
        return false;

    // Each HP level adds 20 max HP
    if (m_owner)
    {
        auto *health = m_owner->GetComponent<HealthComponent>();
        if (health)
        {
            health->SetMaxHP(health->GetMaxHP() + 20);
        }
    }
    return true;
}

// ==============================================================================
// Private helpers
// ==============================================================================

bool StatsComponent::TryUpgradeStat(int &statLevel, int baseCost,
                                    const std::string &statName)
{
    int cost = baseCost * (statLevel + 1);

    if (!SpendCoins(cost))
        return false; // SpendCoins fires CoinSpent event

    statLevel++;

    // Fire the stat upgraded event — UI listens to refresh stat panel
    m_eventBus.Fire(EventType::StatUpgraded, StatUpgradedEvent{
                                                 statName,  // statName
                                                 statLevel, // newValue
                                                 cost       // costPaid
                                             });

    return true;
}

void StatsComponent::LevelUp()
{
    int previousLevel = m_level;
    m_level++;

    // Each level increases XP required for the next one
    m_xpToNextLevel = static_cast<int>(m_xpToNextLevel * 1.5f);

    m_eventBus.Fire(EventType::HeroLeveledUp, HeroLeveledUpEvent{
                                                  m_level,      // newLevel
                                                  previousLevel // previousLevel
                                              });
}