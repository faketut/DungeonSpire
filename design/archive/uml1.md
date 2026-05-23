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
  - int visibility
  - int movementSpeed
  - std::unordered_map<Type, int> killCounts
  - std::unordered_map<Type, int> itemCounts
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
  + void modifyVisibility(int mod)
  + void modifyMovement(int mod)
  + int getVisibility() const
  + int getMovementSpeed() const
  + void incrementKillCount(Type enemyType)
  + int getKillCount(Type enemyType) const
  + void incrementItemCount(Type itemType)
  + int getItemCount(Type itemType) const
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


@enduml