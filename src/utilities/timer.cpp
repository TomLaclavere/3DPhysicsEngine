#include "utilities/timer.hpp"

void Timer::reset() { start_time = std::chrono::high_resolution_clock::now(); }

[[nodiscard]] long long Timer::elapsedMicroseconds() const
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() -
                                                                 start_time)
        .count();
}

[[nodiscard]] decimal Timer::elapsedMilliseconds() const { return elapsedMicroseconds() / 1000_d; }

[[nodiscard]] decimal Timer::elapsedSeconds() const { return elapsedMicroseconds() / 1e6_d; }
