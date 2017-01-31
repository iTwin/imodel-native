/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dpoint3ddvec3d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*//**
* @description @struct DPoint3d
* A DPoint3d structure holds cartesian components of a 3D vector or point.
* @fields
* @field double x x component of point or vector
* @field double y y component of point or vector
* @field double z z component of point or vector
* @endfields
* @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/* VBSUB(Point3dFromVec3d) */

/*-----------------------------------------------------------------*//**
* @description Initialize a point by copying x,y,z from a vector.
* @param pPoint OUT The point
* @param pVector IN The vecotr
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_initFromDVec3d

(
DPoint3dP pPoint,
DVec3dCP pVector
)
    {
    pPoint->x = pVector->x;
    pPoint->y = pVector->y;
    pPoint->z = pVector->z;
    }


/* VBSUB(Point3dAddPoint3dVec3d) */
/* CSVFUNC(Add) */

/*-----------------------------------------------------------------*//**
* @description Compute the sum of a point and a vector
*
* @instance pSum OUT The computed sum.
* @param pBase IN The the first point or vector
* @param pVector IN The second point or vector
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_addDPoint3dDVec3d

(
DPoint3dP pSum,
DPoint3dCP pBase,
DVec3dCP pVector
)
    {
    pSum->x = pBase->x + pVector->x;
    pSum->y = pBase->y + pVector->y;
    pSum->z = pBase->z + pVector->z;
    }

/* VBSUB(Point3dSubtractPoint3dVec3d) */
/* CSVFUNC(Subtract) */

/*-----------------------------------------------------------------*//**
* @description Subtract a vector from a point.
*
* @instance pSum OUT The computed sum.
* @param pBase IN The the first point or vector
* @param pVector IN The second point or vector
* @bsihdr                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_subtractDPoint3dDVec3d

(
DPoint3dP pSum,
DPoint3dCP pBase,
DVec3dCP pVector
)
    {
    pSum->x = pBase->x - pVector->x;
    pSum->y = pBase->y - pVector->y;
    pSum->z = pBase->z - pVector->z;
    }




/*-----------------------------------------------------------------*//**
* @description Adds a vector to a pointer or vector, returns the result in place.
*
* @instance pSum <IN The point or vector to be modified.
* @param pVector IN The vector to add.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_addDVec3d

(
DPoint3dP pSum,
DVec3dCP pVector
)
    {
    pSum->x += pVector->x;
    pSum->y += pVector->y;
    pSum->z += pVector->z;
    }


/*-----------------------------------------------------------------*//**
* @description Subtract a vector from a point, returning the result in place of the point.
*
* @instance pVector2 <IN The point or vector to be modified.
* @param pVector2 IN The vector to subtract.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_subtractDVec3d

(
DPoint3dP pVector1,
DVec3dCP pVector2
)
    {
    pVector1->x -= pVector2->x;
    pVector1->y -= pVector2->y;
    pVector1->z -= pVector2->z;
    }
/* VBSUB(Point3dAddScaledVec3d) */
/* CSVFUNC(Add) */

/*-----------------------------------------------------------------*//**
* @description Adds an origin and a scaled vector.
*
* @instance pSum OUT The sum
* @param pOrigin IN Origin for the sum.
* @param pVector IN The vector to be added.
* @param scale IN The scale factor.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_addScaledDVec3d

(
DPoint3dP pSum,
DPoint3dCP pOrigin,
DVec3dCP pVector,
double           scale
)
    {
    if (pOrigin)
        {
        pSum->x = pOrigin->x + pVector->x * scale;
        pSum->y = pOrigin->y + pVector->y * scale;
        pSum->z = pOrigin->z + pVector->z * scale;
        }
    else
        {
        pSum->x = pVector->x * scale;
        pSum->y = pVector->y * scale;
        pSum->z = pVector->z * scale;
        }
    }

/* VBSUB(Point3dAdd2ScaledVec3d) */
/* CSVFUNC(Add) */

/*-----------------------------------------------------------------*//**
* @description Adds an origin and two scaled vectors.
*
* @instance pSum OUT sum
* @param pOrigin IN The origin.  May be null.
* @param pVector1 IN The first direction vector
* @param scale1 IN The first scale factor
* @param pVector2 IN The second direction vector
* @param scale2 IN The second scale factor
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_add2ScaledDVec3d

(
DPoint3dP pSum,
DPoint3dCP pOrigin,
DVec3dCP pVector1,
double           scale1,
DVec3dCP pVector2,
double           scale2
)
    {
    if (pOrigin)
        {
        pSum->x = pOrigin->x + pVector1->x * scale1 + pVector2->x * scale2;
        pSum->y = pOrigin->y + pVector1->y * scale1 + pVector2->y * scale2;
        pSum->z = pOrigin->z + pVector1->z * scale1 + pVector2->z * scale2;
        }
    else
        {
        pSum->x = pVector1->x * scale1 + pVector2->x * scale2;
        pSum->y = pVector1->y * scale1 + pVector2->y * scale2;
        pSum->z = pVector1->z * scale1 + pVector2->z * scale2;
        }
    }

/* VBSUB(Point3dAdd3ScaledVec3d) */
/* CSVFUNC(Add) */

/*-----------------------------------------------------------------*//**
* @description Adds an origin and three scaled vectors.
*
* @instance pSum OUT The sum.
* @param pOrigin IN The origin. May be null
* @param pVector1 IN The first direction vector
* @param scale1 IN The first scale factor
* @param pVector2 IN The second direction vector
* @param scale2 IN The second scale factor
* @param pVector3 IN The third direction vector
* @param scale3 IN The third scale factor
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_add3ScaledDVec3d

(
DPoint3dP pSum,
DPoint3dCP pOrigin,
DVec3dCP pVector1,
double          scale1,
DVec3dCP pVector2,
double          scale2,
DVec3dCP pVector3,
double          scale3
)
    {
    if  (pOrigin)
        {
        pSum->x = pOrigin->x + pVector1->x * scale1 + pVector2->x * scale2 + pVector3->x * scale3;
        pSum->y = pOrigin->y + pVector1->y * scale1 + pVector2->y * scale2 + pVector3->y * scale3;
        pSum->z = pOrigin->z + pVector1->z * scale1 + pVector2->z * scale2 + pVector3->z * scale3;
        }
    else
        {
        pSum->x = pVector1->x * scale1 + pVector2->x * scale2 + pVector3->x * scale3;
        pSum->y = pVector1->y * scale1 + pVector2->y * scale2 + pVector3->y * scale3;
        pSum->z = pVector1->z * scale1 + pVector2->z * scale2 + pVector3->z * scale3;
        }
    }





END_BENTLEY_GEOMETRY_NAMESPACE
