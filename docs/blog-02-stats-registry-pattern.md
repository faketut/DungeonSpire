# Four nearly identical files, and why I didn't extract a base class

> Second in a 3-part series on the [DungeonSpire](https://github.com/faketut/DungeonSpire) modernization.
> **Previously:** [A one-line constant kept the UI alive for 8 years](blog-01-hud-bug.md).
> **This post:** the four `*Stats` files look like four copies of the same class. I left them that way on purpose. **Next:** [why my `EventBus` is deliberately under-built](blog-03-deliberate-half-built-eventbus.md).

---

## What the repo looks like today

There are four files in [src/](../src/) whose silhouettes are basically identical:

```
src/EnemyStats.h   ~120 lines    backs data/enemies.json
src/ItemStats.h    ~110 lines    backs data/items.json
src/RaceStats.h    ~100 lines    backs data/races.json
src/FloorStats.h    ~80 lines    backs data/floor.json
```

Each one:

1. Is a singleton accessed through `getInstance()`.
2. Holds an in-memory table of plain-old values.
3. Installs built-in defaults in its constructor.
4. Tries to load `data/<thing>.json` from three candidate paths (`data/`, `./data/`, `../data/`) on first use.
5. Exposes a typed getter (`get(Type)`, `getGoldValue(Type)`, `potions()`, etc.).
6. Has a public constructor *only so tests can build an isolated instance*; production code uses the singleton.
7. On `loadFromFile`, replaces the table on success; on failure, leaves the previous table untouched.

When I show this to other C++ programmers the first reaction is universally: *you should extract a base class*. Or a CRTP template. Or at least a macro. Four copies of the same pattern is a smell.

I disagree, and this post is about why.

## What "the same" actually means here

Yes, the four files share a *shape*. But their **payloads have nothing in common**:

| File | Stored value type | Key type | Lookup signature |
|---|---|---|---|
| `EnemyStats` | `struct{int hp, atk, def;}` | `Type` (enum) | `get(Type) -> EnemyStats::Stats` |
| `ItemStats` | `int` (gold) **and** `int` (potion delta) — two unrelated tables | `Type` | `getGoldValue(Type) -> int`, `getPotionDelta(Type) -> int` |
| `RaceStats` | `struct{int maxHp, atk, def; double goldModifier;}` | `Race` (enum) | `get(Race) -> RaceStats::Stats` |
| `FloorStats` | three `int` fields | — (no key) | `potions()`, `gold()`, `enemies()` |

The "duplication" is structural — same skeleton — but the meat is different in every column. A base class would need to be parameterized on:

- The value type (`Stats` struct, single int, three named scalars, …).
- The key type (enum `Type`, enum `Race`, nothing).
- The JSON shape (top-level int, nested object per key, two sections in one file, …).
- The lookup shape (single getter, two getters, three property getters).
- The "what counts as a valid file" predicate (`FloorStats` needs all three keys; `ItemStats` needs a `gold` section but treats `potions` as optional; `EnemyStats` needs a non-empty map).

This is the classic shape of an abstraction trap. If you reach for the obvious tool — a CRTP base or a `template<typename Key, typename Value>` — you end up:

- specializing the JSON-parsing for each subclass (because the JSON shapes don't generalize),
- specializing the getter signatures (because callers don't want `get(Key)` when the natural API is `potions()`),
- specializing the validity rules,
- and inheriting from a class whose only contribution is the three `loadFromFile` candidate paths and the `static T inst;` line.

The base class would save you maybe 15 lines per subclass and cost you a layer of indirection plus template error messages. That is not a trade I take.

## The full smallest one, for reference

[src/FloorStats.h](../src/FloorStats.h) in full (80 lines including license-shaped docblock):

```cpp
class FloorStats {
public:
    static FloorStats* getInstance() {
        static FloorStats inst;
        return &inst;
    }

    bool loadFromFile(const std::string& path) {
        std::ifstream in(path);
        if (!in) return false;
        nlohmann::json j;
        try { in >> j; } catch (...) { return false; }
        try {
            if (!j.contains("potions") || !j.contains("gold") || !j.contains("enemies"))
                return false;
            potions_ = j.at("potions").get<int>();
            gold_    = j.at("gold").get<int>();
            enemies_ = j.at("enemies").get<int>();
        } catch (...) { return false; }
        loaded_ = true;
        source_ = path;
        return true;
    }

    int potions() const { return potions_; }
    int gold()    const { return gold_; }
    int enemies() const { return enemies_; }

    // Public so tests can build isolated instances.
    FloorStats() {
        potions_ = 10; gold_ = 10; enemies_ = 20;
        source_  = "<built-in defaults>";
        for (const char* p : { "data/floor.json", "./data/floor.json", "../data/floor.json" })
            if (loadFromFile(p)) break;
    }

private:
    int potions_ = 10, gold_ = 10, enemies_ = 20;
    bool loaded_ = false;
    std::string source_;
};
```

Read that and ask yourself: *what would a base class actually take out of this?* Six lines of `loadFromFile` candidate-path iteration? The `static inst;` accessor? Both of those are cheaper inline than dressed up.

## The convention that does the deduplication

What deduplicates the four files is **not** code reuse. It's a written-down convention every new `*Stats` follows:

1. **Singleton with public ctor.** Production uses `getInstance()`. Tests construct local instances and call `loadFromFile()` directly, so they don't fight the singleton.
2. **Defaults in the ctor body.** Always present, always complete, never `std::nullopt`. There is no path through the code where a `*Stats` lookup can fail because the file wasn't shipped.
3. **Three candidate paths.** `data/X.json`, `./data/X.json`, `../data/X.json`. Matches how CMake stages the `data/` folder next to the binary (build dir) and how the executable can also be run from `src/` or the repo root during development.
4. **Replace-on-success, untouched-on-failure.** `loadFromFile` is atomic from the caller's perspective: either the entire in-memory table is overwritten, or nothing changes. Half-loaded state would be a footgun for both tests and production.
5. **Lowercase enum-name keys in the JSON.** `"goblin"`, `"normal_gold_pile"`, `"dwarf"`. The C++ side maps these once; the JSON side stays human-readable.
6. **Missing key → `std::out_of_range`.** Lookups are strict. There is no fallback "0" for unknown enum values; if you ask for a thing the registry doesn't know about, you get a thrown exception loud enough to surface in tests.

That's the actual interface. It lives in convention, not inheritance. The price is that adding a fifth `*Stats` requires reading two of the existing ones to see the shape — which is roughly the same effort as reading a base class's documentation, and you also get to skim a concrete example.

## What I get for the duplication

- **Each file fits on a screen.** No "go to the parent class to see what `protected: virtual bool validate()` does". The whole story is one open editor.
- **Compile errors point at the right file.** A typo in `RaceStats` doesn't drag in the template instantiation context of `EnemyStats`.
- **Adding a new shape doesn't require generalizing.** When `RaceStats` needed a `double goldModifier` (no other `*Stats` has a double), I just added it. No `if constexpr`, no specialization, no opt-in.
- **Tests are mechanical.** Each `*Stats` has the same five test cases (defaults, override, malformed JSON, missing keys, end-to-end via a generator). They were copy-pasted across files because the *test shape* is also a convention, and that's fine.

## What I'd do if there were ten of them

The "duplication is fine" position has a limit. At four files I'm happy. At ten I'd reconsider — not by extracting a base class, but by stamping them out from a code generator. Define a small DSL (JSON or Python) that says:

```
name: FloorStats
file: data/floor.json
fields:
  potions: int = 10
  gold:    int = 10
  enemies: int = 20
required: [potions, gold, enemies]
```

…and have it emit the C++. The output would be the same shape it is today: standalone, self-contained, single-file, no inheritance. Just generated rather than hand-typed. That's a future problem; today's duplication is paying its rent.

## The lesson

**Shape sharing is not the same as abstraction.** Four classes with the same silhouette but different cargo are not asking to be unified — they're asking you to *write down the silhouette as a rule* and let the implementations stay specific. Inheritance and templates are answers to questions about *behavior reuse*, not questions about *layout similarity*.

The reason this matters: the wrong abstraction is much more expensive than the right duplication. The wrong abstraction is invisible — it just sits there making future changes harder by 5% forever. The right duplication is visible — you can see the four files in the directory listing and you can grep for the pattern.

I prefer the visible cost.

---

**Files referenced:** [src/EnemyStats.h](../src/EnemyStats.h), [src/ItemStats.h](../src/ItemStats.h), [src/RaceStats.h](../src/RaceStats.h), [src/FloorStats.h](../src/FloorStats.h).
**JSON data:** [data/enemies.json](../data/enemies.json), [data/items.json](../data/items.json), [data/races.json](../data/races.json), [data/floor.json](../data/floor.json).

**Next:** [Why my `EventBus` is deliberately half-built](blog-03-deliberate-half-built-eventbus.md).
