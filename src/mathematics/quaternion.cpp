Quaternion::Quaternion()
    : w(1)
    , x(0)
    , y(0)
    , z(0)
{}

Quaternion::Quaternion(decimal w, decimal x, decimal y, decimal z)
    : w(w)
    , x(x)
    , y(y)
    , z(z)
{}

Quaternion::Quaternion(const Vector3D& axis, decimal angleRad)
{
    decimal halfAngle = angleRad / 2.0;
    decimal s         = std::sin(halfAngle);
    w                 = std::cos(halfAngle);
    x                 = axis.x * s;
    y                 = axis.y * s;
    z                 = axis.z * s;
}

// ===== Basic Operations =====
Quaternion Quaternion::conjugate() const { return Quaternion(w, -x, -y, -z); }

decimal Quaternion::norm() const { return std::sqrt(w * w + x * x + y * y + z * z); }

Quaternion Quaternion::normalized() const
{
    decimal n = norm();
    if (n < PRECISION_MACHINE)
        throw std::runtime_error("Cannot normalize zero quaternion");
    return Quaternion(w / n, x / n, y / n, z / n);
}

Quaternion Quaternion::inverse() const
{
    decimal n2 = w * w + x * x + y * y + z * z;
    if (n2 < PRECISION_MACHINE)
        throw std::runtime_error("Cannot invert zero quaternion");
    return conjugate() / n2;
}

// ===== Operators =====
Quaternion Quaternion::operator+(const Quaternion& q) const
{
    return Quaternion(w + q.w, x + q.x, y + q.y, z + q.z);
}

Quaternion Quaternion::operator-(const Quaternion& q) const
{
    return Quaternion(w - q.w, x - q.x, y - q.y, z - q.z);
}

Quaternion Quaternion::operator*(const Quaternion& q) const
{
    return Quaternion(w * q.w - x * q.x - y * q.y - z * q.z, w * q.x + x * q.w + y * q.z - z * q.y,
                      w * q.y - x * q.z + y * q.w + z * q.x, w * q.z + x * q.y - y * q.x + z * q.w);
}

Quaternion Quaternion::operator*(decimal scalar) const
{
    return Quaternion(w * scalar, x * scalar, y * scalar, z * scalar);
}

Quaternion Quaternion::operator/(decimal scalar) const
{
    if (std::fabs(scalar) < PRECISION_MACHINE)
        throw std::runtime_error("Division by zero");
    return Quaternion(w / scalar, x / scalar, y / scalar, z / scalar);
}

Quaternion& Quaternion::operator+=(const Quaternion& q)
{
    w += q.w;
    x += q.x;
    y += q.y;
    z += q.z;
    return *this;
}

Quaternion& Quaternion::operator-=(const Quaternion& q)
{
    w -= q.w;
    x -= q.x;
    y -= q.y;
    z -= q.z;
    return *this;
}

Quaternion& Quaternion::operator*=(const Quaternion& q)
{
    *this = *this * q;
    return *this;
}

Quaternion& Quaternion::operator*=(decimal scalar)
{
    w *= scalar;
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Quaternion& Quaternion::operator/=(decimal scalar)
{
    if (std::fabs(scalar) < PRECISION_MACHINE)
        throw std::runtime_error("Division by zero");
    w /= scalar;
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

bool Quaternion::operator==(const Quaternion& q) const
{
    return w == q.w && x == q.x && y == q.y && z == q.z;
}

bool Quaternion::operator!=(const Quaternion& q) const { return !(*this == q); }

bool Quaternion::approxEqual(const Quaternion& q, decimal epsilon) const
{
    return std::fabs(w - q.w) < epsilon && std::fabs(x - q.x) < epsilon && std::fabs(y - q.y) < epsilon &&
           std::fabs(z - q.z) < epsilon;
}

// ===== Vector Rotation =====
Vector3D Quaternion::rotate(const Vector3D& v) const
{
    Quaternion qv(0, v.x, v.y, v.z);
    Quaternion res = (*this) * qv * inverse();
    return Vector3D(res.x, res.y, res.z);
}

// ===== Conversions =====
Matrix3x3 Quaternion::toMatrix3x3() const
{
    decimal xx = x * x, yy = y * y, zz = z * z;
    decimal xy = x * y, xz = x * z, yz = y * z;
    decimal wx = w * x, wy = w * y, wz = w * z;

    return Matrix3x3(1 - 2 * (yy + zz), 2 * (xy - wz), 2 * (xz + wy), 2 * (xy + wz), 1 - 2 * (xx + zz),
                     2 * (yz - wx), 2 * (xz - wy), 2 * (yz + wx), 1 - 2 * (xx + yy));
}

Quaternion Quaternion::fromMatrix3x3(const Matrix3x3& m)
{
    decimal trace = m(0, 0) + m(1, 1) + m(2, 2);
    if (trace > 0)
    {
        decimal s = std::sqrt(trace + 1.0) * 2;
        return Quaternion(0.25 * s, (m(2, 1) - m(1, 2)) / s, (m(0, 2) - m(2, 0)) / s,
                          (m(1, 0) - m(0, 1)) / s);
    }
    else if ((m(0, 0) > m(1, 1)) && (m(0, 0) > m(2, 2)))
    {
        decimal s = std::sqrt(1.0 + m(0, 0) - m(1, 1) - m(2, 2)) * 2;
        return Quaternion((m(2, 1) - m(1, 2)) / s, 0.25 * s, (m(0, 1) + m(1, 0)) / s,
                          (m(0, 2) + m(2, 0)) / s);
    }
    else if (m(1, 1) > m(2, 2))
    {
        decimal s = std::sqrt(1.0 + m(1, 1) - m(0, 0) - m(2, 2)) * 2;
        return Quaternion((m(0, 2) - m(2, 0)) / s, (m(0, 1) + m(1, 0)) / s, 0.25 * s,
                          (m(1, 2) + m(2, 1)) / s);
    }
    else
    {
        decimal s = std::sqrt(1.0 + m(2, 2) - m(0, 0) - m(1, 1)) * 2;
        return Quaternion((m(1, 0) - m(0, 1)) / s, (m(0, 2) + m(2, 0)) / s, (m(1, 2) + m(2, 1)) / s,
                          0.25 * s);
    }
}

// ===== Interpolation =====
Quaternion Quaternion::slerp(const Quaternion& q1, const Quaternion& q2, decimal t)
{
    Quaternion q2Copy = q2;
    decimal    dot    = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
    if (dot < 0.0f)
    {
        q2Copy = q2Copy * -1;
        dot    = -dot;
    }

    if (dot > 0.9995f)
    {
        Quaternion result = q1 + t * (q2Copy - q1);
        return result.normalized();
    }

    decimal theta_0 = std::acos(dot);
    decimal theta   = theta_0 * t;

    Quaternion q3 = (q2Copy - q1 * dot).normalized();

    return q1 * std::cos(theta) + q3 * std::sin(theta);
}

// ===== Stream Output =====
std::ostream& operator<<(std::ostream& os, const Quaternion& q)
{
    os << "(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ")";
    return os;
}
