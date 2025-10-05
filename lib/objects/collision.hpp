#pragma once

// Forward declarations
struct Sphere;
struct AABB;
struct Plane;

bool sphereCollision(const Sphere& sphere1, const Sphere& sphere2);
bool sphereAABBCollision(const Sphere& sphere, const AABB& aabb);
bool spherePlaneCollision(const Sphere& sphere, const Plane& plane);

bool aabbCollision(const AABB& aabb1, const AABB& aabb2);
bool aabbSphereCollision(const AABB& aabb, const Sphere& sphere);
bool aabbPlaneCollision(const AABB& aabb, const Plane& plane);

bool planeCollision(const Plane& plane1, const Plane& plane2);
bool planeSphereCollision(const Plane& plane, const Sphere& sphere);
bool planeAABBCollision(const Plane& plane, const AABB& aabb);
