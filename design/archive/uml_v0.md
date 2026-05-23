@startuml
skinparam classAttributeIconSize 0
class PlayerCharacter {
  - Race race
  - int maxHp
  - int hp
  - int atk
  - int def
  - int gold
  - double goldModifier
  - bool hasBarrierSuit
  + PlayerCharacter(Race r = Race::HUMAN)
  + void setAttributes(Race r)
  + void setHp(int delta)
  + void setAtk(int delta)
  + void setDef(int delta)
  + void setGold(int delta)
  + void setBarrierSuit()
  + Race getRace() const
  + bool hasSuit()
  + bool isAlive()
  + std::map<std::string, int> getInfo() const
  # static constexpr int DEFAULT_MAX_HP = 140
  # static constexpr int DEFAULT_HP = 140
  # static constexpr int DEFAULT_ATK = 20
  # static constexpr int DEFAULT_DEF = 20
  # static constexpr double DEFAULT_GOLD_MODIFIER = 1.0
}
PlayerCharacter --|> Entity : inheritance


class Entity {
  + virtual ~Entity()
}

class Enemy {
  - Type enemyType
  - int maxHp
  - int hp
  - int atk
  - int def
  - std::shared_ptr<Item> protectedItem
  - Position proectedItemPos
  + Enemy(Type t)
  + std::shared_ptr<Item> getProtectedItem() const
  + Position getProtectedItemPosition() const
  + void setProtectedItem(std::shared_ptr<Item> it, Position pos)
  + std::map<std::string, int> getInfo() const
  + Type getType() const
  + int getHp() const
  + int getAtk() const
  + int getDef() const
  + int getMaxHp() const
  + void setHp(int delta)
  + bool isDead() const
  + virtual void useSpecialAbility(PlayerCharacter& player)
}

class VAMPIRE {
  - int stealAmount = 10
  + VAMPIRE()
  + void useSpecialAbility(PlayerCharacter& player)
}

class MERCHANT {
  + MERCHANT()
}

class DRAGON {
  + DRAGON()
}

class WEREWOLF {
  + WEREWOLF()
}

class TROLL {
  - int healAmount = 5
  + TROLL()
  + void useSpecialAbility()
}

class PHOENIX {
  - bool isRisen = false
  + PHOENIX()
  + void useSpecialAbility()
}

class GOBLIN {
  - int stealAmount = 5
  + GOBLIN()
  + void useSpecialAbility(PlayerCharacter& player)
}

class EnemyGenerator {
  + static std::shared_ptr<Enemy> generateEnemy()
  + static std::shared_ptr<Enemy> generateDragon()
}

Entity <|-- Enemy
Enemy <|-- VAMPIRE
Enemy <|-- MERCHANT
Enemy <|-- DRAGON
Enemy <|-- WEREWOLF
Enemy <|-- TROLL
Enemy <|-- PHOENIX
Enemy <|-- GOBLIN


class Item {
  - Type ItemType
  - bool protecting
  + Item(Type t, bool isProtected = false)
  + virtual ~Item() = default
  + void unprotect()
  + bool isProtected() const
  + Type getType() const
}

class Potion {
  + Potion(Type t)
  + virtual void use(std::shared_ptr<PlayerCharacter> pc) = 0
}

class RestoreHealthPotion {
  + RestoreHealthPotion()
  + void use(std::shared_ptr<PlayerCharacter> pc)
}

class PoisonHealthPotion {
  + PoisonHealthPotion()
  + void use(std::shared_ptr<PlayerCharacter> pc)
}

class BoostAtkPotion {
  + BoostAtkPotion()
  + void use(std::shared_ptr<PlayerCharacter> pc)
}

class WoundAtkPotion {
  + WoundAtkPotion()
  + void use(std::shared_ptr<PlayerCharacter> pc)
}

class BoostDefPotion {
  + BoostDefPotion()
  + void use(std::shared_ptr<PlayerCharacter> pc)
}

class WoundDefPotion {
  + WoundDefPotion()
  + void use(std::shared_ptr<PlayerCharacter> pc)
}

class Gold {
  - int gold
  + Gold(int g, Type t, bool isProtected = false)
  + void getGold(std::shared_ptr<PlayerCharacter>& pc)
  + int getValue() const
}

class NormalGoldPile {
  + NormalGoldPile()
}

class SmallHoard {
  + SmallHoard()
}

class DragonHoard {
  + DragonHoard()
}

class MerchantHoard {
  + MerchantHoard()
}

class Compass {
  + Compass()
}

class BarrierSuit {
  - static BarrierSuit* instance
  + static BarrierSuit* getInstance()
  + BarrierSuit(const BarrierSuit&) = delete
  + BarrierSuit& operator=(const BarrierSuit&) = delete
}

class ItemGenerator {
  + static std::shared_ptr<Item> generatePotion()
  + static std::shared_ptr<Gold> generateGold()
  + static std::shared_ptr<Gold> generateMerchantHoard()
  + static std::shared_ptr<Item> generateCompass()
}


Entity <|-- Item
Item <|-- Potion
Potion <|-- RestoreHealthPotion
Potion <|-- PoisonHealthPotion
Potion <|-- BoostAtkPotion
Potion <|-- WoundAtkPotion
Potion <|-- BoostDefPotion
Potion <|-- WoundDefPotion
Item <|-- Gold
Gold <|-- NormalGoldPile
Gold <|-- SmallHoard
Gold <|-- DragonHoard
Gold <|-- MerchantHoard
Item <|-- Compass
Item <|-- BarrierSuit
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

' Class with static members
class TypeCategories {
    +static potions: unordered_set<Type>
    +static gold: unordered_set<Type>
    +static enemies: unordered_set<Type>
    +static isPotion(t: Type): bool
    +static isGold(t: Type): bool
    +static isEnemy(t: Type): bool
}
class Effect {
  +{abstract} void apply(std::shared_ptr<PlayerCharacter> pc)
  +{abstract} void remove(std::shared_ptr<PlayerCharacter> pc)
  +{abstract} bool isTemporary() const
  +virtual ~Effect() = default
}

class BoostAtkEffect {
  +void apply(std::shared_ptr<PlayerCharacter> pc)
  +void remove(std::shared_ptr<PlayerCharacter> pc)
  +bool isTemporary() const
}

class WoundAtkEffect {
  +void apply(std::shared_ptr<PlayerCharacter> pc)
  +void remove(std::shared_ptr<PlayerCharacter> pc)
  +bool isTemporary() const
}

class BoostDefEffect {
  +void apply(std::shared_ptr<PlayerCharacter> pc)
  +void remove(std::shared_ptr<PlayerCharacter> pc)
  +bool isTemporary() const
}

class WoundDefEffect {
  +void apply(std::shared_ptr<PlayerCharacter> pc)
  +void remove(std::shared_ptr<PlayerCharacter> pc)
  +bool isTemporary() const
}

class RestoreHealthEffect {
  +void apply(std::shared_ptr<PlayerCharacter> pc)
  +void remove(std::shared_ptr<PlayerCharacter> pc)
  +bool isTemporary() const
}

class PoisonHealthEffect {
  +void apply(std::shared_ptr<PlayerCharacter> pc)
  +void remove(std::shared_ptr<PlayerCharacter> pc)
  +bool isTemporary() const
}

Effect <|-- BoostAtkEffect
Effect <|-- WoundAtkEffect
Effect <|-- BoostDefEffect
Effect <|-- WoundDefEffect
Effect <|-- RestoreHealthEffect
Effect <|-- PoisonHealthEffect

class EffectManager {
  -static std::shared_ptr<EffectManager> instance
  -static std::once_flag initFlag
  -std::vector<std::unique_ptr<Effect>> effects
  -EffectManager()
  +{static} std::shared_ptr<EffectManager> getInstance()
  +void addEffect(std::unique_ptr<Effect> effect)
  +void clearTemporaryEffects(PlayerCharacter& pc)
  +EffectManager(const EffectManager&) = delete
  +EffectManager& operator=(const EffectManager&) = delete
}

EffectManager --> Effect : manages

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

@enduml