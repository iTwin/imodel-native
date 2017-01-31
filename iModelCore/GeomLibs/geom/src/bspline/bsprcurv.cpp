/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bsprcurv.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/BspPrivateApi.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define LINEAR_CURVATURE    1.0e-8

#ifdef COUNT_CALLS

static int s_minDistToCurve = 0;
static int s_closeTan = 0;
static int s_allTans = 0;
static int s_intersectSegment = 0;
static int s_cusp = 0;
static int s_inflections = 0;
static int s_offset = 0;
static int s_parallelTans = 0;
static int s_planeIntesect = 0;
static int s_subdivideZ = 0;
static int s_outputFrequency = 1000;
static void reportCall
(
char *name,
int *pCounter
)
    {
    if ((*pCounter % s_outputFrequency) == 0)
        {
        printf (" %s %d\n", name, *pCounter);
        }
    *pCounter += 1;
    }
#else
#define reportCall(string,couter)
#endif
/*----------------------------------------------------------------------+
|                                                                       |
|   Debugging Variables                                                 |
|                                                                       |
+----------------------------------------------------------------------*/
#if defined (debug_time)
static unsigned long       ticks;
static int                 callCount;
#endif

typedef struct inflPt
    {
    DPoint3d        pt;
    MSBsplineCurve  *curve;
    } InflPt;


/*----------------------------------------------------------------------+
|                                                                       |
|   Newton-Raphson routines                                             |
|                                                                       |
+----------------------------------------------------------------------*/
#define STATUS_CONVERGED                    0
#define STATUS_NONCONVERGED                 1
#define STATUS_DIVERGING                    2
typedef int(*PF_DoublePDoublePDoubleVoidP) (double*f,double*df,double x,void *);

/*----------------------------------------------------------------------+
|                                                                       |
|   Recursive Processing On Single Curve Routines                       |
|                                                                       |
+----------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprcurv_flattenCurve
(
MSBsplineCurve  *flattenedCurveP,           /* <= flattened curve */
MSBsplineCurve  *inCurveP,                  /* => input curve */
RotMatrix       *rotMatrixP                 /* => rotation matrix */
)
    {
    int         status;
    DPoint3d    *pP, *endP;

    if (flattenedCurveP != inCurveP &&
        SUCCESS != (status = bspcurv_copyCurve (flattenedCurveP, inCurveP)))
        return status;

    bsiRotMatrix_multiplyDPoint3dArray ( rotMatrixP, flattenedCurveP->poles, flattenedCurveP->poles,  flattenedCurveP->params.numPoles);

    for (pP=endP=flattenedCurveP->poles, endP += flattenedCurveP->params.numPoles;
            pP < endP;
                pP++)
        pP->z = 0.0;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             06/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprcurv_minDistToCurve
(
double          *distance,             /* <= distance to closest point */
DPoint3d        *minPt,                /* <= closest point of curve */
double          *param,                /* <= param of closest point */
DPoint3dCP      inTestPt,             /* => point to calculate dist from */
MSBsplineCurveCP curve,
double          *tolerance,
RotMatrix       *rotMatrix
)
    {
    double dist, value;
    DPoint3d point;
    
    if (rotMatrix != NULL)
        {
        DMatrix4d hMatrix, zMatrix;
        hMatrix.InitFrom (*rotMatrix);
        zMatrix.InitFromRowValues (
                    1,0,0,0,
                    0,1,0,0,
                    0,0,0,0,
                    0,0,0,1);
        hMatrix.InitProduct (zMatrix, hMatrix);
        
        curve->ClosestPointXY (point, value, dist, *inTestPt, &hMatrix);
        }
    else
        curve->ClosestPoint (point, value, *inTestPt);

    if (distance)   *distance = inTestPt->Distance (point);
    if (minPt)      *minPt    = point;
    if (param)      *param    = value;

    return SUCCESS;
    }

#ifndef MAX_POLE_BUFFER
#define MAX_POLE_BUFFER 2000
#endif

/*----------------------------------------------------------------------+
|									|
| name		bsprcurv_closeTanToCurve				|
|									|
| author	BFP					12/90		|
|									|
+----------------------------------------------------------------------*/
Public int      bsprcurv_closeTanToCurve
(
DPoint3d    	*tanPt,    	       /* <= on curve with desired tangent */
double	    	*tanFraction,	       /* <= param of tanPt on curve, or -1 */
DPoint3d    	*inFindPt,	       /* => point to find tangent to */
DPoint3d    	*inBiasPoint,	       /* => closest tan to this point */
MSBsplineCurve	*curve,
double	    	*tolerance,
RotMatrix    	*rotMatrix
)
    {
    double f;
    DPoint3d curvePoint;
    bool ret;
    
    if (rotMatrix != NULL)
        {
        DMatrix4d hMatrix, zMatrix;
        hMatrix.InitFrom (*rotMatrix);
        zMatrix.InitFromRowValues (
                    1,0,0,0,
                    0,1,0,0,
                    0,0,0,0,
                    0,0,0,1);
        hMatrix.InitProduct (zMatrix, hMatrix);
        
        ret = curve->ClosestTangentXY (curvePoint, f, *inFindPt, *inBiasPoint, &hMatrix);
        }
    else
        ret = curve->ClosestTangent (curvePoint, f, *inFindPt, *inBiasPoint);

    if (tanPt)          *tanPt = curvePoint;
    if (tanFraction)    *tanFraction    = f;

    return ret ? SUCCESS : ERROR;
    }

/*----------------------------------------------------------------------+
|									|
| name		bsprcurv_allTansToCurve				    	|
|									|
| author	BFP					12/90		|
|									|
+----------------------------------------------------------------------*/
Public int      bsprcurv_allTansToCurve
(
DPoint3d    	**outTanPoints,	       /* <= tangent point(s) on curve */
double	    	**outTanFractions,     /* <= param(s) of tangent pts */
int    	    	*outNumPoints,         /* <= number of tangents */
DPoint3d    	*inFindPt,	       /* => point to find tangent to */
MSBsplineCurve	*curve,
double	    	*tolerance,
RotMatrix    	*rotMatrix
)
    {
    if (NULL != outNumPoints)
        *outNumPoints = 0;
    if (NULL != outTanFractions)
        *outTanFractions = NULL;
    if (NULL != outTanPoints)
        *outTanPoints = NULL;

    bvector<DPoint3d> points;
    bvector<double> fractions;

    if (rotMatrix != NULL)
        {
        DMatrix4d hMatrix, zMatrix;
        hMatrix.InitFrom (*rotMatrix);
        zMatrix.InitFromRowValues (
                    1,0,0,0,
                    0,1,0,0,
                    0,0,0,0,
                    0,0,0,1);
        hMatrix.InitProduct (zMatrix, hMatrix);
        
        curve->AllTangentsXY (points, fractions, *inFindPt, &hMatrix);
        }
    else
        curve->AllTangents (points, fractions, *inFindPt);

    *outNumPoints = (int)points.size ();
    if (NULL != outTanPoints)
        *outTanPoints = DPoint3dOps::MallocAndCopy (points);
    if (NULL != outTanFractions)
        *outTanFractions = DoubleOps::MallocAndCopy (fractions);

    return SUCCESS;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             01/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsprcurv_intersectSegment
(
DPoint3d        *intPt,
double          *param,
DSegment3d *segment,
MSBsplineCurve  *curve,
RotMatrix       *rotMatrix,
double          *tolerance
)
    {
    bvector<DPoint3d> curvePoints;
    bvector<double> curveParams;

    if (intPt)
        intPt->Zero ();
    if (param)
        *param = -1.0;
    if (rotMatrix)
        {
        DMatrix4d matrix;
        matrix.InitFrom (*rotMatrix);
        curve->AddLineIntersectionsXY (&curvePoints, &curveParams, NULL, NULL, *segment, false, &matrix);
        }
    else
        {
        curve->AddLineIntersectionsXY (&curvePoints, &curveParams, NULL, NULL, *segment, false, NULL);
        }

    if (curveParams.size () > 0)
        {
        if (intPt)
            *intPt = curvePoints[0];
        if (param)
            *param = curveParams[0];
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    01/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprcurv_cuspPoints
(
DPoint3d        **cuspPts,
double          **params,
int             *numPoints,
MSBsplineCurve  *curve,
double          tolerance,
RotMatrix       *rotMatrix
)
    {
    bvector<DPoint3d> points;
    bvector<double> fractions;

    if (rotMatrix != NULL)
        {
        DMatrix4d hMatrix, zMatrix;
        hMatrix.InitFrom (*rotMatrix);
        zMatrix.InitFromRowValues (
                    1,0,0,0,
                    0,1,0,0,
                    0,0,0,0,
                    0,0,0,1);
        hMatrix.InitProduct (zMatrix, hMatrix);
        
        curve->AddCuspsXY (&points, &fractions, &hMatrix);
        }
    else
        curve->AddCusps (&points, &fractions);

    *numPoints = (int)points.size ();
    if (NULL != cuspPts)
        *cuspPts = DPoint3dOps::MallocAndCopy (points);
    if (NULL != params)
        *params = DoubleOps::MallocAndCopy (fractions);

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    01/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprcurv_inflectionPoints
(
DPoint3d        **cuspPts,
double          **params,
int             *numPoints,
MSBsplineCurve  *curve,
double          tolerance,
RotMatrix       *rotMatrix
)
    {
    int         status;
    bvector<DPoint3d> points;
    bvector<double> fractions;

    if (rotMatrix != NULL)
        status = curve->ComputeInflectionPointsXY (points, fractions, rotMatrix);
    else
        status = curve->ComputeInflectionPoints (points, fractions);

    if (SUCCESS == status)
        {
        *numPoints = (int)points.size ();
        if (NULL != cuspPts)
            *cuspPts = DPoint3dOps::MallocAndCopy (points);
        if (NULL != params)
            *params = DoubleOps::MallocAndCopy (fractions);
        }

    return status;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             06/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprcurv_closestBtwCurves
(
double          *distance,             /* <= closest distance between curves */
DPoint3d        *minPt0,               /* <= closest point on curve0 */
DPoint3d        *minPt1,               /* <= closest point on curve1 */
double          *param0,               /* <= param of point on curve0 */
double          *param1,               /* <= param of point on curve1 */
MSBsplineCurve  *curve0,
MSBsplineCurve  *curve1,
double          *tolerance
)
    {
    return bsprcurv_extClosestBtwCurves (distance, minPt0, minPt1, param0, param1, curve0, curve1, tolerance, fc_hugeVal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             01/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprcurv_minDistTo2Curves
(
double          *distance,
DPoint3d        *minPt0,
DPoint3d        *minPt1,
double          *param0,
double          *param1,
MSBsplineCurve  *curve0,
MSBsplineCurve  *curve1,
double          *tolerance,
DPoint3d        *inTestPt,
RotMatrix       *rotMatrix
)
    {
    StatusInt status = ERROR;
    if (NULL == rotMatrix)
        {
        return bsprcurv_closestBtwCurves (distance, minPt0, minPt1, param0, param1, curve0, curve1, tolerance);
        }
    else
        {
        DMatrix4d flatten = DMatrix4d::From (*rotMatrix);
        DPoint4d zero;
        zero.Zero ();
        flatten.SetRow (3, zero);
        MSBsplineCurve curveA, curveB;
        curveA.CopyFrom (*curve0);
        curveB.CopyFrom (*curve1);
        curveA.TransformCurve4d (flatten);
        curveB.TransformCurve4d (flatten);

        double paramA, paramB;
        DPoint3d pointA, pointB;
        double distanceXY;
        if (SUCCESS == bsprcurv_closestBtwCurves (&distanceXY, &pointA, &pointB,
                    &paramA, &paramB, &curveA, &curveB, tolerance))
            {
            if (distance)
                *distance = distanceXY;
            if (NULL != param0)
                *param0 = paramA;
            if (NULL != minPt0)
                curve0->FractionToPoint (*minPt0, paramA);

            if (NULL != param1)
                *param1 = paramB;
            if (NULL != minPt0)
                curve1->FractionToPoint (*minPt1, paramB);

            status = SUCCESS;
            }
        curveA.ReleaseMem ();
        curveB.ReleaseMem ();
        }
    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprcurv_allParallelTangents
(
DPoint3d        **tanPts,              /* <= parallel tangent point(s) on curve */
double          **params,              /* <= param(s) of tangent pts */
int             *numPoints,            /* <= number of tangents */
DPoint3d        *inDirection,          /* => direction to find parallel to */
MSBsplineCurve  *curve
)
    {
    bvector<DPoint3d> points;
    bvector<double> fractions;
    DVec3d vector = DVec3d::From (*inDirection);

    curve->AllParallellTangentsXY (points, fractions, vector);

    *numPoints = (int)points.size ();
    if (NULL != tanPts)
        *tanPts = DPoint3dOps::MallocAndCopy (points);
    if (NULL != params)
        *params = DoubleOps::MallocAndCopy (fractions);

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprcurv_curvePlaneIntersects
(
double          **paramPP,              /* <= param(s) of intersection pts */
int             *numPointsP,            /* <= number of intersectionPoints */
DPoint3d        *planeNormalP,          /* => plane normal */
double          planeDistance,          /* => plane distance (from origin) */
MSBsplineCurve  *curveP,                /* => curve */
double          tolerance               /* => tolerance */
)
    {
    DPlane3d plane;
    DPoint3d point;
    
    DVec3d   direction = DVec3d::From (*planeNormalP);
    direction.Normalize ();
    point.SumOf (DPoint3d::FromXYZ (0, 0, 0), direction, planeDistance);
    plane.InitFromOriginAndNormal (point, direction);

    bvector<DPoint3d>   points;
    bvector<double>     fractions;

    curveP->AddPlaneIntersections (&points, &fractions, plane);

    *numPointsP = (int)points.size ();
    if (NULL != paramPP)
        *paramPP = DoubleOps::MallocAndCopy (fractions);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt    bsprcurv_subdivideToZNegative
(
MSBsplineCurve      *curveP,
double              zMinimum
)
    {
    // Try to find some curve fragment below zMinimum.
    static double s_minParameterStep = 1.0e-5;
    bvector <double> fractionParams;
    DPlane3d zPlane;
    zPlane.origin = DPoint3d::From (0,0,zMinimum);
    zPlane.normal = DVec3d::From (0,0,1);
    curveP->AddPlaneIntersections (NULL, &fractionParams, zPlane);
    if (fractionParams.size () > 0)
        {
        double startFraction = 0.0;
        double endFraction;
        fractionParams.push_back (1.0);
        DoubleOps::Sort (fractionParams);
        for (size_t i = 0; i < fractionParams.size  (); i++, startFraction = endFraction)
            {
            endFraction = fractionParams[i];
            if (endFraction > startFraction + s_minParameterStep)
                {
                DPoint3d xyz;
                curveP->FractionToPoint (xyz, 0.5 * (startFraction + endFraction));
                MSBsplineCurve partialCurve;
                if (xyz.z < 0.0 && SUCCESS == partialCurve.CopyFractionSegment (*curveP, startFraction, endFraction))
                    {
                    curveP->ReleaseMem ();
                    *curveP = partialCurve;
                    return SUCCESS;
                    }
                }
            }
        }
    else
        {
        DPoint3d xyz;
        curveP->FractionToPoint (xyz, 0.5);
        if (xyz.z < zMinimum)
            return SUCCESS;
        }
    return ERROR;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
