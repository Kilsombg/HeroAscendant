#include "GameStateManager.h"

#include <iostream>

GameStateManager::GameStateManager() = default;
GameStateManager::~GameStateManager() = default;

// ==============================================================================
// Stack operations — queue a PendingOp, applied next frame
// ==============================================================================

void GameStateManager::Push(std::unique_ptr<IGameState> state)
{
    m_pendingOps.emplace_back(OpType::Push, std::move(state));
}

void GameStateManager::Pop()
{
    // Pop carries no state — pass nullptr
    m_pendingOps.emplace_back(OpType::Pop, nullptr);
}

void GameStateManager::Replace(std::unique_ptr<IGameState> state)
{
    m_pendingOps.emplace_back(OpType::Replace, std::move(state));
}

void GameStateManager::PopAll(std::unique_ptr<IGameState> state)
{
    m_pendingOps.emplace_back(OpType::PopAll, std::move(state));
}

// ==============================================================================
// ApplyPendingChanges — called by Game at start of each frame
// ==============================================================================

void GameStateManager::ApplyPendingChanges()
{
    // Move the queue out before iterating — same safe-swap pattern as EventBus.
    // If an operation itself queues another op (e.g. OnEnter calls Push),
    // it goes into the real m_pendingOps and is processed next frame.
    std::vector<PendingOp> ops;
    ops.swap(m_pendingOps);

    for (auto &op : ops)
    {
        switch (op.type)
        {
        case OpType::Push:
            DoPush(std::move(op.state));
            break;
        case OpType::Pop:
            DoPop();
            break;
        case OpType::Replace:
            DoReplace(std::move(op.state));
            break;
        case OpType::PopAll:
            DoPopAll(std::move(op.state));
            break;
        }
    }
}

// ==============================================================================
// Per-frame
// ==============================================================================

void GameStateManager::HandleInput(const SDL_Event &event)
{
    if (m_states.empty())
        return;

    // Only the top state handles input.
    m_states.back()->HandleInput(event);
}

void GameStateManager::Update(float deltaTime)
{
    if (m_states.empty())
        return;

    // Only the top state updates.
    m_states.back()->Update(deltaTime);
}

void GameStateManager::Render()
{
    if (m_states.empty())
        return;

    // Find the lowest state that should be rendered.
    // Walk from top downward until we hit a non-transparent state.

    int stateSize = static_cast<int>(m_states.size());
    int firstToRender = stateSize - 1;

    while (firstToRender > 0 && m_states[firstToRender]->IsTransparent())
    {
        firstToRender--;
    }

    // Render from firstToRender up to top (back of vector)
    for (int i = firstToRender; i < stateSize; ++i)
    {
        m_states[i]->Render();
    }
}

// ==============================================================================
// Utility
// ==============================================================================

IGameState *GameStateManager::GetCurrentState() const
{
    if (m_states.empty())
        return nullptr;
    return m_states.back().get();
}

// ==============================================================================
// Private — immediate operations, only called from ApplyPendingChanges
// ==============================================================================

void GameStateManager::DoPush(std::unique_ptr<IGameState> state)
{
    // Pause the current top state before pushing
    if (!m_states.empty())
        m_states.back()->OnPause();

    state->OnEnter();
    m_states.push_back(std::move(state));
}

void GameStateManager::DoPop()
{
    if (m_states.empty())
    {
        std::cerr << "[GameStateManager] Pop called on empty stack.\n";
        return;
    }

    // Exit and destroy the top state
    m_states.back()->OnExit();
    m_states.pop_back();

    // Resume the state that's now on top
    if (!m_states.empty())
        m_states.back()->OnResume();
}

void GameStateManager::DoReplace(std::unique_ptr<IGameState> state)
{
    // Exit and remove the current top state
    if (!m_states.empty())
    {
        m_states.back()->OnExit();
        m_states.pop_back();
    }

    // Enter the new state
    state->OnEnter();
    m_states.push_back(std::move(state));
}

void GameStateManager::DoPopAll(std::unique_ptr<IGameState> state)
{
    // Exit all states from top to bottom
    while (!m_states.empty())
    {
        m_states.back()->OnExit();
        m_states.pop_back();
    }

    // Push the new root state
    state->OnEnter();
    m_states.push_back(std::move(state));
}