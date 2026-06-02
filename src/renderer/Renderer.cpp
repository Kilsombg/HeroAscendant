#include "Renderer.h"

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

void Renderer::Init(SDL_Renderer *sdlRenderer)
{
    m_sdlRenderer = sdlRenderer;
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
    SDL_Rect srcRect = {srcX, srcY, srcW, srcH};
    SDL_Rect dstRect = {dstX, dstY, dstW, dstH};

    SDL_RendererFlip flip = flipH ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    SDL_RenderCopyEx(m_sdlRenderer, texture, &srcRect, &dstRect,
                     angle, nullptr, flip);
}

void Renderer::DrawRect(int x, int y, int w, int h, Color color, bool filled)
{
    SDL_SetRenderDrawColor(m_sdlRenderer, color.r, color.g, color.b, color.a);

    SDL_Rect rect = {x, y, w, h};

    if (filled)
    {
        // SDL_RenderFillRect fills the rectangle with the draw color.
        SDL_RenderFillRect(m_sdlRenderer, &rect);
    }
    else
    {
        // SDL_RenderDrawRect draws only the outline (4 lines).
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

    // TTF_RenderText_Blended renders text to a new SDL_Surface.
    // "Blended" = high quality anti-aliased rendering with alpha blending.
    // Other modes:
    //   TTF_RenderText_Solid   — fastest, no anti-aliasing, aliased edges
    //   TTF_RenderText_Shaded  — anti-aliased on a solid background box
    //   TTF_RenderText_Blended — best quality, slightly slower
    //
    // For pixel art UI you might prefer Solid (crisp, no blur).
    // For readable body text, Blended looks better.
    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), sdlColor);

    if (!surface)
    {
        std::cerr << "[Renderer] TTF_RenderText_Blended failed: " << TTF_GetError() << "\n";
        return;
    }

    // SDL_CreateTextureFromSurface converts the CPU surface to a GPU texture.
    // After this the surface is no longer needed.
    SDL_Texture *texture = SDL_CreateTextureFromSurface(m_sdlRenderer, surface);

    // SDL_FreeSurface frees the CPU memory. Always do this after creating the texture.
    SDL_FreeSurface(surface);

    if (!texture)
    {
        std::cerr << "[Renderer] SDL_CreateTextureFromSurface failed: " << SDL_GetError() << "\n";
        return;
    }

    // Query the texture size so we draw it at the right size.
    int tw, th;
    SDL_QueryTexture(texture, nullptr, nullptr, &tw, &th);

    SDL_Rect dstRect = {x, y, tw, th};
    SDL_RenderCopy(m_sdlRenderer, texture, nullptr, &dstRect);

    // Destroy the temporary texture — it was created just for this draw call.
    // In practice you'd cache font textures in AssetManager for performance,
    // but for now this is correct and simple.
    SDL_DestroyTexture(texture);
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