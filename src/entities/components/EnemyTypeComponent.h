// src/entities/components/EnemyTypeComponent.h
#pragma once

#include "IComponent.h"

enum class EnemyType;

class EnemyTypeComponent : public IComponent
{
public:
    explicit EnemyTypeComponent(EnemyType type) : m_type(type) {}

    EnemyType GetType() const { return m_type; }

private:
    EnemyType m_type;
};