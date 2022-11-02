/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

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

/* Mask values for individual points */
#define HPOINT_NORMAL      0
#define HPOINT_MASK_BREAK  0x00000001   /* LAST point in polygon or polyline is marked as a break */
#define HPOINT_MASK_POINT  0x00000002   /* This point is isolated. Function that sets
                                                this also sets BREAK on both this and its predecessor*/
#define HPOINT_MASK_FRAGMENT_BREAK      0x00000004   /* Start or end of a fragment during clipping */
#define HPOINT_MASK_MAJOR_BREAK         0x00000008  /* End of major block (e.g. loop of character or grouped hole?) */
#define HPOINT_MASK_USER1               0x00010000  /* Temporary mask */
#define HPOINT_MASK_GROUP_BREAK         0x00020000  /* End of group of loops (major breaks) in loop containment */


#define HPOINT_MASK_CURVE_BITS           0x00000FF0    /* All curve definition points have at least one of these bits set */

#define HPOINT_MASK_CURVETYPE_BITS       0x00000F00    /* These bits indicated curve type */
#define HPOINT_MASK_POINTTYPE_BITS       0x000000F0    /* This bits indicate point type.  Interpretation
                                                            depends on curve type */
#define HPOINT_MASK_BREAK_BITS           0x0000000F     /* These bits indicate end of curve or loop */
#define HPOINT_MASK_INOUT_BITS           0x00003000    /* These 2 bits indicate in/out classification.
                                                                Note that there are 4 states:
                                                                00 = unclassified, ambiguous, unknown
                                                                11 = ON the boundary
                                                                01 = IN
                                                                11 = OUT
                                                        If you just look for IN bits, you will also
                                                                get the ON case.  Ditto for OUT.
                                                        */
#define HPOINT_MASK_INOUT_BIT_IN        0x000001000     /* "in" bit */
#define HPOINT_MASK_INOUT_BIT_OUT       0x000002000     /* "out" bit */


#define HPOINT_MASK_CURVETYPE_ELLIPSE    0x00000100    /* All points of an ellipse get this mask */
#define HPOINT_MASK_ELLIPSE_STARTEND     0x00000010    /* Ellipse start or end point */
#define HPOINT_MASK_ELLIPSE_CENTER       0x00000020    /* Ellipse center point */
#define HPOINT_MASK_ELLIPSE_VECTOR       0x00000030    /* Vector to ellipse  0 or 90 degree point */

#define HPOINT_MASK_CURVETYPE_BEZIER     0x00000200    /* All points of a bezier get this mask */
#define HPOINT_MASK_BEZIER_STARTEND      0x00000010    /* Start or end point of bezier */
#define HPOINT_MASK_BEZIER_POLE          0x00000020    /* Any intermediate pole of the bezier */


// Bspline packing.
#define HPOINT_MASK_CURVETYPE_BSPLINE    0x00000300    /* All points of a bspline get this mask */
#define HPOINT_MASK_BSPLINE_STARTEND     0x00000010    /* Actual start or end point of bspline.  (NOT a pole) knot values IS significant.*/
#define HPOINT_MASK_BSPLINE_POLE         0x00000020    /* Bspline pole with knot. */
#define HPOINT_MASK_BSPLINE_EXTRA_POLE   0x00000030    /* Bspline "extra" pole -- coordinates repolicate start pole to facilitate transform and range
                                                                logic, but knot value may be different. */



#define HPOINT_GET_CURVETYPE_BITS(mask)                 ((mask) & HPOINT_MASK_CURVETYPE_BITS)
#define HPOINT_GET_POINTTYPE_BITS(mask)                 ((mask) & HPOINT_MASK_POINTTYPE_BITS)
#define HPOINT_IS_ELLIPSE_POINT(mask)                   (HPOINT_GET_CURVETYPE_BITS(mask) == HPOINT_MASK_CURVETYPE_ELLIPSE)
#define HPOINT_IS_BEZIER_POINT(mask)                    (HPOINT_GET_CURVETYPE_BITS(mask) == HPOINT_MASK_CURVETYPE_BEZIER)
#define HPOINT_IS_CURVE_POINT(mask)                     (HPOINT_GET_CURVETYPE_BITS(mask) != 0)

#define HPOINT_MASK_ORDER               0x00FF0000
#define HPOINT_MASK_ORDER_BITSHIFT      (16)
#define HPOINT_MAX_ORDER                (255)



END_BENTLEY_GEOMETRY_NAMESPACE
