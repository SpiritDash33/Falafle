#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "engine/renderer.h"
#include "engine/input.h"
#include "game/world.h"
#include "game/items.h"
#include "engine/utils/log.h"

// Forward declare Input class for world.update
class Input;

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Window* window = SDL_CreateWindow("Cataclysm RPG", 800, 600, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    Renderer engine_renderer(renderer);
    Input input;
    World world;
    Items items_db;

    world.load_tiles("assets/data/tilesets.json");
    world.load_map("assets/data/map.json");
    items_db.load_from_json("assets/data/items.json");

    Actor& player = world.get_actors().emplace_back();
    player.x = 25; player.y = 25; player.current_map_layer = 1;

    bool running = true;
    float dt = 1.0f / 60.0f;
    float time = 0.0f;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            input.handle_event(event);
        }

        world.update(input);
        world.update_time(dt);
        engine_renderer.update_camera(player.x, player.y);
        engine_renderer.render_world(world, player.current_map_layer, time);

        time += dt;
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
