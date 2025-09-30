#include "actor.h"
#include <random>
#include <algorithm>

void Actor::move(int dx, int dy) {
    x += dx;
    y += dy;
    // Bounds check stub
}

void Actor::move_z(int dz) {
    current_map_layer = std::max(0, std::min(5, current_map_layer + dz));
    // Camera follow stub
}

bool Actor::learn_spell(const Spell& spell) {
    if (spellcraft < spell.min_level) return false;  // Skill check
    int learn_chance = 50 + (intellect - 10) * 5 + spellcraft * 10;  // Base 50% + mods
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    if (dis(gen) <= learn_chance) {
        known_spells.insert(spell.id);
        return true;
    }
    return false;  // Fail; retry possible
}

bool Actor::cast_spell(const std::string& spell_id, Actor& target) {
    if (known_spells.find(spell_id) == known_spells.end()) return false;
    if (mana < 20) return false;  // Min cost stub

    // Find spell (stub: assume global Spells db or from items)
    Spell spell;  // Placeholder; load from data
    spell.mana_cost = 20;
    spell.failure_base = 50;

    mana -= spell.mana_cost;
    int fail_chance = std::max(0, spell.failure_base - spellcraft * 5);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    if (dis(gen) <= fail_chance) return false;  // Fizzle

    // Effect stub
    if (spell.effect_type == "heal") {
        target.health = std::min(target.max_health, target.health + spell.effect_value);
    } else if (spell.effect_type == "damage") {
        target.health -= spell.effect_value;
    }
    // Add particles/sound via engine

    return true;
}

void Actor::regen_mana(int turns) {
    mana = std::min(max_mana, mana + turns * 10);
}
