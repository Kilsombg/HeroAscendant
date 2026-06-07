#pragma once

#include "../events/EventBus.h"
#include "../events/GameEvents.h"
#include "../renderer/Renderer.h"
#include "../core/AssetManager.h"

#include <vector>
#include <string>

// ==============================================================================
// DamageNumber — one active floating number
//
// Spawned when ShowDamageNumber event fires.
// Floats upward and fades out over its lifetime.
//
// ==============================================================================
struct DamageNumber
{
    float x, y;        // Current position (virtual coords)
    std::string text;  // "15" or "30!" for crits
    float lifetime;    // How long it lives (seconds)
    float elapsed;     // Time since spawn
    bool isCritical;   // Crits are larger and yellow
    bool isHeroDamage; // Hero damage numbers are red, enemy damage white

    // Alpha 0.0-1.0 — fades out in the last 40% of lifetime
    float GetAlpha() const
    {
        float remaining = 1.0f - (elapsed / lifetime);
        return remaining < 0.4f ? remaining / 0.4f : 1.0f;
    }

    bool IsDead() const { return elapsed >= lifetime; }
};

// ==============================================================================
// DamageNumberSystem — Observer Pattern (Behavioral)
//
// Subscribes to ShowDamageNumber events and maintains a list of active
// floating numbers. Updates their position and alpha each frame.
// Draws them during the render pass.
//
// Lifetime:
//   BattleState creates one DamageNumberSystem in OnEnter().
//   It subscribes to ShowDamageNumber in Init().
//   It unsubscribes and is destroyed in OnExit().
//
// Why a separate system and not drawn inside BattleState::Render()?
//   Damage numbers are a visual effect with their own state (position,
//   timer, alpha). Keeping them in their own system means BattleState
//   doesn't need to manage a list of visual effects — it just fires events.
//
// ==============================================================================
class DamageNumberSystem
{
public:
    DamageNumberSystem(EventBus &eventBus, AssetManager &assets);
    ~DamageNumberSystem();

    // Init — subscribes to ShowDamageNumber events
    void Init();

    // Shutdown — unsubscribes
    void Shutdown();

    // Update — advances all active numbers (position, timer)
    void Update(float deltaTime);

    // Render — draws all active numbers
    void Render(Renderer &renderer);

private:
    EventBus &m_eventBus;
    AssetManager &m_assets;

    std::vector<DamageNumber> m_numbers;

    ListenerID m_damageNumberListenerId = 0;

    // Configuration
    static constexpr float FLOAT_SPEED = 120.0f; // Pixels per second upward
    static constexpr float LIFETIME = 1.2f;      // Seconds before disappearing
    static constexpr float CRIT_SCALE = 1.4f;    // Crits drawn 40% larger

    void OnShowDamageNumber(const ShowDamageNumberEvent &data);
};