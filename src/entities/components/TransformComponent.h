#pragma once

#include "IComponent.h"

// ==============================================================================
// TransformComponent
//
// Holds an entity's position and size in virtual screen coordinates.
// Every visible entity needs this — it's the most fundamental component.
//
// Coordinate system:
//   Origin (0,0) is top-left of the virtual 1080x1920 screen.
//
// x, y = top-left corner of the entity's bounding rectangle.
// w, h = width and height in virtual pixels.
//
// ==============================================================================
class TransformComponent : public IComponent
{
public:
    TransformComponent(float x, float y, int w, int h)
        : m_x(x), m_y(y), m_w(w), m_h(h)
    {
    }

    // Position getters / setters
    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    void SetX(float x) { m_x = x; }
    void SetY(float y) { m_y = y; }
    void SetPosition(float x, float y)
    {
        m_x = x;
        m_y = y;
    }

    // Move by delta — used by AIComponent for movement
    void Translate(float dx, float dy)
    {
        m_x += dx;
        m_y += dy;
    }

    // Size getters — size rarely changes after creation
    int GetW() const { return m_w; }
    int GetH() const { return m_h; }
    void SetSize(int w, int h)
    {
        m_w = w;
        m_h = h;
    }

    // Center helpers — useful for positioning effects, damage numbers
    float GetCenterX() const { return m_x + m_w * 0.5f; }
    float GetCenterY() const { return m_y + m_h * 0.5f; }

    // Overlap check — used by simple collision detection
    // Returns true if this entity's rect overlaps another transform
    bool Overlaps(const TransformComponent &other) const
    {
        return m_x < other.m_x + other.m_w &&
               m_x + m_w > other.m_x &&
               m_y < other.m_y + other.m_h &&
               m_y + m_h > other.m_y;
    }

private:
    float m_x, m_y; // Position — float for smooth sub-pixel movement
    int m_w, m_h;   // Size — int because sprites are whole pixels
};