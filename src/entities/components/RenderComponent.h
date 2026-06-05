#pragma once

#include "IComponent.h"
#include "../../renderer/Renderer.h"

#include <string>

// ==============================================================================
// RenderComponent
//
// Stores rendering data for an entity. The actual draw call is made by
// RenderSystem, which iterates all entities and calls Render()
// on their RenderComponent.
//
//   AnimationTimer counts up each frame. When it exceeds frameInterval,
//   currentFrame advances and timer resets. Simple and reliable.
//
// Facing direction:
//   Hero faces right by default. When moving left, flipH=true mirrors
//   the sprite horizontally — no need for separate left-facing sprites.
//   Enemies face left by default (toward hero).
// ==============================================================================
class RenderComponent : public IComponent
{
public:
    // textureId    = key used with AssetManager::GetTexture()
    // frameW/H     = size of one animation frame in the sprite sheet
    // totalFrames  = how many frames in this animation
    // frameInterval= seconds per frame (e.g. 0.1 = 10 fps animation)
    // facingLeft   = initial facing direction
    RenderComponent(const std::string &textureId,
                    int frameW, int frameH,
                    int totalFrames = 1,
                    float frameInterval = 0.1f,
                    bool facingLeft = false)
        : m_textureId(textureId), m_frameW(frameW), m_frameH(frameH), m_totalFrames(totalFrames), m_frameInterval(frameInterval), m_facingLeft(facingLeft)
    {
    }

    // -----------------------------------------------------------------------
    // Per-frame — advances the animation
    // -----------------------------------------------------------------------
    void Update(float deltaTime) override
    {
        if (m_totalFrames <= 1)
            return; // Static sprite, nothing to animate+

        m_animTimer += deltaTime;
        if (m_animTimer >= m_frameInterval)
        {
            m_animTimer = 0.0f;
            // Advance frame, loop back to 0 at end
            m_currentFrame = (m_currentFrame + 1) % m_totalFrames;
        }
    }

    // -----------------------------------------------------------------------
    // Animation control
    // -----------------------------------------------------------------------

    // SetAnimation — switch to a different animation strip.
    // rowIndex = which row in the sprite sheet (0=idle, 1=run, 2=attack...)
    // Resets to frame 0 when animation changes.
    void SetAnimation(int rowIndex, int totalFrames, float frameInterval)
    {
        if (m_animRow == rowIndex)
            return; // Already playing this animation
        m_animRow = rowIndex;
        m_totalFrames = totalFrames;
        m_frameInterval = frameInterval;
        m_currentFrame = 0;
        m_animTimer = 0.0f;
    }

    // -----------------------------------------------------------------------
    // Getters used by RenderSystem when drawing
    // -----------------------------------------------------------------------

    const std::string &GetTextureId() const { return m_textureId; }
    int GetCurrentFrame() const { return m_currentFrame; }
    int GetFrameW() const { return m_frameW; }
    int GetFrameH() const { return m_frameH; }
    int GetAnimRow() const { return m_animRow; }
    bool IsFacingLeft() const { return m_facingLeft; }
    bool IsVisible() const { return m_visible; }

    // GetSrcX — X position of current frame in the sprite sheet
    int GetSrcX() const { return m_currentFrame * m_frameW; }
    // GetSrcY — Y position (which animation row)
    int GetSrcY() const { return m_animRow * m_frameH; }

    // -----------------------------------------------------------------------
    // Setters
    // -----------------------------------------------------------------------
    void SetFacingLeft(bool left) { m_facingLeft = left; }
    void SetVisible(bool visible) { m_visible = visible; }
    void SetTextureId(const std::string &id) { m_textureId = id; }

private:
    std::string m_textureId;  // Key for AssetManager lookup
    int m_frameW;             // Width of one frame in sprite sheet
    int m_frameH;             // Height of one frame
    int m_totalFrames;        // Total frames in current animation
    float m_frameInterval;    // Seconds per frame
    int m_animRow = 0;        // Which row in the sprite sheet
    int m_currentFrame = 0;   // Current frame index
    float m_animTimer = 0.0f; // Time accumulator for animation
    bool m_facingLeft;        // Flip horizontally when rendering
    bool m_visible = true;    // Can be hidden without destroying
};