#ifndef PLAYER_H
#define PLAYER_H
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include "Enum.h"
#include "Entity.h"
#include "RaceStats.h"
#include <unordered_map>

class PlayerCharacter : public Entity {
protected:
    Race race;
    int maxHp, hp, atk, def, gold,score;
    double goldModifier;
    bool hasBarrierSuit;
    int visibility;
    int movementSpeed;
    std::unordered_map<Type, int> killCounts;
    std::unordered_map<Type, int> itemCounts;

    static constexpr int DEFAULT_MAX_HP = 140;
    static constexpr int DEFAULT_HP = 140;
    static constexpr int DEFAULT_ATK = 20;
    static constexpr int DEFAULT_DEF = 20;
    static constexpr double DEFAULT_GOLD_MODIFIER = 1.0;
    static constexpr double DEFAULT_VISIBILITY = 100;


public:
    PlayerCharacter(Race r = Race::HUMAN) : race(r), maxHp(DEFAULT_MAX_HP), 
        hp(DEFAULT_HP), atk(DEFAULT_ATK), def(DEFAULT_DEF), gold(0), 
        goldModifier(DEFAULT_GOLD_MODIFIER), hasBarrierSuit(false), visibility(DEFAULT_VISIBILITY), movementSpeed(1) {}

    // Sets attributes based on race
    void setAttributes(Race r) {
        // Pull base stats from the data-driven registry. Throws if the race
        // is unknown (preserving the historical invalid-race contract).
        const auto& s = RaceStats::getInstance()->get(r);
        maxHp = s.maxHp;
        hp = s.maxHp;
        atk = s.atk;
        def = s.def;
        goldModifier = s.goldModifier;
        race = r;
    }
    void setHp(int delta){
        hp+=delta;
        if(hp>maxHp)hp=maxHp;
        if(hp<0)hp=0;
    }
    void setAtk(int delta) {
        atk += delta;
        if (atk < 0) atk = 0;
    }
    void setDef(int delta){
        def+=delta;
        if(def<0)def=0;
    }
    void setGold(int delta) {
        gold+=delta*goldModifier;
        if(gold<0)gold=0;
    }
    void setScore(int delta){
        score+=delta;
    }
    void setBarrierSuit() {
        hasBarrierSuit=true;
    }
    Race getRace() const {return race;}
    bool hasSuit() {return hasBarrierSuit;}
    bool isAlive(){return hp>0;}
    std::map<std::string, int> getInfo() const {
        std::map<std::string, int> info;
        info["HP"] = hp;
        info["Atk"] = atk;
        info["Def"] = def;
        info["Gold"] = gold;
        return info;
    }
    int getHp()const{return hp;}
    int getMaxHp()const{return maxHp;}
    int getAtk()const{return atk;}
    int getDef()const{return def;}
    int getGold()const{return gold;}
    int getScore()const{return score;}
    double getGoldModifier()const{return goldModifier;}
    int getVisibility() const{return visibility;}
    // Restore every cross-floor mutable field from a save snapshot.
    // Kept as a single call to keep load logic out of Game::restart.
    void applyLoadedState(Race r, int mh, int h, int a, int d, int g, int sc,
                          double gm, bool suit, int vis, int spd) {
        race = r;
        maxHp = mh;
        hp = h;
        atk = a;
        def = d;
        gold = g;
        score = sc;
        goldModifier = gm;
        hasBarrierSuit = suit;
        visibility = vis;
        movementSpeed = spd;
    }
    void setVisibility(int mod) { visibility = mod; }
    int getMovementSpeed()const{return movementSpeed;}
    void setMovementSpeed(int mod) { movementSpeed = mod; }
    void incrementKillCount(Type enemyType) {
        killCounts[enemyType]++;
    }
    
    int getKillCount(Type enemyType) const {
        auto it = killCounts.find(enemyType);
        return it != killCounts.end() ? it->second : 0;
    }
    
    void incrementItemCount(Type itemType) {
        itemCounts[itemType]++;
    }
    
    int getItemCount(Type itemType) const {
        auto it = itemCounts.find(itemType);
        return it != itemCounts.end() ? it->second : 0;
    }
};
#endif
