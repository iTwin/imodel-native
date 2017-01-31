/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dsegment4d.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); else FIX_MAX(value, max)

/*----------------------------------------------------------------------+
|                                                                       |
|   external variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|       local function definitions                                      |
|                                                                       |
+----------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------**//**
* Initialize a segment from endpoints.
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDSegment4d_initFromStartEndXYZXYZ

(
DSegment4dP pInstance,
double          x0,
double          y0,
double          z0,
double          x1,
double          y1,
double          z1
)
    {
    pInstance->point[0].x = x0;
    pInstance->point[0].y = y0;
    pInstance->point[0].z = z0;
    pInstance->point[0].w = 1.0;

    pInstance->point[1].x = x1;
    pInstance->point[1].y = y1;
    pInstance->point[1].z = z1;
    pInstance->point[1].w = 1.0;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize a segment from endpoints.
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDSegment4d_initFromStartEndXYZWXYZW

(
DSegment4dP pInstance,
double          x0,
double          y0,
double          z0,
double          w0,
double          x1,
double          y1,
double          z1,
double          w1
)
    {
    pInstance->point[0].x = x0;
    pInstance->point[0].y = y0;
    pInstance->point[0].z = z0;
    pInstance->point[0].w = w0;

    pInstance->point[1].x = x1;
    pInstance->point[1].y = y1;
    pInstance->point[1].z = z1;
    pInstance->point[1].w = w1;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize a segment from endpoints.
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDSegment4d_initFromDPoint3d

(
DSegment4dP pInstance,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
)
    {
    pInstance->point[0].x = pPoint0->x;
    pInstance->point[0].y = pPoint0->y;
    pInstance->point[0].z = pPoint0->z;
    pInstance->point[0].w = 1.0;

    pInstance->point[1].x = pPoint1->x;
    pInstance->point[1].y = pPoint1->y;
    pInstance->point[1].z = pPoint1->z;
    pInstance->point[1].w = 1.0;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize a segment from endpoints.
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDSegment4d_initFromDPoint4d

(
DSegment4dP pInstance,
DPoint4dCP pPoint0,
DPoint4dCP pPoint1
)
    {
    pInstance->point[0] = *pPoint0;
    pInstance->point[1] = *pPoint1;
    }



/*---------------------------------------------------------------------------------**//**
* Initialize a segment from endpoints.
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDSegment4d_initFromDPoint2d

(
DSegment4dP pInstance,
DPoint2dCP pPoint0,
DPoint2dCP pPoint1
)
    {
    pInstance->point[0].x = pPoint0->x;
    pInstance->point[0].y = pPoint0->y;

    pInstance->point[1].x = pPoint1->x;
    pInstance->point[1].y = pPoint1->y;

    pInstance->point[0].z = pInstance->point[1].z = 0.0;
    pInstance->point[0].w = pInstance->point[1].w = 1.0;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize a segment from endpoints.
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDSegment4d_initFromDPoint3dTangent

(
DSegment4dP pInstance,
DPoint3dCP pPoint0,
DPoint3dCP pTangent
)
    {
    DPoint3d point1;
    bsiDPoint3d_addDPoint3dDPoint3d (&point1, pPoint0, pTangent);
    bsiDSegment4d_initFromDPoint3d (pInstance, pPoint0, &point1);
    }


/*---------------------------------------------------------------------------------**//**
* Initialize a segment from a ray.
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDSegment4d_initFromDRay3d

(
DSegment4dP pInstance,
DRay3dCP pRay
)
    {
    DPoint3d point1;
    bsiDPoint3d_addDPoint3dDPoint3d (&point1, &pRay->origin, &pRay->direction);
    bsiDSegment4d_initFromDPoint3d (pInstance, &pRay->origin, &point1);
    }


/*---------------------------------------------------------------------------------**//**
* Initialize a segment from DPoint3d and weight values.
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDSegment4d_initFromWeightedDPoint3d

(
DSegment4dP pInstance,
DPoint3dCP pPoint0,
double          weight0,
DPoint3dCP pPoint1,
double          weight1
)
    {
    pInstance->point[0].x = pPoint0->x;
    pInstance->point[0].y = pPoint0->y;
    pInstance->point[0].z = pPoint0->z;
    pInstance->point[0].w = weight0;

    pInstance->point[1].x = pPoint0->x;
    pInstance->point[1].y = pPoint0->y;
    pInstance->point[1].z = pPoint0->z;
    pInstance->point[1].w = weight1;
    }


/*---------------------------------------------------------------------------------**//**
* Return the endpoints of the segment, normalized to 3d.  This conversions does NOT
* preserve parameterization, and can fail if there is a zero weight in the 4D definition.
*
* @indexVerb get
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDSegment4d_getDSegment3d

(
DSegment4dCP pInstance,
DSegment3dP pSegment
)
    {
    bool    b0 = bsiDPoint4d_normalize (&pInstance->point[0], &pSegment->point[0]);
    bool    b1 = bsiDPoint4d_normalize (&pInstance->point[1], &pSegment->point[1]);
    return b0 && b1;
    }


/*---------------------------------------------------------------------------------**//**
* Move the xyw endpoints to xyz of a DSegment3d.  This captures perspective effects for xy viewing,
* and preserves parameterization.
* preserve parameterization.
*
* @indexVerb get
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDSegment4d_getDSegment3dXYW

(
DSegment4dCP pInstance,
DSegment3dP pSegment
)
    {
    bsiDPoint4d_getXYW (&pInstance->point[0], &pSegment->point[0]);
    bsiDPoint4d_getXYW (&pInstance->point[1], &pSegment->point[1]);
    }



/*---------------------------------------------------------------------------------**//**
* Project a point onto the extended line in true 4D.
*
* @indexVerb projection
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDSegment4d_projectDPoint4d

(
DSegment4dCP pInstance,
DPoint4dP pClosestPoint,
double          *pClosestParam,
DPoint4dCP pPoint
)
    {
    DPoint4d vectorU, vectorV;
    double UdotU, UdotV, param;
    bool    result;

    bsiDPoint4d_subtractDPoint4dDPoint4d (&vectorU, &pInstance->point[1], &pInstance->point[0]);
    bsiDPoint4d_subtractDPoint4dDPoint4d (&vectorV, pPoint, &pInstance->point[0]);
    UdotU = bsiDPoint4d_dotProduct (&vectorU, &vectorU);
    UdotV = bsiDPoint4d_dotProduct (&vectorU, &vectorV);

    result = bsiTrig_safeDivide (&param, UdotV, UdotU, 0.0);

    if (pClosestParam)
        *pClosestParam = param;

    if (pClosestPoint)
        bsiDPoint4d_addScaledDPoint4d (pClosestPoint, &pInstance->point[0], &vectorU, param);
    return result;
    }


/*---------------------------------------------------------------------------------**//**
* Project a point onto the extended, cartesian line using only xyw parts of the line.
*
* @indexVerb projection
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDSegment4d_projectDPoint4dCartesianXYW

(
DSegment4dCP pInstance,
DPoint4dP pClosestPoint,
double          *pClosestParam,
DPoint4dCP pPointP
)
    {
    DPoint4d vectorU;
    DPoint3d diffPA, diffUP, vectorUBar;

    double dot0, dot1, param;
    bool    result;

    bsiDPoint4d_subtractDPoint4dDPoint4d (&vectorU, &pInstance->point[1], &pInstance->point[0]);
    bsiDPoint3d_weightedDifference (&vectorUBar,  &pInstance->point[1], &pInstance->point[0]);
    bsiDPoint3d_weightedDifference (&diffPA, pPointP, &pInstance->point[0]);
    bsiDPoint3d_weightedDifference (&diffUP, &vectorU, pPointP);

    dot0 = bsiDPoint3d_dotProductXY (&diffPA, &vectorUBar);
    dot1 = bsiDPoint3d_dotProductXY (&diffUP, &vectorUBar);
    result = bsiTrig_safeDivide (&param, dot0, dot1, 0.0);

    if (pClosestParam)
        *pClosestParam = param;

    if (pClosestPoint)
        bsiDPoint4d_addScaledDPoint4d (pClosestPoint, &pInstance->point[0], &vectorU, param);
    return result;
    }


/*---------------------------------------------------------------------------------**//**
* Project a point onto the bounded line in 4D.  If nearest point of extended line
* is outside the 0..1 parameter range, returned values are for nearest endpoint.
*
* @indexVerb projection
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDSegment4d_projectDPoint4dBounded

(
DSegment4dCP pInstance,
DPoint4dP pClosestPoint,
double          *pClosestParam,
DPoint4dCP pPoint
)
    {
    DPoint4d vectorU, vectorV;
    double UdotU, UdotV, param;
    bool    result;

    bsiDPoint4d_subtractDPoint4dDPoint4d (&vectorU, &pInstance->point[1], &pInstance->point[0]);
    bsiDPoint4d_subtractDPoint4dDPoint4d (&vectorV, pPoint, &pInstance->point[0]);
    UdotU = bsiDPoint4d_dotProduct (&vectorU, &vectorU);
    UdotV = bsiDPoint4d_dotProduct (&vectorU, &vectorV);

    result = bsiTrig_safeDivide (&param, UdotV, UdotU, 0.0);

    if (param < 0.0)
        param = 0.0;
    if (param > 1.0)
        param = 1.0;

    if (pClosestParam)
        *pClosestParam = param;

    if (pClosestPoint)
        {
        if (param <= 0.0)
            *pClosestPoint = pInstance->point[0];
        else if (param >= 1.0)
            *pClosestPoint = pInstance->point[1];
        else
            bsiDPoint4d_addScaledDPoint4d (pClosestPoint, &pInstance->point[0], &vectorU, param);
        }
    return result;
    }


/*---------------------------------------------------------------------------------**//**
* Return the (unnormalized) intersection of the (unbounded) segment with a plane.
* If the line is parallel to the plane, returns the weighted difference of the
*   endpoints (i.e. a vector along the line) as the intersection point.
* @param pIntPoint => intersection point. May be NULL.
* @param pIntParam => parameter along the line.  May be NULL.
* @param pPlane => plane as homogeneous coefficients.
* @return false if line, plane are parallel.
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDSegment4d_intersectDPoint4dPlane

(
DSegment4dCP pInstance,
DPoint4dP pIntPoint,
double          *pIntParam,
DPoint4dCP pPlaneCoffs
)
    {
    double h0, h1, lambda;
    bool    boolstat;
    h0 = bsiDPoint4d_dotProduct (pPlaneCoffs, &pInstance->point[0]);
    h1 = bsiDPoint4d_dotProduct (pPlaneCoffs, &pInstance->point[1]);

    boolstat = bsiTrig_safeDivide (&lambda, -h0, h1 - h0, 0.0);


    if (pIntPoint)
        {
        if (boolstat)
            {
            bsiDPoint4d_interpolate (pIntPoint,
                                    &pInstance->point[0], lambda, &pInstance->point[1]);
            }
        else
            {
            bsiDPoint4d_weightedDifference (pIntPoint,
                                    &pInstance->point[1], &pInstance->point[0]);
            }
        }

    if (pIntParam)
        *pIntParam = lambda;

    return boolstat;
    }



/*-----------------------------------------------------------------*//**
*
* Apply a transformation to the source segment.
* @param pTransform => transformation to apply.
* @param pSource => source segment
* @indexVerb transform
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDSegment4d_multiplyTransformDSegment4d
(
DSegment4dP pDest,
TransformCP pTransform,
DSegment4dCP pSource
)
    {
    bsiTransform_multiplyDPoint4dArray (pTransform, pDest->point, pSource->point, 2);
    return true;
    }





static double bsiDPoint4d_dotProductAbsValues


(
DPoint4dCP pPoint0,
DPoint4dCP pPoint1
)
    {
    return fabs (pPoint0->x * pPoint1->x)
        +  fabs (pPoint0->y * pPoint1->y)
        +  fabs (pPoint0->z * pPoint1->z)
        +  fabs (pPoint0->w * pPoint1->w);
    }


static bool ExtractCartesianRay
(
DSegment4dCR segment,
DRay3dR ray,
DPoint3dR endPoint
)
    {
    if (segment.point[0].w == 1.0
        && segment.point[1].w == 1.0)
        {
        ray.origin.x = segment.point[0].x;
        ray.origin.y = segment.point[0].y;
        ray.origin.z = segment.point[0].z;
        endPoint.x = segment.point[1].x;
        endPoint.y = segment.point[1].y;
        endPoint.z = segment.point[1].z;
        ray.direction.x = endPoint.x - ray.origin.x;
        ray.direction.y = endPoint.y - ray.origin.y;
        ray.direction.z = endPoint.z - ray.origin.z;
        return true;
        }
    return false;
    }

/*-----------------------------------------------------------------*//**
* Compute the parameters and points where the xy projections of two rays intersect.
*
* @param pPoint0 <= intersection point on line 0.
* @param s0P <= parametric coordinate on segment 0
* @param pPoint1 <= intesection point on line 1.
* @param s1P <= parametric coordinate on segment 1
* @param pStart0 => start of first line segment
* @param pEnd0 => end of first line
* @param pStart1 => start of second segment
* @param pEnd1 => end of second segment
* @see
* @return true unless lines are parallel
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDSegment4d_intersectXYDSegment4dDSegment4d

(
DPoint4dP pPoint01,
double      *pParam01,
DPoint4dP pPoint23,
double      *pParam23,
DSegment4dCP pSegment01,
DSegment4dCP pSegment23
)

    {
    // If it's all cartesian do the simplest thing ...
    DRay3d ray01, ray23;
    DPoint3d point1, point3;
    if (    ExtractCartesianRay (*pSegment01, ray01, point1)
        && ExtractCartesianRay (* pSegment23, ray23, point3))
        {
        DPoint3d point01, point23;
        double param01, param23;
        bool stat = bsiGeom_intersectXYRays
                            (
                            &point01, &param01,
                            &point23, &param23,
                            &ray01.origin, &ray01.direction,
                            &ray23.origin, &ray23.direction
                            );
        if (!stat)
            return false;
        // recompute at maximum accuracy ...
        point01.Interpolate (ray01.origin, param01, point1);
        point23.Interpolate (ray23.origin, param23, point3);
        if (pPoint01)
            pPoint01->Init (point01, 1.0);
        if (pPoint23)
            pPoint23->Init (point23, 1.0);
        if (pParam01)
            *pParam01 = param01;
        if (pParam23)
            *pParam23 = param23;
        return true;
        }

    double param01, param23;
    DPoint4d plane01, plane23;
    double h0, h1, h2, h3;
    bool    b0, b1;
    double a01, a23;
    double d01, d23;
    double dummy0, dummy1;
    DMatrix4d worldToLocal, localToWorld;

    DSegment4d segment01;
    DSegment4d segment23;

    bsiDMatrix4d_initForDPoint4dOrigin (&worldToLocal, &localToWorld, &pSegment01->point[0]);

    bsiDSegment4d_transformDMatrix4d (&segment01, &worldToLocal, pSegment01);
    bsiDSegment4d_transformDMatrix4d (&segment23, &worldToLocal, pSegment23);

    bsiDSegment4d_getXYWImplicitDPoint4dPlane (&segment01, &plane01);
    bsiDSegment4d_getXYWImplicitDPoint4dPlane (&segment23, &plane23);

    h0 = bsiDPoint4d_dotProduct (&segment01.point[0], &plane23);
    h1 = bsiDPoint4d_dotProduct (&segment01.point[1], &plane23);

    h2 = bsiDPoint4d_dotProduct (&segment23.point[0], &plane01);
    h3 = bsiDPoint4d_dotProduct (&segment23.point[1], &plane01);

    // h0,h1,h2,h3 are (scaled) heights of endpoints above planes.
    // To get a sense of what is a "big" number at this scale, recompute
    // with absolute values imposed on all the altitude terms.   Height
    //  differences are then significant only if large compared to the big number.
    a01 = bsiDPoint4d_dotProductAbsValues (&segment01.point[0], &plane23)
        + bsiDPoint4d_dotProductAbsValues (&segment01.point[1], &plane23);

    a23 = bsiDPoint4d_dotProductAbsValues (&segment23.point[0], &plane01)
        + bsiDPoint4d_dotProductAbsValues (&segment23.point[1], &plane01);

    d01 = h1 - h0;
    d23 = h3 - h2;

    b0 =  bsiTrig_safeDivide (&param01, - h0, d01, 0.0)
       && bsiTrig_safeDivide (&dummy0, a01, d01, 0.0);

    b1 =  bsiTrig_safeDivide (&param23, - h2, d23, 0.0)
       && bsiTrig_safeDivide (&dummy1, a23, d23, 0.0);

    /* Use original (untransformed) segments for coordinate calculations. */
    if (pPoint01)
        bsiDPoint4d_interpolate
                    (
                    pPoint01,
                    &pSegment01->point[0],
                    param01,
                    &pSegment01->point[1]
                    );

    if (pPoint23)
        bsiDPoint4d_interpolate
                    (
                    pPoint23,
                    &pSegment23->point[0],
                    param23,
                    &pSegment23->point[1]
                    );

    if (pParam01)
        *pParam01 = param01;

    if (pParam23)
        *pParam23 = param23;

    return b0 && b1;
    }


/*-----------------------------------------------------------------*//**
* Compute the parameters and points where the xy projections of two rays intersect.
* Only return points in 0..1 parameter range.  Output parameters are
* untouched (undefined) if no intersections occur in range.
*
* @param pPoint0 <= intersection point on line 0.
* @param s0P <= parametric coordinate on segment 0
* @param pPoint1 <= intesection point on line 1.
* @param s1P <= parametric coordinate on segment 1
* @param pStart0 => start of first line segment
* @param pEnd0 => end of first line
* @param pStart1 => start of second segment
* @param pEnd1 => end of second segment
* @see
* @return true unless lines are parallel
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDSegment4d_intersectXYDSegment4dDSegment4dBounded


(
DPoint4dP pPoint01,
double      *pParam01,
DPoint4dP pPoint23,
double      *pParam23,
DSegment4dCP pSegment01,
DSegment4dCP pSegment23
)

    {
    DPoint4d plane01, plane23;
    double param01, param23;
    double h0, h1, h2, h3;
    bool    boolstat = false;

    /* This could be implemented by calling the unbounded intersector and checking
        parameters.  However, looking directly at the altitudes allows no-intersect
        cases to be filtered with no divisions. */

    bsiDSegment4d_getXYWImplicitDPoint4dPlane (pSegment01, &plane01);
    bsiDSegment4d_getXYWImplicitDPoint4dPlane (pSegment23, &plane23);

    h0 = bsiDPoint4d_dotProduct (&pSegment01->point[0], &plane23);
    h1 = bsiDPoint4d_dotProduct (&pSegment01->point[1], &plane23);

    if (h0 * h1 <= 0.0)
        {
        h2 = bsiDPoint4d_dotProduct (&pSegment23->point[0], &plane01);
        h3 = bsiDPoint4d_dotProduct (&pSegment23->point[1], &plane01);

        if (h2 * h3 <= 0.0
            && bsiTrig_safeDivide (&param01, - h0, h1 - h0, 0.0)
            && bsiTrig_safeDivide (&param23, - h2, h3 - h2, 0.0))
            {
            if (pPoint01)
                bsiDPoint4d_interpolate
                            (
                            pPoint01,
                            &pSegment01->point[0],
                            param01,
                            &pSegment01->point[1]
                            );

            if (pPoint23)
                bsiDPoint4d_interpolate
                            (
                            pPoint23,
                            &pSegment23->point[0],
                            param23,
                            &pSegment23->point[1]
                            );

            if (pParam01)
                *pParam01 = param01;

            if (pParam23)
                *pParam23 = param23;
            boolstat = true;
            }
        }
    return boolstat;
    }




/*---------------------------------------------------------------------------------**//**
* Get start point for a linear object. Only valid if isLinear is true.
* @param        start point of object.
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiDSegment4d_getStartDPoint4d

(
DSegment4dCP pInstance,
DPoint4dP pPt
)
    {
    *pPt = pInstance->point[0];
    }


/*---------------------------------------------------------------------------------**//**
* Get end point for a linear object. Only valid if isLinear is true.
* @param        end point of object.
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDSegment4d_getEndDPoint4d

(
DSegment4dCP pInstance,
DPoint4dP pPt
)
    {
    *pPt = pInstance->point[1];
    }



/*---------------------------------------------------------------------------------**//**
* Set the "start" point.
* @param        pPoint          new start point.
* @indexVerb set
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiDSegment4d_setStartDPoint4d

(
DSegment4dP pInstance,
DPoint4dCP pPoint
)
    {
    pInstance->point[0] = *pPoint;
    }


/*---------------------------------------------------------------------------------**//**
* Set the "end" point.
* @param        pPoint          new end point.
* @indexVerb set
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDSegment4d_setEndDPoint4d

(
DSegment4dP pInstance,
DPoint4dCP pPoint
)
    {
    pInstance->point[1] = *pPoint;
    }


/*---------------------------------------------------------------------------------**//**
* @param pPointOut  <= coordinates at fractional parameter.
* @param param      => fractional parameter
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDSegment4d_fractionParameterToDPoint4d

(
DSegment4dCP pInstance,
DPoint4dP pPoint,
double  param
)
    {
    bsiDPoint4d_interpolate (pPoint, &pInstance->point[0], param, &pInstance->point[1]);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @param pPointOut  <= coordinates at fractional parameter.
* @param param      => fractional parameter
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDSegment4d_fractionParameterToDPoint3d

(
DSegment4dCP pInstance,
DPoint3dP pPoint,
double  param
)
    {
    DPoint4d point;
    bsiDPoint4d_interpolate (&point, &pInstance->point[0], param, &pInstance->point[1]);
    return bsiDPoint4d_normalize (&point, pPoint);
    }



/*---------------------------------------------------------------------------------**//**
* @param pParamOut  <= where pPoint projects to the line.
* @param pPoint     => point to project to the line.
* @indexVerb projection
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDSegment4d_dPoint4dToFractionParameter

(
DSegment4dCP pInstance,
double      *pParamOut,
DPoint4dCP pPoint
)
    {
    bsiDSegment4d_projectDPoint4d (pInstance, NULL, pParamOut, pPoint);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @param pPointOut      <= point on line at fractional parameter.
* @param pTangentOut    <= tangent vector at fractional parameter.
* @param param          => fractional parameter.
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDSegment4d_fractionParameterToDPoint4dTangent

(
DSegment4dCP pInstance,
DPoint4dP pPoint,
DPoint4dP pTangent,
double      param
)
    {

    if (pPoint)
        bsiDPoint4d_interpolate (pPoint, &pInstance->point[0], param, &pInstance->point[1]);

    if (pTangent)
        bsiDPoint4d_subtractDPoint4dDPoint4d (pTangent, &pInstance->point[1], &pInstance->point[0]);

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @param pTangent   <= A cartesian vector in the direction of the cartesian
*       line.  Informally, its length is arbitrary.  Formally, the vector is the
*       weighted difference of the endpoints, w1*P0 - w0*P1, if that helps make
*       sense out of the length.
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDSegment4d_pseudoTangent

(
DSegment4dCP pSegment,
DPoint3dP pTangent
)
    {
    bsiDPoint3d_weightedDifference (pTangent, &pSegment->point[1], &pSegment->point[0]);
    }


/*---------------------------------------------------------------------------------**//**
* @param pPt <= returned point.
* @param index => index of point to return.
* @indexVerb get
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiDSegment4d_getIndexedDPoint4d

(
DSegment4dCP pInstance,
DPoint4dP pPt,
int             index
)
    {
    if (index == 0 || index == 1)
        {
        *pPt = pInstance->point[index];
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @param pPt <= returned point.
* @param index => index of point to return.
* @indexVerb set
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiDSegment4d_setIndexedDPoint4d

(
DSegment4dP pInstance,
DPoint4dCP pPt,
int             index
)
    {
    if (index == 0 || index == 1)
        {
        pInstance->point[index] = *pPt;
        return true;
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* Compute closest approach equation for 4D rays with one of the pseudotangents.
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void jmdlDSegment4d_closeApproachEquation

(
double *paxy,
double *pax,
double *pay,
double *pa,
DPoint4dCP pA,
DPoint4dCP pU,
DPoint4dCP pB,
DPoint4dCP pV,
DPoint3dCP pT
)
    {
    /* First ray is A + alpha U.  Call the weight part a + alpha u.
       Second ray is B + beta V.  Call the weight part b + beta v.
       A vector between points at alpha, beta is
            (b + beta v) (A + alpha U) - (a + alpha u) (B + beta V)
        and only the xyz parts of the homogeneous points are nonzero.
        Dot with the tangent vector to get perpendicular condition
            (b + beta v) (A.T + alpha U.T) - (a + alpha u) (B.T + beta V.T) = 0
    */
    double AdotT = bsiDPoint3d_dotXYZ (pT, pA->x, pA->y, pA->z);
    double BdotT = bsiDPoint3d_dotXYZ (pT, pB->x, pB->y, pB->z);
    double UdotT = bsiDPoint3d_dotXYZ (pT, pU->x, pU->y, pU->z);
    double VdotT = bsiDPoint3d_dotXYZ (pT, pV->x, pV->y, pV->z);

    *pa   = pB->w * AdotT - pA->w * BdotT;
    *pax  = pB->w * UdotT - pU->w * BdotT;
    *pay  = pV->w * AdotT - pA->w * VdotT;
    *paxy = pV->w * UdotT - pU->w * VdotT;
    }

/*---------------------------------------------------------------------------------**//**
* Find the closest approach of two (unbounded) lines.
*
* @param pParam0 <= parameter on first segment
* @param pParam1 <= parameter on second segment
* @param pPoint0 <= point on first segment.
* @param pPoint1 <= point on second segment.
* @param pSegment0   <= first segment
* @param pSegment1   <= second segment
* @indexVerb closestApproach
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDSegment4d_closestApproach

(
double          *pParamA,
double          *pParamB,
DPoint4dP pPointA,
DPoint4dP pPointB,
DSegment4dCP pSegmentA,
DSegment4dCP pSegmentB
)
    {
    DPoint4d vectorA, vectorB;
    DPoint3d pseudoTangentA, pseudoTangentB;
    double axy, ax, ay, a, bxy, bx, by, b;
    double xx[2], yy[2];
    int numSolution;
    int i;
    bool    boolstat = false;

    /* Pseudo tangent is a single vector along the direction of each line */
    bsiDPoint4d_subtractDPoint4dDPoint4d (&vectorA, &pSegmentA->point[1], &pSegmentA->point[0]);
    bsiDPoint3d_weightedDifference (&pseudoTangentA, &vectorA, &pSegmentA->point[0]);

    bsiDPoint4d_subtractDPoint4dDPoint4d (&vectorB, &pSegmentB->point[1], &pSegmentB->point[0]);
    bsiDPoint3d_weightedDifference (&pseudoTangentB, &vectorB, &pSegmentB->point[0]);

    jmdlDSegment4d_closeApproachEquation (&axy, &ax, &ay, &a,
                    &pSegmentA->point[0], &vectorA, &pSegmentB->point[0], &vectorB,
                    &pseudoTangentA);

    jmdlDSegment4d_closeApproachEquation (&bxy, &bx, &by, &b,
                    &pSegmentA->point[0], &vectorA, &pSegmentB->point[0], &vectorB,
                    &pseudoTangentB);

    bsiMath_solveBilinear (xx, yy, &numSolution, axy, ax, ay, a, bxy, bx, by, b);

    i = -1;
    if (numSolution == 2)
        {
        double w0, w1;
        DPoint4d pointA[2], pointB[2];
        /*
           Homogeneous effects create an artificial solution "at infinity".
           The artificial solution is recognizable by near-zero weights.
           From the 2 solutions, choose the one with larger weights.
        */
        for (i = 0; i < 2; i++)
            {
            bsiDPoint4d_addScaledDPoint4d (&pointA[i], &pSegmentA->point[0], &vectorA, xx[i]);
            bsiDPoint4d_addScaledDPoint4d (&pointB[i], &pSegmentB->point[0], &vectorB, yy[i]);
            }
        w0 = fabs (pointA[0].w) + fabs (pointB[0].w);
        w1 = fabs (pointA[1].w) + fabs (pointB[1].w);
        i = w0 > w1 ? 0 : 1;
        }

    if (numSolution == 1)
        {
        i = 0;
        }

    if (i == -1)
        {
        /* Force a trivial solution for filling in output fields */
        xx[0] = yy[0] = 0.0;
        i = 0;
        boolstat = false;
        }
    else
        {
        boolstat = true;
        }

    if (pParamA)
        *pParamA = xx[i];
    if (pParamB)
        *pParamB = yy[i];

    if (pPointA)
        bsiDPoint4d_addScaledDPoint4d (pPointA, &pSegmentA->point[0], &vectorA, xx[i]);
    if (pPointB)
        bsiDPoint4d_addScaledDPoint4d (pPointB, &pSegmentB->point[0], &vectorB, yy[i]);

    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
* Test if the segment has unit weights at both ends.
* @bsihdr                                                       EarlinLutz      12/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDSegment4d_isUnitWeighted

(
DSegment4dCP pSegment
)
    {
    double tol = bsiTrig_smallAngle ();
    return fabs (pSegment->point[0].w - 1.0) <= tol
        && fabs (pSegment->point[1].w - 1.0) <= tol;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
