/**
 * @file csv.hpp
 * @brief Data structure for buffered motion CSV output.
 */
#include "mathematics/vector.hpp"
#include "objects/object.hpp"

/// One snapshot of an object's kinematic state, buffered before being written to CSV.
struct MotionCSV
{
    decimal  time; /// Simulation time at which the snapshot was taken (s).
    Object*  obj;  /// Pointer to the object being recorded.
    Vector3D pos;  /// Object position at `time`.
    Vector3D vel;  /// Object velocity at `time`.
    Vector3D acc;  /// Object acceleration at `time`.
};