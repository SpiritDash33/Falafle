#include "lighting.h"
#include <algorithm>
#include <cmath>  // For visibility calc stub
#include <random>

void Lighting::update_occluders(const Tiles& tileset, const World& world, int map_layer) {
    occluders.clear();
    for (int x = 0; x < World::WIDTH; ++x) {
        for (int y = 0; y < World::HEIGHT; ++y) {
            for (const auto& h_entry : world.get_layer(map_layer)[x][y]) {
                const auto* tile = tileset.get(*h_entry.second);
                if (tile) {
                    for (const auto& lev : tile->height_levels) {
                        if (!lev.transparent) {
                            int base_x = x * 32 - y * 32;  // Iso
                            int base_y = (x + y) * 16;
                            for (const auto& edge : lev.edges) {
                                SDL_FPoint start = {base_x + edge.first, base_y + edge.second};
                                SDL_FPoint end = {base_x + edge.third, base_y + edge.fourth};  // Assume edge as {x1,y1,x2,y2}
                                occluders.emplace_back(start, end);
                            }
                        }
                    }
                }
            }
        }
    }
}

VisibilityPolygon Lighting::compute_visibility(const SDL_FPoint& light_pos, const std::vector<std::pair<SDL_FPoint, SDL_FPoint>>& walls) {
    VisibilityPolygon poly;
    int r = 100;  // Scaled
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0f, 2 * M_PI);
    for (int i = 0; i < 8; ++i) {
        float angle = i * M_PI / 4;
        poly.points.push_back({light_pos.x + static_cast<float>(r * cos(angle)), light_pos.y + static_cast<float>(r * sin(angle))});
    }
    return poly;  // Stub; real clip with walls
}

void Lighting::render_lighting(const World& world, int map_layer) {
    if (!shadow_tex) return;

    SDL_SetRenderTarget(renderer, shadow_tex);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black shadow base
    SDL_RenderClear(renderer);

    // Additive lights
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
    for (const auto& light : sources) {
        // Stub circle; use poly for visibility
        SDL_SetRenderDrawColor(renderer, light.color[0], light.color[1], light.color[2], light.intensity * 255);
        // SDL_RenderFillCircle (stub)
    }

    // Ambient dim for layer
    int ambient = map_layer * 30;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255 - ambient);
    SDL_RenderFillRect(renderer, nullptr);

    SDL_SetRenderTarget(renderer, nullptr);
    SDL_SetTextureBlendMode(shadow_tex, SDL_BLENDMODE_MOD);
    SDL_RenderTexture(renderer, shadow_tex, nullptr, nullptr);
}
