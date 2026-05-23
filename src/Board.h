#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <memory>
#include <fstream>
#include <string>
#include <random>
#include "Enum.h"
#include "Player.h"
#include "Item.h"
#include "Position.h"
#include "Enemy.h"
#include "PRNG.h"

const int BOARD_WIDTH = 79;
const int BOARD_HEIGHT = 25;
const int POTION_CNT = 10;
const int GOLD_CNT = 10;
const int ENEMY_CNT = 20;

class Board; // Forward declaration for Board

class Tile {
private:
    Position pos;
    std::shared_ptr<Entity> entity; // Base class for Enemy, PlayerCharacter, Item
    Type t;
public:
    Tile(Position pos, std::shared_ptr<Entity> const& ent, Type type);
    Position getPosition() const;
    void setPosition(const Position& newPos);
    bool isPickable();
    int getX() const;
    int getY() const;
    bool near(const Position& other) const;
    Type getType() const;
    void setType(Type type);
    void setEntity(std::shared_ptr<Entity> newEntity);
    std::shared_ptr<Entity> getEntity() const;
};
using EntityPtr = std::shared_ptr<Entity>;
using TilePtr = std::shared_ptr<Tile>;
using TileGrid = std::vector<std::vector<TilePtr>>;
class Board {
private:
    std::vector<std::vector<std::shared_ptr<Tile>>> tiles;
    std::vector<std::shared_ptr<Tile>> enemyTiles;
    std::shared_ptr<Tile> pt;
    bool compassPickedUp;          
    bool allMerchantsHostile;
    int floorId;
    std::string filename;
    std::string dialog;
    bool setSuit;
    int maxFloorId;

    // Private helper methods
    void loadFromFile(int floorId);


public:
    // Constructors and Initialization
    Board(std::string filename = "./files/default.txt", int floorId = 0);
    void initFloor();
    // Setters
    void setCompassPickedUp(bool b);
    void setAllMerchantsHostile();
    void setFloorId(int i);
    void setTile(const Position& pos, Type t, std::shared_ptr<Entity> entity = std::shared_ptr<Entity>());
    void setMaxFloorId(std::string fn);
    void setBarrierSuit();
    // Getters
    std::shared_ptr<Tile> getPc() const;
    int getFloorId() const;
    bool isAllMerchantsHostile() const;
    bool isCompassPickedUp() const;
    int getMaxFloorId() const;
    bool isHostile(const Tile& t) const;
    bool getSetSuit() const;
    // Game Logic Processing
    void updateEnemies();
    void attackPc(Tile& et);
    void moveEnemy(Tile& t);
    void attackEnemy(Tile& et);
    void die(Tile& t);

    // Player Control
    void movePc(Direction dir);
    void pickUp(Direction dir);

    // Game Flow
    bool nextFloor();

    // Board Operations
    Position convertDirection(Tile& t, Direction dir);
    template<typename Func>
    void loadBoard(int floorId, std::string& filename,Func processChar);
    void initByFile(int floorId);
    // Rendering moved to cc3k::IRenderer (see Renderer.h / AnsiRenderer.h);
    // Board exposes getTiles() so renderers can walk the full grid.
    const std::vector<std::vector<std::shared_ptr<Tile>>>& getTiles() const { return tiles; }
    std::shared_ptr<Entity> createEntity(Type type, const Position& pos);

    // Data Query
    bool isValidMove(const Tile& t, const Position& pos) const;
    std::vector<std::shared_ptr<Tile>> getNeighbourTiles(const Position& pos,Type t) const;
    std::shared_ptr<Tile> getTile(const Position& pos) const;
    std::vector<std::shared_ptr<Tile>> getFloorTile() const;
    std::shared_ptr<Tile> getRandomTile(const std::vector<std::shared_ptr<Tile>>& tiles,size_t start = 0, size_t end = 0);
    std::vector<std::shared_ptr<Tile>> getNeighbourEnemyTilesExceptDragonAndMerchant(const Position& pos) const;
    std::vector<std::shared_ptr<Tile>> getAllEnemyTileExceptDragonAndMerchant() const;
    // Help Information
    std::string seeDiaHelp(Direction dir);
    std::string diaHelp();
    std::shared_ptr<Tile> findEnemyTile(const Tile& tile);
    void assignItemToDragon(const Position& pos, const std::shared_ptr<Item>& it);

    // Damage Calculation
    int calDamage(const Tile& player, const Tile& enemy);

    // Special Items Placement
    std::shared_ptr<Tile> placeDragonHoard(const std::shared_ptr<Item>& gold, const Position& pos);
    std::shared_ptr<Entity> getEntity(Type type, Position& pos);

    // Enemy Management
    void addEnemyTile(std::shared_ptr<Tile>& enemyTile);
    void removeEnemyTile(const std::shared_ptr<Tile>& tileToRemove);

    // File Operations
};

class LoadException : public std::runtime_error {
public:
    explicit LoadException(const std::string& message)
        : std::runtime_error(message) {}
};

template<typename Func>
void Board::loadBoard(int floorId, std::string& filename, Func processChar) {
    std::ifstream fs(filename);
    if (!fs.is_open()) {
        throw LoadException("Failed to open file: " + filename);
    }

    // Skip lines that do not belong to the current floor
    std::string line;
    int skipLines = floorId * BOARD_HEIGHT;
    for (int i = 0; i < skipLines && std::getline(fs, line); ++i);

    // Load the current floor
    int y = 0;
    enemyTiles.clear();
    while (std::getline(fs, line) && y < BOARD_HEIGHT) {
        if (static_cast<int>(line.length()) < BOARD_WIDTH) {
            throw LoadException("Error: Line " + std::to_string(y) +
                                     " has fewer columns than expected (" +
                                     std::to_string(BOARD_WIDTH) + ").");
        }

        for (int x = 0; x < BOARD_WIDTH; ++x) {
            tiles[y][x] = std::make_shared<Tile>(
                Position(x, y), nullptr, processChar(line[x]));
        }
        ++y;
    }

    if (y != BOARD_HEIGHT) {
        throw LoadException("Error: File contains fewer rows than expected (" +
                                 std::to_string(BOARD_HEIGHT) + ").");
    }
}
#endif
