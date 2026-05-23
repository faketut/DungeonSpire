#ifndef POSITION_CC
#define POSITION_CC
#include <algorithm>
class Position{
protected:
    int x,y;
public:
    Position():x(0),y(0){}
    Position(int x,int y):x(x),y(y){}
    int getX() const{return x;}
    int getY() const{return y;}
    Position getPosition() const{return *this;}
    void setX(int X) {x=X;}
    void setY(int Y) {y=Y;}
    void setPosition(Position pos) {x=pos.x;y=pos.y;}
    bool near(const Position& other) const {
        return std::abs(x - other.getX()) <= 1 && std::abs(y - other.getY()) <= 1;
    }
    bool operator==(const Position& other) const {
        return getX() == other.getX() && getY() == other.getY();
    }
    int distanceTo(const Position& other) const {
        int dx = x - other.x;
        int dy = y - other.y;
        return abs(dx)+abs(dy);
    }
};
#endif
