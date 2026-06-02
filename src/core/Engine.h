#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <string>

// Forward declarations — tells the compiler these classes exist
// without including their full headers here. Keeps compile times fast.
// We only need the full headers in Engine.cpp where we actually use them.
class Renderer;
class AudioManager;

// ==============================================================================
// Engine
//
// Wraps ALL SDL2 initialization and shutdown. The rest of the game never
// calls SDL_Init, SDL_Quit, or creates windows/renderers directly.
//
// Cross-platform note:
//   SDL2 on Android doesn't use a normal main(). SDL2 provides its own
//   Java entry point that eventually calls your main(). The Engine class
//   stays identical — SDL2 handles the platform difference internally.
//
// ==============================================================================
class Engine
{
public:
    // Virtual resolution — the coordinate system your game is designed for.
    // All gameplay positions use these values regardless of real screen size.
    // Portrait orientation for a mobile RPG.
    //
    // Cross-platform note:
    //   On Android, the real screen might be 1440x3200 or 720x1560.
    //   SDL_RenderSetLogicalSize() maps your virtual resolution to whatever
    //   the real screen is. You never think about real pixels in game code.
    static constexpr int VIRTUAL_WIDTH = 1080;
    static constexpr int VIRTUAL_HEIGHT = 1920;

    // Target frames per second. SDL_RENDERER_PRESENTVSYNC handles this
    // automatically when the monitor runs at 60hz, but we store it for
    // manual delta time calculations.
    static constexpr int TARGET_FPS = 60;

    Engine();
    ~Engine();

    // Disable copy — Engine owns SDL resources, copying makes no sense.
    // "= delete" tells the compiler to error if anyone tries to copy it.
    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;

    // Init sets up SDL2, creates the window, creates the renderer.
    // Returns false if anything fails — caller checks and exits cleanly.
    // title   = window title bar text
    // width   = real window width in pixels (not virtual)
    // height  = real window height in pixels (not virtual)
    bool Init(const std::string &title, int width, int height);

    // Shutdown destroys all SDL2 resources in the correct reverse order.
    // Called once when the game exits.
    void Shutdown();

    // Accessors — other systems need the raw SDL pointers occasionally
    // (e.g. Renderer needs SDL_Renderer* to draw, AudioManager needs init flag)
    SDL_Window *GetWindow() const { return m_window; }
    SDL_Renderer *GetSDLRenderer() const { return m_sdlRenderer; }

    bool IsInitialized() const { return m_initialized; }

private:
    SDL_Window *m_window = nullptr;
    SDL_Renderer *m_sdlRenderer = nullptr;
    bool m_initialized = false;

    // Private helpers — break Init() into readable steps
    bool InitSDL();
    bool InitWindow(const std::string &title, int width, int height);
    bool InitRenderer();
    bool InitSDLImage();
    bool InitSDLMixer();
    bool InitSDLTTF();
};