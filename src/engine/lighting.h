#ifndef LIGHTING_H
#define LIGHTING_H

#include <SDL3/SDL.h>
#include <vector>
#include <utility>  // pair
#include "../game/tiles.h"  // For LightSource
#include <array>

// Forward declarations
class World;

struct Light {
    SDL_FPoint pos;  // Iso screen pos
    int radius = 5;
    std::array<int, 3> color = {255, 255, 255};
    float intensity = 1.0f;
};

struct VisibilityPolygon {
    std::vector<SDL_FPoint> points;  // Clockwise polygon verts
};

class Lighting {
private:
    SDL_Renderer* renderer;
    std::vector<Light> sources;
    SDL_Texture* shadow_tex = nullptr;
    std::vector<std::pair<SDL_FPoint, SDL_FPoint>> occluders;  // Lines (start, end)

public:
    Lighting(SDL_Renderer* r) : renderer(r) {
        shadow_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 800, 600);
    }
    ~Lighting() { if (shadow_tex) SDL_DestroyTexture(shadow_tex); }

    void add_source(const Light& light) { sources.push_back(light); }
    void update_occluders(const Tiles& tileset, const World& world, int map_layer);
    VisibilityPolygon compute_visibility(const SDL_FPoint& light_pos, const std::vector<std::pair<SDL_FPoint, SDL_FPoint>>& walls);
    void render_lighting(const World& world, int map_layer);
};

#endif
