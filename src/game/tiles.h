#ifndef TILES_H
#define TILES_H

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <array>

struct LightSource {
    int intensity = 0;
    int radius = 0;
    std::array<int, 3> color = {255, 255, 255};
};

struct HeightLevel {
    int height = 0;
    bool passable = true;
    bool transparent = false;
    std::map<std::string, std::string> views;
    std::vector<std::pair<int, int>> edges;
};

struct Tile {
    std::string id, type, description;
    int preferred_layer = 1;
    bool blocks_sight = false;
    std::vector<HeightLevel> height_levels;
    bool supports_furniture = false;
    std::vector<int> connects_layers;
    LightSource emits_light;
    int flammability = 0;
    int wetness_threshold = 10;
    std::vector<std::string> animation_frames;
    float animation_speed = 0.1f;
    bool wind_sway = false;

    std::string get_animated_frame(float time) const;  // Inline for simplicity
};

class Tiles {
private:
    std::unordered_map<std::string, Tile> tileset;

public:
    void load(const std::string& path);
    const Tile* get(const std::string& id) const;
    std::vector<LightSource> get_all_lights() const;
};

#endif
