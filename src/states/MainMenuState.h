#pragma once

#include "IGameState.h"
#include "../events/GameEvents.h"
#include "../events/EventBus.h"

// ==============================================================================
// MainMenuState
//
// The first state the game starts in.
// Handles: character display, navigation to Battle/Dungeon/Crafting/Quests
//
// ==============================================================================
class MainMenuState : public IGameState
{
public:
    explicit MainMenuState(StateContext ctx);

    void OnEnter() override;
    void OnExit() override;
    void OnResume() override;

    void HandleInput(const SDL_Event &event) override;
    void Update(float deltaTime) override;
    void Render() override;

    // MainMenu is the bottom of the stack — nothing renders below it
    bool IsTransparent() const override { return false; }

private:
    StateContext m_ctx;

    // Notification text shown after returning from battle
    std::string m_notification;
    float m_notificationTimer = 0.0f;
    static constexpr float NOTIFICATION_DURATION = 2.5f;

    // Coin listener — keeps displayed count up to date without polling
    ListenerID m_coinListenerId = 0;

    // Cached hero stats for display (refreshed in OnEnter/OnResume)
    int m_displayLevel = 1;
    int m_displayCoins = 0;
    int m_displayHP = 200;
    int m_displayMaxHP = 200;
    int m_displayAtk = 0;
    int m_displayDef = 0;

    void RefreshHeroStats();

    // Hit testing — returns true if the tap lands inside the rect.
    // Takes virtual coords so desktop mouse and Android finger both work.
    //
    // Cross-platform: finger coords arrive as 0.0-1.0 floats normalized
    // to screen size. We convert them to virtual coords (1080x1920) before
    // calling HitTest, so one function handles both input types.
    static bool HitTest(int tapX, int tapY,
                        int rectX, int rectY, int rectW, int rectH);

    // Extracts virtual tap position from either mouse or finger event.
    // Returns false if the event is not a tap type.
    static bool GetTapPosition(const SDL_Event &event, int &outX, int &outY);

    void HandleTap(int tapX, int tapY);
};