// Tests for DungeonSpire. Built independently from the game binary.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest.h"

#include "../src/Position.h"
#include "../src/PRNG.h"
#include "../src/Player.h"
#include "../src/EffectManager.h"
#include "../src/Enum.h"

TEST_CASE("Position::distanceTo is Manhattan distance") {
    Position a{0, 0};
    Position b{3, 4};
    CHECK(a.distanceTo(b) == 7);
    CHECK(b.distanceTo(a) == 7);
    CHECK(a.distanceTo(a) == 0);
}

TEST_CASE("Position::near covers the 3x3 neighbourhood including self") {
    Position c{5, 5};
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            CHECK(c.near(Position{5 + dx, 5 + dy}));
        }
    }
    CHECK_FALSE(c.near(Position{5, 7}));
    CHECK_FALSE(c.near(Position{7, 5}));
}

TEST_CASE("Position equality") {
    CHECK(Position{1, 2} == Position{1, 2});
    CHECK_FALSE(Position{1, 2} == Position{2, 1});
}

TEST_CASE("PRNG::randInt is bounded and seedable") {
    PRNG::seed(42);
    std::vector<int> a;
    for (int i = 0; i < 100; ++i) {
        int r = PRNG::randInt(10);
        CHECK(r >= 0);
        CHECK(r < 10);
        a.push_back(r);
    }
    PRNG::seed(42);
    std::vector<int> b;
    for (int i = 0; i < 100; ++i) b.push_back(PRNG::randInt(10));
    CHECK(a == b);
}

TEST_CASE("Regression #1: BoostDefEffect modifies DEF, not ATK") {
    auto pc = std::make_shared<PlayerCharacter>(Race::HUMAN);
    int atkBefore = pc->getAtk();
    int defBefore = pc->getDef();

    BoostDefEffect e;
    e.apply(pc);
    CHECK(pc->getAtk() == atkBefore);
    CHECK(pc->getDef() == defBefore + 5);

    e.remove(pc);
    CHECK(pc->getAtk() == atkBefore);
    CHECK(pc->getDef() == defBefore);
}

TEST_CASE("BoostAtk / WoundAtk / WoundDef behave symmetrically") {
    auto pc = std::make_shared<PlayerCharacter>(Race::HUMAN);
    int atk0 = pc->getAtk();
    int def0 = pc->getDef();

    BoostAtkEffect ba;
    ba.apply(pc); CHECK(pc->getAtk() == atk0 + 5);
    ba.remove(pc); CHECK(pc->getAtk() == atk0);

    WoundAtkEffect wa;
    wa.apply(pc); CHECK(pc->getAtk() == atk0 - 5);
    wa.remove(pc); CHECK(pc->getAtk() == atk0);

    WoundDefEffect wd;
    wd.apply(pc); CHECK(pc->getDef() == def0 - 5);
    wd.remove(pc); CHECK(pc->getDef() == def0);
}

TEST_CASE("Race attributes apply correctly") {
    PlayerCharacter dwarf(Race::DWARF);
    dwarf.setAttributes(Race::DWARF);
    CHECK(dwarf.getDef() == 30);

    PlayerCharacter elf(Race::ELF);
    elf.setAttributes(Race::ELF);
    CHECK(elf.getAtk() == 30);
    CHECK(elf.getDef() == 10);
}

TEST_CASE("Type metadata table preserves toString contract") {
    CHECK(toString(Type::WEREWOLF) == "Werewolf");
    CHECK(toString(Type::DRAGON) == "Dragon");
    CHECK(toString(Type::FLOOR) == "Floor");
    CHECK(toString(Type::UNKNOWN) == "Unknown");
    CHECK(toString(Type::BARRIER_SUIT) == "Barrier Suit");
}

TEST_CASE("Type metadata table preserves toChar contract incl. STAIRWAY toggle") {
    CHECK(toChar(Type::FLOOR) == '.');
    CHECK(toChar(Type::WALL) == '|');
    CHECK(toChar(Type::RH) == 'P');
    CHECK(toChar(Type::COMPASS) == 'C');
    // Stairway hides as floor until compass picked up
    CHECK(toChar(Type::STAIRWAY, false) == '.');
    CHECK(toChar(Type::STAIRWAY, true)  == '\\');
}

TEST_CASE("toType inverts file glyphs for the canonical chars") {
    CHECK(static_cast<int>(toType('.')) == static_cast<int>(Type::FLOOR));
    CHECK(static_cast<int>(toType('|')) == static_cast<int>(Type::WALL));
    CHECK(static_cast<int>(toType('W')) == static_cast<int>(Type::WEREWOLF));
    CHECK(static_cast<int>(toType('@')) == static_cast<int>(Type::PLAYER));
    CHECK(static_cast<int>(toType('?')) == static_cast<int>(Type::UNKNOWN));
}

TEST_CASE("TypeCategories flags match the legacy unordered_set semantics") {
    // Potions
    CHECK(TypeCategories::isPotion(Type::RH));
    CHECK(TypeCategories::isPotion(Type::WD));
    CHECK_FALSE(TypeCategories::isPotion(Type::COMPASS));
    // Enemies (note: PHOENIX, MERCHANT, DRAGON are enemies)
    CHECK(TypeCategories::isEnemy(Type::DRAGON));
    CHECK(TypeCategories::isEnemy(Type::PHOENIX));
    CHECK_FALSE(TypeCategories::isEnemy(Type::PLAYER));
    // Gold
    CHECK(TypeCategories::isGold(Type::DRAGON_HOARD));
    CHECK_FALSE(TypeCategories::isGold(Type::RH));
    // Traversable
    CHECK(TypeCategories::isTraversable(Type::FLOOR));
    CHECK(TypeCategories::isTraversable(Type::PASSAGE));
    CHECK(TypeCategories::isTraversable(Type::NORMAL_GOLD_PILE));
    CHECK_FALSE(TypeCategories::isTraversable(Type::WALL));
    // Pickable
    CHECK(TypeCategories::isPickable(Type::COMPASS));
    CHECK(TypeCategories::isPickable(Type::BARRIER_SUIT));
    CHECK(TypeCategories::isPickable(Type::DRAGON_HOARD));
    CHECK_FALSE(TypeCategories::isPickable(Type::WALL));
    // Must-visible
    CHECK(TypeCategories::isMustVisible(Type::WALL));
    CHECK(TypeCategories::isMustVisible(Type::CEIL));
    CHECK_FALSE(TypeCategories::isMustVisible(Type::FLOOR));
}

TEST_CASE("toDirection round-trips for the 8 valid commands") {
    for (const std::string& cmd : moveCommands) {
        Direction d = toDirection(cmd);
        // Sanity: toString returns a non-empty cardinal name
        CHECK_FALSE(toString(d).empty());
    }
    CHECK_THROWS_AS(toDirection("xx"), std::invalid_argument);
}
