/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/cpp/refmethods/reffrange3d.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); FIX_MAX(value, max);

#define MSVECTOR_
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define ROUND_FLOAT_AWAY_FROM_ZERO 1.000000119
#define ROUND_FLOAT_TOWARDS_ZERO   0.9999999404


float DoubleOps::DoubleToFloatRoundLeft (double d)
    {
    float f = (float) d;
    return (f<d) ? f : (float) (d * (d<0 ? ROUND_FLOAT_AWAY_FROM_ZERO : ROUND_FLOAT_TOWARDS_ZERO));
    }

float DoubleOps::DoubleToFloatRoundRight (double d)
    {
    float f = (float) d;
    return (f>d) ? f : (float) (d * (d<0 ? ROUND_FLOAT_TOWARDS_ZERO : ROUND_FLOAT_TOWARDS_ZERO));
    }
bool FRange3d::IsNull () const
    {
    return low.x == FLT_MAX
        && low.y == FLT_MAX
        && low.z == FLT_MAX
        && high.x == -FLT_MAX
        && high.y == -FLT_MAX
        && high.z == -FLT_MAX
        ;
    }

FRange3d FRange3d::NullRange ()
    {
    return FRange3d::from (
             FLT_MAX,  FLT_MAX,  FLT_MAX,
            -FLT_MAX, -FLT_MAX, -FLT_MAX
            );
    }

FRange3d FRange3d::From (DRange3dCR dRange)
    {
    if (dRange.IsEmpty ())
        return NullRange ();
    return FRange3d::from (
        FPoint3d::FromRoundLeft (dRange.low),
        FPoint3d::FromRoundRight (dRange.high)
        );
    }

FPoint3d FRange3d::Low  () const {return low;}
FPoint3d FRange3d::High () const {return high;}

END_BENTLEY_GEOMETRY_NAMESPACE
