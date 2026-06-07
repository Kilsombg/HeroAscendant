#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <string>
#include <unordered_map>

// ==============================================================================
// AssetManager
//
// Loads and caches all game assets. The same texture, sound, or font is
// never loaded more than once regardless of how many entities use it.
//
// Ownership:
//   AssetManager OWNS all loaded assets.
//   Callers get raw pointers — they must NOT call SDL_DestroyTexture on them.
//   AssetManager::Shutdown() destroys everything.
//
// Cross-platform note:
//   All paths go through SDL_GetBasePath() internally so assets load
//   correctly on different OS without any change in calling code.
//
// ==============================================================================
class AssetManager
{
public:
    AssetManager();
    ~AssetManager();

    // Disable copy — AssetManager owns SDL resources
    AssetManager(const AssetManager &) = delete;
    AssetManager &operator=(const AssetManager &) = delete;

    // Init stores the SDL_Renderer needed for texture creation.
    // Must be called before any Load function.
    void Init(SDL_Renderer *renderer);

    // Shutdown destroys ALL cached assets. Call once on game exit.
    void Shutdown();

    // ==== Textures ====

    // LoadTexture loads a PNG/JPG from disk (or returns cached copy).
    // id       = your chosen name, e.g. "hero", "goblin", "ui_button"
    // filePath = relative to assets/textures/, e.g. "hero/idle.png"
    //
    // Returns nullptr if the file doesn't exist or fails to load.
    // The returned pointer is valid until AssetManager::Shutdown().
    SDL_Texture *LoadTexture(const std::string &id,
                             const std::string &filePath);

    // GetTexture retrieves a previously loaded texture by id.
    // Returns nullptr if id was never loaded.
    SDL_Texture *GetTexture(const std::string &id) const;

    // ==== Audio ====

    // LoadSound loads a short sound effect (WAV/OGG) for Mix_PlayChannel.
    // id       = your chosen name, e.g. "attack_swing", "coin_pickup"
    // filePath = relative to assets/audio/sfx/, e.g. "combat/swing.wav"
    Mix_Chunk *LoadSound(const std::string &id,
                         const std::string &filePath);

    // GetSound retrieves a previously loaded sound effect by id.
    // Returns nullptr if id was never loaded.
    Mix_Chunk *GetSound(const std::string &id) const;

    // LoadMusic loads background music (OGG/MP3) for Mix_PlayMusic.
    // Music streams from disk — not fully loaded into memory like sounds.
    // id       = e.g. "battle_theme", "dungeon_ambient"
    // filePath = relative to assets/audio/music/, e.g. "battle.ogg"
    Mix_Music *LoadMusic(const std::string &id,
                         const std::string &filePath);

    // GetMusic retrieves a previously loaded background music by id.
    // Returns nullptr if id was never loaded.
    Mix_Music *GetMusic(const std::string &id) const;

    // ==== Fonts ====

    // LoadFont loads a TTF font at a specific size.
    // id       = e.g. "ui_small", "ui_large", "damage_number"
    // filePath = relative to assets/fonts/, e.g. "pixel_font.ttf"
    // size     = point size — for pixel art fonts, use exact pixel sizes
    TTF_Font *LoadFont(const std::string &id,
                       const std::string &filePath,
                       int size);

    // GetFont retrieves a previously loaded TTF font by id.
    // Returns nullptr if id was never loaded.
    TTF_Font *GetFont(const std::string &id) const;

    // ==== Utility ====

    // UnloadTexture removes one texture from cache and frees GPU memory.
    // Use for level-specific assets that shouldn't stay in memory.
    void UnloadTexture(const std::string &id);

    void UnloadSound(const std::string &id);
    void UnloadMusic(const std::string &id);

    // GetOrCreateTextTexture — returns a cached SDL_Texture for the given text,
    // font id, and color. Creates and caches it on first call.
    // The cache key is "fontId:text:r,g,b" — distinct colors = distinct textures.
    //
    // Returned pointer is valid until Shutdown() or UnloadFont() for that font id.
    // Caller must NOT destroy it.
    SDL_Texture *GetOrCreateTextTexture(const std::string &fontId,
                                        const std::string &text,
                                        SDL_Color color);

    // ClearTextCache — frees all cached text textures.
    // Call when the locale or font changes.
    void ClearTextCache();

private:
    SDL_Renderer *m_renderer = nullptr;

    // unordered_map gives O(1) average lookup by string key.
    // The map OWNS the SDL pointers — destroyed in Shutdown().
    std::unordered_map<std::string, SDL_Texture *> m_textures;
    // Text texture cache — key: "fontId:text:r,g,b,a"
    std::unordered_map<std::string, SDL_Texture *> m_textCache;
    std::unordered_map<std::string, Mix_Chunk *> m_sounds;
    std::unordered_map<std::string, Mix_Music *> m_music;
    std::unordered_map<std::string, TTF_Font *> m_fonts;

    // Builds the full absolute path from a relative asset path.
    // Uses SDL_GetBasePath() for cross-platform correctness.
    std::string BuildPath(const std::string &subDir,
                          const std::string &filePath) const;
};