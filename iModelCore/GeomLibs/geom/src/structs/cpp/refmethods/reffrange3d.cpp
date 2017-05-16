/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/cpp/refmethods/reffrange3d.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

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
// -- FROM with DPoint3d inputs
FRange3d FRange3d::From (DPoint3dCR point)
    {
    auto range = NullRange ();
    range.Extend (point);   // due to directional rounding, this is generally NOT a single point!!!
    return range;
    }

FRange3d FRange3d::From (DPoint3dCR pointA, DPoint3dCR pointB)
    {
    auto range = NullRange ();
    range.Extend (pointA);
    range.Extend (pointB);
    return range;
    }
FRange3d FRange3d::From (bvector<DPoint3d> const &points)
    {
    DRange3d rangeD = DRange3d::From (points);
    return FRange3d::From (rangeD);
    }

// -- FROM with FPoint3d inputs
FRange3d FRange3d::From (FPoint3dCR point)
    {
    return from (point, point);
    }

FRange3d FRange3d::From (FPoint3dCR pointA, FPoint3dCR pointB)
    {
    auto range = from (pointA, pointA);
    range.Extend (pointB);
    return range;
    }

FRange3d FRange3d::From (bvector<FPoint3d> const &points)
    {
    auto range = NullRange ();
    for (auto &p: points)
        range.Extend (p);
    return range;
    }




// extend low, high with a new double value, using directional double-to-float
inline void FixLowHigh (float &low, float a, float &high)
    {
    if (a < low)
        low = a;
    if (a > high)
        high = a;
    }

// extend low, high with a new double value, using directional double-to-float
inline void FixLowHighD (float &low, double x, float &high)
    {
    float a = DoubleOps::DoubleToFloatRoundLeft (x);
    if (a < low)
        low = a;
    float b = DoubleOps::DoubleToFloatRoundRight (x);
    if (b > high)
        high = b;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      05/17
+----------------------------------------------------------------------*/
void FRange3d::Extend (DPoint3dCR point)
    {
    FixLowHighD (low.x, point.x, high.x);
    FixLowHighD (low.y, point.y, high.y);
    FixLowHighD (low.z, point.z, high.z);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      05/17
+----------------------------------------------------------------------*/
void FRange3d::Extend (DPoint3dCR pointA, DPoint3dCR pointB)
    {
    Extend (pointA);
    Extend (pointB);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      05/17
+----------------------------------------------------------------------*/
void FRange3d::Extend (bvector<DPoint3d> const &points)
    {
    for (auto &xyz : points)
        Extend (xyz);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      05/17
+----------------------------------------------------------------------*/
void FRange3d::Extend (FPoint3dCR point)
    {
    FixLowHigh (low.x, point.x, high.x);
    FixLowHigh (low.y, point.y, high.y);
    FixLowHigh (low.z, point.z, high.z);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      05/17
+----------------------------------------------------------------------*/
void FRange3d::Extend (FPoint3dCR pointA, FPoint3dCR pointB)
    {
    Extend (pointA);
    Extend (pointB);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      05/17
+----------------------------------------------------------------------*/
void FRange3d::Extend (bvector<FPoint3d> const &points)
    {
    for (auto &xyz : points)
        Extend (xyz);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
