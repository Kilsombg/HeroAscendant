#pragma once

#include "IComponent.h"

// ==============================================================================
// CombatComponent
//
// Holds all combat-related stats for an entity.
// Both Hero and enemies use this component — they share the same stats
// structure, just different values.
//
// Auto-attack:
//   The component tracks its own auto-attack cooldown timer.
//   Update() counts down each frame. IsAutoAttackReady() returns true
//   when the cooldown has elapsed. CombatSystem calls this each frame
//   and performs the attack when ready.
//
// Critical hits:
//   critChance = 0.0 to 1.0 (e.g. 0.1 = 10% chance)
//   On a crit, damage is multiplied by critMultiplier (default 2.0x)
//   CombatSystem rolls the crit, not this component.
//   This component just stores the stats.
//
// ==============================================================================
class CombatComponent : public IComponent
{
public:
    // attackDamage      = base damage per hit
    // autoAttackSpeed   = attacks per second (1.0 = one attack per second)
    // critChance        = 0.0 to 1.0
    CombatComponent(int attackDamage, float autoAttackSpeed, float critChance)
        : m_attackDamage(attackDamage), m_autoAttackSpeed(autoAttackSpeed), m_critChance(critChance), m_autoAttackTimer(0.0f)
    {
    }

    // -----------------------------------------------------------------------
    // Per-frame — counts down the auto-attack cooldown
    //
    // -----------------------------------------------------------------------
    void Update(float deltaTime) override
    {
        // Count up — when timer reaches interval, auto-attack is ready
        m_autoAttackTimer += deltaTime;
    }

    // -----------------------------------------------------------------------
    // Auto-attack
    // -----------------------------------------------------------------------

    // IsAutoAttackReady — true when enough time has passed since last attack
    bool IsAutoAttackReady() const
    {
        // autoAttackSpeed is attacks per second
        // interval = 1.0 / speed (e.g. speed=2.0 → interval=0.5s)
        return m_autoAttackTimer >= (1.0f / m_autoAttackSpeed);
    }

    // ResetAutoAttackTimer — call after performing an auto-attack
    void ResetAutoAttackTimer() { m_autoAttackTimer = 0.0f; }

    // -----------------------------------------------------------------------
    // Stat getters
    // -----------------------------------------------------------------------

    int GetAttackDamage() const { return m_attackDamage; }
    float GetAutoAttackSpeed() const { return m_autoAttackSpeed; }
    float GetCritChance() const { return m_critChance; }
    float GetCritMultiplier() const { return m_critMultiplier; }

    // -----------------------------------------------------------------------
    // Stat setters — called by StatsComponent when hero upgrades stats
    // -----------------------------------------------------------------------

    void SetAttackDamage(int damage) { m_attackDamage = damage; }
    void SetAutoAttackSpeed(float speed) { m_autoAttackSpeed = speed; }
    void SetCritChance(float chance) { m_critChance = chance; }
    void SetCritMultiplier(float mult) { m_critMultiplier = mult; }

    // IncreaseAttackDamage — used by stat upgrade system (coins → strength)
    void IncreaseAttackDamage(int amount) { m_attackDamage += amount; }

private:
    int m_attackDamage;
    float m_autoAttackSpeed;       // Attacks per second
    float m_critChance;            // 0.0 to 1.0
    float m_critMultiplier = 2.0f; // Crit damage multiplier
    float m_autoAttackTimer;       // Counts up each frame
};