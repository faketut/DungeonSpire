#include "Board.h"
#include "FloorStats.h"
#include "Quest.h"
#include "QuestManager.h"
#include <iostream>
#include <sstream>
#include <cmath>

void Board::initFloor() {
    if (tiles.empty()) return;

    try {
        std::vector<std::shared_ptr<Tile>> floorTiles = getFloorTile();
        if (floorTiles.empty()) return;

        size_t firstQuarterEnd = floorTiles.size() / 4;
        auto tile = getRandomTile(floorTiles, 0, firstQuarterEnd);
        if (!tile) return;
        auto player = std::make_shared<PlayerCharacter>();
        auto pos = tile->getPosition();
        pt = std::make_shared<Tile>(pos, player, Type::PLAYER);
        tile->setPosition(pos);
        tile->setType(Type::FLOOR);
        floorTiles.erase(std::remove(floorTiles.begin(), floorTiles.end(), tile), floorTiles.end());

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

        for (int i = 0, n = FloorStats::getInstance()->potions(); i < n; ++i) {
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

        for (int i = 0, n = FloorStats::getInstance()->gold(); i < n; ++i) {
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

        bool setCompass = false;
        int generatedCount = 0;
        const int enemyCnt = FloorStats::getInstance()->enemies();
        while (generatedCount < enemyCnt && !floorTiles.empty()) {
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

