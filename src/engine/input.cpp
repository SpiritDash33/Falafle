#include "input.h"

void Input::handle_event(const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
        keys[event.key.key] = true;
    } else if (event.type == SDL_EVENT_KEY_UP) {
        keys[event.key.key] = false;
    }
}

bool Input::is_key_down(SDL_Keycode key) const {
    auto it = keys.find(key);
    return it != keys.end() && it->second;
}
