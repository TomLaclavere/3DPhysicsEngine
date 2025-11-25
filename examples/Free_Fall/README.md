# Goal

The goal of this example is to demonstrate the robustness of the free-fall simulation implemented in `3DPhysicsEngine`.

---

# Features

This example adds several objects to the physical world and lets them fall under Earth's gravity.  
Each object stops when it reaches the ground.  

For every object, the acceleration, velocity, position, and impact time are computed and compared with analytical results derived below.  

**Note:**  
Bouncing is not yet implemented — objects stop instantaneously when they hit the ground.

---

# Analytical Computation

The gravitational acceleration on Earth is:

g = 9.81 m/s²

From the fundamental law of dynamics (along the vertical axis):

a = -g
v = -g·t + v0
h = -½·g·t² + v0·t + x0

---

## Object Definitions

| Object | Initial Position `x0` | Initial Velocity `v0` | Size / Shape Properties |
|:--------|:----------------------:|:----------------------:|:------------------------|
| **Sphere** | (0, 0, 20) | (0, 0, 1) | Diameter `d = 0.2 m` |
| **Plane**  | (10, 0, 15) | (0, 0, 0) | Normal `n = (0, 1, 0)`, Half-width `0.5 m`, Half-height `0.2 m` |
| **Cube**   | (0, 10, 10) | (0, 0, 0) | Edge length `a = 0.15 m` |

---

## Contact Conditions

| Object | Ground Contact Condition (`h`) |
|:--------|:-------------------------------|
| **Sphere** | `h = d / 2` |
| **Plane**  | `h = half-height` |
| **Cube**   | `h = a / √2` |

---

## Contact Time Computations

Using `h(t) = -½·g·t² + v0·t + x0`, the impact times are:

| Object | Equation | Contact Time (`t`) |
|:--------|:----------|:------------------|
| **Sphere** | `0.1 = -0.5·g·t² - t + 20` | `t = 1.91486 s` |
| **Plane**  | `0.2 = -0.5·g·t² + 15` | `t = 1.73704 s` |
| **Cube**   | `0.15 / √2 = -0.5·g·t² + 10` | `t = 1.42025 s` |

---

# Future Improvements

- Implement object bouncing upon ground contact.  
- Add energy conservation analysis before and after impact.  
- Visualize position–time and velocity–time curves.
