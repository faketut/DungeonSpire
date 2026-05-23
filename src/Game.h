#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <optional>
#include "Board.h"
#include "Enum.h"
#include "Enemy.h"
#include "Renderer.h"
#include "SaveGame.h"

class Game {
    std::shared_ptr<Board> board;
    GameState state;
    std::string filename;
    std::string dialog;
    bool wealtherEnabled;
    bool questEnabled;
    int loopCounter = 0;
    std::uint32_t seedValue;             // original seed, echoed into saves
    std::string savePath;                // empty = no save-on-quit
    std::optional<cc3k::SaveData> pendingLoad;
    std::unique_ptr<cc3k::IRenderer> renderer;
public:
    Game(int seed, std::string filename, bool wealtherEnabled, bool questEnabled,
         std::string savePath = std::string{},
         std::optional<cc3k::SaveData> pendingLoad = std::nullopt);
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
    void writeSaveIfRequested();
};

#endif
