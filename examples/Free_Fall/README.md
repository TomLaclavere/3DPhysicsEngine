# Free Fall

Drops three objects under gravity (g = 9.81 m/s²) and checks that the simulated time of contact
with the ground matches the analytical one, within 2 timesteps.

```bash
./build/examples/Free_Fall    # from the repository root (reads examples/Free_Fall/config.yml)
```

The world is stepped with `integrateWithoutCollisions()`: each object is frozen at its first
contact with the ground plane (z = 0), so only the free flight is compared.

## Scene and expected results

Along z: `z(t) = z0 + v0·t − ½·g·t²`, and contact happens when `z(t) = h`.

| Object | Start position (m) | v0 (m/s) | Size | Contact height h (m) | Contact time (s) |
|---|---|---|---|---|---|
| Sphere | (0, 0, 20) | (0, 0, −1) | diameter 0.2 | 0.1 (radius) | 1.91486 |
| Plane | (10, 0, 15) | 0 | size (1, 0.4) = height × width, normal (0, 1, 0) | 0.5 (half-height) | 1.71935 |
| Cube (AABB) | (0, 10, 10) | 0 | edge 0.15 | 0.075 (half-edge) | 1.42248 |

`plotting.ipynb` plots the trajectories from the CSV output.
