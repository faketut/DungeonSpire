#ifndef QUEST_MANAGER_H
#define QUEST_MANAGER_H

#include <memory>
#include <vector>
#include "Quest.h"

class QuestManager {
private:
    std::vector<std::unique_ptr<Quest>> activeQuests;
    
    QuestManager() = default;
public:
    QuestManager(const QuestManager&) = delete;
    QuestManager& operator=(const QuestManager&) = delete;
    static QuestManager* getInstance() {
        static QuestManager inst;
        return &inst;
    }
    
    void addQuest(std::unique_ptr<Quest> quest) {
        activeQuests.push_back(std::move(quest));
    }
    
    void updateQuests(std::shared_ptr<PlayerCharacter> pc) {
        auto it = activeQuests.begin();
        while (it != activeQuests.end()) {
            if (!(*it)->isCompleted()) {
                (*it)->checkCompletion(pc);
                if ((*it)->isCompleted()) {
                    std::cout << "Quest Completed: " << (*it)->getDescription() << std::endl;
                    (*it)->giveReward(pc);
                    it = activeQuests.erase(it); // Remove completed quest
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }
    }
    
    void clearQuests() {
        activeQuests.clear();
    }
    
    const std::vector<std::unique_ptr<Quest>>& getActiveQuests() const {
        return activeQuests;
    }
};
#endif 
