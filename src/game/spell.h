#ifndef SPELL_H
#define SPELL_H

#include <string>
#include <vector>

struct Spell {
    std::string id, name, description;
    int mana_cost = 20;
    int min_level = 1;  // Req'd spellcraft to learn
    int failure_base = 50;  // % chance to fail cast (reduced by spellcraft)
    std::string effect_type;  // e.g., "heal", "damage", "detect"
    int effect_value = 10;  // e.g., heal amount
};

#endif
