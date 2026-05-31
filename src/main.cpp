// main.cpp
// Entry point. On Windows SDL2 redirects WinMain to this automatically
// via SDL2::SDL2main in CMake. On Android SDL2 calls this from Java.
// Your code stays identical on both platforms.

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>

int main(int argc, char *argv[])
{
    // SDL_Init initializes SDL subsystems as a bitmask.
    // SDL_INIT_VIDEO = window + renderer
    // SDL_INIT_AUDIO = sound system
    // SDL_INIT_TIMER = needed for SDL_Delay and timing
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
    {
        // SDL_GetError() returns a human-readable error string
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return 1;
    }

    // Set nearest-neighbor scaling BEFORE creating renderer.
    // Critical for pixel art — prevents blurring when scaling sprites.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    // SDL_CreateWindow params:
    // title, x pos, y pos, width, height, flags
    // SDL_WINDOW_SHOWN      = visible immediately
    // SDL_WINDOW_RESIZABLE  = user can resize (we'll handle scaling)
    SDL_Window *window = SDL_CreateWindow(
        "Hero Ascendant",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1080, 1920,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // SDL_CreateRenderer params: window, driver index (-1 = auto), flags
    // SDL_RENDERER_ACCELERATED = use GPU
    // SDL_RENDERER_PRESENTVSYNC = cap to monitor refresh rate (prevents tearing)
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop
    bool running = true;
    SDL_Event event;

    while (running)
    {
        // SDL_PollEvent fills `event` with the next event in the queue.
        // Returns 0 when the queue is empty.
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_ESCAPE)
                running = false;
        }

        // Clear screen to dark blue
        SDL_SetRenderDrawColor(renderer, 10, 10, 40, 255);
        SDL_RenderClear(renderer);

        // SDL_RenderPresent swaps the back buffer to screen (double buffering)
        SDL_RenderPresent(renderer);
    }

    // Always destroy in reverse order of creation
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}