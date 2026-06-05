#pragma once

#include "IComponent.h"

// ==============================================================================
// AIComponent
//
// Controls enemy movement and attack timing.
//
//   AIComponent holds an AIBehaviour enum. CombatSystem reads
//   this and selects the correct ICombatStrategy for the enemy type.
//   The actual damage calculation lives in the strategy, not here.
//   This component just stores state — am I moving? am I in attack range?
//
//
// States:
//   Waiting    → hero not in range, idle animation plays
//   Attacking  → hero arrived in range, attack timer counting
//   Stunned    → briefly unable to act
//
// ==============================================================================

enum class AIBehaviour
{
    Stationary, // Stands still, attacks when hero arrives — all current enemies
    // Future: Ranged, ChargingBoss
};

enum class AIState
{
    Waiting,   // Hero not yet in range
    Attacking, // Hero in range — attack timer counting
    Stunned,   // Briefly disabled
};

class AIComponent : public IComponent
{
public:
    AIComponent(AIBehaviour behaviour,
                float attackRange,
                float attackInterval)
        : m_behaviour(behaviour), m_attackRange(attackRange), m_attackInterval(attackInterval)
    {
    }

    // -----------------------------------------------------------------------
    // Per-frame — advances attack timer
    //
    // -----------------------------------------------------------------------
    void Update(float deltaTime) override
    {
        if (m_state == AIState::Attacking)
        {
            m_attackTimer += deltaTime;
        }
        if (m_state == AIState::Stunned)
        {
            m_stunTimer -= deltaTime;
            if (m_stunTimer <= 0.0f)
            {
                m_stunTimer = 0.0f;
                m_state = AIState::Attacking;
            }
        }
    }

    // -----------------------------------------------------------------------
    // State machine
    // -----------------------------------------------------------------------

    void SetState(AIState s) { m_state = s; }
    AIState GetState() const { return m_state; }
    bool IsWaiting() const { return m_state == AIState::Waiting; }
    bool IsAttacking() const { return m_state == AIState::Attacking; }
    bool IsStunned() const { return m_state == AIState::Stunned; }

    // -----------------------------------------------------------------------
    // Attack readiness
    // -----------------------------------------------------------------------

    bool IsAttackReady() const
    {
        return m_state == AIState::Attacking &&
               m_attackTimer >= m_attackInterval;
    }

    void ResetAttackTimer() { m_attackTimer = 0.0f; }

    void ApplyStun(float duration)
    {
        m_state = AIState::Stunned;
        m_stunTimer = duration;
    }

    // -----------------------------------------------------------------------
    // Getters — read by CombatSystem and movement logic in BattleState
    // -----------------------------------------------------------------------

    AIBehaviour GetBehaviour() const { return m_behaviour; }
    float GetAttackRange() const { return m_attackRange; }
    float GetAttackInterval() const { return m_attackInterval; }

private:
    AIBehaviour m_behaviour;
    AIState m_state = AIState::Waiting;

    float m_attackRange;    // Distance in pixels — hero must be this close
    float m_attackInterval; // Seconds between enemy attacks
    float m_attackTimer = 0.0f;
    float m_stunTimer = 0.0f;
};