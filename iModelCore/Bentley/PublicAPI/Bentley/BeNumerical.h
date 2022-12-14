/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include "Bentley.h"
#include <math.h>
#include <float.h>
#include <algorithm>

BEGIN_BENTLEY_NAMESPACE
/*=================================================================================**//**
* Numerical utilities
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct          BeNumerical
{
#if defined (_WIN32) // Windows && WinRT
    static double   BeNextafter (double x, double y)    {return _nextafter(x,y);}
    static float   BeNextafterf (float x, float y)    {return _nextafterf(x,y);}
    static int      BeIsnan (double v)                  {return _isnan(v);}
    static int      BeFinite (double v)                 {return _finite(v);}
#else
    static double   BeNextafter (double x, double y)    {return nextafter(x,y);}
    static float   BeNextafterf (float x, float y)    {return nextafterf(x,y);}
    static int      BeIsnan (double v)                  {return isnan (v);}
    static int      BeFinite (double v)                 {return isfinite(v);}
#endif

    //! A platform-specific function that clears any pending floating point exceptions.
    BENTLEYDLL_EXPORT static uint32_t ResetFloatingPointExceptions (uint32_t newFpuMask);

    //! Get the smallest value that, when added to fabs(sv), yields a number that is not equal to fabs(sv).
    //! This can be used to compute a comparison tolerance between numbers that are about as large as sv.
    static double NextafterDelta (double sv)
        {
        double v = fabs (sv);
        return BeNextafter (v, DBL_MAX) - v;
        }

    //! Compute the tolerance that should be used to check if these two numbers are not equal.
    //! @remarks Do not use the difference between the numbers as a basis for computing the comparison tolerance!
    //! @param[in] sv1  a value
    //! @param[in] sv2  another value
    static double ComputeComparisonTolerance (double sv1, double sv2)
        {
        double uv = std::max<double> (fabs(sv1), fabs(sv2));
        return (uv < 1.0)? DBL_EPSILON: NextafterDelta(uv);
        }

    //! Compare two values to the closest tolerance possible.
    //! @param[in] sv1  a value
    //! @param[in] sv2  another value
    //! @return 0 if equal within the closest possible tolerance, -1 if sv1 is less than sv2, and 1 if sv1 is greater than sv2
    static int Compare (double sv1, double sv2)
        {
        if (fabs(sv2 - sv1) < ComputeComparisonTolerance(sv1, sv2))
             return 0;
        return sv1 < sv2 ? -1 : 1;
        }

    //! Checks if value1 is greater than value2 to the closest tolerance possible.
    static bool IsGreater (double value1, double value2) { return (Compare(value1, value2) == 1); }

    //! Checks if value1 is greater or equal to value2 to the closest tolerance possible.
    static bool IsGreaterOrEqual (double value1, double value2) { return (Compare(value1, value2) >= 0); }

    //! Checks if value1 is less than value2 to the closest tolerance possible.
    static bool IsLess (double value1, double value2) { return (Compare(value1, value2) == -1); }

    //! Checks if value1 is less or equal to value2 to the closest tolerance possible.
    static bool IsLessOrEqual (double value1, double value2) { return (Compare(value1, value2) <= 0); }

    //! Checks if given values are equal to the closest tolerance possible.
    static bool IsEqual (double value1, double value2) { return (Compare(value1, value2) == 0); }

    //! Checks if value is equal to zero to the closest tolerance possible.
    static bool IsEqualToZero (double value) { return IsEqual(value, 0.0); }

    //! Checks if value is greater than zero to the closest tolerance possible.
    static bool IsGreaterThanZero (double value) { return IsGreater (value, 0.0); }

    //! Checks if value is greater or equal to zero to the closest tolerance possible.
    static bool IsGreaterOrEqualToZero (double value) { return IsGreaterOrEqual (value, 0.0); }

    //! Checks if value is less than zero to the closest tolerance possible.
    static bool IsLessThanZero (double value) { return IsLess (value, 0.0); }

    //! Checks if value is less or equal to zero to the closest tolerance possible.
    static bool IsLessOrEqualToZero (double value) { return IsLessOrEqual (value, 0.0); }
};

END_BENTLEY_NAMESPACE
