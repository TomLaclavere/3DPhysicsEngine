/**
 * @file timer.hpp
 * @brief High-resolution timer utility for measuring elapsed time in simulations.
 *
 * This module provides a simple, RAII-style timer class for measuring time intervals with microsecond
 * precision. It is used to profile simulation steps, configuration load times, and command execution
 * durations.
 *
 * @see <chrono> (for underlying timing mechanism)
 */
#pragma once
#include "precision.hpp"

#include <chrono>

/**
 * @brief High-resolution timer for measuring elapsed time.
 *
 *
 */
class Timer
{
private:
    /// Stores the time point when Timer was last constructed or reset().
    std::chrono::high_resolution_clock::time_point start_time;

public:
    Timer() { reset(); }

    /// @brief Reset the timer to the current time.
    void reset();

    /// @brief Query elapsed time in microseconds.
    [[nodiscard]] long long elapsedMicroseconds() const;
    /// @brief Query elapsed time in milliseconds.
    [[nodiscard]] decimal elapsedMilliseconds() const;
    /// @brief Query elapsed time in seconds.
    [[nodiscard]] decimal elapsedSeconds() const;
};
