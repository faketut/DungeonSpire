#ifndef BOARD_CC
#define BOARD_CC
#include "Board.h"
#include "EventBus.h"

Tile::Tile(Position pos, std::shared_ptr<Entity>const& ent, Type type) : pos(pos), entity(ent), t(type) {}
Position Tile::getPosition()const{return pos;}
int Tile::getX()const{return pos.getX();}
int Tile::getY()const{return pos.getY();}
bool Tile::near(const Position& other) const {return pos.near(other); }
Type Tile::getType()const{return t;}
void Tile::setType(Type type){t=type;}
void Tile::setPosition(const Position& newPos) {pos.setPosition(newPos);}


void Tile::setEntity(std::shared_ptr<Entity> newEntity){
    entity = newEntity;
}
std::shared_ptr<Entity> Tile::getEntity() const {return entity;}

Position Board::convertDirection(Tile& t, Direction dir) {
    int dx = 0, dy = 0;
    switch (dir) {
        case Direction::no: dy = -1; break; // North
        case Direction::so: dy = 1;  break; // South
        case Direction::ea: dx = 1;  break; // East
        case Direction::we: dx = -1; break; // West
        case Direction::ne: dx = 1;  dy = -1; break; // Northeast
        case Direction::nw: dx = -1; dy = -1; break; // Northwest
        case Direction::se: dx = 1;  dy = 1;  break; // Southeast
        case Direction::sw: dx = -1; dy = 1;  break; // Southwest
    }
    return Position(t.getX() + dx, t.getY() + dy);
}


Board::Board(std::string name, int id) 
    : tiles(BOARD_HEIGHT, std::vector<std::shared_ptr<Tile>>(BOARD_WIDTH)),
    compassPickedUp(false), 
    allMerchantsHostile(false),
    floorId(id),
    filename(name),
    setSuit(false) {
} // ctor
void Board::addEnemyTile(std::shared_ptr<Tile>& t) {
    enemyTiles.emplace_back(t);
}

std::shared_ptr<Tile> Board::getTile(const Position& pos) const {
    return tiles[pos.getY()][pos.getX()];
}
std::shared_ptr<Tile> Board::getPc() const {
    return pt;
}

void Board::setTile(const Position& pos,Type t,std::shared_ptr<Entity> entity){
    int x=pos.getX();int y= pos.getY();
    if(t==Type::PLAYER){
        tiles[y][x]->setEntity(nullptr);
        tiles[y][x]->setType(Type::FLOOR);
        tiles[y][x]->setPosition(pos);        
    }else {
        tiles[y][x]->setEntity(entity);
        tiles[y][x]->setType(t);
        tiles[y][x]->setPosition(pos);
    }
}
std::shared_ptr<Tile> Board::getRandomTile(const std::vector<std::shared_ptr<Tile>>& tiles, size_t start, size_t end) {
    if (end == 0 || end > tiles.size()) {
        end = tiles.size();
    }
    if (start >= end || tiles.empty()) {
        return nullptr;
    }
    size_t index = start + PRNG::randInt(static_cast<int>(end - start));
    return tiles[index];
}

bool Board::isAllMerchantsHostile() const{return allMerchantsHostile;}
void Board::setAllMerchantsHostile() {allMerchantsHostile=true;}
bool Board::isCompassPickedUp() const{return compassPickedUp;}

std::vector<std::shared_ptr<Tile>> Board::getFloorTile()const{
    std::vector<std::shared_ptr<Tile>> floorTiles;
    for (auto& row : tiles) {
        for (auto& tile : row) {
            if (tile && tile->getType() == Type::FLOOR) {
                floorTiles.emplace_back(tile);
            }
        }
    }
    return floorTiles;
}

void Board::setFloorId(int i){
    floorId=i;
    cc3k::EventBus::getInstance()->publish(cc3k::events::FloorChanged{i});
}
void Board::setBarrierSuit(){
    setSuit=true;
}
bool Board::getSetSuit()const{return setSuit;}
void Board::setCompassPickedUp(bool b){compassPickedUp=b;}
int Board::getMaxFloorId()const{return maxFloorId;}

int Board::getFloorId() const{return floorId;}

void Board::removeEnemyTile(const std::shared_ptr<Tile>& tileToRemove) {
    if (!tileToRemove) {
        std::cerr << "Error: Attempted to remove a null tile.\n";
        return;
    }

    enemyTiles.erase(
        std::remove_if(enemyTiles.begin(), enemyTiles.end(),
                       [&tileToRemove](const std::shared_ptr<Tile>& tile) {
                           return tile.get() == tileToRemove.get(); 
                       }),
        enemyTiles.end());
}

std::shared_ptr<Tile> Board::findEnemyTile(const Tile& tile) {
    for (const auto& enemyTile : enemyTiles) {
        if (enemyTile.get() == &tile) { // 比较原始指针
            return enemyTile;
        }
    }
    return nullptr; 
}

#endif
