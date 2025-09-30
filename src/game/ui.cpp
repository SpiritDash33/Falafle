#include "ui.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>  // For text; init in main if needed

void UI::render_menu(SDL_Renderer* renderer, const Actor& player, const Items& items_db) {
    // Existing inventory stub
    SDL_FRect rect = {100, 100, 200, 100};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_RenderFillRect(renderer, &rect);

    // New: Spell menu (e.g., on 'M' key)
    if (/* key == SDLK_m */) {
        // List known spells
        for (const auto& spell_id : player.known_spells) {
            // Render text stub (use SDL_ttf later)
            // e.g., TTF_RenderText_Solid(font, spell_name, color);
        }
        // Cast option: Select -> player.cast_spell(selected, target)
    }

    // Read action: From inventory, select book/scroll
    // Stub: If item in inventory, for each spell in get_spells_for_item(item_id):
    //   if (player.learn_spell(spell_db.get(spell_id))) { success msg }
    // For scrolls: Cast once, consume (remove from inventory)
}

void UI::render_spell_menu(SDL_Renderer* renderer, const Actor& player) {
    // Stub: Draw spell list rect
    SDL_FRect rect = {50, 50, 300, 400};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_RenderFillRect(renderer, &rect);
    // List player.known_spells with mana cost
}

void UI::render_inventory(SDL_Renderer* renderer, const Actor& player, const Items& items_db) {
    // Stub: Draw inventory rect
    SDL_FRect rect = {400, 50, 300, 400};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_RenderFillRect(renderer, &rect);
    // List player.inventory items with description/price
    for (const auto& item_id : player.inventory) {
        const auto* item = items_db.get(item_id);
        if (item) {
            // Render text stub: item->name + " - " + item->description
        }
    }
}
