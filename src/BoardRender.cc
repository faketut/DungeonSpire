#include "Board.h"
#include "Quest.h"
#include "QuestManager.h"
#include <iostream>
#include <sstream>
#include <cmath>

void Board::displayBoard() const {
    for (const auto& row : tiles) {
        for (const auto& tile : row) {
            std::cout << toChar(tile->getType(), compassPickedUp);
        }
        std::cout << std::endl;
    }

    std::cout << "Race: Human Gold: 0"
              << "\t\t\t\t\t\t\tFloor " << 1 << std::endl
              << "HP: 20\nAtk: 20\nDef: 20\nAction:" << std::endl;
}
void Board::printBoard() const {
    auto pcPos = pt->getPosition();
    auto player = std::dynamic_pointer_cast<PlayerCharacter>(pt->getEntity());
    int visibility = player ? player->getVisibility() : 8;
    
    std::cout << std::endl;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            Position pos(x, y);
            auto dist=pos.distanceTo(pcPos);
            if (dist <= visibility) {
                if(dist==visibility && PRNG::randInt(2)) std::cout<<" ";
                else if (pcPos == pos) 
                    std::cout << "\033[34m" << '@';
                else {
                    auto tile = getTile(pos);
                    std::cout << getColor(tile->getType(), compassPickedUp) 
                          << toChar(tile->getType(), compassPickedUp);
                }
            }
            else {
                std::cout << " "; // Hidden tiles
            }
            std::cout << RESET;
        }
        std::cout << std::endl;
    }
}

