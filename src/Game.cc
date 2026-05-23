#ifndef GAME_CC
#define GAME_CC
#include <algorithm>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include "Game.h"
#include "AnsiRenderer.h"
#include "EffectManager.h"
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
Game::Game(int seed, std::string fn, bool wealtherEnabled, bool questEnabled,
           std::string savePath_, std::optional<cc3k::SaveData> pendingLoad_)
    : board(nullptr), state(GameState::RESTART), filename(fn),
      wealtherEnabled(wealtherEnabled), questEnabled(questEnabled),
      seedValue(static_cast<std::uint32_t>(seed)),
      savePath(std::move(savePath_)),
      pendingLoad(std::move(pendingLoad_)),
      renderer(std::make_unique<cc3k::AnsiRenderer>(std::cout)) {
    PRNG::seed(seedValue);
    if (questEnabled) {
        initializeQuests();
    }
}

void Game::writeSaveIfRequested() {
    if (savePath.empty() || !board) return;
    auto player = std::dynamic_pointer_cast<PlayerCharacter>(board->getPc()->getEntity());
    if (!player) return;
    cc3k::SaveData d;
    d.seed = seedValue;
    d.filename = filename;
    d.weatherEnabled = wealtherEnabled;
    d.questEnabled = questEnabled;
    d.floorId = board->getFloorId();
    d.player.race = static_cast<int>(player->getRace());
    d.player.maxHp = player->getMaxHp();
    d.player.hp = player->getHp();
    d.player.atk = player->getAtk();
    d.player.def = player->getDef();
    d.player.gold = player->getGold();
    d.player.score = player->getScore();
    d.player.goldModifier = player->getGoldModifier();
    d.player.hasBarrierSuit = player->hasSuit();
    d.player.visibility = player->getVisibility();
    d.player.movementSpeed = player->getMovementSpeed();
    if (cc3k::save(savePath, d)) {
        std::cout << "[cc3k] saved to " << savePath << std::endl;
    } else {
        std::cerr << "[cc3k] failed to write save file: " << savePath << std::endl;
    }
}

void Game::restart() {
    board=std::make_shared<Board>(filename,0); //filename is "files/potions.txt", ctor for board is Board(std::string filename = "./files/default.txt", int floorId = 0);
    board->setFloorId(0);
    board->setCompassPickedUp(false);
    board->setMaxFloorId(filename);
    if(filename!="./files/default.txt"){
        board->loadBoard(board->getFloorId(),filename,toTypeBoard);
        renderer->drawInitialBoard(*board);
        board->initByFile(board->getFloorId());
    }else{
        board->loadBoard(board->getFloorId(),filename,toTypeBoard);
        renderer->drawInitialBoard(*board);
        board->initFloor();
    }
    
    state = GameState::PLAYING;
    std::cout << "Game started." << std::endl;
    auto entity = board->getPc()->getEntity();
    auto player = std::dynamic_pointer_cast<PlayerCharacter>(entity);
    if (!player) {
        std::cerr << "Error: No valid player character found!" << std::endl;
        return;
    }

    if (pendingLoad) {
        // Restore player snapshot and replay floor advancement so the
        // dungeon is regenerated to the saved floor under the loaded seed.
        const auto& s = pendingLoad->player;
        player->applyLoadedState(static_cast<Race>(s.race),
                                 s.maxHp, s.hp, s.atk, s.def, s.gold, s.score,
                                 s.goldModifier, s.hasBarrierSuit,
                                 s.visibility, s.movementSpeed);
        board->getPc()->setEntity(player);
        std::cout << "[cc3k] loaded save: floor=" << (pendingLoad->floorId + 1)
                  << " race=" << toString(player->getRace())
                  << " gold=" << player->getGold() << std::endl;
        for (int i = 0; i < pendingLoad->floorId; ++i) {
            if (!board->nextFloor()) break;
        }
        pendingLoad.reset(); // only apply once; subsequent restarts are normal
        return;
    }

    // Player is asked to choose a race
    // Each race can be selected
    std::cout << "Choose your race h[Human]|e[Elf]|o[Orc]|d[Dwarf]: ";
    char c;
    std::cin >> c;
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
    if(!board) return;
    auto player = std::dynamic_pointer_cast<PlayerCharacter>(board->getPc()->getEntity());
    if(!player) return;
    cc3k::HudInfo hud;
    hud.race = toString(player->getRace());
    hud.gold = player->getGold();
    hud.floor = board->getFloorId() + 1;
    hud.hp = player->getHp();
    hud.atk = player->getAtk();
    hud.def = player->getDef();
    hud.action = dialog;
    hud.questEnabled = questEnabled;
    if (questEnabled) {
        const auto& q = QuestManager::getInstance()->getActiveQuests();
        hud.activeQuests.reserve(q.size());
        for (const auto& quest : q) hud.activeQuests.push_back(quest->getDescription());
    }
    hud.weatherEnabled = wealtherEnabled;
    if (wealtherEnabled) {
        hud.weather = EffectManager::getInstance()->getCurrentWeatherDescription();
        hud.movementSpeed = player->getMovementSpeed();
    }
    renderer->drawHud(hud);
}
void Game::renderBoard() {
    if(board) renderer->drawBoard(*board);
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
    // Write the save AFTER the main loop. The loop exits when state == QUIT
    // (the case above may never be reached because the while-condition is
    // checked before the switch on the iteration that flips state to QUIT).
    writeSaveIfRequested();
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
