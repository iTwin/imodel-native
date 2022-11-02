/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bspcurv_computeLength
(
MSBsplineCurve  *curveP,
double          tolerance
)
    {
    double      length = 0.0;
    bspcurv_arcLength (&length, NULL, 0.0, 1.0, curveP, NULL, 0.0);
    return length;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     refineSegmentArcLengthFraction
(
BCurveSegmentR  segment,
double          &fraction,      // on input, estimated fractional position of length.
                                // on output, improved estimate
double          &absLengthError,
double          targetLength,
double          totalLength,     // precomputed total length of this bezier.
double          lengthTol,
double          fractionTol
)
    {
    absLengthError = 0.0;
    if (targetLength < 0.0)
        return false;
    if (targetLength == 0.0)
        {
        fraction = 0.0;
        return true;
        }

    static size_t maxStep = 20;
    int numConverged = 0;
    static int sNumConvergedNeeded = 2;

    for (size_t numStep = 0; numStep < maxStep; numStep++)
        {
        double newLength;
        DPoint3d point;
        DVec3d tangent;
        segment.Length (newLength, 0.0, fraction);
        segment.FractionToPoint (point, tangent, fraction, false);
        double a = tangent.Magnitude ();
        double du;
        double signedLengthError = targetLength - newLength;
        absLengthError = fabs (signedLengthError);
        if (!DoubleOps::SafeDivideParameter (du, signedLengthError, a, 0.0))
            return false;
        if (absLengthError < lengthTol)
            {
            if (++numConverged >= sNumConvergedNeeded)
                return true;
            }
        else if (fabs (du) < fractionTol)
            {
            if (++numConverged >= sNumConvergedNeeded)
                return true;
            }
        else
            numConverged = 0;
        fraction += du;
        }
    return false;
    }


/*----------------------------------------------------------------------+
For internal use only.
ASSUME tolerance is computed.
ASSUME targetLength is positive.
ASSUME paramB > paramA
ASSUME paramP pointer is valid.
+----------------------------------------------------------------------*/
static int      bspcurv_parameterFromArcLengthForward
(
double          &finalParam,            /* <= fraction parameter to match the arc length */
double          targetLength,          /* => from start parameter to paramP */
double          paramA,             /* => lower fraction parameter of live interval */
double          paramB,             /* => upper fraction parameter of live interval */
MSBsplineCurve  *curveP,            /* => input curve */
double          distanceAB,         /* => precomputed distance from paramA to paramB */
double          lengthTol          /* => length tolerance for intergration */
)
    {
    BCurveSegment segment;
    static double s_bezierFractionTol = 1.0e-10;
    double remainingLength = targetLength;
    double lengthError;
    DRange1d interval = DRange1d::From (curveP->FractionToKnot  (paramA),
                        curveP->FractionToKnot (paramB));
    for (size_t i = 0; curveP->AdvanceToBezierInKnotInterval (segment, i, interval);)
        {
        double bezierLength;
        segment.Length (bezierLength, 0.0, 1.0);
        if (bezierLength < remainingLength)
            {
            remainingLength -= bezierLength;
            }
        else
            {
            double fraction = remainingLength / bezierLength;
            if (refineSegmentArcLengthFraction (segment, fraction, lengthError, remainingLength, bezierLength,
                    lengthTol, s_bezierFractionTol))
                {
                double finalKnot = segment.FractionToKnot (fraction);
                finalParam = curveP->KnotToFraction (finalKnot);
                return SUCCESS;
                }
            }

        }
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int      bspcurv_parameterFromArcLength
(
DPoint3d        *pointP,            /* <= point of paramP */
double          *paramP,            /* <= parameter to match the arc length */
double          *errorAchievedP,    /* <= the absolute error achieved */
double          targetLength,          /* => from start parameter to paramP */
double          startParam,         /* => fractional start parameter */
MSBsplineCurve  *curveP,            /* => input curve */
double          *totalLengthP,      /* => NOT USED */
double          relativeTol,        /* => relative tolerance for integration */
double          paramTol            /* => parameter tolerance for iteration */
)
    {
    double distanceAB;
    double polygonLength;
    double distanceTol;
    StatusInt status = ERROR;
    double errorEstimate;
    double resultParam;
    if (relativeTol < 1.0e-10)
        relativeTol = 1.0e-10;

    errorEstimate = polygonLength = bspcurv_polygonLength (curveP);
    distanceTol = relativeTol * polygonLength;

    if (fabs (targetLength) < distanceTol)
        {
        resultParam = startParam;
        status = SUCCESS;
        }
    else if (targetLength >= 0.0)
        {
        distanceAB = curveP->LengthBetweenFractions (startParam, 1.0);
        if (distanceAB < 0.0)
            return ERROR;
        if (fabs (targetLength - distanceAB) <= distanceTol)
            {
            resultParam = 1.0;
            status = SUCCESS;
            }
        else if (targetLength > distanceAB)
            {
            resultParam = 1.0;
            errorEstimate = distanceAB - targetLength;
            status = SUCCESS;
            }
        else
            {
            status = bspcurv_parameterFromArcLengthForward (resultParam, targetLength,
                        startParam, 1.0,
                        curveP, distanceAB, distanceTol);
            }
        }
    else
        {
        double targetFromStart;
        distanceAB = curveP->LengthBetweenFractions (0.0, startParam);
        if (distanceAB < 0.0)
            return ERROR;
        targetFromStart = distanceAB + targetLength;
        if (fabs(targetFromStart) <= distanceTol)
            {
            resultParam = 0.0;
            status = SUCCESS;
            }
        else if (targetFromStart < 0.0)
            {
            resultParam = 0.0;
            errorEstimate = fabs (targetFromStart);
            status = SUCCESS;
            }
        else
            {
            status = bspcurv_parameterFromArcLengthForward (resultParam, targetFromStart,
                        0.0, startParam,
                        curveP, distanceAB, relativeTol);
            }
        }

    // We always have a result parameter to evaluate.
    if (paramP)
        *paramP = resultParam;

    if (pointP)
        bspcurv_evaluateCurvePoint  (pointP, NULL, curveP, resultParam);

    if (errorAchievedP)
        *errorAchievedP = errorEstimate;
    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_arcLengthFromParameters
(
double          *arcLengthP,        /* <= arc length */
double          *errorAchievedP,     /* <= the absolute error achieved, i.e.,
                                          abs(arcLengthP - trueArcLength) */
double          startParam,         /* => strating parameter */
double          endParam,           /* => ending parameter */
MSBsplineCurve  *curveP,            /* => input curve */
double          relativeTol         /* => relative tolerance for integration
                                          using Simpson's Rule, i.e.,
                                          abs(errorAchivedP) / arcLengthP <=
                                          relativeTol */
)
    {
    return bspcurv_arcLength (arcLengthP, errorAchievedP,
                    startParam, endParam, curveP, NULL,
                    Angle::SmallAngle ());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_arcLength
(
double          *fP,                /* <= integral */
double          *errorAchievedP,     /* <= the absolute error achieved, i.e.,
                                          abs(arcLengthP - trueArcLength) */
double          startFraction,      /* => strating parameter */
double          endFraction,        /* => ending parameter */
MSBsplineCurve  *curveP,            /* => input curve */
const DPoint3d  *originP,           /* => origin for sweeping ray */
double          relativeTol         /* => relative tolerance for integration
                                          using Simpson's Rule, i.e.,
                                          abs(errorAchivedP) / arcLengthP <=
                                          relativeTol */
)
    {
    DPoint3d origin;
    if (NULL == originP)
        origin.zero();
    else
        origin = *originP;
    static double s_minimumArcLength = 1.0;
    relativeTol = bspcurv_fixRelTol (relativeTol);

    return bspcurv_integrateTangentialFunction (fP, errorAchievedP, startFraction, endFraction, curveP,
                                                1,
                                                bspcurv_sweepFunc,
                                                recursiveSimpsonIntegrationExt,
                                                &origin,
                                                ArcLength,
                                                relativeTol * s_minimumArcLength,
                                                relativeTol);
    }
    
END_BENTLEY_GEOMETRY_NAMESPACE
