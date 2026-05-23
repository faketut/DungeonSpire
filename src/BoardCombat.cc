#include "Board.h"
#include "Quest.h"
#include "QuestManager.h"
#include "EventBus.h"
#include <iostream>
#include <sstream>
#include <cmath>

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
    cc3k::EventBus::getInstance()->publish(cc3k::events::EnemyDied{
        static_cast<int>(enemy->getType()), pos.getX(), pos.getY()
    });
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

