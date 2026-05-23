#ifndef EFFECT_MANAGER_H
#define EFFECT_MANAGER_H
#include <memory>
#include <algorithm>
#include "ItemStats.h"
#include "Player.h"
#include "PRNG.h"
class Effect {
public:
    virtual void apply(std::shared_ptr<PlayerCharacter> pc) = 0;
    virtual void remove(std::shared_ptr<PlayerCharacter> pc) = 0;         
    virtual ~Effect() = default;
    virtual bool isTemporary() const = 0; 
};

// BoostAtk效果（临时）
class BoostAtkEffect : public Effect {
public:
    void apply(std::shared_ptr<PlayerCharacter> pc) override {pc->setAtk( ItemStats::getInstance()->getPotionDelta(Type::BA));}
    void remove(std::shared_ptr<PlayerCharacter> pc) override {pc->setAtk(-ItemStats::getInstance()->getPotionDelta(Type::BA));}
    bool isTemporary() const override { return true; }
};
class WoundAtkEffect : public Effect {
public:
    void apply(std::shared_ptr<PlayerCharacter> pc) override {pc->setAtk(-ItemStats::getInstance()->getPotionDelta(Type::WA));}
    void remove(std::shared_ptr<PlayerCharacter> pc) override {pc->setAtk( ItemStats::getInstance()->getPotionDelta(Type::WA));}
    bool isTemporary() const override { return true; }
};
class BoostDefEffect : public Effect {
public:
    void apply(std::shared_ptr<PlayerCharacter> pc) override {pc->setDef( ItemStats::getInstance()->getPotionDelta(Type::BD));}
    void remove(std::shared_ptr<PlayerCharacter> pc) override {pc->setDef(-ItemStats::getInstance()->getPotionDelta(Type::BD));}
    bool isTemporary() const override { return true; }
};
class WoundDefEffect : public Effect {
public:
    void apply(std::shared_ptr<PlayerCharacter> pc) override {pc->setDef(-ItemStats::getInstance()->getPotionDelta(Type::WD));}
    void remove(std::shared_ptr<PlayerCharacter> pc) override {pc->setDef( ItemStats::getInstance()->getPotionDelta(Type::WD));}
    bool isTemporary() const override { return true; }
};
class RestoreHealthEffect : public Effect {
public:
    void apply(std::shared_ptr<PlayerCharacter> pc) override {pc->setHp( ItemStats::getInstance()->getPotionDelta(Type::RH));}
    void remove(std::shared_ptr<PlayerCharacter> pc) override {pc->setHp(-ItemStats::getInstance()->getPotionDelta(Type::RH));}
    bool isTemporary() const override { return false; }
};
class PoisonHealthEffect : public Effect {
public:
    void apply(std::shared_ptr<PlayerCharacter> pc) override {
        const int d = ItemStats::getInstance()->getPotionDelta(Type::PH);
        if(pc->getRace()==Race::ELF)
            pc->setHp( d);
        else
            pc->setHp(-d);
    }
    void remove(std::shared_ptr<PlayerCharacter> pc) override {pc->setHp(ItemStats::getInstance()->getPotionDelta(Type::PH));}
    bool isTemporary() const override { return false; }
};

class RainEffect : public Effect {
public:
    void apply(std::shared_ptr<PlayerCharacter> pc) override {
        pc->setVisibility(5);
        pc->setMovementSpeed(2);
    }
    void remove(std::shared_ptr<PlayerCharacter> pc) override {
        pc->setVisibility(8);
        pc->setMovementSpeed(1);
    }
    bool isTemporary() const override { return true; }
};

class StormEffect : public Effect {
public:
    void apply(std::shared_ptr<PlayerCharacter> pc) override {
        pc->setVisibility(3);
        pc->setMovementSpeed(-1);
    }
    void remove(std::shared_ptr<PlayerCharacter> pc) override {
        pc->setVisibility(8);
        pc->setMovementSpeed(1);
    }
    bool isTemporary() const override { return true; }
};

class FogEffect : public Effect {
public:
    void apply(std::shared_ptr<PlayerCharacter> pc) override {
        pc->setVisibility(2);
    }
    void remove(std::shared_ptr<PlayerCharacter> pc) override {
        pc->setVisibility(8);
    }
    bool isTemporary() const override { return true; }
};

inline std::unique_ptr<Effect> generateWeather() {
    int random=PRNG::randInt(3);
    switch(random) {
        case 0: return std::make_unique<RainEffect>();
        case 1: return std::make_unique<StormEffect>();
        case 2: return std::make_unique<FogEffect>();
        default: return nullptr;
    }
}
// EffectManager（单例）
class EffectManager {
private:
    std::vector<std::unique_ptr<Effect>> effects;
    std::vector<std::unique_ptr<Effect>> weatherEffects;

    // 私有构造函数，防止外部实例化
    EffectManager() = default;
    std::string getCurrentWeatherDescription(const Effect* effect) const {
        if (dynamic_cast<const RainEffect*>(effect)) {
            return "A chilling rain falls from the cavern's ceiling, making the stone floor slick with moisture.";
        } else if (dynamic_cast<const StormEffect*>(effect)) {
            return "Thunder shakes the very walls of the dungeon, and lightning briefly illuminates forgotten secrets.";
        } else if (dynamic_cast<const FogEffect*>(effect)) {
            return "Thick fog rolls in, obscuring your vision and muffling all sounds. Beware of lurking dangers!";
        }
        return "";
    }

    // 随机冒险提示
    std::string getRandomAdventureTip() const {
        static const std::vector<std::string> adventureTips = {
            "Listen carefully, you might hear the footsteps of unseen foes approaching.",
            "Perhaps now is the time to explore that mysterious passage you passed earlier.",
            "Stay alert, the walls themselves may shift, revealing new paths or closing old ones.",
            "The spirits of this place are watching. Show them respect, and they may offer aid.",
            "In the depths of the dungeon, even the smallest light can be a beacon of hope.",
            "Remember, sometimes the greatest danger lies not in what you see, but in what you don't."
        };

        // 使用随机数选择提示
        return adventureTips[PRNG::randInt(static_cast<int>(adventureTips.size()))];
    }
public:
    EffectManager(const EffectManager&) = delete;
    EffectManager& operator=(const EffectManager&) = delete;
    int getWeatherEffectsCnt() {return weatherEffects.size();}
    // 提供全局访问点
    static EffectManager* getInstance() {
        static EffectManager inst;
        return &inst;
    }
    void addWeatherEffect(std::unique_ptr<Effect> effect) {
        weatherEffects.push_back(std::move(effect));
    }
    void addEffect(std::unique_ptr<Effect> effect) {
        effects.push_back(std::move(effect));
    }
    void clearWealtherEffects(){
        weatherEffects.clear();
    }
    void clearTemporaryEffects(std::shared_ptr<PlayerCharacter> pc) {
        effects.erase(std::remove_if(effects.begin(), effects.end(),
            [&pc](const auto& e) {
                if (e->isTemporary()) {
                    e->remove(pc);
                    return true;
                }
                return false;
            }),
            effects.end());
    }
    // Add new method to get current weather description
    std::string getCurrentWeatherDescription() const {
        if (weatherEffects.empty()) {
            return "The air is still, and the only sound is your own breath echoing through the empty halls.";
        }

        std::string description;
        for (const auto& effect : weatherEffects) {
            description += getCurrentWeatherDescription(effect.get()) + " ";
        }
        // Add random adventure tip
        description += "\n" + getRandomAdventureTip();

        return description;
    }
};


#endif
