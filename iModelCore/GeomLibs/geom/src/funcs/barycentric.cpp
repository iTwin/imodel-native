/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/barycentric.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <assert.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------------+
| BARYCENTRIC COORDINATE FUNCTIONS:
|
| For a given triangle T with vertices v0, v1, v2, every point q in the plane
| of T is uniquely represented by its barycentric coordinates (b0, b1, b2)
| relative to T:
|
| q = b0 * v0 + b1 * v1 + b2 * v2,
| 1 = b0 + b1 + b2.
|
+----------------------------------------------------------------------------*/


static bool    bsiDPoint3d_barycentricFromDPoint2dTriangleVectors

(
DPoint3dP pInstance,
double    *pArea,
DPoint2dCP pPoint,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
DPoint2dCP pVector2
)
    {
    bool        status = true;
    double      denom, numer1, numer2;

    /*
    Let vectors v1 and v2 emanating from the plane's origin define triangle T,
    and let q be a point in the plane.  Then the system for finding the
    barycentric coordinates of q relative to T reduces from 3x3 to 2x2:
      -             - -    -   -     -      -            - -    -   -     -
      | 0 v1.x v2.x | | b0 |   | q.x |      | v1.x  v2.x | | b1 |   | q.x |
      | 0 v1.y v2.y | | b1 | = | q.y |  =>  | v1.y  v2.y | | b2 | = | q.y |
      | 1    1    1 | | b2 |   | 1   |      -            - -    -   -     -
      -             - -    -   -     -
    We use Cramer's Rule to solve this system for b1 and b2; b0 can be found
    by subtracting the sum of the other two coords from 1.0.
    */

    /* Calculate numerators of Cramer's Rule formulae */
    if (pOrigin)
        {
        /*
        Since barycoords are invariant under affine transformation, we can
        translate the triangle and point so that pOrigin is the origin.  This
        gives us the dimension reduction detailed above.
        */
        numer1 = (pPoint->x - pOrigin->x) * pVector2->y -
                 (pPoint->y - pOrigin->y) * pVector2->x;
        numer2 = (pPoint->y - pOrigin->y) * pVector1->x -
                 (pPoint->x - pOrigin->x) * pVector1->y;
        }
    else
        {
        numer1 = pPoint->x * pVector2->y - pPoint->y * pVector2->x;
        numer2 = pPoint->y * pVector1->x - pPoint->x * pVector1->y;
        }

    /*
    Calculate denominator of Cramer's Rule formulae.  On a good day, denom is
    twice the signed area of T.  On a bad day (i.e. when T is long and skinny)
    we get subtractive cancellation, but there's no way around it!
    */
    denom  = pVector1->x * pVector2->y - pVector2->x * pVector1->y;

    /* Return false and barycoords (1,0,0) if denom relatively close to zero */
    if (! bsiTrig_safeDivide (&pInstance->y, numer1, denom, 0.0))
        status = false;

    if (! bsiTrig_safeDivide (&pInstance->z, numer2, denom, 0.0))
        status = false;

    pInstance->x = 1.0 - pInstance->y - pInstance->z;

    if (pArea)
        *pArea = 2.0 * denom;
    return status;
    }


/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pOrigin, pVector1-pOrigin, pVector2-pOrigin)
* in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pArea          <= area of triangle.
* @param pPoint         => point in plane
* @param pOrigin        => vertex of triangle T (may be null for origin)
* @param pVector1       => side vector of T (emanating from pOrigin)
* @param pVector2       => side vector of T (emanating from pOrigin)
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangleVectors
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsihdr                                      DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint2dTriangleVectors

(
DPoint3dP pInstance,
DPoint2dCP pPoint,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
DPoint2dCP pVector2
)
    {
    return bsiDPoint3d_barycentricFromDPoint2dTriangleVectors (pInstance, NULL, pPoint, pOrigin,
                    pVector1, pVector2);
    }



/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in plane
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangle
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint2dTriangle

(
DPoint3dP pInstance,
DPoint2dCP pPoint,
DPoint2dCP pVertex0,
DPoint2dCP pVertex1,
DPoint2dCP pVertex2
)
    {
    DPoint2d        q, v1, v2;

    q.x  = pPoint->x   - pVertex0->x;
    q.y  = pPoint->y   - pVertex0->y;
    v1.x = pVertex1->x - pVertex0->x;
    v1.y = pVertex1->y - pVertex0->y;
    v2.x = pVertex2->x - pVertex0->x;
    v2.y = pVertex2->y - pVertex0->y;

    return bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
            (pInstance, NULL, &q, NULL, &v1, &v2);
    }


/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in plane
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangle
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint2dTriangle
(
DPoint3dR uvw,
DPoint3dR dUVWdX,
DPoint3dR dUVWdY,
double   &area,
DPoint2dCR point,
DPoint2dCR vertex0,
DPoint2dCR vertex1,
DPoint2dCR vertex2
)
    {
    DPoint2d        q, v1, v2;

    q.x  = point.x   - vertex0.x;
    q.y  = point.y   - vertex0.y;
    v1.x = vertex1.x - vertex0.x;
    v1.y = vertex1.y - vertex0.y;
    v2.x = vertex2.x - vertex0.x;
    v2.y = vertex2.y - vertex0.y;
    double divArea;
    if (bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
            (&uvw, &area, &q, NULL, &v1, &v2)
        && DoubleOps::SafeDivide (divArea, 2.0, area, 0.0))
        {
        
        DVec2d edgeVector[3];
        edgeVector[0].DifferenceOf (vertex2, vertex1);  // vector opposite point0
        edgeVector[1].DifferenceOf (vertex0, vertex2);  // vector opposite point1
        edgeVector[2].DifferenceOf (vertex1, vertex0);  // vector opposite point2
        dUVWdX.Init (-edgeVector[0].y * divArea, -edgeVector[1].y * divArea, -edgeVector[2].y * divArea);
        dUVWdY.Init ( edgeVector[0].x * divArea,  edgeVector[1].x * divArea, edgeVector[2].x * divArea);
        return true;
        }
    uvw.Zero ();
    dUVWdX.Zero ();
    dUVWdY.Zero ();
    area = 0.0;
    return false;
    }





/*-----------------------------------------------------------------*//**
* @description Applies transformation to simplify the problem of finding the barycentric
* coordinates of a 3d point relative to a triangle.  Returned are the
* components of the new 2d problem.
*
* @instance pInstance   => point to find barycentric coords of
* @param pNewPoint      <= point in plane of new triangle with same barycoords
* @param pNewVector1    <= side of new triangle
* @param pNewVector2    <= side of new triangle
* @param pVector1       => side of old triangle
* @param pVector2       => side of old triangle
* @group "DPoint3d Barycentric"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
static void transformBarycentric3dSystemTo2d

(
DPoint3dCP pInstance,
DPoint2dP pNewPoint,
DPoint2dP pNewVector1,
DPoint2dP pNewVector2,
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    /*
    Projecting the 3D point q onto the plane spanned by 3D vectors v1, v2
    (which form triangle T) is a least squares problem:

      [v1 v2] [b1 b2]^ = q

    The resulting normal equations (below) determine the barycoords b1, b2
    (corresponding to v1, v2) of the projection relative to T:
      -              - -    -   -      -
      | v1.v1  v1.v2 | | b1 | = | q.v1 |
      | v1.v2  v2.v2 | | b2 |   | q.v2 |
      -              - -    -   -      -

    or equivalently, [newVector1 newVector2] [b1 b2]^ = newPoint.

    This latter form shows that the 3D problem reduces to a 2D problem:
    finding the barycentric coordinates of newPoint relative to the
    triangle in the xy-plane spanned by vectors newVector1, newVector2.
    */
    pNewVector1->x =    pVector1->x * pVector1->x +
                        pVector1->y * pVector1->y +
                        pVector1->z * pVector1->z;
    pNewVector1->y =
    pNewVector2->x =    pVector1->x * pVector2->x +
                        pVector1->y * pVector2->y +
                        pVector1->z * pVector2->z;

    pNewVector2->y =    pVector2->x * pVector2->x +
                        pVector2->y * pVector2->y +
                        pVector2->z * pVector2->z;

    pNewPoint->x =     pInstance->x * pVector1->x +
                       pInstance->y * pVector1->y +
                       pInstance->z * pVector1->z;

    pNewPoint->y =     pInstance->x * pVector2->x +
                       pInstance->y * pVector2->y +
                       pInstance->z * pVector2->z;
    }



/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in space.
* Points p and r in space have the same barycentric coordinates relative to
* T if and only if they project to the same point q in the plane of T;
* then their barycentric coordinates relative to T are those of q.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in space
* @param pVertex0       => vertex of triangle T
* @param pVertex1       => vertex of triangle T
* @param pVertex2       => vertex of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @see bsiDPoint3d_fromBarycentricAndDPoint3dTriangle
* @return true if and only if the area of T is sufficiently large.
* @group "DPoint3d Barycentric"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint3dTriangle

(
DPoint3dP pInstance,
DPoint3dCP pPoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2
)
    {
    DPoint3d        q, v1, v2;
    DPoint2d        newPoint, newV1, newV2;

    /*
    Translating by pVertex0 compresses the triangle definition from 3 points
    to 2 vectors while preserving barycentric coords.
    */
    q.x  = pPoint->x   - pVertex0->x;
    q.y  = pPoint->y   - pVertex0->y;
    q.z  = pPoint->z   - pVertex0->z;
    v1.x = pVertex1->x - pVertex0->x;
    v1.y = pVertex1->y - pVertex0->y;
    v1.z = pVertex1->z - pVertex0->z;
    v2.x = pVertex2->x - pVertex0->x;
    v2.y = pVertex2->y - pVertex0->y;
    v2.z = pVertex2->z - pVertex0->z;

    /* decrement dimension of problem */
    transformBarycentric3dSystemTo2d (&q, &newPoint, &newV1, &newV2, &v1, &v2);

    return bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
            (pInstance, &newPoint, NULL, &newV1, &newV2);
    }



/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pOrigin, pVector1-pOrigin, pVector2-pOrigin)
* in space.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in space
* @param pOrigin        => vertex of triangle T (may be null for origin)
* @param pVector1       => side vector of T (emanating from pOrigin)
* @param pVector2       => side vector of T (emanating from pOrigin)
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @see bsiDPoint3d_fromBarycentricAndDPoint3dTriangleVectors
* @return true if and only if the area of T is sufficiently large.
* @group "DPoint3d Barycentric"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint3dTriangleVectors

(
DPoint3dP pInstance,
DPoint3dCP pPoint,
DPoint3dCP pOrigin,
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    DPoint3d    q;
    DPoint2d    newPoint, newV1, newV2;

    /* decrement dimension of problem */
    if (pOrigin)
        {
        /*
        Translating by pOrigin compresses the triangle definition while
        preserving barycentric coords.
        */
        q.x  = pPoint->x - pOrigin->x;
        q.y  = pPoint->y - pOrigin->y;
        q.z  = pPoint->z - pOrigin->z;
        transformBarycentric3dSystemTo2d
                (&q, &newPoint, &newV1, &newV2, pVector1, pVector2);
        }
    else
        transformBarycentric3dSystemTo2d
                (pPoint, &newPoint, &newV1, &newV2, pVector1, pVector2);

    return bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
            (pInstance, &newPoint, NULL, &newV1, &newV2);
    }



/*-----------------------------------------------------------------*//**
* @description Sets this instance to the point in the plane of triangle T
* (pVertex0, pVertex1, pVertex2) with the given barycentric coordinates
* relative to T.
*
* @instance pInstance   <= point in plane of T with given barycoords
* @param pBaryCoords    => barycentric coordinates relative to T
* @param pVertex0       => vertex of triangle T
* @param pVertex1       => vertex of triangle T
* @param pVertex2       => vertex of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @group "DPoint3d Barycentric"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_fromBarycentricAndDPoint3dTriangle

(
DPoint3dP pInstance,
DPoint3dCP pBaryCoords,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2
)
    {
    pInstance->x =  pBaryCoords->x * pVertex0->x +
                    pBaryCoords->y * pVertex1->x +
                    pBaryCoords->z * pVertex2->x;
    pInstance->y =  pBaryCoords->x * pVertex0->y +
                    pBaryCoords->y * pVertex1->y +
                    pBaryCoords->z * pVertex2->y;
    pInstance->z =  pBaryCoords->x * pVertex0->z +
                    pBaryCoords->y * pVertex1->z +
                    pBaryCoords->z * pVertex2->z;
    }



/*-----------------------------------------------------------------*//**
* @description Sets this instance to the point in the plane of triangle T
* (pOrigin, pVector1-pOrigin, pVector2-pOrigin) with the given barycentric
* coordinates relative to T.
*
* @instance pInstance   <= point in plane of T with given barycoords
* @param pBaryCoords    => barycentric coordinates relative to T
* @param pOrigin        => vertex of triangle T (may be null for origin)
* @param pVector1       => side vector of T (emanating from pOrigin)
* @param pVector2       => side vector of T (emanating from pOrigin)
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @group "DPoint3d Barycentric"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_fromBarycentricAndDPoint3dTriangleVectors

(
DPoint3dP pInstance,
DPoint3dCP pBaryCoords,
DPoint3dCP pOrigin,
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    pInstance->x =  pBaryCoords->y * pVector1->x +
                    pBaryCoords->z * pVector2->x;
    pInstance->y =  pBaryCoords->y * pVector1->y +
                    pBaryCoords->z * pVector2->y;
    pInstance->z =  pBaryCoords->y * pVector1->z +
                    pBaryCoords->z * pVector2->z;

    if (pOrigin)
        {
        /* q = b0*v0 + b1(v0+v1) + b2(v0+v2) = v0 + b1v1 + b2v2 */
        pInstance->x += pOrigin->x;
        pInstance->y += pOrigin->y;
        pInstance->z += pOrigin->z;
        }
    }



/*-----------------------------------------------------------------*//**
* @description Returns true if this instance is on or inside the unbounded right prism
* (orthogonally projected indefinitely in both directions) whose base is
* the triangle T with vertices pVertex0, pVertex1, pVertex2.  Also optionally
* returns the barycentric coordinates of the orthogonal projection of this
* instance (if inside T) and the signed volume of the tetrahedron spanned by T
* and this instance.  This volume is nonnegative if T, viewed from this
* instance, has vertices 0,1,2 in counterclockwise order.
*
* @instance pInstance   <= point in space
* @param pVertex0       => vertex of T
* @param pVertex1       => vertex of T
* @param pVertex2       => vertex of T
* @param pBCoords       <= barycentric coordinates of projection of pInstance if inside T (or null)
* @param pVolume        <= signed volume of tetrahedron (or null)
* @see bsiDPoint2d_inDPoint2dTriangleVectors
* @return true if and only if T is sufficiently large
* @group "DPoint3d Barycentric"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_inDPoint3dTrianglePrism

(
DPoint3dCP pInstance,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2,
DPoint3dP pBCoords,
double                  *pVolume
)
    {
    DPoint3d    q, v1, v2;
    DPoint2d    newPoint, newV1, newV2;

    /*
    Translating by pVertex0 compresses the triangle definition from 3 points
    to 2 vectors while preserving barycentric coords.
    */
    q.x  = pInstance->x - pVertex0->x;
    q.y  = pInstance->y - pVertex0->y;
    q.z  = pInstance->z - pVertex0->z;
    v1.x = pVertex1->x  - pVertex0->x;
    v1.y = pVertex1->y  - pVertex0->y;
    v1.z = pVertex1->z  - pVertex0->z;
    v2.x = pVertex2->x  - pVertex0->x;
    v2.y = pVertex2->y  - pVertex0->y;
    v2.z = pVertex2->z  - pVertex0->z;

    /* decrement dimension of problem by projecting pInstance into plane of T */
    transformBarycentric3dSystemTo2d (&q, &newPoint, &newV1, &newV2, &v1, &v2);

    /*
    The system for projection actually squares the area of T, so we compute the
    signed volume from the parallelepiped instead.
    */
    if (pVolume)
        *pVolume = bsiDPoint3d_tripleProduct (&q, &v1, &v2) / 6.0;

    return bsiDPoint2d_inDPoint2dTriangleVectors
            (&newPoint, NULL, &newV1, &newV2, pBCoords, NULL);
    }

/*-----------------------------------------------------------------*//**
* @description Compute the minimum distance from a point to a triangle.
* @instance pSpacePoint   <= point in space
* @param pVertex0       => vertex of T
* @param pVertex1       => vertex of T
* @param pVertex2       => vertex of T
* @param pClosePoint    <= projection of space point onto plane of triangle
* @param pBCoords       <= barycentric coordinates of closest point
* @return minimum distance
* @group "DPoint3d Barycentric"
* @bsihdr                                       EarlinLutz      10/04
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_minDistToTriangle

(
DPoint3dCP pSpacePoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2,
DPoint3dP pClosePoint,
DPoint3dP pBoundedUVW,
DPoint3dP pUnboundedUVW
)
    {
    DPoint3d planePoint;
    double s, t;
    DPoint3d xyz[3];
    static DPoint3d uvwCorner[3] =
        {
            {1,0,0},
            {0,1,0},
            {0,0,1}
        };

    /* Ugh.  Compute for each edge independently.  */
    DSegment3d testSeg[3];
    DPoint3d   testPoint[3];
    double     testParam[3];
    double     testDistanceSquared[3];

    int i, iMin, jMin;
    xyz[0] = *pVertex0;
    xyz[1] = *pVertex1;
    xyz[2] = *pVertex2;
    if (pUnboundedUVW)
        pUnboundedUVW->Zero ();
    
    if (    bsiGeom_closestPointOnSkewedPlane (&planePoint, &s, &t, xyz, pSpacePoint))
        {
        if (pUnboundedUVW)
            bsiDPoint3d_setXYZ (pUnboundedUVW, 1.0 - s - t, s, t);
        if (s >= 0.0
        &&  t >= 0.0
        &&  s + t <= 1.0)
            {
            if (pBoundedUVW)
                bsiDPoint3d_setXYZ (pBoundedUVW, 1.0 - s - t, s, t);
            if (pClosePoint)
                *pClosePoint = planePoint;
            return bsiDPoint3d_distance (&planePoint, pSpacePoint);
            }
        }

    bsiDSegment3d_initFromDPoint3d (&testSeg[0], pVertex0, pVertex1);
    bsiDSegment3d_initFromDPoint3d (&testSeg[1], pVertex1, pVertex2);
    bsiDSegment3d_initFromDPoint3d (&testSeg[2], pVertex2, pVertex0);

    for (i = 0; i < 3; i++)
        {
        bsiDSegment3d_projectPointBounded (&testSeg[i], &testPoint[i], &testParam[i], pSpacePoint);
        testDistanceSquared[i] = bsiDPoint3d_distanceSquared (&testPoint[i], pSpacePoint);
        }
    iMin = 0;
    if (testDistanceSquared[1] < testDistanceSquared[0])
        iMin = 1;
    if (testDistanceSquared[2] < testDistanceSquared[iMin])
        iMin = 2;

    jMin = iMin + 1;
    if (jMin == 3)
        jMin = 0;

    if (pClosePoint)
        *pClosePoint = testPoint[iMin];
    if (pBoundedUVW)
        bsiDPoint3d_interpolate (pBoundedUVW, &uvwCorner[iMin], testParam[iMin], &uvwCorner[jMin]);
    return sqrt (testDistanceSquared[iMin]);
    }


Public GEOMDLLIMPEXP double bsiDPoint3d_minDistToTriangle

(
DPoint3dCP pSpacePoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2,
DPoint3dP pClosePoint,
DPoint3dP pBoundedBarycentric
)
    {
    DPoint3d unboundedBarycentric;
    return bsiDPoint3d_minDistToTriangle (pSpacePoint, pVertex0, pVertex1, pVertex2, pClosePoint, pBoundedBarycentric, &unboundedBarycentric);
    }


/*-----------------------------------------------------------------*//**
* @description Returns true if this instance is on or inside the unbounded right prism
* (orthogonally projected indefinitely in both directions) whose base is
* the triangle T with vertices pOrigin, pVector1-pOrigin, pVector2-pOrigin.
* Also optionally returns the barycentric coordinates of the orthogonal
* projection of this instance (if inside T) and the signed volume of the
* tetrahedron spanned by T and this instance.  This volume is nonnegative
* if T, viewed from this instance, has vertices 0,1,2 in counterclockwise order.
*
* @instance pInstance   <= point in space
* @param pOrigin        => vertex of T (may be null for origin)
* @param pVector1       => side of T (emanating from pOrigin)
* @param pVector2       => side of T (emanating from pOrigin)
* @param pBCoords       <= barycentric coordinates of projection of pInstance if inside T (or null)
* @param pVolume        <= signed volume of tetrahedron (or null)
* @return Whether pInstance is on/inside the prism.
* @see bsiDPoint2d_inDPoint2dTriangleVectors
* @group "DPoint3d Barycentric"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_inDPoint3dTrianglePrismVectors

(
DPoint3dCP pInstance,
DPoint3dCP pOrigin,
DPoint3dCP pVector1,
DPoint3dCP pVector2,
DPoint3dP pBCoords,
double                  *pVolume
)
    {
    DPoint3d    q;
    DPoint2d    newPoint, newV1, newV2;

    /*
    Decrement dimension of problem by projecting pInstance into plane of T.
    Since the system for projection actually squares the area of T, we compute
    a signed volume instead.
    */
    if (pOrigin)
        {
        /*
        Translating by pOrigin compresses the triangle definition while
        preserving barycentric coords.
        */
        q.x  = pInstance->x - pOrigin->x;
        q.y  = pInstance->y - pOrigin->y;
        q.z  = pInstance->z - pOrigin->z;
        transformBarycentric3dSystemTo2d
            (&q, &newPoint, &newV1, &newV2, pVector1, pVector2);

        if (pVolume)
            *pVolume = bsiDPoint3d_tripleProduct (&q, pVector1, pVector2) / 6.0;
        }
    else
        {
        transformBarycentric3dSystemTo2d
            (pInstance, &newPoint, &newV1, &newV2, pVector1, pVector2);

        if (pVolume)
            *pVolume =
                bsiDPoint3d_tripleProduct (pInstance, pVector1, pVector2) / 6.0;
        }

    return bsiDPoint2d_inDPoint2dTriangleVectors
            (&newPoint, NULL, &newV1, &newV2, pBCoords, NULL);
    }



/*-----------------------------------------------------------------*//**
* @description Returns true if this instance is in the same plane as the triangle with vertex
* at pOrigin and spanned by vectors pVector1 and pVector2.
*
* @instance pInstance   <= point in space
* @param pOrigin        => vertex of triangle (may be null for origin)
* @param pVector1       => side of triangle (emanating from pOrigin)
* @param pVector2       => side of triangle (emanating from pOrigin)
* @see bsiDPoint3d_isCoplanar
* @return Whether pInstance is in the plane of the triangle.
* @group "DPoint3d Queries"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_isCoplanarVectors

(
DPoint3dCP pInstance,
DPoint3dCP pOrigin,
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    static double   s_relTol = 1.0e-12;
    double          dist2, maxCoord, maxCoordProj;
    DPoint3d        q, baryCoords, projection;
    DPoint2d        newPoint, newV1, newV2;

    /* Decrement dimension by projecting pInstance into plane of triangle */
    if (pOrigin)
        {
        /*
        Translating by pOrigin compresses the triangle definition while
        preserving barycentric coords.
        */
        q.x  = pInstance->x - pOrigin->x;
        q.y  = pInstance->y - pOrigin->y;
        q.z  = pInstance->z - pOrigin->z;
        transformBarycentric3dSystemTo2d
            (&q, &newPoint, &newV1, &newV2, pVector1, pVector2);
        }
    else
        transformBarycentric3dSystemTo2d
            (pInstance, &newPoint, &newV1, &newV2, pVector1, pVector2);

    /* Find barycentric coordinates of projection */
    if (bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
            (&baryCoords, &newPoint, NULL, &newV1, &newV2))
        {
        /* Compute projection onto plane of triangle */
        bsiDPoint3d_fromBarycentricAndDPoint3dTriangleVectors
                (&projection, &baryCoords, pOrigin, pVector1, pVector2);

        /* Find tolerance multiplier */
        maxCoordProj = bsiDPoint3d_maxAbs (&projection);
        maxCoord = bsiDPoint3d_maxAbs (pInstance);
        if (maxCoordProj > maxCoord)
            maxCoord = maxCoordProj;

        /* Compute square of projection distance */
        dist2 = bsiDPoint3d_distanceSquared (pInstance, &projection);

        /* distance within tolerance of zero, then coplanar */
        if (dist2 <= s_relTol * s_relTol * maxCoord * maxCoord)
            return true;
        }

    return false;
    }



/*-----------------------------------------------------------------*//**
* @description Returns true if this instance is in the same plane as the triangle with
* vertices pVertex0, pVertex1, pVertex2.
*
* @instance pInstance   <= point in space
* @param pVertex0       => vertex of triangle
* @param pVertex1       => vertex of triangle
* @param pVertex2       => vertex of triangle
* @see bsiDPoint3d_isCoplanarVectors
* @return Whether pInstance is in the plane of the triangle.
* @group "DPoint3d Queries"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_isCoplanar

(
DPoint3dCP pInstance,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2
)
    {
    DPoint3d        q, v1, v2;

    /*
    Translating by pVertex0 compresses the triangle definition from 3 points
    to 2 vectors while preserving barycentric coords.
    */
    q.x  = pInstance->x - pVertex0->x;
    q.y  = pInstance->y - pVertex0->y;
    q.z  = pInstance->z - pVertex0->z;
    v1.x = pVertex1->x  - pVertex0->x;
    v1.y = pVertex1->y  - pVertex0->y;
    v1.z = pVertex1->z  - pVertex0->z;
    v2.x = pVertex2->x  - pVertex0->x;
    v2.y = pVertex2->y  - pVertex0->y;
    v2.z = pVertex2->z  - pVertex0->z;

    return bsiDPoint3d_isCoplanarVectors (&q, NULL, &v1, &v2);
    }

/*----------------------------------------------------------------------------+
| 2D BARYCENTRIC COORDINATE FUNCTIONS:
+----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------*//**
* @description Sets this instance to the point in the plane with the given barycentric
* coordinates relative to triangle T (pVertex0, pVertex1, pVertex2).
*
* @instance pInstance   <= point with given barycoords relative to T
* @param pBaryCoords    => barycentric coordinates relative to T
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @group "DPoint2d Barycentric"
* @bsihdr                                                               DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_fromBarycentricAndDPoint2dTriangle

(
DPoint2dP pInstance,
DPoint3dCP pBaryCoords,
DPoint2dCP pVertex0,
DPoint2dCP pVertex1,
DPoint2dCP pVertex2
)
    {
    pInstance->x =  pBaryCoords->x * pVertex0->x +
                    pBaryCoords->y * pVertex1->x +
                    pBaryCoords->z * pVertex2->x;
    pInstance->y =  pBaryCoords->x * pVertex0->y +
                    pBaryCoords->y * pVertex1->y +
                    pBaryCoords->z * pVertex2->y;
    }


/*-----------------------------------------------------------------*//**
* @description Sets this instance to the point in the plane of triangle T
* (pOrigin, pVector1-pOrigin, pVector2-pOrigin) with the given barycentric
* coordinates relative to T.
*
* @instance pInstance   <= point in plane of T with given barycoords
* @param pBaryCoords    => barycentric coordinates relative to T
* @param pOrigin        => vertex of triangle T (may be null for origin)
* @param pVector1       => side vector of T (emanating from pOrigin)
* @param pVector2       => side vector of T (emanating from pOrigin)
* @see bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
* @group "DPoint2d Barycentric"
* @bsihdr                                                               DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_fromBarycentricAndDPoint2dTriangleVectors

(
DPoint2dP pInstance,
DPoint3dCP pBaryCoords,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
DPoint2dCP pVector2
)
    {
    pInstance->x =  pBaryCoords->y * pVector1->x +
                    pBaryCoords->z * pVector2->x;
    pInstance->y =  pBaryCoords->y * pVector1->y +
                    pBaryCoords->z * pVector2->y;

    if (pOrigin)
        {
        /* q = b0*v0 + b1(v0+v1) + b2(v0+v2) = v0 + b1v1 + b2v2 */
        pInstance->x += pOrigin->x;
        pInstance->y += pOrigin->y;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Returns true if this instance is on or inside the triangle T spanned by
* vectors pVector1 and pVector2.  Also optionally returns the barycentric
* coordinates of this instance (if inside T) and the signed area of T, which
* is nonnegative if the given points are ordered counterclockwise.
*
* @instance pInstance   => point in the xy-plane
* @param pOrigin        => vertex of T (can be null for origin)
* @param pVector1       => side of T (emanating from pOrigin)
* @param pVector2       => side of T (emanating from pOrigin)
* @param pBCoords       <= barycentric coordinates of pInstance if inside T (or null)
* @param pArea          <= signed area of T (or null)
* @see bsiDPoint3d_inDPoint2dTriangle
* @group "DPoint2d Barycentric"
* @return true if and only if T is sufficiently large
* @bsihdr                                                               DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint2d_inDPoint2dTriangleVectors

(
DPoint2dCP pInstance,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
DPoint2dCP pVector2,
DPoint3dP pBCoords,
double                  *pArea
)
    {
    static double   s_relTol = 1.0e-12;
    double          a0, a1, a2, sumA, minA, maxA, sumArecip;

    /*
    Compute numerators of barycentric coordinates of pInstance relative to the
    triangle T: if their signs are the same, then pInstance lies inside T.
    */
    if (pOrigin)
        {
        /*
        Since barycoords are invariant under affine transformations, we can
        translate pInstance (and T) by pOrigin so that the 3x3 linear system
        for finding the barycoords becomes 2x2.
        */
        a1 = (pInstance->x - pOrigin->x) * pVector2->y -
             (pInstance->y - pOrigin->y) * pVector2->x;
        a2 = (pInstance->y - pOrigin->y) * pVector1->x -
             (pInstance->x - pOrigin->x) * pVector1->y;
        }
    else
        {
        a1 = pInstance->x * pVector2->y - pInstance->y * pVector2->x;
        a2 = pInstance->y * pVector1->x - pInstance->x * pVector1->y;
        }

    /*
    Compute barycoord numerator sum (= twice signed area of T) and barycoord
    numerator corresponding to pOrigin
    */
    sumA =
    a0 = pVector1->x * pVector2->y - pVector2->x * pVector1->y;
    a0 -= a1 + a2;

    if (pArea)
        *pArea = sumA / 2.0;

    /* Find barycoord min/max (non-absolute) */
    minA = maxA = a0;

    if (a1 < minA)
        minA = a1;
    else
        maxA = a1;

    if (a2 < minA)
        minA = a2;
    else if (a2 > maxA)
        maxA = a2;

    /*
    If T is degenerate (a line segment or point with area identically zero)
    then return true only if pInstance is on the line segment or is the point;
    barycoords not returned since they don't exist.
    */
    if (!sumA)
        {
        if (- s_relTol <= minA && maxA <= s_relTol)
            return true;
        }

    /* If min and max have same sign, then pInstance is inside closure of T. */
    else if (minA * maxA >= - s_relTol * sumA * sumA)
        {
        if (pBCoords)
            {
            /*
            Now we don't have to worry about sumA being too small:  since
            pInstance is inside closure of T, then WLOG, |a0| <= |sumA|.  Thus
            the inequality |sumA| <= s_relTol * |a0| <= s_relTol * |sumA|
            implies 1 <= s_relTol, a contradiction.  But this inequality is
            precisely the one which would cause bsiTrig_safeDivide to fail.
            */
            sumArecip = 1.0 / sumA;
            pBCoords->x = a0 * sumArecip;
            pBCoords->y = a1 * sumArecip;
            pBCoords->z = 1.0 - pBCoords->x - pBCoords->y;
            }

        return true;
        }

    return false;
    }


/*-----------------------------------------------------------------*//**
* @description Returns true if this instance is on or inside the triangle T with vertices
* pVertex0, pVertex1, pVertex2.  Also optionally returns the barycentric
* coordinates of this instance (if inside T) and twice the signed area
* of T, which is nonnegative if the ordering (pVertex0, pVertex1, pVertex2) is
* counter-clockwise.
*
* @instance pInstance   => point in the xy-plane
* @param pVertex0       => vertex 0 of T
* @param pVertex1       => vertex 1 of T
* @param pVertex2       => vertex 2 of T
* @param pBCoords       <= barycentric coordinates of pInstance if inside T (or null)
* @param pArea          <= twice the signed area of T (or null)
* @see bsiDPoint3d_inDPoint2dTriangleVectors
* @group "DPoint2d Barycentric"
* @return true if pInstance is on/inside T.
* @bsihdr                                                               DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint2d_inDPoint2dTriangle

(
DPoint2dCP pInstance,
DPoint2dCP pVertex0,
DPoint2dCP pVertex1,
DPoint2dCP pVertex2,
DPoint3dP pBCoords,
double                  *pArea
)
    {
    DPoint2d        q, v1, v2;

    /*
    Since barycoords are invariant under affine transformations, we can
    translate pInstance and the triangle T by one of its vertices so that
    the 3x3 linear system for the barycoords becomes 2x2.
    */
    q.x = pInstance->x - pVertex0->x;
    q.y = pInstance->y - pVertex0->y;
    v1.x = pVertex1->x - pVertex0->x;
    v1.y = pVertex1->y - pVertex0->y;
    v2.x = pVertex2->x - pVertex0->x;
    v2.y = pVertex2->y - pVertex0->y;

    return bsiDPoint2d_inDPoint2dTriangleVectors
                (&q, NULL, &v1, &v2, pBCoords, pArea);
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute the barycentric coordinates of a point in the plane relative to a convex polygon.
*
* @remarks The points in the input array are assumed to bound a convex planar region.
* @remarks The returned barycentric coordinates reproduce the given point (e.g., in ~mbsiDPoint2dArray_linearCombination).
*
* @param pBaryCoords    <= barycentric coordinates of pPoint relative to P (allocated by caller with space for numVertices doubles)
* @param pPoint         => point in space
* @param pVertices      => vertices of polygon P (first, last vertices distinct)
* @param numVertices    => number of vertices of P
* @return true if and only if valid inputs, sufficient memory, and the area of P is sufficiently large.
* @group "DPoint2d Barycentric"
* @bsimethod                                                    DavidAssaf      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint2d_barycentricFromDPoint2dConvexPolygon

(
double*     pBaryCoords,
DPoint2dCP    pPoint,
DPoint2dCP    pVertices,
int         numVertices
)
    {
    // Construction based on "Barycentric Coordinates for Convex Sets",
    // by Warren, Schaefer, Hirani and Desbrun, submitted to CAGD (2003)

    // pVertices[i] lies between pEdge[i-1] and pEdge[i]

    DVec2d  diff, normal, prevNormal, lastNormal;
    double  signZ, dist, prevDist, lastDist, area, coord, denom = 0.0, defaultCoord;
    bool    normalDegen, prevNormalDegen, lastNormalDegen;
    bool    useRightHandRule = true;    // default for ccw polygon
    int     i;

    lastNormal.Zero ();
    lastNormalDegen = false;
    lastDist = 0.0;
    if (!pBaryCoords || !pPoint || !pVertices || numVertices <= 2)
        return false;

    // optimize for triangle
    if (3 == numVertices)
        return bsiDPoint3d_barycentricFromDPoint2dTriangle ((DPoint3d*) pBaryCoords, pPoint, &pVertices[0], &pVertices[1], &pVertices[2]);

    // check first nontrivial local orientation of convex polygon so we can ensure that normals point outward
    for (i = 0; i < numVertices - 2; i++)
        {
        signZ = bsiDPoint2d_crossProduct3DPoint2d (&pVertices[i + 1], &pVertices[i + 2], &pVertices[i]);
        if (signZ)
            {
            useRightHandRule = signZ > 0.0;
            break;
            }
        }

    // compute normal and signed distance-to-pPoint of last nondegenerate edge
    for (i = numVertices - 1; i > 0; i--)
        {
        bsiDPoint2d_subtractDPoint2dDPoint2d (&diff, &pVertices[0], &pVertices[i]);
        lastNormalDegen = !bsiDPoint2d_unitPerpendicularWithHandedness (&lastNormal, &diff, useRightHandRule);
        lastDist = bsiDPoint2d_dotDifference (&pVertices[i], pPoint, &lastNormal);
        if (!lastNormalDegen)
            break;
        pBaryCoords[i] = 0.0;
        numVertices--;
        }

    // polygon points are identical
    if (numVertices <= 1)
        {
        pBaryCoords[0] = 1.0;
        return false;
        }

    prevNormalDegen = lastNormalDegen;
    prevNormal = lastNormal;
    prevDist = lastDist;

    for (i = 0; i < numVertices; i++)
        {
        // compute normal and signed distance-to-pPoint of this edge
        if (i < numVertices - 1)
            {
            bsiDPoint2d_subtractDPoint2dDPoint2d (&diff,  &pVertices[i + 1], &pVertices[i]);
            normalDegen = !bsiDPoint2d_unitPerpendicularWithHandedness (&normal, &diff, useRightHandRule);
            dist = bsiDPoint2d_dotDifference (&pVertices[i], pPoint, &normal);
            }
        else
            {
            normalDegen = lastNormalDegen;
            normal = lastNormal;
            dist = lastDist;
            }

        // this edge is degenerate; set zero coord but don't update previous data
        if (normalDegen)
            {
            pBaryCoords[i] = 0.0;
            continue;
            }

        // compute area of normal parallelogram
        area = fabs (bsiDPoint2d_crossProduct (&prevNormal, &normal));

        // if either distance is close to zero, we're on an edge; nonadjacent vertices' coords are zero
        if (!bsiTrig_safeDivide (&coord, area, prevDist * dist, 0.0))
            {
            DSegment3d  edge;
            DPoint3d    point;
            double      prevDistAbs = fabs (prevDist), distAbs = fabs (dist);
            int         i0, i1;

            memset (pBaryCoords, 0, numVertices * sizeof (*pBaryCoords));

            if (prevDistAbs < distAbs)
                {
                // pPoint is "on" previous edge
                i0 = i > 0 ? i - 1 : numVertices - 1;
                i1 = i;
                }
            else if (prevDistAbs > distAbs)
                {
                // pPoint is "on" this edge
                i0 = i;
                i1 = i < numVertices - 1 ? i + 1 : 0;
                }
            else
                {
                // pPoint is "on" the vertex between this & previous edges
                pBaryCoords[i] = 1.0;
                return true;
                }

            bsiDSegment3d_initFromDPoint2d (&edge, &pVertices[i0], &pVertices[i1]);
            bsiDPoint3d_initFromDPoint2d (&point, pPoint);
            bsiDSegment3d_dPoint3dToFractionParameter (&edge, &coord, &point);
            pBaryCoords[i0] = coord;
            pBaryCoords[i1] = 1.0 - coord;
            return true;
            }

        // unnormalized barycentric coordinate
        pBaryCoords[i] = coord;

        // accumulate normalization factor
        denom += pBaryCoords[i];

        prevNormalDegen = normalDegen;
        prevNormal = normal;
        prevDist = dist;
        }

    // normalize the barycentric coordinates
    defaultCoord = 1.0 / numVertices;
    for (i = 0; i < numVertices; i++)
        {
        if (!bsiTrig_safeDivide (&pBaryCoords[i], pBaryCoords[i], denom, defaultCoord))
            assert (false);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute the barycentric coordinates of a point relative to a convex polygon.
*
* @remarks The points in the input array will be flattened to an approximating plane.  The
*       barycentric coordinates returned are for the projection of the given point into this plane.
*       Thus the returned barycentric coordinates only reproduce the given point (e.g., in
*       ~mbsiDPoint3dArray_linearCombination) if the given polygon is planar and the given point
*       is in this plane.
* @param pBaryCoords    <= barycentric coordinates of pPoint relative to P (allocated by caller with space for numVertices doubles)
* @param pPoint         => point in space
* @param pVertices      => vertices of polygon P (first, last vertices distinct)
* @param numVertices    => number of vertices of P
* @return true if and only if valid inputs, sufficient memory, and the area of P is sufficiently large.
* @group "DPoint3d Barycentric"
* @bsimethod                                                    DavidAssaf      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint3d_barycentricFromDPoint3dConvexPolygon

(
double*     pBaryCoords,
DPoint3dCP    pPoint,
DPoint3dCP    pVertices,
int         numVertices
)
    {
    Transform   worldToPlane;
    DPoint2d*   pPlaneVerts = NULL, planePt;
    bool        bResult = false;

    if (!pBaryCoords || !pPoint || !pVertices || numVertices <= 2)
        return false;

    // optimize for triangle
    if (3 == numVertices)
        return bsiDPoint3d_barycentricFromDPoint3dTriangle ((DPoint3d*) pBaryCoords, pPoint, &pVertices[0], &pVertices[1], &pVertices[2]);

    // working array
    if (!(pPlaneVerts = (DPoint2d*) _alloca (numVertices * sizeof (*pPlaneVerts))))
        return false;

    // flatten so we can ignore z
    if (bsiDPoint3dArray_transformToPlane (NULL, &worldToPlane, NULL, NULL, pVertices, numVertices))
        {
        bsiTransform_multiplyDPoint3dArrayTo2d (&worldToPlane, pPlaneVerts, pVertices, numVertices);
        bsiTransform_multiplyDPoint3dArrayTo2d (&worldToPlane, &planePt, pPoint, 1);

        bResult = bsiDPoint2d_barycentricFromDPoint2dConvexPolygon (pBaryCoords, &planePt, pPlaneVerts, numVertices);
        }

    return bResult;
    }

/**
* @description Append three points to an output buffer.
* @param pOutputTriangle IN OUT Array of (3 * numTriangleOut) points of the three triangles.
* @param pNumTriangleOut IN OUT number of split triangles.
* @param pVertex0 IN vertex 0
* @param pVertex1 IN vertex 1
* @param pVertex2 IN vertex 2
*/
static void addTriangle

(
DPoint3dP pOutputTriangleBuffer,
int *pNumTriangleOut,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2
)
    {
    int i = 3 * (*pNumTriangleOut);
    pOutputTriangleBuffer[i++] = *pVertex0;
    pOutputTriangleBuffer[i++] = *pVertex1;
    pOutputTriangleBuffer[i++] = *pVertex2;
    *pNumTriangleOut += 1;
    }


/**
* @description Generate 1 or 2 triangles as needed, based on projection of space point to vertex0,
*       edge 01, or edge 12.  (i.e. the case where the space point is in the vertex region between
*       edge 01 and 02 -- either the vertex or one of the edges
*/
static void splitToVertexOrEdge

(
DPoint3dP pOutputTriangleBuffer,
int *pNumTriangleOut,
DPoint3dCP pSpacePoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2
)
    {
    DPoint3d vectorU, vectorV;
    DPoint3d edgePoint;
    double uv, uu;
    double fraction;
    /* Form outgoing vectors from vertex 1 towards vertex 0.
        Positive dot between this vector and space vector means only this edge
        to be considered.
    */
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorU, pVertex0, pVertex1);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorV, pSpacePoint, pVertex1);
    uv = bsiDPoint3d_dotProduct (&vectorU, &vectorV);
    if (uv > 0.0)
        {
        /* Project within or beyond vector to vertex0 */
        uu = bsiDPoint3d_dotProduct (&vectorU, &vectorU);
        if (uv >= uu)
            addTriangle (pOutputTriangleBuffer, pNumTriangleOut, pVertex0, pVertex1, pVertex2);
        else
            {
            fraction = uv / uu;
            bsiDPoint3d_addScaledDPoint3d (&edgePoint, pVertex1, &vectorU, fraction);
            addTriangle (pOutputTriangleBuffer, pNumTriangleOut, &edgePoint, pVertex1, pVertex2);
            addTriangle (pOutputTriangleBuffer, pNumTriangleOut, &edgePoint, pVertex2, pVertex0);
            }
        }
    else
        {
        /* Consider the other edge ... */
        bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorU, pVertex2, pVertex1);
        uv = bsiDPoint3d_dotProduct (&vectorU, &vectorV);
        if (uv < 0.0)
            addTriangle (pOutputTriangleBuffer, pNumTriangleOut, pVertex1, pVertex2, pVertex0);
        else
            {
            uu = bsiDPoint3d_dotProduct (&vectorU, &vectorU);
            if (uv >= uu)
                addTriangle (pOutputTriangleBuffer, pNumTriangleOut, pVertex2, pVertex1, pVertex0);
            else
                {
                fraction = uv / uu;
                bsiDPoint3d_addScaledDPoint3d (&edgePoint, pVertex1, &vectorU, fraction);
                addTriangle (pOutputTriangleBuffer, pNumTriangleOut, &edgePoint, pVertex2, pVertex0);
                addTriangle (pOutputTriangleBuffer, pNumTriangleOut, &edgePoint, pVertex0, pVertex1);
                }
            }
        }
    }


/**
* @description Generate 1 or 2 triangles as needed, based on projection of space point to edge 01.
*/
static void splitToEdge

(
DPoint3dP pOutputTriangleBuffer,
int *pNumTriangleOut,
DPoint3dCP pSpacePoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2
)
    {
    DPoint3d vectorU, vectorV;
    DPoint3d edgePoint;
    double uu, uv;
    double fraction;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorU, pVertex1, pVertex0);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorV, pSpacePoint, pVertex0);
    uu = bsiDPoint3d_dotProduct (&vectorU, &vectorU);
    uv = bsiDPoint3d_dotProduct (&vectorU, &vectorV);
    if (uv <= 0.0)
        addTriangle (pOutputTriangleBuffer, pNumTriangleOut, pVertex0, pVertex1, pVertex2);
    else if (uv >= uu)
        addTriangle (pOutputTriangleBuffer, pNumTriangleOut, pVertex1, pVertex2, pVertex0);
    else
        {
        fraction = uv / uu;
        bsiDPoint3d_addScaledDPoint3d (&edgePoint, pVertex0, &vectorU, fraction);
        addTriangle (pOutputTriangleBuffer, pNumTriangleOut, &edgePoint, pVertex1, pVertex2);
        addTriangle (pOutputTriangleBuffer, pNumTriangleOut, &edgePoint, pVertex2, pVertex0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute 1, 2, or 3 triangles to be used for integrating from a space point to
* a triangle, splitting the triangle so that the closest point of the original triangle
* (whether or not it is a vertex) is vertex 0 of each subdivision triangle, and is the closest
* point of that triangle.
* @param pOutputTriangleBuffer  OUT     Array of (3 * numTriangleOut) points of the three triangles.  Up to 9 points may be returned.
* @param pNumTriangleOut        OUT     number of split triangles.
* @param pSpacePoint            IN      point to project to plane of triangle
* @param pVertex0               IN      vertex of original triangle.
* @param pVertex1               IN      vertex of original triangle.
* @param pVertex2               IN      vertex of original triangle.
* @return true if triangle is nondegenerate
* @group Polygons
* @bsimethod                                                    EarlinLutz      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_splitDPoint3dTriangleForIntegration

(
DPoint3dP pOutputTriangleBuffer,
int *pNumTriangleOut,
DPoint3dCP pSpacePoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2
)
    {
    DPoint3d vector01, vector02, vectorZ;
    Transform localToWorld, worldToLocal;
    DPoint3d localPoint;
    int signMask;
    DPoint3d planePoint, barycentricPoint;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector01, pVertex1, pVertex0);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector02, pVertex2, pVertex0);
    bsiDPoint3d_normalizedCrossProduct (&vectorZ, &vector01, &vector02);
    bsiTransform_initFromOriginAndVectors (&localToWorld, pVertex0,
            &vector01, &vector02, &vectorZ);

    *pNumTriangleOut = 0;
    if (!bsiTransform_invertTransform (&worldToLocal, &localToWorld))
    return false;

    bsiTransform_multiplyDPoint3d (&worldToLocal, &localPoint, pSpacePoint);
    bsiDPoint3d_add2ScaledDPoint3d
                        (
                        &planePoint,
                        pVertex0,
                        &vector01, localPoint.x,
                        &vector02, localPoint.y
                        );

    barycentricPoint = localPoint;
    barycentricPoint.z = 1.0 - barycentricPoint.y - barycentricPoint.x;

#define XBIT 0x01
#define YBIT 0x02
#define ZBIT 0x04
#define XYBITS 0x03
#define XZBITS 0x05
#define YZBITS 0x06
#define XYZBITS 0x07

    signMask = 0;
    if (barycentricPoint.x > 0.0)
        signMask |= XBIT;
    if (barycentricPoint.y > 0.0)
        signMask |= YBIT;
    if (barycentricPoint.z > 0.0)
        signMask |= ZBIT;

    switch (signMask)
        {
        case XYZBITS:
            addTriangle (pOutputTriangleBuffer, pNumTriangleOut, &planePoint, pVertex0, pVertex1);
            addTriangle (pOutputTriangleBuffer, pNumTriangleOut, &planePoint, pVertex1, pVertex2);
            addTriangle (pOutputTriangleBuffer, pNumTriangleOut, &planePoint, pVertex2, pVertex0);
            break;
        case XBIT:
            splitToVertexOrEdge (pOutputTriangleBuffer, pNumTriangleOut, pSpacePoint, pVertex0, pVertex1, pVertex2);
            break;
        case YBIT:
            splitToVertexOrEdge (pOutputTriangleBuffer, pNumTriangleOut, pSpacePoint, pVertex1, pVertex2, pVertex0);
            break;
        case ZBIT:
            splitToVertexOrEdge (pOutputTriangleBuffer, pNumTriangleOut, pSpacePoint, pVertex2, pVertex0, pVertex1);
            break;
        case XYBITS:
            splitToEdge (pOutputTriangleBuffer, pNumTriangleOut, pSpacePoint, pVertex1, pVertex2, pVertex0);
            break;
        case XZBITS:
            splitToEdge (pOutputTriangleBuffer, pNumTriangleOut, pSpacePoint, pVertex0, pVertex1, pVertex2);
            break;
        case YZBITS:
            splitToEdge (pOutputTriangleBuffer, pNumTriangleOut, pSpacePoint, pVertex2, pVertex0, pVertex1);
            break;
        }
    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
