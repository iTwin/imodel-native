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

#include "TypesAndDefinitions.h"

/*---------------------------------------------------------------------------------**//**
*   Vec2 class and its inline methods.
+---------------+---------------+---------------+---------------+---------------+------*/
class Vec2 {
    private:
        double elt[2];

    protected:
        inline void copy(const Vec2& v);

    public:

        // Standard constructors
        Vec2(double x = 0, double y = 0) { elt[0] = x; elt[1] = y; }
        Vec2(const Vec2& v) { copy(v); }
        Vec2(const double *v) { elt[0] = v[0]; elt[1] = v[1]; }

        // Access methods
        double& operator()(int i)       { return elt[i]; }
        double  operator()(int i) const { return elt[i]; }
        double& operator[](int i)       { return elt[i]; }
        double  operator[](int i) const { return elt[i]; }

        double *raw()             { return elt; }
        const double *raw() const { return elt; }

        // Comparison operators
        inline bool operator==(const Vec2& v) const;
        inline bool operator!=(const Vec2& v) const;

        // Assignment and in-place arithmetic methods
        inline void set(double x, double y) { elt[0] = x; elt[1] = y; }
        inline Vec2& operator=(const Vec2& v);
        inline Vec2& operator+=(const Vec2& v);
        inline Vec2& operator-=(const Vec2& v);
        inline Vec2& operator*=(double s);
        inline Vec2& operator/=(double s);

        // Binary arithmetic methods
        inline Vec2 operator+(const Vec2& v) const;
        inline Vec2 operator-(const Vec2& v) const;
        inline Vec2 operator-() const;

        inline Vec2 operator*(double s) const;
        inline Vec2 operator/(double s) const;
        inline double operator*(const Vec2& v) const;
};

inline void Vec2::copy(const Vec2& v)
{
    elt[0] = v.elt[0]; elt[1] = v.elt[1];
}

inline bool Vec2::operator==(const Vec2& v) const
{
    double dx = elt[X] - v[X], dy = elt[Y] - v[Y];
    return (dx*dx + dy*dy) < FEQ_EPS2;
}

inline bool Vec2::operator!=(const Vec2& v) const
{
    double dx = elt[X] - v[X], dy = elt[Y] - v[Y];
    return (dx*dx + dy*dy) > FEQ_EPS2;
}

inline Vec2& Vec2::operator=(const Vec2& v)
{
    copy(v);
    return *this;
}

inline Vec2& Vec2::operator+=(const Vec2& v)
{
    elt[0] += v[0];   elt[1] += v[1];
    return *this;
}

inline Vec2& Vec2::operator-=(const Vec2& v)
{
    elt[0] -= v[0];   elt[1] -= v[1];
    return *this;
}

inline Vec2& Vec2::operator*=(double s)
{
    elt[0] *= s;   elt[1] *= s;
    return *this;
}

inline Vec2& Vec2::operator/=(double s)
{
    elt[0] /= s;   elt[1] /= s;
    return *this;
}

inline Vec2 Vec2::operator+(const Vec2& v) const
{
    return Vec2(elt[0] + v[0], elt[1] + v[1]);
}

inline Vec2 Vec2::operator-(const Vec2& v) const
{
    return Vec2(elt[0] - v[0], elt[1] - v[1]);
}

inline Vec2 Vec2::operator-() const
{
    return Vec2(-elt[0], -elt[1]);
}

inline Vec2 Vec2::operator*(double s) const
{
    return Vec2(elt[0] * s, elt[1] * s);
}

inline Vec2 Vec2::operator/(double s) const
{
    return Vec2(elt[0] / s, elt[1] / s);
}

inline double Vec2::operator*(const Vec2& v) const
{
    return elt[0] * v[0] + elt[1] * v[1];
}

inline Vec2 operator*(double s, const Vec2& v) { return v*s; }

inline double norm(const Vec2& v) { return sqrt(v[0] * v[0] + v[1] * v[1]); }
inline double norm2(const Vec2& v) { return v[0] * v[0] + v[1] * v[1]; }
inline double length(const Vec2& v) { return norm(v); }

inline double unitize(Vec2& v)
{
    double l = norm2(v);
    if (l != 1.0 && l != 0.0)
    {
        l = sqrt(l);
        v /= l;
    }
    return l;
}