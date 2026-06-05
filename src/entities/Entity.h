#pragma once

#include "components/IComponent.h"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <string>

// ==============================================================================
// Entity
//
// An Entity is just a container for components. It has no behaviour of its
// own — everything it can do comes from the components attached to it.
//
// Identification:
//   Each entity gets a unique uint32_t ID from a static counter.
//   This ID is what EventBus events carry (EnemyDiedEvent.enemyId).
//   You can look up "which entity died" by ID from EntityManager.
//
// Component storage:
//   Components are stored in an unordered_map keyed by std::type_index.
//   std::type_index is a wrapper around std::type_info that can be used
//   as a map key. typeid(TransformComponent) gives a unique key per type.
//
//   This means each entity can only have ONE of each component type,
//
// Ownership:
//   Entity owns all its components via unique_ptr.
//   Components have a raw back-pointer to their owner (set in OnAttach).
//
// Usage:
//   // Create a goblin
//   auto goblin = std::make_unique<Entity>("Goblin");
//   goblin->AddComponent<TransformComponent>(540.0f, 800.0f, 64, 64);
//   goblin->AddComponent<HealthComponent>(80, 80);
//   goblin->AddComponent<CombatComponent>(12, 0.8f, 0.05f);
//   goblin->AddComponent<RenderComponent>("goblin_idle");
//   goblin->AddComponent<AIComponent>(AIBehaviour::MoveAndAttack);
//
//   // Later — get a component
//   auto* hp = goblin->GetComponent<HealthComponent>();
//   if (hp) hp->TakeDamage(10);
//
// ==============================================================================
class Entity
{
public:
    // name is for debugging — shows up in logs ("Goblin died")
    explicit Entity(const std::string &name = "Entity");
    ~Entity();

    // Disable copy — entities are unique, owned by EntityManager
    Entity(const Entity &) = delete;
    Entity &operator=(const Entity &) = delete;

    // -----------------------------------------------------------------------
    // Component management
    // -----------------------------------------------------------------------

    // AddComponent — constructs a component in-place and attaches it.
    //
    // Template parameter T    = component type to add
    // Template parameter Args = constructor arguments forwarded to T
    //
    // Returns a raw pointer to the newly added component.
    // The entity owns it — caller must not delete it.
    //
    // Example:
    //   entity.AddComponent<HealthComponent>(100, 100); // maxHP=100, currentHP=100
    template <typename T, typename... Args>
    T *AddComponent(Args &&...args)
    {
        // std::type_index gives a unique key for each component type at runtime
        auto key = std::type_index(typeid(T));

        // Construct the component and store it
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T *raw = component.get();

        // Notify the component it has an owner
        component->OnAttach(this);

        m_components[key] = std::move(component);
        return raw;
    }

    // GetComponent — retrieve a component by type.
    // Returns nullptr if this entity doesn't have that component type.
    //
    // Example:
    //   auto* hp = entity.GetComponent<HealthComponent>();
    //   if (hp && hp->IsDead()) { ... }
    template <typename T>
    T *GetComponent() const
    {
        auto key = std::type_index(typeid(T));
        auto it = m_components.find(key);
        if (it == m_components.end())
            return nullptr;

        // static_cast is safe here — we know the type from the key
        return static_cast<T *>(it->second.get());
    }

    // HasComponent — check without retrieving
    template <typename T>
    bool HasComponent() const
    {
        return m_components.find(std::type_index(typeid(T))) != m_components.end();
    }

    // RemoveComponent — detaches and destroys one component type
    template <typename T>
    void RemoveComponent()
    {
        auto key = std::type_index(typeid(T));
        auto it = m_components.find(key);
        if (it != m_components.end())
        {
            it->second->OnDetach();
            m_components.erase(it);
        }
    }

    // -----------------------------------------------------------------------
    // Per-frame
    // -----------------------------------------------------------------------

    // Update — calls Update() on every component that overrides it.
    // Called by EntityManager every frame.
    void Update(float deltaTime);

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------

    uint32_t GetId() const { return m_id; }
    const std::string &GetName() const { return m_name; }

    bool IsAlive() const { return m_alive; }

    // MarkDead — called by HealthComponent when HP reaches 0.
    // EntityManager removes dead entities at end of frame.
    void MarkDead() { m_alive = false; }

private:
    uint32_t m_id;
    std::string m_name;
    bool m_alive = true;

    // Component storage — type_index key ensures one component per type
    std::unordered_map<std::type_index, std::unique_ptr<IComponent>> m_components;

    // Static counter for generating unique IDs
    // Every Entity ever created gets the next number
    static uint32_t s_nextId;
};