#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include <unordered_set>
#include <vector>
#include "spell.h"

class Actor {
public:
    int x = 0, y = 0, current_map_layer = 1;  // Z-level
    int height = 1;  // Actor height for blocking
    int health = 100, max_health = 100;
    int mana = 100, max_mana = 100;
    int spellcraft = 0;  // Skill 0-10; higher = better learning/casting
    int intellect = 10;  // Stat for learning checks (Moria-inspired)
    std::string tile_id = "player";
    std::unordered_set<std::string> known_spells;  // Learned spell IDs
    std::vector<std::string> inventory;  // Item IDs (e.g., books)

    void move(int dx, int dy);
    void move_z(int dz);  // Shift layer
    bool learn_spell(const Spell& spell);  // Returns success
    bool cast_spell(const std::string& spell_id, Actor& target);  // Returns success; affects target
    void regen_mana(int turns);  // Regens over time
};

#endif
