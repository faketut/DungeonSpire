#include "SaveGame.h"

#include <fstream>
#include <sstream>

#include "third_party/json.hpp"

namespace cc3k {

using nlohmann::json;

namespace {

json playerToJson(const PlayerSnapshot& p) {
    json j;
    j["race"] = p.race;
    j["maxHp"] = p.maxHp;
    j["hp"] = p.hp;
    j["atk"] = p.atk;
    j["def"] = p.def;
    j["gold"] = p.gold;
    j["score"] = p.score;
    j["goldModifier"] = p.goldModifier;
    j["hasBarrierSuit"] = p.hasBarrierSuit;
    j["visibility"] = p.visibility;
    j["movementSpeed"] = p.movementSpeed;
    return j;
}

PlayerSnapshot playerFromJson(const json& j) {
    PlayerSnapshot p;
    j.at("race").get_to(p.race);
    j.at("maxHp").get_to(p.maxHp);
    j.at("hp").get_to(p.hp);
    j.at("atk").get_to(p.atk);
    j.at("def").get_to(p.def);
    j.at("gold").get_to(p.gold);
    j.at("score").get_to(p.score);
    j.at("goldModifier").get_to(p.goldModifier);
    j.at("hasBarrierSuit").get_to(p.hasBarrierSuit);
    j.at("visibility").get_to(p.visibility);
    j.at("movementSpeed").get_to(p.movementSpeed);
    return p;
}

} // namespace

std::string toJson(const SaveData& d) {
    json j;
    j["version"] = d.version;
    j["seed"] = d.seed;
    j["filename"] = d.filename;
    j["weatherEnabled"] = d.weatherEnabled;
    j["questEnabled"] = d.questEnabled;
    j["floorId"] = d.floorId;
    j["player"] = playerToJson(d.player);
    return j.dump(2);
}

std::optional<SaveData> fromJson(const std::string& s) {
    try {
        json j = json::parse(s);
        SaveData d;
        j.at("version").get_to(d.version);
        if (d.version != 1) return std::nullopt;
        j.at("seed").get_to(d.seed);
        j.at("filename").get_to(d.filename);
        j.at("weatherEnabled").get_to(d.weatherEnabled);
        j.at("questEnabled").get_to(d.questEnabled);
        j.at("floorId").get_to(d.floorId);
        d.player = playerFromJson(j.at("player"));
        return d;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

bool save(const std::string& path, const SaveData& d) {
    std::ofstream out(path);
    if (!out) return false;
    out << toJson(d);
    return out.good();
}

std::optional<SaveData> load(const std::string& path) {
    std::ifstream in(path);
    if (!in) return std::nullopt;
    std::stringstream ss;
    ss << in.rdbuf();
    return fromJson(ss.str());
}

} // namespace cc3k
