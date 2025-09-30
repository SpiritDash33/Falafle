#include "items.h"
#include <fstream>
#include <algorithm>

void Items::load_from_json(const std::string& path) {
    std::ifstream f(path);
    nlohmann::json j;
    j << f;

    items.clear();
    for (const auto& entry : j["items"]) {
        Item i;
        i.id = entry["id"];
        i.name = entry["name"];
        i.category = entry["category"];
        i.description = entry.value("description", "No description available.");
        i.durability = entry.value("durability", "N/A");
        i.price = entry.value("price", 0);

        if (i.category == "consumables") {
            i.hunger_restoration = entry.value("hunger_restoration", 0);
        }

        // Parse contained_spells for books/scrolls
        if (entry.contains("contained_spells") && (i.category == "books" || i.category == "scrolls")) {
            for (const auto& spell : entry["contained_spells"]) {
                i.contained_spells.push_back(spell);
            }
        }

        items.push_back(i);
    }
}

const Item* Items::get(const std::string& id) const {
    for (const auto& item : items) {
        if (item.id == id) return &item;
    }
    return nullptr;
}

std::vector<Item> Items::get_by_category(const std::string& cat) const {
    std::vector<Item> filtered;
    std::copy_if(items.begin(), items.end(), std::back_inserter(filtered),
                 [cat](const Item& i) { return i.category == cat; });
    return filtered;
}

const std::vector<std::string>& Items::get_spells_for_item(const std::string& item_id) const {
    static const std::vector<std::string> empty;
    const Item* item = get(item_id);
    if (item) return item->contained_spells;
    return empty;
}
