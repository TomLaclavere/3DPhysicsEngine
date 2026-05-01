/**
 * @file perf_target.cpp
 *
 * @brief Minimal profiling target — pure simulation loop, zero I/O overhead.
 *
 * Runs a single fixed simulation (one integrator, one timestep) with no file
 * output during the loop. Designed to be profiled with perf so that every
 * sampled frame belongs to physics computation, not CSV writing or arg parsing.
 *
 * Usage:
 *   ./build/profiling/perf_target
 *   ./build/profiling/perf_target --solver Verlet --dt 5e-5 --dur 30
 *   ./build/profiling/perf_target --solver RK4    --dt 1e-4 --dur 20
 *   ./build/profiling/perf_target --solver Euler  --dt 1e-5 --dur 10
 *
 * Options:
 *   --solver {Euler|Verlet|RK4}  Integrator (default: Verlet)
 *   --dt     SECONDS             Timestep   (default: 5e-5)
 *   --dur    SECONDS             Total simulation duration (default: 30)
 *   --contact-forces             Use spring-damper contact instead of impulse
 */

#include "mathematics/common.hpp"
#include "mathematics/vector.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

int main(int argc, char** argv)
{
    // ── Defaults ──────────────────────────────────────────────────────────────
    const char* solver          = "Verlet";
    double      dt              = 5e-5;
    double      duration        = 30.0;
    bool        contactForces   = false;

    // ── Argument parsing ──────────────────────────────────────────────────────
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--solver") == 0 && i + 1 < argc)
            solver = argv[++i];
        else if (std::strcmp(argv[i], "--dt") == 0 && i + 1 < argc)
            dt = std::atof(argv[++i]);
        else if (std::strcmp(argv[i], "--dur") == 0 && i + 1 < argc)
            duration = std::atof(argv[++i]);
        else if (std::strcmp(argv[i], "--contact-forces") == 0)
            contactForces = true;
    }

    const int maxIter = static_cast<int>(duration / dt);

    // ── Scene setup ───────────────────────────────────────────────────────────
    Config& config = Config::get();
    config.setSolver(solver);
    config.setTimeStep(static_cast<decimal>(dt));
    config.setMaxIterations(maxIter);
    config.setSimplifiedCollision(!contactForces);

    PhysicsWorld world(config);

    auto ground = std::make_unique<Plane>(
        Vector3D(0_d), Vector3D(50_d, 50_d, 0_d), Vector3D(0_d, 0_d, 1_d));

    auto sphere = std::make_unique<Sphere>(
        Vector3D(0_d, 0_d, 20_d), 2_d, 1_d); // z0=20, radius=2, mass=1

    sphere->setIsFixed(false);
    sphere->setRestitutionCst(0.9_d);

    world.addObject(sphere.get());
    world.addObject(ground.get());
    world.start();

    // ── Status line (printed before the hot loop — not counted in profile) ────
    std::cout << "[perf_target] solver=" << solver
              << "  dt=" << dt
              << "  iter=" << maxIter
              << "  contact=" << (contactForces ? "spring-damper" : "impulse")
              << "\n"
              << "[perf_target] running — attach perf now or let profile.sh handle it\n"
              << std::flush;

    // ── Simulation loop — zero I/O ────────────────────────────────────────────
    // world.integrate() modifies the sphere's state on every call.
    // Reading final_z after the loop prevents the compiler from treating the
    // loop as dead code even under aggressive optimisation.
    for (int i = 0; i < maxIter; ++i)
        world.integrate();

    // ── One-line summary (after the loop — not in the hot path) ──────────────
    const decimal final_z = sphere->getPosition().getZ();
    const decimal final_vz = sphere->getVelocity().getZ();
    std::cout << "[perf_target] done  final_z=" << final_z
              << "  final_vz=" << final_vz << "\n";

    world.clearObjects();
    return 0;
}
