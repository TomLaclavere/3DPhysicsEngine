/**
 * @file scenario.hpp
 *
 * @brief What the performance benchmarks simulate and time, shared by both pipelines:
 *        the in-house benchmark (benchmark.cpp) and Google Benchmark (gbench.cpp).
 *
 * Both executables take the same scenario options, build the same scene and time the same function
 * (runSteps), so a change made here applies to both and their results stay comparable.
 */

#pragma once

#include "mathematics/vector.hpp"
#include "objects/sphere.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <numbers>
#include <random>
#include <string>
#include <vector>

/// One benchmark point: what is simulated, and for how long.
struct Scenario
{
    std::string solver        = "Verlet";
    std::string broad         = "Brut";
    decimal     dt            = 1e-3_d;
    decimal     duration      = 22_d;
    int         nObjects      = 1;
    bool        contactForces = false;

    /// Number of steps of one run.
    [[nodiscard]] int steps() const { return static_cast<int>(duration / dt); }
};

/// Read the scenario option at argv[i] (--solver, --broad, --dt, --dur, --n_objects, --contact-forces).
/// Returns false if argv[i] is not one of them; otherwise advances i past its value.
inline bool parseScenarioOption(Scenario& sc, int& i, int argc, char** argv)
{
    const bool hasValue = i + 1 < argc;
    if (std::strcmp(argv[i], "--solver") == 0 && hasValue)
        sc.solver = argv[++i];
    else if (std::strcmp(argv[i], "--broad") == 0 && hasValue)
        sc.broad = argv[++i];
    else if (std::strcmp(argv[i], "--dt") == 0 && hasValue)
        sc.dt = static_cast<decimal>(std::atof(argv[++i]));
    else if (std::strcmp(argv[i], "--dur") == 0 && hasValue)
        sc.duration = static_cast<decimal>(std::atof(argv[++i]));
    else if (std::strcmp(argv[i], "--n_objects") == 0 && hasValue)
        sc.nObjects = std::atoi(argv[++i]);
    else if (std::strcmp(argv[i], "--contact-forces") == 0)
        sc.contactForces = true;
    else
        return false;
    return true;
}

/// The engine silently falls back to brute force on an unknown name, which would mislabel the results.
inline bool isKnownBroad(const std::string& broad)
{
    return broad == "Brut" || broad == "Grid" || broad == "SweepPrune";
}

/// Set the engine configuration for this scenario.
inline void configure(const Scenario& sc)
{
    Config& config = Config::get();
    config.setSolver(sc.solver);
    config.setBroadCollision(sc.broad);
    config.setTimeStep(sc.dt);
    config.setMaxIterations(sc.steps());
    config.setSimplifiedCollision(!sc.contactForces);
}

/// Initial state of the spheres of the scene.
struct Cloud
{
    std::vector<decimal>  diameters;
    std::vector<Vector3D> positions;
    std::vector<Vector3D> velocities;
};

/// Random 3D cloud of n spheres, with a fixed seed so every run, broad phase and tool sees the same
/// scene. Diameters span a factor 8: the grid sizes its cells on the largest object, so this exposes
/// its known weakness. The box grows with n to keep the volume fraction, hence the collision rate,
/// constant. Random velocities make spheres collide.
inline Cloud makeCloud(int n)
{
    const decimal minDiameter    = 1_d;
    const decimal maxDiameter    = 8_d;
    const decimal volumeFraction = 0.1_d;
    const decimal maxSpeed       = 5_d;

    std::mt19937                            rng(42);
    std::uniform_real_distribution<decimal> diameterDist(minDiameter, maxDiameter);
    std::uniform_real_distribution<decimal> speedDist(-maxSpeed, maxSpeed);

    Cloud cloud;
    cloud.diameters.resize(static_cast<size_t>(n));
    decimal spheresVolume = 0_d;
    for (decimal& d : cloud.diameters)
    {
        d = diameterDist(rng);
        spheresVolume += std::numbers::pi_v<decimal> * d * d * d / 6_d;
    }
    const decimal                           boxSide = std::cbrt(spheresVolume / volumeFraction);
    std::uniform_real_distribution<decimal> positionDist(0_d, boxSide);

    for (const decimal d : cloud.diameters)
    {
        // Rejection sampling: no overlap at t=0
        Vector3D p;
        bool     overlaps = true;
        while (overlaps)
        {
            p        = Vector3D(positionDist(rng), positionDist(rng), positionDist(rng));
            overlaps = false;
            for (size_t k = 0; k < cloud.positions.size() && !overlaps; ++k)
                overlaps = (p - cloud.positions[k]).getNorm() < 0.5_d * (d + cloud.diameters[k]);
        }
        cloud.positions.push_back(p);
        cloud.velocities.emplace_back(speedDist(rng), speedDist(rng), speedDist(rng));
    }
    return cloud;
}

/// Fill `world` with the cloud and start it. Spheres only, with no ground: this scene measures the
/// broad phase, and a large plane would dictate the grid cell size.
inline void buildScene(PhysicsWorld& world, const Cloud& cloud)
{
    for (size_t obj = 0; obj < cloud.diameters.size(); ++obj)
    {
        auto sph = std::make_unique<Sphere>(cloud.positions[obj], cloud.diameters[obj], cloud.velocities[obj], 1_d);
        sph->setIsFixed(false);
        sph->setRestitutionCst(1_d);
        world.addObject(sph.release()); // the world owns and deletes its objects
    }
    world.start();
}

/// The world of one run: built from the cloud on construction, emptied on destruction (not timed).
struct ScenarioWorld
{
    PhysicsWorld world;

    explicit ScenarioWorld(const Cloud& cloud) : world(Config::get()) { buildScene(world, cloud); }
    ~ScenarioWorld() { world.clearObjects(); }
    ScenarioWorld(const ScenarioWorld&)            = delete;
    ScenarioWorld& operator=(const ScenarioWorld&) = delete;
};

/// The timed operation, in both tools: `steps` simulation steps.
inline void runSteps(PhysicsWorld& world, int steps)
{
    for (int i = 0; i < steps && world.getIsRunning(); ++i)
        world.integrate();
}
