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
```bash
git clone https://github.com/faketut/DungeonSpire.git
cd DungeonSpire/src
make
```

### Running
```bash
./cc3k [-enableWeather] [-enableQuest] [-file <floorfilename>] [-seed <n>]
```

| Flag | Description |
|------|-------------|
| `-enableWeather`        | Enable the dynamic weather system |
| `-enableQuest`          | Enable the side-quest system |
| `-file <filename>`      | Load a specific floor layout from file |
| `-seed <n>`             | Seed the PRNG for reproducible runs |

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

CI builds the game with both `g++` and `clang++`, runs the test suite, then smoke-runs the game under Valgrind — see [.github/workflows/ci.yml](.github/workflows/ci.yml).

---

## Project Layout

```
src/        Game source (header-only classes + 3 .cc translation units)
tests/      doctest-based unit tests
design/     Original design docs, UML, plans (historical)
```

Key modules:
- [src/Board.h](src/Board.h) / [src/Board.cc](src/Board.cc) — map, enemies, combat dispatch
- [src/Game.h](src/Game.h) / [src/Game.cc](src/Game.cc) — main loop, command parsing
- [src/Player.h](src/Player.h), [src/Enemy.h](src/Enemy.h), [src/Item.h](src/Item.h) — entity hierarchy
- [src/EffectManager.h](src/EffectManager.h), [src/QuestManager.h](src/QuestManager.h) — Meyers singleton managers
- [src/PRNG.h](src/PRNG.h) — seedable `std::mt19937` wrapper

---

## Development Notes

- Memory-safety verified via Valgrind (run in CI).
- Code style: see [.clang-format](.clang-format). Run `clang-format -i src/*.{h,cc}` before submitting changes.
- Editor settings: see [.editorconfig](.editorconfig).

---

## License

MIT — see [LICENSE](LICENSE).

## Author

[@faketut](https://github.com/faketut). PRs welcome.
