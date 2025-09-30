#ifndef AUDIO_H
#define AUDIO_H

#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

class AudioManager {
private:
    std::unordered_map<std::string, Mix_Chunk*> sfx;  // One-shots
    std::unordered_map<std::string, Mix_Music*> music;  // Loops/ambients
    int num_channels = 8;
    std::vector<std::function<void()>> event_callbacks;  // e.g., on_thunder
    std::string current_bgm = "";
    std::string current_overlay = "";

public:
    AudioManager();
    ~AudioManager();

    void load_sfx(const std::string& path, const std::string& id);
    void load_bgm(const std::string& path, const std::string& id);  // For music
    int play_sfx(const std::string& id, int volume = MIX_MAX_VOLUME, float pan = 0.0f, float distance = 1.0f);  // Returns channel
    void play_bgm(const std::string& id, int volume = 60, int fade_ms = 2000);  // Loop BGM with fade
    void play_overlay(const std::string& id, int volume = 40, bool additive = true);  // Weather layer
    void stop_bgm(int fade_ms = 2000);
    void stop_overlay(const std::string& id, int fade_ms = 1000);
    void crossfade_bgm(const std::string& new_id, int fade_ms = 3000);  // Smooth switch
    void register_event(const std::string& event, std::function<void()> callback);

    void set_channel_volume(int channel, int volume);
    void set_channel_pan(int channel, float left_right);  // -1 left, 1 right
    void set_distance_attenuation(int channel, float dist);  // Volume /= dist factor
};

#endif
