#include "HealthComponent.h"
#include "../Entity.h"
#include "TransformComponent.h"

#include <algorithm>

void HealthComponent::TakeDamage(int amount, bool isCritical)
{
    if (m_currentHP <= 0)
        return; // Already dead — ignore further damage

    m_currentHP -= amount;
    if (m_currentHP < 0)
        m_currentHP = 0;

    if (m_isHero)
    {
        // Fire HeroTookDamage — UISystem listens to update health bar
        // CombatSystem listens to check if death state should be pushed
        m_eventBus.Fire(EventType::HeroTookDamage, HeroTookDamageEvent{
                                                       amount,      // damage
                                                       m_currentHP, // remainingHP
                                                       m_maxHP,     // maxHP
                                                       isCritical   // wasCritical
                                                   });

        if (m_currentHP <= 0 && m_owner)
        {
            // Use FireDeferred so HeroTookDamage finishes delivering
            // to all listeners before HeroDied starts being processed.
            // Without this, a HeroDied listener could push DeathState
            // while HeroTookDamage listeners are still iterating.
            m_eventBus.FireDeferred(EventType::HeroDied, HeroDiedEvent{
                                                             m_floorIndex, // floorIndex
                                                             m_stageIndex  // stageIndex
                                                         });
            m_owner->MarkDead();
        }
    }
    else
    {
        // For enemies: BattleState and QuestSystem both listen to EnemyDied.
        // We don't fire per-hit events for enemies — just death.
        // CombatSystem handles showing damage numbers via ShowDamageNumber event.
        if (m_currentHP <= 0 && m_owner)
        {
            // Get position from TransformComponent for loot/effect spawning
            float posX = 0.0f, posY = 0.0f;

            // We include TransformComponent here by getting it from the owner.
            // This is the one case where a component looks up a sibling component.
            // It's acceptable because HealthComponent has no meaning without
            // a position (where do we spawn the death effect/loot?)
            auto *transform = m_owner->GetComponent<class TransformComponent>();
            if (transform)
            {
                posX = transform->GetCenterX();
                posY = transform->GetCenterY();
            }

            m_eventBus.FireDeferred(EventType::EnemyDied, EnemyDiedEvent{
                                                              m_owner->GetId(), // enemyId
                                                              0,                // coinsDropped — set by CombatSystem/LootSystem
                                                              m_floorIndex,     // floorIndex
                                                              posX,             // posX
                                                              posY              // posY
                                                          });
            m_owner->MarkDead();
        }
    }
}

void HealthComponent::Heal(int amount)
{
    m_currentHP = std::min(m_currentHP + amount, m_maxHP);
}