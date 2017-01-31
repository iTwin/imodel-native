/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/interfac/dpoint3dconvertedtodvec3d.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*//**
* @description @struct DPoint3d
* A DPoint3d structure holds cartesian components of a 3D vector
* @fields
* @field double x x component of vector
* @field double y y component of vector
* @field double z z component of vector
* @endfields
* @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE

/* VBSUB(Point3dCrossProduct) */
/* CSVFUNC(crossProduct) */

/*-----------------------------------------------------------------*//**
* @vbdescription Returns the cross (vector) cross product of two vectors.
* @instance pCrossProduct <= cross product of vector1 and vector2
* @param pVector1 => The first vector
* @param pVector2 => The second vector
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_crossProduct
(
DPoint3dP pCrossProduct,
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    double xx = pVector1->y * pVector2->z - pVector1->z * pVector2->y;
    double yy = pVector1->z * pVector2->x - pVector1->x * pVector2->z;
    double zz = pVector1->x * pVector2->y - pVector1->y * pVector2->x;
    pCrossProduct->x = xx;
    pCrossProduct->y = yy;
    pCrossProduct->z = zz;
    }

/* VBSUB(Point3dCrossProduct3Points) */
/* CSVFUNC(crossProductToPoints) */

/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
*   The vectors are computed from the Origin to Target1 and Target2.
* @instance pCrossProduct <= product
* @param pOrigin => The base point for computing vectors.
* @param pTarget1 => The target point for the first vector.
* @param pTarget2 => The target point for the second vector.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_crossProduct3DPoint3d
(
DPoint3dP pCrossProduct,
DPoint3dCP pOrigin,
DPoint3dCP pTarget1,
DPoint3dCP pTarget2
)
    {
    double x1 = pTarget1->x - pOrigin->x;
    double y1 = pTarget1->y - pOrigin->y;
    double z1 = pTarget1->z - pOrigin->z;

    double x2 = pTarget2->x - pOrigin->x;
    double y2 = pTarget2->y - pOrigin->y;
    double z2 = pTarget2->z - pOrigin->z;

    pCrossProduct->x = y1 * z2 - z1 * y2;
    pCrossProduct->y = z1 * x2 - x1 * z2;
    pCrossProduct->z = x1 * y2 - y1 * x2;
    }



/* VBSUB(Point3dCrossProductXY) */
/* CSVFUNC(crossProductXY) */

/*-----------------------------------------------------------------*//**
* @description Return the (scalar) cross product of the xy parts of two vectors.
* @param pVector1 => The first vector
* @param pVector2 => The second vector
* @return The 2d cross product.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_crossProductXY
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    return  pVector1->x * pVector2->y - pVector1->y * pVector2->x;
    }



/*-----------------------------------------------------------------*//**
* @description Compute the normalized cross product of two vectors
* and return the length of the unnormalized cross product.
*
* @instance pCrossProduct <= normalized (unit) vector in the direction of the
*           cross product of vector1 and vector2
* @param pVector1 => The first vector
* @param pVector2 => The second vector
* @return The length of the original (prenormalization) cross product vector
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_normalizedCrossProduct
(
DPoint3dP pCrossProduct,
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    bsiDPoint3d_crossProduct( pCrossProduct, pVector1, pVector2 );
    return bsiDPoint3d_normalizeInPlace( pCrossProduct );
    }



/*-----------------------------------------------------------------*//**
* @description Computes the cross product of the two parameter vectors and scales it to a given
* length.  The scaled vector is stored as the product vector, and the length of the original
* cross product vector is returned.
*
* @instance pProduct <= vector of given length in direction of the cross product of 2 vectors.
* @param pVector1 => The first vector
* @param pVector2 => The second vector
* @param productLength => The Desired length
* @return The length of original vector.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_sizedCrossProduct
(
DPoint3dP   pProduct,
DPoint3dCP  pVector1,
DPoint3dCP  pVector2,
double      productLength
)
    {
    double length;
    bsiDPoint3d_crossProduct( pProduct, pVector1, pVector2 );
    length = bsiDPoint3d_magnitude (pProduct);
    if (length != 0)
        {
        bsiDPoint3d_scale (pProduct, pProduct, productLength / length);
        }
    return length;
    }



/*-----------------------------------------------------------------*//**
* @description Computes the cross product of two vectors and scales it to the
* geometric mean of the lengths of the two vectors.  This is useful
* because it has the direction of the cross product (i.e. normal to the plane
* of the two vectors) and a size in between the two vectors.
*
* @instance pProduct <= cross product vector
* @param pVector1 => The first vector
* @param pVector2 => The second vector
* @return The length of original vector.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_geometricMeanCrossProduct
(
DPoint3dP pProduct,
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    double length;
    double aa, bb, c;
    bsiDPoint3d_crossProduct( pProduct, pVector1, pVector2 );
    length = bsiDPoint3d_magnitude (pProduct);
    aa = bsiDPoint3d_magnitudeSquared (pVector1);
    bb = bsiDPoint3d_magnitudeSquared (pVector2);
    if (length != 0)
        {
        c = sqrt (sqrt (aa * bb));
        bsiDPoint3d_scale (pProduct, pProduct, c / length);
        }
    return length;
    }


/* VBSUB(Point3dDotProduct) */
/* CSVFUNC(dotProduct) */

/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The dot product of the two vectors
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_dotProduct
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    return (pVector1->x*pVector2->x + pVector1->y*pVector2->y + pVector1->z*pVector2->z);
    }


/* VBSUB(Point3dDotProductXY) */
/* CSVFUNC(dotProductXY) */

/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of xy parts of two vectors.
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The dot product of the xy parts of the two vectors
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_dotProductXY
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    return (pVector1->x*pVector2->x + pVector1->y*pVector2->y);
    }


/* CSVFUNC(dotProduct) */

/*-----------------------------------------------------------------*//**
* @description Computes the dot product of one vector given as a point structure and another given as
* xyz components.
* @instance pVector => The first vector.
* @param ax => The x component of second vector.
* @param ay => The y component of second vector.
* @param az => The z component of second vector.
* @return The dot product of the vector with a vector with the given components
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_dotXYZ
(
DPoint3dCP  pVector,
double      ax,
double      ay,
double      az
)
    {
    return pVector->x * ax + pVector->y * ay + pVector->z * az;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|SECTION        vectorTriads    Generating perpendicular triads         |
| These functions generate mutually-perpendicular axis triads with      |
| specified alignments.                                                 |
+----------------------------------------------------------------------*/

/*-----------------------------------------------------------------*//**
* @description Sets three vectors so that they are mutually
* perpendicular, the third (Z) vector is identical to the
* given axis vector, and all have the same length.
* If the given axis vector contains only zeros, a (0,0,1) vector
*   is used instead.
*
* @instance pGivenAxis => input z direction vector
* @param pXAxis <= x direction of the coordinate system
* @param pYAxis <= y direction of the coordinate system
* @param pZAxis <= z direction of the coordinate system
* @return true unless given vector is z zero vector.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_getTriad
(
DPoint3dCP pGivenAxis,
DPoint3dP pXAxis,
DPoint3dP pYAxis,
DPoint3dP pZAxis
)
    {
    DPoint3d      vector;
    double      length = bsiDPoint3d_magnitude (pGivenAxis);
    double      zTest = length / 64.0;
    bool      boolStat = true;
    *pZAxis = *pGivenAxis;

    if ( length == 0.0 )
        {
        pZAxis->z = 1.0;
        length = 1.0;
        boolStat = false;
        }

    /* Pick a principle axis not too parallel to pZAxis.
       This is autocad's rule from DXF interchange book.
    */
    vector.x = vector.y = vector.z = 0.0;
    if (fabs (pZAxis->x) < zTest && fabs (pZAxis->y) < zTest)
        {
        vector.y = 1.0;
        }
    else
        {
        vector.z = 1.0;
        }

    bsiDPoint3d_crossProduct (pXAxis, &vector, pZAxis);
    bsiDPoint3d_crossProduct (pYAxis, pZAxis, pXAxis);
    bsiDPoint3d_normalizeInPlace (pXAxis);
    bsiDPoint3d_normalizeInPlace (pYAxis);
    bsiDPoint3d_normalizeInPlace (pZAxis);
    bsiDPoint3d_scale (pXAxis, pXAxis, length);
    bsiDPoint3d_scale (pYAxis, pYAxis, length);
    bsiDPoint3d_scale (pZAxis, pZAxis, length);

    return  boolStat;
    }


/*-----------------------------------------------------------------*//**
* @description Sets three vectors so that they are mutually
* perpendicular unit vectors with the  third (Z) vector in the
* direction of the given axis vector.
* If the given axis vector contains only zeros, a (0,0,1) vector
*   is used instead.
*
* @instance pGivenAxis   => input z direction vector
* @param    pXAxis      <= unit x direction vector
* @param    pYAxis      <= unit y direction vector
* @param    pZAxis      <= unit z direction vector
* @return true unless given vector has zero length.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_getNormalizedTriad
(
DPoint3dCP pGivenAxis,
DPoint3dP pXAxis,
DPoint3dP pYAxis,
DPoint3dP pZAxis
)
    {
    DPoint3d      vector;
    double      length = bsiDPoint3d_magnitude( pGivenAxis );
    double      zTest = length / 64.0;
    bool      boolStat = true;
    *pZAxis = *pGivenAxis;

    if (length == 0.0)
        {
        pZAxis->z = 1.0;
        length   = 1.0;
        boolStat   = false;
        }

    /* Pick a principle axis not too parallel to pZAxis.
       This is autocad's rule from DXF interchange book.
    */
    vector.x = vector.y = vector.z = 0.0;
    if (fabs (pZAxis->x) < zTest && fabs (pZAxis->y) < zTest)
        {
        vector.y = 1.0;
        }
    else
        {
        vector.z = 1.0;
        }

    bsiDPoint3d_crossProduct (pXAxis, &vector, pZAxis);
    bsiDPoint3d_crossProduct (pYAxis, pZAxis, pXAxis);
    bsiDPoint3d_normalizeInPlace (pXAxis);
    bsiDPoint3d_normalizeInPlace (pYAxis);
    bsiDPoint3d_normalizeInPlace (pZAxis);

    return  boolStat;
    }


/*---------------------------------------------------------------------------------**//**
* @description Apply a Givens to the two points, as if they are columns of a matrix to be postmultiplied
* by the Givens matrx.
* The Givens matrix is
*       R(0,0)=R(1,1)=c
*       R(0,1)=-s
*       R(1,0)=s
*
* @param    pOut0   <= rotation of pIn0
* @param    pOut1   <= rotation of pIn1
* @param    pIn0    => first vector
* @param    pIn1    => second vector
* @param    c   => cosine of givens rotation.
* @param    s   => sine of givens rotation.
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_applyGivensRotation
(
DPoint3dP   pOut0,
DPoint3dP   pOut1,
DPoint3dCP  pIn0,
DPoint3dCP  pIn1,
double      c,
double      s
)
    {
    double a, b;
    int i;
    double *p0 = (double*)pIn0;
    double *p1 = (double*)pIn1;

    for (i = 0; i < 3; i++)
        {
        a = p0[i];
        b = p1[i];
        p0[i] = a * c + b * s;
        p1[i] = b * c - a * s;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Apply a hyperbolic rotation to the two points, as if they are columns of a matrix to be postmultiplied
* by the hyperbolic matrix
* The hyperbolic matrix is
*       R(0,0)=R(1,1)=secant
*       R(0,1)=R(1,0)= tangent
*
* @param    pOut0   <= rotation of pIn0
* @param    pOut1   <= rotation of pIn1
* @param    pIn0    => first vector
* @param    pIn1    => second vector
* @param    secant  => secant (diagonal) part of the hyperbolic transformation
* @param    tangent => tangent (offdiagonal) part of the hyperbolic transformation
* @bsihdr                                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_applyHyperbolicReflection
(
DPoint3dP   pOut0,
DPoint3dP   pOut1,
DPoint3dCP  pIn0,
DPoint3dCP  pIn1,
double      secant,
double      tangent
)
    {
    double a, b;
    int i;
    double *p0 = (double*)pIn0;
    double *p1 = (double*)pIn1;

    for (i = 0; i < 3; i++)
        {
        a = p0[i];
        b = p1[i];
        p0[i] = a * secant + b * tangent;
        p1[i] = b * secant + a * tangent;
        }
    }



/*-----------------------------------------------------------------*//**
* @description Computes a unit vector  in the direction of the difference of the points
* or vectors (Second parameter vector is subtracted from the first parameter vector,
* exactly as in the subtract function.)
*
* @instance pVector <= The normalized vector in direction (pTarget - pOrigin)
* @param pTarget => The target point.
* @param pOrigin => The origin point.
* @return The length of original difference vector.
* @bsimethod                                            EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_computeNormal
(
DPoint3dP pVector,
DPoint3dCP pTarget,
DPoint3dCP pOrigin
)
    {
    bsiDPoint3d_subtractDPoint3dDPoint3d (pVector, pTarget, pOrigin);
    return bsiDPoint3d_normalizeInPlace (pVector);
    }

/* VBSUB(Point3dAngleBetweenVectors) */
/* CSVFUNC(angleTo) */

/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors.  This angle is between 0 and
* pi.  Rotating the first vector by this angle around the cross product
* between the vectors aligns it with the second vector.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The angle between the vectors.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_angleBetweenVectors
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    DPoint3d   crossProd;
    double cross, dot;
    bsiDPoint3d_crossProduct (&crossProd, pVector1, pVector2);
    cross   = bsiDPoint3d_magnitude (&crossProd);
    dot     = bsiDPoint3d_dotProduct (pVector1, pVector2);
    return  bsiTrig_atan2 (cross, dot);
    }

/* VBSUB(Point3dSmallerAngleBetweenUnorientedVectors) */
/* CSVFUNC(smallerUnorientedAngleTo) */

/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors, choosing the smaller
*   of the two possible angles when both the vectors and their negations are considered.
*    This angle is between 0 and pi/2.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The angle between the vectors.
* @bsihdr                                 EarlinLutz    12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_smallerAngleBetweenUnorientedVectors
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    DPoint3d   crossProd;
    double cross, dot;
    bsiDPoint3d_crossProduct (&crossProd, pVector1, pVector2);
    cross   = bsiDPoint3d_magnitude (&crossProd);
    dot     = bsiDPoint3d_dotProduct (pVector1, pVector2);
    return  bsiTrig_atan2 (cross, fabs (dot));
    }

/* VBSUB(Point3dIsVectorInSmallerSector) */
/* CSVFUNC(isVectorInSmallerSector) */

/*-----------------------------------------------------------------*//**
* @description Test a vector is "between" vector0 and vector1.
* If the vectors are coplanar and vector0 is neither parallel nor antiparallel
* to vector1, betweenness has the expected meaning: there are two angles between
* vector0 and vector1; one is less than 180; the test vector is tested to
* see if it is in the smaller angle.
* If the vectors are not coplanar, the test is based on the projection of
* the test vector into the plane of the other two vectors.
*
* Zero testing is untoleranced, and is biased to all parallel conditions "false".
* That is, if any pair of the input vectors is parallel or antiparallel,
* the mathematical answer is false.  Floating point tolerances
* will cause "nearby" cases to be unpredictable.  It is assumed that if
* the caller considers the "parallel" cases important they will be
* checked explicitly.
*
* @instance pTestVector => The vector to test
* @param pVector0 => The first boundary vector.
* @param pVector1 => The second boundary vector.
* @return true if the test vector is within the angle.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_isVectorInSmallerSector
(
DPoint3dCP pTestVector,
DPoint3dCP pVector0,
DPoint3dCP pVector1
)
    {
    DPoint3d   cross01;
    bsiDPoint3d_crossProduct (&cross01, pVector0, pVector1);
    return      bsiDPoint3d_tripleProduct (pVector0, pTestVector, &cross01) > 0.0
            &&  bsiDPoint3d_tripleProduct (pTestVector, pVector1, &cross01) > 0.0;
    }


/* VBSUB(Point3dIsVectorInCCWSector) */
/* CSVFUNC(isVectorInCCWSector) */

/*-----------------------------------------------------------------*//**
* @description Test if the test vector vector is "between" vector0 and vector1, with CCW direction resolved by an up vector.
* @remarks The cross product of vector0 and vector1 is considered the positive plane normal if its dot product with the up vector is positive.
* @remarks The containment test is strict in the sense that if the test vector equals a boundary vector, this function returns false.
* @instance pTestVector => The vector to test
* @param pVector0 => The boundary vector
* @param pVector1 => The boundary vector
* @param pUpVector => The out of plane vector.  If null, the z-axis vector (001) is used.
* @return true if test vector is within the angle.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_isVectorInCCWSector
(
DPoint3dCP pTestVector,
DPoint3dCP pVector0,
DPoint3dCP pVector1,
DPoint3dCP pUpVector
)
    {
    DPoint3d    cross01;
    double      dot;

    bsiDPoint3d_crossProduct (&cross01, pVector0, pVector1);

    if (bsiDPoint3d_pointEqual (NULL, &cross01))
        {
        dot = bsiDPoint3d_dotProduct (pVector0, pVector1);
        if (dot > 0.0)
            return false;
        }

    if (pUpVector)
        dot = bsiDPoint3d_dotProduct (&cross01, pUpVector);
    else
        dot = cross01.z;

    if (dot > 0.0)
        return  bsiDPoint3d_tripleProduct (pVector0, pTestVector, &cross01) > 0.0
            &&  bsiDPoint3d_tripleProduct (pTestVector, pVector1, &cross01) > 0.0;
    else
        return  bsiDPoint3d_tripleProduct (pVector0, pTestVector, &cross01) < 0.0
            ||  bsiDPoint3d_tripleProduct (pTestVector, pVector1, &cross01) < 0.0;
    }

/*-----------------------------------------------------------------*//**
* @description Test if the test vector vector is "between" vector0 and vector1, in CCW sense using only xy components of the 3 vectors.
* @remarks The containment test is strict in the sense that if the xy-components of the test vector equal those of a boundary vector, this function returns false.
* @instance pTestVector => The vector to test
* @param pVector0 => The boundary vector
* @param pVector1 => The boundary vector
* @return true if test vector is within the angle.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_isVectorInCCWXYSector
(
DPoint3dCP pTestVector,
DPoint3dCP pVector0,
DPoint3dCP pVector1
)
    {
    double cross = bsiDPoint3d_crossProductXY (pVector0, pVector1);

    if (cross == 0.0)
        {
        double dot   = bsiDPoint3d_dotProductXY (pVector0, pVector1);
        if (dot > 0.0)
            return false;
        }

    if (cross > 0.0)
        return  bsiDPoint3d_crossProductXY (pVector0, pTestVector) > 0.0
            &&  bsiDPoint3d_crossProductXY (pTestVector, pVector1) > 0.0;
    else
        return  bsiDPoint3d_crossProductXY (pVector0, pTestVector) > 0.0
            ||  bsiDPoint3d_crossProductXY (pTestVector, pVector1) > 0.0;
    }

/* VBSUB(Point3dAngleBetweenVectorsXY) */
/* CSVFUNC(angleToXY) */

/*-----------------------------------------------------------------*//**
* @description Returns the angle from Vector1 to Vector2 using only xy parts.
*  This angle is between -pi and +pi.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The angle between vectors.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_angleBetweenVectorsXY
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    double cross, dot;
    cross = bsiDPoint3d_crossProductXY (pVector1, pVector2);
    dot     = bsiDPoint3d_dotProductXY (pVector1, pVector2);
    return  bsiTrig_atan2 (cross, dot);
    }

/* VBSUB(Point3dSmallerAngleBetweenUnorientedVectorsXY) */
/* CSVFUNC(smallerUnorientedAngleToXY) */

/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors, considering both
*   the vectors and their negations and choosing the smaller.
*   This angle is between 0 and pi/2.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The angle between vectors.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_smallerAngleBetweenUnorientedVectorsXY
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    double cross, dot;
    cross = bsiDPoint3d_crossProductXY (pVector1, pVector2);
    dot     = bsiDPoint3d_dotProductXY (pVector1, pVector2);
    return  bsiTrig_atan2 (fabs (cross), fabs(dot));
    }


/* VBSUB(Point3dRotateXY) */
/* CSVFUNC(rotateXY) */

/*-----------------------------------------------------------------*//**
* @description Rotate a vector around the z axis.
* @instance pRotatedVector <= rotated vector
* @param theta   => The rotation angle.
* @bsihdr                               DavidAssaf      6/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_rotateXY
(
DPoint3dP   pRotatedVector,
DPoint3dCP  pVector,
double      theta
)
    {
    double c, s, xx, yy;
    s = sin (theta);
    c = cos (theta);

    xx = pVector->x;
    yy = pVector->y;

    pRotatedVector->x = xx * c - yy * s;
    pRotatedVector->y = xx * s + yy * c;
    pRotatedVector->z = pVector->z;
    }



/*-----------------------------------------------------------------*//**
* @description Rotate a vector around the z axis.
* @instance pVector <=> rotated vector
* @param theta   => The rotation angle.
* @bsihdr                                                                       DavidAssaf      6/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_rotateXYInPlace
(
DPoint3dP   pVector,
double      theta
)
    {
    double c, s, xx, yy;
    s = sin (theta);
    c = cos (theta);

    xx = pVector->x;
    yy = pVector->y;

    pVector->x = xx * c - yy * s;
    pVector->y = xx * s + yy * c;
    }

/* VBSUB(Point3dSignedAngleBetweenVectors) */
/* CSVFUNC(signedAngleTo) */

/*-----------------------------------------------------------------*//**
* @description Computes the signed from one vector to another, in the plane
*       of the two vectors.   Initial computation using only the two vectors
*       yields two possible angles depending on which side of the plane of the
*       vectors is viewed.  To choose which side to view, go on the side whose
*       normal has a positive dot product with the orientation vector.
* This angle can be between -pi and +pi.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @param pOrientationVector => The vector used to determine orientation.
* @return The signed angle
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_signedAngleBetweenVectors
(
DPoint3dCP pVector1,
DPoint3dCP pVector2,
DPoint3dCP pOrientationVector
)
    {
    DPoint3d   crossProd;
    double cross, dot, theta;
    bsiDPoint3d_crossProduct (&crossProd, pVector1, pVector2);
    cross   = bsiDPoint3d_magnitude (&crossProd);
    dot     = bsiDPoint3d_dotProduct (pVector1, pVector2);
    theta   = bsiTrig_atan2 (cross, dot);

    if (bsiDPoint3d_dotProduct (&crossProd, pOrientationVector) < 0.0)
        return  -theta;
    else
        return  theta;
    }

/* VBSUB(Point3dPlanarAngleBetweenVectors) */
/* CSVFUNC(planarAngleTo) */

/*-----------------------------------------------------------------*//**
* @description Computes the signed angle between the projection of two vectors
*       onto a plane with given normal.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @param pNormal => The plane normal vector
* @return The angle in plane
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_planarAngleBetweenVectors
(
DPoint3dCP pVector1,
DPoint3dCP pVector2,
DPoint3dCP pPlaneNormal
)
    {
    DPoint3d      projection1, projection2;
    double      square = bsiDPoint3d_dotProduct (pPlaneNormal, pPlaneNormal);
    double      projectionFactor1, projectionFactor2;
    double      factor;

    if (square == 0.0)
        return 0.0;

    factor = 1.0 / square;

    projectionFactor1 = - bsiDPoint3d_dotProduct (pVector1, pPlaneNormal) * factor;
    projectionFactor2 = - bsiDPoint3d_dotProduct (pVector2,  pPlaneNormal) * factor;

    bsiDPoint3d_addScaledDPoint3d (&projection1, pVector1, pPlaneNormal, projectionFactor1);
    bsiDPoint3d_addScaledDPoint3d (&projection2, pVector2, pPlaneNormal, projectionFactor2);

    return  bsiDPoint3d_signedAngleBetweenVectors (&projection1, &projection2, pPlaneNormal);
    }

/*-----------------------------------------------------------------*//**
* @description Scale each point by the other's weight and subtract, i.e. form
* (point1 * point2.w - point2 * point1.w).  The weight term
* vanishes.   Copy the xyz parts back as a vector.
*
* @instance pDifference <= The difference vector
* @param pPoint1 => The first point
* @param pTarget2 => The second pont.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_weightedDifference
(
DPoint3dP   pDifference,
DPoint4dCP  pPoint1,
DPoint4dCP  pPoint2
)
    {
    double w2 = pPoint2->w;
    double w1 = pPoint1->w;
    pDifference->x = pPoint1->x * w2 - pPoint2->x * w1;
    pDifference->y = pPoint1->y * w2 - pPoint2->y * w1;
    pDifference->z = pPoint1->z * w2 - pPoint2->z * w1;
    }


/*-----------------------------------------------------------------*//**
* @description Form the cross product of the weighted differences from point0 to point1 and point2.
*
* @instance pProduct <= cross product result
* @param pBasePoint => The common base point (second point for differences)
* @param pTarget1 => The first target point.
* @param pTarget2 => The second target point.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_weightedDifferenceCrossProduct
(
DPoint3dP   pProduct,
DPoint4dCP  pBasePoint,
DPoint4dCP  pTarget1,
DPoint4dCP  pTarget2
)
    {
    DPoint3d   U, V;
    bsiDPoint3d_weightedDifference (&U, pTarget1, pBasePoint);
    bsiDPoint3d_weightedDifference (&V, pTarget2, pBasePoint);
    bsiDPoint3d_crossProduct (pProduct, &U, &V);
    }

/* VBSUB(Point3dMagnitudeSquared) */


/*-----------------------------------------------------------------*//**
* @description Computes the squared magnitude of a vector.
*
* @instance pVector => The vector whose magnitude is computed.
* @return The squared magnitude of the vector.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_magnitudeSquared
(
DPoint3dCP pVector
)
    {
    return   pVector->x * pVector->x
           + pVector->y * pVector->y
           + pVector->z * pVector->z;
    }

/* VBSUB(Point3dMagnitudeXY) */
/* CSVFUNC(magnitudeXY) */

/*-----------------------------------------------------------------*//**
* @description Computes the magnitude of the xy part of a vector.
* @instance pVector => The vector
* @return The magnitude of the xy parts of the given vector.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_magnitudeXY
(
DPoint3dCP pVector
)
    {
    return sqrt (pVector->x * pVector->x + pVector->y * pVector->y);
    }

/* VBSUB(Point3dMagnitudeSquaredXY) */
/* CSVFUNC(magnitudeSquaredXY) */

/*-----------------------------------------------------------------*//**
* @description Computes the squared magnitude of the xy part of a vector.
* @instance pVector => The vector
* @return The squared magnitude of the xy parts of the given vector.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_magnitudeSquaredXY
(
DPoint3dCP pVector
)
    {
    return pVector->x * pVector->x + pVector->y * pVector->y;
    }

/* csimethod(unitPerpendicularXY) */

/*---------------------------------------------------------------------------------**//**
* @description Compute a unit vector perpendicular to the xy parts of given vector.
*
* @remarks Input may equal output.
*
* @param pRotatedVector <=      The rotated and scaled vector.  Z-coordinate is zero.
* @param pVector        =>      The source vector
* @return true if the input vector has nonzero length
* @bsimethod                                                    EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint3d_unitPerpendicularXY
(
DPoint3dP    pRotatedVector,
DPoint3dCP    pVector
)
    {
    double  a, d2, x = pVector->x, y = pVector->y;
    pRotatedVector->x = -y;
    pRotatedVector->y =  x;
    pRotatedVector->z = 0.0;
    d2 = x * x + y * y;
    if (d2 == 0.0)
        return false;
    a = 1.0 / sqrt (d2);
    pRotatedVector->x *= a;
    pRotatedVector->y *= a;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute the unit vector perpendicular to the xy parts of the given vector with the given handedness.
*
* @remarks Input may equal output.
*
* @param pRotatedVector <=  The rotated and scaled vector.  Z-coordinate is zero.
* @param pVector        =>  The source vector
* @param bRightHanded   =>  Whether the returned vector points to the right of the xy parts of the given vector.
* @return true if the input vector has nonzero length
* @bsimethod                                                    DavidAssaf      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint3d_unitPerpendicularXYWithHandedness
(
DPoint3dP   pRotatedVector,
DPoint3dCP  pVector,
bool        bRightHanded
)
    {
    double  a, d2, x = pVector->x, y = pVector->y;
    pRotatedVector->x = -y;
    pRotatedVector->y =  x;
    pRotatedVector->z = 0.0;
    d2 = x * x + y * y;
    if (d2 == 0.0)
        return false;
    a = 1.0 / sqrt (d2);
    if (bRightHanded)
        a = -a;
    pRotatedVector->x *= a;
    pRotatedVector->y *= a;
    return true;
    }

/* VBSUB(Point3dMagnitude) */




/*-----------------------------------------------------------------*//**
* Determines within angular tolerance if normals are parallel and pointing
* in same direction.  Input -1 for dot0, dot1 to compute them in this function.
*
* @param pNormal0   => first normal
* @param dot0       => squared magnitude of first normal (or -1)
* @param pNormal1   => second normal
* @param dot1       => squared magnitude of second normal (or -1)
* @param eps2       => max sin^2(angle) for parallel normals
* @return true if normals are same within tolerance
* @bsihdr                                       DavidAssaf      10/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint3d_normalEqualTolerance
(
DPoint3dCP  pNormal0,
double      dot0,
DPoint3dCP  pNormal1,
double      dot1,
double      eps2
)
    {
    DPoint3d   cross;
    double mag2, dot;

    if (dot0 <= 0.0)
        dot0 = bsiDPoint3d_magnitudeSquared (pNormal0);
    if (dot1 <= 0.0)
        dot1 = bsiDPoint3d_magnitudeSquared (pNormal1);

    if (0.0 == dot0 && 0.0 == dot1)
        return true;

    bsiDPoint3d_crossProduct (&cross, pNormal0, pNormal1);
    mag2 = bsiDPoint3d_magnitudeSquared (&cross);
    dot = bsiDPoint3d_dotProduct (pNormal0, pNormal1);

    return ((mag2 <= eps2 * dot0 * dot1) && (dot > 0.0)) ? true : false;
    }


/* VBSUB(Point3dScale) */
/* CSVFUNC(scale) */

/*-----------------------------------------------------------------*//**
* @description Multiplies a vector by a scale factor.
* @instance pScaledVector <= The scaled vector.
* @param pVector => The vector to be scaled.
* @param scale => The scale factor.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_scale
(
DPoint3dP   pScaledVector,
DPoint3dCP  pVector,
double      scale
)
    {
    pScaledVector->x = pVector->x*scale;
    pScaledVector->y = pVector->y*scale;
    pScaledVector->z = pVector->z*scale;
    }

/* VBSUB() */

/*-----------------------------------------------------------------*//**
* @description Multiplies a vector (in place) by a scale factor.
* @instance pVector <=> scaled vector
* @param scale => The scale
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_scaleInPlace
(
DPoint3dP   pVector,
double      scale
)
    {
    pVector->x *= scale;
    pVector->y *= scale;
    pVector->z *= scale;
    }

/* VBSUB(Point3dNegate) */
/* CSVFUNC(negate) */

/*-----------------------------------------------------------------*//**
* @description Computes a negated (opposite) vector.
*
* @instance pNegated <= The negated vector.
* @param pVector => The vector to be negated.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_negate
(
DPoint3dP pNegatedVector,
DPoint3dCP pVector
)
    {
    pNegatedVector->x = - pVector->x;
    pNegatedVector->y = - pVector->y;
    pNegatedVector->z = - pVector->z;
    }




/*-----------------------------------------------------------------*//**
* @description Negate a vector in place.
*
* @instance pVector <=> negated vector
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_negateInPlace
(
DPoint3dP pVector
)
    {
    pVector->x = - pVector->x;
    pVector->y = - pVector->y;
    pVector->z = - pVector->z;
    }

/* VBSUB() */

/*-----------------------------------------------------------------*//**
* @description Normalizes (scales) a vector to length 1.
* If the input vector length is 0, the output vector is a zero vector
* and the returned length is 0.
*
* @instance pUnitVector <= The normalized vector.
* @param pVector => The vector to be normalized.
* @return The length prior to normalization
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_normalize
(
DPoint3dP pUnitVector,
DPoint3dCP pVector
)
    {
    double  magnitude =
            sqrt ( pVector->x * pVector->x
                 + pVector->y * pVector->y
                 + pVector->z * pVector->z);


    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        pUnitVector->x = pVector->x * f;
        pUnitVector->y = pVector->y * f;
        pUnitVector->z = pVector->z * f;
        }
    else
        {
        pUnitVector->x = pVector->x;
        pUnitVector->y = pVector->y;
        pUnitVector->z = pVector->z;
        }

    return  magnitude;
    }

/* csimethod(scaleToLength) */

/*-----------------------------------------------------------------*//**
* @description Scales a vector to specified length.
* If the input vector length is 0, the output vector is a zero vector
* and the returned length is 0.
*
* @instance pScaledVector <= The scaled vector.
* @param    pVector => The original vector.
* @param    length => The requested length.
* @return The length prior to scaling.
* @bsihdr                                                                       EarlinLutz      01/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_scaleToLength
(
DPoint3dP   pScaledVector,
DPoint3dCP  pVector,
double      length
)
    {
    double  magnitude =
            sqrt ( pVector->x * pVector->x
                 + pVector->y * pVector->y
                 + pVector->z * pVector->z);

    if (magnitude > 0.0)
        {
        double f = length / magnitude;
        pScaledVector->x = pVector->x * f;
        pScaledVector->y = pVector->y * f;
        pScaledVector->z = pVector->z * f;
        }
    else
        {
        pScaledVector->x = length;
        pScaledVector->y = 0.0;
        pScaledVector->z = 0.0;
        }

    return  magnitude;
    }



/*-----------------------------------------------------------------*//**
* @description Scales a vector to a specified length, and returns
* the prior length.
* If the input vector length is 0, the output vector is a zero vector
* and the returned length is 0.
*
* @instance pScaledVector <= scaled
* @param    length => The requested length
* @return The length prior to scaling.
* @bsihdr                                                                       EarlinLutz      01/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_scaleToLengthInPlace
(
DPoint3dP   pScaledVector,
double      length
)
    {
    return bsiDPoint3d_scaleToLength (pScaledVector, pScaledVector, length);
    }



/*-----------------------------------------------------------------*//**
* @description Replaces a vector by a unit vector in the same direction, and returns
* the original length.
* If the input vector length is 0, the output vector is a zero vector
* and the returned length is 0.
*
* @instance pVector <=> vector to be normalized
* @return The length prior to normalization
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_normalizeInPlace
(
DPoint3dP pVector
)
    {
    double  magnitude =
            sqrt ( pVector->x * pVector->x
                 + pVector->y * pVector->y
                 + pVector->z * pVector->z);

    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        pVector->x *= f;
        pVector->y *= f;
        pVector->z *= f;
        }

    return  magnitude;
    }


/* VBSUB(Point3dNormalize) */
/* CSVFUNC(normalize) */
/*-----------------------------------------------------------------*//**
* @description Computes a unit vector in the direction of a given vector.
* If the input vector length is 0, the output vector is a zero vector.
*
* @instance pUnitVector <= The normalized vector.
* @instance pVector => The vector to normalize.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDPoint3d_unitVector
(
DPoint3dP pUnitVector,
DPoint3dCP pVector
)
    {
    bsiDPoint3d_normalize (pUnitVector, pVector);
    }


/* VBSUB(Point3dAreVectorsParallel) */
/* CSVFUNC(isParallelTo) */

/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are parallel.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return true if the vectors are parallel within tolerance
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_areParallel
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    DPoint3d      vecC;
    double      a2 = bsiDPoint3d_dotProduct (pVector1, pVector1);
    double      b2 = bsiDPoint3d_dotProduct (pVector2, pVector2);
    double      cross;
    double      eps = bsiTrig_smallAngle(); /* small angle tolerance (in radians) */
    bsiDPoint3d_crossProduct (&vecC, pVector1, pVector2);
    cross = bsiDPoint3d_magnitudeSquared (&vecC);

    /* a2,b2,c2 are squared lengths of respective vectors */
    /* c2 = sin^2(theta) * a2 * b2 */
    /* For small theta, sin^2(theta)~~theta^2 */
    /* Zero-length vector case falls through as equality */

    return  cross <= eps * eps * a2 * b2;
    }

/* VBSUB(Point3dAreVectorsPerpendicular) */
/* CSVFUNC(isPerpendicularTo) */

/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are perpendicular.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return true if vectors are perpendicular within tolerance
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_arePerpendicular
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    double      aa = bsiDPoint3d_dotProduct (pVector1, pVector1);
    double      bb = bsiDPoint3d_dotProduct (pVector2, pVector2);
    double      ab = bsiDPoint3d_dotProduct (pVector1, pVector2);
    double      eps = bsiTrig_smallAngle();

    return  ab * ab <= eps * eps * aa * bb;
    }

/*---------------------------------------------------------------------------------**//**
* @description Tests if two vectors are parallel.
* @remarks Use bsiTrig_smallAngle() for tolerance corresponding to bsiDPoint3d_areParallel.
*
* @param pVector1   IN      the first vector
* @param pVector2   IN      the second vector
* @param tolerance  IN      radian tolerance for angle between vectors
* @return true if the vectors are parallel within tolerance
* @bsimethod                                                    DavidAssaf      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint3d_areParallelTolerance
(
DPoint3dCP  pVector1,
DPoint3dCP  pVector2,
double      tolerance
)
    {
    DPoint3d    cross;
    double      a2 = bsiDPoint3d_dotProduct (pVector1, pVector1);
    double      b2 = bsiDPoint3d_dotProduct (pVector2, pVector2);
    double      c2;

    bsiDPoint3d_crossProduct (&cross, pVector1, pVector2);

    c2 = bsiDPoint3d_dotProduct (&cross, &cross);

    /* a2,b2,c2 are squared lengths of respective vectors */
    /* c2 = sin^2(theta) * a2 * b2 */
    /* For small theta, sin^2(theta)~~theta^2 */
    /* Zero-length vector case falls through as equality */

    return c2 <= tolerance * tolerance * a2 * b2;
    }

/*---------------------------------------------------------------------------------**//**
* @description Tests if two vectors are perpendicular.
* @remarks Use bsiTrig_smallAngle() for tolerance corresponding to bsiDPoint3d_arePerpendicular.
*
* @param pVector1   IN      the first vector
* @param pVector2   IN      the second vector
* @param tolerance  IN      radian tolerance for angle between vectors
* @return true if the vectors are perpendicular within tolerance
* @bsimethod                                                    DavidAssaf      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint3d_arePerpendicularTolerance
(
DPoint3dCP  pVector1,
DPoint3dCP  pVector2,
double      tolerance
)
    {
    double  aa = bsiDPoint3d_dotProduct (pVector1, pVector1);
    double  bb = bsiDPoint3d_dotProduct (pVector2, pVector2);
    double  ab = bsiDPoint3d_dotProduct (pVector1, pVector2);

    return  ab * ab <= tolerance * tolerance * aa * bb;
    }


/*-----------------------------------------------------------------*//**
* @description Try to divide each component of a vector by a scalar.  If the denominator
* near zero compared to any numerator component, return the original
* vector.
* @param pScaledVector <= The vector after scaling.
* @param pVector => The initial vector.
* @param denominator => The divisor.
* @return true if division is numerically safe.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_safeDivide
(
DPoint3dP   pScaledVector,
DPoint3dCP  pVector,
double      denominator
)
    {
    static double s_relTol = 1.0e-12;
    double absD = fabs (denominator);
    double tol = s_relTol * bsiDPoint3d_maxAbs (pVector);

    if (absD <= tol)
        {
        *pScaledVector = *pVector;
        return false;
        }
    else
        {
        double a = 1.0 / denominator;
        pScaledVector->x = pVector->x * a;
        pScaledVector->y = pVector->y * a;
        pScaledVector->z = pVector->z * a;
        return true;
        }
    }
/* VBSUB(Point3dPolarAngle) */

/*-----------------------------------------------------------------*//**
* @description Return the angle component of the polar coordinates
*           form of a vector.  Z part of the vector is ignored.
* @instance pVector => vector in xyz form.
* @return angle from x axis to this vector, using only xy parts.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_polarAngle
(
DPoint3dCP pVector
)
    {
    return atan2 (pVector->y, pVector->x);
    }
/* VBSUB(Point3dTripleProduct) */
/* CSVFUNC(tripleProduct) */

/*-----------------------------------------------------------------*//**
* @description Computes the triple product of three vectors.
* The following are equivalent definitions of the triple product of three
* vectors V1, V2, and V3:
*
*<UL>
*<LI> (V1 cross V2) dot V3
*<LI> V1 dot (V2 cross V3)
*<LI>The determinant of the 3x3 matrix with the three vectors as its
*               columns.
*<LI>The determinant of the 3x3 matrix with the three vectors as its
*               rows.
*<LI>The (signed)volume of the parallelepiped whose 4 vertices are at the
*               origin and at the ends of the 3 vectors placed at the
*               origin.
*</UL>
*
* @instance pVector1 => The first vector.
* @param pVector2 => The second vector.
* @param pVector3 => The third vector.
* @return The triple product
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_tripleProduct
(
DPoint3dCP pVector1,
DPoint3dCP pVector2,
DPoint3dCP pVector3
)
    {
    return
          pVector1->x * ( pVector2->y * pVector3->z - pVector2->z * pVector3->y )
        + pVector1->y * ( pVector2->z * pVector3->x - pVector2->x * pVector3->z )
        + pVector1->z * ( pVector2->x * pVector3->y - pVector2->y * pVector3->x )
        ;
    }



/*-----------------------------------------------------------------*//**
* @description Subtract two points or vectors, and return the result in
*           place of the first.
*
* @instance pVector2 <=> The point or vector to be modified.
* @param pVector2 => The vector to subtract.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_subtract
(
DPoint3dP pVector1,
DPoint3dCP pVector2
)
    {
    pVector1->x -= pVector2->x;
    pVector1->y -= pVector2->y;
    pVector1->z -= pVector2->z;
    }

/* VBSUB(Point3dSubtract) */
/* CSVFUNC(Subtract) */

/*-----------------------------------------------------------------*//**
* @description Subtract coordinates of two vectors or points. (Compute Point1 - Point2)
*
* @instance pVector <= The difference vector
* @param pPoint1 => The first point
* @param pPoint2 => The second (subtracted) point.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_subtractDPoint3dDPoint3d
(
DPoint3dP pVector,
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
)
    {
    pVector->x = pPoint1->x - pPoint2->x;
    pVector->y = pPoint1->y - pPoint2->y;
    pVector->z = pPoint1->z - pPoint2->z;
    }



/* VBSUB(Point3dAddScaled) */
/* CSVFUNC(Add) */

/*-----------------------------------------------------------------*//**
* @description Adds an origin and a scaled vector.
*
* @instance pSum <= The sum
* @param pOrigin => Origin for the sum.
* @param pVector => The vector to be added.
* @param scale => The scale factor.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_addScaledDPoint3d
(
DPoint3dP   pSum,
DPoint3dCP  pOrigin,
DPoint3dCP  pVector,
double      scale
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
/* VBSUB(Point3dAdd2Scaled) */
/* CSVFUNC(Add) */

/*-----------------------------------------------------------------*//**
* @description Adds an origin and two scaled vectors.
*
* @instance pSum <= sum
* @param pOrigin => The origin.  May be null.
* @param pVector1 => The first direction vector
* @param scale1 => The first scale factor
* @param pVector2 => The second direction vector
* @param scale2 => The second scale factor
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_add2ScaledDPoint3d
(
DPoint3dP   pSum,
DPoint3dCP  pOrigin,
DPoint3dCP  pVector1,
double      scale1,
DPoint3dCP  pVector2,
double      scale2
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

/* VBSUB(Point3dAdd3Scaled) */
/* CSVFUNC(Add) */

/*-----------------------------------------------------------------*//**
* @description Adds an origin and three scaled vectors.
*
* @instance pSum <= The sum.
* @param pOrigin => The origin. May be null
* @param pVector1 => The first direction vector
* @param scale1 => The first scale factor
* @param pVector2 => The second direction vector
* @param scale2 => The second scale factor
* @param pVector3 => The third direction vector
* @param scale3 => The third scale factor
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_add3ScaledDPoint3d
(
DPoint3dP   pSum,
DPoint3dCP  pOrigin,
DPoint3dCP  pVector1,
double      scale1,
DPoint3dCP  pVector2,
double      scale2,
DPoint3dCP  pVector3,
double      scale3
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



/*-----------------------------------------------------------------*//**
* @description Adds a vector to a pointer or vector, returns the result in place.
*
* @instance pSum <=> The point or vector to be modified.
* @param pVector => The vector to add.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_addDPoint3dInPlace
(
DPoint3dP pSum,
DPoint3dCP pVector
)
    {
    pSum->x += pVector->x;
    pSum->y += pVector->y;
    pSum->z += pVector->z;
    }

/* VBSUB(Point3dAdd) */
/* CSVFUNC(Add) */

/*-----------------------------------------------------------------*//**
* @description Compute the sum of two points or vectors.
*
* @instance pSum <= The computed sum.
* @param pPoint1 => The the first point or vector
* @param pPoint2 => The second point or vector
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_addDPoint3dDPoint3d
(
DPoint3dP pSum,
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
)
    {
    pSum->x = pPoint1->x + pPoint2->x;
    pSum->y = pPoint1->y + pPoint2->y;
    pSum->z = pPoint1->z + pPoint2->z;
    }

END_BENTLEY_NAMESPACE
