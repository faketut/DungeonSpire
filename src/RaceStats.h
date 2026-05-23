#ifndef RACE_STATS_H
#define RACE_STATS_H

// Data-driven race base-stat registry (Phase 3.9c).
//
// Mirrors EnemyStats / ItemStats: reads `data/races.json` (relative to CWD)
// on first use and caches the result. If the file is missing or malformed,
// falls back to built-in defaults that exactly match the original
// hardcoded setAttributes() switch in PlayerCharacter.

#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "Enum.h"
#include "third_party/json.hpp"

class RaceStats {
public:
    struct Stats {
        int maxHp;
        int atk;
        int def;
        double goldModifier;
    };

    static RaceStats* getInstance() {
        static RaceStats inst;
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
        std::unordered_map<Race, Stats> next;
        try {
            for (const auto& [key, value] : j.items()) {
                auto r = parseRace(key);
                if (!r.has_value()) continue; // skip _comment etc.
                if (!value.contains("maxHp") || !value.contains("atk") ||
                    !value.contains("def")   || !value.contains("goldModifier")) {
                    return false;
                }
                next[*r] = Stats{
                    value.at("maxHp").get<int>(),
                    value.at("atk").get<int>(),
                    value.at("def").get<int>(),
                    value.at("goldModifier").get<double>()
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

    // Returns stats for a race, throwing std::out_of_range if unknown.
    const Stats& get(Race r) const {
        auto it = table_.find(r);
        if (it == table_.end()) {
            throw std::out_of_range("RaceStats: no entry for requested Race");
        }
        return it->second;
    }

    bool isLoaded() const { return loaded_; }
    const std::string& source() const { return source_; }

    // Public so tests can build isolated instances. Production code should
    // prefer getInstance().
    RaceStats() {
        installDefaults();
        const char* candidates[] = {
            "data/races.json",
            "./data/races.json",
            "../data/races.json"
        };
        for (const char* p : candidates) {
            if (loadFromFile(p)) break;
        }
    }

private:
    void installDefaults() {
        table_[Race::HUMAN] = Stats{140, 20, 20, 1.0};
        table_[Race::DWARF] = Stats{100, 20, 30, 2.0};
        table_[Race::ELF]   = Stats{140, 30, 10, 1.0};
        table_[Race::ORC]   = Stats{180, 30, 25, 0.5};
        source_ = "<built-in defaults>";
    }

    static std::optional<Race> parseRace(const std::string& key) {
        if (key == "human") return Race::HUMAN;
        if (key == "dwarf") return Race::DWARF;
        if (key == "elf")   return Race::ELF;
        if (key == "orc")   return Race::ORC;
        return std::nullopt;
    }

    std::unordered_map<Race, Stats> table_;
    bool loaded_ = false;
    std::string source_;
};

#endif // RACE_STATS_H
