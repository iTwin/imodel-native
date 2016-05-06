/**
 @file g3dmath.h
 
 Math util class.
 
 @maintainer Morgan McGuire, matrix@graphics3d.com
 @cite Portions by Laura Wollstadt 
 @cite Portions based on Dave Eberly's Magic Software Library
        at <A HREF="http://www.magic-software.com">http://www.magic-software.com</A>
 @cite highestBit by Jukka Liimatta
 
 @created 2001-06-02
 @edited  2004-07-25

 Copyright 2000-2003, Morgan McGuire.
 All rights reserved.
 */

#ifndef PTMATH_H
#define PTMATH_H

// Prevent MSVC from defining min and max macros
#define NOMINMAX

#include <ctype.h>
#include <float.h>
#include <limits>
#ifdef __INTEL_COMPILER
#include <mathimf.h>
#else
#include <math.h>
#endif
#include <pt/debug.h>

#undef min
#undef max

namespace pt {

const double fuzzyEpsilon = 0.000001;

inline const double& inf() {
	static const double i = 1.0/sin(0.0);
	return i;
}

inline const double& nan() {
	static const double n = 0.0/sin(0.0);
	return n;
}

#define G3D_PI      (3.1415926535898)
#define G3D_HALF_PI (1.5707963267949)
#define G3D_TWO_PI  (6.283185)

typedef signed char		int8;
typedef unsigned char	uint8;
typedef short			int16;
typedef unsigned short	uint16;
typedef int				int32;
typedef unsigned int	uint32;

typedef float			float32;
typedef double			float64;

int iAbs(int iValue);
int iCeil(double fValue);

/**
 Clamps the value to the range [low, hi] (inclusive)
 */
int iClamp(int val, int low, int hi);
double clamp(double val, double low, double hi);

/**
 Returns a + (b - a) * f;
 */
inline double lerp(double a, double b, double f) {
    return a + (b - a) * f;
}

/**
 Wraps the value to the range [0, hi) (exclusive
 on the high end).  This is like the clock arithmetic
 produced by % (modulo) except the result is guaranteed
 to be positive.
 */
int iWrap(int val, int hi);

int iFloor(double fValue);
int iSign(int iValue);
int iSign(double fValue);
int iRound(double fValue);

/**
 Returns a random number uniformly at random between low and hi
 (inclusive).
 */
int iRandom(int low, int hi);

double abs (double fValue);
double aCos (double fValue);
double aSin (double fValue);
double aTan (double fValue);
double aTan2 (double fY, double fX);
double sign (double fValue);
double square (double fValue);

/**
 Returns true if the argument is a finite real number.
 */
bool isFinite(double x);

/**
 Returns true if the argument is NaN (not a number).
 You can't use x == nan to test this because all
 comparisons against nan return false.
 */
bool isNaN(double x);

/**
 Computes x % 3.
 */
int iMod3(int x);

/** [0, 1] */
double unitRandom ();

/**
 Uniform random number between low and hi, inclusive.
 */
double random(double low, double hi);

/** [-1, 1] */
double symmetricRandom ();
double min(double x, double y);
double max(double x, double y);
int iMin(int x, int y);
int iMax(int x, int y);

double square(double x);
double sumSquares(double x, double y);
double sumSquares(double x, double y, double z);
double distance(double x, double y);
double distance(double x, double y, double z);

/**
  Returnes the 0-based index of the highest 1 bit from
  the left.  -1 means the number was 0.

  @cite Based on code by jukka@liimatta.org
 */ 
int highestBit(uint32 x);

/**
 Note that fuzzyEq(a, b) && fuzzyEq(b, c) does not imply
 fuzzyEq(a, c), although that will be the case on some
 occasions.
 */
bool fuzzyEq(double a, double b);

/** True if a is definitely not equal to b.  
    Guaranteed false if a == b. 
    Possibly false when a != b.*/
bool fuzzyNe(double a, double b);

/** Is a strictly greater than b? (Guaranteed false if a <= b).
    (Possibly false if a > b) */
bool fuzzyGt(double a, double b);

/** Is a near or greater than b? */
bool fuzzyGe(double a, double b);

/** Is a strictly less than b? (Guaranteed false if a >= b)*/
bool fuzzyLt(double a, double b);

/** Is a near or less than b? */
bool fuzzyLe(double a, double b);

/**
 Computes 1 / sqrt(x).
 */
inline float rsq(float x) {
    return 1.0f / sqrt(x);
}

/**
 Uses SSE to implement rsq.
 @cite Nick nicolas@capens.net
 */
inline float SSErsq(float x) {

    #ifdef SSE
        __asm {
           movss xmm0, x
           rsqrtss xmm0, xmm0
           movss x, xmm0
        }
        return x;
    #else
        return 1.0f / sqrt(x);
    #endif
}

/**
 Return the next power of 2 higher than the input
 If the input is already a power of 2, the output will be the same 
 as the input.
 */
int ceilPow2(unsigned int in);

/**
 * True if num is a power of two.
 */
bool isPow2(int num);

bool isOdd(int num);
bool isEven(int num);

double toRadians(double deg);
double toDegrees(double rad);

/**
 Returns true if x is not exactly equal to 0.0f.
 */
inline bool any(float x) {
    return x != 0;
}

/**
 Returns true if x is not exactly equal to 0.0f.
 */
inline bool all(float x) {
    return x != 0;
}

/**
 v / v (for DirectX/Cg support)
 */
inline float normalize(float v) {
    return v / v;
}

/**
 a * b (for DirectX/Cg support)
 */
inline float dot(float a, float b) {
    return a * b;
}


/**
 a * b (for DirectX/Cg support)
 */
inline float mul(float a, float b) {
    return a * b;
}

/**
 2^x
 */
inline double exp2(double x) {
    return pow(2.0, x);
}

inline double rsqrt(double x) {
    return 1.0 / sqrt(x);
}


/**
 sin(x)/x
 */
inline double sinc(double x) {
    double r = sin(x) / x;

    if (isNaN(r)) {
        return 1.0;
    } else {
        return r;
    }
}

/**
 Computes a floating point modulo; the result is t wrapped to the range [lo, hi).
 */
inline double wrap(double t, double lo, double hi) {
    if ((t >= lo) && (t < hi)) {
        return t;
    }

    //debugAssert(hi > lo);

    double interval = hi - lo;

    return t - interval * iFloor((t - lo) / interval);

}

inline double wrap(double t, double hi) {
    return wrap(t, 0, hi);
}


} // namespace


#include "ptmath.inl"

#endif

