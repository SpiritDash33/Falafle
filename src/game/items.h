#ifndef ITEMS_H
#define ITEMS_H

#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "spell.h"  // For contained_spells

struct Item {
    std::string id, name, category, description, durability;
    int price = 0;
    int hunger_restoration = 0;  // Consumables only
    std::vector<std::string> contained_spells;  // Spell IDs for books/scrolls (e.g., {"detect_monsters"})
};

class Items {
private:
    std::vector<Item> items;

public:
    void load_from_json(const std::string& path);
    const Item* get(const std::string& id) const;
    std::vector<Item> get_by_category(const std::string& cat) const;
    const std::vector<std::string>& get_spells_for_item(const std::string& item_id) const;  // For reading
};

#endif
