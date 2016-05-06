/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeNumerical.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__

#pragma once

#include "Bentley.h"
#include <math.h>
#include <float.h>
#include <algorithm>

BEGIN_BENTLEY_NAMESPACE
/*=================================================================================**//**
* Numerical utilities
* @bsiclass                                     Sam.Wilson                      07/2011
+===============+===============+===============+===============+===============+======*/
struct          BeNumerical
{
#if defined (_WIN32) // Windows && WinRT
    static double   BeNextafter (double x, double y)    {return _nextafter(x,y);}
    static int      BeIsnan (double v)                  {return _isnan(v);}
    static int      BeFinite (double v)                 {return _finite(v);}
#elif defined (__APPLE__) || defined (ANDROID) || defined (__linux) || defined (__EMSCRIPTEN__)
    static double   BeNextafter (double x, double y)    {return nextafter(x,y);}

    #if defined (__APPLE__)
        static int      BeIsnan (double v)                  {return isnan (v);}
        static int      BeFinite (double v)                 {return isfinite(v);}
    #else
        static int      BeIsnan (double v)                  {return std::isnan (v);}
        static int      BeFinite (double v)                 {return finite(v);}
    #endif
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
        if (fabs (sv2 - sv1) < ComputeComparisonTolerance (sv1, sv2))
            return 0;
        return sv1 < sv2? -1: 1;
        }
};

END_BENTLEY_NAMESPACE

