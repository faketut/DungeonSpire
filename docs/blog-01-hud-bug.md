# A one-line constant kept the UI of my roguelike alive for 8 years — until I changed Human's HP

> First in a 3-part series on the [DungeonSpire](https://github.com/faketut/DungeonSpire) modernization.
> **This post:** one specific bug that lived in the source from 2018 to 2026 without anyone noticing — and what it taught me about where bugs actually hide. **Next:** [why I left four nearly identical files un-deduplicated](blog-02-stats-registry-pattern.md).

---

## The crime scene

DungeonSpire started life as `cc3k`, a CS246 final project I submitted in 2018. Eight years later, somewhere in the original `Board.cc`, this function existed verbatim:

```cpp
// Original 2018 code (paraphrased; the real one was tangled with more rendering)
void Board::displayBoard() {
    for (const auto& row : tiles) {
        for (const auto& tile : row) std::cout << toChar(tile->getType());
        std::cout << '\n';
    }
    std::cout << "HP: 20\nAtk: 20\nDef: 20\n";   // <-- this line
}
```

`displayBoard()` was called **once**, by `Game::restart()`, the very first frame after the dungeon was generated. Its job was to print the initial board so the player could see the room they spawned in. That last line — the hardcoded HUD — was supposed to be a stand-in until the real HUD kicked in on the next turn.

The kicker: this function was called **before the player object existed**. The `PlayerCharacter` is constructed inside the race-selection prompt, which happens *after* `restart()` calls `displayBoard()`. So the HUD was hardcoded because, in the literal sense, there was no player to ask.

In 2018 the line happened to be correct. The default Human race had HP=20, Atk=20, Def=20. The marker saw it, the grade was fine, the file was forgotten.

## The drift

At some point between 2018 and now — I can't even pin down which commit — Human's default stats were rebalanced to **HP=140, Atk=20, Def=20**. The race-selection HUD that printed *after* the player existed showed `HP: 140`. The startup HUD inside `displayBoard()` still printed `HP: 20`.

For one rendered frame, every new game told the player they had 20 HP. Then on the next keystroke the real HUD repainted with 140. The eye doesn't catch it: you're looking at the dungeon glyphs, not the panel below them, and by the time you read the panel it already says 140.

I only noticed in 2026 when I was extracting an `IRenderer` interface and trying to figure out *why* `displayBoard` was special-cased differently from the per-turn renderer. The answer was: it wasn't. It was a placeholder that survived because nothing tested it and nobody looked at frame 0.

## The actual root causes

If I had to file this bug, the title would be `Hardcoded display value drifts from default state`. But that's the symptom. There were three structural causes, and each one became a deliberate fix in the modernization:

### Cause 1 — Render coupled to state at the source level

The renderer was a method on `Board`. The state lived on `PlayerCharacter`. To render the HUD you needed the player. To get the player you needed the board to be constructed. To construct the board you went through `displayBoard()`. The dependency graph was a circle; the placeholder was the workaround.

The fix was [src/Renderer.h](../src/Renderer.h):

```cpp
struct HudInfo {
    std::string race;
    int floor, gold, hp, atk, def;
    std::string action;
    bool questEnabled;
    std::vector<std::string> activeQuests;
    bool weatherEnabled;
    std::string weather;
    int movementSpeed;
};

class IRenderer {
public:
    virtual void drawInitialBoard(const Board&) = 0;
    virtual void drawBoard(const Board&) = 0;
    virtual void drawHud(const HudInfo&) = 0;
    virtual ~IRenderer() = default;
};
```

The renderer never reaches into the board for HUD data. `Game::renderInfo()` builds a `HudInfo` once per turn and hands it over. There is now no path by which the renderer can render player state without a player existing — because there's no `player` field on the renderer at all.

### Cause 2 — `restart()` called the renderer before populating the board

```cpp
// Before
void Game::restart() {
    board = std::make_shared<Board>(filename, 0);
    renderer->drawInitialBoard(*board);   // <-- board is empty here
    board->loadBoard(...);
    board->initFloor();
    // ... race selection ...
}
```

This was the kind of ordering bug that a placeholder masks: the placeholder HUD made the empty grid "look fine" because the panel under it had numbers in it. With the placeholder gone, the empty grid was a visible bug, which forced the reorder:

```cpp
// After
void Game::restart() {
    board = std::make_shared<Board>(filename, 0);
    board->loadBoard(...);
    board->initFloor();
    renderer->drawInitialBoard(*board);   // <-- now the dungeon is populated
    // ... race selection ...
}
```

Note that I still **can't** call `renderInfo()` here, because the player races haven't been chosen yet. That's fine — the initial frame just shows the dungeon and lets the player pick a race. The first HUD draw happens at the top of the main loop, by which time the player object is real.

### Cause 3 — Magic numbers everywhere

Even after the placeholder was deleted, the underlying problem remained: the literal `20` in the source was a copy of a state field that lived 4 files away. Change the state, and any literal that mirrors it silently rots. The first one that broke was the visible one. The next ones would have broken silently.

The fix is the second post in this series. For now, suffice to say that every numeric constant that drives gameplay — race stats, potion deltas, gold pile values, floor generation counts — now lives in `data/*.json` and is loaded into a `*Stats` registry at startup. There is no `20` literal you could change without also changing the registry, and no `20` in the source you could miss when changing the registry.

## What I'm taking away

This bug never showed up in a stack trace, never got filed in an issue tracker, and never broke a test — because there were no tests. It was caught visually, eight years late, while doing something unrelated. That's the most uncomfortable kind of bug to find, because it means:

1. **Hardcoded UI values are a form of state cache.** Every `cout << "HP: 20"` is a cache of `player.hp`. Caches need invalidation; placeholders don't.
2. **"Temporary" stand-ins outlive their context.** The 2018-me would have said "it's just for the first frame, I'll fix it later." Eight-years-later-me had no idea that line existed.
3. **You can't see what isn't tested.** A two-line snapshot test of the very first rendered frame would have failed the moment Human's HP changed. There wasn't one, because the renderer was untestable: you couldn't construct a renderer without constructing a board without constructing a player.
4. **Extracting an abstraction is also a forcing function for fixing latent ordering bugs.** The `IRenderer` extraction wasn't *aimed* at this bug. But once the renderer became a pure consumer of a `HudInfo` struct, the placeholder line had nowhere to live, and the ordering of `restart()` had to be defended in code rather than assumed.

The line `HP: 20\nAtk: 20\nDef: 20` is now gone. The dungeon you see on the first frame is the real dungeon, the HUD you see on the first turn is the real HUD, and the only place HP=140 lives is `data/races.json`. If I change it to 200 tomorrow, the only thing that changes is the gameplay — not what the renderer thinks the gameplay is.

---

**Commit that introduced the fix:** [`9c4ac35`](https://github.com/faketut/DungeonSpire/commit/9c4ac35) (HUD placeholder removal + restart reorder), built on top of [`d92d1d3`](https://github.com/faketut/DungeonSpire/commit/d92d1d3) (IRenderer extraction).

**Next in the series:** [Four nearly identical files, and why I didn't extract a base class](blog-02-stats-registry-pattern.md).
