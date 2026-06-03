#pragma once

#include "GameEvents.h"

#include <functional>
#include <unordered_map>
#include <vector>
#include <cstdint>

// ==============================================================================
// EventBus
//
// A central hub that connects event publishers to event listeners.
// Publishers fire events without knowing who listens.
// Listeners subscribe without knowing who publishes.
//
// How it works:
//   Subscribe: "when EventType::EnemyDied fires, call this function"
//   Fire:      "EventType::EnemyDied just happened, here's the data"
//   EventBus:  calls every function subscribed to EnemyDied
//
// Listener ID:
//   Subscribe() returns a ListenerID (uint32_t).
//   Keep it — pass it to Unsubscribe() when your object is destroyed.
//   Forgetting to unsubscribe = crash when EventBus calls a deleted object.
//
// Thread safety:
//   NOT thread-safe. All events must be fired from the main thread.
//   For a single-threaded game loop this is fine.
//
// ==============================================================================

// A listener is any callable that takes a const Event&
// std::function wraps lambdas, member functions, free functions equally
using EventListener = std::function<void(const Event &)>;

// ListenerID is returned by Subscribe and used by Unsubscribe
using ListenerID = uint32_t;

class EventBus
{
public:
    EventBus();
    ~EventBus() = default;

    // Disable copy — EventBus is a unique hub, copying makes no sense
    EventBus(const EventBus &) = delete;
    EventBus &operator=(const EventBus &) = delete;

    // -----------------------------------------------------------------
    // Subscribe — register a listener for an event type.
    //
    // Returns a ListenerID. Store it and pass to Unsubscribe() when
    // the listening object is destroyed.
    //
    // Usage — lambda:
    //   auto id = eventBus.Subscribe(EventType::EnemyDied,
    //       [this](const Event& e) {
    //           auto& data = std::get<EnemyDiedEvent>(e.data);
    //           OnEnemyDied(data);
    //       });
    //
    // Usage — member function:
    //   auto id = eventBus.Subscribe(EventType::EnemyDied,
    //       [this](const Event& e) { HandleEnemyDied(e); });
    //
    // -----------------------------------------------------------------
    ListenerID Subscribe(EventType type, EventListener listener);

    // -----------------------------------------------------------------
    // Unsubscribe — remove a listener by its ID.
    // MUST be called when the subscribing object is destroyed.
    //
    // Usage — in destructor:
    //   eventBus.Unsubscribe(EventType::EnemyDied, m_enemyDiedListenerId);
    //
    // -----------------------------------------------------------------
    void Unsubscribe(EventType type, ListenerID id);

    // -----------------------------------------------------------------
    // Fire — immediately deliver an event to all subscribers.
    //
    // Usage:
    //   m_eventBus.Fire(EventType::EnemyDied, EnemyDiedEvent{
    //       .enemyId      = enemy.GetId(),
    //       .coinsDropped = enemy.GetCoinDrop(),
    //       .floorIndex   = m_currentFloor,
    //       .posX         = enemy.GetX(),
    //       .posY         = enemy.GetY()
    //   });
    //
    // The event is delivered synchronously — all listeners are called
    // before Fire() returns. This keeps event handling predictable.
    //
    // -----------------------------------------------------------------
    void Fire(EventType type, EventData data);

    // -----------------------------------------------------------------
    // FireDeferred — queues an event to be delivered on next Dispatch().
    //
    // Use this when firing from inside an event handler to avoid
    // re-entrant delivery (a listener triggering the same event again).
    //
    // Example: HeroTookDamage handler checks if hero died and fires
    // HeroDied. Using FireDeferred prevents HeroDied from being
    // processed before HeroTookDamage finishes.
    //
    // -----------------------------------------------------------------
    void FireDeferred(EventType type, EventData data);

    // -----------------------------------------------------------------
    // Dispatch — delivers all deferred events.
    // Call once per frame in Game::Update() BEFORE other system updates.
    //
    // -----------------------------------------------------------------
    void Dispatch();

    // -----------------------------------------------------------------
    // Clear — removes ALL listeners for all event types.
    // Call when transitioning between major states (e.g. returning to
    // main menu) to avoid stale listeners from the previous scene.
    //
    // -----------------------------------------------------------------
    void Clear();

    // GetSubscriberCount — useful for debugging
    size_t GetSubscriberCount(EventType type) const;

private:
    // For each EventType, a list of (id, listener) pairs
    struct ListenerEntry
    {
        ListenerID id;
        EventListener listener;
    };

    // Indexed by EventType cast to uint32_t
    // unordered_map is fine here — event type lookup happens per-fire, not per-frame pixel
    std::unordered_map<uint32_t, std::vector<ListenerEntry>> m_listeners;

    // Deferred event queue — filled by FireDeferred, emptied by Dispatch
    std::vector<Event> m_deferredQueue;

    // Counter for generating unique listener IDs
    ListenerID m_nextId = 1;

    // Converts EventType to map key
    uint32_t Key(EventType type) const
    {
        return static_cast<uint32_t>(type);
    }
};