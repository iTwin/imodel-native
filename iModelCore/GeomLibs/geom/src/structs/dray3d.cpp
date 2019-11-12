/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*//**
* @struct DRay3d
* A DRay3d structure a ray (directed line) defined by a start point and
*       direction vector.
* @fields
* @field DPoint3d origin start point of the ray.
* @field DPoint3d ray direction vector for the ray.
* @endfields
* @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); else FIX_MAX(value, max)


/* CSVFUNC(initFromStartEnd) */
/*---------------------------------------------------------------------------------**//**
* Initialize a ray from start and target points
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDRay3d_initFromDPoint3dStartEnd

(
DRay3dP pInstance,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
)
    {
    pInstance->origin = *pPoint0;
    pInstance->direction.DifferenceOf (*pPoint1, *pPoint0);
    }


/*---------------------------------------------------------------------------------**//**
* Initialize a ray from a segment.
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDRay3d_initFromDSegment3d

(
DRay3dP pInstance,
DSegment3dCP pSegment
)
    {
    pInstance->origin = pSegment->point[0];
    pInstance->direction.DifferenceOf (*(&pSegment->point[1]), *(&pSegment->point[0]));
    }


/*---------------------------------------------------------------------------------**//**
* Initialize a ray from 2d endpoints
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDRay3d_initFromDPoint2dStartEnd

(
DRay3dP pInstance,
DPoint2dCP pPoint0,
DPoint2dCP pPoint1
)
    {
    pInstance->origin.x = pPoint0->x;
    pInstance->origin.y = pPoint0->y;

    pInstance->direction.x = pPoint1->x - pPoint0->x;
    pInstance->direction.y = pPoint1->y - pPoint0->y;

    pInstance->origin.z = pInstance->direction.z = 0.0;
    }


/* CSVFUNC(initFromStartTangent) */
/*---------------------------------------------------------------------------------**//**
* Initialize a ray from origin and direction.
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDRay3d_initFromDPoint3dTangent

(
DRay3dP pInstance,
DPoint3dCP pPoint0,
DVec3dCP pTangent
)
    {
    pInstance->origin = *pPoint0;
    pInstance->direction = *pTangent;
    }


/* CSVFUNC(evaluate) */
/*---------------------------------------------------------------------------------**//**
* Evaluate the segment at a parametric coordinate.
*
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDRay3d_evaluateDPoint3d

(
DRay3dCP pInstance,
DPoint3dP pPoint,
double            param
)
    {
    pPoint->SumOf (pInstance->origin, pInstance->direction, param);
    }


/*---------------------------------------------------------------------------------**//**
* Return the origin and target of the ray.
*
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDRay3d_evaluateEndPoints

(
DRay3dCP pInstance,
DPoint3dP pPoint0,
DPoint3dP pPoint1
)
    {
    *pPoint0 = pInstance->origin;
    pPoint1->SumOf (pInstance->origin, pInstance->direction);
    }


/* CSVFUNC(evaluateTangent) */
/*---------------------------------------------------------------------------------**//**
* Return the (unnormalized) tangent vector along the segment.
*
* @indexVerb tangent
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDRay3d_evaluateTangent

(
DRay3dCP pInstance,
DVec3dP pTangent
)
    {
    *pTangent = pInstance->direction;
    }


/*---------------------------------------------------------------------------------**//**
* @return squared length of the ray's direction vector.
*
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double          bsiDRay3d_lengthSquared

(
DRay3dCP pInstance
)
    {
    return pInstance->direction.MagnitudeSquared ();
    }


/* CSVFUNC(projectPoint) */
/*---------------------------------------------------------------------------------**//**
* Project a point onto the extended ray in 3D.
*
* @indexVerb projection
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDRay3d_projectPoint

(
DRay3dCP pInstance,
DPoint3dP pClosestPoint,
double          *pClosestParam,
DPoint3dCP pPoint
)
    {
    DVec3d vectorU, vectorV;
    double UdotU, UdotV, param;
    bool    result;

    vectorU = pInstance->direction;
    vectorV.DifferenceOf (*pPoint, pInstance->origin);
    UdotU = vectorU.DotProduct (vectorU);
    UdotV = vectorU.DotProduct (vectorV);

    result = DoubleOps::SafeDivide (param, UdotV, UdotU, 0.0);

    if (pClosestParam)
        *pClosestParam = param;

    if (pClosestPoint)
        pClosestPoint->SumOf (pInstance->origin, vectorU, param);
    return result;
    }


/*---------------------------------------------------------------------------------**//**
* Project a point onto the bounded line in 3D.  If nearest point of extended line
* is outside the 0..1 parameter range, returned values are for nearest endpoint.
*
* @indexVerb projection
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDRay3d_projectPointBounded

(
DRay3dCP pInstance,
DPoint3dP pClosestPoint,
double          *pClosestParam,
DPoint3dCP pPoint
)
    {
    DVec3d vectorU, vectorV;
    double UdotU, UdotV, param;
    bool    result;

    vectorU = pInstance->direction;
    vectorV.DifferenceOf (*pPoint, pInstance->origin);
    UdotU = vectorU.DotProduct (vectorU);
    UdotV = vectorU.DotProduct (vectorV);

    result = DoubleOps::SafeDivide (param, UdotV, UdotU, 0.0);

    if (param < 0.0)
        param = 0.0;
    if (param > 1.0)
        param = 1.0;

    if (pClosestParam)
        *pClosestParam = param;

    if (pClosestPoint)
        {
        if (param <= 0.0)
            *pClosestPoint = pInstance->origin;
        else
            pClosestPoint->SumOf (pInstance->origin, vectorU, param);
        }
    return result;
    }


/*-----------------------------------------------------------------*//**
*
* Apply a transformation to the source ray.
* @param pTransform => transformation to apply.
* @param pSource => source ray
* @indexVerb transform
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRay3d_multiplyTransformDRay3d

(
DRay3dP pDest,
TransformCP pTransform,
DRay3dCP pSource
)
    {
    pTransform->Multiply (&pDest->origin, &pSource->origin, 1);
    pTransform->MultiplyMatrixOnly (pDest->direction, pSource->direction);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Find the closest approach of two (unbounded) rays.
*
* @param pParam0 <= parameter on first ray.
* @param pParam1 <= parameter on second ray.
* @param pPoint0 <= point on first ray.
* @param pPoint1 <= point on second ray.
* @param pRay0   => first ray.
* @param pRay1   => second ray.
* @indexVerb
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDRay3d_closestApproach

(
double          *pParam0,
double          *pParam1,
DPoint3dP pPoint0,
DPoint3dP pPoint1,
DRay3dCP pRay0,
DRay3dCP pRay1
)
    {
    return bsiGeom_closestApproachOfRays
                (
                pParam0,
                pParam1,
                pPoint0,
                pPoint1,
                &pRay0->origin,
                &pRay0->direction,
                &pRay1->origin,
                &pRay1->direction
                );
    }

/*---------------------------------------------------------------------------------**//**
* Return the intersection of the (unbounded) ray with a plane.
* @param pIntPoint <= intersection point
* @param pIntParam <= parameter along the ray
* @param pPlane => plane (origin and normal)
* @return false if ray, plane are parallel.
* @indexVerb
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDRay3d_intersectDPlane3d

(
DRay3dCP pInstance,
DPoint3dP pIntPoint,
double          *pIntParam,
DPlane3dCP pPlane
)
    {
    DVec3d vectorA;
    double UdotN, AdotN, param;
    bool    result;

    vectorA.DifferenceOf (pInstance->origin, pPlane->origin);

    UdotN = pInstance->direction.DotProduct (pPlane->normal);
    AdotN = vectorA.DotProduct (pPlane->normal);
    result = DoubleOps::SafeDivide (param, -AdotN, UdotN, 0.0);

    if (pIntParam)
        *pIntParam = param;

    if (pIntPoint)
        pIntPoint->SumOf (pInstance->origin, pInstance->direction, param);

    return result;
    }


/*---------------------------------------------------------------------------------**//**
* Return the intersection of the (unbounded) ray with a circle, using only
* xy coordinates.
* @param pIntPoint <= 0, 1, or 2 intersection points.
* @param pInParam  <= 0, 1, or 2 intersection parameters.
* @param pIntParam => parameter along the line
* @param pCenter => circle center.
* @param radius  => circle radius.
* @return   number of intersections.
* @indexVerb
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             bsiDRay3d_intersectCircleXY

(
DRay3dCP pRay,
DPoint3dP pIntPoint,
double          *pIntParam,
DPoint3dCP pCenter,
double          radius
)
    {
    DVec3d V;
    double a, b, c;
    int numSolution;
    double param[2];
    int i;

    V.DifferenceOf (pRay->origin, *pCenter);
    a = pRay->direction.DotProductXY (pRay->direction);
    b = 2.0 * pRay->direction.DotProductXY (V);
    c = V.DotProductXY (V) - radius * radius;

    numSolution = bsiMath_solveQuadratic (param, a, b, c);
    for (i = 0; i < numSolution; i++)
        {
        if (pIntParam)
            pIntParam[i] = param[i];
        if (pIntPoint)
            pIntPoint[i].SumOf (pRay->origin, pRay->direction, param[i]);
        }
    return numSolution;
    }





/*---------------------------------------------------------------------------------**//**
* @description Compute (simple) intersection of a line and a triangle.
*
* @remarks If the returned barycentric coordinates all lie within [0,1], then the intersection lies on or inside the triangle.
* @remarks If the returned ray parameter is nonnegative, then the intersection lies on the ray.
*
* @param pRay           IN      ray with which to intersect triangle
* @param pXYZ           OUT     point of intersection
* @param pBarycentric   OUT     intersection's barycentric coords relative to triangle
* @param pRayParameter  OUT     intersection's parameter along the ray (intersection = rayOrigin + parameter * rayDirection)
* @param pTriangleXYZ   IN      array of 3 triangle vertex coords
* @return true if intersection computed
* @bsihdr                                                       EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDRay3d_intersectTriangle

(
DRay3dCP      pRay,
DPoint3dP    pXYZ,
DPoint3dP    pBarycentric,
double*     pRayParameter,
DPoint3dCP    pTriangleXYZ
)
    {
    RotMatrix matrix, inverse;
    DVec3d    vectorU, vectorV, vectorW;
    DVec3d    vectorC, solution;
    bool        result = false;

    vectorU.DifferenceOf (pTriangleXYZ[1], pTriangleXYZ[0]);
    vectorV.DifferenceOf (pTriangleXYZ[2], pTriangleXYZ[0]);
    vectorW = pRay->direction;
    matrix.InitFromColumnVectors (vectorU, vectorV, vectorW);

    if (inverse.InverseOf (matrix))
        {
        vectorC.DifferenceOf (pRay->origin, pTriangleXYZ[0]);
        inverse.Multiply (solution, vectorC);

        if (pBarycentric)
            {
            pBarycentric->x = 1.0 - solution.x - solution.y;
            pBarycentric->y = solution.x;
            pBarycentric->z = solution.y;
            }

        if (pRayParameter)
            *pRayParameter = -solution.z;

        if (pXYZ)
            pXYZ->SumOf (pRay->origin, pRay->direction, -solution.z);

        result = true;
        }
    return result;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
