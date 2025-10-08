#include "precision.hpp"

#include <chrono>

// ============================================================================
// Timer utility for consistent duration measurement
// ============================================================================
class Timer
{
private:
    std::chrono::high_resolution_clock::time_point start_time;

public:
    Timer() { reset(); }

    void reset();

    [[nodiscard]] long long elapsedMicroseconds() const;

    [[nodiscard]] decimal elapsedMilliseconds() const;

    [[nodiscard]] decimal elapsedSeconds() const;
};
