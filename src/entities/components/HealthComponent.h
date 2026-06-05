#pragma once

#include "IComponent.h"
#include "../../events/EventBus.h"
#include "../../events/GameEvents.h"

// ==============================================================================
// HealthComponent
//
// Tracks current and maximum HP. Fires events through EventBus when
// damage is taken or the entity dies.
//
// Death flow:
//   TakeDamage(amount)
//     → reduces m_currentHP
//     → fires HeroTookDamage or EnemyDied depending on entity type
//     → calls entity.MarkDead() so EntityManager removes it next frame
//
// ==============================================================================
class HealthComponent : public IComponent
{
public:
    HealthComponent(int maxHP, EventBus &eventBus)
        : m_maxHP(maxHP), m_currentHP(maxHP), m_eventBus(eventBus)
    {
    }

    // -----------------------------------------------------------------------
    // Damage and healing
    // -----------------------------------------------------------------------

    // TakeDamage — reduces HP and fires the appropriate event.
    // isCritical is passed through from CombatSystem for UI effects.
    void TakeDamage(int amount, bool isCritical = false);

    // Heal — restores HP up to max. Used by potions.
    void Heal(int amount);

    // FullHeal — used on resurrection
    void FullHeal() { m_currentHP = m_maxHP; }

    // -----------------------------------------------------------------------
    // Queries
    // -----------------------------------------------------------------------

    int GetCurrentHP() const { return m_currentHP; }
    int GetMaxHP() const { return m_maxHP; }
    bool IsDead() const { return m_currentHP <= 0; }

    // GetHPPercent — returns 0.0 to 1.0, used to draw health bars
    float GetHPPercent() const
    {
        if (m_maxHP == 0)
            return 0.0f;
        return static_cast<float>(m_currentHP) / static_cast<float>(m_maxHP);
    }

    // -----------------------------------------------------------------------
    // Max HP upgrades — used by StatsComponent when hero levels up
    // -----------------------------------------------------------------------
    void SetMaxHP(int newMax, bool healToFull = false)
    {
        m_maxHP = newMax;
        if (healToFull)
            m_currentHP = m_maxHP;
        // Clamp current HP to new max
        if (m_currentHP > m_maxHP)
            m_currentHP = m_maxHP;
    }

    // IsHero flag — determines which event to fire on damage/death
    // Set by EntityManager when creating the hero entity
    void SetIsHero(bool isHero) { m_isHero = isHero; }

private:
    int m_maxHP;
    int m_currentHP;
    bool m_isHero = false; // true = fire HeroTookDamage, false = EnemyDied
    EventBus &m_eventBus;
};