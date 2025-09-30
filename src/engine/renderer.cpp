#include "renderer.h"
#include <algorithm>
#include <nlohmann/json.hpp>
#include <SDL3_image/SDL_image.h>

SDL_FPoint Renderer::grid_to_iso(int grid_x, int grid_y) {
    float sx = (grid_x - grid_y) * (tile_w / 2.0f) + camera.x;
    float sy = (grid_x + grid_y) * (tile_h / 2.0f) + camera.y;
    return {sx, sy};
}

std::pair<int, int> Renderer::screen_to_grid(float mx, float my) {
    mx -= camera.x; my -= camera.y;
    int gx = (mx / (tile_w / 2.0f) + my / (tile_h / 2.0f)) / 2;
    int gy = (my / (tile_h / 2.0f) - mx / (tile_w / 2.0f)) / 2;
    return {gx, gy};
}

void Renderer::update_camera(int px, int py) {
    auto iso_pos = grid_to_iso(px, py);
    camera.x = 400 - iso_pos.x;
    camera.y = 300 - iso_pos.y;
}

SDL_Texture* Renderer::load_texture(const std::string& path) {
    auto it = texture_cache.find(path);
    if (it != texture_cache.end()) return it->second;

    std::string full_path = "assets/graphics/tiles/" + path;
    SDL_Surface* surf = IMG_Load(full_path.c_str());
    if (!surf) {
        SDL_Log("Texture load error: %s", SDL_GetError());
        return nullptr;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(sdl_renderer, surf);
    SDL_DestroySurface(surf);
    texture_cache[path] = tex;
    return tex;
}

void Renderer::render_layer(const World& world, int map_layer, float time) {
    std::vector<Renderable> batch;

    for (int gy = 0; gy < World::HEIGHT; ++gy) {
        for (int gx = World::WIDTH + gy; gx >= gy; --gx) {
            if (gx >= World::WIDTH || gy >= World::HEIGHT) continue;
            for (const auto& height_entry : world.get_layer(map_layer)[gx][gy]) {
                int h = height_entry.first;
                const auto& tile_id = height_entry.second;
                if (!tile_id) continue;
                const auto* tile = world.get_tileset().get(*tile_id);
                if (!tile || h >= tile->height_levels.size()) continue;

                const auto& lev = tile->height_levels[h];
                auto [sx, sy] = grid_to_iso(gx, gy);
                sy -= lev.height * (tile_h / 2.0f);
                SDL_FRect rect = {sx, sy, (float)tile_w, (float)tile_h};
                float depth = sy + lev.height;

                // Animated frame
                std::string frame_path = tile->get_animated_frame(time);
                SDL_Texture* tex = load_texture(frame_path);
                if (!tex) {
                    SDL_SetRenderDrawColor(sdl_renderer, 0, 128, 255, 128);  // Water blue fallback
                    SDL_RenderFillRect(sdl_renderer, &rect);
                }

                batch.push_back({tex, rect, depth, 0});
            }
        }
    }

    std::sort(batch.begin(), batch.end(), [](const Renderable& a, const Renderable& b) { return a.depth < b.depth; });

    for (const auto& item : batch) {
        if (item.texture) {
            SDL_SetTextureBlendMode(item.texture, SDL_BLENDMODE_BLEND);
            SDL_RenderTexture(sdl_renderer, item.texture, nullptr, &item.dst);
        }
    }

    // Emitters (fire, smoke, rain, snow, splash, spark, fog, grass_sway)
    // Assume world has vectors; call emitter.render(sdl_renderer);

    lighting.update_occluders(world.get_tileset(), world, map_layer);
    lighting.render_lighting(world, map_layer);
}

void Renderer::render_world(const World& world, int player_layer, float time) {
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl_renderer);
    
    // Render layers
    for (int layer = 0; layer < World::NUM_MAP_LAYERS; ++layer) {
        render_layer(world, layer, time);
    }
    
    SDL_RenderPresent(sdl_renderer);
}
