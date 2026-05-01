---

# Benchmark Report — Bouncing Ball with Contact Forces (Spring-Damper)

**Simulation:** sphere (z₀ = 20 m, radius = 2 m, mass = 1 kg) dropped onto a fixed plane, using a continuous spring-damper contact model (k = 10 000 N/m, ζ = 0.05, `setSimplifiedCollision = false`).  
**Integrators:** Semi-implicit Euler, Störmer-Verlet, RK4.  
**Reference solution:** RK4 at dt = 10⁻⁴ s (3 peaks detected, well inside stability bounds).  
**Test range:** dt ∈ [5×10⁻⁴, 1.5×10⁻²] s — 25 logarithmically spaced values per solver.

**Key scene parameters:**
- ω = √(k/m) = 100 rad/s — natural frequency of the contact spring
- T_contact = π/ω = 31.4 ms — duration of one contact phase
- v_impact = √(2g·18) = 18.8 m/s — sphere velocity at first contact
- Stability limit (Euler/Verlet): dt < 2/ω = 20.0 ms
- Stability limit (RK4): dt < 2√2/ω = 28.3 ms

---

## Figure 1 — Peak height error vs dt

**What is measured.**
Peak heights from each test simulation are compared to those of the reference solution (RK4 at dt = 10⁻⁴ s, analytically near-exact). The error is the maximum relative deviation across all matched post-bounce peaks.

**What the plot shows — three distinct curves, unlike the impulse benchmark.**
In the impulse-based benchmark, Verlet and RK4 produced numerically identical height-error curves. Here they are separated by up to **337×** at the same dt, with RK4 consistently the most accurate. The measured regression slopes are:

| Solver | Slope | R² | Error at dt=5×10⁻⁴ | Error at dt=1.5×10⁻² |
|---|---|---|---|---|
| Euler | 1.28 | 0.845 | 4.3×10⁻³ | 7.3×10⁻¹ |
| Verlet | 1.17 | 0.871 | 4.5×10⁻³ | 2.6×10⁻¹ |
| RK4 | 2.37 | 0.856 | 7.1×10⁻⁵ | 3.2×10⁻¹ |

At fine dt, RK4 is **60× more accurate** than Euler and Verlet. This is the key result: with a smooth continuous force, integrator order *is* visible.

**Why integrator order appears here but not in the impulse benchmark.**
In the impulse case, the dominant error was contact timing: the velocity at impact is discretely sampled, introducing O(dt) error for all integrators. With a continuous spring-damper force, the contact force is evaluated *at each integrator stage during the contact phase*:

- Euler evaluates the contact force once (at position x_n).
- Verlet evaluates at x_n and x_{n+1}, averaging the accelerations.
- RK4 evaluates at four intermediate positions, achieving near-O(dt⁴) local accuracy.

The contact phase has T_contact = 31.4 ms. For the finest tested dt (0.5 ms), this spans ~63 substeps — enough for the integrator's local accuracy to accumulate into a measurable global difference. At the coarsest dt (15 ms), the contact is resolved in only ~2 steps, and all solvers degrade.

**Why the slopes are not clean O(1)/O(2)/O(4).**
The contact force model applies a `max(F_spring + F_damp, 0)` clamp. This clamp is zero when the sphere is above the surface and non-zero when it penetrates, creating a **C⁰ discontinuity** in the ODE right-hand side at the moments of contact onset and separation. A discontinuous derivative degrades the global convergence order: Runge-Kutta methods achieve O(dt^p) only when the force has p continuous derivatives. The clamp limits the effective order to O(dt^1) at the discontinuous instants, which explains why the measured slopes (~1.2–2.4) are all below their nominal values (1, 2, 4). RK4 is still substantially more accurate because it has more stages to average over the contact phase, but its advantage is capped below O(dt⁴).

**Why Verlet does not behave like RK4.**
In the impulse benchmark, Verlet and RK4 were identical because the only force was constant gravity, for which both reduce to the same exact formula. Here, the contact force varies with position during the contact phase. Verlet evaluates the acceleration at x_n and x_{n+1} (trapezoidal), which is second-order accurate for the velocity update but shares the C⁰ limitation. RK4 evaluates at four stages, achieving much better coverage of the contact arc. At dt = 5×10⁻⁴ s (62 steps per contact), RK4 is already 60× more accurate than Verlet.

---

## Figure 2 — Convergence slopes (regression)

The regression confirms the three-way separation. Points at very large dt (coarsest resolution: ~2 steps per contact phase) show outlier errors as the contact model breaks down, which reduces the R² values to ~0.85. The central trend — Euler ~O(dt^1.3), Verlet ~O(dt^1.2), RK4 ~O(dt^2.4) — is physically meaningful and reproducible.

The observation that Euler (slope 1.28) is *slightly better* than Verlet (slope 1.17) in terms of measured slope is a consequence of the C⁰ discontinuity: both are bounded by the same O(dt) floor at the contact instants, but Euler's systematic flight-phase dissipation partially offsets the contact error in the opposite direction, coincidentally improving the slope estimate. The absolute errors (prefactors) tell the more important story: Euler is always 1×–50× worse than Verlet at the same dt, and Verlet is 5×–337× worse than RK4.

---

## Figure 3 — Stability map

**What is measured.**
For each combination of solver, stiffness k, and timestep dt, the simulation runs for 5 seconds and records `max_energy_ratio = max_t |E(t)| / E₀`. A ratio above 1.0 indicates energy injection (the solver is amplifying the system). A ratio above 2–5 signals practical instability.

**What the map shows.**

The stability map reveals a striking difference between the three solvers:

**Euler** becomes unstable (ratio > 5×) for k = 10 000 N/m at dt = 20 ms and for k = 100 000 N/m at dt = 10 ms. Its maximum stable k (for all tested dt values) is only **5 000 N/m**. The worst observed ratio is 32.7× (k=10⁵, dt=10ms).

**Verlet** behaves similarly to Euler at moderate k, but catastrophically at high k and large dt: k = 100 000 N/m at dt = 20 ms gives a ratio of **355×** — the solver injects 355 times the initial energy, indicating complete numerical explosion. Its maximum stable k is 10 000 N/m (the reference k used for convergence tests).

**RK4** remains stable across the entire tested range. Its maximum observed ratio is only 1.9× (k=10⁵, dt=20ms), indicating it is near the stability boundary but not crossing it. Its maximum stable k across all tested dt is **100 000 N/m** — 20× larger than Euler and 10× larger than Verlet.

**Why Verlet is less stable than Euler at high k.**
The stability condition for a linear spring is ω·dt < 2 for both symplectic Euler and Störmer-Verlet when no damping is present. With damping, the two methods differ. In the code, Euler applies the contact acceleration *once* per step before integrating (using the force at x_n). Verlet evaluates the acceleration at both x_n and x_{n+1}, then averages. At large ω·dt, the position x_{n+1} computed by Verlet may lie deep inside the contact zone, producing a large force at the second evaluation point that the trapezoidal average cannot damp — leading to energy injection that Euler, evaluating only at x_n, avoids. This is a known pathology of Velocity-Verlet near the stability boundary for stiff forces.

**Why RK4 is dramatically more stable.**
For a linear ODE with eigenvalue iω, RK4's stability region extends to ω·dt < 2√2 ≈ 2.83, versus 2.0 for Euler/Verlet. This 41% advantage in dt corresponds to a factor of (2√2/2)² = 2 in the maximum stable k at fixed dt. At k = 100 000 N/m (ω = 316 rad/s), the limits are:

```
Euler/Verlet:   dt_max = 2/316 = 6.3 ms
RK4:            dt_max = 2√2/316 = 8.9 ms
```

Both 5 ms and 10 ms fall outside the Euler/Verlet stable region, but RK4 remains within its boundary at dt = 5 ms and near the boundary at dt = 10 ms. The energy ratios confirm this: RK4 at k=10⁵, dt=10ms → 1.9×; Euler at the same point → 32.7×.

---

## Figure 4 — Stability boundary: measured vs theoretical

**What the plot shows.**
The measured first-unstable dt (first tested dt where max_energy_ratio > 2×) matches the theoretical stability boundary dt = 2/ω very closely for Euler and Verlet, and dt = 2√2/ω for RK4. The measured points fall near or slightly above the theoretical lines because:
1. The theory is derived for an undamped spring; the ζ = 0.05 damping provides a small extra stabilising margin.
2. The stability condition is asymptotic (it describes the limit of bounded growth, not the first step where growth becomes visible), so the boundary in practice is slightly above the theoretical limit.

**The 41% advantage of RK4 is directly visible** as a vertical gap between the red (Euler/Verlet) and green (RK4) theoretical lines. At k = 50 000 N/m: Euler/Verlet are stable only for dt ≤ 8.9 ms, while RK4 is stable up to 12.6 ms.

**Practical consequence for HPC simulations.**
In a production simulation, the timestep is constrained by the stiffest contact in the scene. If that contact has k = 50 000 N/m:
- Euler/Verlet require dt ≤ 8.9 ms → at least 1 124 steps per 10 s
- RK4 allows dt ≤ 12.6 ms → 794 steps per 10 s at 4 force evaluations each = 3 176 equivalent evaluations
- Euler/Verlet at dt = 8.9 ms: 1 124 force evaluations (1 per step)

RK4 requires 3 176 vs 1 124 equivalent evaluations — it is 2.8× more expensive than Euler/Verlet for the same physical time, despite its 41% larger stable dt. **For highly stiff contacts where Euler/Verlet are near their stability limit, RK4's accuracy advantage becomes decisive, but its cost advantage disappears.** The crossover depends on the error tolerance required.

---

## Figure 5 — Energy drift E(t) time series

**What is measured.**
Total energy drift `|E(t) − E₀| / E₀` over the 10-second simulation, sampled at 500 points for three representative timesteps: dt ≈ 0.66 ms (~47 steps/contact), 2.74 ms (~11 steps/contact), and 11.3 ms (~2.8 steps/contact).

**What the plot shows.**

**Staircase steps — physical energy loss from damping.**
As in the impulse case, the steps in E(t) coincide with bounce events. Each step represents energy dissipated by the viscous damping term `c·min(v_n, 0)` during the contact phase. At fine dt (0.66 ms), all three solvers produce nearly identical step heights — the damping physics is well-resolved and integrator-independent. At coarser dt (11.3 ms), the step heights diverge: Euler and Verlet over-estimate the energy loss because their coarse sampling of the contact force distorts the integral of the damping term.

**Between-step floor — solver-independent energy drift.**
Unlike the impulse benchmark — where the floor between steps sloped downward for Euler (free-flight numerical dissipation) — here the drift pattern between steps is more complex. At fine dt, all solvers show a relatively flat floor between bounces, confirming that free-flight energy conservation is near-exact for all three (constant gravity, quadratic trajectory, exact for all integrators as established in the impulse benchmark). At coarser dt, the floor is not flat because the contact force acts throughout the contact phase, and integrator errors during that phase compound with the inter-bounce free-flight.

**Solver-independence of the energy floor at fine dt.**
At dt = 0.66 ms, the final measured drift values for all three solvers converge to ~0.96 (96% energy loss over 10 seconds, 3 bounces with ζ = 0.05 damping). The near-equal plateau confirms that the total energy dissipation is determined by the physical contact model parameters (k, ζ), not the integrator, when dt is small enough to resolve the contact phase properly (here, ~47 steps per contact). This is the contact forces counterpart of the impulse benchmark's restitution floor.

---

## Conclusions

### What this benchmark establishes

| Metric | Euler | Verlet | RK4 |
|---|---|---|---|
| Convergence slope (height error) | ~O(dt^1.3) | ~O(dt^1.2) | ~O(dt^2.4) |
| Height error at dt = 5×10⁻⁴ s | 4.3×10⁻³ | 4.5×10⁻³ | **7.1×10⁻⁵** |
| CPU at dt = 5×10⁻⁴ s | 11 ms (×1) | 23 ms (×2.1) | 59 ms (×5.4) |
| Max stable k (all tested dt) | 5 000 N/m | 10 000 N/m | **100 000 N/m** |
| Worst energy ratio seen | 32.7× | **355×** | 1.9× |
| Energy drift at fine dt (physical) | ~96% | ~96% | ~96% |

### Fundamental difference from the impulse benchmark

In the impulse-based case, Verlet and RK4 were algorithmically equivalent (both exact for quadratic free flight, same O(dt) contact timing error). The dominant physics made the choice of integrator irrelevant.

In the contact forces case, each integrator's behaviour during the **contact phase** (31.4 ms long) determines both accuracy and stability. The continuous spring-damper force creates a proper ODE that the integrators solve with different accuracy and different stability properties. The choice of integrator now matters decisively.

### Practical recommendations for contact force simulations

**Use RK4 when accuracy is the priority.** It is 60× more accurate than Euler/Verlet at fine dt, has a 41% larger stable dt range, and never approaches the catastrophic instability seen in Verlet (355× energy ratio). At k = 10 000 N/m, RK4 at dt = 10 ms (3.1 steps per contact) achieves comparable accuracy to Euler at dt = 2 ms (15 steps per contact) — a 5× reduction in step count that partially offsets RK4's 5× higher cost per step.

**Avoid Verlet for stiff contacts.** Verlet's stability pathology near the ω·dt = 2 boundary (355× energy injection at k=10⁵, dt=20ms) makes it **less safe than Euler** in the stiff regime. The Velocity-Verlet force-averaging scheme amplifies large forces computed at the predicted position x_{n+1}, which can become dangerously large when the step is only partially inside the contact zone. This is a well-known issue in molecular dynamics (where Verlet is standard) when simulating stiff bonded interactions.

**Euler is the conservative fallback.** It underperforms RK4 in accuracy by 60× but never exhibits catastrophic instability within the tested range. For rapid prototyping with unknown stiffness parameters, Euler's predictable stability behaviour (clean ω·dt < 2 boundary) makes it easier to work with safely.

### Connection to the impulse benchmark

The two benchmarks are complementary:

| Aspect | Impulse (e = 0.9) | Contact forces (k=10⁴, ζ=0.05) |
|---|---|---|
| Force during contact | Instantaneous, discontinuous | Continuous, C⁰ |
| Integrator order visible? | No — O(dt) contact timing floor | Yes — partially visible |
| Verlet vs RK4 in accuracy | Identical | RK4 60× more accurate |
| Stability concern | Tunneling at large dt | Energy injection at large k·dt² |
| Recommended solver | Verlet | RK4 |
| Energy floor cause | Impulse restitution (e = 0.9) → 73% | Spring-damper dissipation (ζ=0.05) → 96% |

The contact forces model is physically richer and numerically more demanding. It is the appropriate choice when simulating: compliant surfaces, material contact mechanics, dense particle packings, or any scenario where the contact duration is long compared to the timestep and physical stiffness matters.