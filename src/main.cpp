// main.cpp — Entry point.
//
// This file is intentionally minimal. All initialization logic lives in
// Game::Run() so it's testable and not trapped in main().
//
// Cross-platform note:
//   On Windows, SDL2 redirects WinMain → main automatically via
//   SDL2::SDL2main (linked in CMakeLists.txt). Your signature stays
//   int main(int argc, char* argv[]) on all platforms.
//
//   On Android, SDL2's Java layer calls your main() after the activity
//   is created. Again, identical signature, SDL2 handles the difference.
//
//   Important: you MUST have int argc, char* argv[] in the signature
//   even if unused. SDL2 requires this exact signature.

#include "core/Game.h"

int main(int argc, char *argv[])
{
    (void)argc; // not used but required by SDL2
    (void)argv;

    Game game;
    return game.Run();
}