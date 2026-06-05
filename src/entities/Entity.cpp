#include "Entity.h"

// Static counter — starts at 1. 0 is reserved as "invalid/no entity".
uint32_t Entity::s_nextId = 1;

Entity::Entity(const std::string &name)
    : m_id(s_nextId++), m_name(name)
{
}

Entity::~Entity()
{
    // Notify all components they are being detached before destruction.
    // This gives components a chance to clean up (e.g. unsubscribe events)
    // while the entity is still valid.
    for (auto &[key, component] : m_components)
    {
        component->OnDetach();
    }
}

void Entity::Update(float deltaTime)
{
    // Only update if alive — dead entities are removed this frame
    if (!m_alive)
        return;

    for (auto &[key, component] : m_components)
    {
        component->Update(deltaTime);
    }
}