#include "Engine.h"

#include <iostream>

Engine::Engine() = default;
Engine::~Engine() = default;

// ==============================================================================
// Init — calls each subsystem in the required order.
// If any step fails we return false immediately. The caller (main.cpp or
// Game.cpp) is responsible for not proceeding if Init returns false.
//
// ==============================================================================
bool Engine::Init(const std::string &title, int width, int height)
{
    if (!InitSDL())
        return false;
    if (!InitWindow(title, width, height))
        return false;
    if (!InitRenderer())
        return false;
    if (!InitSDLImage())
        return false;
    if (!InitSDLMixer())
        return false;
    if (!InitSDLTTF())
        return false;

    m_initialized = true;
    return true;
}

// ==============================================================================
// Shutdown — destroys resources in REVERSE order of creation.
// This is required by SDL2 — destroying the window before the renderer
// causes undefined behavior.
//
// ==============================================================================
void Engine::Shutdown()
{
    // TTF, Mixer, Image each have their own quit function.
    // These must be called before SDL_Quit.
    TTF_Quit();

    // Mix_CloseAudio closes the audio device opened in InitSDLMixer.
    // Mix_Quit unloads the format libraries (mp3, ogg etc.)
    Mix_CloseAudio();
    Mix_Quit();

    // IMG_Quit unloads the image format libraries (png, jpg etc.)
    IMG_Quit();

    // Renderer must be destroyed before Window.
    if (m_sdlRenderer)
    {
        SDL_DestroyRenderer(m_sdlRenderer);
        m_sdlRenderer = nullptr;
    }

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    // SDL_Quit shuts down all SDL subsystems initialized by SDL_Init.
    SDL_Quit();

    m_initialized = false;
}

// ==============================================================================
// Private helpers
// ==============================================================================

bool Engine::InitSDL()
{
    // SDL_Init takes a bitmask of subsystems to initialize.
    // SDL_INIT_VIDEO  — window creation, rendering, input events
    // SDL_INIT_AUDIO  — audio device access
    // SDL_INIT_TIMER  — SDL_GetTicks(), SDL_Delay() timing functions
    //
    // Note: SDL_INIT_GAMECONTROLLER implicitly includes SDL_INIT_JOYSTICK.
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        // SDL_GetError() returns a thread-local string describing the last error.
        // Always call it immediately after a failure before any other SDL call.
        std::cerr << "[Engine] SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    // SDL_HINT_RENDER_SCALE_QUALITY controls texture filtering.
    // "0" = nearest-neighbor — required for pixel art.
    //       Pixels stay sharp when scaled up.
    // "1" = linear filtering — blurs pixels, looks bad for pixel art.
    // "2" = anisotropic — even smoother, even worse for pixel art.
    //
    // Must be set BEFORE creating the renderer.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    return true;
}

bool Engine::InitWindow(const std::string &title, int width, int height)
{
    // SDL_CreateWindow parameters:
    // title                  — text in OS title bar
    // SDL_WINDOWPOS_CENTERED — center on screen (x and y)
    // width, height          — initial window size in real pixels
    //
    // SDL_WINDOW_SHOWN      — make visible immediately (not minimized)
    // SDL_WINDOW_RESIZABLE  — allow user to resize; we handle scaling via
    //                         SDL_RenderSetLogicalSize so game content
    //                         always fills the window correctly.
    //
    // Cross-platform note:
    //   On Android this call creates a fullscreen surface matching the
    //   device screen. The width/height parameters are ignored on Android.
    //   SDL2 uses the actual device resolution instead.
    m_window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!m_window)
    {
        std::cerr << "[Engine] SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }

    return true;
}

bool Engine::InitRenderer()
{
    // SDL_CreateRenderer parameters:
    // window   — the window to render into
    // -1       — driver index; -1 means auto-select best available
    //
    // SDL_RENDERER_ACCELERATED  — use GPU hardware rendering.
    //                             Falls back to software if unavailable.
    // SDL_RENDERER_PRESENTVSYNC — lock to monitor refresh rate (usually 60hz).
    //                             Prevents screen tearing. Means SDL_RenderPresent
    //                             blocks until the next frame boundary.
    m_sdlRenderer = SDL_CreateRenderer(
        m_window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!m_sdlRenderer)
    {
        std::cerr << "[Engine] SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        return false;
    }

    // SDL_RenderSetLogicalSize is the key to resolution independence.
    //
    // We tell SDL2: "my game is designed for VIRTUAL_WIDTH x VIRTUAL_HEIGHT".
    // SDL2 then automatically scales and letterboxes/pillarboxes all rendering
    // to fit any real window size.
    //
    // Example: game designed at 1080x1920, running on a 1440x2960 Android phone.
    // SDL2 scales everything up proportionally and adds black bars if needed.
    // Your game code always draws at 1080x1920 coordinates.
    //
    // Cross-platform note:
    //   This single call makes your rendering resolution-independent on all
    //   platforms. A 16x16 sprite drawn at (540, 960) appears in the center
    //   of the screen on every device.
    SDL_RenderSetLogicalSize(m_sdlRenderer, Engine::VIRTUAL_WIDTH, Engine::VIRTUAL_HEIGHT);

    return true;
}

bool Engine::InitSDLImage()
{
    // IMG_Init loads format support libraries (libpng, libjpeg etc.)
    // IMG_INIT_PNG — we need PNG for pixel art sprites (lossless + transparency)
    // IMG_INIT_JPG — useful for background images
    //
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    int initialized = IMG_Init(imgFlags);

    if (!(initialized & IMG_INIT_PNG))
    {
        std::cerr << "[Engine] IMG_Init failed to initialize PNG support: "
                  << IMG_GetError() << "\n";
        return false;
    }

    // JPEG is optional — warn but continue
    if (!(initialized & IMG_INIT_JPG))
    {
        std::cerr << "[Engine] Warning: JPEG support unavailable: "
                  << IMG_GetError() << "\n";
    }

    return true;
}

bool Engine::InitSDLMixer()
{
    // Mix_OpenAudio parameters:
    // MIX_DEFAULT_FREQUENCY — 44100 hz, standard CD quality audio
    // MIX_DEFAULT_FORMAT    — signed 16-bit, native byte order
    // 2                     — stereo channels (1 = mono)
    // 2048                  — chunk size in bytes; smaller = less latency
    //                         but more CPU. 2048 is good for games.
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) != 0)
    {
        std::cerr << "[Engine] Mix_OpenAudio failed: " << Mix_GetError() << "\n";
        return false;
    }

    // Mix_Init loads format decoders.
    // MIX_INIT_OGG — OGG Vorbis, best format for game music (good quality, small files)
    // MIX_INIT_MP3 — MP3 support
    int mixFlags = MIX_INIT_OGG | MIX_INIT_MP3;
    if ((Mix_Init(mixFlags) & mixFlags) != mixFlags)
    {
        // Non-fatal — game still works without audio formats, just log a warning
        std::cerr << "[Engine] Mix_Init warning: " << Mix_GetError() << "\n";
    }

    return true;
}

bool Engine::InitSDLTTF()
{
    // TTF_Init has no parameters — just initializes the FreeType font library.
    // Returns -1 on failure.
    if (TTF_Init() != 0)
    {
        std::cerr << "[Engine] TTF_Init failed: " << TTF_GetError() << "\n";
        return false;
    }

    return true;
}