#pragma once

#include <chrono>

namespace core
{
class Timer
{
  public:
    Timer()
    {
        reset();
    }

    void reset()
    {
        m_start = clock::now();
    }

    double elapsed_seconds() const
    {
        return std::chrono::duration<double>(clock::now() - m_start).count();
    }

  private:
    using clock = std::chrono::steady_clock;
    clock::time_point m_start;
};
} // namespace core
