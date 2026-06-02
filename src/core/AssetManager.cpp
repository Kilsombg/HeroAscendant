#include "AssetManager.h"

#include <SDL2/SDL_image.h>
#include <iostream>

AssetManager::AssetManager() = default;
AssetManager::~AssetManager() = default;

void AssetManager::Init(SDL_Renderer *renderer)
{
    m_renderer = renderer;
}

void AssetManager::Shutdown()
{
    // Destroy all textures
    for (auto &[id, texture] : m_textures)
    {
        if (texture)
            SDL_DestroyTexture(texture);
    }
    m_textures.clear();

    // Free all sound chunks
    for (auto &[id, chunk] : m_sounds)
    {
        if (chunk)
            Mix_FreeChunk(chunk);
    }
    m_sounds.clear();

    // Free all music
    for (auto &[id, music] : m_music)
    {
        if (music)
            Mix_FreeMusic(music);
    }
    m_music.clear();

    // Close all fonts
    for (auto &[id, font] : m_fonts)
    {
        if (font)
            TTF_CloseFont(font);
    }
    m_fonts.clear();
}

// ==============================================================================
// Textures
// ==============================================================================

SDL_Texture *AssetManager::LoadTexture(const std::string &id,
                                       const std::string &filePath)
{
    // if already loaded, return existing texture immediately.
    auto it = m_textures.find(id);
    if (it != m_textures.end())
    {
        return it->second; // Already loaded, no disk access needed
    }

    std::string fullPath = BuildPath("textures", filePath);

    // IMG_LoadTexture loads directly to a GPU texture.
    // Supports PNG, JPG, BMP, and more depending on IMG_Init flags.
    // Internally: loads file → SDL_Surface → SDL_Texture → frees surface.
    // We don't need the intermediate surface so this is the preferred function.
    SDL_Texture *texture = IMG_LoadTexture(m_renderer, fullPath.c_str());

    if (!texture)
    {
        std::cerr << "[AssetManager] Failed to load texture '" << id
                  << "' from: " << fullPath << "\n"
                  << "  IMG error: " << IMG_GetError() << "\n";
        return nullptr;
    }

    // Cache it. All future calls with the same id skip the disk load.
    m_textures[id] = texture;

    return texture;
}

SDL_Texture *AssetManager::GetTexture(const std::string &id) const
{
    auto it = m_textures.find(id);
    return (it != m_textures.end()) ? it->second : nullptr;
}

void AssetManager::UnloadTexture(const std::string &id)
{
    auto it = m_textures.find(id);
    if (it != m_textures.end())
    {
        SDL_DestroyTexture(it->second);
        m_textures.erase(it);
    }
}

// ==============================================================================
// Audio — Sound Effects
// ==============================================================================

Mix_Chunk *AssetManager::LoadSound(const std::string &id,
                                   const std::string &filePath)
{
    auto it = m_sounds.find(id);
    if (it != m_sounds.end())
        return it->second;

    std::string fullPath = BuildPath("audio/sfx", filePath);

    // Mix_LoadWAV loads a sound effect into memory as a Mix_Chunk.
    // Despite the name it supports WAV and OGG (if OGG support was init'd).
    // Mix_Chunk is fully loaded into RAM — suitable for short sounds.
    Mix_Chunk *chunk = Mix_LoadWAV(fullPath.c_str());

    if (!chunk)
    {
        std::cerr << "[AssetManager] Failed to load sound '" << id
                  << "': " << Mix_GetError() << "\n";
        return nullptr;
    }

    m_sounds[id] = chunk;
    return chunk;
}

Mix_Chunk *AssetManager::GetSound(const std::string &id) const
{
    auto it = m_sounds.find(id);
    return (it != m_sounds.end()) ? it->second : nullptr;
}

void AssetManager::UnloadSound(const std::string &id)
{
    auto it = m_sounds.find(id);
    if (it != m_sounds.end())
    {
        Mix_FreeChunk(it->second);
        m_sounds.erase(it);
    }
}

// ==============================================================================
// Audio — Music
// ==============================================================================

Mix_Music *AssetManager::LoadMusic(const std::string &id,
                                   const std::string &filePath)
{
    auto it = m_music.find(id);
    if (it != m_music.end())
        return it->second;

    std::string fullPath = BuildPath("audio/music", filePath);

    // Mix_LoadMUS loads music. Unlike Mix_Chunk, music streams from disk
    // during playback rather than loading fully into RAM.
    // This keeps memory usage low for long music tracks.
    Mix_Music *music = Mix_LoadMUS(fullPath.c_str());

    if (!music)
    {
        std::cerr << "[AssetManager] Failed to load music '" << id
                  << "': " << Mix_GetError() << "\n";
        return nullptr;
    }

    m_music[id] = music;
    return music;
}

Mix_Music *AssetManager::GetMusic(const std::string &id) const
{
    auto it = m_music.find(id);
    return (it != m_music.end()) ? it->second : nullptr;
}

void AssetManager::UnloadMusic(const std::string &id)
{
    auto it = m_music.find(id);
    if (it != m_music.end())
    {
        Mix_FreeMusic(it->second);
        m_music.erase(it);
    }
}

// ==============================================================================
// Fonts
// ==============================================================================

TTF_Font *AssetManager::LoadFont(const std::string &id,
                                 const std::string &filePath,
                                 int size)
{
    auto it = m_fonts.find(id);
    if (it != m_fonts.end())
        return it->second;

    std::string fullPath = BuildPath("fonts", filePath);

    // TTF_OpenFont loads a font file at a specific point size.
    // The same font file at different sizes must be loaded separately.
    // Each open font is a separate TTF_Font* with its own cache.
    TTF_Font *font = TTF_OpenFont(fullPath.c_str(), size);

    if (!font)
    {
        std::cerr << "[AssetManager] Failed to load font '" << id
                  << "': " << TTF_GetError() << "\n";
        return nullptr;
    }

    m_fonts[id] = font;
    return font;
}

TTF_Font *AssetManager::GetFont(const std::string &id) const
{
    auto it = m_fonts.find(id);
    return (it != m_fonts.end()) ? it->second : nullptr;
}

// ==============================================================================
// Private
// ==============================================================================

std::string AssetManager::BuildPath(const std::string &subDir,
                                    const std::string &filePath) const
{
    // SDL_GetBasePath() returns the directory where the executable lives.
    //
    // SDL allocates this string — we must SDL_free() it.
    char *basePath = SDL_GetBasePath();
    std::string full = std::string(basePath) + "assets/" + subDir + "/" + filePath;
    SDL_free(basePath);
    return full;
}