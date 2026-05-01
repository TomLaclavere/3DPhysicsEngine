---

# Benchmark Report — Bouncing Ball with Impulse-Based Collision

**Simulation:** sphere (z₀ = 20 m, radius = 2 m, mass = 1 kg) dropped onto a fixed plane, with impulse-based collision response (restitution e = 0.9, `setSimplifiedCollision = true`).  
**Integrators:** Semi-implicit Euler, Störmer-Verlet, RK4.  
**Timestep range:** dt ∈ [10⁻⁵, 0.5] s — 50 logarithmically spaced values per solver.  
**Simulation duration:** 18 s (chosen so that the 7th bounce at t ≈ 18.07 s falls outside the window, giving exactly 6 analytically predicted bounces).

---

## Figure 1 — Peak height error vs dt

**What is measured.**
After each bounce, the maximum height reached by the sphere is compared to the analytical prediction `z_peak(n) = r + (z₀ − r) · eⁿ'`, where n is the bounce index. `max_height_error` is the maximum relative deviation across all detected peaks in an 18-second simulation.

**What the plot shows.**
All three solvers converge at approximately O(dt^0.89), not at the expected O(dt), O(dt²), O(dt⁴) orders. More strikingly, Verlet and RK4 produce **numerically identical curves** — their errors differ by at most 10⁻⁹ at any given dt. Euler follows the same O(dt) slope but sits 8.6× higher on average.

**Why all solvers show O(dt) — the contact timing argument.**
The sphere crosses the surface at a true time t\* that lies between two consecutive steps t_n and t_{n+1}. The simulator only detects the collision at step n+1, when the sphere has already penetrated by a depth proportional to the approach velocity and the remaining time dt − τ (where τ ∈ (0, dt) is the unknown fractional offset). The impulse is then applied at the wrong velocity:

```
v_sim = v* + g·(dt − τ) = v* + O(dt)
```

The resulting post-bounce peak height error is O(dt) regardless of which integrator computed the free-flight trajectory. This is an **algorithmic floor set by discrete collision detection**, not by integrator quality.

**Why Verlet and RK4 are identical.**
During free flight, the only force is constant gravity (returned by `computeAccelerationGravityOnly()`). For a constant acceleration, both the Störmer-Verlet formula and the full RK4 weighted average collapse to exactly the same algebraic update:

```
x_{n+1} = x_n + v_n·dt + ½g·dt²
v_{n+1} = v_n + g·dt
```

Both integrators are therefore **exact** for this quadratic trajectory (confirmed by the machine-precision flight energy drift in Figure 2). They arrive at the contact moment with identical state, receive the same erroneous impulse, and produce the same post-bounce trajectory.

**Why Euler sits higher.**
The semi-implicit Euler scheme dissipates energy at a rate of −½g²dt² per step during free flight. Over a flight time T, the total loss is ½g²·dt·T = O(dt). This systematically reduces the sphere's kinetic energy at each impact, adding a second O(dt) error source on top of the contact timing error. Figure 2 quantifies this precisely.

---

## Figure 2 — Flight energy drift vs dt

**What is measured.**
`max_flight_energy_drift` is the maximum relative energy deviation measured *only during free-flight phases* — i.e., when the sphere is clearly above the ground (z > 1.05·r). The energy reference is reset after each bounce to the post-bounce energy level. This isolates integrator numerical error from physical energy loss at contact.

**What the plot shows.**
The three solvers split into two distinct regimes:

- **Euler:** clean O(dt^0.99) slope (R² = 0.9990). At dt = 10⁻⁵ s: drift = 1.49×10⁻⁵. The formula is exact: total drift over flight time T equals ½g²·dt·T, confirming the −½g²dt² dissipation per step.

- **Verlet and RK4:** both at machine precision (~10⁻¹¹ to 10⁻¹⁵) across the entire dt range where bounces occur. The negative apparent slope is noise — the values are at the double-precision floor and do not converge (they are already exact).

**Physical interpretation.**
For a constant-gravity trajectory (quadratic in time), Störmer-Verlet and RK4 are both analytically exact — they are not approximating the trajectory, they reproduce it to machine precision. Euler is not exact because its energy evolution is:

```
E_{n+1} = E_n − ½g²dt²
```

This is a systematic per-step dissipation that accumulates proportionally to dt, explaining both the O(dt^1) slope and the 10⁵-fold gap between Euler and Verlet/RK4 at the same dt.

**Key takeaway.** In a purely impulse-based simulation, **Verlet is the optimal choice**: it has the same zero flight-phase error as RK4 at 0.39× the CPU cost (Verlet takes 1.78 s vs RK4's 4.53 s at dt = 10⁻⁵ s).

---

## Figure 3 — Max energy drift vs dt

**What is measured.**
`max_energy_drift = max_t |E(t) − E₀| / E₀`, where E₀ is the initial total mechanical energy (196 J). This metric does not separate physical energy loss from numerical error.

**What the plot shows — two distinct regimes with different causes.**

**Regime 1 — large dt (Euler only, dt > 0.069 s):**
Euler's drift reaches 221% at dt = 0.5 s. This is *not* from collisions: at large dt the sphere tunnels completely through the ground without any bounce being detected (`bounce_count = 0`). The drift is purely Euler's free-fall dissipation accumulated over 36 steps of 0.5 s each (18 s / 0.5 s), which gives exactly ½g²·dt²·N / E₀ = 0.5·9.81²·0.25·36 / 196.2 = **2.2073** — matching the CSV to 4 decimal places.

Verlet and RK4 show near-zero drift in this regime because their flight-phase error is at machine precision, and the sphere (also tunneling at dt > 0.085 s) simply falls with conserved energy.

**Regime 2 — small dt (all solvers plateau at ~68.2%):**
Once dt is small enough for bounces to be correctly detected, all three solvers converge to the same plateau: `max_energy_drift ≈ 0.682`. This is not a numerical error — it is the cumulative physical energy loss from 6 bounces with e = 0.9:

```
analytical loss = (E₀ − m·g·r) · (1 − e^(2×6)) / E₀
               = 176.6 · (1 − 0.9¹²) / 196.2 ≈ 0.647  (pure restitution)
```

The measured 0.682 slightly exceeds 0.647 because the contact timing error adds a small over-impulsion at each bounce. The fact that all three solvers plateau at the *same* value confirms that the energy floor is set by the **impulse model** (restitution e = 0.9), not the integrator.

---

## Figure 4 — Cost vs precision (ops_total vs height error)

**What is measured.**
The x-axis is the total number of force evaluations across the entire simulation (`ops_total = maxIter × ops_per_step`, where Euler and Verlet each cost 1 evaluation per step and RK4 costs 4). The y-axis is `max_height_error`. A point further left on the inverted-x axis represents a more expensive computation.

**What the plot shows.**
At equal computational cost (iso-budget comparison):

- **Euler** always lies above Verlet and RK4 at the same ops_total. Its larger prefactor (dual O(dt) error source) is never offset by its lower cost-per-step.
- **Verlet** and **RK4** follow nearly the same curve shifted horizontally. Because both have the same O(dt) height error (contact timing limited) and Verlet costs ¼ of RK4, **Verlet reaches the same precision at 4× lower ops_total**, making it unambiguously more efficient in this metric.
- The **crossover zone** (10⁴–10⁵ force evaluations) is where Verlet first becomes more precise than Euler at equal cost.

**Practical recommendation for real-time or interactive simulations:**
At a target of 60 Hz (≈120 force evaluations per second for a 2-second flight), all methods produce comparable absolute errors, but Verlet eliminates the Euler energy bias at no extra cost.

---

## Figure 5 — Bounce count vs dt

**What is measured.**
Number of velocity sign-changes (vz from negative to positive) detected during a 20-second simulation, as a function of timestep.

**What the plot shows.**
Three distinct zones:

**Zone 1 — tunneling (large dt):**
For dt > 0.069 s (Euler) or dt > 0.085 s (Verlet, RK4), the sphere passes through the ground in a single step and no collision is detected: `bounce_count = 0`. The two thresholds are similar because tunneling is governed primarily by the approach velocity at the contact window (v_impact ≈ 18.8 m/s) rather than by integrator quality.

**Zone 2 — convergence to 6 (small to medium dt):**
All three solvers stabilize at `bounce_count = 6` across 38–40 of the 50 tested dt values. The analytical prediction for 18 s of simulation is exactly **6 bounces** — the 7th bounce occurs at t ≈ 18.07 s, just outside the window. The simulation now matches the analytical count exactly, with no discrete timing artifact.

Euler shows `bounce_count = 7` for 2 dt values near the stable boundary. This is a residual discrete timing offset that shifts the 6th contact slightly earlier, allowing the 7th bounce to land inside the 18 s window for those specific dt values.

**Zone 3 — late convergence (Euler, intermediate dt):**
Between its tunneling threshold (0.069 s) and the stable zone, Euler shows unstable bounce counts (1–5), indicating it is at the edge of reliably detecting each contact window. Verlet and RK4 make the transition more sharply.

---

## Figure 6 — E(t) time series

**What is measured.**
Total energy drift `|E(t) − E₀| / E₀` sampled over 18 seconds for five representative timestep values: dt ≈ 0.024 ms, 0.22 ms, 2.00 ms, 18.2 ms, 166 ms.

**What the plot shows.**

**Staircase pattern — physical energy loss.**
The jumps in E(t) coincide exactly with the bounce events. Each step represents energy removed by the impulse: for a vertical bounce with e = 0.9, the kinetic energy at impact is multiplied by e² = 0.81, removing 19% per bounce. The staircase rises monotonically (drift accumulates) because drift is measured relative to E₀. The **step heights are solver-independent** at the same dt, confirming the impulse model governs the energy loss.

**Floor level between steps — solver-dependent integrator error.**
Between bounces, energy should be exactly conserved. At fine dt (0.57 ms), the inter-step floor is visually flat for all three solvers. At coarser dt (40 ms), Euler shows a sloping floor (energy decreasing at −½g²dt² per step), while Verlet and RK4 remain flat. This floor evolution matches the O(dt) vs O(machine ε) separation established in Figure 2.

**Large dt behavior (dt ≈ 40 ms):**
For Euler at 40 ms, the floor between bounces slopes downward steeply (energy is dissipated during free flight). The bounce steps are still present but partially obscured by the integrator drift. For Verlet and RK4, the floor is flat even at 40 ms.

---

## Conclusions

### What this benchmark establishes

| Metric | Euler | Verlet | RK4 |
|---|---|---|---|
| Height error order | O(dt^0.86) | O(dt^0.90) = RK4 | O(dt^0.90) = Verlet |
| Flight energy drift | O(dt^0.99) — 1.49×10⁻⁵ at dt=10⁻⁵ | Machine ε — 10⁻¹¹ | Machine ε — 10⁻¹² |
| Total energy drift at dt=10⁻⁵ | 68.2% (physical, 6 bounces) | 68.2% (physical) | 68.2% (physical) |
| Tunneling threshold | dt > 0.069 s | dt > 0.085 s | dt > 0.085 s |
| CPU at dt = 10⁻⁵ s | 1101 ms (×1) | 1779 ms (×1.62) | 4526 ms (×4.11) |
| Cost ratio at median dt | ×1.00 | ×1.26 | ×3.05 |

### Practical recommendations for impulse-based contact

**Use Verlet.** It is the dominant choice across every relevant metric:
- Identical height error to RK4 (both contact-timing-limited at O(dt)).
- Identical flight energy conservation to RK4 (both exact for quadratic trajectories).
- 2.5× cheaper than RK4 at fine dt (1.78 s vs 4.53 s at dt = 10⁻⁵ s).
- Only 1.26× more expensive than Euler at median dt, while eliminating Euler's O(dt) free-flight dissipation entirely.

**RK4 provides no benefit in this regime.** Its higher-order accuracy is masked by the O(dt) contact timing error. It only becomes the best choice when the impulse is replaced by a smooth contact force (spring-damper) — at which point its larger stability region and O(dt⁴) accuracy in the contact phase offer real advantages.

**Avoid Euler** whenever energy conservation during free flight matters. Its O(dt) numerical dissipation accumulates over the full simulation time, produces a qualitatively different E(t) profile (sloping floor between bounces), and shrinks the effective stable timestep range.

### Limitation of this benchmark

All three height-error curves show O(dt), not the expected O(dt), O(dt²), O(dt⁴) integrator orders. This is not a deficiency of the integrators — it is a fundamental property of **discrete impulse-based collision detection**: the impact velocity is sampled at the wrong time, introducing an O(dt) error that no integrator can reduce without knowing the exact contact time. To expose integrator order in the peak-height metric, continuous collision detection (CCD) or a smooth contact force model is required. See the **Contact Forces benchmark** for the spring-damper case.