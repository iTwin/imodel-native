/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bsprange.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define BIG_DOUBLE 1.0e100
static void extendRangePoints
(
DRange3d *pRange,
DPoint3d *pPointArray,
double *pWeightArray,
int numPole
)
    {
    int i;
    DPoint3d newPoint;
    for (i = 0; i < numPole; i++)
        {
        if (pWeightArray)
            {
            newPoint.x = pPointArray[i].x / pWeightArray[i];
            newPoint.y = pPointArray[i].y / pWeightArray[i];
            newPoint.z = pPointArray[i].z / pWeightArray[i];
            }
        else
            newPoint = pPointArray[i];

        bsiDRange3d_extendByDPoint3d (pRange, &newPoint);
        }
    }

/*--------------------------------------------------------------------*//**
* @description Compute range of the poles of a surface
* @param pRange <= computed range.
* @param pSurface => surface
*------------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_poleRange
(
DRange3d *pRange,
MSBsplineSurface *pSurface
)
    { 
    int numPole = pSurface->uParams.numPoles * pSurface->vParams.numPoles;
    bsiDRange3d_init (pRange);
    extendRangePoints (pRange, pSurface->poles, pSurface->weights, numPole);
    }

/*--------------------------------------------------------------------*//**
* @description Compute range of the poles of a curve.
* @param pRange <= computed range.
* @param pCurve => curve
*------------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspcurv_poleRange
(
DRange3d            *pRange,
MSBsplineCurveCP    pCurve
)
    {
    int numPole = pCurve->params.numPoles;
    bsiDRange3d_init (pRange);
    extendRangePoints (pRange, pCurve->poles, pCurve->weights, numPole);
    }

#define UOR_RESOLUTION      1.0
#define SMALLEST_ALLOWED_REFERENCE_SIZE 1.0
#define RELATIVE_RESOLUTION 1.0e-8
#define TANGENT_CONTINUITY  1

/*----------------------------------------------------------------------+
Return larger of
 (1) abstol
 (2) larger of relTol or RELATIVE_RESOLUTION times
          larger of dataSize or SMALLEST_ALLOWED_REFERENCE_SIZE
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double  bsputil_sizeToTol
(
double dataSize,
double absTol,
double relTol
)
    {
    double tol;
    dataSize = fabs (dataSize);
    if (dataSize < SMALLEST_ALLOWED_REFERENCE_SIZE)
        dataSize = SMALLEST_ALLOWED_REFERENCE_SIZE;
    if (relTol < RELATIVE_RESOLUTION)
        relTol = RELATIVE_RESOLUTION;
    tol = dataSize * relTol;
    if (tol < absTol)
        tol = absTol;
    return tol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double  expandMaxAbsDoubleArray
(
double *pData,
int     count,
double  dMax
)
    {
    int  i;
    double dCurr;
    for (i = 0; i < count; i++)
        {
        dCurr = fabs (pData[i]);
        if (dCurr > dMax)
            dMax = dCurr;
        }
    return dMax;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bspcurv_getCurveTol
(
MSBsplineCurve const *pCurve,                /* => b-spline curve */
double absTol,
double relTol
)
    {
    return bsputil_sizeToTol (
                expandMaxAbsDoubleArray
                    (
                    (double *)pCurve->poles,
                    3 * pCurve->params.numPoles,
                    0.0
                    ),
                absTol,
                relTol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double  bspcurv_getResolution
(
MSBsplineCurve const *pCurve                /* => b-spline curve */
)
    {
    return bsputil_sizeToTol (
                expandMaxAbsDoubleArray
                    (
                    (double *)pCurve->poles,
                    3 * pCurve->params.numPoles,
                    0.0
                    ),
                0.0,
                0.0);
    }

Public GEOMDLLIMPEXP double  MSBsplineCurve::Resolution (double abstol, double reltol) const
    {
    return bsputil_sizeToTol (
                expandMaxAbsDoubleArray
                    (
                    (double *)poles,
                    3 * params.numPoles,
                    0.0
                    ),
                abstol,
                reltol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    earlin.lutz                     03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineSurface::Resolution () const
    {
    return bspsurf_getResolution (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    earlin.lutz                     03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineSurface::Resolution (double abstol, double reltol) const
    {
    return bsputil_sizeToTol (
                expandMaxAbsDoubleArray
                    (
                    (double*)poles,
                    3 * uParams.numPoles * vParams.numPoles,
                    SMALLEST_ALLOWED_REFERENCE_SIZE
                    ),
                abstol,
                reltol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double  bspsurf_getResolution
(
MSBsplineSurface const *pSurface          /* => b-spline surface */
)
    {
    return bsputil_sizeToTol (
                expandMaxAbsDoubleArray
                    (
                    (double*)pSurface->poles,
                    3 * pSurface->uParams.numPoles * pSurface->vParams.numPoles,
                    SMALLEST_ALLOWED_REFERENCE_SIZE
                    ),
                0.0,
                0.0);
    }


/*--------------------------------------------------------------------*//**
* @description Compute range of the poles of a surface
* @param pRange <= computed range.
* @param pSurface => surface
*------------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void mdlBspline_surfacePoleRange
(
DRange3d *pRange,
MSBsplineSurface *pSurface
)
    {
    int numPole = pSurface->uParams.numPoles * pSurface->vParams.numPoles;
    bsiDRange3d_init (pRange);
    extendRangePoints (pRange, pSurface->poles, pSurface->weights, numPole);
    }

/*--------------------------------------------------------------------*//**
* @description Compute range of the poles of a curve.
* @param pRange <= computed range.
* @param pCurve => curve
*------------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void mdlBspline_curvePoleRange
(
DRange3d *pRange,
MSBsplineCurve *pCurve
)
    {
    int numPole = pCurve->params.numPoles;
    bsiDRange3d_init (pRange);
    extendRangePoints (pRange, pCurve->poles, pCurve->weights, numPole);
    }


END_BENTLEY_GEOMETRY_NAMESPACE