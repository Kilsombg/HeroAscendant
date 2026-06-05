#pragma once

// Forward declarations
class Entity;

// ==============================================================================
// IComponent
//
// Every component inherits from this.
//
// Lifecycle:
//   OnAttach() — called when the component is added to an entity.
//                Store the owner pointer here for components that need it.
//
//   Update()   — called every frame by Entity::Update().
//                Not all components need per-frame updates (HealthComponent
//                is purely reactive, TransformComponent just holds data).
//                Default does nothing — only override when needed.
//
//   OnDetach() — called just before the component is removed or entity dies.
//                Clean up any resources here.
// ==============================================================================
class IComponent
{
public:
    virtual ~IComponent() = default;

    virtual void OnAttach(Entity *owner) { m_owner = owner; }
    virtual void Update(float deltaTime) { (void)deltaTime; }
    virtual void OnDetach() {}

protected:
    // Raw pointer back to the owning entity — not owned, just referenced.
    // Safe because Entity always outlives its components.
    Entity *m_owner = nullptr;
};