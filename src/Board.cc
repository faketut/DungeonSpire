#ifndef BOARD_CC
#define BOARD_CC
#include "Board.h"

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
// check if enemy tile t is hostile
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
    // Notify observers if necessary
}
std::shared_ptr<Tile> Board::getRandomTile(const std::vector<std::shared_ptr<Tile>>& tiles, size_t start, size_t end) {
    // 如果未指定范围，默认使用整个 vector
    if (end == 0 || end > tiles.size()) {
        end = tiles.size();
    }

    // 检查范围是否有效
    if (start >= end || tiles.empty()) {
        return nullptr; // 范围无效或 tiles 为空，返回 nullptr
    }

    // 在指定范围内随机选择一个索引
    size_t index = start + PRNG::randInt(static_cast<int>(end - start));
    return tiles[index];
}

void Board::movePc(Direction dir) {
    dialog.clear();
    
    auto player=std::dynamic_pointer_cast<PlayerCharacter>(pt->getEntity());
    if (!player) return;
    if(player->getMovementSpeed()<1) return;
    
    Position pos = convertDirection(*pt, dir);
    auto tile = getTile(pos);
    // Safety Check: Ensure tile exists
    if (!tile) {
        return;
    }
    // if player's speed is 2, move extra one tile
    if(player->getMovementSpeed()==2) {
        pos=convertDirection(*tile,dir);
        tile=getTile(pos);
    }
    // Check if player steps on stairs and moves to nextFloor level
    if (dir == Direction::we && isCompassPickedUp() && tile->getType() == Type::STAIRWAY) {
        nextFloor();
        dialog="PC goes up the stairway.";
        return;
    }
    // Validate movement
    if (isValidMove(*pt, pos)) {
        if(TypeCategories::isPickable(tile->getType())) {
            pickUp(dir);
        }
        pt->setPosition(pos);
        dialog="PC moves "+toString(dir) +seeDiaHelp(dir);
    }else{
        dialog="PC fails to move "+toString(dir);
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

bool Board::isAllMerchantsHostile() const{return allMerchantsHostile;}
void Board::setAllMerchantsHostile() {allMerchantsHostile=true;}
bool Board::isCompassPickedUp() const{return compassPickedUp;}
void Board::initFloor() {
    if (tiles.empty()) return;

    try {
        std::vector<std::shared_ptr<Tile>> floorTiles = getFloorTile();
        if (floorTiles.empty()) return;

        // Generating the player
        size_t firstQuarterEnd = floorTiles.size() / 4;
        auto tile = getRandomTile(floorTiles, 0, firstQuarterEnd);
        if (!tile) return;
        auto player = std::make_shared<PlayerCharacter>();
        auto pos = tile->getPosition();
        pt = std::make_shared<Tile>(pos, player, Type::PLAYER);
        tile->setPosition(pos);
        tile->setType(Type::FLOOR);
        floorTiles.erase(std::remove(floorTiles.begin(), floorTiles.end(), tile), floorTiles.end());

        // Generate stairway
        size_t lastQuarterStart = floorTiles.size() * 3 / 4;
        tile = getRandomTile(floorTiles, lastQuarterStart, floorTiles.size());
        pos=tile->getPosition();
        if (!tile) {
            std::cerr << "Error: Failed to choose a valid tile for the stairway." << std::endl;
            return;
        }
        tile->setPosition(pos);
        tile->setType(Type::STAIRWAY);
        setTile(tile->getPosition(), Type::STAIRWAY);
        floorTiles.erase(std::remove(floorTiles.begin(), floorTiles.end(), tile), floorTiles.end());

        // Generate potions
        for (int i = 0; i < POTION_CNT; ++i) {
            tile = getRandomTile(floorTiles);
            auto pos=tile->getPosition();
            if (!tile) break;  // Avoid invalid access
            auto item = ItemGenerator::generatePotion();
            tile->setEntity(item);
            tile->setPosition(pos);
            tile->setType(item->getType());
            setTile(tile->getPosition(), item->getType(), item);
            floorTiles.erase(std::remove(floorTiles.begin(), floorTiles.end(), tile), floorTiles.end());
        }

        // Generate gold
        for (int i = 0; i < GOLD_CNT; ++i) {
            tile = getRandomTile(floorTiles);
            auto pos=tile->getPosition();
            if (!tile) break;
            auto gold = ItemGenerator::generateGold();
            if (gold->getType() == Type::DRAGON_HOARD) {
                auto tilePos=tile->getPosition();
                auto nearbyTiles = getNeighbourTiles(tilePos,Type::FLOOR);
                auto dragonTile = getRandomTile(nearbyTiles);
                if (dragonTile) {
                    auto dragon = EnemyGenerator::generateDragon();
                    dragon->setProtectedItem(gold,pos);
                    dragonTile->setEntity(dragon);
                    dragonTile->setType(Type::DRAGON);
                    dragonTile->setPosition(dragonTile->getPosition());
                    setTile(dragonTile->getPosition(), Type::DRAGON, dragon);
                    addEnemyTile(dragonTile);
                    floorTiles.erase(std::remove(floorTiles.begin(), floorTiles.end(), dragonTile), floorTiles.end());
                }
            }
            tile->setEntity(gold);
            tile->setPosition(pos);
            tile->setType(gold->getType());
            setTile(tile->getPosition(), gold->getType(), gold);
            floorTiles.erase(std::remove(floorTiles.begin(), floorTiles.end(), tile), floorTiles.end());
        }

        // Generate enemies
        bool setCompass = false;
        int generatedCount = 0;
        while (generatedCount < ENEMY_CNT && !floorTiles.empty()) {
            tile = getRandomTile(floorTiles);
            auto pos=tile->getPosition();
            if (!tile) continue;
            auto enemy = EnemyGenerator::generateEnemy();
            auto enemyType=enemy->getType();
            if(enemyType==Type::MERCHANT){
                auto hoard=ItemGenerator::generateMerchantHoard();
                enemy->setProtectedItem(hoard,pos);
            }else if (!setCompass && enemyType!=Type::DRAGON && enemyType!=Type::MERCHANT) {
                auto compass = ItemGenerator::generateCompass();
                enemy->setProtectedItem(compass,pos);
                setCompass = true;
            }
            tile->setEntity(enemy);
            tile->setType(enemyType);
            tile->setPosition(pos);
            setTile(pos, enemyType, enemy);
            addEnemyTile(tile);
            floorTiles.erase(std::remove(floorTiles.begin(), floorTiles.end(), tile), floorTiles.end());
            generatedCount++;
        }

        // generate barrier suit
        if(!getSetSuit()){
            if(floorId==4 || PRNG::randInt(2)){
                tile = getRandomTile(floorTiles);
                auto pos=tile->getPosition();
                if (tile) {
                    auto suit=ItemGenerator::generateBarrierSuit();
                    auto nearbyTiles = getNeighbourTiles(pos,Type::FLOOR);
                    auto dragonTile = getRandomTile(nearbyTiles);
                    if (dragonTile) {
                        auto dragon = EnemyGenerator::generateDragon();
                        dragon->setProtectedItem(suit,pos);
                        dragonTile->setEntity(dragon);
                        dragonTile->setType(Type::DRAGON);
                        dragonTile->setPosition(dragonTile->getPosition());
                        addEnemyTile(dragonTile);
                        floorTiles.erase(std::remove(floorTiles.begin(), floorTiles.end(), dragonTile), floorTiles.end());
                    }
                    tile->setEntity(suit);
                    tile->setPosition(pos);
                    tile->setType(Type::BARRIER_SUIT);
                    setTile(pos,Type::BARRIER_SUIT,suit);
                    floorTiles.erase(std::remove(floorTiles.begin(), floorTiles.end(), tile), floorTiles.end());
                }
                setBarrierSuit();
            }
        }
        return;
    } catch (const std::exception& e) {
        std::cerr << "Error during initialization: " << e.what() << std::endl;
        return;
    }
}

bool Board::nextFloor() {
    auto player=std::dynamic_pointer_cast<PlayerCharacter>(pt->getEntity());
    if (!player) return false;
    if (!player->isAlive() ||
        (filename!="./files/default.txt"&&getFloorId()==getMaxFloorId()-1)||
        (filename=="./files/default.txt"&&getFloorId()==4)){
        floorId++;
        return false;
    }

    auto p = std::dynamic_pointer_cast<PlayerCharacter>(pt->getEntity());
    if (p) EffectManager::getInstance()->clearTemporaryEffects(p);
    floorId++;
    compassPickedUp = false;
    if (filename != "./files/default.txt") {
        initByFile(floorId);
    } else {
        loadBoard(floorId, filename, toTypeBoard);
        initFloor();
    }
    return true;
}

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
std::shared_ptr<Entity> Board::getEntity(Type type, Position& pos) {
    std::shared_ptr<Entity> ent;
    if (TypeCategories::isEnemy(type)) {
        auto enemy = EnemyGenerator::generateEnemy(type);
        if (enemy->getType() == Type::MERCHANT) {
            auto hoard = ItemGenerator::generateMerchantHoard();
            enemy->setProtectedItem(hoard, pos);
        } else if (enemy->getType() == Type::DRAGON) {
            auto hoardTiles = getNeighbourTiles(pos, Type::DRAGON_HOARD);
            if (hoardTiles.empty()) {
                auto suiTiles = getNeighbourTiles(pos, Type::BARRIER_SUIT);
                if (!suiTiles.empty()) {
                    auto tile = suiTiles[0];
                    auto suit = std::dynamic_pointer_cast<Item>(tile->getEntity());
                    if (suit) {
                        enemy->setProtectedItem(suit, tile->getPosition());
                    } else {
                        std::cerr << "Error: Tile does not contain a valid BARRIER_SUIT entity.\n";
                    }
                } else {
                    std::cerr << "Error: No nearby BARRIER_SUIT tiles found for DRAGON.\n";
                }
            } else {
                auto tile = getRandomTile(hoardTiles);
                if (tile) {
                    auto hoard = std::dynamic_pointer_cast<Gold>(tile->getEntity());
                    if (hoard) {
                        enemy->setProtectedItem(hoard, tile->getPosition());
                    } else {
                        std::cerr << "Error: Tile does not contain a valid DRAGON_HOARD entity.\n";
                    }
                } else {
                    std::cerr << "Error: Failed to choose a random DRAGON_HOARD tile.\n";
                }
            }
        }
        ent=enemy;
    } else if (TypeCategories::isGold(type)) {
        ent = ItemGenerator::generateGold(type);
    } else if (TypeCategories::isPotion(type)) {
        ent = ItemGenerator::generatePotion(type);
    } else if (type == Type::COMPASS) {
        ent = ItemGenerator::generateCompass();
    } else if (type == Type::BARRIER_SUIT) {
        ent = ItemGenerator::generateBarrierSuit();
    } else if (type == Type::PLAYER) {
        auto player = std::make_shared<PlayerCharacter>();
        pt = std::make_shared<Tile>(pos, player, Type::PLAYER);
        ent=player; 
    }else{
        ent=nullptr;
    }
    return ent;
}


void Board::setFloorId(int i){
    floorId=i;
}
void Board::setBarrierSuit(){
    setSuit=true;
}
bool Board::getSetSuit()const{return setSuit;}
void Board::setCompassPickedUp(bool b){compassPickedUp=b;}
int Board::getMaxFloorId()const{return maxFloorId;}

void Board::setMaxFloorId(std::string fn){
    std::ifstream fs(fn);
    if (!fs.is_open()) {
        throw LoadException("Failed to open file: " + fn);
    }

    // Count the number of lines in the file
    int lineCnt = 0;
    std::string line;
    while (std::getline(fs, line)) {
        ++lineCnt;
    }
    // Validate that the line count is a multiple of BOARD_HEIGHT
    if (lineCnt % BOARD_HEIGHT != 0) {
        throw LoadException("Invalid file format: Total number of lines (" +
                                std::to_string(lineCnt) +
                                ") is not a multiple of BOARD_HEIGHT (" +
                                std::to_string(BOARD_HEIGHT) + ").");
    }

    // Calculate the number of floors
    int floorCnt = lineCnt / BOARD_HEIGHT;
    // Set the maximum floor ID
    std::cout<<"max floor id:" <<floorCnt<<std::endl;
    maxFloorId=floorCnt;
}


void Board::initByFile(int floorId) {
    // Open and validate file
    std::ifstream fs(filename);
    if (!fs.is_open()) {
        throw LoadException("Failed to open file: " + filename);
    }

    // Clear existing state
    enemyTiles.clear();
    pt = nullptr;

    // First pass: Read and store the board layout
    std::vector<std::string> boardLayout;
    try {
        // Skip previous floors
        std::string line;
        for (int i = 0; i < floorId * BOARD_HEIGHT && std::getline(fs, line); ++i);

        // Read current floor
        int y = 0;
        while (std::getline(fs, line) && y < BOARD_HEIGHT) {
            if (line.length() < BOARD_WIDTH) {
                throw LoadException("Line " + std::to_string(y) + " is too short");
            }
            boardLayout.push_back(line);
            ++y;
        }
        if (y < BOARD_HEIGHT) {
            throw LoadException("Not enough lines for floor " + std::to_string(floorId));
        }
    } catch (const std::exception& e) {
        throw LoadException("Error reading floor layout: " + std::string(e.what()));
    }

    // Second pass: Create entities and initialize tiles
    try {
        for (int y = 0; y < BOARD_HEIGHT; ++y) {
            for (int x = 0; x < BOARD_WIDTH; ++x) {
                auto type = toType(boardLayout[y][x]);
                auto pos = Position(x, y);
                auto tile = getTile(pos);
                
                // Create entity first
                std::shared_ptr<Entity> ent = createEntity(type, pos);
                
                // Special handling for player
                if (type == Type::PLAYER) {
                    type = Type::FLOOR;
                }

                // Update tile state
                tile->setEntity(ent);
                tile->setPosition(pos);
                tile->setType(type);
                
                // Update board state
                if (TypeCategories::isEnemy(type)) {
                    addEnemyTile(tile);
                }
            }
        }

        // Third pass: Handle special relationships (dragons and protected items, compass).
        // Honor any compass tile present in the file by attaching it to a non-Dragon/Merchant
        // enemy if one exists; otherwise (no compass tile in the file) generate one and assign
        // it to a random enemy, matching initFloor() behavior.
        std::shared_ptr<Item> compass;
        for (int y = 0; y < BOARD_HEIGHT && !compass; ++y) {
            for (int x = 0; x < BOARD_WIDTH && !compass; ++x) {
                auto tile = tiles[y][x];
                if (tile && tile->getType() == Type::COMPASS) {
                    auto item = std::dynamic_pointer_cast<Item>(tile->getEntity());
                    if (item) compass = item;
                }
            }
        }
        if (!compass) compass = ItemGenerator::generateCompass();
        auto compassCandidates = getAllEnemyTileExceptDragonAndMerchant();
        auto compassHolder = getRandomTile(compassCandidates);
        if (compassHolder) {
            auto enemy = std::dynamic_pointer_cast<Enemy>(compassHolder->getEntity());
            if (enemy) {
                enemy->setProtectedItem(compass, compassHolder->getPosition());
            }
        }
        for (int y = 0; y < BOARD_HEIGHT; ++y) {
            for (int x = 0; x < BOARD_WIDTH; ++x) {
                auto pos = Position(x, y);
                auto tile = getTile(pos);
                auto type = tile->getType();

                if (type == Type::DRAGON_HOARD || type == Type::BARRIER_SUIT) {
                    auto item = std::dynamic_pointer_cast<Item>(tile->getEntity());
                    if (!item) {
                        std::cerr << "Warning: Invalid item at position " << x << "," << y << std::endl;
                        continue;
                    }

                    // Find nearby unassigned dragon
                    auto neighbours = getNeighbourTiles(pos, Type::DRAGON);
                    for (auto& dragonTile : neighbours) {
                        auto dragon = std::dynamic_pointer_cast<Enemy>(dragonTile->getEntity());
                        if (dragon && !dragon->getProtectedItem()) {
                            dragon->setProtectedItem(item, pos);
                            break;
                        }
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        throw LoadException("Error initializing board: " + std::string(e.what()));
    }
}

std::shared_ptr<Entity> Board::createEntity(Type type, const Position& pos) {
    if (TypeCategories::isEnemy(type)) {
        auto enemy = EnemyGenerator::generateEnemy(type);
        if (type == Type::MERCHANT) {
            auto hoard = ItemGenerator::generateMerchantHoard();
            enemy->setProtectedItem(hoard, pos);
        }
        return enemy;
    } 
    else if (TypeCategories::isGold(type)) {
        return ItemGenerator::generateGold(type);
    } 
    else if (TypeCategories::isPotion(type)) {
        return ItemGenerator::generatePotion(type);
    } 
    else if (type == Type::COMPASS) {
        return ItemGenerator::generateCompass();
    } 
    else if (type == Type::BARRIER_SUIT) {
        return ItemGenerator::generateBarrierSuit();
    } 
    else if (type == Type::PLAYER) {
        auto player = std::make_shared<PlayerCharacter>();
        pt = std::make_shared<Tile>(pos, player, Type::PLAYER);
        return nullptr;
    }
    return nullptr;
}

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

int Board::getFloorId() const{return floorId;}
void Board::pickUp(Direction dir){
    auto pos=convertDirection(*pt,dir);
    auto tile = getTile(pos);
    if (!tile) return;
    auto tileType=tile->getType();
    auto player=std::dynamic_pointer_cast<PlayerCharacter>(pt->getEntity());
    if (!player) return;
    if (TypeCategories::isPotion(tileType)) {
        auto item=std::dynamic_pointer_cast<Potion>(tile->getEntity());
        if (item) {
            item->use(player);
            dialog="PC uses "+toString(item->getType());
        }
    }else if(TypeCategories::isGold(tileType)){
        auto item=std::dynamic_pointer_cast<Gold>(tile->getEntity());
        if (!item) return;
        if(tileType==Type::DRAGON_HOARD &&item->isProtected()){
            dialog="PC fails to pick up protected Dragon Hoard";
            return;
        }else{
            item->getGold(player);
            dialog="PC picks up "+toString(item->getType());
            player->setScore(item->getValue());
        }
    }else if (tileType==Type::COMPASS){
        setCompassPickedUp(true);
        dialog="PC picks up Compass and stairway is now appeared";
    }else if (tileType==Type::BARRIER_SUIT){
        auto item=std::dynamic_pointer_cast<Item>(tile->getEntity());
        if (!item) return;
        if(item->isProtected()) {
            dialog="PC fails to pick up protected Barrier suit";
            return;
        }else {
            player->setBarrierSuit();
            dialog="PC picks up Barrier suit";
        }
    }
    player->incrementItemCount(tileType);
    setTile(pos,Type::FLOOR);
}
bool Board::isValidMove(const Tile& t, const Position& pos) const {
    auto objType = t.getType();
    auto tile = getTile(pos);
    if (!tile) return false; // Ensure tile exists
    auto tileType = tile->getType();
    if (objType == Type::PLAYER) {
        return  TypeCategories::isTraversable(tileType);
    } 
    else {  // Enemy movement logic
        return tileType == Type::FLOOR;
    }
}
std::string Board::seeDiaHelp(Direction dir) {
    auto pos=convertDirection(*pt,dir);
    auto tile=getTile(pos);
    auto tileType=tile->getType();
    if(TypeCategories::isPotion(tileType))  dialog+= " and sees an unknown potion";
    else if (TypeCategories::isGold(tileType)) dialog+=  " and sees an unknown gold";
    else dialog+=  "";
    return dialog;
}

std::string Board::diaHelp() {
    return dialog;
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
int Board::calDamage(const Tile& atk, const Tile& def) {
    int damage;
    if (atk.getType() == Type::PLAYER) {
        auto player=std::dynamic_pointer_cast<PlayerCharacter>(pt->getEntity());
        auto enemy = std::dynamic_pointer_cast<Enemy>(def.getEntity());
        
        if (!enemy) {
            std::cerr << "Error: Invalid entity type in calDamage" << std::endl;
            return 0;
        }
        
        damage = static_cast<int>(std::ceil(
            (100.0 / (100 + enemy->getDef())) 
            * player->getAtk()));
    } else {
        auto enemy = std::dynamic_pointer_cast<Enemy>(atk.getEntity());
        auto player = std::dynamic_pointer_cast<PlayerCharacter>(pt->getEntity());
        
        if (!enemy || !player) {
            std::cerr << "Error: Invalid entity type in calDamage" << std::endl;
            return 0;
        }
        // Enemies have a 50% chance not to hit PC
        if (PRNG::randInt(2)) { return 0; }
        damage = static_cast<int>(std::ceil(
            (100.0 / (100 + player->getDef())) 
            * enemy->getAtk()));
        if (player->hasSuit()) {
            damage = static_cast<int>(std::ceil(damage / 2.0));
        }
    }
    return damage;
}
void Board::attackPc(Tile& et){
    auto pos=et.getPosition();
    auto pcPos=pt->getPosition();
    if(!pos.near(pcPos)) return;
    int damage=calDamage(et,*pt);
    auto player=std::dynamic_pointer_cast<PlayerCharacter>(pt->getEntity());
    auto enemy=std::dynamic_pointer_cast<Enemy>(et.getEntity());
    if (!player || !enemy) return;
    player->setHp(-damage);
    enemy->useSpecialAbility(*player);
    dialog+= " and "+toString(enemy->getType())+ " deals "+ std::to_string(damage)+" damage to PC";
}
void Board::attackEnemy(Tile& et){
    int damageToEnemy=calDamage(*pt,et);
    auto player=std::dynamic_pointer_cast<PlayerCharacter>(pt->getEntity());
    auto enemy=std::dynamic_pointer_cast<Enemy>(et.getEntity());
    if (!player || !enemy) return;
    enemy->setHp(-damageToEnemy);
    if(enemy->getType()==Type::PHOENIX && enemy->getHp()<=0) {
        auto phonix=std::dynamic_pointer_cast<Phoenix>(enemy);
        if(phonix && !phonix->isRisen()) phonix->useSpecialAbility(*player);
    }
    dialog="PC deals " + std::to_string(damageToEnemy) +
            " damage to " + toChar(et.getType()) + " (" +
            std::to_string(enemy->getHp()) + ")";
}
void Board::die(Tile& t) {
    auto pos = t.getPosition();
    auto enemy = std::dynamic_pointer_cast<Enemy>(t.getEntity());
    if (!enemy) {
        std::cerr << "Error: Tile does not contain an Enemy entity.\n";
        return;
    }
    if (t.getType() == Type::DRAGON) {
        auto protectedItem = enemy->getProtectedItem();
        if (protectedItem) {
            protectedItem->unprotect();
        }
        setTile(pos, Type::FLOOR);
    } else if (t.getType() == Type::MERCHANT) {
        setAllMerchantsHostile();
        auto hoard = enemy->getProtectedItem();
        if (hoard) {
            hoard->unprotect();
        }
        int x=pos.getX();int y= pos.getY();
        tiles[y][x]->setEntity(hoard);
        tiles[y][x]->setType(Type::MERCHANT_HOARD);
        tiles[y][x]->setPosition(pos);
        std::cout << toString(enemy->getType()) << " drop Merchant Hoard.\n";
    } else {
        auto player = std::dynamic_pointer_cast<PlayerCharacter>(pt->getEntity());
        if (player) {
            player->setGold(1);
        }

        auto item = enemy->getProtectedItem();
        if (item) {
            auto tile=getTile(pos);
            tile->setEntity(item);
            tile->setType(item->getType());
            tile->setPosition(pos);
            setTile(pos,item->getType(),item);
            std::cout << toString(enemy->getType()) << " drop " << toString(item->getType()) << "\n";
        } else {
            setTile(pos, Type::FLOOR);
        }
    }

    auto player=std::dynamic_pointer_cast<PlayerCharacter>(pt->getEntity());
    player->setScore(enemy->getAtk()+enemy->getDef()+enemy->getMaxHp());
    player->incrementKillCount(enemy->getType());
    removeEnemyTile(findEnemyTile(t));
}

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
