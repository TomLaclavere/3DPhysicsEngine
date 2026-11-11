# Projectiles

Launches three objects with an initial velocity under gravity (g = 9.81 m/s²) and checks the
simulated contact time and landing position against the analytical ones.

```bash
./build/examples/Projectiles    # from the repository root (reads examples/Projectiles/config.yml)
```

The world is stepped with `integrateWithoutCollisions()`: each object is frozen at its first
contact with the ground plane (z = 0), so only the flight is compared.

## Scene and expected results

`x(t) = x0 + v0·t − ½·g·t²·ẑ`, and contact happens when `z(t) = h`.

| Object | Start position (m) | v0 (m/s) | Size | h (m) | Contact time (s) | Landing position (m) |
|---|---|---|---|---|---|---|
| Sphere | (0, 0, 1.5) | (1000, 0, 0) | diameter 0.2 | 0.1 | 0.53425 | (534.250, 0, 0.1) |
| Plane | (10, 0, 1) | (0, 0, 100) | size (0.2, 0.2), normal (0, 1, 0) | 0.1 (half-height) | 20.39636 | (10, 0, 0.1) |
| Cube (AABB) | (0, 10, 1) | (0, 50, 50) | edge 0.1 | 0.05 (half-edge) | 10.21264 | (0, 520.632, 0.05) |

`computation.ipynb` derives these values; `plotting.ipynb` plots the trajectories.
