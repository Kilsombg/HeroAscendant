#pragma once

#include "../renderer/Renderer.h"
#include "../core/AssetManager.h"

#include <string>

// ==============================================================================
// UIRenderer — Facade Pattern (Structural)
//
// Draws all HUD and menu elements. States call this instead of calling
// Renderer directly — UIRenderer handles the layout maths, colors, and
// SDL calls internally.
//
// Why a separate class from Renderer?
//   Renderer draws raw primitives (textures, rects, text).
//   UIRenderer draws game-specific widgets (HP bars, buttons, labels)
//   built from those primitives. Two levels of abstraction:
//
//   BattleState → UIRenderer::DrawHeroHP(currentHP, maxHP)
//               → Renderer::DrawRect (background bar)
//               → Renderer::DrawRect (filled portion)
//               → Renderer::DrawText (HP numbers)
//
// Virtual resolution:
//   All coordinates are in virtual 1080x1920 space.
//   SDL_RenderSetLogicalSize (set in Engine) scales to real screen.
//
// ==============================================================================
class UIRenderer
{
public:
    UIRenderer(Renderer &renderer, AssetManager &assets);

    // -----------------------------------------------------------------------
    // Health bars
    // -----------------------------------------------------------------------

    // DrawHealthBar — draws a labeled HP bar.
    // x, y = top-left corner of the bar
    // w, h = bar dimensions
    // current, max = HP values (used to calculate fill percentage)
    // label = text shown above or beside the bar (e.g. "HERO", "GOBLIN")
    // isHero = true = green bar, false = red bar (enemy)
    void DrawHealthBar(int x, int y, int w, int h,
                       int current, int max,
                       const std::string &label,
                       bool isHero = true);

    // DrawHeroHealthBar — draws the hero HP bar at its fixed HUD position.
    // BattleState calls this without knowing the coordinates.
    void DrawHeroHealthBar(int current, int max);

    // DrawEnemyHealthBar — draws the current enemy HP bar at its fixed HUD position.
    // name = the enemy's display name shown as the bar label.
    void DrawEnemyHealthBar(int current, int max, const std::string &name);

    // -----------------------------------------------------------------------
    // Buttons
    // -----------------------------------------------------------------------

    // DrawButton — draws a tappable button rectangle with centered label.
    // Returns true if the button was pressed this frame.
    // (For now just draws — input is still handled in state HandleInput.
    //  Full hit-test integration comes with the input refactor.)
    void DrawButton(int x, int y, int w, int h,
                    const std::string &label,
                    Color bgColor = Color{60, 60, 120, 220},
                    Color labelColor = Color::White);

    // DrawButton with hit-test — returns true if tapX/tapY falls inside the button.
    // States call this during input handling to remove hardcoded coordinate ranges.
    //
    // Usage:
    //   int tx, ty; GetTapPosition(event, tx, ty);
    //   if (m_ctx.uiRenderer.HitButton(tx, ty, btnX, btnY, btnW, btnH, "RESUME", bg))
    //       m_ctx.stateManager.Pop();
    bool HitButton(int tapX, int tapY,
                   int x, int y, int w, int h,
                   const std::string &label,
                   Color bgColor = Color{60, 60, 120, 220},
                   Color labelColor = Color::White);

    // HitTest — pure geometry check, no drawing. Use when you need to test
    // a region that isn't drawn by UIRenderer (e.g. custom sprite buttons).
    static bool HitTest(int tapX, int tapY, int x, int y, int w, int h);

    // DrawAttackButton — large bottom-right attack button
    // highlighted = true when hero is in combat range (pulses slightly)
    void DrawAttackButton(bool highlighted);

    // DrawPauseButton — small top-right corner button
    void DrawPauseButton();

    // -----------------------------------------------------------------------
    // Labels and text
    // -----------------------------------------------------------------------

    // DrawLabel — centered text at a position
    void DrawLabel(const std::string &text,
                   int centerX, int y,
                   Color color = Color::White,
                   bool large = false);

    // MeasureText — measures text width and height using the given font.
    // Wraps m_renderer.MeasureText so call sites stay one line.
    SDL_Point MeasureText(const std::string &text, bool large = false) const;

    // DrawFloorIndicator — "Floor 2 / 4" shown at top center in battle
    void DrawFloorIndicator(int currentFloor, int totalFloors);

    // DrawCoinCount — shows current coin total
    void DrawCoinCount(int coins);

    // -----------------------------------------------------------------------
    // Overlays — full-screen semi-transparent panels
    // -----------------------------------------------------------------------

    // DrawPauseOverlay — dark overlay with PAUSED title and buttons
    void DrawPauseOverlay();

    // DrawDeathOverlay — dark red overlay with death message and choices
    // Returns: 0=nothing, 1=Resurrect tapped, 2=Quit tapped
    // (Tapping detection uses last mouse/finger position — basic for now)
    void DrawDeathOverlay();

    // DrawVictoryOverlay — golden overlay with stage clear and reward
    void DrawVictoryOverlay(int coinsEarned);

    // -----------------------------------------------------------------------
    // Potion slots
    // -----------------------------------------------------------------------

    // DrawPotionSlots — draws up to 3 potion slot icons at bottom-left
    // slotCount = how many slots have a potion equipped
    void DrawPotionSlots(int slotCount, int maxSlots = 3);

    // -----------------------------------------------------------------------
    // Stage progress bar
    // -----------------------------------------------------------------------

    // DrawStageProgress — thin bar at top showing floor progress
    // e.g. 3 segments for 3 normal floors + 1 boss
    void DrawStageProgress(int currentFloor, int totalFloors);

private:
    Renderer &m_renderer;
    AssetManager &m_assets;

    // Virtual screen dimensions — everything relative to these
    static constexpr int SCREEN_W = 1920;
    static constexpr int SCREEN_H = 1080;

    // --- Hero HP bar --- top-left
    static constexpr int HERO_HP_BAR_X = 20;
    static constexpr int HERO_HP_BAR_Y = 20;
    static constexpr int HERO_HP_BAR_W = 500;
    static constexpr int HERO_HP_BAR_H = 50;

    // --- Enemy HP bar --- top-right, mirrored
    static constexpr int ENEMY_HP_BAR_X = SCREEN_W - 520; // 1400
    static constexpr int ENEMY_HP_BAR_Y = 20;
    static constexpr int ENEMY_HP_BAR_W = 500;
    static constexpr int ENEMY_HP_BAR_H = 50;

    // --- Attack button --- bottom-right
    static constexpr int ATTACK_BTN_X = SCREEN_W - 260; // 1660
    static constexpr int ATTACK_BTN_Y = SCREEN_H - 180; // 900
    static constexpr int ATTACK_BTN_W = 240;
    static constexpr int ATTACK_BTN_H = 160;

    // --- Pause button --- top-right corner
    static constexpr int PAUSE_BTN_X = SCREEN_W - 90; // 1830
    static constexpr int PAUSE_BTN_Y = 20;
    static constexpr int PAUSE_BTN_W = 70;
    static constexpr int PAUSE_BTN_H = 70;

    // Helper — draws a filled bar with a background
    void DrawBar(int x, int y, int w, int h,
                 float fillPct,
                 Color fillColor,
                 Color bgColor = Color{40, 40, 40, 200});

    // Helper — gets the font for UI text (loaded in AssetManager)
    TTF_Font *GetFont(bool large = false) const;
};