#ifndef ENUM_H
#define ENUM_H
#include <array>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

enum GameState { PLAYING, RESTART, QUIT, VICTORY, DEFEAT };

enum class Race { DWARF, ELF, ORC, HUMAN };
inline std::string toString(const Race r) {
    switch (r) {
        case Race::DWARF: return "Dwarf";
        case Race::ELF:   return "Elf";
        case Race::ORC:   return "Orc";
        case Race::HUMAN: return "Human";
    }
    return "Unknown";
}

// Enum class to represent different types of tiles in the dungeon.
// IMPORTANT: the order here is the index into kTypeTable below — keep them in sync.
enum class Type {
    RAIN, STORM, FOG,
    RH, BA, BD, PH, WA, WD,
    NORMAL_GOLD_PILE, SMALL_HOARD, DRAGON_HOARD, MERCHANT_HOARD,
    WEREWOLF, VAMPIRE, GOBLIN, TROLL, PHOENIX, MERCHANT, DRAGON, PLAYER,
    FLOOR, DOORWAY, PASSAGE, WALL, CEIL, STAIRWAY, EMPTY, UNKNOWN,
    COMPASS, BARRIER_SUIT
};
constexpr int kTypeCount = 31;

// ANSI color codes used in rendering
#define RESET "\033[0m"
namespace TypeColor {
    inline constexpr const char* RED    = "\033[31m";
    inline constexpr const char* GREEN  = "\033[32m";
    inline constexpr const char* YELLOW = "\033[33m";
    inline constexpr const char* BLUE   = "\033[34m";
    inline constexpr const char* WHITE  = "\033[37m";
}

// Single source of truth for per-Type metadata. Order MUST match enum Type.
struct TypeMeta {
    const char* name;      // toString
    char glyph;            // toChar (no compass override)
    const char* color;     // nullptr if none
    // Category flags
    bool potion;
    bool gold;
    bool enemy;
    bool traversable;
    bool pickable;
    bool mustVisible;
    bool weather;
};

inline constexpr std::array<TypeMeta, kTypeCount> kTypeTable = {{
    // name              glyph color                     pot   gold  enem  trav  pick  vis   weat
    {"Rain",             'R',  nullptr,                  false,false,false,false,false,false,true },  // RAIN
    {"Storm",            'S',  nullptr,                  false,false,false,false,false,false,true },  // STORM
    {"Fog",              'F',  nullptr,                  false,false,false,false,false,false,true },  // FOG
    {"RH",               'P',  TypeColor::GREEN,         true, false,false,false,true, false,false},  // RH
    {"BA",               'P',  TypeColor::GREEN,         true, false,false,false,true, false,false},  // BA
    {"BD",               'P',  TypeColor::GREEN,         true, false,false,false,true, false,false},  // BD
    {"PH",               'P',  TypeColor::GREEN,         true, false,false,false,true, false,false},  // PH
    {"WA",               'P',  TypeColor::GREEN,         true, false,false,false,true, false,false},  // WA
    {"WD",               'P',  TypeColor::GREEN,         true, false,false,false,true, false,false},  // WD
    {"Normal Gold Pile", 'G',  TypeColor::YELLOW,        false,true, false,true, true, false,false},  // NORMAL_GOLD_PILE
    {"Small Hoard",      'G',  TypeColor::YELLOW,        false,true, false,true, true, false,false},  // SMALL_HOARD
    {"Dragon Hoard",     'G',  TypeColor::YELLOW,        false,true, false,true, true, false,false},  // DRAGON_HOARD
    {"Merchant Hoard",   'G',  TypeColor::YELLOW,        false,true, false,true, true, false,false},  // MERCHANT_HOARD
    {"Werewolf",         'W',  TypeColor::RED,           false,false,true, false,false,false,false},  // WEREWOLF
    {"Vampire",          'V',  TypeColor::RED,           false,false,true, false,false,false,false},  // VAMPIRE
    {"Goblin",           'N',  TypeColor::RED,           false,false,true, false,false,false,false},  // GOBLIN
    {"Troll",            'T',  TypeColor::RED,           false,false,true, false,false,false,false},  // TROLL
    {"Phoenix",          'X',  TypeColor::RED,           false,false,true, false,false,false,false},  // PHOENIX
    {"Merchant",         'M',  TypeColor::RED,           false,false,true, false,false,false,false},  // MERCHANT
    {"Dragon",           'D',  TypeColor::RED,           false,false,true, false,false,false,false},  // DRAGON
    {"Player",           ' ',  TypeColor::BLUE,          false,false,false,false,false,false,false},  // PLAYER (rendered specially)
    {"Floor",            '.',  nullptr,                  false,false,false,true, false,false,false},  // FLOOR
    {"Doorway",          '+',  nullptr,                  false,false,false,true, false,false,false},  // DOORWAY
    {"Passage",          '#',  nullptr,                  false,false,false,true, false,false,false},  // PASSAGE
    {"Wall",             '|',  nullptr,                  false,false,false,false,false,true, false},  // WALL
    {"Ceil",             '-',  nullptr,                  false,false,false,false,false,true, false},  // CEIL
    {"Stairway",         '.',  TypeColor::BLUE,          false,false,false,false,false,false,false},  // STAIRWAY (glyph/color depend on compass)
    {"Empty",            ' ',  nullptr,                  false,false,false,false,false,false,false},  // EMPTY
    {"Unknown",          ' ',  nullptr,                  false,false,false,false,false,false,false},  // UNKNOWN
    {"Compass",          'C',  nullptr,                  false,false,false,false,true, false,false},  // COMPASS
    {"Barrier Suit",     'B',  nullptr,                  false,false,false,false,true, false,false},  // BARRIER_SUIT
}};

inline const TypeMeta& meta(Type t) {
    return kTypeTable[static_cast<int>(t)];
}

inline std::string toString(Type t) { return meta(t).name; }

inline char toChar(const Type t, bool compassPickedUp = false) {
    if (t == Type::STAIRWAY) return compassPickedUp ? '\\' : '.';
    return meta(t).glyph;
}

inline std::string getColor(Type t, bool compassPickedUp) {
    if (t == Type::STAIRWAY && !compassPickedUp) return TypeColor::WHITE;
    const char* c = meta(t).color;
    return c ? std::string(c) : std::string(RESET);
}

class TypeCategories {
public:
    static bool isPotion(Type t)      { return meta(t).potion; }
    static bool isGold(Type t)        { return meta(t).gold; }
    static bool isEnemy(Type t)       { return meta(t).enemy; }
    static bool isTraversable(Type t) { return meta(t).traversable; }
    static bool isPickable(Type t)    { return meta(t).pickable; }
    static bool isMustVisible(Type t) { return meta(t).mustVisible; }
    static bool isWeather(Type t)     { return meta(t).weather; }
};

// Char → Type for file loading. Multiple Types may share a glyph (e.g. all
// potions render as 'P'), so this is a separate small map; it is *not* the
// inverse of toChar.
inline Type toType(char c) {
    switch (c) {
        case ' ': return Type::EMPTY;
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
        case 'W': return Type::WEREWOLF;
        case 'V': return Type::VAMPIRE;
        case 'N': return Type::GOBLIN;
        case 'T': return Type::TROLL;
        case 'X': return Type::PHOENIX;
        case 'M': return Type::MERCHANT;
        case 'D': return Type::DRAGON;
        case '+': return Type::DOORWAY;
        case '#': return Type::PASSAGE;
        case '|': return Type::WALL;
        case '-': return Type::CEIL;
        case '.': return Type::FLOOR;
        case 'C': return Type::COMPASS;
        case 'B': return Type::BARRIER_SUIT;
        case 'R': return Type::RAIN;
        case 'S': return Type::STORM;
        case 'F': return Type::FOG;
        default:  return Type::UNKNOWN;
    }
}

// "Empty board" loader: only structural tiles + spaces matter; everything
// else is treated as FLOOR.
inline Type toTypeBoard(const char c) {
    switch (c) {
        case ' ': return Type::EMPTY;
        case '+': return Type::DOORWAY;
        case '#': return Type::PASSAGE;
        case '|': return Type::WALL;
        case '-': return Type::CEIL;
        case '.': return Type::FLOOR;
        default:  return Type::FLOOR;
    }
}

enum class Direction { no, so, ea, we, ne, nw, se, sw };

inline const std::unordered_set<std::string> moveCommands =
    {"no", "so", "ea", "we", "ne", "nw", "se", "sw"};

inline Direction toDirection(const std::string& command) {
    static const std::unordered_map<std::string, Direction> DirectionMap = {
        {"no", Direction::no}, {"so", Direction::so},
        {"ea", Direction::ea}, {"we", Direction::we},
        {"ne", Direction::ne}, {"nw", Direction::nw},
        {"se", Direction::se}, {"sw", Direction::sw},
    };
    auto it = DirectionMap.find(command);
    if (it == DirectionMap.end()) {
        throw std::invalid_argument("Invalid direction command: " + command);
    }
    return it->second;
}

inline std::string toString(Direction dir) {
    switch (dir) {
        case Direction::no: return "North";
        case Direction::so: return "South";
        case Direction::ea: return "East";
        case Direction::we: return "West";
        case Direction::ne: return "North East";
        case Direction::nw: return "North West";
        case Direction::se: return "South East";
        case Direction::sw: return "South West";
    }
    return "Invalid Direction";
}

#endif
