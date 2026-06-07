// src/states/MainMenuState.cpp
#include "MainMenuState.h"
#include "GameStateManager.h"
#include "BattleState.h"

#include "../renderer/Renderer.h"
#include "../entities/components/StatsComponent.h"
#include "../entities/components/HealthComponent.h"
#include "../systems/Inventory.h"
#include "../ui/UIRenderer.h"

#include <iostream>
#include <algorithm>

static constexpr int BTN_W = 560;
static constexpr int BTN_H = 100;
static constexpr int BTN_X = (1920 - BTN_W) / 2; // 680 — centred on landscape width

// Buttons stacked in the right half of the screen,
// leaving the left half for the hero stat panel.
static constexpr int BTN_STAGE = 200;
static constexpr int BTN_DUNGEON = 320;
static constexpr int BTN_CRAFT = 440;
static constexpr int BTN_QUESTS = 560;

// Stat panel — left side of screen
static constexpr int PANEL_X = 40;
static constexpr int PANEL_Y = 80;
static constexpr int PANEL_W = 580;
static constexpr int PANEL_H = 900;

MainMenuState::MainMenuState(StateContext ctx)
    : m_ctx(ctx)
{
}

// ------------------------------------------------------------------------------
// Lifecycle
// ------------------------------------------------------------------------------

void MainMenuState::OnEnter()
{
    std::cout << "[MainMenuState] Entered.\n";
    RefreshHeroStats();

    // Observer pattern — subscribe to CoinGained so the displayed coin
    // count updates immediately when coins are awarded, without us needing
    // to poll the inventory every frame.
    //
    // Why ListenerID? We must unsubscribe in OnExit to avoid calling into
    // a destroyed object. ListenerID is the token EventBus gives us back.
    m_coinListenerId = m_ctx.eventBus.Subscribe(EventType::CoinGained,
                                                [this](const Event &e)
                                                {
                                                    m_displayCoins = std::get<CoinGainedEvent>(e.data).newTotal;
                                                });
}

void MainMenuState::OnExit()
{
    std::cout << "[MainMenuState] Exited.\n";
    m_ctx.eventBus.Unsubscribe(EventType::CoinGained, m_coinListenerId);
}

void MainMenuState::OnResume()
{
    std::cout << "[MainMenuState] Resumed (returned from sub-state).\n";
    // Refresh stats — player may have gained XP, coins, or new equipment
    // in the battle they just returned from.
    RefreshHeroStats();
    m_notification = "Returned to camp.";
    m_notificationTimer = NOTIFICATION_DURATION;
}

// ------------------------------------------------------------------------------
// Input
// ------------------------------------------------------------------------------

void MainMenuState::HandleInput(const SDL_Event &event)
{
    // Keyboard shortcut kept for fast desktop testing
    if (event.type == SDL_KEYDOWN &&
        event.key.keysym.sym == SDLK_RETURN)
    {
        m_ctx.stateManager.Replace(
            std::make_unique<BattleState>(m_ctx, 0));
        return;
    }

    int tapX = 0, tapY = 0;
    if (GetTapPosition(event, tapX, tapY))
        HandleTap(tapX, tapY);
}

// GetTapPosition — cross-platform tap extraction.
//
// Cross-platform note:
//   On Android the player uses their finger. SDL2 sends SDL_FINGERDOWN
//   with x/y as floats in [0, 1] relative to the window size.
//   On desktop the player uses a mouse. SDL2 sends SDL_MOUSEBUTTONDOWN
//   with x/y in real window pixels.
//
//   SDL_RenderSetLogicalSize (set in Engine::InitRenderer) maps real pixels
//   to virtual coords automatically for rendering, but NOT for input.
//   We must do the conversion for input ourselves — multiply the normalized
//   finger coord by the virtual resolution.
//
//   Result: every state calls GetTapPosition() once and gets virtual coords
//   regardless of device or input method. This is the correct pattern.
bool MainMenuState::GetTapPosition(const SDL_Event &event, int &outX, int &outY)
{
    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        outX = event.button.x;
        outY = event.button.y;
        return true;
    }
    if (event.type == SDL_FINGERDOWN)
    {
        outX = static_cast<int>(event.tfinger.x * 1920); // landscape virtual width
        outY = static_cast<int>(event.tfinger.y * 1080);
        return true;
    }
    return false;
}

bool MainMenuState::HitTest(int tapX, int tapY,
                            int rectX, int rectY, int rectW, int rectH)
{
    return tapX >= rectX && tapX <= rectX + rectW &&
           tapY >= rectY && tapY <= rectY + rectH;
}

void MainMenuState::HandleTap(int tapX, int tapY)
{
    if (HitTest(tapX, tapY, BTN_X, BTN_STAGE, BTN_W, BTN_H))
    {
        std::cout << "[MainMenuState] Enter Stage tapped.\n";
        m_ctx.stateManager.Replace(
            std::make_unique<BattleState>(m_ctx, 0));
    }
    else if (HitTest(tapX, tapY, BTN_X, BTN_DUNGEON, BTN_W, BTN_H))
    {
        std::cout << "[MainMenuState] Dungeon tapped — not yet implemented.\n";
        m_notification = "Dungeons coming soon!";
        m_notificationTimer = NOTIFICATION_DURATION;
    }
    else if (HitTest(tapX, tapY, BTN_X, BTN_CRAFT, BTN_W, BTN_H))
    {
        std::cout << "[MainMenuState] Crafting tapped — not yet implemented.\n";
        m_notification = "Crafting coming soon!";
        m_notificationTimer = NOTIFICATION_DURATION;
    }
    else if (HitTest(tapX, tapY, BTN_X, BTN_QUESTS, BTN_H, BTN_H))
    {
        std::cout << "[MainMenuState] Quests tapped — not yet implemented.\n";
        m_notification = "Quests coming soon!";
        m_notificationTimer = NOTIFICATION_DURATION;
    }
}

// ------------------------------------------------------------------------------
// Update
// ------------------------------------------------------------------------------

void MainMenuState::Update(float deltaTime)
{
    if (m_notificationTimer > 0.0f)
        m_notificationTimer -= deltaTime;
}

// ------------------------------------------------------------------------------
// Render
// ------------------------------------------------------------------------------

void MainMenuState::Render()
{
    m_ctx.renderer.Clear(Color{10, 8, 25, 255});

    // ---- Title ---- centred on the full 1920-wide canvas
    m_ctx.uiRenderer.DrawLabel("HERO ASCENDANT",
                               1920 / 2, 20,
                               Color{255, 200, 50, 255}, /*large=*/true);

    // ---- Hero stat panel ---- left side
    m_ctx.renderer.DrawRect(PANEL_X, PANEL_Y, PANEL_W, PANEL_H,
                            Color{20, 20, 50, 200}, true);
    m_ctx.renderer.DrawRect(PANEL_X, PANEL_Y, PANEL_W, PANEL_H,
                            Color{80, 80, 160, 180}, false);

    m_ctx.uiRenderer.DrawLabel(
        "Level " + std::to_string(m_displayLevel),
        PANEL_X + PANEL_W / 2, PANEL_Y + 30,
        Color{220, 220, 255, 255});

    m_ctx.uiRenderer.DrawLabel(
        "Coins: " + std::to_string(m_displayCoins),
        PANEL_X + PANEL_W / 2, PANEL_Y + 90,
        Color{255, 215, 0, 255});

    m_ctx.uiRenderer.DrawHealthBar(
        PANEL_X + 20, PANEL_Y + 160, PANEL_W - 40, 50,
        m_displayHP, m_displayMaxHP, "HP", true);

    m_ctx.uiRenderer.DrawLabel(
        "ATK  " + std::to_string(m_displayAtk),
        PANEL_X + PANEL_W / 4, PANEL_Y + 260,
        Color{255, 160, 80, 255});

    m_ctx.uiRenderer.DrawLabel(
        "DEF  " + std::to_string(m_displayDef),
        PANEL_X + 3 * PANEL_W / 4, PANEL_Y + 260,
        Color{80, 160, 255, 255});

    auto *weapon = m_ctx.inventory.GetEquippedItem(ItemSlot::Weapon);
    auto *armor = m_ctx.inventory.GetEquippedItem(ItemSlot::Armor);

    m_ctx.uiRenderer.DrawLabel(
        "Weapon: " + (weapon ? weapon->name : "None"),
        PANEL_X + PANEL_W / 2, PANEL_Y + 360,
        Color{200, 200, 200, 255});

    m_ctx.uiRenderer.DrawLabel(
        "Armor:  " + (armor ? armor->name : "None"),
        PANEL_X + PANEL_W / 2, PANEL_Y + 430,
        Color{200, 200, 200, 255});

    // ---- Navigation buttons ---- right side, vertically centred
    m_ctx.uiRenderer.DrawButton(BTN_X, BTN_STAGE, BTN_W, BTN_H,
                                "ENTER STAGE",
                                Color{60, 160, 60, 230});
    m_ctx.uiRenderer.DrawButton(BTN_X, BTN_DUNGEON, BTN_W, BTN_H,
                                "DUNGEON",
                                Color{60, 60, 160, 200});
    m_ctx.uiRenderer.DrawButton(BTN_X, BTN_CRAFT, BTN_W, BTN_H,
                                "CRAFTING",
                                Color{160, 100, 20, 200});
    m_ctx.uiRenderer.DrawButton(BTN_X, BTN_QUESTS, BTN_W, BTN_H,
                                "QUESTS",
                                Color{120, 40, 140, 200});

    // ---- Notification ----
    if (m_notificationTimer > 0.0f)
    {
        float alpha = std::min(1.0f, m_notificationTimer / 0.5f);
        Uint8 a = static_cast<Uint8>(alpha * 220.0f);
        m_ctx.uiRenderer.DrawLabel(m_notification,
                                   1920 / 2, 1020,
                                   Color{255, 255, 160, a});
    }
}

// ------------------------------------------------------------------------------
// Private helpers
// ------------------------------------------------------------------------------

void MainMenuState::RefreshHeroStats()
{
    // The hero entity only exists inside BattleState while a battle is running.
    // In MainMenuState we read stats from the persistent Inventory and from
    // a StatsComponent that would live on the hero entity if we had one.
    //
    // For now we derive display values from the Inventory bonuses.
    // When the full hero persistence system is implemented, this reads from
    // a saved StatsComponent snapshot instead.

    m_displayCoins = 0; // Will be updated by CoinGained events
    m_displayLevel = 1;
    m_displayHP = 200 + m_ctx.inventory.GetTotalHPBonus();
    m_displayMaxHP = m_displayHP;
    m_displayAtk = 15 + m_ctx.inventory.GetTotalAttackBonus();
    m_displayDef = 0 + m_ctx.inventory.GetTotalDefenseBonus();
}