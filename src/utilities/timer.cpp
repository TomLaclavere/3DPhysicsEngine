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

    void reset() { start_time = std::chrono::high_resolution_clock::now(); }

    [[nodiscard]] long long elapsedMicroseconds() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now() - start_time)
            .count();
    }

    [[nodiscard]] double elapsedMilliseconds() const { return elapsedMicroseconds() / 1000.0; }

    [[nodiscard]] double elapsedSeconds() const { return elapsedMicroseconds() / 1e6; }
};
