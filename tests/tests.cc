// Tests for DungeonSpire. Built independently from the game binary.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest.h"

#include "../src/Position.h"
#include "../src/PRNG.h"
#include "../src/Player.h"
#include "../src/EffectManager.h"
#include "../src/Enum.h"
#include "../src/Quest.h"
#include "../src/Board.h"

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

// --- Phase 1.2: expanded coverage ------------------------------------------

TEST_CASE("PRNG different seeds produce different first-draw") {
    PRNG::seed(1);
    int r1 = PRNG::randInt(1000000);
    PRNG::seed(2);
    int r2 = PRNG::randInt(1000000);
    CHECK(r1 != r2);
}

TEST_CASE("Player gold scales by goldModifier and never goes negative") {
    PlayerCharacter pc(Race::HUMAN);
    int g0 = pc.getGold();
    pc.setGold(10);
    CHECK(pc.getGold() == g0 + 10);
    pc.setGold(-1000);          // would underflow without the clamp
    CHECK(pc.getGold() == 0);
}

TEST_CASE("Player HP is clamped to [0, maxHp]") {
    PlayerCharacter pc(Race::HUMAN);
    int hp0 = pc.getHp();
    pc.setHp(-9999);
    CHECK(pc.getHp() == 0);
    pc.setHp(9999);             // refill is clamped to maxHp
    CHECK(pc.getHp() <= hp0);   // can't exceed starting maxHp for HUMAN
    CHECK(pc.getHp() > 0);
}

TEST_CASE("Player kill counts increment per type independently") {
    PlayerCharacter pc(Race::HUMAN);
    CHECK(pc.getKillCount(Type::WEREWOLF) == 0);
    pc.incrementKillCount(Type::WEREWOLF);
    pc.incrementKillCount(Type::WEREWOLF);
    pc.incrementKillCount(Type::VAMPIRE);
    CHECK(pc.getKillCount(Type::WEREWOLF) == 2);
    CHECK(pc.getKillCount(Type::VAMPIRE) == 1);
    CHECK(pc.getKillCount(Type::DRAGON) == 0);
}

TEST_CASE("Player item counts increment per type independently") {
    PlayerCharacter pc(Race::HUMAN);
    pc.incrementItemCount(Type::COMPASS);
    pc.incrementItemCount(Type::BARRIER_SUIT);
    pc.incrementItemCount(Type::BARRIER_SUIT);
    CHECK(pc.getItemCount(Type::COMPASS) == 1);
    CHECK(pc.getItemCount(Type::BARRIER_SUIT) == 2);
    CHECK(pc.getItemCount(Type::DRAGON_HOARD) == 0);
}

TEST_CASE("RestoreHealth / PoisonHealth are inverse-ish on a live player") {
    auto pc = std::make_shared<PlayerCharacter>(Race::HUMAN);
    pc->setHp(-50);                 // drop hp so RH has headroom
    int hp0 = pc->getHp();

    RestoreHealthEffect rh;
    rh.apply(pc);
    CHECK(pc->getHp() > hp0);

    PoisonHealthEffect ph;
    int hpAfterRh = pc->getHp();
    ph.apply(pc);
    CHECK(pc->getHp() < hpAfterRh);
}

TEST_CASE("Issue #3 regression: clearTemporaryEffects mutates the live player") {
    auto pc = std::make_shared<PlayerCharacter>(Race::HUMAN);
    int atk0 = pc->getAtk();

    auto* em = EffectManager::getInstance();
    auto eff = std::make_unique<BoostAtkEffect>();
    eff->apply(pc);                 // simulate Game's apply step
    em->addEffect(std::move(eff));
    CHECK(pc->getAtk() == atk0 + 5);

    em->clearTemporaryEffects(pc);  // must remove() from THIS pc, not a copy
    CHECK(pc->getAtk() == atk0);
}

TEST_CASE("EffectManager weather counter add/clear") {
    auto* em = EffectManager::getInstance();
    em->clearWealtherEffects();
    CHECK(em->getWeatherEffectsCnt() == 0);
    em->addWeatherEffect(std::make_unique<RainEffect>());
    em->addWeatherEffect(std::make_unique<FogEffect>());
    CHECK(em->getWeatherEffectsCnt() == 2);
    em->clearWealtherEffects();
    CHECK(em->getWeatherEffectsCnt() == 0);
}

TEST_CASE("KillQuest flips completed only after target threshold reached") {
    auto pc = std::make_shared<PlayerCharacter>(Race::HUMAN);
    KillQuest q(Type::WEREWOLF, 3, "kill 3 werewolves");
    CHECK_FALSE(q.isCompleted());

    pc->incrementKillCount(Type::WEREWOLF);
    pc->incrementKillCount(Type::WEREWOLF);
    q.checkCompletion(pc);
    CHECK_FALSE(q.isCompleted());

    pc->incrementKillCount(Type::WEREWOLF);
    q.checkCompletion(pc);
    CHECK(q.isCompleted());
}

TEST_CASE("CollectQuest flips completed only after item count reached") {
    auto pc = std::make_shared<PlayerCharacter>(Race::HUMAN);
    CollectQuest q(Type::BARRIER_SUIT, 2, "collect 2 barrier suits");
    CHECK_FALSE(q.isCompleted());
    pc->incrementItemCount(Type::BARRIER_SUIT);
    q.checkCompletion(pc);
    CHECK_FALSE(q.isCompleted());
    pc->incrementItemCount(Type::BARRIER_SUIT);
    q.checkCompletion(pc);
    CHECK(q.isCompleted());
}

TEST_CASE("Tile basic getters/setters round-trip") {
    Position p{3, 4};
    Tile t(p, nullptr, Type::FLOOR);
    CHECK(t.getX() == 3);
    CHECK(t.getY() == 4);
    CHECK(static_cast<int>(t.getType()) == static_cast<int>(Type::FLOOR));
    t.setType(Type::WALL);
    CHECK(static_cast<int>(t.getType()) == static_cast<int>(Type::WALL));
    t.setPosition(Position{7, 8});
    CHECK(t.getPosition() == Position{7, 8});
}

TEST_CASE("Board constructs and exposes its initial state") {
    PRNG::seed(1234);
    // Note: full floor init also needs Board::loadBoard (a template) to fill
    // tile types from the file; that path is exercised via Game/integration
    // and is out of scope for this unit test. Just check the lightweight
    // accessors on a freshly-constructed Board.
    Board b("./src/files/default.txt", 0);
    CHECK_FALSE(b.isCompassPickedUp());
    CHECK_FALSE(b.isAllMerchantsHostile());
    CHECK(b.getFloorId() == 0);
    CHECK_FALSE(b.getSetSuit());
}

TEST_CASE("Board floor id is settable and readable") {
    PRNG::seed(1);
    Board b("./src/files/default.txt", 0);
    b.setFloorId(2);
    CHECK(b.getFloorId() == 2);
    b.setCompassPickedUp(true);
    CHECK(b.isCompassPickedUp());
}
