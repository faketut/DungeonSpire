#ifndef ENEMY_STATS_H
#define ENEMY_STATS_H

// Data-driven enemy stat registry (Phase 2.6).
//
// Reads `data/enemies.json` (relative to CWD) on first use and caches the
// result. If the file is missing or malformed, falls back to a built-in
// table that exactly mirrors the historical hardcoded constants — so the
// game still runs in environments where the data file isn't shipped.
//
// All access is via the singleton `EnemyStats::getInstance()`.

#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "Enum.h"
#include "third_party/json.hpp"

class EnemyStats {
public:
    struct Stats {
        int hp;
        int atk;
        int def;
    };

    static EnemyStats* getInstance() {
        static EnemyStats inst;
        return &inst;
    }

    // Try to (re)load from a JSON file. Returns true on success. On failure
    // the existing in-memory table is left untouched (still usable).
    bool loadFromFile(const std::string& path) {
        std::ifstream in(path);
        if (!in) return false;
        nlohmann::json j;
        try {
            in >> j;
        } catch (const std::exception&) {
            return false;
        }
        std::unordered_map<Type, Stats> next;
        try {
            for (const auto& [key, value] : j.items()) {
                auto t = parseType(key);
                if (!t.has_value()) continue; // skip _comment etc.
                if (!value.contains("hp") || !value.contains("atk") || !value.contains("def")) {
                    return false;
                }
                next[*t] = Stats{
                    value.at("hp").get<int>(),
                    value.at("atk").get<int>(),
                    value.at("def").get<int>()
                };
            }
        } catch (const std::exception&) {
            return false;
        }
        if (next.empty()) return false;
        table_ = std::move(next);
        loaded_ = true;
        source_ = path;
        return true;
    }

    // Returns stats for a type, throwing std::out_of_range if unknown.
    const Stats& get(Type t) const {
        auto it = table_.find(t);
        if (it == table_.end()) {
            throw std::out_of_range("EnemyStats: no entry for requested Type");
        }
        return it->second;
    }

    bool isLoaded() const { return loaded_; }
    const std::string& source() const { return source_; }

    // Public so tests can build isolated instances. Production code should
    // prefer getInstance().
    EnemyStats() {
        installDefaults();
        // Best-effort load from the conventional locations. Failures are
        // silent because the defaults are already a complete table.
        const char* candidates[] = {
            "data/enemies.json",
            "./data/enemies.json",
            "../data/enemies.json"
        };
        for (const char* p : candidates) {
            if (loadFromFile(p)) break;
        }
    }

private:

    void installDefaults() {
        table_[Type::VAMPIRE]  = Stats{50,  25, 25};
        table_[Type::WEREWOLF] = Stats{120, 30,  5};
        table_[Type::TROLL]    = Stats{120, 25, 15};
        table_[Type::GOBLIN]   = Stats{70,   5, 10};
        table_[Type::MERCHANT] = Stats{30,  70,  5};
        table_[Type::DRAGON]   = Stats{150, 20, 20};
        table_[Type::PHOENIX]  = Stats{50,  35, 20};
        source_ = "<built-in defaults>";
    }

    static std::optional<Type> parseType(const std::string& key) {
        // Match the lowercased enum names used in the JSON file.
        if (key == "vampire")  return Type::VAMPIRE;
        if (key == "werewolf") return Type::WEREWOLF;
        if (key == "troll")    return Type::TROLL;
        if (key == "goblin")   return Type::GOBLIN;
        if (key == "merchant") return Type::MERCHANT;
        if (key == "dragon")   return Type::DRAGON;
        if (key == "phoenix")  return Type::PHOENIX;
        return std::nullopt;
    }

    std::unordered_map<Type, Stats> table_;
    bool loaded_ = false;
    std::string source_;
};

#endif // ENEMY_STATS_H
