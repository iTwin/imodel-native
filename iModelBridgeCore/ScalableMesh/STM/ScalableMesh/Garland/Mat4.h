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

#include "Vec4.h"

class Mat4
{
    private:
        Vec4 row[4];

    protected:

        inline void copy(const Mat4& m);
        inline Vec4 col(int i) const
        {
            return Vec4(row[0][i], row[1][i], row[2][i], row[3][i]);
        }

    public:
        // Standard matrices
        static Mat4 identity;
        static Mat4 zero;
        static Mat4 unit;

        static Mat4 trans(double, double, double);
        static Mat4 scale(double, double, double);
        static Mat4 xrot(double); //
        static Mat4 yrot(double); // Arguments are in radians
        static Mat4 zrot(double); //

        // Standard constructors
        Mat4() { copy(zero); }
        Mat4(const Vec4& r0, const Vec4& r1, const Vec4& r2, const Vec4& r3)
        {
            row[0] = r0; row[1] = r1; row[2] = r2; row[3] = r3;
        }
        Mat4(const Mat4& m) { copy(m); }

        // Access methods
        // M(i, j) == row i;col j
        double& operator()(int i, int j)       { return row[i][j]; }
        double  operator()(int i, int j) const { return row[i][j]; }
        const Vec4& operator[](int i) const { return row[i]; }

        // Comparison methods
        inline int operator==(const Mat4&);

        // Assignment methods
        inline Mat4& operator=(const Mat4& m) { copy(m); return *this; }
        inline Mat4& operator+=(const Mat4& m);
        inline Mat4& operator-=(const Mat4& m);

        inline Mat4& operator*=(double s);
        inline Mat4& operator/=(double s);

        // Arithmetic methods
        inline Mat4 operator+(const Mat4& m) const;
        inline Mat4 operator-(const Mat4& m) const;
        inline Mat4 operator-() const;

        inline Mat4 operator*(double s) const;
        inline Mat4 operator/(double s) const;
        Mat4 operator*(const Mat4& m) const;

        inline Vec4 operator*(const Vec4& v) const; // [x y z w]
        inline Vec3 operator*(const Vec3& v) const; // [x y z w]

        // Matrix operations
        double det() const;
        Mat4 transpose() const;
        Mat4 adjoint() const;
        double inverse(Mat4&) const;
        double cramerInverse(Mat4&) const;
};

inline void Mat4::copy(const Mat4& m)
{
    row[0] = m.row[0]; row[1] = m.row[1];
    row[2] = m.row[2]; row[3] = m.row[3];
}

inline int Mat4::operator==(const Mat4& m)
{
    return row[0] == m.row[0] &&
    row[1] == m.row[1] &&
    row[2] == m.row[2] &&
    row[3] == m.row[3];
}

inline Mat4& Mat4::operator+=(const Mat4& m)
{
    row[0] += m.row[0]; row[1] += m.row[1];
    row[2] += m.row[2]; row[3] += m.row[3];
    return *this;
}

inline Mat4& Mat4::operator-=(const Mat4& m)
{
    row[0] -= m.row[0]; row[1] -= m.row[1];
    row[2] -= m.row[2]; row[3] -= m.row[3];
    return *this;
}

inline Mat4& Mat4::operator*=(double s)
{
    row[0] *= s; row[1] *= s; row[2] *= s; row[3] *= s;
    return *this;
}

inline Mat4& Mat4::operator/=(double s)
{
    row[0] /= s; row[1] /= s; row[2] /= s; row[3] /= s;
    return *this;
}

inline Mat4 Mat4::operator+(const Mat4& m) const
{
    return Mat4(row[0] + m.row[0],
    row[1] + m.row[1],
    row[2] + m.row[2],
    row[3] + m.row[3]);
}

inline Mat4 Mat4::operator-(const Mat4& m) const
{
    return Mat4(row[0] - m.row[0],
    row[1] - m.row[1],
    row[2] - m.row[2],
    row[3] - m.row[3]);
}

inline Mat4 Mat4::operator-() const
{
    return Mat4(-row[0], -row[1], -row[2], -row[3]);
}

inline Mat4 Mat4::operator*(double s) const
{
    return Mat4(row[0] * s, row[1] * s, row[2] * s, row[3] * s);
}

inline Mat4 Mat4::operator/(double s) const
{
    return Mat4(row[0] / s, row[1] / s, row[2] / s, row[3] / s);
}

inline Vec4 Mat4::operator*(const Vec4& v) const
{
    return Vec4(row[0] * v, row[1] * v, row[2] * v, row[3] * v);
}

// Transform a homogeneous 3-vector and reproject into normal 3-space
inline Vec3 Mat4::operator*(const Vec3& v) const
{
    Vec4 u = Vec4(v, 1);
    double w = row[3] * u;

    if (w == 0.0)
        return Vec3(row[0] * u, row[1] * u, row[2] * u);
    else
        return Vec3(row[0] * u / w, row[1] * u / w, row[2] * u / w);
}

