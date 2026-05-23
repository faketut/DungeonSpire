#ifndef QUEST_MANAGER_CC
#define QUEST_MANAGER_CC

#include <memory>
#include <vector>
#include "Quest.h"

class QuestManager {
private:
    static std::shared_ptr<QuestManager> instance;
    static std::once_flag initFlag;
    std::vector<std::unique_ptr<Quest>> activeQuests;
    
    QuestManager() = default;
public:
    static std::shared_ptr<QuestManager> getInstance() {
        std::call_once(initFlag, [](){
            instance.reset(new QuestManager());
        });
        return instance;
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
inline std::shared_ptr<QuestManager> QuestManager::instance = nullptr;
inline std::once_flag QuestManager::initFlag;
#endif 
