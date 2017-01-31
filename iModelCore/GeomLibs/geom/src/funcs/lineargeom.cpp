/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/lineargeom.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define     DOUBLE_isZero(dbl)          (1.0+dbl == 1.0)

/*----------------------------------------------------------------------+
|                                                                       |
|   external variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
static double s_divisionTolerance = 1.0e-12;
/*----------------------------------------------------------------------+
|                                                                       |
|       local function definitions                                      |
|                                                                       |
+----------------------------------------------------------------------*/




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
* @return true unless lines are parallel
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_intersectXYLines


(
DPoint3dP pPoint0,
double          *pParam0,
DPoint3dP pPoint1,
double          *pParam1,
DPoint3dCP pStart0,
DPoint3dCP pEnd0,
DPoint3dCP pStart1,
DPoint3dCP pEnd1
)

    {
    DPoint3d vector0, vector1;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector0, pEnd0, pStart0);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector1, pEnd1, pStart1);
    return bsiGeom_intersectXYRays
                        (
                        pPoint0, pParam0,
                        pPoint1, pParam1,
                        pStart0, &vector0,
                        pStart1, &vector1
                        );
    }






/*-----------------------------------------------------------------*//**
* sets pProjection to the projection of pPoint on the line from pStart
* to pEnd, and sP to the parametric coordinate of pProjection.
*
* @param pProjection <= projection of point on line
* @param sP <= parametric coordinate along line
* @param pPoint => point to project
* @param pStart => line start
* @param pEnd => line end
* @return true unless line is degenerate.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiVector2d_projectPointToLine

(
DPoint2dP pProjection,
double          *sP,
DPoint2dCP pPoint,
DPoint2dCP pStart,
DPoint2dCP pEnd
)

    {
    double dot1, dot2;
    DVec2d vec0, pVec1;

    static double bigParam = 1.0e14;
    bool    result = false;

    vec0.x = pEnd->x - pStart->x;
    vec0.y = pEnd->y - pStart->y;

    pVec1.x = pPoint->x - pStart->x;
    pVec1.y = pPoint->y - pStart->y;

    dot1 = bsiDVec2d_dotProduct (&vec0, &pVec1);
    dot2 = bsiDVec2d_dotProduct (&vec0, &vec0);

    if (dot1 < bigParam * dot2)
        {
        *sP = dot1 / dot2;
        result = true;
        }
    else
        {
        *sP = 0.0;
        }
    pProjection->SumOf (*pStart,vec0, *sP);
    return result;
    }




/*-----------------------------------------------------------------*//**
* @param pProjection <= Projection of pPoint on line segment
* @param pLambda <= Parametric coordinates of projection
* @param pVector => Vector to be projected
* @param pTarget => Target vector
* @return true unless target vector has zero length.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiVector2d_projectVectorToVector

(
DPoint2dP pProjection,
double      *pLambda,
DPoint2dCP pVector,
DPoint2dCP pTarget
)
    {
    double  dotUU, dotUV;

    dotUU = bsiDPoint2d_dotProduct (pTarget, pTarget);
    dotUV = bsiDPoint2d_dotProduct (pTarget, pVector);

    if (dotUU == 0.0)
        {
        bsiDPoint2d_zero (pProjection);
        *pLambda = 0.0;
        return  false;
        }
    *pLambda = dotUV / dotUU;
    bsiDPoint2d_scale (pProjection, pVector, *pLambda);

    return  true;
    }



/*----------------------------------------------------------------------+
|SECTION skewedPlane    Skewed plane computations                       |
|A skewed plane is a plane represented by three points.  The points are |
|interpreted as origin, x axis, and y axis points.  The axes are NOT    |
|assumed to be perpendicular.                                           |
+----------------------------------------------------------------------*/


/*-----------------------------------------------------------------*//**
* Given an array of three points that are origin, s=1, and t=1 points
* on a skewed plane, evaluate the 3d point at s,t, and the derivatives
* with respect to s and t.  The point is defined as
* pPoint = pPlanePoint[0] + s *(pPlanePoint[1]pPlanePoint[0])
* + t*(pPlanePoint[2]pPlanePoint[0]).
*
* @param pPoint <= space point
* @param deriv1P <= derivative vectors wrt u, v
* @param deriv2P <= 2nd derivative vectors wrt uu, vv, uv
* @param pPlanePoint => origin, s=1, and t=1 points
* @param s => s coordinate of point
* @param t => t coordinate of point
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_evaluateSkewedPlane


(
DPoint3dP pPoint,
DPoint3dP deriv1P,
DPoint3dP deriv2P,
DPoint3dCP pPlanePoint,
double           s,
double           t
)

    {
    DPoint3d    vectorU, vectorV;

    vectorU.x = pPlanePoint[1].x - pPlanePoint[0].x;
    vectorU.y = pPlanePoint[1].y - pPlanePoint[0].y;
    vectorU.z = pPlanePoint[1].z - pPlanePoint[0].z;

    vectorV.x = pPlanePoint[2].x - pPlanePoint[0].x;
    vectorV.y = pPlanePoint[2].y - pPlanePoint[0].y;
    vectorV.z = pPlanePoint[2].z - pPlanePoint[0].z;

    if (pPoint)
        {
        pPoint->x = pPlanePoint[0].x + s * vectorU.x + t * vectorV.x;
        pPoint->y = pPlanePoint[0].y + s * vectorU.y + t * vectorV.y;
        pPoint->z = pPlanePoint[0].z + s * vectorU.z + t * vectorV.z;
        }

    if (deriv1P)
        {
        deriv1P[0] = vectorU;
        deriv1P[1] = vectorV;
        }

    if (deriv2P)
        {
        memset (deriv2P, 0, 3 * sizeof( DPoint3d ));
        }
    }



/*-----------------------------------------------------------------*//**
* Given a space point spacePontP, finds the closest point on the plane
* containing the 3 points in pPlanePoint.  Stores the closest point
* coordinates in pClosePoint, and the s and t coordinates (as defined
* in bsiGeom_evaluateSkewedPlane) in sP and tP.
*
* @param pClosePoint <= point on plane.  May be null pointer
* @param sP <= parametric coordinate on s axis
* @param tP <= parametric coordinate on t axis
* @param pPlanePoint => origin, s=1, and t=1 points
* @param pSpacePoint => point to be projected
* @return true unless the plane points are collinear
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiGeom_closestPointOnSkewedPlane

(
DPoint3dP pClosePoint,
double          *sP,
double          *tP,
DPoint3dCP pPlanePoint,
DPoint3dCP pSpacePoint
)
    {
    double          s,t;
    double          dotUU, dotUV, dotVV, dotUQ, dotVQ;
    bool            result = true;
    DPoint3d        vectorU, vectorV, vectorQ;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorU, &pPlanePoint[1], &pPlanePoint[0]);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorV, &pPlanePoint[2], &pPlanePoint[0]);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorQ, pSpacePoint, &pPlanePoint[0]);

    dotUU = bsiDPoint3d_dotProduct (&vectorU, &vectorU);
    dotVV = bsiDPoint3d_dotProduct (&vectorV, &vectorV);
    dotUV = bsiDPoint3d_dotProduct (&vectorU, &vectorV);
    dotUQ = bsiDPoint3d_dotProduct (&vectorU, &vectorQ);
    dotVQ = bsiDPoint3d_dotProduct (&vectorV, &vectorQ);

    if (!bsiSVD_solve2x2 (&s, &t, dotUU, dotUV, dotUV, dotVV, dotUQ, dotVQ))
        {
        result = false;
        }

    else

        {
        if (sP)
            *sP = s;
        if (tP)
            *tP = t;

        if (pClosePoint)
            bsiDPoint3d_add2ScaledDPoint3d (pClosePoint, &pPlanePoint[0],
                                                &vectorU, s,
                                                &vectorV, t
                                        );
        }

    return  result;
    }


/*-----------------------------------------------------------------*//**
* Given a space point spacePontP, finds the closest point on the plane
* with given origin and 2 vectors.    Stores the closest point
* coordinates in pClosePoint, and the s and t coordinates (as defined
* in bsiGeom_evaluateSkewedPlane) in sP and tP.
*
* @param pClosePoint <= point on plane.  May be null pointer
* @param sP <= parametric coordinate on s axis
* @param tP <= parametric coordinate on t axis
* @param pPlanePoint => origin, s=1, and t=1 points
* @param pSpacePoint => point to be projected
* @see
* @return true unless vectors are parallel
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_closestPointOnSkewedVectors

(
DPoint3dP pClosePoint,
double          *sP,
double          *tP,
DPoint3dCP pOrigin,
DPoint3dCP pVectorU,
DPoint3dCP pVectorV,
DPoint3dCP pSpacePoint
)
    {
    double          s,t;
    double          dotUU, dotUV, dotVV, dotUQ, dotVQ;
    bool            result = true;
    DPoint3d        vectorQ;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorQ, pSpacePoint, pOrigin);

    dotUU = bsiDPoint3d_dotProduct (pVectorU, pVectorU);
    dotVV = bsiDPoint3d_dotProduct (pVectorV, pVectorV);
    dotUV = bsiDPoint3d_dotProduct (pVectorU, pVectorV);

    dotUQ = bsiDPoint3d_dotProduct (pVectorU, &vectorQ);
    dotVQ = bsiDPoint3d_dotProduct (pVectorV, &vectorQ);

    if (!bsiSVD_solve2x2 (&s, &t,
                            dotUU, dotUV, dotUV, dotVV,
                            dotUQ, dotVQ))
        {
        result = false;
        }

    else

        {
        if (sP)
            *sP = s;
        if (tP)
            *tP = t;

        if (pClosePoint)
            bsiDPoint3d_add2ScaledDPoint3d (pClosePoint, pOrigin,
                                                pVectorU, s,
                                                pVectorV, t
                                        );
        }

    return  result;
    }








/*-----------------------------------------------------------------*//**
* @description Compute the intersection point of a line and a plane.
*
* @param pParam <= intersection parameter within line
* @param pPoint <= intersection point
* @param pLineStart => point on line at parameter 0.0
* @param pLineEnd => point on line at parameter 1.0
* @param pOrigin => any point on plane
* @param pNormal => normal vector for plane
* @return true unless the line is parallel to the plane.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_linePlaneIntersection

(
double      *pParam,
DPoint3dP pPoint,
DPoint3dCP pLineStart,
DPoint3dCP pLineEnd,
DPoint3dCP pOrigin,
DPoint3dCP pNormal
)
    {
    bool    result;
    double dot1, dot2, param;
    static double maxFactor = 1.0e14;

    dot2 = bsiDPoint3d_dotDifference (pLineEnd, pLineStart, (DVec3d const*) pNormal);
    dot1 = bsiDPoint3d_dotDifference (pOrigin,  pLineStart, (DVec3d const*) pNormal);

    if (fabs(dot1) < maxFactor * fabs(dot2))
        {
        param = dot1 / dot2;
        result = true;
        }
    else

        {
        param = 0.0;
        result = false;
        }

    if (pParam)
        *pParam = param;
    if (pPoint)
        bsiDPoint3d_interpolate (pPoint, pLineStart, param, pLineEnd);
    return  result;
    }


/*-----------------------------------------------------------------*//**
* Compute the parts of pVector1 tangent and perpendicular to
* pVector0 and in the plane containing both.
*
* @param pTangent       <= projection of pVector1 on pVector0
* @param pParam         <= parameter
* @param pPerpendicular <= pVector1 - pPerpendicular
* @param pVector0       => reference vector
* @param pVector1       => vector being split
* @return true if vector0 has nonzero length.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_tangentAndPerpendicularVectorParts

(
DPoint3dP pTangent,
double          *pParam,
DPoint3dP pPerpendicular,
DPoint3dCP pVector0,
DPoint3dCP pVector1
)
    {
    double dot00, dot01;
    DPoint3d tangent;
    bool    funcStat;
    double s;
    dot00 = bsiDPoint3d_dotProduct (pVector0, pVector0);
    dot01 = bsiDPoint3d_dotProduct (pVector0, pVector1);

    if (fabs(dot00) > s_divisionTolerance * fabs (dot01))
        {
        funcStat = true;
        s = dot01 / dot00;
        if (pParam)
            *pParam = s;
        }
    else
        {
        s = 0.0;
        funcStat = false;
        }

    bsiDPoint3d_scale (&tangent, pVector0, s);
    if (pTangent)
        *pTangent = tangent;
    if (pPerpendicular)
        bsiDPoint3d_subtractDPoint3dDPoint3d (pPerpendicular, pVector1, &tangent);
    return  funcStat;
    }


/*-----------------------------------------------------------------*//**
* Project a point to a plane defined by origin and (not necessarily unit)
* normal vector.
*
* @param pOutPoint <= projected point (or NULL)
* @param pInPoint  => point to project to plane
* @param pNormal   => plane normal
* @param pOrigin   => plane origin
* @return signed distance from point to plane.  If the plane normal has zero length,
*           distance to plane origin.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_distancePointToPlane

(
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
DPoint3dCP pNormal,
DPoint3dCP pOrigin
)
    {
    double      dist, mag;
    DPoint3d    diff;
    DPoint3d    unitNorm;

    mag = bsiDPoint3d_normalize (&unitNorm, pNormal);
    if (mag == 0.0)
        {
        if (pOutPoint)
            *pOutPoint = *pOrigin;
        return bsiDPoint3d_distance (pInPoint, pOrigin);
        }

    bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, pOrigin, pInPoint);
    dist = bsiDPoint3d_dotProduct (&diff, &unitNorm);

    if (pOutPoint)
        bsiDPoint3d_addScaledDPoint3d (pOutPoint, pInPoint, &unitNorm, dist);

    return  dist;
    }




/*-----------------------------------------------------------------*//**
* Sets pProjection to the projection of pPoint onto the infinite
* line through pStart and pEnd, and sets lambdaP to the parametric
* coordinate of the projection point.
*
* @param pProjection <= Projection of pPoint on line
* @param pParameter <= Parametric coordinates of projection
* @param pPoint => Point to be projected
* @param pStart => Line start
* @param pEnd => Line end
* @return true unless the line start and end are coincident
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_projectPointToLine

(
DPoint3dP pProjection,
double      *pParameter,
DPoint3dCP pPoint,
DPoint3dCP pStart,
DPoint3dCP pEnd
)
    {

    DPoint3d    uVector;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&uVector, pEnd, pStart);
    return bsiGeom_projectPointToRay (pProjection, pParameter, pPoint, pStart, &uVector);
    }


/*-----------------------------------------------------------------*//**
* sets prjectionP to the projecion of pVector onto vector pTarget, and
* sets lambdaP to the parametric coordinate of the projection along
* pTarget.
*
* @param pProjection <= Projection of pVector on pTarget
* @param pPerpendicular <= Projection of pVector on plane perpendicular to pTarget
* @param pLambda <= Parametric coordinates of projection
* @param pVector => Vector to be projected
* @param pTarget => plane normal
* @return true unless the target vector has zero length
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiGeom_projectVectorToVector

(
DPoint3dP pProjection,
DPoint3dP pPerpendicular,
double      *pLambda,
DPoint3dCP pVector,
DPoint3dCP pTarget
)
    {
    double  dotUU, dotUV;
    double lambda;

    dotUU = bsiDPoint3d_dotProduct (pTarget, pTarget);
    dotUV = bsiDPoint3d_dotProduct (pTarget, pVector);

    if (dotUU == 0.0)
        {
        if (pProjection)
            bsiDPoint3d_zero (pProjection);
        if (pPerpendicular)
            bsiDPoint3d_zero (pPerpendicular);
        if (pLambda)
            *pLambda = 0.0;
        return  false;
        }

    lambda = dotUV / dotUU;

    if (pProjection)
        bsiDPoint3d_scale (pProjection, pTarget, lambda);

    if (pPerpendicular)
        bsiDPoint3d_addScaledDPoint3d (pPerpendicular, pVector, pTarget, -lambda);

    if (pLambda)
        *pLambda = lambda;

    return  true;
    }




/*-----------------------------------------------------------------*//**
* @param pKeyPoint <= closest grid point
* @param pKeyParam <= closest grid point param
* @param pOrg => line points
* @param pEnd => line end
* @param pNear => nearest point
* @param param0 => parameter to be considered 0 for grid purposes
* @param paramDelta => grid step
* @param minParam => smallest allowed parameter
* @param maxParam => largets allowed parameter
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_closestGridPoint


(
DPoint3dP pKeyPoint,
double          *pKeyParam,
DPoint3dCP pOrg,
DPoint3dCP pEnd,
DPoint3dCP pNear,
double          param0,
double          paramDelta,
double          minParam,
double          maxParam
)

    {
    DPoint3d    linePoint;
    double      projectedParam, gridParam;
    int         gridIndex;

    if (DOUBLE_isZero(paramDelta))
        paramDelta = 1.0;

    bsiGeom_projectPointToLine (&linePoint, &projectedParam, pNear, pOrg, pEnd);

    gridParam = (projectedParam - param0) / paramDelta;
    gridIndex = gridParam > 0.0 ? (int) (gridParam + 0.5) : (int)(gridParam - 0.5);
    gridParam = param0 + gridIndex * paramDelta;

    if (gridParam < minParam)
        gridParam = minParam;
    else if (gridParam > maxParam)
        gridParam = maxParam;

    bsiDPoint3d_interpolate (pKeyPoint, pOrg, gridParam, pEnd);
    *pKeyParam = gridParam;

    }


/*---------------------------------------------------------------------------------**//**
* Project a point to the xy image of a homogeneous line.  Use only (normalized) x and y
* in distance calculations.
*
* @param    pProjection     <= projection.  Start point if line degenerated to point.
* @param    pLambda         <= parametric coordinate of projection.
* @param    pPoint          => point being projected
* @param    pStart          => line start
* @param    pEnd            => line end.
* @return   true unless line degenerated to a point.
* @bsimethod                                                    EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_projectDPoint3dToDPoint4dLineXY

(
DPoint4dP pProjection,
double      *pParam,
DPoint3dCP pPoint,
DPoint4dCP pStart,
DPoint4dCP pEnd
)
    {
    DPoint4d U;
    /* Weighted difference vectors */
    DPoint4d UdP, UdA, PdA;
    double numerator, denominator;
    double param;
    bool    boolStat;
    bsiDPoint4d_subtractDPoint4dDPoint4d (&U, pEnd, pStart);

    bsiDPoint4d_weightedDifferenceDPoint4dDPoint3d (&UdP, &U, pPoint, 1.0);
    bsiDPoint4d_weightedDifference (&UdA, &U, pStart);
    bsiDPoint4d_weightedDifferenceDPoint3dDPoint4d (&PdA, pPoint, 1.0, pStart);

    numerator   = bsiDPoint4d_dotProductXY (&PdA, &UdA);
    denominator = bsiDPoint4d_dotProductXY (&UdP, &UdA);

    boolStat = bsiTrig_safeDivide (&param, numerator, denominator, 0.0);

    if (pParam)
        *pParam = param;

    if (pProjection)
        bsiDPoint4d_addScaledDPoint4d (pProjection, pStart, &U, param);

    return boolStat;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
