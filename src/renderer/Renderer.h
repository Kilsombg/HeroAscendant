#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>

// Forward declarations
class Engine;
class Texture;

// ==============================================================================
// Color — simple RGBA color value.
//
// ==============================================================================
struct Color
{
    Uint8 r = 255;
    Uint8 g = 255;
    Uint8 b = 255;
    Uint8 a = 255;

    // Common colors as static constants — readable in game code
    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Transparent;
};

// ==============================================================================
// Renderer
//
// All drawing goes through here. Game code never calls SDL_Render* directly.
//
// Cross-platform note:
//   SDL_RenderSetLogicalSize (set in Engine) means all coordinates passed to
//   this Renderer are in virtual resolution space (1080x1920). SDL2 scales
//   them to the real screen automatically. You never pass real pixel coords.
//
// ==============================================================================
class Renderer
{
public:
    Renderer();
    ~Renderer();

    // Takes the engine's SDL_Renderer pointer. Renderer doesn't own it —
    // Engine creates and destroys it. Renderer just uses it.
    void Init(SDL_Renderer *sdlRenderer);

    // ==== Frame lifecycle ====

    // Clear() — fills the screen with the current draw color.
    // Call at the START of each frame before drawing anything.
    void Clear(Color color = Color::Black);

    // Present() — swaps the back buffer to the screen.
    // Call at the END of each frame after drawing everything.
    // With PRESENTVSYNC this blocks until the monitor refreshes (~16ms at 60fps)
    void Present();

    // ==== Drawing ====

    // DrawTexture — draws a full texture at (x, y) with given width/height.
    // x, y   = top-left corner in virtual coordinates
    // w, h   = draw size (can differ from texture size for scaling)
    // angle  = rotation in degrees (0 = no rotation)
    // flipH  = mirror horizontally (useful for character facing direction)
    void DrawTexture(SDL_Texture *texture,
                     int x, int y, int w, int h,
                     double angle = 0.0,
                     bool flipH = false);

    // DrawTextureRegion — draws a sub-region of a texture (sprite sheet frame).
    // srcX, srcY, srcW, srcH = the rectangle within the texture to draw.
    // This is how sprite sheet animation works — each frame is a region.
    void DrawTextureRegion(SDL_Texture *texture,
                           int srcX, int srcY, int srcW, int srcH,
                           int dstX, int dstY, int dstW, int dstH,
                           double angle = 0.0,
                           bool flipH = false);

    // DrawRect — draws a filled or outlined rectangle.
    // Useful for UI elements, health bars, debug visualization.
    void DrawRect(int x, int y, int w, int h, Color color, bool filled = true);

    // DrawText — renders a string using a loaded TTF font.
    // Font must be loaded by AssetManager first.
    // x, y = top-left corner of the text
    void DrawText(TTF_Font *font,
                  const std::string &text,
                  int x, int y,
                  Color color);

    // ==== Utility ====

    // SetAlpha — sets global draw alpha for subsequent DrawTexture calls.
    // 0 = fully transparent, 255 = fully opaque.
    // Useful for fade in/out effects.
    void SetDrawAlpha(Uint8 alpha);

    // GetSDLRenderer — occasionally needed by systems that must call SDL directly.
    SDL_Renderer *GetSDLRenderer() const { return m_sdlRenderer; }

private:
    SDL_Renderer *m_sdlRenderer = nullptr;

    // Converts our Color struct to SDL_Color for SDL2 API calls
    SDL_Color ToSDLColor(Color c) const;
};