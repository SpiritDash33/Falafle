#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL.h>
#include <map>

class Input {
private:
    std::map<SDL_Keycode, bool> keys;

public:
    void handle_event(const SDL_Event& event);
    bool is_key_down(SDL_Keycode key) const;
};

#endif
