#ifndef ITEM_STATS_H
#define ITEM_STATS_H

// Data-driven item stat registry (Phase 3.9).
//
// Mirrors the EnemyStats pattern: reads `data/items.json` (relative to CWD)
// on first use and caches the result. If the file is missing or malformed,
// falls back to a built-in table matching the original hardcoded values.
//
// Currently exposes gold pile values; extend by adding new sections to the
// JSON file and matching helpers here.

#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "Enum.h"
#include "third_party/json.hpp"

class ItemStats {
public:
    static ItemStats* getInstance() {
        static ItemStats inst;
        return &inst;
    }

    // Try to (re)load from a JSON file. Returns true on success. On failure
    // the existing in-memory table is left untouched.
    bool loadFromFile(const std::string& path) {
        std::ifstream in(path);
        if (!in) return false;
        nlohmann::json j;
        try {
            in >> j;
        } catch (const std::exception&) {
            return false;
        }
        std::unordered_map<Type, int> next;
        try {
            if (!j.contains("gold") || !j.at("gold").is_object()) return false;
            for (const auto& [key, value] : j.at("gold").items()) {
                auto t = parseGoldType(key);
                if (!t.has_value()) continue;
                next[*t] = value.get<int>();
            }
        } catch (const std::exception&) {
            return false;
        }
        if (next.empty()) return false;
        gold_ = std::move(next);
        loaded_ = true;
        source_ = path;
        return true;
    }

    // Returns gold value for a Type, throwing std::out_of_range if unknown.
    int getGoldValue(Type t) const {
        auto it = gold_.find(t);
        if (it == gold_.end()) {
            throw std::out_of_range("ItemStats: no gold entry for requested Type");
        }
        return it->second;
    }

    bool isLoaded() const { return loaded_; }
    const std::string& source() const { return source_; }

    // Public so tests can build isolated instances. Production code should
    // prefer getInstance().
    ItemStats() {
        installDefaults();
        const char* candidates[] = {
            "data/items.json",
            "./data/items.json",
            "../data/items.json"
        };
        for (const char* p : candidates) {
            if (loadFromFile(p)) break;
        }
    }

private:
    void installDefaults() {
        gold_[Type::NORMAL_GOLD_PILE] = 1;
        gold_[Type::SMALL_HOARD]      = 2;
        gold_[Type::MERCHANT_HOARD]   = 4;
        gold_[Type::DRAGON_HOARD]     = 6;
        source_ = "<built-in defaults>";
    }

    static std::optional<Type> parseGoldType(const std::string& key) {
        if (key == "normal_gold_pile") return Type::NORMAL_GOLD_PILE;
        if (key == "small_hoard")      return Type::SMALL_HOARD;
        if (key == "merchant_hoard")   return Type::MERCHANT_HOARD;
        if (key == "dragon_hoard")     return Type::DRAGON_HOARD;
        return std::nullopt;
    }

    std::unordered_map<Type, int> gold_;
    bool loaded_ = false;
    std::string source_;
};

#endif // ITEM_STATS_H
