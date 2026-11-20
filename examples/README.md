# Examples

| Example | Executable | What it shows |
|---|---|---|
| [Free_Fall](Free_Fall/) | `Free_Fall` | Three objects fall; contact times checked against the analytical ones |
| [Projectiles](Projectiles/) | `Projectiles` | Three launched objects; contact times and landing points checked |
| Bouncing_Ball | `Bouncing` | Wood, steel and rubber balls bouncing on the ground |
| Bouncing_Ball_2 | `Bouncing_2` | A ball bouncing between two tilted planes |
| Rolling_Ball | `Rolling` | A ball rolling down a ramp towards a wall |
| Random_Balls_2D | `Random2D` | Many balls with random velocities in a closed box (x-z plane), without gravity |

Run from the repository root (each example reads its `config.yml` and writes its CSV there):

```bash
./build/examples/Bouncing           # or: cmake --build build --target Bouncing
./build/3DPhysicsViewer examples/Bouncing_Ball/CSV   # replay in 3D
```

Command-line flags override `config.yml`, e.g. `--solver RK4 --timestep 0.001`.
