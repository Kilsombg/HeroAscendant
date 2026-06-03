#include "EventBus.h"

#include <algorithm>
#include <iostream>

EventBus::EventBus() = default;

// ==============================================================================
// Subscribe
// ==============================================================================

ListenerID EventBus::Subscribe(EventType type, EventListener listener)
{
    ListenerID id = m_nextId++;

    // Get or create the listener list for this event type
    m_listeners[Key(type)].push_back({id, std::move(listener)});

    return id;
}

// ==============================================================================
// Unsubscribe
// ==============================================================================

void EventBus::Unsubscribe(EventType type, ListenerID id)
{
    uint32_t key = Key(type);
    auto it = m_listeners.find(key);
    if (it == m_listeners.end())
        return;

    auto &entries = it->second;

    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
                       [id](const ListenerEntry &e)
                       { return e.id == id; }),
        entries.end());
}

// ==============================================================================
// Fire — immediate delivery
// ==============================================================================

void EventBus::Fire(EventType type, EventData data)
{
    uint32_t key = Key(type);
    auto it = m_listeners.find(key);
    if (it == m_listeners.end())
        return; // No listeners — nothing to do

    // Build the event object
    Event event(type, std::move(data));

    // Copy the listener list before iterating.
    // Why? A listener could call Unsubscribe() during handling, which
    // modifies m_listeners[key]. Iterating a vector while it's being
    // modified is undefined behavior. Copying is the safe approach.
    std::vector<ListenerEntry> listenersCopy = it->second;

    for (const auto &entry : listenersCopy)
    {
        entry.listener(event);
    }
}

// ==============================================================================
// FireDeferred — queued delivery
//
// ==============================================================================

void EventBus::FireDeferred(EventType type, EventData data)
{
    // Just push to the queue. Dispatch() processes it next frame.
    m_deferredQueue.emplace_back(type, std::move(data));
}

// ==============================================================================
// Dispatch — process the deferred queue
//
// ==============================================================================

void EventBus::Dispatch()
{
    // We swap the queue into a local copy before processing.
    // Why? If a deferred event handler calls FireDeferred() again,
    // those new events go into the real m_deferredQueue — not the
    // local copy we're iterating. They'll be processed next Dispatch().
    // Without this swap, new events would be processed in the same pass,
    // potentially causing infinite loops.
    std::vector<Event> toProcess;
    toProcess.swap(m_deferredQueue); // m_deferredQueue is now empty

    for (const auto &event : toProcess)
    {
        uint32_t key = Key(event.type);
        auto it = m_listeners.find(key);
        if (it == m_listeners.end())
            continue;

        std::vector<ListenerEntry> listenersCopy = it->second;
        for (const auto &entry : listenersCopy)
        {
            entry.listener(event);
        }
    }
}

// ==============================================================================
// Clear
// ==============================================================================

void EventBus::Clear()
{
    m_listeners.clear();
    m_deferredQueue.clear();
}

// ==============================================================================
// Debug
// ==============================================================================

size_t EventBus::GetSubscriberCount(EventType type) const
{
    auto it = m_listeners.find(Key(type));
    if (it == m_listeners.end())
        return 0;
    return it->second.size();
}