#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>
#include "actor.h"
#include "items.h"

class UI {
public:
    void render_menu(SDL_Renderer* renderer, const Actor& player, const Items& items_db);
    // Stub for spell menu (on 'M')
    void render_spell_menu(SDL_Renderer* renderer, const Actor& player);
    // Stub for inventory (on 'I')
    void render_inventory(SDL_Renderer* renderer, const Actor& player, const Items& items_db);
};

#endif
