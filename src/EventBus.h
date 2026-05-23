#ifndef EVENT_BUS_H
#define EVENT_BUS_H

// Header-only, type-erased publish/subscribe bus.
//
// Goals (Phase 2.5 of the roadmap):
//   * Zero new dependencies, header-only, drop-in.
//   * Strongly-typed events: each event is a plain struct in cc3k::events.
//   * Subscribers receive const-ref to the event; publish() is fire-and-forget.
//   * Singleton accessor mirrors the existing EffectManager pattern so the rest
//     of the codebase can adopt it incrementally without dependency injection.
//   * Synchronous dispatch. No threading guarantees yet.
//
// Usage:
//   auto* bus = cc3k::EventBus::getInstance();
//   auto id = bus->subscribe<cc3k::events::FloorChanged>(
//       [](const cc3k::events::FloorChanged& e){ /* ... */ });
//   bus->publish(cc3k::events::FloorChanged{2});
//   bus->unsubscribe<cc3k::events::FloorChanged>(id);

#include <algorithm>
#include <cstddef>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cc3k {

class EventBus {
public:
    using HandlerId = std::size_t;

    template <typename E>
    HandlerId subscribe(std::function<void(const E&)> handler) {
        auto& list = handlers_[std::type_index(typeid(E))];
        HandlerId id = nextId_++;
        list.push_back(Entry{
            id,
            [h = std::move(handler)](const void* e) {
                h(*static_cast<const E*>(e));
            }
        });
        return id;
    }

    template <typename E>
    bool unsubscribe(HandlerId id) {
        auto it = handlers_.find(std::type_index(typeid(E)));
        if (it == handlers_.end()) return false;
        auto& list = it->second;
        const auto before = list.size();
        list.erase(
            std::remove_if(list.begin(), list.end(),
                           [id](const Entry& e) { return e.id == id; }),
            list.end());
        return list.size() != before;
    }

    template <typename E>
    void publish(const E& event) const {
        auto it = handlers_.find(std::type_index(typeid(E)));
        if (it == handlers_.end()) return;
        // Copy the list so subscribers may safely (un)subscribe during dispatch.
        const auto snapshot = it->second;
        for (const auto& entry : snapshot) {
            entry.fn(&event);
        }
    }

    void clear() { handlers_.clear(); }

    std::size_t handlerCount() const {
        std::size_t n = 0;
        for (const auto& kv : handlers_) n += kv.second.size();
        return n;
    }

    template <typename E>
    std::size_t handlerCount() const {
        auto it = handlers_.find(std::type_index(typeid(E)));
        return it == handlers_.end() ? 0u : it->second.size();
    }

    static EventBus* getInstance() {
        static EventBus inst;
        return &inst;
    }

private:
    struct Entry {
        HandlerId id;
        std::function<void(const void*)> fn;
    };
    std::unordered_map<std::type_index, std::vector<Entry>> handlers_;
    HandlerId nextId_ = 1;
};

// Standard event payloads. New events can be added freely; they only need to
// be visible at both publish and subscribe sites.
namespace events {

struct PlayerMoved {
    int fromX;
    int fromY;
    int toX;
    int toY;
};

struct EnemyDied {
    int enemyTypeId;
    int atX;
    int atY;
};

struct ItemPickedUp {
    int itemTypeId;
    int atX;
    int atY;
};

struct FloorChanged {
    int newFloorId;
};

struct EffectApplied {
    int effectTypeId;
};

struct EffectExpired {
    int effectTypeId;
};

} // namespace events
} // namespace cc3k

#endif // EVENT_BUS_H
