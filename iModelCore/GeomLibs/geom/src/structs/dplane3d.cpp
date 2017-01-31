/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dplane3d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*-----------------------------------------------------------------*//**
* @struct DPlane3d
* A DPlane3d represents a 3D plane defined by an origin and normal vector.
* The normal vector is not necessarily a unit vector.
* @fields
* @field DPoint3d origin Any reference (origin) point on the plane.
* @field DPoint3d normal Vector normal to the plane.
* @endfields
* @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/


/*-----------------------------------------------------------------*//**
* @description Store origin and unnormalized vector.
* @param pPlane <= initialized plane
* @param x0 => x-coordinate of origin point
* @param y0 => y-coordinate of origin point
* @param z0 => z-coordinate of origin point
* @param ux => x-coordinate of normal vector
* @param uy => y-coordinate of normal vector
* @param uz => z-coordinate of normal vector
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPlane3d_initFromOriginAndNormalXYZXYZ

(
DPlane3dP pPlane,
double      x0,
double      y0,
double      z0,
double      ux,
double      uy,
double      uz
)
    {
    pPlane->origin.x = x0;
    pPlane->origin.y = y0;
    pPlane->origin.z = z0;

    pPlane->normal.x = ux;
    pPlane->normal.y = uy;
    pPlane->normal.z = uz;
    }


/*-----------------------------------------------------------------*//**
* @description Store origin and unnormalized vector.
* @param pPlane <= initialized plane.
* @param pOrigin => origin point
* @param pNormal => normal vector
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPlane3d_initFromOriginAndNormal

(
DPlane3dP pPlane,
DPoint3dCP pOrigin,
DVec3dCP pNormal
)
    {
    pPlane->origin = *pOrigin;
    pPlane->normal = *pNormal;
    }


/*-----------------------------------------------------------------*//**
* @description Normalize the plane vector.
* @param pPlane <=> plane to normalize
* @return true if normal vector has nonzero length.
* @group "DPlane3d Modification"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPlane3d_normalize

(
DPlane3dP pPlane
)
    {
    double mag = bsiDPoint3d_normalizeInPlace (&pPlane->normal);
    return  mag != 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Initialize with first point as origin, normal as unnormalized cross product of vectors
*   to 2nd and 3rd points.
* @param pPlane <= initialized plane
* @param pOrigin => origin point
* @param pXPoint => first point in plane (e.g., x-axis point)
* @param pYPoint => second point in plane (e.g., y-axis point)
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPlane3d_initFrom3DPoint3d

(
DPlane3dP pPlane,
DPoint3dCP pOrigin,
DPoint3dCP pXPoint,
DPoint3dCP pYPoint
)
    {
    DVec3d normal;
    bsiDVec3d_crossProduct3DPoint3d (&normal, pOrigin, pXPoint, pYPoint);
    bsiDPlane3d_initFromOriginAndNormal (pPlane, pOrigin, &normal);
    }


/*-----------------------------------------------------------------*//**
* @description Extract origin and normal from 4D plane coefficients.
* @param pPlane <= plane structure with origin, normal
* @param pHPlane => 4D plane coefficients
* @return true if plane has a nonzero normal
* @see bsiDPlane3d_getDPoint4d
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPlane3d_initFromDPoint4d

(
DPlane3dP pPlane,
DPoint4dCP pHPlane
)
    {
    double aa = pHPlane->x * pHPlane->x + pHPlane->y * pHPlane->y + pHPlane->z * pHPlane->z;
    double fOrigin, fNormal;
    bool    normalOK = false;
    if (aa != 0.0)
        {
        fOrigin = -pHPlane->w / aa;
        fNormal = 1.0 / sqrt (aa);
        bsiDPoint3d_setXYZ (&pPlane->origin,
                        pHPlane->x * fOrigin,
                        pHPlane->y * fOrigin,
                        pHPlane->z * fOrigin);
        bsiDPoint3d_setXYZ (&pPlane->normal,
                        pHPlane->x * fNormal,
                        pHPlane->y * fNormal,
                        pHPlane->z * fNormal);
        normalOK = true;
        }
    else
        {
        bsiDPlane3d_initFromOriginAndNormalXYZXYZ (pPlane, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        }
    return  normalOK;
    }


/*-----------------------------------------------------------------*//**
* @description Return the plane as a DPoint4d.
* @param pPlane => plane structure with origin, normal
* @param pHPlane <= 4D plane coefficients
* @see bsiDPlane3d_initFromDPoint4d
* @group "DPlane3d Queries"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bsiDPlane3d_getDPoint4d

(
DPlane3dCP pPlane,
DPoint4dP pHPlane
)
    {
    bsiDPlane3d_getImplicitPlaneCoefficients
                            (
                            pPlane,
                            &pHPlane->x,
                            &pHPlane->y,
                            &pHPlane->z,
                            &pHPlane->w
                            );
    pHPlane->w = -pHPlane->w;
    }


/*-----------------------------------------------------------------*//**
* @description Convert the implicit plane ax+by+cz=d to origin-normal form, with a unit normal vector.
* @remarks WARNING: Check your usage.  It is about equally common to write the plane equation with
*       negated d, i.e. ax+by+cz+d=0.  If so, pass in (a,b,c,-d).
* @param pPlane <= plane structure with origin, normal
* @param a => 4D plane x-coefficient
* @param b => 4D plane y-coefficient
* @param c => 4D plane z-coefficient
* @param d => 4D plane constant coefficient
* @return true if plane has a nonzero normal
* @see bsiDPlane3d_getImplicitPlaneCoefficients
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPlane3d_initFromImplicitPlaneCoefficients

(
DPlane3dP pPlane,
double      a,
double      b,
double      c,
double      d
)
    {
    DPoint4d hCoffs;
    bsiDPoint4d_setComponents (&hCoffs, a, b, c, -d);
    return bsiDPlane3d_initFromDPoint4d (pPlane, &hCoffs);
    }


/*-----------------------------------------------------------------*//**
* @description Convert the plane to implicit coeffcients ax+by+cz=d.
* @remarks WARNING: Check your usage.  It is about equally common to write the plane equation with
*       negated d, i.e. ax+by+cz+d=0.  If so, pass in (a,b,c,-d).
* @param pPlane => plane structure with origin, normal
* @param pA <= 4D plane x-coefficient
* @param pB <= 4D plane y-coefficient
* @param pC <= 4D plane z-coefficient
* @param pD <= 4D plane constant coefficient
* @see bsiDPlane3d_initFromImplicitPlaneCoefficients
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPlane3d_getImplicitPlaneCoefficients

(
DPlane3dCP pPlane,
double      *pA,
double      *pB,
double      *pC,
double      *pD
)
    {
    *pA = pPlane->normal.x;
    *pB = pPlane->normal.y;
    *pC = pPlane->normal.z;
    *pD = bsiDPoint3d_dotProduct (&pPlane->origin, &pPlane->normal);
    }


/*-----------------------------------------------------------------*//**
* @description Apply a transformation to the source plane.
* @param pDest <= transformed plane
* @param pTransform => transformation to apply
* @param pSource => source plane
* @group "DPlane3d Transform"
* @return false if the matrix part of the transform is singular.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPlane3d_multiplyTransformDPlane3d
(
DPlane3dP pDest,
TransformCP pTransform,
DPlane3dCP pSource
)
    {
    RotMatrix matrix;
    /* Origin transforms as a point */
    bsiTransform_multiplyDPoint3d (pTransform, &pDest->origin, &pSource->origin);
    /* Normal transforms as the inverse transpose of the matrix part. */
    /* BTW: If the matrix is orthogonal, this is a long way to multiply by the
         matrix part.  UGH. */
    bsiTransform_getMatrix (pTransform, &matrix);
    return bsiRotMatrix_solveDPoint3dTranspose (&matrix, &pDest->normal, &pSource->normal);
    }



/*-----------------------------------------------------------------*//**
* @description Fill the plane data with zeros.
* @param pDest <= initialized plane
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPlane3d_zero

(
DPlane3dP pDest
)
    {
    bsiDPoint3d_zero (&pDest->origin);
    bsiDPoint3d_zero (&pDest->normal);
    }


/*-----------------------------------------------------------------*//**
* @description Test if the numeric entries in the plane are all absolutely zero (no tolerances).
* @param pPlane => plane to query
* @return true if the plane contains only zero coordinates.
* @group "DPlane3d Queries"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPlane3d_isZero

(
DPlane3dCP pPlane
)
    {
    return
            pPlane->origin.x == 0.0
        &&  pPlane->origin.y == 0.0
        &&  pPlane->origin.z == 0.0
        &&  pPlane->normal.x == 0.0
        &&  pPlane->normal.y == 0.0
        &&  pPlane->normal.z == 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Project a (generally off-plane) point onto the plane.
* @param pPlane => plane to query
* @param pProjection <= projection of pPoint onto the plane
* @param pPoint => point to project to plane
* @return true if the plane has a well defined normal.
* @group "DPlane3d Projection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPlane3d_projectPoint

(
DPlane3dCP pPlane,
DPoint3dP pProjection,
DPoint3dCP pPoint
)
    {
    DPoint3d V;
    double UdotU, UdotV, s = 0;
    bool    result;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&V, pPoint, &pPlane->origin);
    UdotU = bsiDPoint3d_dotProduct (&pPlane->normal, &pPlane->normal);
    UdotV = bsiDPoint3d_dotProduct (&pPlane->normal, &V);

    result = bsiTrig_safeDivide (&s, UdotV, UdotU, 0.0);
    bsiDPoint3d_addScaledDPoint3d (pProjection, pPoint, &pPlane->normal, -s);
    return result;
    }


/*-----------------------------------------------------------------*//**
* @description Dot the plane normal with the vector from the plane origin to the point.
* @remarks If the plane normal is a unit vector, this is the true distance from the
*       plane to the point.  If not, it is a scaled distance.
* @param pPlane => plane to evaluate
* @param pPoint => point for evaluation
* @return dot product
* @group "DPlane3d Projection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double  bsiDPlane3d_evaluate

(
DPlane3dCP pPlane,
DPoint3dCP pPoint
)
    {
    return bsiDPoint3d_dotDifference (pPoint, &pPlane->origin, &pPlane->normal);
    }


/*-----------------------------------------------------------------*//**
* @description Compute the origin and normal so the plane passes (approximiately) through the array of points.
* @param pPlane <= approximating plane
* @param pPointArray => array of points defining the plane
* @param numPoint => number of points in the array
* @return true if the points define a clear plane; false if every point lies on the line joining the two extremal points.
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPlane3d_initFromDPoint3dArray

(
DPlane3dP pPlane,
DPoint3dCP pPointArray,
int             numPoint
)
    {
    bool    result = bsiGeom_planeThroughPoints
                            (
                            &pPlane->normal,
                            &pPlane->origin,
                            pPointArray,
                            numPoint
                            );
    return result;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
