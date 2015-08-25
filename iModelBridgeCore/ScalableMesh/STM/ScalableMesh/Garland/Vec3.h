/*--------------------------------------------------------------------------------------+
|
|   Code taken from Michael Garland's demo application called "QSlim" (version 1.0)
|   which intends to demonstrate an algorithm of mesh simplification based on
|   Garland and Heckbert(1997) "Surface Simplification Using Quadric Error Metrics".
|   The code of the demo is said to be in the public domain.
|   See: http://www.cs.cmu.edu/afs/cs/Web/People/garland/quadrics/qslim10.html
|
|   $Revision: 1.0 $
|       $Date: 2014/09/17 $
|     $Author: Christian.Cote $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "Vec2.h"

/*---------------------------------------------------------------------------------**//**
*   Vec3 class and its inline methods.
+---------------+---------------+---------------+---------------+---------------+------*/
class Vec3 {
    private:
        double elt[3];

    protected:
        inline void copy(const Vec3& v);

    public:

        // Standard constructors
        Vec3(double x = 0, double y = 0, double z = 0) { elt[0] = x; elt[1] = y; elt[2] = z; }
        Vec3(const Vec2& v, double z) { elt[0] = v[0]; elt[1] = v[1]; elt[2] = z; }
        Vec3(const Vec3& v) { copy(v); }
        Vec3(const double *v) { elt[0] = v[0]; elt[1] = v[1]; elt[2] = v[2]; }

        // Access methods
        double& operator()(int i)       { return elt[i]; }
        double  operator()(int i) const { return elt[i]; }
        double& operator[](int i)       { return elt[i]; }
        double  operator[](int i) const { return elt[i]; }
        double *raw()             { return elt; }
        const double *raw() const { return elt; }

        // Comparison operators
        inline bool operator==(const Vec3& v) const;
        inline bool operator!=(const Vec3& v) const;

        // Assignment and in-place arithmetic methods
        inline void set(double x, double y, double z) { elt[0] = x; elt[1] = y; elt[2] = z; }
        inline Vec3& operator=(const Vec3& v);
        inline Vec3& operator+=(const Vec3& v);
        inline Vec3& operator-=(const Vec3& v);
        inline Vec3& operator*=(double s);
        inline Vec3& operator/=(double s);

        // Binary arithmetic methods
        inline Vec3 operator+(const Vec3& v) const;
        inline Vec3 operator-(const Vec3& v) const;
        inline Vec3 operator-() const;

        inline Vec3 operator*(double s) const;
        inline Vec3 operator/(double s) const;
        inline double operator*(const Vec3& v) const;
        inline Vec3 operator^(const Vec3& v) const;
};

inline void Vec3::copy(const Vec3& v)
{
    elt[0] = v.elt[0]; elt[1] = v.elt[1]; elt[2] = v.elt[2];
}

inline bool Vec3::operator==(const Vec3& v) const
{
    double dx = elt[X] - v[X], dy = elt[Y] - v[Y], dz = elt[Z] - v[Z];
    return (dx*dx + dy*dy + dz*dz) < FEQ_EPS2;
}

inline bool Vec3::operator!=(const Vec3& v) const
{
    double dx = elt[X] - v[X], dy = elt[Y] - v[Y], dz = elt[Z] - v[Z];
    return (dx*dx + dy*dy + dz*dz) > FEQ_EPS2;
}

inline Vec3& Vec3::operator=(const Vec3& v)
{
    copy(v);
    return *this;
}

inline Vec3& Vec3::operator+=(const Vec3& v)
{
    elt[0] += v[0];   elt[1] += v[1];   elt[2] += v[2];
    return *this;
}

inline Vec3& Vec3::operator-=(const Vec3& v)
{
    elt[0] -= v[0];   elt[1] -= v[1];   elt[2] -= v[2];
    return *this;
}

inline Vec3& Vec3::operator*=(double s)
{
    elt[0] *= s;   elt[1] *= s;   elt[2] *= s;
    return *this;
}

inline Vec3& Vec3::operator/=(double s)
{
    elt[0] /= s;   elt[1] /= s;   elt[2] /= s;
    return *this;
}

inline Vec3 Vec3::operator+(const Vec3& v) const
{
    return Vec3(elt[0] + v[0], elt[1] + v[1], elt[2] + v[2]);
}

inline Vec3 Vec3::operator-(const Vec3& v) const
{
    return Vec3(elt[0] - v[0], elt[1] - v[1], elt[2] - v[2]);
}

inline Vec3 Vec3::operator-() const
{
    return Vec3(-elt[0], -elt[1], -elt[2]);
}

inline Vec3 Vec3::operator*(double s) const
{
    return Vec3(elt[0] * s, elt[1] * s, elt[2] * s);
}

inline Vec3 Vec3::operator/(double s) const
{
    return Vec3(elt[0] / s, elt[1] / s, elt[2] / s);
}

inline double Vec3::operator*(const Vec3& v) const
{
    return elt[0] * v[0] + elt[1] * v[1] + elt[2] * v[2];
}

inline Vec3 Vec3::operator^(const Vec3& v) const
{
    Vec3 w(elt[1] * v[2] - v[1] * elt[2], -elt[0] * v[2] + v[0] * elt[2], elt[0] * v[1] - v[0] * elt[1]);
    return w;
}

inline Vec3 operator*(double s, const Vec3& v) { return v*s; }

inline double norm(const Vec3& v)
{
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

inline double norm2(const Vec3& v)
{
    return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

inline double length(const Vec3& v) { return norm(v); }

inline double unitize(Vec3& v)
{
    double l = norm2(v);
    if (l != 1.0 && l != 0.0)
    {
        l = sqrt(l);
        v /= l;
    }
    return l;
}