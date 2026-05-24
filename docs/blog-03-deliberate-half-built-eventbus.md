# Why my `EventBus` is deliberately half-built

> Third in a 3-part series on the [DungeonSpire](https://github.com/faketut/DungeonSpire) modernization.
> **Previously:**
> 1. [A one-line constant kept the UI alive for 8 years](blog-01-hud-bug.md)
> 2. [Four nearly identical files, and why I didn't extract a base class](blog-02-stats-registry-pattern.md)
>
> **This post:** the project ships a header-only `EventBus` with three publishers, basically zero in-tree subscribers, and a deliberate decision not to finish it. Here's why a half-built abstraction can be the right amount of abstraction.

---

## What the code does today

[src/EventBus.h](../src/EventBus.h) is ~130 lines, header-only, no dependencies beyond the standard library. The whole thing fits in your head:

```cpp
namespace cc3k {

class EventBus {
public:
    using HandlerId = std::size_t;

    template <typename E>
    HandlerId subscribe(std::function<void(const E&)> handler);
    template <typename E>
    bool unsubscribe(HandlerId id);
    template <typename E>
    void publish(const E& event) const;

    static EventBus* getInstance();
    // ...
};

namespace events {
    struct PlayerMoved   { int fromX, fromY, toX, toY; };
    struct EnemyDied     { int enemyTypeId, atX, atY; };
    struct ItemPickedUp  { int itemTypeId, atX, atY; };
    struct FloorChanged  { int newFloorId; };
    struct EffectApplied { int effectTypeId; };
}}
```

Implementation: a `std::unordered_map<std::type_index, std::vector<Entry>>`. Handlers are erased through a `function<void(const void*)>` thunk. Subscriptions are removable by ID. Dispatch is synchronous, single-threaded, and copies the handler list before iterating so subscribers can `unsubscribe` from inside a callback.

That's the whole bus. It works. It's tested. It's been on `main` since Phase 2.5.

Now the awkward part. Here is the *complete* set of publish sites in the codebase right now:

```
src/Board.cc:97         publish(FloorChanged{i});
src/BoardCombat.cc:75   publish(EnemyDied{...});
src/BoardPlayer.cc:81   publish(ItemPickedUp{...});
```

And the *complete* set of subscribers outside the test suite: **zero**.

To anyone reading the source cold, this looks unfinished. The `PlayerMoved` and `EffectApplied` event types are declared but never published. The bus has a subscribe API that nothing calls. If you opened a PR adding this in a real production codebase, a reviewer would ask "what's this for?" and they wouldn't be wrong to ask.

I shipped it on purpose, and I have no plans to "complete" it.

## The two questions you should ask about any abstraction

When I decide whether to add a layer to a codebase, the two questions are:

1. **What's the marginal cost of having it in the tree?**
2. **What's the marginal cost of adding the next consumer if it's not in the tree?**

For most abstractions in most codebases, both costs are real and need to be weighed. The interesting thing about `EventBus` is that question (1) collapses to nearly zero, which changes the whole calculus.

### Cost of having it in the tree

- **Compile time:** it's a header included in 3 `.cc` files. The template instantiations are trivial — one per event type at the publish site. Measured: indistinguishable from noise.
- **Runtime:** the three live publish calls hit an empty handler list. The cost is one `unordered_map::find` per publish, which returns `end()`, which returns. We're talking nanoseconds per game turn in a roguelike where the bottleneck is `std::cin >> c`.
- **Conceptual:** one file, one class, five event structs. A new contributor can read it in under five minutes and form an accurate model. No inheritance, no virtual dispatch, no threads.
- **Maintenance:** it has tests. It hasn't changed since it was written. There's nothing to maintain.

That's about as close to zero as a real abstraction gets.

### Cost of adding it later, on demand

This is the question people usually undersell.

Imagine I didn't ship the bus. Tomorrow I want to add an achievement system: "kill 100 goblins". Today the kill happens in `Board::die()`, inside `BoardCombat.cc`. Without a bus, the natural implementations are:

- **Option A:** add a direct call from `Board::die()` to `AchievementManager::onEnemyKilled(type)`. This makes `Board` depend on `AchievementManager`. Now `Board.h` includes `AchievementManager.h`, and unit-testing `Board` requires a real or mock `AchievementManager`. The combat code, which has nothing to do with achievements, now has a line about them.
- **Option B:** scan the call sites and add a bus *now*, retrofit a publish, then add the subscriber. This is the same work as just shipping a bus earlier, except now I'm doing it under pressure with a feature in flight.
- **Option C:** add an `Observer*` parameter to `Board::die()` and thread it through the construction chain. This is the "design pattern" answer and it's worse than A in every way.

None of these are awful, but all of them mean *the next person who wants to react to a game event has to also do the bus design*. That's a tax on every future feature that wants to listen.

By contrast, with the bus already in place:

```cpp
// In some future feature file, with no other code changes anywhere:
auto bus = cc3k::EventBus::getInstance();
bus->subscribe<cc3k::events::EnemyDied>([](const auto& e) {
    AchievementManager::onKill(e.enemyTypeId);
});
```

The publishers have already paid the integration cost. New listeners are pure additions.

## The accrual model

I think of in-tree abstractions as having two cost categories:

- **Capital cost** — the work of introducing it. Code + tests + design.
- **Accrual cost** — the ongoing tax on everyone who reads the codebase, every commit that has to navigate around it, every refactor that has to honor it.

A *good* abstraction has a low accrual cost and either repays its capital cost quickly or has a low capital cost to begin with.

The `EventBus` paid its capital in one Phase-2.5 commit and has an accrual cost approaching zero. It's invisible to anyone not looking for it: three lines of `publish(...)` scattered across already-busy methods, plus one file in `src/` next to twenty other files. It's not in the way of anything.

Contrast it with a counterexample from the *same codebase*: `EffectManager`. That one was written in 2018 with a full `Effect` base class and twelve subclasses (`BoostAtkEffect`, `WoundAtkEffect`, `BoostDefEffect`, `WoundDefEffect`, `RestoreHealthEffect`, `PoisonHealthEffect`, plus the weather effects). At the time it had a single owner: the player. Each effect's `apply` and `remove` are 1–4 lines.

That hierarchy *also* looks like an abstraction. But its accrual cost is much higher: every time I want to change how potion magnitudes work (which I did, in Phase 3.9b, to make them data-driven), I have to touch twelve classes. Every reader who wants to know what `BoostAtkEffect` does has to context-switch between header, definition, and the `Effect` interface. The dispatch table at the top of `EffectManager.h` is twelve cases for behavior that could be one function taking a `(stat, sign, magnitude)` triple.

Both look like "abstractions for game events." One is the right amount of abstraction (loose coupling between publishers and unknown future consumers, no behavior commitment). The other is the wrong amount of abstraction (premature class hierarchy where a data table would do).

The difference isn't the size. `EventBus.h` is bigger than the `Effect` class. The difference is the **commitment**.

## What the bus *doesn't* commit to

This is the part I'm proudest of. `EventBus` deliberately doesn't promise:

- **Threading.** Synchronous dispatch only. No locks, no atomics, no `std::shared_mutex`. The minute someone needs threads, they can either upgrade the bus or build something specific. Today the game is single-threaded; pretending otherwise would be dead weight.
- **Ordering between event types.** `EnemyDied` and `ItemPickedUp` can be published in any order; if one needs to happen first, that's a coordination problem at the publish site, not a bus feature.
- **Persistence.** Events are not stored, not replayed, not journaled. If you want a save-game replay, build that on top.
- **Catch-all listeners.** You subscribe to exactly one event type per `subscribe<E>()` call. No "subscribe to everything". This rules out a class of debug-snooping use cases on purpose: if you want a debug listener, subscribe to every type explicitly, which forces you to enumerate what you care about.
- **Wildcard or hierarchical event names.** Events are concrete C++ types. There's no string topic. This means the compiler is your typo-catcher and there's no place for `"plyaer_moved"` to silently never fire.

Each of these would be a reasonable feature in a "real" event bus. None of them are free. The bus stays simple because I refuse to promise any of them until I have a second consumer to make the promise concrete.

## When I'd upgrade it

I have a written-down trigger list. Not "I'd love to do X eventually" — actual concrete conditions:

1. **Three or more independent subscribers across the codebase.** That's when a bus starts paying back its existence. Today the count is zero, so the bus is dormant; I'm fine with that.
2. **Any use case that needs `PlayerMoved`.** I declared the type but didn't publish it because the publish site (movement, every turn, every direction) would burn cycles on a bus that nobody reads. The day a subscriber wants it, I add the three publish lines in `BoardPlayer.cc::movePc()` and turn it on.
3. **A second thread.** If/when the renderer or a future AI worker moves off the main thread, the bus needs locks or per-thread queues or both. Today's bus is documented as single-threaded for a reason: the second I pretend otherwise, I owe a correctness story.
4. **Replay or debug-tooling.** If I want to record a session and play it back, I need a journaling listener and stable event IDs. Today's events are deliberately undocumented as a wire format; once I commit to recording them, I commit to never reorganizing the struct fields again.

None of those are true today. The bus stays the way it is.

## The general principle

Most posts about software design tell you when to *add* an abstraction. This one is about when to **leave one half-finished on purpose**.

The criteria, condensed:

- **Capital cost is small.** A few hours, one file, a few tests.
- **Accrual cost is near-zero.** Invisible to anyone not using it; no inheritance chains, no surprising compile-time costs, no runtime hot path.
- **The cost of *not* having it later is non-trivial.** Retrofitting decoupling under deadline pressure is one of the most common ways "I'll add it when I need it" turns into a worse design than just having it.
- **You can articulate exactly which future commitments you're avoiding.** Threading, persistence, ordering, etc. The list of what the abstraction *won't* do is at least as important as what it does.

If all four are true, ship the half-built thing. Add a comment at the top of the file naming the bound conditions ("Phase 2.5: zero new deps, synchronous dispatch, no threading guarantees yet"), and let it sit. Future-you doesn't need a fully-built bus; future-you needs an obvious spot to plug a listener in, plus the knowledge that the publish sites already exist.

That's what `EventBus.h` is. Not a TODO. Not a half-finished feature. A deliberately bounded abstraction whose dormancy is the whole point.

---

**Files referenced:** [src/EventBus.h](../src/EventBus.h), [src/Board.cc](../src/Board.cc), [src/BoardCombat.cc](../src/BoardCombat.cc), [src/BoardPlayer.cc](../src/BoardPlayer.cc), [src/EffectManager.h](../src/EffectManager.h).

**Series index:**
1. [A one-line constant kept the UI alive for 8 years](blog-01-hud-bug.md)
2. [Four nearly identical files, and why I didn't extract a base class](blog-02-stats-registry-pattern.md)
3. *(this post)* Why my `EventBus` is deliberately half-built
