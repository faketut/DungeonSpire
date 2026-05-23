#ifndef GAME_CC
#define GAME_CC
#include <algorithm>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include "Game.h"
#include "Quest.h"
#include "QuestManager.h"

void Game::processCommand() {
    std::cout << "Enter a command: ";
    std::string command;
    std::getline(std::cin, command);
    dialog.clear();

    if (command == "r") { 
        state = GameState::RESTART;
        return;
    } 
    if (command == "q") { 
        state = GameState::QUIT;
        return;
    }
    std::istringstream iss(command);
    std::string action, dirStr;
    iss >> action >> dirStr;

    if (moveCommands.count(command)) {  
        Direction dir=toDirection(command);
        board->movePc(dir);
    } 
    else if (action == "u" && moveCommands.count(dirStr)) {  
        auto dir=toDirection(dirStr);
        board->pickUp(dir);
        
    } 
    else if (action == "a" && moveCommands.count(dirStr)) {  
        auto pcTile = board->getPc();
        auto dir = toDirection(dirStr);
        auto pos = board->convertDirection(*pcTile, dir);
        auto enemyTile = board->getTile(pos);
        if (enemyTile && TypeCategories::isEnemy(enemyTile->getType())) {
            auto enemy = std::dynamic_pointer_cast<Enemy>(enemyTile->getEntity());
            if (enemy) board->attackEnemy(*enemyTile);
        }
    } 
    else {
        std::cout << "Invalid command!" << std::endl;
    }
    board->updateEnemies();  
    dialog=board->diaHelp();
}

void Game::quit() {
    std::cout << "Goodbye!" << std::endl;
    return;
}
// prompt the player to quit or select a race
void Game::victory() {
    auto player=std::dynamic_pointer_cast<PlayerCharacter>(board->getPc()->getEntity());
    if (!player) { state = GameState::QUIT; return; }
    int gold = player->getInfo().find("Gold")->second;
    double score = gold*2;
    if(player->getRace()==Race::HUMAN) score*=1.5; 
    std::cout << "Congratulations! You have reached the top." << std::endl;
    std::cout << "Score: " << score << " Gold: "<< gold << std::endl;
    std::cout << "Would you like to play again? (y/n)" << std::endl;
    std::string command;
    std::getline(std::cin, command);
    if (command == "y") {
        state = GameState::RESTART;
    } else {
        state = GameState::QUIT;
    }
}
// prompt the player to quit or select a race
void Game::defeat() {
    std::cout << "Game over! You died." << std::endl;
    std::cout << "Would you like to play again? (y/n)" << std::endl;
    std::string command;
    std::getline(std::cin, command);
    if (command == "y") {
        state = GameState::RESTART;
    } else {
        state = GameState::QUIT;
    }
}
bool Game::gameOver() {
    // 游戏结束的逻辑
    auto player=std::dynamic_pointer_cast<PlayerCharacter>(board->getPc()->getEntity());
    if (!player) return true;
    if (!player->isAlive()) {
        state = GameState::DEFEAT;
        return true;
    }else if (filename!="./files/default.txt"&&board->getFloorId()==board->getMaxFloorId()) {
        state = GameState::VICTORY;
        return true;
    }else if(filename=="./files/default.txt"&&board->getFloorId()==5){
        state = GameState::VICTORY;
        return true;
    }else
        return false;
}
Game::Game(int seed, std::string fn, bool wealtherEnabled, bool questEnabled) : board(nullptr), state(GameState::RESTART), filename(fn),wealtherEnabled(wealtherEnabled), questEnabled(questEnabled) {
    PRNG::seed(static_cast<std::uint32_t>(seed));
    if (questEnabled) {
        initializeQuests();
    }
}

void Game::restart() {
    board=std::make_shared<Board>(filename,0); //filename is "files/potions.txt", ctor for board is Board(std::string filename = "./files/default.txt", int floorId = 0);
    board->setFloorId(0);
    board->setCompassPickedUp(false);
    board->setMaxFloorId(filename);
    if(filename!="./files/default.txt"){
        board->loadBoard(board->getFloorId(),filename,toTypeBoard);
        board->displayBoard();
        board->initByFile(board->getFloorId());
    }else{
        board->loadBoard(board->getFloorId(),filename,toTypeBoard);
        board->displayBoard();
        board->initFloor();
    }
    
    state = GameState::PLAYING;
    std::cout << "Game started." << std::endl;
    // Player is asked to choose a race 
    // Each race can be selected
    std::cout << "Choose your race h[Human]|e[Elf]|o[Orc]|d[Dwarf]: ";
    char c;
    std::cin >> c;
    auto entity = board->getPc()->getEntity();
    auto player = std::dynamic_pointer_cast<PlayerCharacter>(entity);
    
    if (!player) {
        std::cerr << "Error: No valid player character found!" << std::endl;
        return; // 或者采取其他适当的措施
    }    
    while(true){
        switch(c) {
            case 'h': player->setAttributes(Race::HUMAN);break;
            case 'e': player->setAttributes(Race::ELF);break;
            case 'o': player->setAttributes(Race::ORC);break;
            case 'd': player->setAttributes(Race::DWARF);break;
            default: 
                std::cout << "Only h[Human]|e[Elf]|o[Orc]|d[Dwarf]: ";
                std::cin >> c; // Prompt for new input
                continue; // Skip the rest and restart the loop
        }
        break;
    }
    if(wealtherEnabled) player->setVisibility(8);
    board->getPc()->setEntity(player);
}

void Game::renderInfo() {
    if(board) {
        auto player = std::dynamic_pointer_cast<PlayerCharacter>(board->getPc()->getEntity());
        if(player) {
            std::string race = toString(player->getRace());
            std::cout << "Race: " <<  race << 
            " Gold: " << player->getGold() <<
            "\t\t\t\t\t\t\tFloor " << board->getFloorId()+1 << std::endl; 
            std::cout << "HP: " << player->getHp() << std::endl <<
            "Atk: " << player->getAtk() << std::endl <<
            "Def: " << player->getDef() << std::endl << 
            "Action: " << dialog << std::endl;
            
            // Display active quests if enabled
            if (questEnabled) {
                auto questManager = QuestManager::getInstance();
                const auto& activeQuests = questManager->getActiveQuests();
                
                if (!activeQuests.empty()) {
                    std::cout << "\nActive Quests:\n";
                    for (const auto& quest : activeQuests) {
                        std::cout << "- " << quest->getDescription() << "\n";
                    }
                }
            }
            // Display weather if enabled
            if (wealtherEnabled) {
                std::cout << std::endl << 
                    EffectManager::getInstance()->getCurrentWeatherDescription() << 
                    std::endl <<
                    "Speed: " <<player->getMovementSpeed() <<std::endl;
            }
            
        }
    }
}
void Game::renderBoard() {
    // 渲染或显示当前棋盘状态的逻辑
    if(board) board->printBoard(); // 假设 Board 类有一个 print 方法用于打印棋盘
}
void Game::run() {

    while (state!=QUIT) {
        switch (state) {
            case GameState::PLAYING:
                if(wealtherEnabled){
                    ++loopCounter;
                    std::cout<<loopCounter<<std::endl;
                    if(loopCounter%5==0) {
                        auto weatherManager=EffectManager::getInstance();
                        if(PRNG::randInt(2) || weatherManager->getWeatherEffectsCnt()>1) EffectManager::getInstance()->clearWealtherEffects();
                        auto weather = generateWeather();
                        auto player=std::dynamic_pointer_cast<PlayerCharacter>(board->getPc()->getEntity());
                        weather->apply(player);
                        EffectManager::getInstance()->addWeatherEffect(std::move(weather));
                    }
                }
                processCommand();
                if (gameOver()) {
                    break;
                }
                renderBoard();
                renderInfo();
                updateQuests();
                break;

            case GameState::VICTORY:
                victory();
                break;

            case GameState::DEFEAT:
                defeat();
                break;

            case GameState::RESTART:
                restart();
                break;

            case GameState::QUIT:
                quit();
                break;

            default:
                std::cerr << "Unknown game state!" << std::endl;
                break;
        }
    }
}

void Game::initializeQuests() {
    auto questManager = QuestManager::getInstance();
    
    // Add a kill quest for goblins
    questManager->addQuest(std::make_unique<KillQuest>(
        Type::GOBLIN, 
        5, 
        "Defeat 5 Goblins (Reward: Small Gold Hoard)"
    ));
    
    // Add a collect quest for health potions
    questManager->addQuest(std::make_unique<CollectQuest>(
        Type::RH, 
        1, 
        "Collect 1 Health Potion (Reward: Restore Health Potion)"
    ));
    questManager->addQuest(std::make_unique<CollectQuest>(
        Type::PH, 
        1, 
        "Collect 1 Poison Potion (Reward: Restore Health Potion)"
    ));
    // Add a kill quest for vampires
    questManager->addQuest(std::make_unique<KillQuest>(
        Type::VAMPIRE, 
        1, 
        "Defeat 1 Vampire (Reward: Small Gold Hoard)"
    ));
    
    // Add a collect quest for barrier suits
    questManager->addQuest(std::make_unique<CollectQuest>(
        Type::BARRIER_SUIT, 
        1, 
        "Find the Barrier Suit (Reward: Restore Health Potion)"
    ));
}

void Game::updateQuests() {
    if (questEnabled) {
        auto player = std::dynamic_pointer_cast<PlayerCharacter>(board->getPc()->getEntity());
        QuestManager::getInstance()->updateQuests(player);
    }
}
#endif
