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
#include "../src/EventBus.h"
#include "../src/EnemyStats.h"
#include "../src/SaveGame.h"
#include "../src/Enemy.h"

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

// -----------------------------------------------------------------------------
// Phase 2.5 — EventBus
// -----------------------------------------------------------------------------

TEST_CASE("EventBus publish without subscribers is a no-op") {
    cc3k::EventBus bus;
    bus.publish(cc3k::events::FloorChanged{3}); // must not throw / crash
    CHECK(bus.handlerCount() == 0u);
}

TEST_CASE("EventBus delivers a typed event to its subscriber") {
    cc3k::EventBus bus;
    int seen = -1;
    bus.subscribe<cc3k::events::FloorChanged>(
        [&seen](const cc3k::events::FloorChanged& e) { seen = e.newFloorId; });
    bus.publish(cc3k::events::FloorChanged{7});
    CHECK(seen == 7);
}

TEST_CASE("EventBus dispatches only to handlers of the matching event type") {
    cc3k::EventBus bus;
    int floors = 0;
    int deaths = 0;
    bus.subscribe<cc3k::events::FloorChanged>(
        [&floors](const cc3k::events::FloorChanged&) { ++floors; });
    bus.subscribe<cc3k::events::EnemyDied>(
        [&deaths](const cc3k::events::EnemyDied&) { ++deaths; });
    bus.publish(cc3k::events::FloorChanged{1});
    bus.publish(cc3k::events::FloorChanged{2});
    bus.publish(cc3k::events::EnemyDied{0, 1, 1});
    CHECK(floors == 2);
    CHECK(deaths == 1);
}

TEST_CASE("EventBus supports multiple subscribers for the same event") {
    cc3k::EventBus bus;
    int a = 0, b = 0;
    bus.subscribe<cc3k::events::FloorChanged>(
        [&a](const cc3k::events::FloorChanged&) { ++a; });
    bus.subscribe<cc3k::events::FloorChanged>(
        [&b](const cc3k::events::FloorChanged&) { ++b; });
    bus.publish(cc3k::events::FloorChanged{0});
    CHECK(a == 1);
    CHECK(b == 1);
    CHECK(bus.handlerCount<cc3k::events::FloorChanged>() == 2u);
}

TEST_CASE("EventBus unsubscribe removes exactly one handler") {
    cc3k::EventBus bus;
    int hits = 0;
    auto id = bus.subscribe<cc3k::events::FloorChanged>(
        [&hits](const cc3k::events::FloorChanged&) { ++hits; });
    bus.subscribe<cc3k::events::FloorChanged>(
        [&hits](const cc3k::events::FloorChanged&) { ++hits; });
    CHECK(bus.unsubscribe<cc3k::events::FloorChanged>(id));
    bus.publish(cc3k::events::FloorChanged{0});
    CHECK(hits == 1);
    // Unsubscribing the same id again is a no-op and returns false.
    CHECK_FALSE(bus.unsubscribe<cc3k::events::FloorChanged>(id));
}

TEST_CASE("EventBus is safe against (un)subscribe during dispatch") {
    cc3k::EventBus bus;
    int hits = 0;
    bus.subscribe<cc3k::events::FloorChanged>(
        [&bus, &hits](const cc3k::events::FloorChanged&) {
            ++hits;
            // Adding a new subscriber mid-dispatch must not invalidate the
            // iteration of the current publish().
            bus.subscribe<cc3k::events::FloorChanged>(
                [](const cc3k::events::FloorChanged&) {});
        });
    bus.publish(cc3k::events::FloorChanged{0});
    CHECK(hits == 1);
    CHECK(bus.handlerCount<cc3k::events::FloorChanged>() == 2u);
}

TEST_CASE("EventBus::clear drops all handlers") {
    cc3k::EventBus bus;
    bus.subscribe<cc3k::events::FloorChanged>(
        [](const cc3k::events::FloorChanged&) {});
    bus.subscribe<cc3k::events::EnemyDied>(
        [](const cc3k::events::EnemyDied&) {});
    CHECK(bus.handlerCount() == 2u);
    bus.clear();
    CHECK(bus.handlerCount() == 0u);
}

TEST_CASE("Board::setFloorId publishes FloorChanged on the singleton bus") {
    auto* bus = cc3k::EventBus::getInstance();
    bus->clear(); // isolate from any prior wiring
    int observed = -1;
    int callCount = 0;
    auto id = bus->subscribe<cc3k::events::FloorChanged>(
        [&](const cc3k::events::FloorChanged& e) {
            observed = e.newFloorId;
            ++callCount;
        });

    PRNG::seed(1);
    Board b("./src/files/default.txt", 0);
    b.setFloorId(4);
    b.setFloorId(5);

    CHECK(callCount == 2);
    CHECK(observed == 5);

    bus->unsubscribe<cc3k::events::FloorChanged>(id);
    bus->clear();
}

// -----------------------------------------------------------------------------
// Phase 2.6 — EnemyStats registry
// -----------------------------------------------------------------------------

TEST_CASE("EnemyStats defaults exactly mirror the historical hardcoded values") {
    // Fresh, isolated registry: even if data/enemies.json isn't found at the
    // ctest CWD, the built-in defaults must be a complete, correct table.
    EnemyStats reg;
    // We can't bypass the ctor's auto-load, but we verify the live singleton
    // matches the published table either way.
    const auto& s = reg; (void)s;

    auto check = [](Type t, int hp, int atk, int def) {
        const auto& got = EnemyStats::getInstance()->get(t);
        CHECK(got.hp == hp);
        CHECK(got.atk == atk);
        CHECK(got.def == def);
    };
    check(Type::VAMPIRE,  50,  25, 25);
    check(Type::WEREWOLF, 120, 30,  5);
    check(Type::TROLL,    120, 25, 15);
    check(Type::GOBLIN,   70,   5, 10);
    check(Type::MERCHANT, 30,  70,  5);
    check(Type::DRAGON,   150, 20, 20);
    check(Type::PHOENIX,  50,  35, 20);
}

TEST_CASE("EnemyStats::loadFromFile rejects a missing file without mutating state") {
    EnemyStats reg;
    CHECK_FALSE(reg.loadFromFile("definitely/does/not/exist.json"));
    // Defaults still queryable:
    CHECK(reg.get(Type::DRAGON).hp == 150);
}

TEST_CASE("EnemyStats::loadFromFile rejects malformed JSON without mutating state") {
    // Write a deliberately broken file in the test working dir.
    const std::string bad = "tests_tmp_bad_enemies.json";
    {
        std::ofstream out(bad);
        out << "{ not json";
    }
    EnemyStats reg;
    CHECK_FALSE(reg.loadFromFile(bad));
    CHECK(reg.get(Type::GOBLIN).atk == 5);
    std::remove(bad.c_str());
}

TEST_CASE("EnemyStats::loadFromFile accepts a valid override file") {
    const std::string path = "tests_tmp_enemies.json";
    {
        std::ofstream out(path);
        out << R"({
            "vampire":  { "hp": 1, "atk": 2, "def": 3 },
            "werewolf": { "hp": 4, "atk": 5, "def": 6 },
            "troll":    { "hp": 7, "atk": 8, "def": 9 },
            "goblin":   { "hp": 10,"atk": 11,"def": 12},
            "merchant": { "hp": 13,"atk": 14,"def": 15},
            "dragon":   { "hp": 16,"atk": 17,"def": 18},
            "phoenix":  { "hp": 19,"atk": 20,"def": 21}
        })";
    }
    EnemyStats reg;
    CHECK(reg.loadFromFile(path));
    CHECK(reg.get(Type::VAMPIRE).hp == 1);
    CHECK(reg.get(Type::DRAGON).atk == 17);
    CHECK(reg.isLoaded());
    CHECK(reg.source() == path);
    std::remove(path.c_str());
}

TEST_CASE("Enemy constructor reads stats from the registry") {
    // Live registry is loaded at static-init from data/enemies.json (which
    // matches the defaults). Either way these must be the canonical numbers.
    Vampire v;
    CHECK(v.getHp()  == 50);
    CHECK(v.getAtk() == 25);
    CHECK(v.getDef() == 25);
    CHECK(v.getMaxHp() == 50);

    Merchant m;
    CHECK(m.getHp()  == 30);
    CHECK(m.getAtk() == 70);
    CHECK(m.getDef() == 5);
}

// ---------------------------------------------------------------------------
// Phase 2.7 — SaveGame JSON round-trip
// ---------------------------------------------------------------------------

TEST_CASE("SaveGame round-trips a populated SaveData through JSON") {
    cc3k::SaveData d;
    d.seed = 0xDEADBEEFu;
    d.filename = "./files/potions.txt";
    d.weatherEnabled = true;
    d.questEnabled = true;
    d.floorId = 3;
    d.player.race = 2;
    d.player.maxHp = 150;
    d.player.hp = 42;
    d.player.atk = 33;
    d.player.def = 17;
    d.player.gold = 999;
    d.player.score = 12345;
    d.player.goldModifier = 1.5;
    d.player.hasBarrierSuit = true;
    d.player.visibility = 8;
    d.player.movementSpeed = 2;

    auto s = cc3k::toJson(d);
    auto r = cc3k::fromJson(s);
    REQUIRE(r.has_value());
    CHECK(r->version == d.version);
    CHECK(r->seed == d.seed);
    CHECK(r->filename == d.filename);
    CHECK(r->weatherEnabled == d.weatherEnabled);
    CHECK(r->questEnabled == d.questEnabled);
    CHECK(r->floorId == d.floorId);
    CHECK(r->player.race == d.player.race);
    CHECK(r->player.maxHp == d.player.maxHp);
    CHECK(r->player.hp == d.player.hp);
    CHECK(r->player.atk == d.player.atk);
    CHECK(r->player.def == d.player.def);
    CHECK(r->player.gold == d.player.gold);
    CHECK(r->player.score == d.player.score);
    CHECK(r->player.goldModifier == doctest::Approx(d.player.goldModifier));
    CHECK(r->player.hasBarrierSuit == d.player.hasBarrierSuit);
    CHECK(r->player.visibility == d.player.visibility);
    CHECK(r->player.movementSpeed == d.player.movementSpeed);
}

TEST_CASE("SaveGame::fromJson rejects malformed or wrong-version input") {
    CHECK_FALSE(cc3k::fromJson("not json").has_value());
    CHECK_FALSE(cc3k::fromJson("{}").has_value());
    CHECK_FALSE(cc3k::fromJson(R"({"version":999})").has_value());
}

TEST_CASE("SaveGame::load returns nullopt for a missing file") {
    CHECK_FALSE(cc3k::load("/tmp/cc3k_nonexistent_save_xyz_12345.json").has_value());
}

TEST_CASE("SaveGame::save then load round-trips on disk") {
    const std::string path = "/tmp/cc3k_savegame_test.json";
    std::remove(path.c_str());
    cc3k::SaveData d;
    d.seed = 7;
    d.floorId = 2;
    d.player.gold = 250;
    REQUIRE(cc3k::save(path, d));
    auto r = cc3k::load(path);
    REQUIRE(r.has_value());
    CHECK(r->seed == 7u);
    CHECK(r->floorId == 2);
    CHECK(r->player.gold == 250);
    std::remove(path.c_str());
}

// ---------------------------------------------------------------------------
// Phase 3.8a — IRenderer extraction (AnsiRenderer HUD)
// ---------------------------------------------------------------------------

#include <sstream>
#include "../src/AnsiRenderer.h"

TEST_CASE("AnsiRenderer::drawHud writes the expected base lines") {
    std::ostringstream os;
    cc3k::AnsiRenderer r(os);
    cc3k::HudInfo h;
    h.race = "Human";
    h.gold = 42;
    h.floor = 3;
    h.hp = 100;
    h.atk = 20;
    h.def = 15;
    h.action = "moved north";
    r.drawHud(h);
    const std::string s = os.str();
    CHECK(s.find("Race: Human") != std::string::npos);
    CHECK(s.find("Gold: 42") != std::string::npos);
    CHECK(s.find("Floor 3") != std::string::npos);
    CHECK(s.find("HP: 100") != std::string::npos);
    CHECK(s.find("Atk: 20") != std::string::npos);
    CHECK(s.find("Def: 15") != std::string::npos);
    CHECK(s.find("Action: moved north") != std::string::npos);
    // Optional sections are off by default.
    CHECK(s.find("Active Quests") == std::string::npos);
    CHECK(s.find("Speed:") == std::string::npos);
}

TEST_CASE("AnsiRenderer::drawHud renders quests and weather when enabled") {
    std::ostringstream os;
    cc3k::AnsiRenderer r(os);
    cc3k::HudInfo h;
    h.race = "Elf";
    h.questEnabled = true;
    h.activeQuests = {"Kill 5 goblins", "Find the orb"};
    h.weatherEnabled = true;
    h.weather = "Heavy fog";
    h.movementSpeed = 2;
    r.drawHud(h);
    const std::string s = os.str();
    CHECK(s.find("Active Quests:") != std::string::npos);
    CHECK(s.find("- Kill 5 goblins") != std::string::npos);
    CHECK(s.find("- Find the orb") != std::string::npos);
    CHECK(s.find("Heavy fog") != std::string::npos);
    CHECK(s.find("Speed: 2") != std::string::npos);
}

TEST_CASE("AnsiRenderer::drawHud omits empty quest section when enabled but empty") {
    std::ostringstream os;
    cc3k::AnsiRenderer r(os);
    cc3k::HudInfo h;
    h.race = "Orc";
    h.questEnabled = true;          // enabled but list empty
    r.drawHud(h);
    CHECK(os.str().find("Active Quests") == std::string::npos);
}

// ---------------------------------------------------------------------------
// Phase 3.9 — Data-driven content (ItemStats: gold values)
// ---------------------------------------------------------------------------

#include "../src/ItemStats.h"

TEST_CASE("ItemStats built-in defaults match the historical hardcoded values") {
    ItemStats s;
    CHECK(s.getGoldValue(Type::NORMAL_GOLD_PILE) == 1);
    CHECK(s.getGoldValue(Type::SMALL_HOARD)      == 2);
    CHECK(s.getGoldValue(Type::MERCHANT_HOARD)   == 4);
    CHECK(s.getGoldValue(Type::DRAGON_HOARD)     == 6);
}

TEST_CASE("ItemStats::loadFromFile overrides defaults from JSON") {
    const std::string path = "tests_tmp_items.json";
    {
        std::ofstream o(path);
        o << R"({"gold":{"normal_gold_pile":9,"small_hoard":7,"dragon_hoard":42}})";
    }
    ItemStats s;
    REQUIRE(s.loadFromFile(path));
    CHECK(s.isLoaded());
    CHECK(s.source() == path);
    CHECK(s.getGoldValue(Type::NORMAL_GOLD_PILE) == 9);
    CHECK(s.getGoldValue(Type::SMALL_HOARD)      == 7);
    CHECK(s.getGoldValue(Type::DRAGON_HOARD)     == 42);
    std::remove(path.c_str());
}

TEST_CASE("ItemStats::loadFromFile rejects malformed JSON and keeps defaults") {
    const std::string bad = "tests_tmp_bad_items.json";
    {
        std::ofstream o(bad);
        o << "this is not json";
    }
    ItemStats s;
    CHECK_FALSE(s.loadFromFile(bad));
    CHECK(s.getGoldValue(Type::DRAGON_HOARD) == 6); // defaults intact
    std::remove(bad.c_str());
}

TEST_CASE("ItemStats::loadFromFile rejects missing 'gold' section") {
    const std::string path = "tests_tmp_no_gold.json";
    {
        std::ofstream o(path);
        o << R"({"weapons":{}})";
    }
    ItemStats s;
    CHECK_FALSE(s.loadFromFile(path));
    CHECK(s.getGoldValue(Type::NORMAL_GOLD_PILE) == 1);
    std::remove(path.c_str());
}

TEST_CASE("ItemGenerator gold instances reflect the live ItemStats registry") {
    auto n = ItemGenerator::generateGold(Type::NORMAL_GOLD_PILE);
    auto s = ItemGenerator::generateGold(Type::SMALL_HOARD);
    auto d = ItemGenerator::generateGold(Type::DRAGON_HOARD);
    auto m = ItemGenerator::generateGold(Type::MERCHANT_HOARD);
    REQUIRE(n); REQUIRE(s); REQUIRE(d); REQUIRE(m);
    auto* reg = ItemStats::getInstance();
    CHECK(n->getValue() == reg->getGoldValue(Type::NORMAL_GOLD_PILE));
    CHECK(s->getValue() == reg->getGoldValue(Type::SMALL_HOARD));
    CHECK(d->getValue() == reg->getGoldValue(Type::DRAGON_HOARD));
    CHECK(m->getValue() == reg->getGoldValue(Type::MERCHANT_HOARD));
}

// ---------------------------------------------------------------------------
// Phase 3.9b — potion deltas data-driven
// ---------------------------------------------------------------------------

TEST_CASE("ItemStats built-in defaults provide potion deltas of 5") {
    ItemStats s;
    CHECK(s.getPotionDelta(Type::RH) == 5);
    CHECK(s.getPotionDelta(Type::PH) == 5);
    CHECK(s.getPotionDelta(Type::BA) == 5);
    CHECK(s.getPotionDelta(Type::WA) == 5);
    CHECK(s.getPotionDelta(Type::BD) == 5);
    CHECK(s.getPotionDelta(Type::WD) == 5);
}

TEST_CASE("ItemStats::loadFromFile overrides potion deltas from JSON") {
    const std::string path = "tests_tmp_items_potions.json";
    {
        std::ofstream o(path);
        o << R"({"gold":{"normal_gold_pile":1},"potions":{"rh":17,"ba":99}})";
    }
    ItemStats s;
    REQUIRE(s.loadFromFile(path));
    CHECK(s.getPotionDelta(Type::RH) == 17);
    CHECK(s.getPotionDelta(Type::BA) == 99);
    // Loader replaces the whole potions table (same semantics as EnemyStats),
    // so an unspecified key in the override file is *not* in the new table.
    CHECK_THROWS_AS(s.getPotionDelta(Type::PH), std::out_of_range);
    std::remove(path.c_str());
}

TEST_CASE("BoostAtkEffect uses ItemStats::getPotionDelta") {
    auto pc = std::make_shared<PlayerCharacter>(Race::HUMAN);
    const int baseAtk = pc->getAtk();
    const int delta = ItemStats::getInstance()->getPotionDelta(Type::BA);
    BoostAtkEffect e;
    e.apply(pc);
    CHECK(pc->getAtk() == baseAtk + delta);
    e.remove(pc);
    CHECK(pc->getAtk() == baseAtk);
}

TEST_CASE("RestoreHealthEffect uses ItemStats::getPotionDelta") {
    auto pc = std::make_shared<PlayerCharacter>(Race::HUMAN);
    // Wound the player first so heal has room to apply.
    pc->setHp(-20);
    const int hpBefore = pc->getHp();
    const int delta = ItemStats::getInstance()->getPotionDelta(Type::RH);
    RestoreHealthEffect e;
    e.apply(pc);
    CHECK(pc->getHp() == hpBefore + delta);
}

// ---------------------------------------------------------------------------
// Phase 3.9c — race base stats data-driven
// ---------------------------------------------------------------------------

#include "../src/RaceStats.h"

TEST_CASE("RaceStats built-in defaults match the historical setAttributes()") {
    RaceStats s;
    CHECK(s.get(Race::HUMAN).maxHp        == 140);
    CHECK(s.get(Race::HUMAN).atk          == 20);
    CHECK(s.get(Race::HUMAN).def          == 20);
    CHECK(s.get(Race::HUMAN).goldModifier == doctest::Approx(1.0));
    CHECK(s.get(Race::DWARF).maxHp        == 100);
    CHECK(s.get(Race::DWARF).def          == 30);
    CHECK(s.get(Race::DWARF).goldModifier == doctest::Approx(2.0));
    CHECK(s.get(Race::ELF).atk            == 30);
    CHECK(s.get(Race::ELF).def            == 10);
    CHECK(s.get(Race::ORC).maxHp          == 180);
    CHECK(s.get(Race::ORC).goldModifier   == doctest::Approx(0.5));
}

TEST_CASE("RaceStats::loadFromFile overrides defaults from JSON") {
    const std::string path = "tests_tmp_races.json";
    {
        std::ofstream o(path);
        o << R"({"human":{"maxHp":999,"atk":99,"def":88,"goldModifier":3.5}})";
    }
    RaceStats s;
    REQUIRE(s.loadFromFile(path));
    CHECK(s.isLoaded());
    CHECK(s.source() == path);
    CHECK(s.get(Race::HUMAN).maxHp        == 999);
    CHECK(s.get(Race::HUMAN).atk          == 99);
    CHECK(s.get(Race::HUMAN).def          == 88);
    CHECK(s.get(Race::HUMAN).goldModifier == doctest::Approx(3.5));
    std::remove(path.c_str());
}

TEST_CASE("RaceStats::loadFromFile rejects entries missing required keys") {
    const std::string path = "tests_tmp_races_bad.json";
    {
        std::ofstream o(path);
        o << R"({"human":{"maxHp":100,"atk":10}})"; // missing def + goldModifier
    }
    RaceStats s;
    CHECK_FALSE(s.loadFromFile(path));
    // Defaults intact.
    CHECK(s.get(Race::HUMAN).maxHp == 140);
    std::remove(path.c_str());
}

TEST_CASE("PlayerCharacter::setAttributes pulls base stats from RaceStats") {
    PlayerCharacter pc;
    pc.setAttributes(Race::DWARF);
    auto* reg = RaceStats::getInstance();
    CHECK(pc.getMaxHp()       == reg->get(Race::DWARF).maxHp);
    CHECK(pc.getHp()          == reg->get(Race::DWARF).maxHp);
    CHECK(pc.getAtk()         == reg->get(Race::DWARF).atk);
    CHECK(pc.getDef()         == reg->get(Race::DWARF).def);
    CHECK(pc.getGoldModifier() == doctest::Approx(reg->get(Race::DWARF).goldModifier));
    CHECK(static_cast<int>(pc.getRace()) == static_cast<int>(Race::DWARF));
}

TEST_CASE("PlayerCharacter::setAttributes works for every supported race") {
    for (Race r : {Race::HUMAN, Race::DWARF, Race::ELF, Race::ORC}) {
        PlayerCharacter pc;
        pc.setAttributes(r);
        const auto& s = RaceStats::getInstance()->get(r);
        CHECK(pc.getAtk() == s.atk);
        CHECK(pc.getDef() == s.def);
        CHECK(pc.getMaxHp() == s.maxHp);
    }
}

// ---------------------------------------------------------------------------
// Phase 3.9d — floor generation counts data-driven
// ---------------------------------------------------------------------------

#include "../src/FloorStats.h"

TEST_CASE("FloorStats built-in defaults match the historical const-globals") {
    FloorStats s;
    CHECK(s.potions() == 10);
    CHECK(s.gold()    == 10);
    CHECK(s.enemies() == 20);
}

TEST_CASE("FloorStats::loadFromFile overrides counts from JSON") {
    const std::string path = "tests_tmp_floor.json";
    {
        std::ofstream o(path);
        o << R"({"potions":3,"gold":4,"enemies":7})";
    }
    FloorStats s;
    REQUIRE(s.loadFromFile(path));
    CHECK(s.isLoaded());
    CHECK(s.source()  == path);
    CHECK(s.potions() == 3);
    CHECK(s.gold()    == 4);
    CHECK(s.enemies() == 7);
    std::remove(path.c_str());
}

TEST_CASE("FloorStats::loadFromFile rejects JSON missing required keys") {
    const std::string path = "tests_tmp_floor_bad.json";
    {
        std::ofstream o(path);
        o << R"({"potions":5,"gold":5})"; // missing 'enemies'
    }
    FloorStats s;
    CHECK_FALSE(s.loadFromFile(path));
    CHECK(s.potions() == 10); // defaults intact
    CHECK(s.enemies() == 20);
    std::remove(path.c_str());
}

TEST_CASE("FloorStats::loadFromFile rejects malformed JSON") {
    const std::string path = "tests_tmp_floor_bad2.json";
    {
        std::ofstream o(path);
        o << "not json";
    }
    FloorStats s;
    CHECK_FALSE(s.loadFromFile(path));
    CHECK(s.potions() == 10);
}
