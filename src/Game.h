#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include "Board.h"
#include "Enum.h"
#include "Enemy.h"

class Game {
    std::shared_ptr<Board> board;
    GameState state;
    std::string filename;
    std::string dialog;
    bool wealtherEnabled;
    bool questEnabled;
    int loopCounter = 0;
public:
    Game(int seed,std::string filename,bool wealtherEnabled,bool questEnabled);
    bool gameOver();
    void processCommand();
    void run();
    void renderBoard();
    void renderInfo();
    void victory();
    void defeat();
    void quit();
    void restart();
    void initializeQuests();
    void updateQuests();
};

#endif 
