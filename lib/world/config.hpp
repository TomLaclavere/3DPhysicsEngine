/**
 * @file precision_constants.hpp
 * @brief Defines numerical precision and physical constants for the physics/math engine.
 *
 * This file centralizes tolerance values and machine precision
 * used in floating-point comparisons and stability checks.
 * It also includes physical constants relevant to simulations.
 */
#pragma once

#include "precision.hpp"

#include <cmath>
#include <stdexcept>
#include <string>

struct Config
{
private:
    // Physical constants
    decimal gravity = 9.81_d; // m/s^2

    // Simulation parameters
    decimal     timeStep           = 0.01_d; // seconds
    decimal     simulationDuration = 10_d;   // simulation duration in seconds
    std::size_t maxIterations      = static_cast<std::size_t>(std::round(simulationDuration / timeStep));
    std::string solver             = "Euler";
    bool        verbose            = true;

    /// Singleton constructor
    Config() = default;

public:
    // Prevent copying of singleton
    Config(const Config&)            = delete;
    Config& operator=(const Config&) = delete;

    /// @name Getters
    /// @{

    /// Singleton accessor
    static Config& get();
    decimal        getGravity() const;
    decimal        getTimeStep() const;
    decimal        getSimulationDuration() const;
    std::size_t    getMaxIterations() const;
    std::string    getSolver() const;
    bool           getVerbose() const;
    /// @}

    /// @name Setters
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

    /**
     * @brief Set the simulation duration & adjust simulation max iterations number
     *
     * @param time
     */
    void setSimulationDuration(decimal time)
    {
        if (time < 0)
            throw std::invalid_argument("Simulation duration must be positive");
        simulationDuration = time;
        maxIterations      = static_cast<std::size_t>(simulationDuration / timeStep);
    }
    /**
     * @brief Set the max iterations number & adjust the simulation duration
     *
     * @param max
     */
    void setMaxIterations(std::size_t max)
    {
        if (max == 0)
            throw std::invalid_argument("Max iterations must be positive");
        maxIterations      = max;
        simulationDuration = decimal(maxIterations) / timeStep;
    }
    void setSolver(const std::string& sol) { solver = sol; }
    void setVerbose(bool verb) { verbose = verb; }
    /// @}

    /// @name Loading Methods
    /// @{
    void loadFromFile(const std::string& path);
    void overrideFromCommandLine(int argc, char** argv);
    /// @}
};
