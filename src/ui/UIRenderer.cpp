#include "UIRenderer.h"

#include <string>

UIRenderer::UIRenderer(Renderer &renderer, AssetManager &assets)
    : m_renderer(renderer), m_assets(assets)
{
}

// ==============================================================================
// Health bars
// ==============================================================================

void UIRenderer::DrawHealthBar(int x, int y, int w, int h,
                               int current, int max,
                               const std::string &label,
                               bool isHero)
{
    float fillPct = (max > 0) ? static_cast<float>(current) / max : 0.0f;
    if (fillPct < 0.0f)
        fillPct = 0.0f;
    if (fillPct > 1.0f)
        fillPct = 1.0f;

    // Choose bar color:
    //   Hero:  green → yellow → red as HP drops
    //   Enemy: always red
    Color fillColor;
    if (isHero)
    {
        if (fillPct > 0.6f)
            fillColor = Color{50, 200, 50, 255}; // Green
        else if (fillPct > 0.3f)
            fillColor = Color{220, 180, 20, 255}; // Yellow
        else
            fillColor = Color{220, 50, 50, 255}; // Red
    }
    else
    {
        fillColor = Color{200, 50, 50, 255}; // Enemy always red
    }

    DrawBar(x, y, w, h, fillPct, fillColor);

    // HP text drawn INSIDE the bar — always on screen regardless of y position.
    // Vertically centers ~14px text within the bar height.
    int textY = y + h / 2 - 7;
    m_renderer.DrawText(GetFont(), label, x + 6, textY, Color::White);

    std::string hpText = std::to_string(current) + " / " + std::to_string(max);
    int hpTextX = x + w - static_cast<int>(hpText.size()) * 9 - 6;
    m_renderer.DrawText(GetFont(), hpText, hpTextX, textY, Color::White);
}

// ==============================================================================
// Buttons
// ==============================================================================

void UIRenderer::DrawButton(int x, int y, int w, int h,
                            const std::string &label,
                            Color bgColor,
                            Color labelColor)
{
    // Background
    m_renderer.DrawRect(x, y, w, h, bgColor, true);

    // Border — 3px slightly lighter
    Color border = Color{
        static_cast<Uint8>(std::min(255, bgColor.r + 40)),
        static_cast<Uint8>(std::min(255, bgColor.g + 40)),
        static_cast<Uint8>(std::min(255, bgColor.b + 40)),
        255};
    m_renderer.DrawRect(x, y, w, h, border, false);

    // Centered label — approximate centering, font metrics not available
    // without TTF_SizeText. Position is offset from center.
    int textX = x + w / 2 - static_cast<int>(label.size()) * 8;
    int textY = y + h / 2 - 14;
    m_renderer.DrawText(GetFont(), label, textX, textY, labelColor);
}

void UIRenderer::DrawAttackButton(bool highlighted)
{
    Color bg = highlighted
                   ? Color{180, 60, 60, 230}  // Red when in combat — tap to attack
                   : Color{80, 80, 100, 150}; // Grey when hero is still moving

    DrawButton(ATTACK_BTN_X, ATTACK_BTN_Y,
               ATTACK_BTN_W, ATTACK_BTN_H,
               "ATTACK", bg);

    // Extra glow border when highlighted
    if (highlighted)
    {
        m_renderer.DrawRect(ATTACK_BTN_X - 3, ATTACK_BTN_Y - 3,
                            ATTACK_BTN_W + 6, ATTACK_BTN_H + 6,
                            Color{255, 100, 100, 180}, false);
    }
}

void UIRenderer::DrawPauseButton()
{
    DrawButton(PAUSE_BTN_X, PAUSE_BTN_Y,
               PAUSE_BTN_W, PAUSE_BTN_H,
               "II", // Pause symbol
               Color{50, 50, 80, 200});
}

// ==============================================================================
// Labels and text
// ==============================================================================

void UIRenderer::DrawLabel(const std::string &text,
                           int centerX, int y,
                           Color color,
                           bool large)
{
    // Approximate text width: ~14px per character for small, ~22px for large
    int charWidth = large ? 22 : 14;
    int textX = centerX - static_cast<int>(text.size()) * charWidth / 2;
    m_renderer.DrawText(GetFont(large), text, textX, y, color);
}

void UIRenderer::DrawFloorIndicator(int currentFloor, int totalFloors)
{
    // "Floor 2 / 4" centered at top
    std::string text = "Floor " + std::to_string(currentFloor + 1) + " / " + std::to_string(totalFloors);
    DrawLabel(text, SCREEN_W / 2, 45, Color::White, false);
}

void UIRenderer::DrawCoinCount(int coins)
{
    // Coin icon placeholder + count — bottom center
    std::string text = "Coins: " + std::to_string(coins);
    DrawLabel(text, SCREEN_W / 2, SCREEN_H - 80, Color{255, 215, 0, 255});
}

// ==============================================================================
// Overlays
// ==============================================================================

void UIRenderer::DrawPauseOverlay()
{
    // Semi-transparent dark backdrop
    m_renderer.DrawRect(0, 0, SCREEN_W, SCREEN_H,
                        Color{0, 0, 0, 160}, true);

    // Title
    DrawLabel("PAUSED", SCREEN_W / 2, 700, Color::White, true);

    // Buttons — centered
    int btnW = 400, btnH = 100;
    int btnX = (SCREEN_W - btnW) / 2;

    DrawButton(btnX, 900, btnW, btnH, "RESUME",
               Color{50, 120, 50, 230});
    DrawButton(btnX, 1040, btnW, btnH, "QUIT TO MENU",
               Color{120, 50, 50, 230});
}

void UIRenderer::DrawDeathOverlay()
{
    // Dark red backdrop
    m_renderer.DrawRect(0, 0, SCREEN_W, SCREEN_H,
                        Color{80, 0, 0, 180}, true);

    DrawLabel("YOU DIED", SCREEN_W / 2, 700,
              Color{220, 50, 50, 255}, true);

    int btnW = 400, btnH = 100;
    int btnX = (SCREEN_W - btnW) / 2;

    DrawButton(btnX, 920, btnW, btnH, "RESURRECT",
               Color{180, 140, 20, 230}); // Gold
    DrawButton(btnX, 1060, btnW, btnH, "QUIT TO MENU",
               Color{80, 80, 80, 230});
}

void UIRenderer::DrawVictoryOverlay(int coinsEarned)
{
    // Gold tint backdrop
    m_renderer.DrawRect(0, 0, SCREEN_W, SCREEN_H,
                        Color{80, 60, 0, 160}, true);

    DrawLabel("STAGE CLEAR!", SCREEN_W / 2, 650,
              Color{255, 215, 0, 255}, true);

    std::string reward = "+" + std::to_string(coinsEarned) + " coins";
    DrawLabel(reward, SCREEN_W / 2, 800,
              Color{255, 215, 0, 255}, false);

    int btnW = 400, btnH = 100;
    int btnX = (SCREEN_W - btnW) / 2;

    DrawButton(btnX, 1000, btnW, btnH, "CONTINUE",
               Color{50, 120, 50, 230});
}

// ==============================================================================
// Potion slots
// ==============================================================================

void UIRenderer::DrawPotionSlots(int slotCount, int maxSlots)
{
    // Bottom-left — square slots
    int slotSize = 90;
    int startX = 20;
    int startY = SCREEN_H - slotSize - 20;
    int gap = 10;

    for (int i = 0; i < maxSlots; ++i)
    {
        int slotX = startX + i * (slotSize + gap);
        bool hasPotion = (i < slotCount);

        Color bg = hasPotion
                       ? Color{60, 120, 60, 220} // Green — has potion
                       : Color{40, 40, 40, 180}; // Dark — empty slot

        m_renderer.DrawRect(slotX, startY, slotSize, slotSize, bg, true);
        m_renderer.DrawRect(slotX, startY, slotSize, slotSize,
                            Color{100, 100, 100, 255}, false); // Border

        if (hasPotion)
        {
            // "P" placeholder — replaced with potion sprite when assets are ready
            m_renderer.DrawText(GetFont(), "P",
                                slotX + slotSize / 2 - 7,
                                startY + slotSize / 2 - 14,
                                Color::White);
        }
    }
}

// ==============================================================================
// Stage progress
// ==============================================================================

void UIRenderer::DrawStageProgress(int currentFloor, int totalFloors)
{
    // Thin bar just below the HP bars
    int barX = 20, barY = 100;
    int segW = (SCREEN_W - 40) / totalFloors;
    int segH = 12;
    int gap = 4;

    for (int i = 0; i < totalFloors; ++i)
    {
        int x = barX + i * (segW + gap);

        bool isBoss = (i == totalFloors - 1);
        bool isCleared = (i < currentFloor);
        bool isCurrent = (i == currentFloor);

        Color c;
        if (isBoss && isCleared)
            c = Color{180, 50, 50, 255}; // Boss cleared
        else if (isBoss)
            c = Color{120, 20, 20, 200}; // Boss ahead
        else if (isCleared)
            c = Color{50, 180, 50, 255}; // Floor cleared
        else if (isCurrent)
            c = Color{50, 120, 200, 255}; // Current floor
        else
            c = Color{60, 60, 60, 200}; // Upcoming

        m_renderer.DrawRect(x, barY, segW, segH, c, true);
    }
}

// ==============================================================================
// Private helpers
// ==============================================================================

void UIRenderer::DrawBar(int x, int y, int w, int h,
                         float fillPct,
                         Color fillColor,
                         Color bgColor)
{
    // Background
    m_renderer.DrawRect(x, y, w, h, bgColor, true);

    // Fill — width proportional to fillPct
    int fillW = static_cast<int>(w * fillPct);
    if (fillW > 0)
        m_renderer.DrawRect(x, y, fillW, h, fillColor, true);

    // Border
    m_renderer.DrawRect(x, y, w, h, Color{150, 150, 150, 200}, false);
}

TTF_Font *UIRenderer::GetFont(bool large) const
{
    // Fonts must be loaded in AssetManager before UIRenderer is used.
    // Convention:
    //   "ui_small" = small font, ~20pt, used for labels and numbers
    //   "ui_large" = large font, ~36pt, used for titles
    //
    // If the font isn't loaded yet, GetFont returns nullptr and DrawText
    // is a no-op — so nothing crashes, just no text drawn.
    return large
               ? m_assets.GetFont("ui_large")
               : m_assets.GetFont("ui_small");
}