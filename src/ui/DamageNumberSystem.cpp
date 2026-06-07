#include "DamageNumberSystem.h"

#include <algorithm>
#include <cstdint>

DamageNumberSystem::DamageNumberSystem(EventBus &eventBus, AssetManager &assets)
    : m_eventBus(eventBus), m_assets(assets)
{
}

DamageNumberSystem::~DamageNumberSystem()
{
    Shutdown();
}

void DamageNumberSystem::Init()
{
    m_damageNumberListenerId = m_eventBus.Subscribe(
        EventType::ShowDamageNumber,
        [this](const Event &e)
        {
            OnShowDamageNumber(std::get<ShowDamageNumberEvent>(e.data));
        });
}

void DamageNumberSystem::Shutdown()
{
    m_eventBus.Unsubscribe(EventType::ShowDamageNumber,
                           m_damageNumberListenerId);
    m_numbers.clear();
}

// ==============================================================================
// Update
// ==============================================================================

void DamageNumberSystem::Update(float deltaTime)
{
    // Advance each number — float upward, count elapsed time
    for (auto &num : m_numbers)
    {
        num.y -= FLOAT_SPEED * deltaTime; // Move upward (y decreases)
        num.elapsed += deltaTime;
    }

    // Remove dead numbers — erase-remove idiom
    m_numbers.erase(
        std::remove_if(m_numbers.begin(), m_numbers.end(),
                       [](const DamageNumber &n)
                       { return n.IsDead(); }),
        m_numbers.end());
}

// ==============================================================================
// Render
// ==============================================================================

void DamageNumberSystem::Render(Renderer &renderer)
{
    TTF_Font *fontSmall = m_assets.GetFont("ui_small");
    TTF_Font *fontLarge = m_assets.GetFont("ui_large");

    if (!fontSmall)
        return; // Font not loaded yet — skip silently

    for (const auto &num : m_numbers)
    {
        // Alpha fades from 255 to 0 in the last portion of lifetime
        Uint8 alpha = static_cast<Uint8>(num.GetAlpha() * 255.0f);

        // Color:
        //   Critical hits  → yellow
        //   Hero damage    → red
        //   Normal enemy hit → white
        Color color;
        if (num.isCritical)
            color = Color{255, 215, 0, alpha}; // Gold
        else if (num.isHeroDamage)
            color = Color{220, 50, 50, alpha}; // Red
        else
            color = Color{255, 255, 255, alpha}; // White

        // Crits use the large font
        TTF_Font *font = (num.isCritical && fontLarge) ? fontLarge : fontSmall;

        renderer.DrawText(font, num.text,
                          static_cast<int>(num.x),
                          static_cast<int>(num.y),
                          color);
    }
}

// ==============================================================================
// Event handler
// ==============================================================================

void DamageNumberSystem::OnShowDamageNumber(const ShowDamageNumberEvent &data)
{
    DamageNumber num;
    num.x = data.worldX;
    num.y = data.worldY;
    num.elapsed = 0.0f;
    num.lifetime = LIFETIME;
    num.isCritical = data.isCritical;
    num.isHeroDamage = data.isHeroTakingDamage;

    // Format the number text
    num.text = std::to_string(data.damage);
    if (data.isCritical)
        num.text += "!"; // Crit indicator

    // Slight horizontal spread so overlapping numbers are readable
    // Alternate left/right based on list size
    float spread = (m_numbers.size() % 2 == 0) ? -20.0f : 20.0f;
    num.x += spread;

    m_numbers.push_back(num);
}