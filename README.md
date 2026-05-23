# DungeonSpire — modular terminal dungeon crawler

**DungeonSpire** is a terminal-based, single-player dungeon crawler implemented in C++. The game features an endless tower structure with procedurally generated floors, turn-based combat, and modular gameplay extensions (dynamic weather + side-quest engine).

![Demo](./Animation.gif)

---

## Features

- **Endless Tower Progression** — procedural rooms, potions, enemies and loot per floor.
- **Race-Based Playstyles** — pick a race with unique base stats and passive effects.
- **Turn-Based Mechanics** — deterministic per-command state updates.
- **Potion System** — hidden effects until first use; some races see special interactions.
- **Combat Engine** — dynamic enemy AI, factional hostility, damage calculation.
- **Gold & Loot System** — diverse treasure types, faction-specific gold modifiers.
- **Dynamic Weather** (optional) — environmental effects on visibility and movement.
- **Quest System** (optional) — side quests with rewards and objective tracking.

---

## Build & Run

### Prerequisites
- A C++20-capable compiler (`g++` ≥ 10 or `clang++` ≥ 12)
- GNU `make`

### Compilation

Two equivalent options — pick whichever fits your workflow:

```bash
# Option A: classic Makefile
git clone https://github.com/faketut/DungeonSpire.git
cd DungeonSpire/src && make

# Option B: CMake (out-of-tree; required for sanitizers + coverage)
git clone https://github.com/faketut/DungeonSpire.git
cd DungeonSpire
cmake -S . -B build && cmake --build build -j
# Optional toggles: -DENABLE_ASAN=ON -DENABLE_UBSAN=ON -DENABLE_COVERAGE=ON
```

### Running
```bash
./cc3k [--enableWeather] [--enableQuest] [--file <path>] [--seed <n>]
       [--save <path>] [--load <path>]
```

| Flag | Description |
|------|-------------|
| `--enableWeather`  | Enable the dynamic weather system |
| `--enableQuest`    | Enable the side-quest system |
| `--file <path>`    | Load a specific floor layout from file |
| `--seed <n>`       | Seed the PRNG for reproducible runs |
| `--save <path>`    | Write a JSON save file on clean quit (`q`) |
| `--load <path>`    | Resume from a JSON save file at startup; loaded settings (seed, file, weather, quest) override the corresponding CLI flags |

---

## Controls

| Command | Description |
|---------|-------------|
| `no`, `so`, `ea`, `we` | Move N / S / E / W |
| `ne`, `nw`, `se`, `sw` | Move diagonally |
| `u <dir>` | Use potion in given direction |
| `a <dir>` | Attack enemy in given direction |
| `r` | Restart |
| `q` | Quit |

---

## Tests

Unit tests use [doctest](https://github.com/doctest/doctest) (vendored, single header).

```bash
cd tests
make run
```

CI runs seven jobs on every push: `build-and-test` under both `g++` and `clang++`, a CMake build, an ASan+UBSan sanitizer run, `clang-tidy`, `cppcheck`, and a coverage upload — see [.github/workflows/ci.yml](.github/workflows/ci.yml).

---

## Project Layout

```
src/         Game source (Board.cc split into Board{Combat,Enemies,Load,Player}.cc + Game, Player, Enemy, Item, AnsiRenderer, ...)
src/third_party/   Vendored single-header libraries (nlohmann/json)
tests/       doctest-based unit tests
tests/third_party/ Vendored doctest single header
data/        Data-driven configs (enemies.json, items.json, races.json, floor.json)
files/       Hand-authored floor layouts
design/      Original design docs, UML, plans (historical)
```

Key modules:
- [src/Board.h](src/Board.h) + `src/Board*.cc` — map, enemies, combat dispatch
- [src/Game.h](src/Game.h) / [src/Game.cc](src/Game.cc) — main loop, command parsing, save/load orchestration
- [src/Player.h](src/Player.h), [src/Enemy.h](src/Enemy.h), [src/Item.h](src/Item.h) — entity hierarchy
- [src/EffectManager.h](src/EffectManager.h), [src/QuestManager.h](src/QuestManager.h) — Meyers singleton managers
- [src/EventBus.h](src/EventBus.h) — header-only type-erased pub/sub; publishers wired for `FloorChanged`, `EnemyDied`, `ItemPickedUp`
- [src/Renderer.h](src/Renderer.h) / [src/AnsiRenderer.h](src/AnsiRenderer.h) — `IRenderer` abstraction + default ANSI terminal renderer
- [src/EnemyStats.h](src/EnemyStats.h), [src/ItemStats.h](src/ItemStats.h), [src/RaceStats.h](src/RaceStats.h), [src/FloorStats.h](src/FloorStats.h) — stat registries overridable via the matching JSON in [data/](data/)
- [src/SaveGame.h](src/SaveGame.h) / [src/SaveGame.cc](src/SaveGame.cc) — JSON v1 save format
- [src/PRNG.h](src/PRNG.h) — seedable `std::mt19937` wrapper

---

## Development Notes

- Builds under `-Wall -Wextra -Wsuggest-override -Werror=vla` and is clean under ASan + UBSan.
- Code style: see [.clang-format](.clang-format). Run `clang-format -i src/*.{h,cc}` before submitting changes.
- Editor settings: see [.editorconfig](.editorconfig).

---

## License

MIT — see [LICENSE](LICENSE).

## Author

[@faketut](https://github.com/faketut). PRs welcome.
