#ifndef FLOOR_STATS_H
#define FLOOR_STATS_H

// Data-driven floor-generation count registry (Phase 3.9d).
//
// Mirrors EnemyStats / ItemStats / RaceStats: reads `data/floor.json`
// (relative to CWD) on first use and caches the result. Falls back to
// built-in defaults matching the original const-globals in Board.h.

#include <fstream>
#include <string>

#include "third_party/json.hpp"

class FloorStats {
public:
    static FloorStats* getInstance() {
        static FloorStats inst;
        return &inst;
    }

    bool loadFromFile(const std::string& path) {
        std::ifstream in(path);
        if (!in) return false;
        nlohmann::json j;
        try {
            in >> j;
        } catch (const std::exception&) {
            return false;
        }
        try {
            if (!j.contains("potions") || !j.contains("gold") || !j.contains("enemies")) {
                return false;
            }
            potions_ = j.at("potions").get<int>();
            gold_    = j.at("gold").get<int>();
            enemies_ = j.at("enemies").get<int>();
        } catch (const std::exception&) {
            return false;
        }
        loaded_ = true;
        source_ = path;
        return true;
    }

    int potions() const { return potions_; }
    int gold()    const { return gold_; }
    int enemies() const { return enemies_; }

    bool isLoaded() const { return loaded_; }
    const std::string& source() const { return source_; }

    // Public so tests can build isolated instances. Production code should
    // prefer getInstance().
    FloorStats() {
        // Built-in defaults match the original Board.h const-globals.
        potions_ = 10;
        gold_    = 10;
        enemies_ = 20;
        source_  = "<built-in defaults>";
        const char* candidates[] = {
            "data/floor.json",
            "./data/floor.json",
            "../data/floor.json"
        };
        for (const char* p : candidates) {
            if (loadFromFile(p)) break;
        }
    }

private:
    int potions_ = 10;
    int gold_    = 10;
    int enemies_ = 20;
    bool loaded_ = false;
    std::string source_;
};

#endif // FLOOR_STATS_H
