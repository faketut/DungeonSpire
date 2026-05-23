// Tests for DungeonSpire. Built independently from the game binary.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest.h"

#include "../src/Position.h"
#include "../src/PRNG.h"
#include "../src/Player.h"
#include "../src/EffectManager.h"

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
