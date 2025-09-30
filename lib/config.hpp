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
    // Physical constants
    decimal gravity = 9.81_d; // m/s^2

    // Simulation parameters
    decimal      timeStep      = 0.01_d; // seconds
    unsigned int maxIterations = 10;     // max iterations for physics solver

    // Singleton accessor
    static Config& get();

    // Load configuration from a file
    void loadFromFile(const std::string& path);

    // Override configuration with command line arguments
    void overrideFromCommandLine(int argc, char** argv);
};
