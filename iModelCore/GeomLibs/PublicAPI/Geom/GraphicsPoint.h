/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/GraphicsPoint.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! A point with markup fields.
//!
struct GEOMDLLIMPEXP GraphicsPoint
{
    //! Homogeneous point.
    DPoint4d point;
    //! double value available for application use.
    double   a;
    //! integer mask used by GraphicsPointArray internals.
    int      mask;
    //! integer value available for application use.
    int      userData;
    //! double value used by the GraphicsPointArray internals.
    double b;
    //! integer value used by GraphcisPointArray internals.
    size_t index;

//! Constructor with 0 coordinates and markup.
GraphicsPoint ();
//! Constructor with xyzw (DPoint4d) data.
GraphicsPoint (DPoint4dCR point, double a = 0.0, int userData = 0, int mask = 0, double b = 0.0, size_t index = 0);
//! Constructor with xyz (DPoint3d) data and separate weight.
GraphicsPoint (DPoint3dCR point, double w = 1.0, double a = 0.0, int userData = 0, int mask = 0, double b = 0.0, size_t index = 0);
//! Constructor with separate coordinate values.
GraphicsPoint (double x, double y, double z = 0.0, double w = 1.0, double a = 0.0, int userData = 0, int mask = 0, double b = 0.0, size_t = 0);


//! @description Set point, but DIVIDE xyz part by weight.
//!  (i.e. caller has been carrying point as wx,wy,wz,w)
//! @param [in] inputPoint coordinates in wx,wy,wz form.
void SetPointPreserveWeight (DPoint4dCR inputPoint);

//! Set mask bits for curve order.  Only valid for points within a bspline.
bool SetOrder(int order);
//! Query the order of the curve this point is part of.  Only valid for points within a bspline.
 int GetOrder() const;

//! Query the curve type this point is part of.
int GetCurveType () const;
//! Query the point type for this point.
int GetPointType () const;
//! Test
bool CheckCurveAndPointType (int curveType, int pointType) const;

//! Ask if this point is a curve break.
bool IsCurveBreak () const;

//! Ask if this point is the end of a looop.
bool IsLoopBreak () const;

//! Set or clear the flag identifying this point as the end of a linestring or other extended curve. (aka "minor" break)
void SetCurveBreak (bool value = true);
//! set or clear the flag identifying this point as a loop break. (aka "Major" break)
void SetLoopBreak (bool value = true);
//!
//! @description normalize the (homogenous) point into caller's point.
//! @param [out] xyz receives coordinates
//! @return false if weight is zero.  (xyz still copied)
//!
bool GetNormalized (DPoint3dR xyz) const;

//! Set entire point to zeros.
void Zero ();
};
END_BENTLEY_GEOMETRY_NAMESPACE
