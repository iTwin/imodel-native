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

#include "Vec3.h"

/*---------------------------------------------------------------------------------**//**
*   Vec4 class and its inline methods.
+---------------+---------------+---------------+---------------+---------------+------*/
class Vec4 {
    private:
        double elt[4];

    protected:
        inline void copy(const Vec4& v);

    public:

        // Standard constructors
        Vec4(double x = 0, double y = 0, double z = 0, double w = 0) {
            elt[0] = x; elt[1] = y; elt[2] = z; elt[3] = w;
        }
        Vec4(const Vec3& v, double w) { elt[0] = v[0]; elt[1] = v[1]; elt[2] = v[2]; elt[3] = w; }
        Vec4(const Vec4& v) { copy(v); }
        Vec4(const double *v) { elt[0] = v[0]; elt[1] = v[1]; elt[2] = v[2]; elt[3] = v[3]; }

        // Access methods
        double& operator()(int i)       { return elt[i]; }
        double  operator()(int i) const { return elt[i]; }
        double& operator[](int i)             { return elt[i]; }
        const double& operator[](int i) const { return elt[i]; }

        double *raw()             { return elt; }
        const double *raw() const { return elt; }

        // Comparison methods
        inline bool operator==(const Vec4&) const;
        inline bool operator!=(const Vec4&) const;

        // Assignment and in-place arithmetic methods
        inline void set(double x, double y, double z, double w){
            elt[0] = x; elt[1] = y; elt[2] = z; elt[3] = w;
        }
        inline Vec4& operator=(const Vec4& v);
        inline Vec4& operator+=(const Vec4& v);
        inline Vec4& operator-=(const Vec4& v);
        inline Vec4& operator*=(double s);
        inline Vec4& operator/=(double s);

        // Binary arithmetic methods
        inline Vec4 operator+(const Vec4& v) const;
        inline Vec4 operator-(const Vec4& v) const;
        inline Vec4 operator-() const;
        inline Vec4 operator*(double s) const;
        inline Vec4 operator/(double s) const;
        inline double operator*(const Vec4& v) const;
};

inline void Vec4::copy(const Vec4& v)
{
    elt[0] = v.elt[0]; elt[1] = v.elt[1]; elt[2] = v.elt[2]; elt[3] = v.elt[3];
}

inline bool Vec4::operator==(const Vec4& v) const
{
    double dx = elt[X] - v[X], dy = elt[Y] - v[Y], dz = elt[Z] - v[Z], dw = elt[W] - v[W];
    return (dx*dx + dy*dy + dz*dz + dw*dw) < FEQ_EPS2;
}

inline bool Vec4::operator!=(const Vec4& v) const
{
    double dx = elt[X] - v[X], dy = elt[Y] - v[Y], dz = elt[Z] - v[Z], dw = elt[W] - v[W];
    return (dx*dx + dy*dy + dz*dz + dw*dw) > FEQ_EPS2;
}

inline Vec4& Vec4::operator=(const Vec4& v)
{
    copy(v);
    return *this;
}

inline Vec4& Vec4::operator+=(const Vec4& v)
{
    elt[0] += v[0];   elt[1] += v[1];   elt[2] += v[2];   elt[3] += v[3];
    return *this;
}

inline Vec4& Vec4::operator-=(const Vec4& v)
{
    elt[0] -= v[0];   elt[1] -= v[1];   elt[2] -= v[2];   elt[3] -= v[3];
    return *this;
}

inline Vec4& Vec4::operator*=(double s)
{
    elt[0] *= s;   elt[1] *= s;   elt[2] *= s;  elt[3] *= s;
    return *this;
}

inline Vec4& Vec4::operator/=(double s)
{
    elt[0] /= s;   elt[1] /= s;   elt[2] /= s;   elt[3] /= s;
    return *this;
}

inline Vec4 Vec4::operator+(const Vec4& v) const
{
    return Vec4(elt[0] + v[0], elt[1] + v[1], elt[2] + v[2], elt[3] + v[3]);
}

inline Vec4 Vec4::operator-(const Vec4& v) const
{
    return Vec4(elt[0] - v[0], elt[1] - v[1], elt[2] - v[2], elt[3] - v[3]);
}

inline Vec4 Vec4::operator-() const
{
    return Vec4(-elt[0], -elt[1], -elt[2], -elt[3]);
}

inline Vec4 Vec4::operator*(double s) const
{
    return Vec4(elt[0] * s, elt[1] * s, elt[2] * s, elt[3] * s);
}

inline Vec4 Vec4::operator/(double s) const
{
    return Vec4(elt[0] / s, elt[1] / s, elt[2] / s, elt[3] / s);
}

inline double Vec4::operator*(const Vec4& v) const
{
    return elt[0] * v[0] + elt[1] * v[1] + elt[2] * v[2] + elt[3] * v[3];
}

inline Vec4 operator*(double s, const Vec4& v) { return v*s; }

// Code adapted from VecLib4d.c in Graphics Gems V
inline Vec4 cross(const Vec4& a, const Vec4& b, const Vec4& c)
{
    Vec4 result;

    double d1 = (b[Z] * c[W]) - (b[W] * c[Z]);
    double d2 = (b[Y] * c[W]) - (b[W] * c[Y]);
    double d3 = (b[Y] * c[Z]) - (b[Z] * c[Y]);
    double d4 = (b[X] * c[W]) - (b[W] * c[X]);
    double d5 = (b[X] * c[Z]) - (b[Z] * c[X]);
    double d6 = (b[X] * c[Y]) - (b[Y] * c[X]);

    result[X] = -a[Y] * d1 + a[Z] * d2 - a[W] * d3;
    result[Y] = a[X] * d1 - a[Z] * d4 + a[W] * d5;
    result[Z] = -a[X] * d2 + a[Y] * d4 - a[W] * d6;
    result[W] = a[X] * d3 - a[Y] * d5 + a[Z] * d6;

    return result;
}

inline double norm(const Vec4& v)
{
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
}

inline double norm2(const Vec4& v)
{
    return v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];
}

inline double length(const Vec4& v) { return norm(v); }

inline double unitize(Vec4& v)
{
    double l = norm2(v);
    if (l != 1.0 && l != 0.0)
    {
        l = sqrt(l);
        v /= l;
    }
    return l;
}