/**
 * @file config.hpp
 * @brief Singleton configuration for the physics simulation.
 *
 * Centralises physical constants (gravity), simulation parameters (time step,
 * duration, solver), and default material constants. Can be loaded from a
 * YAML file or overridden from command-line arguments.
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
    decimal     timeStep               = 0.01_d; // seconds
    decimal     simulationDuration     = 10_d;   // simulation duration in seconds
    std::size_t maxIterations          = static_cast<std::size_t>(std::round(simulationDuration / timeStep));
    std::string solver                 = "Euler";
    bool        verbose                = true;
    bool        save                   = false;
    bool        useSimplifiedCollision = true;

    // Default material constants (applied to newly created objects)
    decimal defaultStiffness   = 1000.0_d;
    decimal defaultDamping     = 50.0_d;
    decimal defaultFriction    = 0.4_d;
    decimal defaultRestitution = 0.7_d;

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
    bool           getSave() const;
    bool           getSimplifiedCollision() const;
    decimal        getDefaultStiffness() const;
    decimal        getDefaultDamping() const;
    decimal        getDefaultFriction() const;
    decimal        getDefaultRestitution() const;
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
        timeStep     = dt;
        maxIterations = static_cast<std::size_t>(std::round(simulationDuration / timeStep));
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
    void setSave(bool sav) { save = sav; }
    void setSimplifiedCollision(bool simpColl) { useSimplifiedCollision = simpColl; }
    void setDefaultStiffness(decimal k) { defaultStiffness = k; }
    void setDefaultDamping(decimal d) { defaultDamping = d; }
    void setDefaultFriction(decimal mu) { defaultFriction = mu; }
    void setDefaultRestitution(decimal e) { defaultRestitution = e; }
    /// @}

    /// @name Loading Methods
    /// @{
    void loadFromFile(const std::string& path);
    void overrideFromCommandLine(int argc, char** argv);
    /// @}
};
