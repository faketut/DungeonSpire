#ifndef CC3K_RENDERER_H
#define CC3K_RENDERER_H

#include <string>
#include <vector>

class Board;

namespace cc3k {

// Snapshot of everything the status panel below the board needs to display.
// Built once per turn by Game; consumed once per turn by the renderer.
struct HudInfo {
    std::string race;
    int floor = 1;
    int gold = 0;
    int hp = 0;
    int atk = 0;
    int def = 0;
    std::string action;
    bool questEnabled = false;
    std::vector<std::string> activeQuests;
    bool weatherEnabled = false;
    std::string weather;
    int movementSpeed = 1;
};

// Frontend-agnostic rendering interface. The Ansi implementation keeps the
// existing cout+ANSI-colour behaviour; future backends (ncurses, SDL2, WASM)
// implement the same three calls without Game needing to know.
class IRenderer {
public:
    virtual ~IRenderer() = default;
    // Full board with no fog of war. Used once per floor when it loads.
    virtual void drawInitialBoard(const Board& board) = 0;
    // Per-turn render with fog of war centred on the player.
    virtual void drawBoard(const Board& board) = 0;
    // Status panel below the board.
    virtual void drawHud(const HudInfo& info) = 0;
};

} // namespace cc3k

#endif
