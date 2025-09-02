#pragma once
#include "mathematics/vector.hpp"
#include "precision.hpp"

struct Object
{
private:
    Vector3D position;
    Vector3D rotation;
    Vector3D scale;
    decimal  mass;

public:
    // ============================================================================
    // ============================================================================
    //  Constructors
    // ============================================================================
    // ============================================================================
    Object() = default;
    Object(const Vector3D& position = Vector3D(), const Vector3D& rotation = Vector3D(),
           const Vector3D& scale = Vector3D(), decimal mass = 0)
        : position { position }
        , rotation { rotation }
        , scale { scale }
        , mass { mass }
    {}

    // ============================================================================
    // ============================================================================
    //  Getters
    // ============================================================================
    // ============================================================================
    Vector3D get_position() const { return position; }
    Vector3D get_rotation() const { return rotation; }
    Vector3D get_scale() const { return scale; }

    // ============================================================================
    // ============================================================================
    //  Transformations
    // ============================================================================
    // ============================================================================
    void translation(const Vector3D& v_translation) { position += v_translation; }
    void rotation(const Vector3D& v_rotation) { rotation += v_rotation; }
    void scaling(const Vector3D& v_scaling) { scale += v_scaling; }
};
