#ifndef WORLD_H
#define WORLD_H

#include <array>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <random>  // For np.random stub
#include "actor.h"
#include "tiles.h"
#include "../engine/particles.h"  // All emitters
#include <nlohmann/json.hpp>
#include <chrono>

// Forward declarations
class AudioManager;
class Input;

class World {
public:
    static constexpr int NUM_MAP_LAYERS = 6;
    static constexpr int WIDTH = 50, HEIGHT = 50;
    static constexpr int MAX_HEIGHT_LEVELS = 3;

    enum class Weather { CLEAR, RAIN, SNOW };
private:
    Weather current_weather = Weather::CLEAR;
    std::array<std::vector<std::vector<std::map<int, std::optional<std::string>>>>, NUM_MAP_LAYERS> grid;
    std::vector<Actor> actors;
    Tiles tileset;
    std::vector<FireEmitter> fire_emitters;
    std::vector<SmokeEmitter> smoke_emitters;
    std::vector<RainEmitter> rain_emitters;
    std::vector<SnowEmitter> snow_emitters;
    std::vector<SplashEmitter> splash_emitters;
    std::vector<SparkEmitter> spark_emitters;
    std::vector<FogEmitter> fog_emitters;
    std::vector<GrassSwayEmitter> grass_emitters;
    AudioManager* audio;

    float game_time = 0.0f;
    int moon_phase = 0;
    std::chrono::steady_clock::time_point start_time;
    float lightning_flash_timer = 0.0f;
    std::vector<std::vector<std::vector<int>>> tile_wetness;  // [layer][x][y]
    std::vector<std::vector<float>> elev_map, moist_map;  // For biomes
    std::string current_bgm = "";
    std::string current_overlay = "";

public:
    World();
    ~World();
    void load_tiles(const std::string& path);
    void load_map(const std::string& path);
    void place_tile(int map_layer, int x, int y, int height_level, const std::string& tile_id);
    bool can_move_to(int from_layer, int to_layer, int x, int y, int actor_height);
    bool has_connection(int from_layer, int to_layer, int x, int y) const;
    bool can_place_on_furniture(int layer, int x, int y) const;
    void update(const Input& input);
    void update_time(float dt);
    void set_weather(Weather w) { current_weather = w; }
    Weather get_weather() const { return current_weather; }
    float get_game_hour() const { return game_time; }
    std::array<int, 3> get_tint_color() const;
    LightSource get_moonlight() const;
    std::string get_biome_at(int layer, int x, int y) const;
    void add_wetness(int layer, int x, int y, int amount = 1);
    void strike_lightning();
    const auto& get_layer(int l) const { return grid[l]; }
    std::vector<Actor>& get_actors() { return actors; }
    const Tiles& get_tileset() const { return tileset; }
    const Tile* get_tile(int layer, int x, int y, int h) const;
    float global_time = 0.0f;  // For anim
};

#endif
