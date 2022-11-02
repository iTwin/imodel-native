/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct RefCountedMSInterpolationCurve;
typedef RefCountedPtr <RefCountedMSInterpolationCurve> MSInterpolationCurvePtr;

//! MSInterpolationCurve carries passthrough (interpolation) points for a curve.
struct MSInterpolationCurve
    {
	//! Control data for order, periodicity, counts, and end conditions
    InterpolationParam  params;
	//! Start tangent end for curve fit.
    DPoint3d            startTangent;   /* normalized and pointing into curve, or zero */
	//! End tangent for curve fit.
    DPoint3d            endTangent;     /* normalized and pointing into curve, or zero */
	//! Passthrough points for curve fit.
    DPoint3d            *fitPoints;     /* duplicated fitpoint at end if periodic */
	//! Knots for curve fit.
    double              *knots;         /* full knot vector, recomputed from fit points at display time */
	//! Display parameters.
    BsplineDisplay      display;

    //! Return a refcounted pointer to an empty curve structure.
    GEOMDLLIMPEXP static MSInterpolationCurvePtr CreatePtr ();
	//! Free the memory allocated to the poles of the interpolation curve.
    GEOMDLLIMPEXP void  ReleaseMem ();
    //! Allocate specified number of fit points. Optionally copy in points from buffer.
    GEOMDLLIMPEXP MSBsplineStatus AllocateFitPoints (int count, DPoint3dCP data = NULL);
	//! Allocate specified number of knots.  Optionally copy in knots from buffer.
    GEOMDLLIMPEXP MSBsplineStatus AllocateKnots (int count, double const *data = NULL);
    //! Allocate memory for the B-spline curve and copies all data from the input inerpolation curve.
    GEOMDLLIMPEXP MSBsplineStatus CopyFrom (MSInterpolationCurveCR source);
    //! Clear to zero state.  DOES NOT RELEASE MEMORY.
    GEOMDLLIMPEXP void Zero ();
    GEOMDLLIMPEXP int GetOrder () const;
    //! This routine creates a MSInterpolationCurve from an array of points and given end tangents.
    //! return ERROR if no results.
    //! @param [in]  inPts Points to be interpolated.
    //! @param [in]  numPts Number of points.
    //! @param [in]  remvData true : remove coincident points.
    //! @param [in]  remvTol For finding coincident pts or closed curve.
    //! @param [in]  endTangents Normalized end tangents or NULL.
    //! @param [in]  closedCurve If true, closed Bspline is created.
    //! @param [in]  colinearTangents true : ensure colinear computed end tangents (zero/NULL endTangent(s), geometrically closed spline).
    //! @param [in]  chordLenTangents true/false: scale endTangent by chordlen/bessel (nonzero endTangent, open spline).
    //! @param [in]  naturalTangents true/false: compute natural/bessel endTangent (zero/NULL endTangent, open spline).
    GEOMDLLIMPEXP MSBsplineStatus InitFromPointsAndEndTangents (DPoint3d *inPts, int numPts, bool remvData, double remvTol, DPoint3d *endTangents, 
                                        bool closedCurve, bool colinearTangents, bool chordLenTangents, bool naturalTangents);


    //! This routine creates a MSInterpolationCurve from an array of points and given end tangents.
    //! return ERROR if no results.
    //! @param [in]  inPoints Points to be interpolated.
    //! @param [in]  remvData true : remove coincident points.
    //! @param [in]  remvTol For finding coincident pts or closed curve.
    //! @param [in]  endTangents Normalized end tangents or NULL.
    //! @param [in]  closedCurve If true, closed Bspline is created.
    //! @param [in]  colinearTangents true : ensure colinear computed end tangents (zero/NULL endTangent(s), geometrically closed spline).
    //! @param [in]  chordLenTangents true/false: scale endTangent by chordlen/bessel (nonzero endTangent, open spline).
    //! @param [in]  naturalTangents true/false: compute natural/bessel endTangent (zero/NULL endTangent, open spline).
    GEOMDLLIMPEXP MSBsplineStatus InitFromPointsAndEndTangents (bvector<DPoint3d> &inPoints, bool remvData, double remvTol, DPoint3d *endTangents, 
                                        bool closedCurve, bool colinearTangents, bool chordLenTangents, bool naturalTangents);

    //! Init by direct copy to members.   This is intended for serialization where there is little checking of geometric validity.
    GEOMDLLIMPEXP MSBsplineStatus Populate (
                            int order, bool periodic,
                            int isChordLenKnots,
                            int isColinearTangents,
                            int isChordLenTangents,
                            int isNaturalTangents,
                            DPoint3dCP pFitPoints,
                            int numFitPoints,
                            double const *pKnots, int numKnots,
                            DVec3dCP startTangent = nullptr, 
                            DVec3dCP endTangent = nullptr
                            );
                            
    GEOMDLLIMPEXP bool AlmostEqual (MSInterpolationCurveCR other, double tolerance) const;
    }; // MSInterpolationCurve

//! MSInterpolationCurve with IRefCounted support for smart pointers.
//! Create via MSInterpolationCurve::CreatePtr ();
struct RefCountedMSInterpolationCurve : public MSInterpolationCurve, RefCountedBase
    {
friend struct MSInterpolationCurve;
    protected:
    RefCountedMSInterpolationCurve ();
    ~RefCountedMSInterpolationCurve ();
    };

END_BENTLEY_GEOMETRY_NAMESPACE
