#include "world.h"
#include "../engine/input.h"
#include "../engine/particles.h"
#include "../engine/audio.h"
#include <algorithm>
#include <fstream>
#include <random>
#include <queue>
#include <cmath>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);

World::World() {
    audio = new AudioManager();
    for (auto& layer : grid) {
        layer.resize(WIDTH, std::vector<std::map<int, std::optional<std::string>>>(HEIGHT));
    }
    tile_wetness = std::array<std::vector<std::vector<int>>, NUM_MAP_LAYERS>();
    for (auto& layer : tile_wetness) {
        layer.resize(WIDTH, std::vector<int>(HEIGHT, 0));
    }
    start_time = std::chrono::steady_clock::now();
    game_time = 12.0f;
    moon_phase = 0;
    global_time = 0.0f;
    current_bgm = "day_ambient";  // Initial
    current_overlay = "";
}

World::~World() {
    delete audio;
}

void World::load_tiles(const std::string& path) {
    tileset.load(path);
}

void World::load_map(const std::string& path) {
    std::ifstream f(path);
    nlohmann::json j;
    j << f;

    for (const auto& layer_entry : j["map"].items()) {
        int layer = std::stoi(layer_entry.key());
        if (layer >= NUM_MAP_LAYERS) continue;
        for (int x = 0; x < WIDTH; ++x) {
            for (int y = 0; y < HEIGHT; ++y) {
                for (const auto& h_entry : layer_entry.value()[x][y]) {
                    int h = h_entry["height"];
                    std::string tile_id = h_entry["tile"];
                    place_tile(layer, x, y, h, tile_id);
                }
            }
        }
    }

    if (j.contains("biomes")) {
        elev_map.resize(WIDTH);
        moist_map.resize(WIDTH);
        for (int x = 0; x < WIDTH; ++x) {
            elev_map[x].resize(HEIGHT);
            moist_map[x].resize(HEIGHT);
            for (int y = 0; y < HEIGHT; ++y) {
                elev_map[x][y] = j["biomes"]["elev"][x][y];
                moist_map[x][y] = j["biomes"]["moist"][x][y];
            }
        }
    }
}

void World::place_tile(int map_layer, int x, int y, int height_level, const std::string& tile_id) {
    if (map_layer >= 0 && map_layer < NUM_MAP_LAYERS && x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT && height_level < MAX_HEIGHT_LEVELS) {
        grid[map_layer][x][y][height_level] = tile_id;
    }
}

bool World::can_move_to(int from_layer, int to_layer, int x, int y, int actor_height) {
    if (from_layer != to_layer) return has_connection(from_layer, to_layer, x, y);

    for (int h = 0; h < MAX_HEIGHT_LEVELS; ++h) {
        auto it = grid[to_layer][x][y].find(h);
        if (it != grid[to_layer][x][y].end() && it->second) {
            const auto* tile = tileset.get(*it->second);
            if (tile && h < tile->height_levels.size()) {
                const auto& lev = tile->height_levels[h];
                if (!lev.passable && actor_height <= lev.height) return false;
            }
        }
    }
    return true;
}

bool World::has_connection(int from_layer, int to_layer, int x, int y) const {
    auto it = grid[from_layer][x][y].find(0);
    if (it != grid[from_layer][x][y].end() && it->second) {
        const auto* tile = tileset.get(*it->second);
        if (tile && tile->type == "bridge") {
            for (int cl : tile->connects_layers) {
                if (cl == to_layer) return true;
            }
        }
    }
    return false;
}

bool World::can_place_on_furniture(int layer, int x, int y) const {
    auto it = grid[layer][x][y].find(0);
    if (it != grid[layer][x][y].end() && it->second) {
        const auto* tile = tileset.get(*it->second);
        return tile && tile->supports_furniture;
    }
    return false;
}

void World::update(const Input& input) {
    for (auto& actor : actors) {
        if (input.is_key_down(SDLK_w)) actor.move(0, -1);
        if (input.is_key_down(SDLK_s)) actor.move(0, 1);
        if (input.is_key_down(SDLK_a)) actor.move(-1, 0);
        if (input.is_key_down(SDLK_d)) actor.move(1, 0);
        if (input.is_key_down(SDLK_PERIOD)) {  // Up
            int new_layer = actor.current_map_layer + 1;
            if (new_layer < NUM_MAP_LAYERS && has_connection(actor.current_map_layer, new_layer, actor.x, actor.y)) {
                actor.move_z(1);
            }
        }
        if (input.is_key_down(SDLK_COMMA)) {  // Down
            int new_layer = actor.current_map_layer - 1;
            if (new_layer >= 0 && has_connection(actor.current_map_layer, new_layer, actor.x, actor.y)) {
                actor.move_z(-1);
            }
        }

        // Footsteps
        const auto* tile = get_tile(actor.current_map_layer, actor.x, actor.y, 0);
        if (tile) {
            std::string sfx = "footstep_" + tile->type;
            audio->play_sfx(sfx, 80, actor.x / 50.0f - 0.5f, 1.0f);
        }

        actor.regen_mana(1);
    }

    float dt = 1.0f / 60.0f;
    float wind = 0.0f;
    if (current_weather == Weather::RAIN || current_weather == Weather::SNOW) wind = std::uniform_real_distribution<float>(-1,1)(gen);
    for (auto& emitter : fire_emitters) emitter.update(dt);
    for (auto& emitter : smoke_emitters) emitter.update(dt, wind);
    for (auto& emitter : rain_emitters) emitter.update(dt, wind, 1.0f);
    for (auto& emitter : snow_emitters) emitter.update(dt, wind);
    for (auto& emitter : splash_emitters) emitter.update(dt);
    for (auto& emitter : spark_emitters) emitter.update(dt);
    for (auto& emitter : fog_emitters) emitter.update(dt);
    for (auto& emitter : grass_emitters) emitter.update(dt, wind);

    // Weather integration
    if (current_weather == Weather::RAIN) {
        if (rain_emitters.empty()) rain_emitters.emplace_back(800, 600, 1.0f);
        for (auto& emitter : rain_emitters) emitter.update(dt, wind, 1.0f);

        // Splashes/wetness
        for (int x = 0; x < WIDTH; x += 5) {
            for (int y = 0; y < HEIGHT; y += 5) {
                if (std::uniform_real_distribution<float>(0,1)(gen) < 0.1) {
                    SDL_FPoint hit_pos = {static_cast<float>(x * 32), static_cast<float>(y * 16)};
                    SplashEmitter splash(hit_pos);
                    splash.spawn_splash(3 + std::uniform_int_distribution<int>(0,3)(gen));
                    splash_emitters.push_back(splash);
                    add_wetness(1, x, y, 1);  // Ground layer
                }
            }
        }
        audio->play_overlay("rain_patter", 40, true);
    } else if (current_weather == Weather::SNOW) {
        if (snow_emitters.empty()) snow_emitters.emplace_back(800, 600, 0.7f);
        for (auto& emitter : snow_emitters) emitter.update(dt, wind);
        audio->play_overlay("snow_wind", 30, true);
    } else {
        rain_emitters.clear();
        snow_emitters.clear();
        audio->stop_overlay("rain_patter");
        audio->stop_overlay("snow_wind");
    }

    // Fog on low/night/biomes
    int check_layer = actors.empty() ? 1 : actors[0].current_map_layer;
    if (check_layer < 2 || game_time > 20 || game_time < 4) {
        std::string biome = get_biome_at(check_layer, 25, 25);  // Avg
        float fog_int = (biome == "swamp" ? 1.5f : 1.0f);
        if (fog_emitters.empty()) {
            SDL_FPoint fog_pos = {400, 300};
            FogEmitter fog(fog_pos, fog_int);
            fog_emitters.push_back(fog);
        }
        for (auto& emitter : fog_emitters) emitter.update(dt);
    } else {
        fog_emitters.clear();
    }

    // Grass sway on wind (for grass_wispy tiles)
    for (int x = 0; x < WIDTH; x += 5) {
        for (int y = 0; y < HEIGHT; y += 5) {
            const auto* tile = get_tile(1, x, y, 0);  // Ground
            if (tile && tile->id == "grass_wispy" && std::uniform_real_distribution<float>(0,1)(gen) < 0.2) {
                SDL_FPoint grass_pos = {static_cast<float>(x * 32), static_cast<float>(y * 16)};
                GrassSwayEmitter grass(grass_pos, 10);
                grass_emitters.push_back(grass);
            }
        }
    }
    for (auto& emitter : grass_emitters) emitter.update(dt, wind);

    // Fire spread
    std::queue<std::tuple<int, int, int>> spread_queue;
    for (int l = 0; l < NUM_MAP_LAYERS; ++l) {
        for (int x = 0; x < WIDTH; ++x) {
            for (int y = 0; y < HEIGHT; ++y) {
                const auto* tile = get_tile(l, x, y, 0);
                if (tile && tile->flammability > 0 && std::uniform_real_distribution<float>(0,1)(gen) < 0.1 * (1 - tile_wetness[l][x][y]/10.0f)) {
                    spread_queue.push({l, x, y});
                }
            }
        }
    }
    while (!spread_queue.empty()) {
        auto [l, x, y] = spread_queue.front(); spread_queue.pop();
        for (auto [dx, dy] : {{0,1},{1,0},{0,-1},{-1,0}}) {
            int nx = x + dx, ny = y + dy;
            if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT) {
                const auto* nt = get_tile(l, nx, ny, 0);
                if (nt && nt->flammability > 50 && std::uniform_real_distribution<float>(0,1)(gen) < 0.3) {
                    SDL_FPoint fire_pos = {static_cast<float>(nx * 32), static_cast<float>(ny * 16)};
                    FireEmitter new_fire(fire_pos, 1);
                    fire_emitters.push_back(new_fire);
                    spread_queue.push({l, nx, ny});
                }
            }
        }
    }

    if (!fire_emitters.empty()) audio->play_overlay("fire_crackle", 60, true);
    else audio->stop_overlay("fire_crackle");
}

std::array<int, 3> World::get_tint_color() const {
    float hour = fmod(game_time, 24.0f);
    std::array<int, 3> color = {100, 100, 100};
    if (hour < 6 || hour > 18) color = {50, 50, 100};
    else if (hour > 18 || hour < 6) color = {255, 150, 100};
    if (current_weather == Weather::RAIN) {
        for (auto& c : color) c = static_cast<int>(c * 0.7f);
    }
    return color;
}

LightSource World::get_moonlight() const {
    LightSource moon;
    moon.radius = 800;
    moon.intensity = 20 + (moon_phase == 14 ? 40 : 0);
    moon.color = {100, 150, 255};
    return moon;
}

void World::update_time(float dt) {
    auto now = std::chrono::steady_clock::now();
    float real_dt = std::chrono::duration<float>(now - start_time).count() / 60.0f;
    game_time += real_dt / 60.0f * 24.0f;
    game_time = fmod(game_time, 24.0f);
    start_time = now;
    moon_phase = (moon_phase + 1) % 29;

    float hour = fmod(game_time, 24.0f);
    std::string new_bgm;
    if (hour < 6 || hour > 18) new_bgm = "night_ambient";
    else new_bgm = "day_ambient";
    if (new_bgm != current_bgm) {
        audio->crossfade_bgm(new_bgm, 3000);
        current_bgm = new_bgm;
    }

    std::string overlay;
    if (current_weather == Weather::RAIN) overlay = "rain_patter";
    else if (current_weather == Weather::SNOW) overlay = "snow_wind";
    else overlay = "";
    if (overlay != current_overlay) {
        audio->play_overlay(overlay, 40, true);
        current_overlay = overlay;
    }

    if (current_weather == Weather::RAIN && std::uniform_real_distribution<float>(0,1)(gen) < 0.02) {
        strike_lightning();
    }

    global_time += dt;
}

std::string World::get_biome_at(int layer, int x, int y) const {
    if (layer < 1 || layer > 2) return "none";
    float elev = elev_map[x][y];
    float moist = moist_map[x][y];
    if (elev > 0.7) return "mountain";
    if (moist > 0.7) return "swamp";
    if (moist < 0.3 && elev < 0.3) return "desert";
    if (moist > 0.5) return "forest";
    return "plains";
}

void World::add_wetness(int layer, int x, int y, int amount) {
    if (layer >= 0 && layer < NUM_MAP_LAYERS && x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        tile_wetness[layer][x][y] += amount;
        tile_wetness[layer][x][y] = std::min(20, tile_wetness[layer][x][y]);
    }
}

void World::strike_lightning() {
    lightning_flash_timer = 1.0f;
    audio->play_sfx("thunder", 100);

    int rx = std::uniform_int_distribution<int>(0, WIDTH-1)(gen);
    int ry = std::uniform_int_distribution<int>(0, HEIGHT-1)(gen);
    SDL_FPoint strike_pos = {static_cast<float>(rx * 32), static_cast<float>(ry * 16)};
    SparkEmitter sparks(strike_pos, 100);
    spark_emitters.push_back(sparks);
}

const Tile* World::get_tile(int layer, int x, int y, int h) const {
    auto it = grid[layer][x][y].find(h);
    if (it != grid[layer][x][y].end() && it->second) {
        return tileset.get(*it->second);
    }
    return nullptr;
}
