#include "tiles.h"
#include <fstream>
#include <unordered_map>
#include <SDL3/SDL.h>

void Tiles::load(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        SDL_Log("Error: Failed to open tileset %s", path.c_str());
        return;
    }
    nlohmann::json j;
    try {
        j << f;
    } catch (const std::exception& e) {
        SDL_Log("Error: Invalid JSON in tileset: %s", e.what());
        return;
    }

    tileset.clear();
    for (const auto& entry : j["tiles"]) {
        Tile t;
        t.id = entry["id"];
        t.type = entry["type"];
        t.description = entry["description"];
        t.preferred_layer = entry.value("preferred_layer", 1);
        t.blocks_sight = entry.value("blocks_sight", false);
        t.supports_furniture = entry.value("supports_furniture", false);
        t.flammability = entry.value("flammability", 0);
        t.wetness_threshold = entry.value("wetness_threshold", 10);

        // Height levels
        for (const auto& hl : entry["height_levels"]) {
            HeightLevel lev;
            lev.height = hl["height"];
            lev.passable = hl["passable"];
            lev.transparent = hl["transparent"];
            if (hl["views"].contains("default")) {
                lev.views["default"] = hl["views"]["default"];
            }
            t.height_levels.push_back(lev);
        }

        // Light
        if (entry.contains("emits_light")) {
            t.emits_light.intensity = entry["emits_light"]["intensity"];
            t.emits_light.radius = entry["emits_light"]["radius"];
            for (int i = 0; i < 3; ++i) t.emits_light.color[i] = entry["emits_light"]["color"][i];
        }

        // Connects
        if (entry.contains("connects_layers")) {
            for (const auto& cl : entry["connects_layers"]) {
                t.connects_layers.push_back(cl);
            }
        }

        // Animation
        if (entry.contains("animation_frames")) {
            for (const auto& frame : entry["animation_frames"]) {
                t.animation_frames.push_back(frame);
            }
            t.animation_speed = entry.value("animation_speed", 0.1f);
            t.wind_sway = entry.value("wind_sway", false);
        }

        // Edges stub (pairs of points for line segments)
        for (auto& lev : t.height_levels) {
            if (!lev.transparent) {
                lev.edges = {{0,0}, {64,0}, {64,32}, {0,32}};
            }
        }

        tileset[t.id] = t;
    }
}

const Tile* Tiles::get(const std::string& id) const {
    auto it = tileset.find(id);
    if (it != tileset.end()) return &it->second;

    static Tile empty_tile;
    static bool init = false;
    if (!init) {
        empty_tile.id = "empty";
        empty_tile.type = "empty";
        empty_tile.description = "Empty tile";
        HeightLevel lev;
        lev.height = 0;
        lev.passable = true;
        lev.transparent = true;
        lev.views["default"] = "";
        empty_tile.height_levels.push_back(lev);
        init = true;
    }
    SDL_Log("Warning: Missing tile '%s', using empty fallback", id.c_str());
    return &empty_tile;
}

std::vector<LightSource> Tiles::get_all_lights() const {
    std::vector<LightSource> lights;
    for (const auto& pair : tileset) {
        if (pair.second.emits_light.intensity > 0) lights.push_back(pair.second.emits_light);
    }
    return lights;
}

std::string Tile::get_animated_frame(float time) const {
    if (animation_frames.empty()) return height_levels[0].views.at("default");

    size_t num_frames = animation_frames.size();
    size_t frame = static_cast<size_t>(fmod(time * animation_speed, num_frames));
    return animation_frames[frame];
}
