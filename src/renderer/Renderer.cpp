#include "Renderer.h"

#include "../core/AssetManager.h"

#include <SDL2/SDL_ttf.h>
#include <iostream>

// ==============================================================================
// Color constants
// ==============================================================================
const Color Color::White = {255, 255, 255, 255};
const Color Color::Black = {0, 0, 0, 255};
const Color Color::Red = {255, 0, 0, 255};
const Color Color::Green = {0, 255, 0, 255};
const Color Color::Blue = {0, 0, 255, 255};
const Color Color::Yellow = {255, 255, 0, 255};
const Color Color::Transparent = {0, 0, 0, 0};

// ==============================================================================
Renderer::Renderer() = default;
Renderer::~Renderer() = default;

void Renderer::Init(SDL_Renderer *sdlRenderer, AssetManager *assets)
{
    m_sdlRenderer = sdlRenderer;
    m_assets = assets;
}

// ==============================================================================
// Frame lifecycle
// ==============================================================================

void Renderer::Clear(Color color)
{
    // SDL_SetRenderDrawColor sets the color used by SDL_RenderClear and
    // SDL_RenderDrawRect/Line/Point. Parameters are R, G, B, A (0-255).
    SDL_SetRenderDrawColor(m_sdlRenderer, color.r, color.g, color.b, color.a);

    // SDL_RenderClear fills the entire render target with the draw color.
    // This erases whatever was drawn last frame — required every frame.
    SDL_RenderClear(m_sdlRenderer);
}

void Renderer::Present()
{
    // SDL_RenderPresent copies the back buffer to the screen.
    // SDL2 uses double buffering: you draw to a hidden back buffer,
    // then Present() swaps it to the visible front buffer atomically.
    // This prevents flickering — the player never sees a half-drawn frame.
    //
    // With SDL_RENDERER_PRESENTVSYNC this also blocks until the next
    // monitor refresh (~16.6ms at 60hz), capping your frame rate naturally.
    SDL_RenderPresent(m_sdlRenderer);
}

// ==============================================================================
// Drawing
// ==============================================================================

void Renderer::DrawTexture(SDL_Texture *texture,
                           int x, int y, int w, int h,
                           double angle,
                           bool flipH)
{
    if (!texture)
        return;

    // SDL_Rect is a simple struct: { int x, y, w, h }
    // dstRect tells SDL2 WHERE and HOW BIG to draw the texture on screen.
    SDL_Rect dstRect = {x, y, w, h};

    // SDL_RenderCopyEx is the full-featured texture draw function.
    // Parameters:
    //   renderer  — our SDL_Renderer
    //   texture   — the texture to draw
    //   nullptr   — source rect (nullptr = whole texture)
    //   &dstRect  — destination rect on screen
    //   angle     — rotation in degrees, clockwise
    //   nullptr   — rotation center point (nullptr = center of dstRect)
    //   flip      — SDL_FLIP_NONE, SDL_FLIP_HORIZONTAL, SDL_FLIP_VERTICAL
    //
    // SDL_RendererFlip is a flags enum. We convert our bool to the right flag.
    SDL_RendererFlip flip = flipH ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    SDL_RenderCopyEx(m_sdlRenderer, texture, nullptr, &dstRect,
                     angle, nullptr, flip);
}

void Renderer::DrawTextureRegion(SDL_Texture *texture,
                                 int srcX, int srcY, int srcW, int srcH,
                                 int dstX, int dstY, int dstW, int dstH,
                                 double angle,
                                 bool flipH)
{
    if (!texture)
        return;

    // srcRect defines WHICH PART of the texture to draw.
    // This is how sprite sheet animation works:
    //   Frame 0: srcRect = {0,   0, 32, 32}
    //   Frame 1: srcRect = {32,  0, 32, 32}
    //   Frame 2: srcRect = {64,  0, 32, 32}
    // Each call draws a different frame from the same texture.
    SDL_Rect srcRect = {srcX, srcY, srcW, srcH};
    SDL_Rect dstRect = {dstX, dstY, dstW, dstH};

    SDL_RendererFlip flip = flipH ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    SDL_RenderCopyEx(m_sdlRenderer, texture, &srcRect, &dstRect,
                     angle, nullptr, flip);
}

void Renderer::DrawRect(int x, int y, int w, int h, Color color, bool filled)
{
    // SDL_BLENDMODE_BLEND must be set for any alpha < 255 to actually blend.
    // Without this, SDL2 ignores the alpha channel and draws fully opaque.
    // SDL_BLENDMODE_NONE is faster when alpha=255, so we switch per call.
    if (color.a < 255)
        SDL_SetRenderDrawBlendMode(m_sdlRenderer, SDL_BLENDMODE_BLEND);
    else
        SDL_SetRenderDrawBlendMode(m_sdlRenderer, SDL_BLENDMODE_NONE);

    SDL_SetRenderDrawColor(m_sdlRenderer, color.r, color.g, color.b, color.a);

    SDL_Rect rect = {x, y, w, h};

    if (filled)
    {
        // SDL_RenderFillRect fills the rectangle with the draw color.
        // Used for: health bars, UI panels, debug overlays.
        SDL_RenderFillRect(m_sdlRenderer, &rect);
    }
    else
    {
        // SDL_RenderDrawRect draws only the outline (4 lines).
        // Used for: selection borders, debug bounding boxes.
        SDL_RenderDrawRect(m_sdlRenderer, &rect);
    }
}

void Renderer::DrawText(TTF_Font *font,
                        const std::string &text,
                        int x, int y,
                        Color color)
{
    if (!font || text.empty())
        return;

    SDL_Color sdlColor = ToSDLColor(color);
    SDL_Texture *texture = nullptr;
    bool owned = false; // whether we must destroy it after drawing

    if (m_assets)
    {
        // We need the font id to build the cache key.
        // Since we receive a raw TTF_Font*, we can't look it up directly.
        // Workaround: use the pointer address as a key component.
        // This works because the same font pointer = same font.
        std::string fontKey = "ptr:" + std::to_string(
                                           reinterpret_cast<uintptr_t>(font));
        texture = m_assets->GetOrCreateTextTexture(fontKey, text, sdlColor);
    }

    if (!texture)
    {
        // Fallback — old behaviour, creates and destroys per call
        SDL_Surface *surface = TTF_RenderText_Solid(font, text.c_str(), sdlColor);
        if (!surface)
            return;
        texture = SDL_CreateTextureFromSurface(m_sdlRenderer, surface);
        SDL_FreeSurface(surface);
        owned = true;
    }

    if (!texture)
        return;

    int tw, th;
    SDL_QueryTexture(texture, nullptr, nullptr, &tw, &th);
    SDL_Rect dst = {x, y, tw, th};
    SDL_RenderCopy(m_sdlRenderer, texture, nullptr, &dst);

    if (owned)
        SDL_DestroyTexture(texture);
}

SDL_Point Renderer::MeasureText(TTF_Font *font, const std::string &text) const
{
    if (!font || text.empty())
        return {0, 0};

    int w = 0, h = 0;

    // TTF_SizeText fills w and h with the pixel dimensions the rendered
    // surface would have. Returns 0 on success, -1 on error.
    // We ignore the return value — if it fails w/h stay 0, which is a
    // safe fallback (text just draws at position 0,0 instead of centred).
    TTF_SizeText(font, text.c_str(), &w, &h);

    return {w, h};
}

void Renderer::SetDrawAlpha(Uint8 alpha)
{
    // SDL_SetRenderDrawBlendMode tells SDL2 how to blend colors when drawing.
    // SDL_BLENDMODE_BLEND = standard alpha blending (src_alpha * src + (1 - src_alpha) * dst)
    // Required for alpha < 255 to be visible.
    SDL_SetRenderDrawBlendMode(m_sdlRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_sdlRenderer, 0, 0, 0, alpha);
}

// ==============================================================================
// Private
// ==============================================================================

SDL_Color Renderer::ToSDLColor(Color c) const
{
    return SDL_Color{c.r, c.g, c.b, c.a};
}