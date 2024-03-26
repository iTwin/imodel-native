/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_catmullRomPoles
(
DPoint3d        *outPts,            /* OUT     poles */
double          *outWts,            /* OUT     weights or NULL */
double          *knots,             /* OUT     knots or NULL */
DPoint3d        *points,            /* IN      points to be interpolated */
double          *weights,           /* IN      weights or NULL */
double          *inValues,          /* IN      u parameters or NULL */
int             numPoles,           /* IN      number of poles */
int             numKnots,           /* IN      number of interior knots */
int             numPoints           /* IN      number of points */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_catmullRomCurve
(
MSBsplineCurve  *curve,             /* OUT     open cubic spline curve */
DPoint3d        *inPts,             /* IN      points to be interpolated */
double          *inValues,          /* IN      u parameters or NULL */
int             inNum               /* IN      number of points */
);

/*---------------------------------------------------------------------------------**//**
* Note: See Farin's book (3rd edition, section 9.6) for ref.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_c2CubicInterpolatePoles
(
DPoint3d        *outPts,            /* OUT     poles */
double          *outWts,            /* OUT     weights or NULL */
double          *knots,             /* OUT     knots or NULL */
double          *inParams,          /* IN      u parameters or NULL */
DPoint3d        *points,            /* IN      points to be interpolated (first = last if closed) */
DPoint3d        *endTangents,       /* IN      end tangents or NULL */
double          *weights,           /* IN      weights or NULL */
BsplineParam    *bsplineParams,     /* IN      B-Spline parameters */
int             numPoints           /* IN      number of points (incl redundant endpt if closed) */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_c2CubicInterpolateCurve
(
MSBsplineCurve  *curve,        /* OUT     cubic spline curve */
DPoint3d        *inPts,        /* IN      points to be interpolated */
double          *inParams,     /* IN      u parameters starting with 0, or NULL */
int             numPts,        /* IN      number of points */
bool            remvData,      /* IN      T: remove coincide points */
double          tolerance,     /* IN      max dist betw coincide pts or closed curve */
DPoint3d        *endTangents,  /* IN      end tangents or NULL */
bool            closedCurve    /* IN      if true, closed Bspline is created */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspknotrm_reduceKnots
(
MSBsplineCurve  *curve,                /* IN OUT  open curve to simplify */
double          **auxPoles,            /* IN      extra dimensional poles */
DPoint3d        *tolerance,            /* IN      tolerance in each coordinates */
double          *auxTolerance,         /* IN      extra dimensional tolerances */
int             dimension              /* IN      number of extra dimensions */
);

/*---------------------------------------------------------------------------------**//**
* NOTE The terminologies follow the ones in Lyche and Morken "A data-reduction strategy for splines with applications to the approximation of
* functions and data, IMA J. of Num. Ana, 1988, 8, 185-208.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_curveDataReduction
(
MSBsplineCurve  *outCurve,        /* IN OUT  new curve with fewer knots and poles */
MSBsplineCurve  *inCurve,         /* IN      input curve */
DPoint3d        *tolerance        /* IN      tolerance in each coordinates */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_cubicDataReduce
(
MSBsplineCurve  *outCurve,          /* cubic non-rational approx curve */
MSBsplineCurve  *inCurve,           /* input curve */
double          tolerance           /* max deviation bwt two curves */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_arcLengthFromParameters
(
double          *arcLengthP,        /* OUT     arc length */
double          *errorAchievedP,     /* OUT     the absolute error achieved, i.e.,
                                          abs(arcLengthP - trueArcLength) */
double          startParam,         /* IN      strating parameter */
double          endParam,           /* IN      ending parameter */
MSBsplineCurve  *curveP,            /* IN      input curve */
double          relativeTol         /* IN      relative tolerance for integration
                                          using Simpson's Rule, i.e.,
                                          abs(errorAchivedP) / arcLengthP OUT     relativeTol */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_make2CompatibleByArcLength
(
MSBsplineCurve  *curve1,
MSBsplineCurve  *curve2
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_computeEqualChordByNumber
(
DPoint3d        *pointsP,   /* OUT     chord points with length of numSeg+1 */
double          *paramsP,   /* OUT     parameters for equal chord length */
MSBsplineCurve  *curveP,    /* IN      input B-spline curve */
int             numSeg,     /* IN      number of chords: number of pointsP - 1 */
double          convergeTol /* IN      tolerance checking if chords are same length */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_computeEqualChordByLengthExt
(
DPoint3d        **pointsPP,     /* OUT     points at each chord (or NULL) */
double          **paramsPP,     /* OUT     parameters at each chord points (or NULL) */
int             *numChordsP,     /* OUT     number of equal length chords + 1 (size of output arrays) */
double          chordLength,    /* IN      chord length */
MSBsplineCurve  *curveP         /* IN      B-spline curve */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int      bspcurv_computeEqualChordByLength
(
DPoint3d        **pointsPP,     /* OUT     points at each chord (or NULL) */
double          **paramsPP,     /* OUT     parameters at each chord points (or NULL) */
int             *numChords,     /* OUT     number of equal length chords + 1 (size of output arrays) */
double          chordLength,    /* IN      chord length */
MSBsplineCurve  *curveP,        /* IN      B-spline curve */
double          tolerance       /* IN      curve-curve intersection tolerance */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspcurv_convertConicToBspline
(
MSBsplineCurve  *pCurve,    /* OUT     rational Bspline with three poles */
DPoint3d        *pStart,    /* IN      starting point */
DPoint3d        *pEnd,      /* IN      ending point */
DPoint3d        *pInt,      /* IN      tangent intersection point */
double          paramS,     /* IN      barry centric cood of another point */
double          paramE,     /* IN      barry centric cood of another point */
double          paramI      /* IN      barry centric cood of another point */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      mdlBspline_computeEqualDeviationChordByNumber
(
DPoint3d        *pointsP,   /* OUT     chord points with length of numSeg+1.  CALLER MUST ALLOCATE */
double          *paramsP,   /* OUT     parameters for equal chord error   CALLER MUST ALLOCATE*/
double          *minDistP,  /* OUT     smallest distance to chord */
double          *maxDistP,  /* OUT     largest distance to chord */
MSBsplineCurve  *curveP,    /* IN      input B-spline curve */
int             numSeg     /* IN      number of chords: number of pointsP - 1 */
);

/*---------------------------------------------------------------------------------**//**
* @description    Creates a planar B-spline curve that simultaneously interpolates the given
* data points and tangent directions at those points, allowing for linear segments.
* Successive tangents at the same point are optionally removed before processing.
* The resulting curve is always planar, quadratic, and C1 continuous almost everywhere
* (it is only C0 at the junctions of linear segments, and only G1 at a closed curve's
* start/end point).  This routine allocates memory for the returned curve as necessary.
* @param    pCurve      OUT planar quadratic B-spline curve
* @param    pTangents   IN  tangents to interpolate
* @param    numPts      IN  number of tangents supplied
* @param    bClosed     IN  true to create a closed interpolant
* @param    bCompress   IN  true to remove successive tangents rooted at the same point
* @param    tolerance   IN  max relative error for duplicate points
* @return   SUCCESS indicates successful completion; ERROR indicates there is
* not enough memory for allocation; ERROR indicates too few points
* were passed in; ERROR indicates that successive tangents were parallel,
* rooted at the same point, or didn't intersect as expected.
* @alinkjoin   usmthmdlBspline_catmullRomCurve usmthmdlBspline_leastSquaresToCurve
*              usmthmdlBspline_cubicInterpolation usmthmdlBspline_cubicInterpolationExt
*              usmthmdlBspline_cubicInterpolationExt2
* @group        "B-spline Creation"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt    mdlBspline_interpolateCoplanarTangents
(
MSBsplineCurve*     pCurve,
const DRay3d*       pTangents,
int                 numTangents,
bool                bClosed,
bool                bCompress,
double              tolerance
);

END_BENTLEY_GEOMETRY_NAMESPACE

