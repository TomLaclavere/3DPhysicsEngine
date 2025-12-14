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

#include <stdexcept>
#include <string>

struct Config
{
private:
    // Physical constants
    decimal gravity = 9.81_d; // m/s^2

    // Simulation parameters
    decimal      timeStep      = 0.01_d; // seconds
    unsigned int maxIterations = 10;     // max iterations for physics solver
    std::string  solver        = "Euler";
    bool         verbose       = true;

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
    decimal        getGravity() const;
    decimal        getTimeStep() const;
    unsigned int   getMaxIterations() const;
    std::string    getSolver() const;
    bool           getVerbose() const;
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
    void setSolver(std::string sol) { solver = sol; }
    void setVerbose(bool verb) { verbose = verb; }
    /// @}

    // ============================================================================
    /// @name Loading Methods
    // ============================================================================
    /// @{
    void loadFromFile(const std::string& path);
    void overrideFromCommandLine(int argc, char** argv);
    /// @}
};
