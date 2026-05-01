/**
 * @file main.cpp
 *
 * @brief Contact Forces Benchmark
 *
 * Uses setSimplifiedCollision(false): collision response is a continuous
 * spring-damper force (Hooke + viscous damping + Coulomb friction).
 * Because the force is smooth, the integrator convergence order is visible —
 * unlike the impulse benchmark where O(dt) contact-timing error hides it.
 *
 * Parameters: sphere z0=20 m, radius=2 m, mass=1 kg, k=10000 N/m, zeta=0.05
 *   - Analytical stability limit (spring): dt < 2/sqrt(k/m) = 0.02 s (Euler/Verlet)
 *                                          dt < 2*sqrt(2)/sqrt(k/m) = 0.028 s (RK4)
 *
 * Three tests, three CSV files:
 *
 *   1. convergence.csv
 *      Reference: RK4 at dt_ref=1e-4 s  →  25 test dt in [5e-4, 1.5e-2]
 *      Metrics: peak height error vs reference, flight energy drift, CPU time
 *      Expected convergence: Euler O(dt¹), Verlet O(dt²), RK4 O(dt⁴)
 *
 *   2. stability.csv
 *      7 stiffness values in [1000, 100000] N/m × 5 dt values in [1e-3, 2e-2]
 *      Determines which (solver, k, dt) combinations remain stable
 *
 *   3. energy_drift.csv
 *      E(t) time series for 3 representative dt values, all 3 solvers
 *      Shows solver-independent energy floor set by the contact damping model
 */

#include "mathematics/common.hpp"
#include "mathematics/vector.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "utilities/timer.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// ============================================================================
//  Scene constants (match these in the notebook's analytical section)
// ============================================================================
static constexpr decimal Z0      = 20_d;    // initial sphere centre height (m)
static constexpr decimal RADIUS  = 2_d;     // sphere radius (m)
static constexpr decimal MASS    = 1_d;     // sphere mass (kg)
static constexpr decimal G       = 9.81_d;  // gravitational acceleration (m/s²)
static constexpr decimal K_REF   = 1e4_d;   // stiffness for tests 1 & 3 (N/m)
static constexpr decimal ZETA    = 0.05_d;  // damping ratio (same for all tests)
static constexpr decimal DT_REF  = 1e-4_d;  // reference timestep (RK4, test 1)

// Divergence guards used in the stability test
static constexpr decimal MAX_SPEED_MULT = 50_d;  // |v| > mult * v_impact  → unstable
static constexpr decimal MAX_DEPTH_MULT = 10_d;  // z < -mult * radius     → unstable

// ============================================================================
//  Helpers
// ============================================================================
static inline decimal computeEnergy(const Sphere& sphere)
{
    const Vector3D& v = sphere.getVelocity();
    return 0.5_d * MASS * v.dotProduct(v) + MASS * G * sphere.getPosition().getZ();
}

static Sphere* makeSphere(decimal k, decimal zeta)
{
    auto* s = new Sphere(Vector3D(0_d, 0_d, Z0), RADIUS, MASS);
    s->setIsFixed(false);
    s->setStiffnessCst(k);
    s->setDampingCst(zeta);
    return s;
}

static Plane* makePlane()
{
    return new Plane(Vector3D(0_d), Vector3D(50_d, 50_d, 0_d), Vector3D(0_d, 0_d, 1_d));
}

// ============================================================================
//  Result structures
// ============================================================================
struct ConvResult
{
    decimal maxHeightError;       // relative error vs reference peaks
    decimal maxEnergyDrift;       // max |E(t) − E0| / E0
    decimal maxFlightEnergyDrift; // same, only during free-flight phases
    decimal cpuUs;
    int     bounceCount;
    bool    stable;

    std::vector<decimal> energyTimes;
    std::vector<decimal> energyDrifts;
};

struct StabResult
{
    bool    stable;
    decimal maxEnergyRatio; // max |E(t)| / E0  (>1 means energy gained)
    int     bounceCount;
};

// ============================================================================
//  Reference peak computation
//  Runs RK4 at DT_REF and records each post-bounce peak height.
//  Peaks are detected by the apex (vz goes from positive to non-positive
//  after at least one bounce).
// ============================================================================
static std::vector<decimal> computeReferencePeaks(decimal totalTime)
{
    Config& config = Config::get();
    config.setSolver("RK4");
    config.setTimeStep(DT_REF);
    config.setMaxIterations(static_cast<size_t>(totalTime / DT_REF));
    config.setSimplifiedCollision(false);

    auto* sphere = makeSphere(K_REF, ZETA);
    auto* ground = makePlane();

    PhysicsWorld world(config);
    world.addObject(sphere);
    world.addObject(ground);
    world.start();

    std::vector<decimal> peaks;
    const size_t         maxIter  = config.getMaxIterations();
    decimal              prevVz   = sphere->getVelocity().getZ();
    decimal              curPeak  = Z0;
    bool                 bounced  = false;

    for (size_t ctr = 0; ctr < maxIter && world.getIsRunning(); ++ctr)
    {
        world.integrate();

        const decimal z  = sphere->getPosition().getZ();
        const decimal vz = sphere->getVelocity().getZ();

        if (vz > 0_d)
            curPeak = std::max(curPeak, z);

        // Bounce: vz flips from non-positive to positive
        if (prevVz <= 0_d && vz > 0_d)
        {
            bounced = true;
            curPeak = z;
        }

        // Apex: vz flips from positive to non-positive, after at least one bounce
        if (prevVz > 0_d && vz <= 0_d && bounced)
        {
            peaks.push_back(curPeak);
            curPeak = RADIUS;
        }

        prevVz = vz;
    }

    world.clearObjects();
    delete sphere;
    delete ground;
    return peaks;
}

// ============================================================================
//  Convergence simulation
// ============================================================================
static ConvResult runConvergence(const std::string&          solver,
                                 decimal                     dt,
                                 size_t                      maxIter,
                                 const std::vector<decimal>& refPeaks,
                                 bool                        recordEnergy)
{
    Config& config = Config::get();
    config.setSolver(solver);
    config.setTimeStep(dt);
    config.setMaxIterations(maxIter);
    config.setSimplifiedCollision(false);

    auto* sphere = makeSphere(K_REF, ZETA);
    auto* ground = makePlane();

    PhysicsWorld world(config);
    world.addObject(sphere);
    world.addObject(ground);
    world.start();

    const decimal E0             = computeEnergy(*sphere);
    const decimal FLIGHT_THR     = RADIUS * 1.05_d;
    const decimal v_impact       = std::sqrt(2_d * G * (Z0 - RADIUS));
    decimal       E_flight_ref   = E0;

    ConvResult result;
    result.maxHeightError       = 0_d;
    result.maxEnergyDrift       = 0_d;
    result.maxFlightEnergyDrift = 0_d;
    result.cpuUs                = 0_d;
    result.bounceCount          = 0;
    result.stable               = true;

    const size_t recordEvery = recordEnergy ? std::max(size_t(1), maxIter / 500) : 0;

    decimal prevVz    = sphere->getVelocity().getZ();
    decimal curPeak   = Z0;
    bool    bounced   = false;
    int     apexCount = 0;

    Timer  timer;
    size_t ctr = 0;

    while (ctr < maxIter && world.getIsRunning())
    {
        const decimal time = static_cast<decimal>(ctr) * dt;
        world.integrate();

        const decimal z  = sphere->getPosition().getZ();
        const decimal vz = sphere->getVelocity().getZ();
        const decimal E  = computeEnergy(*sphere);

        // Divergence guard
        if (commonMaths::absVal(vz) > MAX_SPEED_MULT * v_impact ||
            z < -MAX_DEPTH_MULT * RADIUS)
        {
            result.stable = false;
            break;
        }

        // Peak tracking
        if (vz > 0_d)
            curPeak = std::max(curPeak, z);

        // Bounce
        if (prevVz <= 0_d && vz > 0_d)
        {
            result.bounceCount++;
            bounced = true;
            curPeak = z;
            if (E > 0_d) E_flight_ref = E;
        }

        // Apex — compare height against reference
        if (prevVz > 0_d && vz <= 0_d && bounced)
        {
            if (apexCount < int(refPeaks.size()) && refPeaks[apexCount] > 0_d)
            {
                const decimal err =
                    commonMaths::absVal((curPeak - refPeaks[apexCount]) / refPeaks[apexCount]);
                if (err > result.maxHeightError)
                    result.maxHeightError = err;
            }
            ++apexCount;
            curPeak = RADIUS;
        }

        prevVz = vz;

        // Total energy drift vs E0
        const decimal drift = (E0 != 0_d) ? commonMaths::absVal((E - E0) / E0)
                                          : commonMaths::absVal(E - E0);
        if (drift > result.maxEnergyDrift) result.maxEnergyDrift = drift;

        // Flight energy drift (integrator error between bounces)
        if (z > FLIGHT_THR && E_flight_ref > 0_d)
        {
            const decimal fd = commonMaths::absVal((E - E_flight_ref) / E_flight_ref);
            if (fd > result.maxFlightEnergyDrift) result.maxFlightEnergyDrift = fd;
        }

        if (recordEnergy && (ctr % recordEvery == 0))
        {
            result.energyTimes.push_back(time);
            result.energyDrifts.push_back(drift);
        }

        ++ctr;
    }

    result.cpuUs = timer.elapsedMicroseconds();
    world.clearObjects();
    delete sphere;
    delete ground;
    return result;
}

// ============================================================================
//  Stability simulation
// ============================================================================
static StabResult runStability(const std::string& solver,
                               decimal            dt,
                               decimal            k,
                               decimal            totalTime)
{
    Config& config = Config::get();
    config.setSolver(solver);
    config.setTimeStep(dt);
    config.setMaxIterations(static_cast<size_t>(totalTime / dt));
    config.setSimplifiedCollision(false);

    auto* sphere = makeSphere(k, ZETA);
    auto* ground = makePlane();

    PhysicsWorld world(config);
    world.addObject(sphere);
    world.addObject(ground);
    world.start();

    const decimal E0        = computeEnergy(*sphere);
    const decimal v_impact  = std::sqrt(2_d * G * (Z0 - RADIUS));
    const size_t  maxIter   = config.getMaxIterations();

    StabResult result { true, 1_d, 0 };
    decimal    prevVz = sphere->getVelocity().getZ();

    for (size_t ctr = 0; ctr < maxIter && world.getIsRunning(); ++ctr)
    {
        world.integrate();

        const decimal z  = sphere->getPosition().getZ();
        const decimal vz = sphere->getVelocity().getZ();

        if (commonMaths::absVal(vz) > MAX_SPEED_MULT * v_impact ||
            z < -MAX_DEPTH_MULT * RADIUS)
        {
            result.stable = false;
            break;
        }

        if (prevVz <= 0_d && vz > 0_d)
            result.bounceCount++;

        prevVz = vz;

        if (E0 != 0_d)
        {
            const decimal ratio = commonMaths::absVal(computeEnergy(*sphere) / E0);
            if (ratio > result.maxEnergyRatio)
                result.maxEnergyRatio = ratio;
        }
    }

    world.clearObjects();
    delete sphere;
    delete ground;
    return result;
}

// ============================================================================
//  main
// ============================================================================
int main()
{
    const std::array<std::string, 3> solvers { "Euler", "Verlet", "RK4" };

    // -----------------------------------------------------------------------
    // 1. CONVERGENCE TEST
    // -----------------------------------------------------------------------
    const decimal totalTimeConv = 10_d;

    std::cout << "=== Computing reference (RK4, dt=" << DT_REF
              << ", k=" << K_REF << ", zeta=" << ZETA << ") ===\n";
    const std::vector<decimal> refPeaks = computeReferencePeaks(totalTimeConv);
    std::cout << "Reference peaks detected: " << refPeaks.size() << "\n\n";

    // 25 dt values log-spaced in [5e-4, 1.5e-2]
    // All lie below the Euler/Verlet stability limit 2/sqrt(k/m) = 0.02 s
    constexpr size_t  N_DT  = 25;
    constexpr decimal DT_LO = 5e-4_d;
    constexpr decimal DT_HI = 1.5e-2_d;

    std::array<decimal, N_DT> dts;
    std::array<size_t,  N_DT> iters;
    for (size_t i = 0; i < N_DT; ++i)
    {
        const decimal alpha = decimal(i) / decimal(N_DT - 1);
        dts[i]  = DT_LO * std::pow(DT_HI / DT_LO, alpha);
        iters[i] = static_cast<size_t>(totalTimeConv / dts[i]);
    }

    // Three dt indices for E(t) recording (small, mid, large)
    const std::array<size_t, 3> energyIdx { 2, 12, 22 };
    auto shouldRecord = [&](size_t j) {
        for (auto idx : energyIdx) if (idx == j) return true;
        return false;
    };

    std::array<std::array<ConvResult, N_DT>, 3> convRes;

    std::cout << "=== Convergence test ===\n";
    for (size_t iS = 0; iS < solvers.size(); ++iS)
    {
        std::cout << "Solver: " << solvers[iS] << "\n";
        for (size_t jDt = 0; jDt < N_DT; ++jDt)
        {
            convRes[iS][jDt] = runConvergence(
                solvers[iS], dts[jDt], iters[jDt], refPeaks, shouldRecord(jDt));

            const auto& r = convRes[iS][jDt];
            std::cout << "  dt=" << dts[jDt]
                      << "  height_err=" << r.maxHeightError
                      << "  bounces="    << r.bounceCount
                      << "  flt_drift="  << r.maxFlightEnergyDrift
                      << "  stable="     << (r.stable ? "yes" : "NO")
                      << "  cpu="        << r.cpuUs << " µs\n";
        }
    }

    // convergence.csv
    {
        std::ofstream f("benchmarks/Contact_Forces/convergence.csv");
        if (!f) { std::cerr << "Cannot open convergence.csv\n"; return 1; }
        f << "solver,dt,max_height_error,max_energy_drift,max_flight_energy_drift,"
             "cpu_us,bounce_count,stable\n";
        for (size_t iS = 0; iS < solvers.size(); ++iS)
            for (size_t jDt = 0; jDt < N_DT; ++jDt)
            {
                const auto& r = convRes[iS][jDt];
                f << solvers[iS]            << ","
                  << dts[jDt]               << ","
                  << r.maxHeightError       << ","
                  << r.maxEnergyDrift       << ","
                  << r.maxFlightEnergyDrift << ","
                  << r.cpuUs               << ","
                  << r.bounceCount         << ","
                  << (r.stable ? 1 : 0)    << "\n";
            }
    }

    // energy_drift.csv
    {
        std::ofstream f("benchmarks/Contact_Forces/energy_drift.csv");
        if (!f) { std::cerr << "Cannot open energy_drift.csv\n"; return 1; }
        f << "solver,dt,time,energy_drift\n";
        for (size_t iS = 0; iS < solvers.size(); ++iS)
            for (size_t jDt : energyIdx)
            {
                const auto& r = convRes[iS][jDt];
                for (size_t k = 0; k < r.energyTimes.size(); ++k)
                    f << solvers[iS]        << ","
                      << dts[jDt]           << ","
                      << r.energyTimes[k]   << ","
                      << r.energyDrifts[k]  << "\n";
            }
    }

    // -----------------------------------------------------------------------
    // 2. STABILITY TEST
    // -----------------------------------------------------------------------
    // Theoretical stability limits (undamped spring, symplectic methods):
    //   Euler / Verlet : omega * dt < 2        →  dt_max = 2 / sqrt(k/m)
    //   RK4            : omega * dt < 2*sqrt(2) →  dt_max = 2*sqrt(2) / sqrt(k/m)
    const decimal totalTimeStab = 5_d;

    constexpr size_t N_K       = 7;
    constexpr size_t N_DT_STAB = 5;
    const std::array<decimal, N_K>       kVals  { 1e3_d, 2e3_d, 5e3_d, 1e4_d, 2e4_d, 5e4_d, 1e5_d };
    const std::array<decimal, N_DT_STAB> dtVals { 1e-3_d, 2e-3_d, 5e-3_d, 1e-2_d, 2e-2_d };

    std::cout << "\n=== Stability test ===\n";
    std::ofstream stabF("benchmarks/Contact_Forces/stability.csv");
    if (!stabF) { std::cerr << "Cannot open stability.csv\n"; return 1; }
    stabF << "solver,k,dt,omega_dt,dt_limit_euler_verlet,dt_limit_rk4,"
             "is_stable,max_energy_ratio,bounce_count\n";

    for (size_t iS = 0; iS < solvers.size(); ++iS)
    {
        std::cout << "Solver: " << solvers[iS] << "\n";
        for (decimal k : kVals)
        {
            const decimal omega      = std::sqrt(k / MASS);
            const decimal dtLimEuler = 2_d / omega;
            const decimal dtLimRK4   = 2_d * std::sqrt(2_d) / omega;

            for (decimal dt : dtVals)
            {
                const StabResult sr = runStability(solvers[iS], dt, k, totalTimeStab);

                std::cout << "  k=" << k << " dt=" << dt
                          << " omega*dt=" << omega * dt
                          << " stable=" << (sr.stable ? "yes" : "NO")
                          << " E_ratio=" << sr.maxEnergyRatio << "\n";

                stabF << solvers[iS]        << ","
                      << k                  << ","
                      << dt                 << ","
                      << omega * dt         << ","
                      << dtLimEuler         << ","
                      << dtLimRK4           << ","
                      << (sr.stable ? 1 : 0) << ","
                      << sr.maxEnergyRatio  << ","
                      << sr.bounceCount     << "\n";
            }
        }
    }

    std::cout << "\nCSV files written to benchmarks/Contact_Forces/\n";
    return 0;
}
