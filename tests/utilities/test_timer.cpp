#include "utilities/timer.hpp"

#include <gtest/gtest.h>
#include <thread>

TEST(TimerTest, MeasuresElapsedTime)
{
    Timer timer;

    // Small sleep to let some time pass
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    const auto us  = timer.elapsedMicroseconds();
    const auto ms  = timer.elapsedMilliseconds();
    const auto sec = timer.elapsedSeconds();

    // --- Sanity checks ---
    EXPECT_GT(us, 0);
    EXPECT_GT(ms, 0);
    EXPECT_GT(sec, 0);

    // --- Consistency checks ---
    EXPECT_NEAR(ms, us / 1000.0, 0.5); // allow rounding tolerance
    EXPECT_NEAR(sec, ms / 1000.0, 0.001);

    // --- Reset test ---
    timer.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    const auto usAfterReset = timer.elapsedMicroseconds();
    EXPECT_GT(usAfterReset, 0);
    EXPECT_LT(usAfterReset, us); // after reset, elapsed should be smaller
}
