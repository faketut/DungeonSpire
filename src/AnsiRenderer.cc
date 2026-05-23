#include "AnsiRenderer.h"

#include <iostream>

#include "Board.h"
#include "PRNG.h"

namespace cc3k {

AnsiRenderer::AnsiRenderer(std::ostream& out) : out_(out) {}

void AnsiRenderer::drawInitialBoard(const Board& board) {
    // Full grid dump with no fog of war. Matches the original
    // Board::displayBoard() except for its legacy hardcoded HUD line — the
    // proper HUD is now rendered by drawHud() right after this call, using
    // real player values.
    const bool compass = board.isCompassPickedUp();
    for (const auto& row : board.getTiles()) {
        for (const auto& tile : row) {
            out_ << toChar(tile->getType(), compass);
        }
        out_ << std::endl;
    }
}

void AnsiRenderer::drawBoard(const Board& board) {
    // Fog-of-war render centred on the player. Matches the original
    // Board::printBoard() exactly, including the edge-of-vision flicker
    // (50% chance to drop the outermost ring on each frame).
    auto pc = board.getPc();
    if (!pc) return;
    auto pcPos = pc->getPosition();
    auto player = std::dynamic_pointer_cast<PlayerCharacter>(pc->getEntity());
    int visibility = player ? player->getVisibility() : 8;
    const bool compass = board.isCompassPickedUp();

    out_ << std::endl;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            Position pos(x, y);
            auto dist = pos.distanceTo(pcPos);
            if (dist <= visibility) {
                if (dist == visibility && PRNG::randInt(2)) {
                    out_ << " ";
                } else if (pcPos == pos) {
                    out_ << "\033[34m" << '@';
                } else {
                    auto tile = board.getTile(pos);
                    out_ << getColor(tile->getType(), compass)
                         << toChar(tile->getType(), compass);
                }
            } else {
                out_ << " ";
            }
            out_ << RESET;
        }
        out_ << std::endl;
    }
}

void AnsiRenderer::drawHud(const HudInfo& info) {
    out_ << "Race: " << info.race
         << " Gold: " << info.gold
         << "\t\t\t\t\t\t\tFloor " << info.floor << std::endl;
    out_ << "HP: " << info.hp << std::endl
         << "Atk: " << info.atk << std::endl
         << "Def: " << info.def << std::endl
         << "Action: " << info.action << std::endl;

    if (info.questEnabled && !info.activeQuests.empty()) {
        out_ << "\nActive Quests:\n";
        for (const auto& q : info.activeQuests) {
            out_ << "- " << q << "\n";
        }
    }

    if (info.weatherEnabled) {
        out_ << std::endl
             << info.weather << std::endl
             << "Speed: " << info.movementSpeed << std::endl;
    }
}

} // namespace cc3k
