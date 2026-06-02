#include "AudioManager.h"

AudioManager::AudioManager() = default;
AudioManager::~AudioManager() = default;

// ==============================================================================
// Sound Effects
// ==============================================================================

int AudioManager::PlaySFX(Mix_Chunk *chunk, float volume)
{
    if (!chunk)
        return -1;

    // Mix_VolumeChunk sets the volume of a specific chunk.
    // This is per-chunk, not per-channel, so the volume is remembered.
    Mix_VolumeChunk(chunk, ToSDLVolume(volume));

    // Mix_PlayChannel parameters:
    // -1      = auto-select the first available channel
    // chunk   = the sound to play
    // 0       = loops; 0 means play once (not loop once — confusingly, 0 = no repeats)
    //
    // Returns the channel used, or -1 if no channels are available.
    // SDL_mixer defaults to 8 channels. If all are busy, new sounds are dropped.
    // We'll increase this if needed with Mix_AllocateChannels().
    return Mix_PlayChannel(-1, chunk, 0);
}

int AudioManager::PlaySFXLoop(Mix_Chunk *chunk, int loops, float volume)
{
    if (!chunk)
        return -1;
    Mix_VolumeChunk(chunk, ToSDLVolume(volume));
    // loops = number of additional repeats. -1 = infinite.
    return Mix_PlayChannel(-1, chunk, loops);
}

void AudioManager::StopChannel(int channel)
{
    if (channel < 0)
        return;
    // Mix_HaltChannel stops the channel immediately with no fade.
    Mix_HaltChannel(channel);
}

void AudioManager::StopAllSFX()
{
    // -1 means all channels
    Mix_HaltChannel(-1);
}

// ==============================================================================
// Music
// ==============================================================================

void AudioManager::PlayMusic(Mix_Music *music, int loops)
{
    if (!music)
        return;

    // If music is already playing, stop it first.
    if (Mix_PlayingMusic())
    {
        Mix_HaltMusic();
    }

    // Mix_PlayMusic parameters:
    // music  = the Mix_Music* to play
    // loops  = -1 for infinite, 0 plays once, 1 plays twice, etc.
    Mix_PlayMusic(music, loops);
}

void AudioManager::StopMusic()
{
    Mix_HaltMusic();
}

void AudioManager::PauseMusic()
{
    // Mix_PauseMusic preserves playback position.
    // The track resumes from the exact same position when ResumeMusic() is called.
    if (Mix_PlayingMusic())
        Mix_PauseMusic();
}

void AudioManager::ResumeMusic()
{
    if (Mix_PausedMusic())
        Mix_ResumeMusic();
}

void AudioManager::FadeOutMusic(int ms)
{
    // Mix_FadeOutMusic gradually reduces volume to 0 over `ms` milliseconds.
    // After fading, the music stops automatically.
    // Returns 1 if music is fading, 0 if nothing is playing.
    Mix_FadeOutMusic(ms);
}

// ==============================================================================
// Volume
// ==============================================================================

void AudioManager::SetMasterSFXVolume(float volume)
{
    // Mix_Volume(-1, volume) sets all channels at once.
    // -1 as the channel means "all channels".
    Mix_Volume(-1, ToSDLVolume(volume));
}

void AudioManager::SetMusicVolume(float volume)
{
    // Mix_VolumeMusic sets the music stream volume independently of SFX.
    Mix_VolumeMusic(ToSDLVolume(volume));
}

// ==============================================================================
// Queries
// ==============================================================================

bool AudioManager::IsMusicPlaying() const
{
    // Mix_PlayingMusic returns non-zero if music is currently playing.
    // Note: returns true even if paused.
    return Mix_PlayingMusic() != 0;
}

bool AudioManager::IsMusicPaused() const
{
    return Mix_PausedMusic() != 0;
}

// ==============================================================================
// Private
// ==============================================================================

int AudioManager::ToSDLVolume(float volume) const
{
    // Clamp to [0.0, 1.0] then scale to [0, MIX_MAX_VOLUME]
    // MIX_MAX_VOLUME = 128 (defined by SDL_mixer)
    if (volume < 0.0f)
        volume = 0.0f;
    if (volume > 1.0f)
        volume = 1.0f;
    return static_cast<int>(volume * MIX_MAX_VOLUME);
}