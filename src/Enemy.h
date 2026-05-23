#ifndef ENEMY_CC
#define ENEMY_CC
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <cmath>
#include <map>
#include "Player.h"
#include "Enum.h"
#include "Entity.h"
#include "Item.h"
#include "Position.h"

class Enemy:public Entity{
protected:
    // Enemy stat constants
    static constexpr int VAMPIRE_HP  = 50;
    static constexpr int VAMPIRE_ATK = 25;
    static constexpr int VAMPIRE_DEF = 25;
    static constexpr int WEREWOLF_HP  = 120;
    static constexpr int WEREWOLF_ATK = 30;
    static constexpr int WEREWOLF_DEF = 5;
    static constexpr int TROLL_HP  = 120;
    static constexpr int TROLL_ATK = 25;
    static constexpr int TROLL_DEF = 15;
    static constexpr int GOBLIN_HP  = 70;
    static constexpr int GOBLIN_ATK = 5;
    static constexpr int GOBLIN_DEF = 10;
    static constexpr int MERCHANT_HP  = 30;
    static constexpr int MERCHANT_ATK = 70;
    static constexpr int MERCHANT_DEF = 5;
    static constexpr int DRAGON_HP  = 150;
    static constexpr int DRAGON_ATK = 20;
    static constexpr int DRAGON_DEF = 20;
    static constexpr int PHOENIX_HP  = 50;
    static constexpr int PHOENIX_ATK = 35;
    static constexpr int PHOENIX_DEF = 20;

    Type enemyType;
    int maxHp, hp, atk, def;
    std::shared_ptr<Item> protectedItem;
    Position protectedItemPos;
public:
    Enemy(Type t)
        :  enemyType(t) {
        switch (t) {
            case Type::VAMPIRE:  maxHp = hp = VAMPIRE_HP;  atk = VAMPIRE_ATK;  def = VAMPIRE_DEF;  break;
            case Type::WEREWOLF: maxHp = hp = WEREWOLF_HP; atk = WEREWOLF_ATK; def = WEREWOLF_DEF; break;
            case Type::TROLL:    maxHp = hp = TROLL_HP;    atk = TROLL_ATK;    def = TROLL_DEF;    break;
            case Type::GOBLIN:   maxHp = hp = GOBLIN_HP;   atk = GOBLIN_ATK;   def = GOBLIN_DEF;   break;
            case Type::MERCHANT: maxHp = hp = MERCHANT_HP; atk = MERCHANT_ATK; def = MERCHANT_DEF; break;
            case Type::DRAGON:   maxHp = hp = DRAGON_HP;   atk = DRAGON_ATK;   def = DRAGON_DEF;   break;
            case Type::PHOENIX:  maxHp = hp = PHOENIX_HP;  atk = PHOENIX_ATK;  def = PHOENIX_DEF;  break;
            default: throw std::invalid_argument("Unknown enemy type");
        }
    }
    std::shared_ptr<Item> getProtectedItem() const{return protectedItem;}
    Position getProtectedItemPosition() const{return protectedItemPos;}
    void setProtectedItem(std::shared_ptr<Item> it, Position pos){protectedItem=it; protectedItemPos=pos;}
    std::map<std::string, int> getInfo() const {
        std::map<std::string, int> info;
        info["HP"] = hp;
        info["Atk"] = atk;
        info["Def"] = def;
        return info;
    }
    Type getType() const {return enemyType;}
    int getHp()const{return hp;}
    int getAtk()const{return atk;}
    int getDef()const{return def;}
    int getMaxHp()const{return maxHp;}
    void setHp(int delta) {
        hp += delta;
        if(hp > maxHp) hp = maxHp;
        if(hp < 0) hp = 0;
    }
    bool isDead() const { return hp<=0;}
    virtual void useSpecialAbility(PlayerCharacter& /*player*/) {}

};

class Vampire : public Enemy {
    static constexpr int STEAL_AMOUNT = 10;
public:
Vampire(): Enemy(Type::VAMPIRE) {}

    void useSpecialAbility(PlayerCharacter& player) override {
        player.setHp(-STEAL_AMOUNT);
        hp += STEAL_AMOUNT;
        std::cout << "Vampire drained " << STEAL_AMOUNT << " HP from the player and restored its own HP!" << std::endl;
    }
};

class Merchant : public Enemy {
public:
Merchant(): Enemy(Type::MERCHANT) {}
};

class Dragon : public Enemy {
public:
    Dragon() : Enemy(Type::DRAGON) {}
};
class Werewolf : public Enemy {
public:
Werewolf() : Enemy(Type::WEREWOLF) {}
};
class Troll : public Enemy {
    static constexpr int HEAL_AMOUNT = 5;
public:
Troll() : Enemy(Type::TROLL) {}
    void useSpecialAbility(PlayerCharacter& /*player*/) override {
        hp += HEAL_AMOUNT;
        if (hp > maxHp) hp = maxHp;
        std::cout << "Troll regenerates " << HEAL_AMOUNT << " HP using its natural healing ability!" << std::endl;

    }
};
class Phoenix : public Enemy {
    bool isRisen_ = false;
public:
    Phoenix() : Enemy(Type::PHOENIX) {}
    // Called by attackEnemy() when Phoenix dies; argument unused.
    void useSpecialAbility(PlayerCharacter& /*player*/) override {
        if (hp <= 0 && !isRisen_) {
            hp = maxHp / 2;
            isRisen_ = true;
            std::cout << "Phoenix rises from the ashes with half of its maximum HP (" << hp << ")!" << std::endl;
        } 
    }
    bool isRisen() const { return isRisen_; }
};
class Goblin : public Enemy {
    static constexpr int STEAL_AMOUNT = 5;
public:
    Goblin() : Enemy(Type::GOBLIN) {}
    void useSpecialAbility(PlayerCharacter& player) override {
        player.setGold(-STEAL_AMOUNT);
        std::cout << "Goblin steals " << STEAL_AMOUNT << " gold from the player!" << std::endl;
    }
};

class EnemyGenerator {
public:
    // WEREWOLF: 2/9 VAMPIRE: 3/18 GOBLIN: 5/18 TROLL: 1/9 PHOENIX: 1/9 MERCHANT: 1/9
    static std::shared_ptr<Enemy> generateEnemy() {
        int i = rand()%18;
        // 设置随机种子
        if (i < 4) {          // 2/9 (4/18): 狼人
            return std::make_shared<Werewolf>();
        } else if (i < 7) {   // 3/18: 吸血鬼
            return std::make_shared<Vampire>();
        } else if (i < 12) {  // 5/18: 哥布林
            return std::make_shared<Goblin>();
        } else if (i < 14) {  // 1/9 (2/18): 巨魔
            return std::make_shared<Troll>();
        } else if (i < 16) {  // 1/9 (2/18): 凤凰
            return std::make_shared<Phoenix>();
        } else {              // 1/9 (2/18): 商人
            return std::make_shared<Merchant>();
        }
    }
    static std::shared_ptr<Enemy> generateEnemy(Type t) {
        switch(t){
            case Type::WEREWOLF: return std::make_shared<Werewolf>();
            case Type::VAMPIRE: return std::make_shared<Vampire>();
            case Type::GOBLIN: return std::make_shared<Goblin>();
            case Type::PHOENIX: return std::make_shared<Phoenix>();
            case Type::MERCHANT: return std::make_shared<Merchant>();
            case Type::TROLL: return std::make_shared<Troll>();
            case Type::DRAGON: return std::make_shared<Dragon>();
            default: return nullptr;
        }
    }
    static std::shared_ptr<Enemy> generateDragon() {
        return std::make_shared<Dragon>();
    }
};
#endif
