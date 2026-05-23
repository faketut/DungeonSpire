#ifndef ENUM_CC
#define ENUM_CC
#include <unordered_map>
#include <unordered_set>
#include <string>

enum GameState { PLAYING, RESTART, QUIT, VICTORY, DEFEAT };

enum class Race {DWARF, ELF,ORC,HUMAN};
inline std::string toString(const Race r) {
    std::string s;
    switch(r){
        case Race::DWARF: s= std::string("Dwarf"); break;
        case Race::ELF: s= std::string("Elf");break;
        case Race::ORC: s= std::string("Orc");break;
        case Race::HUMAN: s= std::string("Human");break;
    }
    return s;
}

// Enum class to represent different types of tiles in the dungeon
enum class Type {
    RAIN, STORM, FOG,
    RH, BA, BD, PH, WA, WD,
    NORMAL_GOLD_PILE, SMALL_HOARD, DRAGON_HOARD, MERCHANT_HOARD,
    WEREWOLF, VAMPIRE, GOBLIN, TROLL, PHOENIX, MERCHANT, DRAGON, PLAYER,
    FLOOR, DOORWAY, PASSAGE, WALL,CEIL,STAIRWAY,EMPTY,UNKNOWN,
    COMPASS, BARRIER_SUIT
};

class TypeCategories {
public:
    // Define static sets outside the class
    static const std::unordered_set<Type> weather;
    static const std::unordered_set<Type> potions;
    static const std::unordered_set<Type> gold;
    static const std::unordered_set<Type> enemies;
    static const std::unordered_set<Type> traversable;
    static const std::unordered_set<Type> pickable;
    static const std::unordered_set<Type> mustVisible;
    // Check if a type belongs to a category
    static bool isPotion(Type t) { return potions.count(t) > 0; }
    static bool isGold(Type t) { return gold.count(t) > 0; }
    static bool isEnemy(Type t) { return enemies.count(t) > 0; }
    static bool isTraversable(Type t) { return traversable.count(t) > 0; }
    static bool isPickable(Type t) { return pickable.count(t) > 0; }
    static bool isMustVisible(Type t) { return mustVisible.count(t) > 0; }
};

// Initialize static members
inline const std::unordered_set<Type> TypeCategories::weather = {Type::RAIN, Type::STORM, Type::FOG};
inline const std::unordered_set<Type> TypeCategories::potions = {Type::RH, Type::BA, Type::BD, Type::PH, Type::WA, Type::WD};
inline const std::unordered_set<Type> TypeCategories::gold = {Type::NORMAL_GOLD_PILE, Type::SMALL_HOARD, Type::DRAGON_HOARD, Type::MERCHANT_HOARD};
inline const std::unordered_set<Type> TypeCategories::enemies = {Type::WEREWOLF, Type::VAMPIRE, Type::GOBLIN, Type::TROLL, Type::PHOENIX, Type::MERCHANT, Type::DRAGON};
inline const std::unordered_set<Type> TypeCategories::traversable = {Type::FLOOR, Type::DOORWAY, Type::PASSAGE, Type::NORMAL_GOLD_PILE, Type::SMALL_HOARD, Type::DRAGON_HOARD, Type::MERCHANT_HOARD};
inline const std::unordered_set<Type> TypeCategories::pickable = {Type::RH, Type::BA, Type::BD, Type::PH, Type::WA, Type::WD, Type::COMPASS, Type::BARRIER_SUIT, Type::NORMAL_GOLD_PILE, Type::SMALL_HOARD, Type::DRAGON_HOARD, Type::MERCHANT_HOARD};
inline const std::unordered_set<Type> TypeCategories::mustVisible = {Type::WALL,Type::CEIL};


inline std::string toString(Type t) {
    switch (t) {
        case Type::BA: return "BA";
        case Type::BD: return "BD";
        case Type::RH: return "RH";
        case Type::PH: return "PH";
        case Type::WA: return "WA";
        case Type::WD: return "WD";
        case Type::BARRIER_SUIT: return "Barrier Suit";
        case Type::COMPASS: return "Compass";
        case Type::SMALL_HOARD: return "Small Hoard";
        case Type::NORMAL_GOLD_PILE: return "Normal Gold Pile";
        case Type::DRAGON_HOARD: return "Dragon Hoard";
        case Type::MERCHANT_HOARD: return "Merchant Hoard";
        case Type::WEREWOLF: return "Werewolf";
        case Type::VAMPIRE: return "Vampire";
        case Type::GOBLIN: return "Goblin";
        case Type::TROLL: return "Troll";
        case Type::PHOENIX: return "Phoenix";
        case Type::MERCHANT: return "Merchant";
        case Type::DRAGON: return "Dragon";
        case Type::DOORWAY: return "Doorway";
        case Type::PASSAGE: return "Passage";
        case Type::WALL: return "Wall";
        case Type::CEIL: return "Ceil";
        case Type::FLOOR: return "Floor";
        case Type::EMPTY: return "Empty";
        case Type::PLAYER: return "Player";
        case Type::STAIRWAY: return "Stairway";
        case Type::RAIN: return "Rain";
        case Type::STORM: return "Storm";
        case Type::FOG: return "Fog";
        default: return "Unknown";
    }
}
inline Type toType(char c) {
    switch (c) {
        case ' ': return Type::EMPTY;                           // Empty tile
        case '@': return Type::PLAYER;
        case '\\': return Type::STAIRWAY; 
        case '0': return Type::RH;
        case '1': return Type::BA;     
        case '2': return Type::BD;     
        case '3': return Type::PH;     
        case '4': return Type::WA;     
        case '5': return Type::WD;                     
        case '6': return Type::NORMAL_GOLD_PILE;                         
        case '7': return Type::SMALL_HOARD;                   
        case '8': return Type::MERCHANT_HOARD;     
        case '9': return Type::DRAGON_HOARD;     
        case 'W': return Type::WEREWOLF;                        // WEREWOLF enemy
        case 'V': return Type::VAMPIRE;                         // VAMPIRE enemy
        case 'N': return Type::GOBLIN;                          // GOBLIN enemy
        case 'T': return Type::TROLL;                           // TROLL enemy
        case 'X': return Type::PHOENIX;                         // PHOENIX enemy
        case 'M': return Type::MERCHANT;                        // MERCHANT enemy
        case 'D': return Type::DRAGON;                          // DRAGON enemy
        case '+': return Type::DOORWAY;                        // Doorway
        case '#': return Type::PASSAGE;                         // Passage
        case '|': return Type::WALL;                            // Wall
        case '-': return Type::CEIL;                            // Ceiling
        case '.': return Type::FLOOR;                           // Floor
        case 'C': return Type::COMPASS;
        case 'B': return Type::BARRIER_SUIT;
        case 'R': return Type::RAIN;
        case 'S': return Type::STORM;
        case 'F': return Type::FOG;
        default: return Type::UNKNOWN;                          // Default to unknown type
    }
}
// load empty board
inline Type toTypeBoard(const char c) {
    Type t;
    switch(c){
        case ' ': t = Type::EMPTY; break;                       // Empty tile
        case '+': t = Type::DOORWAY; break;                     // Doorway
        case '#': t = Type::PASSAGE; break;                     // Passage
        case '|': t = Type::WALL; break;                        // Wall
        case '-': t = Type::CEIL; break;                        // Ceiling
        case '.': t = Type::FLOOR; break;                       // 
        default: t = Type::FLOOR; break;                        // 
    }
    return t;
}
enum class Direction {
    no,  // North
    so,  // South
    ea,  // East
    we,  // West
    ne,  // Northeast
    nw,  // Northwest
    se,  // Southeast
    sw   // Southwest
};
inline const std::unordered_set<std::string> moveCommands = 
    {"no", "so", "ea", "we", "ne", "nw", "se", "sw"};
inline Direction toDirection(const std::string& command) {
    static const std::unordered_map<std::string, Direction> DirectionMap = {
        {"no", Direction::no},
        {"so", Direction::so},
        {"ea", Direction::ea},
        {"we", Direction::we},
        {"ne", Direction::ne},
        {"nw", Direction::nw},
        {"se", Direction::se},
        {"sw", Direction::sw}
    };
    auto it = DirectionMap.find(command);
    if (it != DirectionMap.end()) {
        return it->second;
    } else {
        throw "Invalid direction command: " + command;
    }
}
inline std::string toString(Direction dir){
    switch (dir){
        case Direction::no: return "North" ;
        case Direction::so: return "South" ;
        case Direction::ea: return "East" ;
        case Direction::we: return "West" ;
        case Direction::ne: return "North East" ;
        case Direction::nw: return "North West" ;
        case Direction::se: return "South East" ;
        case Direction::sw: return "South West" ;
        default:
            return "Invalid Direction";
    }
}
inline char toChar(const Type t, bool compassPickedUp=false) {
    char c;
    switch (t) {
        case Type::EMPTY: c = ' '; break;                       // Empty tile
        //case Type::PLAYER: c = '@'; break;                      // Player character
        case Type::STAIRWAY: c = compassPickedUp? '\\':'.'; break;                     // Stairs
        case Type::RH: c = 'P'; break;                          // Restore health
        case Type::BA: c = 'P'; break;                          // Boost Atk
        case Type::BD: c = 'P'; break;                          // Boost Def
        case Type::PH: c = 'P'; break;                          // Poison health
        case Type::WA: c = 'P'; break;                          // Wound Atk
        case Type::WD: c = 'P'; break;                          // Wound Def
        case Type::COMPASS: c = 'C'; break; 
        case Type::BARRIER_SUIT: c='B';break;
        case Type::NORMAL_GOLD_PILE: c = 'G'; break;            // Normal gold pile
        case Type::SMALL_HOARD: c = 'G'; break;                 // Small hoard of gold
        case Type::MERCHANT_HOARD: c = 'G'; break;              // MERCHANT hoard
        case Type::DRAGON_HOARD: c = 'G'; break;                // DRAGON hoard of gold

        case Type::WEREWOLF: c = 'W'; break;              // WEREWOLF enemy
        case Type::VAMPIRE: c = 'V'; break;               // VAMPIRE enemy
        case Type::GOBLIN: c = 'N'; break;                // GOBLIN enemy
        case Type::TROLL: c = 'T'; break;                 // TROLL enemy
        case Type::PHOENIX: c = 'X'; break;               // PHOENIX enemy
        case Type::MERCHANT: c = 'M'; break;              // MERCHANT enemy
        case Type::DRAGON: c = 'D'; break;   
        case Type::DOORWAY: c = '+'; break;                     // Doorway
        case Type::PASSAGE: c = '#'; break;                     // Passage
        case Type::WALL: c = '|'; break;                        // Wall
        case Type::CEIL: c = '-'; break;                        // Ceiling
        case Type::FLOOR: c = '.'; break;                       // Floor
        case Type::RAIN: c = 'R'; break;
        case Type::STORM: c = 'S'; break;
        case Type::FOG: c = 'F'; break;
        default: c = ' '; break;                                    // Default to empty if unknown
    }
    return c;  
}
#define RESET "\033[0m"
// Display the PC and staircase in blue, so they are easy to spot. Enemies are displayed in red, treasure is displayed in yellow, and potions are displayed in green
inline const std::unordered_map<Type, std::string> colorMap = {
    {Type::STAIRWAY, "\033[34m"},        
    {Type::PLAYER, "\033[34m"},        
    {Type::VAMPIRE, "\033[31m"}, 
    {Type::WEREWOLF, "\033[31m"},
    {Type::GOBLIN, "\033[31m"},  
    {Type::MERCHANT, "\033[31m"},
    {Type::DRAGON, "\033[31m"},  
    {Type::PHOENIX, "\033[31m"}, 
    {Type::TROLL, "\033[31m"},   
    {Type::NORMAL_GOLD_PILE, "\033[33m"},
    {Type::SMALL_HOARD, "\033[33m"},      
    {Type::DRAGON_HOARD, "\033[33m"},     
    {Type::MERCHANT_HOARD, "\033[33m"},   
    {Type::BA, "\033[32m"},        
    {Type::BD, "\033[32m"},        
    {Type::RH, "\033[32m"},        
    {Type::PH, "\033[32m"},        
    {Type::WA, "\033[32m"},        
    {Type::WD, "\033[32m"},        
};
inline std::string getColor(Type t,bool compassPickedUp) {
    auto it = colorMap.find(t);
    if (it != colorMap.end()) {
        if(t==Type::STAIRWAY && !compassPickedUp) return "\033[37m";
        return it->second;
    }
    return RESET; // Default to reset if not found
}
#endif
