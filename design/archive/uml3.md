@startuml
skinparam classAttributeIconSize 0

class Tile {
  -Position pos
  -std::shared_ptr<Entity> entity
  -Type t
  -std::weak_ptr<Board> board
  +Tile(Position, std::shared_ptr<Entity> const&, Type, std::shared_ptr<Board> const&)
  +Position getPosition() const
  +void setPosition(Position)
  +bool isPickable()
  +int getX() const
  +int getY() const
  +bool near(Position)
  +Type getType() const
  +void setType(Type)
  +void setEntity(std::shared_ptr<Entity>)
  +std::shared_ptr<Entity> getEntity() const
}

class Board {
  -TileGrid tiles
  -std::vector<std::shared_ptr<Tile>> enemyTiles
  -std::shared_ptr<Tile> pt
  -bool compassPickedUp
  -bool allMerchantsHostile
  -int floorId
  -std::string filename
  -std::string dialog
  -bool setSuit
  -int maxFloorId
  +Board(std::string filename = "./files/default.txt", int floorId = 0)
  +void initFloor()
  +void setCompassPickedUp(bool b)
  +void setAllMerchantsHostile()
  +void setFloorId(int i)
  +void setMaxFloorId(std::string fn)
  +void setBarrierSuit()
  +std::shared_ptr<Tile> getPc() const
  +int getFloorId() const
  +bool isAllMerchantsHostile() const
  +bool isCompassPickedUp() const
  +int getMaxFloorId() const
  +bool isHostile(const Tile& t) const
  +bool getSetSuit() const
}

Tile ..|> std::enable_shared_from_this<Board>
Board ..|> Tile : "contains"


class Game {
  -std::shared_ptr<Board> board
  -GameState state
  -int seed
  -std::string filename
  -std::string dialog
  
  +Game(int seed, std::string filename)
  +bool gameOver()
  +void processCommand()
  +void run()
  +void renderBoard()
  +void renderInfo()
  +void victory()
  +void defeat()
  +void quit()
  +void restart()
}
Game --> Board : "has-a"

' Class with static members
class TypeCategories {
    +static potions: unordered_set<Type>
    +static gold: unordered_set<Type>
    +static enemies: unordered_set<Type>
    +static isPotion(t: Type): bool
    +static isGold(t: Type): bool
    +static isEnemy(t: Type): bool
}



class Position {
  -int x
  -int y
  
  +Position(int, int)
  +int getX() const
  +int getY() const
  +Position getPosition() const
  +void setX(int)
  +void setY(int)d
  +void setPosition(Position)
  +bool near(Position)
  +bool operator==(const Position&) const
}

class Quest {
  -std::string description
  -bool completed
  +Quest(const std::string& desc)
  +virtual ~Quest()
  +virtual void checkCompletion(std::shared_ptr<PlayerCharacter> pc)
  +virtual void giveReward(std::shared_ptr<PlayerCharacter> pc)
  +bool isCompleted() const
  +std::string getDescription() const
}

class KillQuest {
  -Type targetType
  -int requiredCount
  -int currentCount
  -bool includeItems
  +KillQuest(Type target, int count, const std::string& desc, bool countItems = false)
  +void checkCompletion(std::shared_ptr<PlayerCharacter> pc) override
  +void giveReward(std::shared_ptr<PlayerCharacter> pc) override
}

class CollectQuest {
  -Type itemType
  -int requiredCount
  +CollectQuest(Type item, int count, const std::string& desc)
  +void checkCompletion(std::shared_ptr<PlayerCharacter> pc) override
  +void giveReward(std::shared_ptr<PlayerCharacter> pc) override
}

class QuestManager {
  -static std::shared_ptr<QuestManager> instance
  -static std::once_flag initFlag
  -std::vector<std::unique_ptr<Quest>> activeQuests
  +{static} std::shared_ptr<QuestManager> getInstance()
  +void addQuest(std::unique_ptr<Quest> quest)
  +void updateQuests(std::shared_ptr<PlayerCharacter> pc)
  +void clearQuests()
  +const std::vector<std::unique_ptr<Quest>>& getActiveQuests() const
}

Quest <|-- KillQuest
Quest <|-- CollectQuest
QuestManager --> Quest : manages


' Enumerations
enum GameState {
    PLAYING
    RESTART
    QUIT
    VICTORY
    DEFEAT
}

enum Race {
    DWARF
    ELF
    ORC
    HUMAN
}

enum Type {
    EMPTY
    PLAYER
    STAIRWAY
    RH
    BA
    BD
    PH
    WA
    WD
    COMPASS
    BARRIER_SUIT
    NORMAL_GOLD_PILE
    SMALL_HOARD
    DRAGON_HOARD
    MERCHANT_HOARD
    WEREWOLF
    VAMPIRE
    GOBLIN
    TROLL
    PHOENIX
    MERCHANT
    DRAGON
    DOORWAY
    PASSAGE
    WALL
    CEIL
    FLOOR
}

enum Direction {
    no
    so
    ea
    we
    ne
    nw
    se
    sw
}
@enduml
