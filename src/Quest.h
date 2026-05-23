#ifndef QUEST_H
#define QUEST_H

#include <string>
#include <memory>
#include "Player.h"
#include "Item.h"

class Quest {
protected:
    std::string description;
    bool completed;
public:
    Quest(const std::string& desc) : description(desc), completed(false) {}
    virtual ~Quest() = default;
    virtual void checkCompletion(std::shared_ptr<PlayerCharacter> pc) = 0;
    virtual void giveReward(std::shared_ptr<PlayerCharacter> pc) = 0;
    bool isCompleted() const { return completed; }
    std::string getDescription() const { return description; }
};

class KillQuest : public Quest {
    Type targetType;
    int requiredCount;
public:
    KillQuest(Type target, int count, const std::string& desc)
        : Quest(desc), targetType(target), requiredCount(count) {}
    
    void checkCompletion(std::shared_ptr<PlayerCharacter> pc) override {
        if (pc->getKillCount(targetType) >= requiredCount) {
            completed = true;
        }
    }
    
    void giveReward(std::shared_ptr<PlayerCharacter> pc) override {
        auto reward = ItemGenerator::generateGold(Type::SMALL_HOARD);
        pc->setScore(reward->getValue());
        reward->getGold(pc);
    }
};

class CollectQuest : public Quest {
    Type itemType;
    int requiredCount;
public:
    CollectQuest(Type item, int count, const std::string& desc)
        : Quest(desc), itemType(item), requiredCount(count) {}
    
    void checkCompletion(std::shared_ptr<PlayerCharacter> pc) override {
        if (pc->getItemCount(itemType) >= requiredCount) {
            completed = true;
        }
    }
    
    void giveReward(std::shared_ptr<PlayerCharacter> pc) override {
        auto reward = ItemGenerator::generatePotion(Type::RH);
        reward->use(pc);
    }
};

#endif 
