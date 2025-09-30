#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include <unordered_map>
#include <vector>
#include "../game/world.h"
#include "particles.h"
#include "lighting.h"

struct Renderable {
    SDL_Texture* texture;
    SDL_FRect dst;
    float depth;
    int z_offset = 0;
};

class Renderer {
private:
    SDL_Renderer* sdl_renderer;
    int tile_w = 64, tile_h = 32;
    SDL_Point camera = {0, 0};
    Lighting lighting;
    std::unordered_map<std::string, SDL_Texture*> texture_cache;

public:
    Renderer(SDL_Renderer* r) : sdl_renderer(r), lighting(r) {}
    SDL_FPoint grid_to_iso(int x, int y);
    std::pair<int, int> screen_to_grid(float mx, float my);
    void update_camera(int px, int py);
    void render_layer(const World& world, int map_layer, float time);
    void render_world(const World& world, int player_layer, float time);
    void add_light(const Light& light) { lighting.add_source(light); }
    SDL_Texture* load_texture(const std::string& path);
};

#endif
