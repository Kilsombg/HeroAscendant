#pragma once

#include <SDL2/SDL_mixer.h>
#include <string>

// ==============================================================================
// AudioManager
//
// All audio playback goes through here. Game code never calls Mix_* directly.
//
// Usage:
//   audio.PlaySFX("attack_swing")      ← short sound effect
//   audio.PlayMusic("battle_theme")    ← background music
//   audio.SetMusicVolume(0.5f)         ← 0.0 to 1.0
//
// ==============================================================================
class AudioManager
{
public:
    AudioManager();
    ~AudioManager();

    // ==== Sound Effects ====

    // PlaySFX plays a loaded sound effect once on any available channel.
    // soundId = id used when loading with AssetManager::LoadSound()
    // volume  = 0.0 (silent) to 1.0 (full volume)
    //
    // Returns the channel number used (-1 if failed).
    // You can use the channel number to stop a specific sound early.
    int PlaySFX(Mix_Chunk *chunk, float volume = 1.0f);

    // PlaySFXLoop plays a sound effect repeatedly.
    // loops = number of times to repeat (-1 = infinite)
    int PlaySFXLoop(Mix_Chunk *chunk, int loops, float volume = 1.0f);

    // StopChannel stops a specific channel. Use the return value from PlaySFX.
    void StopChannel(int channel);

    // StopAllSFX stops all sound effect channels immediately.
    void StopAllSFX();

    // ==== Music ====

    // PlayMusic starts background music.
    // loops = -1 for infinite loop, 0 plays once, N plays N+1 times.
    void PlayMusic(Mix_Music *music, int loops = -1);

    // StopMusic stops background music immediately.
    void StopMusic();

    // PauseMusic pauses without losing position — use on game pause screen.
    void PauseMusic();

    // ResumeMusic resumes from where it was paused.
    void ResumeMusic();

    // FadeOutMusic fades volume to 0 over `ms` milliseconds then stops.
    // Useful for smooth scene transitions.
    void FadeOutMusic(int ms);

    // ==== Volume ====

    // SetMasterSFXVolume sets volume for ALL sound effect channels.
    // volume = 0.0 to 1.0
    void SetMasterSFXVolume(float volume);

    // SetMusicVolume sets background music volume.
    void SetMusicVolume(float volume);

    // ==== Queries ====

    bool IsMusicPlaying() const;
    bool IsMusicPaused() const;

private:
    // SDL_mixer volume range is 0 to MIX_MAX_VOLUME (128).
    // We convert from our 0.0-1.0 float range to SDL's range.
    int ToSDLVolume(float volume) const;
};