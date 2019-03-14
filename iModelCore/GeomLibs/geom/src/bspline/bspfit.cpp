/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspfit.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_constructInterpolationWithKnots
(
MSInterpolationCurve    *pCurve,        /* <= interpolation curve */
DPoint3d                *pPoints,       /* => interpolation points */
DPoint3d                *pStartTan,     /* => not used if periodic */
DPoint3d                *pEndTan,       /* => not used if periodic */
double                  *pKnots,        /* => full knot vector */
InterpolationParam      *pParams
)
    {
    int     status;

    if (!pCurve || !pPoints || !pKnots || !pParams)
        {
        return  ERROR;
        }

    pCurve->params = *pParams;
    if (SUCCESS == (status = bspcurv_allocateInterpolationCurve (pCurve)))
        {
        DPoint3d    zeroTan = {0.0, 0.0, 0.0};

        memcpy (pCurve->fitPoints, pPoints, pParams->numPoints * sizeof(*pPoints));
        memcpy (pCurve->knots, pKnots, pParams->numKnots * sizeof(*pKnots));
        pCurve->startTangent = (!pParams->isPeriodic && pStartTan) ? *pStartTan : zeroTan;
        pCurve->endTangent = (!pParams->isPeriodic && pEndTan) ? *pEndTan : zeroTan;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_constructInterpolationCurve
(
MSInterpolationCurve    *pCurve,
DPoint3d        *inPts,             /* => points to be interpolated */
int             numPts,             /* => number of points */
bool            remvData,           /* => true = remove coincident points */
double          remvTol,            /* => for finding coincident pts or closed curve */
DPoint3d        *endTangents,       /* => normalized end tangents or NULL */
bool            closedCurve,        /* => if true, closed Bspline is created */
bool            chordLenKnots,      /* => T/F: chordlen/uniform knots (!inParams, closed spline) */
bool            colinearTangents,   /* => T: ensure colinear computed end tangents (zero/NULL endTangent(s), geometrically closed spline) */
bool            chordLenTangents,   /* => T/F: scale endTangent by chordlen/bessel (nonzero endTangent, open spline) */
bool            naturalTangents     /* => T/F: compute natural/bessel endTangent (zero/NULL endTangent, open spline) */
)
    {
    int             status;
    DPoint3d*       points = NULL;
    MSBsplineCurve  curve;

    /* Make a local copy */
    if (NULL == (points = (DPoint3d*)msbspline_malloc ((numPts+1)*sizeof(DPoint3d), HEAPSIG_BCRV)))
        {
        status = ERROR;
        goto wrapup;
        }

    /* We need the compressed points and numPts below */
    if (SUCCESS != (status = mdlBspline_c2CubicInterpolatePrepareFitPoints (points, NULL, &numPts, &closedCurve, inPts, NULL, remvData, remvTol)))
        goto wrapup;

    memset (&curve, 0, sizeof (MSBsplineCurve));
    if (SUCCESS == (status = bspcurv_c2CubicInterpolateCurveExt (&curve, points, NULL, numPts, false, remvTol, endTangents, closedCurve,
                                                                 chordLenKnots, colinearTangents, chordLenTangents, naturalTangents)))
        {
        InterpolationParam  params;
        DPoint3d*           pStartTan = NULL, *pEndTan = NULL;

        params.isChordLenKnots = chordLenKnots;
        params.isColinearTangents = colinearTangents;
        params.isChordLenTangents = chordLenTangents;
        params.isNaturalTangents = naturalTangents;
        params.isPeriodic = curve.params.closed;
        params.numKnots = bspknot_numberKnots (curve.params.numPoles, curve.params.order, curve.params.closed);
        params.order = curve.params.order;
        params.numPoints = numPts;

        if (endTangents)
            {
            // edl june 2017 this use of null arg and pointer assignment as parameter setup defeats gema remap
            // leave pTan NULL if tan is zero vector; o.w., set it to normalized tan
            DPoint3d zeroPoint = DPoint3d::FromZero ();
            if (!endTangents[0].IsEqual (zeroPoint, fc_nearZero))
                {
                pStartTan = &endTangents[0];
                endTangents[0].Normalize ();
                }
            if (!endTangents[1].IsEqual (zeroPoint, fc_nearZero))
                {
                pEndTan = &endTangents[1];
                endTangents[1].Normalize ();
                }
            }

        status = bspcurv_constructInterpolationWithKnots (pCurve, points, pStartTan, pEndTan, curve.knots, &params);
        bspcurv_freeCurve (&curve);
        }

wrapup:
    if (points) msbspline_free (points);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          05/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     mdlBspline_removeCoincidePoint
(
DPoint3d        *outPts,
int             *numOut,
DPoint3d        *inPts,
int             numIn,
double          tol
)
    {
    mdlBspline_removeCoincidentPoints (outPts, NULL, numOut, inPts, NULL, numIn, tol, false, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    David.Assaf     10/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     mdlBspline_removeCoincidePointExt
(
DPoint3d        *outPts,        // compressed points
double          *outParams,     // compressed params (or NULL)
int             *numOut,        // size of both output arrays
const DPoint3d  *inPts,         // point array to compress
const double    *inParams,      // param associated with each pt (or NULL)
int             numIn,          // size of both input arrays
double          tol             // min squared distance between different points
)
    {
    mdlBspline_removeCoincidentPoints (outPts, outParams, numOut, inPts, inParams, numIn, tol, false, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  mdlBspline_extractInterpolationTangentPoints
(
DPoint3d                *pStartTangentPoint,
DPoint3d                *pEndTangentPoint,
MSInterpolationCurve    *pCurve
)
    {
    int         numPoints = pCurve->params.numPoints;
    double      mag;

    if (pStartTangentPoint)
        {
        mag = pCurve->fitPoints[0].Distance (*(&pCurve->fitPoints[1])) * INTERPOLATION_TANGENT_SCALE;
        pStartTangentPoint->SumOf (*(&pCurve->fitPoints[0]), pCurve->startTangent, mag);
        }

    if (pEndTangentPoint)
        {
        mag = pCurve->fitPoints[numPoints-1].Distance (*(&pCurve->fitPoints[numPoints-2])) * INTERPOLATION_TANGENT_SCALE;
        pEndTangentPoint->SumOf (*(&pCurve->fitPoints[numPoints-1]), pCurve->endTangent, mag);
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          02/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      mdlBspline_convertInterpolationToBspline
(
MSBsplineCurve          *pCurve,
MSInterpolationCurve    *pFitCurve
)
    {
    DPoint3d    tan[2];

    tan[0] = pFitCurve->startTangent;
    tan[1] = pFitCurve->endTangent;

    StatusInt   status;

    if (SUCCESS != (status = bspcurv_c2CubicInterpolateCurveExt (pCurve, pFitCurve->fitPoints, NULL, pFitCurve->params.numPoints,
                                                                 true, fc_epsilon, tan, 0 != pFitCurve->params.isPeriodic, 
                                                                 0 != pFitCurve->params.isChordLenKnots, 
                                                                 0 != pFitCurve->params.isColinearTangents, 
                                                                 0 != pFitCurve->params.isChordLenTangents,
                                                                 0 != pFitCurve->params.isNaturalTangents)))
        return status;

    pCurve->display = pFitCurve->display;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          02/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      mdlBspline_openInterpolationCurve
(
MSInterpolationCurve    *outCurve,
MSInterpolationCurve    *inCurve,
double                  u          /* => currently only allows u=0.0 */
)
    {
    MSBsplineCurve  curve;
    int             status;

    if (!inCurve->params.isPeriodic)
        return SUCCESS;

    /* u != 0.0 or 1.0 means we have to add a start/end fitpoint (future task?) */
    if (fabs(u) >= fc_epsilon && fabs (u - 1.0) >= fc_epsilon)
        return ERROR;

    if (SUCCESS != (status = bspcurv_copyInterpolationCurve (outCurve, inCurve)))
        return status;

    /*
    NOTE: until we can compute the end tangent LENGTHS of the periodic curve
    and extend the interpolator to use these lengths (and not autocompute them),
    this WILL NOT PRESERVE CURVE SHAPE in all cases.  Even if we do that, the
    open curve will not export to ACAD, which does not allow for custom end
    tangent scaling.
    */

    status = mdlBspline_convertInterpolationToBspline (&curve, outCurve);
    if (SUCCESS == status)
        {
        double  start = 0.0, end = 0.0;
        int     i;
        mdlBspline_getNaturalParameterRange (&start, &end, &curve);

        /* periodic curve end tangents aren't stored: convert to pole-based and compute 'em */
        bspcurv_evaluateCurvePoint (NULL, &outCurve->startTangent, &curve, start);  /* into curve */
        bspcurv_evaluateCurvePoint (NULL, &outCurve->endTangent, &curve, end);      /* away from curve */
        outCurve->endTangent.Scale (outCurve->endTangent, -1.0);    /* into curve */
        outCurve->startTangent.Normalize ();
        outCurve->endTangent.Normalize ();

        // clamp knot vector
        for (i = 0; i < outCurve->params.order - 1; i++)
            {
            outCurve->knots[i] = start;
            outCurve->knots[outCurve->params.numKnots - i - 1] = end;
            }

        outCurve->params.isPeriodic = 0;

        bspcurv_freeCurve (&curve);
        }

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          02/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      mdlBspline_reverseInterpolationCurve
(
MSInterpolationCurve  *inCurve,
MSInterpolationCurve  *outCurve
)
    {
    int         status, i;

    if (outCurve != inCurve)
        {
        if (SUCCESS != (status = bspcurv_copyInterpolationCurve (outCurve, inCurve)))
            return status;

        for (i = 0; i < inCurve->params.numPoints; i++)
            {
            outCurve->fitPoints[i] = inCurve->fitPoints[inCurve->params.numPoints - i - 1];
            }

        for (i = 0; i < inCurve->params.numKnots; i++)
            {
            outCurve->knots[i] = 1.0 - inCurve->knots[inCurve->params.numKnots - i - 1];
            }

        outCurve->startTangent = inCurve->endTangent;
        outCurve->endTangent = inCurve->startTangent;
        }
    else
        {
        double      *knots = (double*)BSIBaseGeom::Malloc (inCurve->params.numKnots * sizeof(double));
        DPoint3d    *points = (DPoint3d*)BSIBaseGeom::Malloc (inCurve->params.numPoints * sizeof(DPoint3d));
        memcpy (points, inCurve->fitPoints, inCurve->params.numPoints * sizeof(DPoint3d));
        memcpy (knots,  inCurve->knots, inCurve->params.numKnots * sizeof(double));
        for (i = 0; i < inCurve->params.numPoints; i++)
            {
            outCurve->fitPoints[i] = points[inCurve->params.numPoints - i - 1];
            }

        for (i = 0; i < inCurve->params.numKnots; i++)
            {
            outCurve->knots[i] = 1.0 - knots[inCurve->params.numKnots - i - 1];
            }

        points[0] = inCurve->startTangent;
        points[1] = inCurve->endTangent;
        outCurve->startTangent = points[1];
        outCurve->endTangent = points[0];
        
        BSIBaseGeom::Free (points);
        BSIBaseGeom::Free (knots);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          02/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      mdlBspline_closeInterpolationCurve
(
MSInterpolationCurve    *outCurve,
MSInterpolationCurve    *inCurve
)
    {
    double  start, end;
    int     i, status;

    if (inCurve->params.isPeriodic)
        return SUCCESS;

    /* must be geometrically closed */
    if (inCurve->fitPoints[0].Distance (*(&inCurve->fitPoints[inCurve->params.numPoints-1])) >= fc_epsilon * fc_epsilon)
        return ERROR;

    // cannot make curve periodic with less than 5 points (counting first = last point)
    if (inCurve->params.numPoints <= 4)
        return ERROR;

    /*
    NOTE: since the interpolator ignores end tangent information when
    constructing a periodic interpolant, it is ALMOST ALWAYS IMPOSSIBLE TO
    PRESERVE CURVE SHAPE here.
    */

    if (SUCCESS != (status = bspcurv_copyInterpolationCurve (outCurve, inCurve)))
        return status;

    // null out the tangents, since they are meaningless in a periodic curve
    memset (&outCurve->startTangent, 0, sizeof (outCurve->startTangent));
    memset (&outCurve->endTangent, 0, sizeof (outCurve->endTangent));

    // extend knot vector periodically
    start = outCurve->knots[outCurve->params.order - 1];
    end = outCurve->knots[outCurve->params.numKnots - outCurve->params.order];
    for (i = 0; i < outCurve->params.order - 1; i++)
        {
        outCurve->knots[i] = start - (end - outCurve->knots[outCurve->params.numKnots - 2 * outCurve->params.order + 1 + i]);
        outCurve->knots[outCurve->params.numKnots - i - 1] = end + (outCurve->knots[2 * outCurve->params.order - i - 2] - start);
        }

    outCurve->params.isPeriodic = 1;

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    David.Assaf     10/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt    mdlBspline_computeChordLengthKnotVector
(
double*         pKnots,         // full knot vector
const DPoint3d* pPoles,         // weighted if rational
const double*   pWeights,       // for rational (or NULL)
int             numPoles,
int             order,
bool            bPeriodic       // if true, it is assumed that first/last pole are NOT equal
)
    {
    DPoint3d    diff;
    double      diffw, scale, delta, *pIntKnots;
    int         i, j, nIntKnots, degree = order - 1;
    StatusInt   status = SUCCESS;

    if (!pKnots || !pPoles || order < 2 || numPoles < order)
        return ERROR;

    pKnots[degree] = 0.0;
    pIntKnots = &pKnots[order];

    if (bPeriodic)
        {
        nIntKnots = numPoles - 1;

        // compute interior knots
        for (i = 0; i < nIntKnots; i++)
            {
            diff.DifferenceOf (pPoles[i + 1], pPoles[i]);
            delta = diff.DotProduct (diff);
            if (pWeights)
                {
                diffw = pWeights[i + 1] - pWeights[i];
                delta += diffw * diffw;
                }
            pIntKnots[i] = pIntKnots[i - 1] + sqrt(delta);
            }

        // compute end knot
        diff.DifferenceOf (pPoles[0], pPoles[nIntKnots]);
        delta = diff.DotProduct (diff);
        if (pWeights)
            {
            diffw = pWeights[0] - pWeights[nIntKnots];
            delta += diffw * diffw;
            }
        pIntKnots[nIntKnots] = pIntKnots[nIntKnots - 1] + sqrt(delta);

        // start/end pole shouldn't be duplicated in periodic case
        if (pIntKnots[nIntKnots] == pIntKnots[nIntKnots - 1])
            return ERROR;

        // compute first/last degree knots
        for (i = 0; i < degree; i++)
            {
            pKnots[i] = pIntKnots[nIntKnots - degree + i] - 1.0;
            pIntKnots[nIntKnots + 1 + i] = pIntKnots[i] + 1.0;
            }
        }
    else
        {
        double  sum;
        double* pDist = (double*)msbspline_malloc (numPoles * sizeof (double), HEAPSIG_BCRV);
        if (!pDist)
            return ERROR;

        nIntKnots = numPoles - order;

        // initialize pole distances
        for (i = 0; i < numPoles - 1; i++)
            {
            diff.DifferenceOf (pPoles[i + 1], pPoles[i]);
            delta = diff.DotProduct (diff);
            if (pWeights)
                {
                diffw = pWeights[i + 1] - pWeights[i];
                delta += diffw * diffw;
                }
            pDist[i] = sqrt (delta);
            }

        // compute interior knots + end knot
        for (i = 0; i < nIntKnots + 1; i++)
            {
            sum = 0.0;
            for (j = 0; j < degree; j++)
                sum += pDist[i + j];
            pIntKnots[i] = pIntKnots[i - 1] + sum;
            }

        // compute first/last degree knots
        for (i = 0; i < degree; i++)
            {
            pKnots[i] = 0.0;
            pIntKnots[nIntKnots + 1 + i] = 1.0;
            }

        msbspline_free (pDist);
        }

    // normalize interior knots
    scale = 1.0 / pIntKnots[nIntKnots];
    for (i = 0; i < nIntKnots; i++)
        pIntKnots[i] *= scale;

    // normalize end knot
    pIntKnots[nIntKnots] = 1.0;

    return status;
    }



// zeroVector replicated in mdlcurv.c!!!
/*---------------------------------------------------------------------------------**//**
* function zeroVector
* @bsimethod                                                    Earlin.Lutz     07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool      zeroVector    /* <= true if vector is zero to tolerance */
(
DPoint3d    *pVector
)
    {
    /* Any better ideas for tolerance?  This will protect from true zeros, not much else. */
    static double s_tolerance = 1.0e-10;
    return      fabs (pVector->x) < s_tolerance
            &&  fabs (pVector->y) < s_tolerance
            &&  fabs (pVector->z) < s_tolerance;
    }


/*---------------------------------------------------------------------------------**//**
* function samePoint
* @bsimethod                                                    Earlin.Lutz     07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool      samePoint    /* <= true if same point to within tolerance */
(
DPoint3d    *pPoint0,
DPoint3d    *pPoint1,
double tolerance
)
    {
    return      fabs (pPoint0->x - pPoint1->x) < tolerance
            &&  fabs (pPoint0->y - pPoint1->y) < tolerance
            &&  fabs (pPoint0->z - pPoint1->z) < tolerance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/97
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       appendSmoothSegment
(
CurveVectorPtr &curves,
MSBsplineCurve  *curveP,
DPoint3d        *pointP,
int             nPoints,
int             startVertex,
double          tolerance
)
    {
    StatusInt       status;
    MSBsplineCurve  segmentCurve;
    int numKnots;
    static double s_zeroLengthRelTol = 1.0e-8;   // pretty fat tolerance -- but in line with typical equal knots.
    static double s_zeroLengthAbsTol = 1.0e-10;
    double lengthTol = s_zeroLengthAbsTol + s_zeroLengthRelTol * PolylineOps::Length (pointP, NULL, 1, nPoints);

    /* Trim duplicate points from end */
    while (nPoints > 1 && samePoint (&pointP[nPoints-1], &pointP[nPoints-2], lengthTol))
        {
        nPoints--;
        }
    /* Exit quietly if nothing left */
    if (nPoints < 2)
        return SUCCESS;

    if (nPoints > 2)
        {
        DPoint3d            tol3d;

        tol3d.x = tol3d.y = tol3d.z = tolerance;

        memset (&segmentCurve, 0, sizeof(segmentCurve));
        if (curveP != nullptr)
            segmentCurve.display = curveP->display;
        if (SUCCESS == (status = bspcurv_c2CubicInterpolateCurve (&segmentCurve, pointP, NULL, nPoints, true, tolerance, NULL, false)))
            {
            numKnots = bspknot_numberKnots (segmentCurve.params.numPoles, segmentCurve.params.order,
                                     segmentCurve.params.closed);
            // We know that nothing will be done if there aren't enough interior knots ....
            if (numKnots - 2 * segmentCurve.params.order > 1 && tolerance > 0.0)
                status = bspcurv_curveDataReduction (&segmentCurve, &segmentCurve, &tol3d);
            else
                {
                numKnots = 0;// Just here to make it easy to stop in debugger...
                }
            }
        }
    else
        {
        status = bspconv_lstringToCurveStruct (&segmentCurve, pointP, nPoints);
        }

    if (SUCCESS == status)
        {
        if (curves.IsValid ())
            {
            // Append the segment to the curve vector
            ICurvePrimitivePtr segmentPrim = ICurvePrimitive::CreateBsplineCurveSwapFromSource (segmentCurve);
            curves->Add (segmentPrim);
            }
        else if (curveP->params.numPoles)
            {
            bspknot_scaleCurveKnots (curveP, (double) startVertex);
            bspknot_scaleCurveKnots (&segmentCurve, (double) nPoints);
            bspcurv_appendCurves (curveP, curveP, &segmentCurve, false, false);
            bspcurv_freeCurve (&segmentCurve);
            }
        else
            {
            *curveP = segmentCurve;
            }
        }
    else
        {
        bspcurv_freeCurve (&segmentCurve);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* function angleBetweenVectors
* Returns the angle between two vectors. This angle is between 0 and pi. Rotating the first vector by this angle around the cross product
* between the vectors aligns it with the second vector.
* @bsimethod                                                    Earlin.Lutz     07/96
+---------------+---------------+---------------+---------------+---------------+------*/
static double   angleBetweenVectors    /* <= angle between vectors */
(
DPoint3d    *pVector1,               /* => first vector */
DPoint3d    *pVector2                /* => second vector */
)
    {
    DPoint3d crossProduct;
    double cross, dot;
    double theta;
    crossProduct.CrossProduct (*pVector1, *pVector2);
    cross   = crossProduct.Magnitude ();
    dot     = pVector1->DotProduct (*pVector2);
    theta = Angle::Atan2 (cross, dot);
    return theta;
    }
#define TOLERANCE_BoundaryCornerAngle           .5

/*---------------------------------------------------------------------------------**//**
* Handles data with sharp corners.
* @bsimethod                                                    Ray.Bentley     10/97
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt bspconv_cubicFitAndReducePoints
(
CurveVectorPtr &curves,
MSBsplineCurve      *curveP,
DPoint3d            *inPointP,
int                 nPoints,
double              tolerance
)
    {
    int             nSegmentPoints;
    StatusInt       status;
    DPoint3d        delta, nextDelta, *startP = NULL, *pointP = inPointP, *endP = pointP + nPoints - 1;
    static double   s_sharpTurnTolerance = TOLERANCE_BoundaryCornerAngle;


    while (pointP < endP)
        {
        nextDelta.DifferenceOf (pointP[ 1], *pointP);
        for (startP = pointP, pointP++; pointP < endP; pointP++)
            {
            delta = nextDelta;
            nextDelta.DifferenceOf (pointP[ 1], *pointP);

            if (angleBetweenVectors (&delta, &nextDelta) > s_sharpTurnTolerance
                || zeroVector (&delta))
                break;
            }

        if (pointP < endP)
            {
#ifdef TODO_EliminateAddressArithmetic
#endif
            if (SUCCESS != (status = appendSmoothSegment (curves, curveP, startP, (int)(pointP - startP + 1), (int)(startP - inPointP), tolerance)))
                return status;

            startP = pointP;
            }
        }
#ifdef TODO_EliminateAddressArithmetic
#endif
    if ((nSegmentPoints = (int)(endP - startP + 1)) > 1)
        {
#ifdef TODO_EliminateAddressArithmetic
#endif
        if (SUCCESS != (status = appendSmoothSegment (curves, curveP, startP, nSegmentPoints, (int)(startP - inPointP), tolerance)))
            return status;
        }
    return SUCCESS;
    }


Public GEOMDLLIMPEXP StatusInt bspconv_cubicFitAndReducePoints
(
MSBsplineCurve      *curveP,
DPoint3d            *inPointP,
int                 nPoints,
double              tolerance
)
    {
    CurveVectorPtr nullCurves;
    StatusInt stat = bspconv_cubicFitAndReducePoints (nullCurves, curveP, inPointP, nPoints, tolerance);
    if (SUCCESS != stat)
        return stat;
    return (0 == curveP->params.numPoles) ? ERROR : SUCCESS;
    }

Public GEOMDLLIMPEXP StatusInt bspconv_cubicFitAndReducePoints
(
CurveVectorPtr &curves,
DPoint3d            *inPointP,
int                 nPoints,
double              tolerance
)
    {
    return bspconv_cubicFitAndReducePoints (curves, NULL, inPointP, nPoints, tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* DavidAssaf 08/04
* @bsimethod                                                    Lu.Han          01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_transformInterpolationCurve
(
MSInterpolationCurve    *outCurveP,         /* <= transformed curve */
MSInterpolationCurve    *inCurveP,          /* => input curve */
Transform const         *transformP         /* => transform */
)
    {
    MSBsplineCurve  curve;
    RotMatrix       matrix;
    DPoint3d        pole;
    StatusInt       status;
    int             i, numPoles;

    if (SUCCESS != (status = bspcurv_copyInterpolationCurve (outCurveP, inCurveP)))
        return status;

    if (!transformP || transformP->IsIdentity ())
        return SUCCESS;

    matrix = RotMatrix::From (*transformP);
    RotMatrix inverseMatrix;
    // Special cases where tangents can be transformed (or where they don't need to be transformed)
    if (outCurveP->params.isPeriodic || inverseMatrix.InverseOf (matrix))
        {
        transformP->Multiply (outCurveP->fitPoints, outCurveP->params.numPoints);

        // we can handle tangents explicitly
        if (!outCurveP->params.isPeriodic)
            {
            matrix.Multiply (outCurveP->startTangent);
            matrix.Multiply (outCurveP->endTangent);
            outCurveP->startTangent.Normalize ();
            outCurveP->endTangent.Normalize ();
            }

        return SUCCESS;
        }

    // General case: Curve will not in general transform to what is expected.  For reason why, see comments
    //      in mdlBspline_openInterpolationCurve and mdlBspline_closeInterpolationCurve.
    // Example: flattening an interpolation curve in a view doesn't match the viewed 3D curve.  See also TR #132976.
    // Best Effort: use endtangents from exact transformed pole-based curve.
    if (SUCCESS != (status = mdlBspline_convertInterpolationToBspline (&curve, outCurveP)))
        return status;

    if (SUCCESS != (status = bspcurv_transformCurve (&curve, &curve, transformP)))
        {
        bspcurv_freeCurve (&curve);
        return status;
        }

    numPoles = curve.params.numPoles;

    // extract geometric end tangents pointing into curve (actual end derivatives may be zero!)
    pole = curve.poles[0];
    outCurveP->startTangent.Zero ();
    for (i = 1; i < numPoles; i++)
        {
            if (!pole.IsEqual (*(&curve.poles[i]), fc_epsilon))
            {
                outCurveP->startTangent.NormalizedDifference (*(&curve.poles[i]), pole);
            break;
            }
        }
    pole = curve.poles[numPoles - 1];
    outCurveP->endTangent.Zero ();
    for (i = numPoles - 2; i >= 0; i--)
        {
            if (!pole.IsEqual (*(&curve.poles[i]), fc_epsilon))
            {
                outCurveP->endTangent.NormalizedDifference (*(&curve.poles[i]), pole);
            break;
            }
        }

    // new interpolation curve will have transformed fitpoints and tangents gleaned from the transformed pole-based equivalent
    transformP->Multiply (outCurveP->fitPoints, outCurveP->params.numPoints);

    bspcurv_freeCurve (&curve);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int     bspcurv_ellipticalArc
(
MSBsplineCurve  *curveP,
DPoint3d        *centerP,
double          x1,
double          x2,
double          start,
double          sweep,
RotMatrix       *rotMatrixP,
bool            close
)
    {
    DPoint3d    center;
    RotMatrix   matrix;

    if (!curveP)
        return ERROR;

    if (close)
        sweep = sweep < 0.0 ? - msGeomConst_2pi  : msGeomConst_2pi ;

    if (centerP)
        center = *centerP;
    else
        center.x = center.y = center.z = 0.0;

    if (rotMatrixP)
        matrix = *rotMatrixP;
    else
        matrix.InitIdentity ();

    return bspconv_arcToCurveStruct (curveP, start, sweep, x1, x2, &matrix, &center);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             07/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspcurv_finiteSpiral
(
MSBsplineCurve  *curve,     /* <= spiral bspline curve */
double          iRad,       /* => initial Radius */
double          fRad,       /* => final Radius */
double          sweep,              /* => angle of spiral */
DPoint3d        *startPt,
DPoint3d        *tangentPt,
DPoint3d        *directionPt
)
    {
    int             i, index, numSections, status;
    double          sectionSweep, sectionDeltaR, sectIRad, sectFRad,
                    wts[7], knots[6], twoWover3, weight;
    DPoint3d        temp;
    DVec3d          poles[7];
    BsplineParam    params;
    RotMatrix       rotMatrix1, rotMatrix2;

    memset (curve, 0, sizeof(*curve));

    numSections = (int) (fabs(sweep)*fc_3/ msGeomConst_2pi ) + 1;
    sectionSweep = sweep / numSections;
    sectionDeltaR = (fRad - iRad)/ numSections;

    /* Allocate the curve */
    curve->rational = curve->display.curveDisplay = true;
    curve->params.order = 4;
    curve->params.numPoles = 3*numSections + 1;
    curve->params.numKnots = 3*(numSections - 1);

    if (SUCCESS != (status = bspcurv_allocateCurve (curve)))
        return status;

    /* Get rotation matrix to rotate unit poles for each section */
    rotMatrix1.InitFromAxisAndRotationAngle (2, sectionSweep);

    /* Get poles of unit circle arc */
    /* This routine assumes that poles, wts, knots are allocated for
        seven poles, third order; also, it returns un(!)-weighted poles */
    if (SUCCESS != (status = bspconv_computeCurveFromArc (poles, knots, wts, &params,
                                                          0.0, sectionSweep, 1.0, 1.0)))
        return status;

    twoWover3 = 2.0 * wts[1] / fc_3;
    weight = (2.0*wts[1] + 1.0) / fc_3;
    poles[1].Scale (poles[1], twoWover3);

    /* Blend to get poles of spiral */
    for (i=0, sectIRad=iRad; i < numSections; i++)
        {
        index = 3*i;
        sectFRad = sectIRad + sectionDeltaR;

        curve->poles[index].Scale (poles[0], sectIRad);

        curve->poles[index+1].Scale (poles[1], sectIRad);
        temp.Scale (poles[0], sectFRad/fc_3);
        curve->poles[index+1].SumOf (*(&curve->poles[index+1]), temp);

        curve->poles[index+2].Scale (poles[2], sectIRad/fc_3);
        temp.Scale (poles[1], sectFRad);
        curve->poles[index+2].SumOf (*(&curve->poles[index+2]), temp);

        curve->weights[index] = 1.0;
        curve->weights[index+1] = curve->weights[index+2] = weight;

        if (i)  /* Set internal knots (remember to offset by order=4) */
            {
            curve->knots[index+1] = curve->knots[index+2] =
            curve->knots[index+3] = (double) i / numSections;
            }

        sectIRad = sectFRad;
        rotMatrix1.Multiply (poles, poles, 3);
        }

    curve->poles[3*numSections].Scale (poles[0], fRad);
    curve->weights[3*numSections] = 1.0;
    curve->knots[0] = curve->knots[1] =
    curve->knots[2] = curve->knots[3] = 0.0;
    index = bspknot_numberKnots (curve->params.numPoles, 4, false);
    curve->knots[index-1] = curve->knots[index-2] =
    curve->knots[index-3] = curve->knots[index-4] = 1.0;

    if (!(startPt && tangentPt && directionPt))
        return SUCCESS;

    /* Position and orient spiral */
    bsputil_unWeightPoles (curve->poles, curve->poles, curve->weights,
                           curve->params.numPoles);
    bsiDPoint3d_subtractDPoint3dArray (curve->poles+1, curve->poles, curve->params.numPoles-1);
    curve->poles[0].x = curve->poles[0].y = curve->poles[0].z = 0.0;

    /* Orthogonal system from spiral poles, x-axis = tangent at zero */
    poles->NormalizedDifference (*(curve->poles+1), *(curve->poles));
    poles[1].DifferenceOf (*(curve->poles+2), *(curve->poles));
    poles[2].CrossProduct (*poles, poles[1]);
    poles[2].Normalize ();
    poles[1].CrossProduct (poles[2], *poles);
    rotMatrix1.InitFromColumnVectors (*poles, poles[1], poles[2]);

    /* Orthogonal system from input data points, x-axis = tangentPt-startPt */
    poles->NormalizedDifference (*tangentPt, *startPt);
    poles[1].DifferenceOf (*directionPt, *startPt);
    poles[2].CrossProduct (*poles, poles[1]);
    poles[2].Normalize ();
    poles[1].CrossProduct (poles[2], *poles);
    rotMatrix2.InitFromRowVectors (*poles, poles[1], poles[2]);

    rotMatrix1.InitProduct (rotMatrix1, rotMatrix2);
    rotMatrix1.MultiplyTranspose (curve->poles, curve->poles, curve->params.numPoles);
    DPoint3d::AddToArray (curve->poles, curve->params.numPoles, *startPt);

    bsputil_weightPoles (curve->poles, curve->poles, curve->weights, curve->params.numPoles);

    return SUCCESS;
    }


#if defined (infinite_radii)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             07/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspcurv_infiniteSpiral
(
MSBsplineCurve  *curve,     /* <= spiral bspline curve */
double          fRad,       /* => final Radius */
double          sweep,              /* => angle of spiral */
DPoint3d        *startPt,
DPoint3d        *tangentPt,
DPoint3d        *endPt
)
    {
    int         status;
    double      dist, area;
    DPoint3d    norm, leg0, leg1;

    if (sweep > fc_180)
        return (ERROR);

    memset (curve, 0, sizeof(*curve));

    /* Allocate the curve */
    curve->rational = curve->display.curveDisplay = true;
    curve->params.order = curve->params.numPoles = 4;

    if (SUCCESS != (status = bspcurv_allocateCurve (curve)))
        return status;
    curve->poles[0] = *startPt;
    curve->poles[2] = *tangentPt;
    curve->poles[3] = *endPt;

/*    norm.NormalizedDifference (*tangentPt, *startPt);
    dist = diff.NormalizedDifference (*endPt, *startPt);
    dot = norm.DotProduct (diff);
    alpha = acos (dot);
    scale = dist / sin ( msGeomConst_pi  - sweep) * sin (sweep - alpha);

    (curve->poles+2)->SumOf (*startPt, norm, scale);*/

    curve->poles[1].x = (curve->poles[0].x + curve->poles[2].x) / 2.0;
    curve->poles[1].y = (curve->poles[0].y + curve->poles[2].y) / 2.0;
    curve->poles[1].z = (curve->poles[0].z + curve->poles[2].z) / 2.0;

    leg0.DifferenceOf (*(curve->poles+1), *(curve->poles+2));
    leg1.DifferenceOf (*(curve->poles+3), *(curve->poles+2));
    dist = leg1.Magnitude ();
    norm.CrossProduct (leg0, leg1);
    area = norm.Magnitude ();

    curve->weights[0] = curve->weights[2] = curve->weights[3] = 1.0;
    curve->weights[1] = fc_3/fc_4 * dist * dist * dist / area / fRad;
    bsputil_weightPoles (curve->poles, curve->poles, curve->weights, 4);

    curve->knots[0] = curve->knots[1] =
    curve->knots[2] = curve->knots[3] = 0.0;
    curve->knots[4] = curve->knots[5] =
    curve->knots[6] = curve->knots[7] = 1.0;

    return SUCCESS;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             07/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_spiral
(
MSBsplineCurve  *curve,     /* <= spiral bspline curve */
double          iRad,       /* => initial Radius */
double          fRad,       /* => final Radius */
double          sweep,              /* => angle of spiral */
DPoint3d        *startPt,
DPoint3d        *tangentPt,
DPoint3d        *directionPt
)
    {
#if defined (infinite_radii)
    if (iRad < 0.0)
        {
        if (fRad < 0.0)
            return (ERROR);
        else
            return bspcurv_infiniteSpiral (curve, fRad, sweep, startPt, tangentPt,
                                           directionPt);
        }
    else if (fRad < 0.0)
        {
        bspcurv_infiniteSpiral (curve, iRad, sweep, startPt, tangentPt, directionPt);
        return bspcurv_reverseCurve (curve, curve);
        }
#else
    if (iRad < 0.0 || fRad < 0.0)
        return (ERROR);
#endif
    else
        {
        return bspcurv_finiteSpiral (curve, iRad, fRad, sweep, startPt, tangentPt,
                                     directionPt);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             10/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_helix
(
MSBsplineCurve  *curve,                /* <= spiral bspline curve */
double          iRad,                  /* => initial Radius */
double          fRad,                  /* => final Radius */
double          pitchValue,            /* => if valueIsHeight != 0, pitchValue is the axis advance per 360 degrees. 
                                              if valueIsHeight == 0, pitchValue is the number of 360 degree turns in the distance from startCenter to endCenter */
DPoint3d        *xAxisTargetPoint,
DPoint3d        *axisStartPoint,
DPoint3d        *axisTargetPoint,
int             valueIsHeight
)
    {
    int         i, status;
    double      sweep, height, turns;
    DVec3d      xDir, yDir, zDir;
    DPoint3d     *pP, *endP;
    RotMatrix   rotMatrix;

    height = axisStartPoint->Distance (*axisTargetPoint);
    turns = valueIsHeight ? height/pitchValue : pitchValue;
    sweep = msGeomConst_2pi  * turns;

    /* Generate a spiral centered at the origin */
    if (SUCCESS != (status = bspcurv_spiral (curve, iRad, fRad, sweep, NULL, NULL, NULL)))
        return status;

    /* Orient and position the helix */
    bsputil_unWeightPoles (curve->poles, curve->poles, curve->weights,
                           curve->params.numPoles);

    for (i=0, pP=endP=curve->poles, endP += curve->params.numPoles; pP < endP; pP++, i++)
        pP->z = i * height / (curve->params.numPoles - 1);

    xDir.NormalizedDifference (*xAxisTargetPoint, *axisStartPoint);
    zDir.NormalizedDifference (*axisTargetPoint, *axisStartPoint);
    yDir.CrossProduct (zDir, xDir);
    rotMatrix.InitFromRowVectors (xDir, yDir, zDir);

    rotMatrix.MultiplyTranspose (curve->poles, curve->poles, curve->params.numPoles);
    DPoint3d::AddToArray (curve->poles, curve->params.numPoles, *axisStartPoint);

    bsputil_weightPoles (curve->poles, curve->poles, curve->weights, curve->params.numPoles);

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* pure 3D transform
* @bsimethod                                                    BFP             09/91
+---------------+---------------+---------------+---------------+---------------+------*/
static void bsputil_rotatePoint
(
DPoint3d    *point,
RotMatrix   *rmatrix
)
    {
    DPoint3d    oldPoint;

    oldPoint = *point;
    point->x =  oldPoint.x * rmatrix->form3d[0][0] +
                oldPoint.y * rmatrix->form3d[0][1] +
                oldPoint.z * rmatrix->form3d[0][2];
    point->y =  oldPoint.x * rmatrix->form3d[1][0] +
                oldPoint.y * rmatrix->form3d[1][1] +
                oldPoint.z * rmatrix->form3d[1][2];
    point->z =  oldPoint.x * rmatrix->form3d[2][0] +
                oldPoint.y * rmatrix->form3d[2][1] +
                oldPoint.z * rmatrix->form3d[2][2];
    }

/*---------------------------------------------------------------------------------**//**
* pure 3D transform
* @bsimethod                                                    BFP             09/91
+---------------+---------------+---------------+---------------+---------------+------*/
static void bsputil_unrotatePoint
(
DPoint3d        *point,
RotMatrix       *rmatrix
)
    {
    DPoint3d    oldPoint;

    oldPoint = *point;
    point->x =  oldPoint.x * rmatrix->form3d[0][0] +
                oldPoint.y * rmatrix->form3d[1][0] +
                oldPoint.z * rmatrix->form3d[2][0];
    point->y =  oldPoint.x * rmatrix->form3d[0][1] +
                oldPoint.y * rmatrix->form3d[1][1] +
                oldPoint.z * rmatrix->form3d[2][1];
    point->z =  oldPoint.x * rmatrix->form3d[0][2] +
                oldPoint.y * rmatrix->form3d[1][2] +
                oldPoint.z * rmatrix->form3d[2][2];
    }

/*---------------------------------------------------------------------------------**//**
* pure 3D transform
* @bsimethod                                                    BFP             09/91
+---------------+---------------+---------------+---------------+---------------+------*/
static void bsputil_arbitraryTransform
(
RotMatrix   *trans,
DPoint3d    *zAxis
)
    {
    int     i;
    double      *xvec, *yvec, *zvec;
    DPoint3d    world, xNormal, yNormal, zNormal;

    world.Zero ();
    zNormal = *zAxis;
    zNormal.Normalize ();
    if ((fabs (zNormal.x) < fc_p01) && (fabs (zNormal.y) < fc_p01))
        world.y = 1.0;
    else
        world.z = 1.0;

    xNormal.CrossProduct (world, zNormal);
    xNormal.Normalize ();
    yNormal.CrossProduct (zNormal, xNormal);
    yNormal.Normalize ();

    xvec = (double *) &xNormal;
    yvec = (double *) &yNormal;
    zvec = (double *) &zNormal;
    for (i=0; i<3; i++)
        {
        trans->form3d[0][i] = xvec[i];
        trans->form3d[1][i] = yvec[i];
        trans->form3d[2][i] = zvec[i];
        }
    }

/*---------------------------------------------------------------------------------**//**
* mirrors curve in plane perpendicular to end tangent
* @bsimethod                                                    BFP             09/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspcurv_mirrorCurve
(
MSBsplineCurve  *out,
MSBsplineCurve  *in,
int             aboutZero
)
    {
    int         status, incr;
    DPoint3d    *pP, *first, *second, *last, tmp;
    RotMatrix   rM;

    if (SUCCESS != (status = bspcurv_copyCurve (out, in)))
        return status;

    if (aboutZero)
        {
        first  = out->poles;
        second = first + 1;
        last   = out->poles + out->params.numPoles - 1;
        incr   = 1;
        }
    else
        {
        first  = out->poles + out->params.numPoles - 1;
        second = first - 1;
        last   = out->poles;
        incr   = -1;
        }

    tmp.DifferenceOf (*second, *first);
    bsputil_arbitraryTransform (&rM, &tmp);

    for (pP=second; pP != last; pP += incr)
        {
        tmp.DifferenceOf (*pP, *first);
        bsputil_rotatePoint (&tmp, &rM);
        tmp.z *= -1.0 ;
        bsputil_unrotatePoint (&tmp, &rM);
        pP->SumOf (tmp, *first);
        }

    tmp.DifferenceOf (*last, *first);
    bsputil_rotatePoint (&tmp, &rM);
    tmp.z *= -1.0 ;
    bsputil_unrotatePoint (&tmp, &rM);
    last->SumOf (tmp, *first);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Return a curve that blends from inCurve1 at param1 to inCurve2 at param2 with specified degree of continuity
* @bsimethod                                                    BFP             08/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_blendCurve
(
MSBsplineCurve  *blend,        /* <= resulting curve */
MSBsplineCurve  *inCurve1,     /* => curve to blend */
MSBsplineCurve  *inCurve2,     /* => curve to blend */
double          param1,        /* => blend curve1 from here */
double          param2,        /* => blend curve2 to here */
int             degree,        /* => degree of continuity desired */
double          mag1,          /* => relative magnitude of blend  */
double          mag2           /*    tangent between 0.0 and 1.0 */
)
    {
    int             status, necessaryDegree, copySize, offset;
    double          segTanMag, scaleFactor, tmp;
    DPoint3d        tan, segTan, origin, *pP, *endP;
    MSBsplineCurve  seg[2];

    memset (seg, 0, sizeof(seg));
    memset (blend, 0, sizeof(*blend));

    if (degree < 0 || degree > MAX_ORDER - 1)
        return ERROR;

    if (param1 >= 1.0)
        {
        if (SUCCESS != (status = bspcurv_mirrorCurve (seg, inCurve1, false)) ||
            SUCCESS != (status = bspcurv_reverseCurve (seg, seg)))
            goto wrapup;
        }
    else
        {
        if (SUCCESS != (status = bspcurv_segmentCurve (seg, inCurve1, param1, 1.0)))
            goto wrapup;
        }

    if (SUCCESS != (status = bspcurv_getTangent (&tan, param1, inCurve1, false)) ||
        SUCCESS != (status = bspcurv_getTangent (&segTan, 0.0, seg, false)))
        goto wrapup;

    if ((segTanMag = segTan.Magnitude ()) > fc_epsilon)
        scaleFactor = mag1 * tan.Magnitude () / segTanMag;
    else
        scaleFactor = mag1 * tan.Magnitude ();
    if ((tmp = 1.0 - param1) > 0.0)
        scaleFactor /= tmp;
    if (seg[0].rational)
        bsputil_unWeightPoles (seg[0].poles, seg[0].poles, seg[0].weights,
                                  seg[0].params.numPoles);
    origin = seg[0].poles[0];
    for (pP=seg[0].poles+1, endP=seg[0].poles + seg[0].params.numPoles; pP < endP; pP++)
        {
        pP->DifferenceOf (*pP, origin);
        pP->Scale (*pP, scaleFactor);
        pP->SumOf (*pP, origin);
        }
    if (seg[0].rational)
        bsputil_weightPoles (seg[0].poles, seg[0].poles, seg[0].weights,
                             seg[0].params.numPoles);

    if (param2 <= 0.0)
        {
        if (SUCCESS != (status = bspcurv_mirrorCurve (seg + 1, inCurve2, true)) ||
            SUCCESS != (status = bspcurv_reverseCurve (seg + 1, seg + 1)))
            goto wrapup;
        }
    else
        {
        if (SUCCESS != (status = bspcurv_segmentCurve (seg+1, inCurve2, 0.0, param2)))
            goto wrapup;
        }

    if (SUCCESS != (status = bspcurv_getTangent (&tan, param2, inCurve2, false)) ||
        SUCCESS != (status = bspcurv_getTangent (&segTan, 1.0, seg + 1, true)))
        goto wrapup;
    if ((segTanMag = segTan.Magnitude ()) > fc_epsilon)
        scaleFactor = mag2 * tan.Magnitude () / segTanMag;
    else
        scaleFactor = mag2 * tan.Magnitude ();
    if (param2 > 0.0)
        scaleFactor /= param2;

    if (seg[1].rational)
        bsputil_unWeightPoles (seg[1].poles, seg[1].poles, seg[1].weights,
                               seg[1].params.numPoles);
    offset = seg[1].params.numPoles - 1;
    origin = seg[1].poles[offset];
    for (pP=endP=seg[1].poles, endP += offset; pP < endP; pP++)
        {
        pP->DifferenceOf (*pP, origin);
        pP->Scale (*pP, scaleFactor);
        pP->SumOf (*pP, origin);
        }
    if (seg[1].rational)
        bsputil_weightPoles (seg[1].poles, seg[1].poles, seg[1].weights,
                             seg[1].params.numPoles);

    blend->rational = seg[0].rational || seg[1].rational;
    blend->params.order = degree + 2;
    necessaryDegree = degree + 1;
    if (seg[0].params.order < blend->params.order &&
        SUCCESS != (status = bspcurv_elevateDegree (seg, seg, necessaryDegree)))
        goto wrapup;
    if (seg[1].params.order < blend->params.order &&
        SUCCESS != (status = bspcurv_elevateDegree (seg+1, seg+1, necessaryDegree)))
        goto wrapup;

    if (blend->rational &&
        (SUCCESS != (status = bspcurv_makeRational (seg, seg)) ||
         SUCCESS != (status = bspcurv_makeRational (seg+1, seg+1))))
        goto wrapup;

    blend->params.numPoles = 2 * necessaryDegree;

    if (SUCCESS != (status = bspcurv_allocateCurve (blend)))
        goto wrapup;

    copySize = necessaryDegree * sizeof(DPoint3d);
    offset = seg[1].params.numPoles - necessaryDegree;
    memcpy (blend->poles, seg[0].poles, copySize);
    memcpy (blend->poles + necessaryDegree, seg[1].poles + offset, copySize);
    if (blend->rational)
        {
        copySize = necessaryDegree * sizeof(double);
        memcpy (blend->weights, seg[0].weights, copySize);
        memcpy (blend->weights + necessaryDegree, seg[1].weights + offset, copySize);
        }
    blend->display.curveDisplay = true;
    bspknot_computeKnotVector (blend->knots, &blend->params, NULL);

wrapup:
    bspcurv_freeCurve (seg);
    bspcurv_freeCurve (seg+1);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_leastSquaresToCurve
(
MSBsplineCurve  *curve,
double          *avgDistance,
double          *maxDistance,
DPoint3d        *pnts,
double          *uValues,
int             numPnts
)
    {
    int         left, row, col, i, j, k, numPoles, order, numPoints,
                allocSize, status=SUCCESS;
    double      *u, *matrix, *distance, *rhs, *rhsP0, *rhsP1, *rhsP2, *dP, *endD,
                bfuncs[MAX_ORDER], totalDistance, divisor;
    DPoint3d    temp, *points, *pP, *endP;

    u = matrix = distance = rhs = NULL; points = NULL;
    numPoles = curve->params.numPoles;
    order = curve->params.order;

    /* Check ok params for number of data points */
    if (numPoles < order)
        return (ERROR);
    if (numPnts < numPoles)
        return (ERROR);
    if (curve->params.numKnots && !curve->knots)
        return (ERROR);

    /* Allocate memory */
    allocSize = (numPnts + (curve->params.closed ? 1 : 0)) * sizeof(DPoint3d);
    if (NULL == (points = static_cast<DPoint3d *>(msbspline_malloc (allocSize, HEAPSIG_BCRV))))
        return ERROR;

    /* Get local copy of data points, removing repeated points to keep matrix good */
    points[0] = pnts[0];
    for (numPoints=1, pP=pnts+1; pP < pnts+numPnts; pP++)
        if (!bsputil_isSamePoint (points+numPoints-1, pP))
            points[numPoints++] = *pP;

    if (numPoints < numPoles)   /* not enough distinct data points */
        {
        msbspline_free (points);
        return (ERROR);
        }
    if (curve->params.closed && !bsputil_isSamePoint (points, points+numPoints-1))
        points[numPoints++] = points[0];

    status = ERROR;
    allocSize = numPoles * sizeof(DPoint3d);
    if (NULL == (curve->poles = static_cast<DPoint3d *>(BSIBaseGeom::Malloc (allocSize))))
        goto wrapup;
    memset (curve->poles, 0, allocSize);

    if (NULL == (rhs = static_cast<double *>(msbspline_malloc (allocSize, HEAPSIG_BCRV))))
        goto wrapup;

    allocSize = numPoles * numPoles * sizeof(double);
    if (NULL == (matrix = static_cast<double *>(msbspline_malloc (allocSize, HEAPSIG_BCRV))))
        goto wrapup;
    memset (matrix, 0, allocSize);

    allocSize = numPoints * sizeof(double);
    if (NULL == (u = static_cast<double *>(msbspline_malloc (allocSize, HEAPSIG_BCRV))))
        goto wrapup;

    /* Allocate memory and compute knot vector if it is not supplied */
    if (!curve->knots)
        {
        allocSize= bspknot_numberKnots (numPoles, order, curve->params.closed);
        if (NULL == (curve->knots = static_cast<double *>(BSIBaseGeom::Malloc (allocSize * sizeof (double)))))
            goto wrapup;
        bspknot_computeKnotVector (curve->knots, &curve->params, NULL);
        }

    /* If uValues is passed use it, else calculate parameter values for
        approximation points; i.e. normalized cumulative length */
    if (uValues)
        {
        memcpy (u, uValues, numPnts * sizeof (double));
        if (curve->params.closed)
            u[numPoints-1] = 1.0;
        }
    else
        {
        for (i=1, u[0] = 0.0; i < numPoints; i++)
            u[i] =  u[i-1] + points[i].Distance (points[i-1]);
        divisor = u[numPoints-1];
        for (dP=endD=u, endD += numPoints; dP < endD; dP++)
            *dP /= divisor;
        }

    /* Construct system of equations to be solved */
    /* If open curve, interpolate end points */
    if (!curve->params.closed)
        {
        matrix[0] = matrix[numPoles*numPoles-1] = 1.0;
        curve->poles[0] = points[0];
        curve->poles[numPoles-1] = points[numPoints-1];
        }

    for (i=0; i < numPoints; i++)
        {
        /* Calculate blending functions at u[i] */
        bsputil_computeBlendingFunctions (bfuncs, NULL, &left, curve->knots,
                                      u[i], curve->params.numPoles, order, curve->params.closed);

        /* Add to equations remembering that only order bfuncs are non-zero */
        for (j=0, row = (left - order + numPoles) % numPoles; j < order; j++, row++)
            {
            if (curve->params.closed || (0 < row && row < numPoles-1))
                {
                temp.Scale (points[i], bfuncs[j]);
                curve->poles[row%numPoles].SumOf (*(&curve->poles[row%numPoles]), temp);
                for (k=0, col = (left - order + numPoles) % numPoles;
                     k < order; k++, col++)
                    matrix[row%numPoles*numPoles + col%numPoles] += bfuncs[j] * bfuncs[k];
                }
            }
        }

    /* Solve system of equations */
    for (rhsP0=rhs, rhsP1=rhs+numPoles, rhsP2=rhsP1+numPoles,
         pP=endP=curve->poles, endP += numPoles;
         pP < endP; rhsP0++, rhsP1++, rhsP2++, pP++)
        {
        *rhsP0 = pP->x;    *rhsP1 = pP->y;    *rhsP2 = pP->z;
        }

    if (SUCCESS != (status = (bsiLinAlg_solveLinearGaussPartialPivot (matrix, numPoles, rhs, 3) ? SUCCESS : ERROR)))
        goto wrapup;

    for (rhsP0=rhs, rhsP1=rhs+numPoles, rhsP2=rhsP1+numPoles,
         pP=endP=curve->poles, endP += numPoles;
         pP < endP; rhsP0++, rhsP1++, rhsP2++, pP++)
        {
        pP->x = *rhsP0;    pP->y = *rhsP1;    pP->z = *rhsP2;
        }

    /* Calculate error if requested */
    if (maxDistance || avgDistance)
        {
        allocSize = numPoints * sizeof(double);
        if (NULL == (distance = static_cast<double *>(msbspline_malloc (allocSize, HEAPSIG_BCRV))))
            {
            status = ERROR;
            goto wrapup;
            }
        for (i=0, totalDistance = 0.0; i < numPoints; i++)
            {
            bsputil_computeBlendingFunctions (bfuncs, NULL, &left, curve->knots,
                                          u[i], curve->params.numPoles, order, curve->params.closed);
            temp.x = temp.y = temp.z = 0.0;
            for (j=0, k = (left - order + numPoles) % numPoles;
                 j < order; j++, k++)
                {
                temp.x += bfuncs[j] * curve->poles[k % numPoles].x;
                temp.y += bfuncs[j] * curve->poles[k % numPoles].y;
                temp.z += bfuncs[j] * curve->poles[k % numPoles].z;
                }
            totalDistance += distance[i] = points[i].Distance (temp);
            }

        for (dP=endD=distance, endD += numPoints, *maxDistance=0.0; dP < endD; dP++)
            if (*dP > *maxDistance)     *maxDistance = *dP;
        *avgDistance = totalDistance / (numPoints-1);
        }

    /* Return B-Spline curve */
    curve->type = BSCURVE_GENERAL;
    curve->rational = false;
    curve->display.curveDisplay = true;
    curve->weights = NULL;

wrapup:
    if (points)         msbspline_free (points);
    if (u)              msbspline_free (u);
    if (matrix)         msbspline_free (matrix);
    if (distance)       msbspline_free (distance);
    if (rhs)            msbspline_free (rhs);
    return status;
    }
END_BENTLEY_GEOMETRY_NAMESPACE