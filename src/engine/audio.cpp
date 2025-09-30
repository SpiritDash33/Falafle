#include "audio.h"
#include <SDL2/SDL_mixer.h>
#include <algorithm>
#include <iostream>

AudioManager::AudioManager() {
    Mix_Init(MIX_INIT_OGG);
    Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048);
    Mix_AllocateChannels(num_channels);
}

AudioManager::~AudioManager() {
    for (auto& pair : sfx) Mix_FreeChunk(pair.second);
    for (auto& pair : music) Mix_FreeMusic(pair.second);
    Mix_CloseAudio();
    Mix_Quit();
}

void AudioManager::load_sfx(const std::string& path, const std::string& id) {
    sfx[id] = Mix_LoadWAV(path.c_str());
    if (!sfx[id]) std::cerr << "SFX load error: " << Mix_GetError() << std::endl;
}

void AudioManager::load_bgm(const std::string& path, const std::string& id) {
    music[id] = Mix_LoadMUS(path.c_str());
    if (!music[id]) std::cerr << "BGM load error: " << Mix_GetError() << std::endl;
}

int AudioManager::play_sfx(const std::string& id, int volume, float pan, float distance) {
    if (sfx.find(id) == sfx.end()) return -1;
    int channel = Mix_PlayChannel(-1, sfx[id], 0);
    if (channel != -1) {
        int vol = std::max(0, static_cast<int>(volume / (distance + 1.0f)));
        Mix_Volume(channel, vol);
        // Pan: -128 left to 127 right
        Mix_SetPanning(channel, static_cast<int>((pan + 1.0f) * 64 - 64), static_cast<int>((1.0f - pan) * 64));
    }
    return channel;
}

void AudioManager::play_bgm(const std::string& id, int volume, int fade_ms) {
    if (music.find(id) == music.end()) return;
    if (current_bgm != id) {
        stop_bgm(fade_ms / 2);
        Mix_FadeInMusic(music[id], -1, fade_ms);
        current_bgm = id;
    }
    Mix_VolumeMusic(volume);
}

void AudioManager::play_overlay(const std::string& id, int volume, bool additive) {
    if (music.find(id) == music.end()) return;
    if (current_overlay != id) {
        stop_overlay(current_overlay, 500);
        Mix_PlayMusic(music[id], -1);
        Mix_VolumeMusic(volume);
        current_overlay = id;
    }
}

void AudioManager::stop_bgm(int fade_ms) {
    if (!current_bgm.empty()) {
        Mix_FadeOutMusic(fade_ms);
        current_bgm = "";
    }
}

void AudioManager::stop_overlay(const std::string& id, int fade_ms) {
    if (current_overlay == id) {
        Mix_FadeOutMusic(fade_ms);
        current_overlay = "";
    }
}

void AudioManager::crossfade_bgm(const std::string& new_id, int fade_ms) {
    play_bgm(new_id, 60, fade_ms);
}

void AudioManager::register_event(const std::string& event, std::function<void()> callback) {
    event_callbacks.push_back(callback);
}

void AudioManager::set_channel_volume(int channel, int volume) {
    Mix_Volume(channel, volume);
}

void AudioManager::set_channel_pan(int channel, float left_right) {
    Mix_SetPanning(channel, static_cast<int>((left_right + 1.0f) * 64 - 64), static_cast<int>((1.0f - left_right) * 64));
}

void AudioManager::set_distance_attenuation(int channel, float dist) {
    int vol = Mix_Volume(channel, -1);
    vol = static_cast<int>(vol / (dist + 1.0f));
    Mix_Volume(channel, vol);
}
