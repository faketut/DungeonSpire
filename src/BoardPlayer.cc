#include "Board.h"
#include "Quest.h"
#include "QuestManager.h"
#include "EventBus.h"
#include <iostream>
#include <sstream>
#include <cmath>

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
    cc3k::EventBus::getInstance()->publish(cc3k::events::ItemPickedUp{
        static_cast<int>(tileType), pos.getX(), pos.getY()
    });
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
