#include "Board.h"
#include "Quest.h"
#include "QuestManager.h"
#include <iostream>
#include <sstream>
#include <cmath>

bool Board::isHostile(const Tile& t) const{
    auto pos=t.getPosition();
    auto type=t.getType();
    auto ent=t.getEntity();
    auto pcPos=pt->getPosition();
    if(type == Type::DRAGON) {
        // DRAGON is hostile when PC is next to DRAGON Horde
        auto dragon=std::dynamic_pointer_cast<Enemy>(ent);
        return dragon->getProtectedItemPosition().near(pcPos);
    } else if (type == Type::MERCHANT) {
        return allMerchantsHostile;
    } else {
        return pos.near(pcPos);
    }
}

// updateEnemyState enemy state(attack/move)
void Board::updateEnemies() {
    for (auto& enemyTile : enemyTiles) {
        if(!enemyTile) {
            continue;
        }
        auto enemy = std::dynamic_pointer_cast<Enemy>(enemyTile->getEntity());
        if(!enemy) {
            continue;}
        if (enemy->isDead()) {
            die(*enemyTile);
        } else if (isHostile(*enemyTile)) {
            attackPc(*enemyTile);
        } else {
            moveEnemy(*enemyTile);
        }
    }
}
void Board::moveEnemy(Tile& t){
    if (t.getType() == Type::DRAGON) return;
    // ~33% chance the enemy acts on this tick.
    constexpr int ENEMY_ACT_DENOM = 3;
    if (PRNG::randInt(ENEMY_ACT_DENOM)) return;
    constexpr int DIRECTION_COUNT = 8;
    int random = PRNG::randInt(DIRECTION_COUNT);
    Direction dir;
    switch (random) {
        case 0: dir=Direction::no;break;
        case 1:dir=Direction::ne;break;
        case 2:dir=Direction::ea;break;
        case 3:dir=Direction::se;break;
        case 4:dir=Direction::so;break;
        case 5:dir=Direction::sw;break;
        case 6:dir=Direction::we;break;
        case 7:dir=Direction::nw;break;
    }
    Position pos = convertDirection(t, dir);
    if (!isValidMove(t,pos)) return;
    auto oldPos = t.getPosition();
    auto enemy = std::dynamic_pointer_cast<Enemy>(t.getEntity());
    if (enemy) {
        auto oldT=getTile(oldPos);
        auto newT=getTile(pos);
        // Update the tile and entity on the board
        setTile(oldPos, Type::FLOOR); 
        setTile(pos,enemy->getType(),enemy);
        newT->setEntity(enemy);
        newT->setType(enemy->getType());
        oldT->setEntity(nullptr);
        oldT->setType(Type::FLOOR);

        for(auto &et: enemyTiles){
            if(et.get()==&t){
                et=newT;
            }
        }
    }
}
std::vector<std::shared_ptr<Tile>> Board::getNeighbourTiles(const Position& pos, Type t) const{
    std::vector<std::shared_ptr<Tile>> neighbours;
    int cx = pos.getX();
    int cy = pos.getY();
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            int nx = cx + dx;
            int ny = cy + dy;
            if (nx < 0 || nx >= BOARD_WIDTH || ny < 0 || ny >= BOARD_HEIGHT) continue;
            const auto& tile = tiles[ny][nx];
            if (tile && tile->getType() == t) {
                neighbours.emplace_back(tile);
            }
        }
    }
    return neighbours;
}

std::vector<std::shared_ptr<Tile>> Board::getAllEnemyTileExceptDragonAndMerchant() const{
    std::vector<std::shared_ptr<Tile>> result;
    result.reserve(enemyTiles.size());
    for (const auto& tile : enemyTiles) {
        if (!tile) continue;
        auto t = tile->getType();
        if (TypeCategories::isEnemy(t) && t != Type::DRAGON && t != Type::MERCHANT) {
            result.emplace_back(tile);
        }
    }
    return result;
}
