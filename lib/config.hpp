/**
 * @file precision_constants.hpp
 * @brief Defines numerical precision  and physical constants for the physics/math engine.
 *
 * This file centralizes tolerance values and machine precision
 * used in floating-point comparisons and stability checks.
 * It also includes physical constants relevant to simulations.
 */
#pragma once

#include "precision.hpp"

#include <limits>
#include <stdexcept>
#include <string>

/**
 * @brief Machine epsilon for the current @ref decimal type.
 *
 * Defined as `std::numeric_limits<decimal>::epsilon()`.
 * This represents the smallest increment such that
 * `1.0 + PRECISION_MACHINE != 1.0`.
 *
 * Useful for:
 * - Approximate comparisons of floating-point numbers.
 * - Stability thresholds in numerical algorithms.
 */
const decimal PRECISION_MACHINE = std::numeric_limits<decimal>::epsilon();

struct Config
{
private:
    // Physical constants
    decimal gravity = 9.81_d; // m/s^2

    // Simulation parameters
    decimal      timeStep      = 0.01_d; // seconds
    unsigned int maxIterations = 10;     // max iterations for physics solver

    /// Singleton constructor
    Config() = default;

public:
    // Prevent copying of singleton
    Config(const Config&)            = delete;
    Config& operator=(const Config&) = delete;

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{

    /// Singleton accessor
    static Config& get();
    decimal        getGravity() const { return gravity; }
    decimal        getTimeStep() const { return timeStep; }
    unsigned int   getMaxIterations() const { return maxIterations; }
    /// @}

    // ============================================================================
    /// @name Setters
    // ============================================================================
    /// @{
    void setGravity(decimal g)
    {
        if (g < 0)
            throw std::invalid_argument("Gravity cannot be negative");
        gravity = g;
    }

    void setTimeStep(decimal dt)
    {
        if (dt <= 0)
            throw std::invalid_argument("Time step must be positive");
        timeStep = dt;
    }

    void setMaxIterations(unsigned int max)
    {
        if (max == 0)
            throw std::invalid_argument("Max iterations must be positive");
        maxIterations = max;
    }
    /// @}

    // ============================================================================
    /// @name Loading Methods
    // ============================================================================
    /// @{
    void loadFromFile(const std::string& path);
    void overrideFromCommandLine(int argc, char** argv);
    /// @}
};
