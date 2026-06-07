// src/core/RNG.h
#pragma once

#include <random>
#include <cstdint>

// ==============================================================================
// RNG
//
// Cross-platform note:
//   std::mt19937 is part of C++11 and produces identical output on every
//   platform. rand() quality varies across Android NDK versions and MSVC.
//
// ==============================================================================
class RNG
{
public:
    // Get the single instance
    static RNG &Get()
    {
        static RNG instance;
        return instance;
    }

    // Seed — call once at startup in Game::Run()
    void Seed(uint32_t seed)
    {
        m_engine.seed(seed);
    }

    // Float in [0.0, 1.0)
    float NextFloat()
    {
        return m_dist(m_engine);
    }

    // Integer in [min, max] inclusive
    int NextInt(int min, int max)
    {
        std::uniform_int_distribution<int> d(min, max);
        return d(m_engine);
    }

private:
    RNG() : m_dist(0.0f, 1.0f) {}
    RNG(const RNG &) = delete;
    RNG &operator=(const RNG &) = delete;

    std::mt19937 m_engine;
    std::uniform_real_distribution<float> m_dist;
};