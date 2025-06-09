/**
 * @brief Represents a 3-dimensional vector with x, y, and z components
 *
 * This class provides a comprehensive implementation of a 3D vector with
 * various mathematical operations, including construction, access, mutation,
 * comparison, and vector arithmetic.
 *
 * @note Uses decimal type for component storage, allowing for flexible precision
 */
#pragma once

#include "config.hpp"

class Vector3D
{
private:
    decimal x;
    decimal y;
    decimal z;

public:
    // ===== Constructors =====
    // Default constructor (zero vector)
    Vector3D();
    // Construct from component values
    Vector3D(decimal newx, decimal newy, decimal newz);

    // ===== Accessors =====
    // Get the value of each component
    decimal getX() const;
    decimal getY() const;
    decimal getZ() const;

    // Get the Euclidean norm (length) of the vector
    decimal getNorm() const;
    // Get the squared norm (avoids sqrt for efficiency)
    decimal getNormSquare() const;
    // Get the minimum component value
    decimal getMinValue() const;
    // Get the maximum component value
    decimal getMaxValue() const;
    // Get a vector with the absolute values of each component
    Vector3D getAbsoluteVector() const;
    // Get normalized vector
    Vector3D getNormalizedVector() const;

    // ===== Mutators =====
    // Set the value of each component
    void setX(decimal x);
    void setY(decimal y);
    void setZ(decimal z);
    // Set all components to zero
    void setToZero();
    // Set all components to the given values
    void setAllValues(decimal newx, decimal newy, decimal newz);
    // Normalize the vector (make it unit length, does nothing if zero)
    void normalize();

    // ===== Property Checks =====
    // Returns true if the vector is exactly zero
    bool isZero();
    // Returns true if the vector is a unit vector (length == 1)
    bool isUnit();
    // Returns true if the squared length is approximately equal to a value
    bool isLengthEqual(decimal value);
    // Returns true if all components are finite numbers
    bool isFinite();

    // ===== Vector Operations =====
    // Compute the dot product with another vector
    decimal dotProduct(const Vector3D& vector) const;
    // Compute the cross product with another vector
    Vector3D crossProduct(const Vector3D& vector) const;

    // ===== Comparison Operators =====
    // Compare vectors for equality/inequality and lexicographical order
    bool operator==(const Vector3D& vector) const;
    bool operator!=(const Vector3D& vector) const;
    bool operator<(const Vector3D& vector) const;
    bool approxEqual(const Vector3D& vector, decimal precision = PRECISION_MACHINE) const;

    // ===== Element Access =====
    // Access components by index: 0=x, 1=y, 2=z
    decimal& operator[](int index);
    decimal  operator[](int index) const;

    // ===== Compound Assignment Operators =====
    // Perform arithmetic operations and assign the result
    Vector3D& operator+=(const Vector3D& vector);
    Vector3D& operator-=(const Vector3D& vector);
    Vector3D& operator*=(const Vector3D& vector);
    Vector3D& operator/=(const Vector3D& vector);

    // ===== Min/Max =====
    // Return the component-wise minimum/maximum of two vectors
    Vector3D min(const Vector3D& vector1, const Vector3D& vector2) const;
    Vector3D max(const Vector3D& vector1, const Vector3D& vector2) const;

    // ===== Non-member Arithmetic Operators =====
    // Standard arithmetic operators for vector math
    friend Vector3D operator+(const Vector3D& vector, const decimal& value);
    friend Vector3D operator+(const decimal& value, const Vector3D& vector);
    friend Vector3D operator+(const Vector3D& lhs, const Vector3D& rhs);
    friend Vector3D operator-(const Vector3D& vector);
    friend Vector3D operator-(const Vector3D& lhs, const Vector3D& rhs);
    friend Vector3D operator*(const decimal& value, const Vector3D& vector);
    friend Vector3D operator*(const Vector3D& vector, const decimal& value);
    friend Vector3D operator*(const Vector3D& lhs, const Vector3D& rhs);
    friend Vector3D operator/(const Vector3D& vector, const decimal& value);
    friend Vector3D operator/(const Vector3D& lhs, const Vector3D& rhs);
};
