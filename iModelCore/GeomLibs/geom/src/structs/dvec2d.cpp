/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dvec2d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------*//**
* @description @struct DVec2d
* A DVec2d structure holds cartesian components of a 2D vector
* @fields
* @field double x x component of vector
* @field double y y component of vector
* @endfields
* @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE


/* VBSUB(Vec3dCrossProduct) */

/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
* @instance pCrossProduct <= cross product of vector1 and vector2
* @param pVector1 => The first vector
* @param pVector2 => The second vector
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_crossProduct
(
DVec2dCP pVector1,
DVec2dCP pVector2
)
    {
    return pVector1->x * pVector2->y - pVector1->y * pVector2->x;
    }

/* VBSUB(Vec3dCrossProductSquared) */

/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
* @instance pCrossProduct <= cross product of vector1 and vector2
* @param pVector1 => The first vector
* @param pVector2 => The second vector
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_crossProductSquared
(
DVec2dCP pVector1,
DVec2dCP pVector2
)
    {
    double cross = pVector1->x * pVector2->y - pVector1->y * pVector2->x;
    return cross * cross;
    }


/* VBSUB(Vec3dDotProduct) */

/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @param pVector1 => The first vector
* @param pVector2 => The second vector, given as point.
        The point's xyz are understood to be a vector from the origin.
* @return dot product of the two vectors.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_dotProductDVec2dDPoint2d
(
DVec2dCP pVector1,
DPoint2dCP pPoint2
)
    {
    return bsiDVec2d_dotProduct (pVector1, (DVec2d*)pPoint2);
    }

/* VBSUB(Vec3dDotProduct) */

/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The dot product of the two vectors
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_dotProduct
(
DVec2dCP pVector1,
DVec2dCP pVector2
)
    {
    return (pVector1->x*pVector2->x + pVector1->y*pVector2->y);
    }

/* VBSUB(Vec3dDotProductXYZ) */

/*-----------------------------------------------------------------*//**
* @description Computes the dot product of one vector given as a vector structure and another given as
* xyz components.
* @instance pVector => The first vector.
* @param ax => The x component of second vector.
* @param ay => The y component of second vector.
* @param az => The z component of second vector.
* @return The dot product of the vector with a vector with the given components
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_dotXY
(
DVec2dCP pVector,
double    ax,
double    ay
)
    {
    return pVector->x * ax + pVector->y * ay;
    }



/*---------------------------------------------------------------------------------**//**
* @description Apply a Givens to the two vectors, as if they are columns of a matrix to be postmultiplied
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_applyGivensRotation
(
DVec2dP pOut0,
DVec2dP pOut1,
DVec2dCP pIn0,
DVec2dCP pIn1,
double          c,
double          s
)
    {
    double a, b;
    int i;
    double *p0 = (double*)pIn0;
    double *p1 = (double*)pIn1;

    for (i = 0; i < 2; i++)
        {
        a = p0[i];
        b = p1[i];
        p0[i] = a * c + b * s;
        p1[i] = b * c - a * s;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Apply a hyperbolic rotation to the two vectors, as if they are columns of a matrix to be postmultiplied
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_applyHyperbolicReflection
(
DVec2dP pOut0,
DVec2dP pOut1,
DVec2dCP pIn0,
DVec2dCP pIn1,
double          secant,
double          tangent
)
    {
    double a, b;
    int i;
    double *p0 = (double*)pIn0;
    double *p1 = (double*)pIn1;

    for (i = 0; i < 2; i++)
        {
        a = p0[i];
        b = p1[i];
        p0[i] = a * secant + b * tangent;
        p1[i] = b * secant + a * tangent;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Computes a unit vector  in the direction of the difference of the vectors
* or vectors (Second parameter vector is subtracted from the first parameter vector,
* exactly as in the subtract function.)
*
* @instance pVector <= The normalized vector in direction (pTarget - pOrigin)
* @param pTarget => The target point.
* @param pOrigin => The origin point.
* @return The length of original difference vector.
* @bsimethod                                            EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_computeNormal
(
DVec2dP pVector,
DPoint2dCP pTarget,
DPoint2dCP pOrigin
)
    {
    pVector->x = pTarget->x - pOrigin->x;
    pVector->y = pTarget->y - pOrigin->y;
    return bsiDVec2d_normalizeInPlace (pVector);
    }

/* VBSUB(Vec3dAngleBetweenVectors) */

/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors.  This angle is between 0 and
* pi.  Rotating the first vector by this angle around the cross product
* between the vectors aligns it with the second vector.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The angle between the vectors.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_angleBetweenVectors
(
DVec2dCP pVector1,
DVec2dCP pVector2
)
    {
    double cross, dot;
    cross   = bsiDVec2d_crossProduct (pVector1, pVector2);
    dot     = bsiDVec2d_dotProduct (pVector1, pVector2);
    return  bsiTrig_atan2 (cross, dot);
    }

/* VBSUB(Vec3dSmallerAngleBetweenUnorientedVectors) */

/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors, choosing the smaller
*   of the two possible angles when both the vectors and their negations are considered.
*    This angle is between 0 and pi/2.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The angle between the vectors.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_smallerAngleBetweenUnorientedVectors
(
DVec2dCP pVector1,
DVec2dCP pVector2
)
    {
    double cross, dot;
    cross   = bsiDVec2d_crossProduct (pVector1, pVector2);
    dot     = bsiDVec2d_dotProduct (pVector1, pVector2);
    return  bsiTrig_atan2 (fabs (cross), fabs (dot));
    }

/* VBSUB(Vec3dIsVectorInSmallerSector) */

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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec2d_isVectorInSmallerSector
(
DVec2dCP pTestVector,
DVec2dCP pVector0,
DVec2dCP pVector1
)
    {
    double c = bsiDVec2d_crossProduct (pVector0, pVector1);
    return      c * bsiDVec2d_crossProduct (pVector0, pTestVector) > 0.0
            &&  c * bsiDVec2d_crossProduct (pTestVector, pVector1) > 0.0;
    }


/* VBSUB(Vec3dIsVectorInCCWSector) */

Public GEOMDLLIMPEXP bool    bsiDVec2d_isVectorInCCWSector
(
DVec2dCP pTestVector,
DVec2dCP pVector0,
DVec2dCP pVector1
)
    {
    double cross = bsiDVec2d_crossProduct (pVector0, pVector1);

    if (cross == 0.0)
        {
        double dot   = bsiDVec2d_dotProduct (pVector0, pVector1);
        if (dot > 0.0)
            return false;
        }

    if (cross > 0.0)
        return  bsiDVec2d_crossProduct (pVector0, pTestVector) >  0.0
            &&  bsiDVec2d_crossProduct (pTestVector, pVector1) >  0.0;
    else
        return  bsiDVec2d_crossProduct (pVector0, pTestVector) > 0.0
            ||  bsiDVec2d_crossProduct (pTestVector, pVector1) > 0.0;
    }

/* VBSUB(Vec3dRotateXY) */

/*-----------------------------------------------------------------*//**
* @description Rotate a vector around the z axis.
* @instance pRotatedVector <= rotated vector
* @param theta   => The rotation angle.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_rotate
(
DVec2dP pRotatedVector,
DVec2dCP pVector,
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
    }


/*-----------------------------------------------------------------*//**
* @description Rotate a vector around the z axis.
* @instance pVector <=> rotated vector
* @param theta   => The rotation angle.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_rotateInPlace
(
DVec2dP pVector,
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


/* VBSUB(Vec3dMagnitudeSquared) */


/*-----------------------------------------------------------------*//**
* @description Computes the squared magnitude of a vector.
*
* @instance pVector => The vector whose magnitude is computed.
* @return The squared magnitude of the vector.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_magnitudeSquared
(
DVec2dCP pVector
)
    {
    return   pVector->x * pVector->x
           + pVector->y * pVector->y;
    }

/* VBSUB(Vec3dUnitPerpendicularXY) */

/*---------------------------------------------------------------------------------**//**
* @description Compute a unit vector perpendicular given vector.
*
* @remarks Input may equal output.
*
* @param pRotatedVector <=      The rotated and scaled vector.  Z-coordinate is zero.
* @param pVector        =>      The source vector
* @return true if the input vector has nonzero length
* @bsimethod                                                    EarlinLutz      03/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDVec2d_unitPerpendicular
(
DVec2dP      pRotatedVector,
DVec2dCP      pVector
)
    {
    double  a, d2, x = pVector->x, y = pVector->y;
    pRotatedVector->x = -y;
    pRotatedVector->y =  x;
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
* @param pRotatedVector <=      The rotated and scaled vector.  Z-coordinate is zero.
* @param pVector        =>      The source vector
* @param bRightHanded   =>      Whether the returned vector points to the right of the projection of the given vector
* @return true if the input vector has nonzero length
* @bsimethod                                                    DavidAssaf      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDVec2d_unitPerpendicularWithHandedness
(
DVec2dP      pRotatedVector,
DVec2dCP      pVector,
bool        bRightHanded
)
    {
    double  a, d2, x = pVector->x, y = pVector->y;
    pRotatedVector->x = -y;
    pRotatedVector->y =  x;
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

/* VBSUB(Vec3dMagnitude) */


/*-----------------------------------------------------------------*//**
* @description Computes the magnitude of a vector.
* @instance pVector => The vector
* @return The length of the vector
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_magnitude
(
DVec2dCP pVector
)
    {
    return  sqrt ( pVector->x*pVector->x
                 + pVector->y*pVector->y);
    }


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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDVec2d_normalEqualTolerance
(
DVec2dCP pNormal0,
double          dot0,
DVec2dCP pNormal1,
double          dot1,
double          eps2
)
    {
    double dot;

    if (dot0 <= 0.0)
        dot0 = bsiDVec2d_magnitudeSquared (pNormal0);
    if (dot1 <= 0.0)
        dot1 = bsiDVec2d_magnitudeSquared (pNormal1);

    if (0.0 == dot0 && 0.0 == dot1)
        return true;

    double mag2 = bsiDVec2d_crossProductSquared (pNormal0, pNormal1);

    dot = bsiDVec2d_dotProduct (pNormal0, pNormal1);

    return ((mag2 <= eps2 * dot0 * dot1) && (dot > 0.0)) ? true : false;
    }


/* VBSUB(Vec3dScale) */

/*-----------------------------------------------------------------*//**
* @description Multiplies a vector by a scale factor.
* @instance pScaledVector <= The scaled vector.
* @param pVector => The vector to be scaled.
* @param scale => The scale factor.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_scale
(
DVec2dP pScaledVector,
DVec2dCP pVector,
double       scale
)
    {
    pScaledVector->x = pVector->x*scale;
    pScaledVector->y = pVector->y*scale;
    }

/* VBSUB() */

/*-----------------------------------------------------------------*//**
* @description Multiplies a vector (in place) by a scale factor.
* @instance pVector <=> scaled vector
* @param scale => The scale
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_scaleInPlace
(
DVec2dP pVector,
double       scale
)
    {
    pVector->x *= scale;
    pVector->y *= scale;
    }

/* VBSUB(Vec3dNegate) */

/*-----------------------------------------------------------------*//**
* @description Computes a negated (opposite) vector.
*
* @instance pNegated <= The negated vector.
* @param pVector => The vector to be negated.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_negate
(
DVec2dP pNegatedVector,
DVec2dCP pVector
)
    {
    pNegatedVector->x = - pVector->x;
    pNegatedVector->y = - pVector->y;
    }




/*-----------------------------------------------------------------*//**
* @description Negate a vector in place.
*
* @instance pVector <=> negated vector
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_negateInPlace
(
DVec2dP pVector
)
    {
    pVector->x = - pVector->x;
    pVector->y = - pVector->y;
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_normalize
(
DVec2dP pUnitVector,
DVec2dCP pVector
)
    {
    double  magnitude =
            sqrt ( pVector->x * pVector->x
                 + pVector->y * pVector->y);


    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        pUnitVector->x = pVector->x * f;
        pUnitVector->y = pVector->y * f;
        }
    else
        {
        pUnitVector->x = pVector->x;
        pUnitVector->y = pVector->y;
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_scaleToLength
(
DVec2dP pScaledVector,
DVec2dCP pVector,
double    length
)
    {
    double  magnitude =
            sqrt ( pVector->x * pVector->x
                 + pVector->y * pVector->y);

    if (magnitude > 0.0)
        {
        double f = length / magnitude;
        pScaledVector->x = pVector->x * f;
        pScaledVector->y = pVector->y * f;
        }
    else
        {
        pScaledVector->x = length;
        pScaledVector->y = 0.0;
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_scaleToLengthInPlace
(
DVec2dP pScaledVector,
double    length
)
    {
    return bsiDVec2d_scaleToLength (pScaledVector, pScaledVector, length);
    }



/*-----------------------------------------------------------------*//**
* @description Replaces a vector by a unit vector in the same direction, and returns
* the original length.
* If the input vector length is 0, the output vector is a zero vector
* and the returned length is 0.
*
* @instance pVector <=> vector to be normalized
* @return The length prior to normalization
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_normalizeInPlace
(
DVec2dP pVector
)
    {
    double  magnitude =
            sqrt ( pVector->x * pVector->x
                 + pVector->y * pVector->y);

    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        pVector->x *= f;
        pVector->y *= f;
        }
    return  magnitude;
    }

/* VBSUB(Vec3dNormalize) */
/*-----------------------------------------------------------------*//**
* @description Computes a unit vector in the direction of a given vector.
* If the input vector length is 0, the output vector is a zero vector.
*
* @instance pUnitVector <= The normalized vector.
* @instance pVector => The vector to normalize.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDVec2d_unitVector
(
DVec2dP pUnitVector,
DVec2dCP pVector
)
    {
    bsiDVec2d_normalize (pUnitVector, pVector);
    }

/* VBSUB(Vec3dAreVectorsParallel) */

/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are parallel.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return true if the vectors are parallel within tolerance
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec2d_areParallel
(
DVec2dCP pVector1,
DVec2dCP pVector2
)
    {
        double      a2 = bsiDVec2d_dotProduct (pVector1, pVector1);
    double      b2 = bsiDVec2d_dotProduct (pVector2, pVector2);
    double      eps = bsiTrig_smallAngle(); /* small angle tolerance (in radians) */
    double      crossSquared = bsiDVec2d_crossProductSquared (pVector1, pVector2);

    /* a2,b2,c2 are squared lengths of respective vectors */
    /* c2 = sin^2(theta) * a2 * b2 */
    /* For small theta, sin^2(theta)~~theta^2 */
    /* Zero-length vector case falls through as equality */

    return  crossSquared <= eps * eps * a2 * b2;
    }

/* VBSUB(Vec3dAreVectorsPerpendicular) */

/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are perpendicular.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return true if vectors are perpendicular within tolerance
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec2d_arePerpendicular
(
DVec2dCP pVector1,
DVec2dCP pVector2
)
    {
    double      aa = bsiDVec2d_dotProduct (pVector1, pVector1);
    double      bb = bsiDVec2d_dotProduct (pVector2, pVector2);
    double      ab = bsiDVec2d_dotProduct (pVector1, pVector2);
    double      eps = bsiTrig_smallAngle();

    return  ab * ab <= eps * eps * aa * bb;
    }

/*---------------------------------------------------------------------------------**//**
* @description Tests if two vectors are parallel.
* @remarks Use bsiTrig_smallAngle() for tolerance corresponding to bsiDVec2d_areParallel.
*
* @param pVector1   IN      the first vector
* @param pVector2   IN      the second vector
* @param tolerance  IN      radian tolerance for angle between vectors
* @return true if the vectors are parallel within tolerance
* @bsimethod                                                    DavidAssaf      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDVec2d_areParallelTolerance
(
DVec2dCP    pVector1,
DVec2dCP    pVector2,
double          tolerance
)
    {
    double      a2 = bsiDVec2d_dotProduct (pVector1, pVector1);
    double      b2 = bsiDVec2d_dotProduct (pVector2, pVector2);
    double      c2 = bsiDVec2d_crossProductSquared (pVector1, pVector2);

    /* a2,b2,c2 are squared lengths of respective vectors */
    /* c2 = sin^2(theta) * a2 * b2 */
    /* For small theta, sin^2(theta)~~theta^2 */
    /* Zero-length vector case falls through as equality */

    return c2 <= tolerance * tolerance * a2 * b2;
    }

/*---------------------------------------------------------------------------------**//**
* @description Tests if two vectors are perpendicular.
* @remarks Use bsiTrig_smallAngle() for tolerance corresponding to bsiDVec2d_arePerpendicular.
*
* @param pVector1   IN      the first vector
* @param pVector2   IN      the second vector
* @param tolerance  IN      radian tolerance for angle between vectors
* @return true if the vectors are perpendicular within tolerance
* @bsimethod                                                    DavidAssaf      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDVec2d_arePerpendicularTolerance
(
DVec2dCP    pVector1,
DVec2dCP    pVector2,
double          tolerance
)
    {
    double  aa = bsiDVec2d_dotProduct (pVector1, pVector1);
    double  bb = bsiDVec2d_dotProduct (pVector2, pVector2);
    double  ab = bsiDVec2d_dotProduct (pVector1, pVector2);

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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec2d_safeDivide
(
DVec2dP pScaledVector,
DVec2dCP pVector,
double  denominator
)
    {
    static double s_relTol = 1.0e-12;
    double absD = fabs (denominator);
    double tol = s_relTol * bsiDVec2d_maxAbs (pVector);

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
        return true;
        }
    }
/* VBSUB(Vec3dPolarAngle) */

/*-----------------------------------------------------------------*//**
* @description Return the angle component of the polar coordinates
*           form of a vector.  Z part of the vector is ignored.
* @instance pVector => vector in xyz form.
* @return angle from x axis to this vector, using only xy parts.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_polarAngle
(
DVec2dCP pVector
)
    {
    return atan2 (pVector->y, pVector->x);
    }

// COPY BLOCK 1

/* VBSUB(Vec3dZero) */

/*-----------------------------------------------------------------*//**
* @description Sets all components of a vector to zero.
* @instance pVector <= The zeroed vector.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_zero
(
DVec2dP pVector
)
    {
    pVector->x = 0.0;
    pVector->y = 0.0;
    }

/* VBSUB(Vec3dOne) */

/*-----------------------------------------------------------------*//**
* @description Returns a vector with all components 1.0.
* @instance pVector <= The initialized vector.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_one
(
DVec2dP pVector
)
    {
    pVector->x = 1.0;
    pVector->y = 1.0;
    }



/*-----------------------------------------------------------------*//**
* @description Copies doubles from a 3 component array to the x,y, and z components
* of a DVec2d
*
* @instance pVector <= vector whose components are set
* @param pXyz => x, y, z components
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_fromArray
(
DVec2dP pVector,
const   double      *pXyz
)
    {
    pVector->x = pXyz[0];
    pVector->y = pXyz[1];
    }


/* VBSUB(Vec3dFromPoint3d) */

/*-----------------------------------------------------------------*//**
* @description Initialize a vector from a point (treating the point
            as a vector from its origin).
* @instance pVector <= vector whose componts are set.
* @param pPoint => the point.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_fromDPoint2d
(
DVec2dP pVector,
DPoint2dCP pPoint
)
    {
    pVector->x = pPoint->x;
    pVector->y = pPoint->y;
    }


/* VBSUB(Vec3dFromXY) */

/*-----------------------------------------------------------------*//**
* @description Sets the x, and y components of a vector. Sets z to zero.
*
* @instance pVector <= vector whose componts are set.
* @param ax => The x component.
* @param ax => The x component.
* @param ay => The y component
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_setXY
(
DVec2dP pVector,
double       ax,
double       ay
)
    {
    pVector->x = ax;
    pVector->y = ay;
    }

/* VBSUB(Vec3dFromXYAngleAndMagnitude) */

/*-----------------------------------------------------------------*//**
* @description Sets a vector from given angle and distance in xy plane.
*
* @instance pVector <= vector whose componts are set.
* @param theta => Angle from X axis to the vector
* @param magnitude => Vector magitude
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_fromAngleAndMagnitude
(
DVec2dP pVector,
double       theta,
double       magnitude
)
    {
    pVector->x = magnitude * cos (theta);
    pVector->y = magnitude * sin (theta);
    }




/*-----------------------------------------------------------------*//**
* @description Sets the x,y, and z components of a DVec2d structure from the
* corresponding parts of a DPoint4d.  Weight part of DPoint4d is not used.
*
* @instance pVector <= vector whose components are set
* @param pHPoint => The homogeneous point
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_getXY
(
DVec2dP pVector,
DPoint4dCP pHPoint
)
    {
    pVector->x = pHPoint->x;
    pVector->y = pHPoint->y;
    }




/*-----------------------------------------------------------------*//**
* @description Set one of three components (x,y) of the vector.
*
* @instance pVector <= vector whose component is set.
* @param a => The component value.
* @param index => Selects the the axis: 0=x, 1=y
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_setComponent
(
DVec2dP pVector,
double       a,
int         index
)
    {
    index = index & 0x01;
    if (index == 0)
        {
        pVector->x = a;
        }
    else
        {
        pVector->y = a;
        }
    }

/* VBSUB(Vec3dGetComponent) */


/*-----------------------------------------------------------------*//**
* @description Gets a single component of a vector.  If the index is out of
* range 0,1 it is interpretted cyclically.
*
* @instance pVector => vector whose component is accessed.
* @param index => Indicates which component is accessed.  The values
*                       are 0=x, 1=y  Other values are treated cyclically.
* @return The specified component of the vector.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_getComponent
(
DVec2dCP pVector,
int         index
)
    {
    index = index & 0x01;
    if (index == 0)
        {
        return pVector->x;
        }
    else
        {
        return pVector->y;
        }
    }




/*-----------------------------------------------------------------*//**
* @description Copies x,y components from a vector to individual variables.
*
* @instance pVector => source vector
* @param pXCoord <= x component
* @param pYCoord <= y component
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_getComponents
(
DVec2dCP pVector,
double      *pXCoord,
double      *pYCoord
)
    {
    if (pXCoord)
        *pXCoord = pVector->x;
    if (pYCoord)
        *pYCoord = pVector->y;
    }

/* VBSUB(Vec3dAdd) */

/*-----------------------------------------------------------------*//**
* @description Compute the sum of two vectors or vectors.
*
* @instance pSum <= The computed sum.
* @param pVector1 => The the first vector
* @param pVector2 => The second vector
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_add
(
DVec2dP pSum,
DVec2dCP pVector1,
DVec2dCP pVector2
)
    {
    pSum->x = pVector1->x + pVector2->x;
    pSum->y = pVector1->y + pVector2->y;
    }



/*-----------------------------------------------------------------*//**
* @description Adds a vector to a pointer or vector, returns the result in place.
*
* @instance pSum <=> The vector to be modified.
* @param pVector => The vector to add.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_addInPlace
(
DVec2dP pSum,
DVec2dCP pVector
)
    {
    pSum->x += pVector->x;
    pSum->y += pVector->y;
    }


/*-----------------------------------------------------------------*//**
* @description Subtract two vectors, and return the result in
*           place of the first.
*
* @instance pVector2 <=> The vector to be modified.
* @param pVector2 => The vector to subtract.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_subtractInPlace
(
DVec2dP pVector1,
DVec2dCP pVector2
)
    {
    pVector1->x -= pVector2->x;
    pVector1->y -= pVector2->y;
    }

/* VBSUB(Vec3dAddScaled) */

/*-----------------------------------------------------------------*//**
* @description Adds an origin and a scaled vector.
*
* @instance pSum <= The sum
* @param pOrigin => Origin for the sum.
* @param pVector => The vector to be added.
* @param scale => The scale factor.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_addScaled
(
DVec2dP pSum,
DVec2dCP pOrigin,
DVec2dCP pVector,
double           scale
)
    {
    if (pOrigin)
        {
        pSum->x = pOrigin->x + pVector->x * scale;
        pSum->y = pOrigin->y + pVector->y * scale;
        }
    else
        {
        pSum->x = pVector->x * scale;
        pSum->y = pVector->y * scale;
        }
    }


/* VBSUB(Vec3dInterpolate) */

/*-----------------------------------------------------------------*//**
* @description Computes a vector whose position is given by a fractional
* argument and two vectors.
*
* @instance pSum <= The interpolated vector.
* @param pVector0 => The vector corresponding to fractionParameter of 0.
* @param fractionParameter => The fractional parametric coordinate.
*               0.0 is the start of the segment, 1.0 is the end, 0.5 is middle
* @param pVector1 => The vector corresponding to fractionParameter of 1.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_interpolate
(
DVec2dP pSum,
DVec2dCP pVector0,
double       fractionParameter,
DVec2dCP pVector1
)
    {
    pSum->x = pVector0->x + fractionParameter * (pVector1->x - pVector0->x);
    pSum->y = pVector0->y + fractionParameter * (pVector1->y - pVector0->y);
    }


/* VBSUB(Vec3dAdd2Scaled) */

/*-----------------------------------------------------------------*//**
* @description Adds an origin and two scaled vectors.
*
* @instance pSum <= sum
* @param pOrigin => The origin.  May be null.
* @param pVector1 => The first direction vector
* @param scale1 => The first scale factor
* @param pVector2 => The second direction vector
* @param scale2 => The second scale factor
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_add2ScaledDVec2d
(
DVec2dP pSum,
DVec2dCP pOrigin,
DVec2dCP pVector1,
double           scale1,
DVec2dCP pVector2,
double           scale2
)
    {
    if (pOrigin)
        {
        pSum->x = pOrigin->x + pVector1->x * scale1 + pVector2->x * scale2;
        pSum->y = pOrigin->y + pVector1->y * scale1 + pVector2->y * scale2;
        }
    else
        {
        pSum->x = pVector1->x * scale1 + pVector2->x * scale2;
        pSum->y = pVector1->y * scale1 + pVector2->y * scale2;
        }
    }

/* VBSUB(Vec3dAdd3Scaled) */

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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_add3ScaledDVec2d
(
DVec2dP pSum,
DVec2dCP pOrigin,
DVec2dCP pVector1,
double          scale1,
DVec2dCP pVector2,
double          scale2,
DVec2dCP pVector3,
double          scale3
)
    {
    if  (pOrigin)
        {
        pSum->x = pOrigin->x + pVector1->x * scale1 + pVector2->x * scale2 + pVector3->x * scale3;
        pSum->y = pOrigin->y + pVector1->y * scale1 + pVector2->y * scale2 + pVector3->y * scale3;
        }
    else
        {
        pSum->x = pVector1->x * scale1 + pVector2->x * scale2 + pVector3->x * scale3;
        pSum->y = pVector1->y * scale1 + pVector2->y * scale2 + pVector3->y * scale3;
        }
    }


// COPY 3
/* VBSUB(Vec3dSubtract) */

/*-----------------------------------------------------------------*//**
* @description Subtract coordinates of two vectors. (Compute Vector1 - Vector2)
*
* @instance pVector <= The difference vector
* @param pVector1 => The first vector
* @param pVector2 => The second (subtracted) vector
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_subtract
(
DVec2dP pVector,
DVec2dCP pVector1,
DVec2dCP pVector2
)
    {
    pVector->x = pVector1->x - pVector2->x;
    pVector->y = pVector1->y - pVector2->y;
    }

/* VBSUB(Vec3dSubtractPoint3DPoint2d) */

/*-----------------------------------------------------------------*//**
* @description Subtract coordinates of two points. (Compute Point1 - Point2)
*
* @instance pVector <= The difference vector
* @param pTarget => The target point
* @param pBase => The base point
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec2d_subtractDPoint2dDPoint2d
(
DVec2dP pVector,
DPoint2dCP pTarget,
DPoint2dCP pBase
)
    {
    pVector->x = pTarget->x - pBase->x;
    pVector->y = pTarget->y - pBase->y;
    }

/* VBSUB(Vec3dDistance) */

/*-----------------------------------------------------------------*//**
* @description Computes the (cartesian) distance between two vectors
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The distance between vector.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_distance
(
DVec2dCP pVector1,
DVec2dCP pVector2
)
    {
    double      xdist, ydist;

    xdist = pVector2->x - pVector1->x;
    ydist = pVector2->y - pVector1->y;

    return (sqrt (xdist*xdist + ydist*ydist));
    }
/* VBSUB(Vec3dDistanceSquared) */

/*-----------------------------------------------------------------*//**
* @description Computes the squared distance between two vectors.
*
* @instance pVector1 => The first vector.
* @param pVector2 => The second vector.
* @return The squared distance between the vectors.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_distanceSquared
(
DVec2dCP pVector1,
DVec2dCP pVector2
)
    {
    double      xdist, ydist;

    xdist = (pVector2->x - pVector1->x);
    ydist = (pVector2->y - pVector1->y);

    return (xdist*xdist + ydist*ydist);
    }


/* VBSUB(Vec3dMaxAbs) */


/*-----------------------------------------------------------------*//**
* @description Finds the largest absolute value among the components of a vector.
* @instance pVector => The vector
* @return The largest absolute value among vector coordinates.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_maxAbs
(
DVec2dCP pVector
)
    {
    double maxVal = fabs (pVector->x);

    if (fabs (pVector->y) > maxVal)
        maxVal = fabs (pVector->y);

    return maxVal;
    }
/* VBSUB(Vec3dMaxAbsDifference) */
/*-----------------------------------------------------------------*//**
* @description Finds the largest absolute value among the components of a
* difference of vectors.
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The largest absolute value among vector coordinates.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec2d_maxAbsDifference
(
DVec2dCP pVector1,
DVec2dCP pVector2
)
    {
    DVec2d diff;
    diff.x = pVector1->x - pVector2->x;
    diff.y = pVector1->y - pVector2->y;

    return bsiDVec2d_maxAbs (&diff);
    }

/* VBSUB(Vec3dEqual) */

/*-----------------------------------------------------------------*//**
* @description Test for exact equality between all components of two vectors.
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return true if the vectors are identical.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec2d_equal
(
DVec2dCP pVector1,
DVec2dCP pVector2
)
    {
    bool                    result;

    if (pVector1 && pVector2)
        {
        result =
                   pVector1->x == pVector2->x
                && pVector1->y == pVector2->y;
        }
    else if (pVector1)
        {
        result =
                   pVector1->x == 0.0
                && pVector1->y == 0.0;
        }
    else if (pVector2)
        {
        result =
                   0.0 == pVector2->x
                && 0.0 == pVector2->y;
        }
    else /* Both are null. Call them equal. */
        result = true;

    return  result;
    }

/* VBSUB(Vec3dEqualTolerance) */

/*-----------------------------------------------------------------*//**
* @description Test if the x, y, and z components of two vectors are
*   equal within tolerance.
* Tests are done independently using the absolute value of each component differences
* (i.e. not the magnitude or sum of squared differences)
*
* @instance pVector1 => The first vector.
* @param pVector2 => The second vector.
* @param tolerance => The tolerance.
* @return true if all components are within given tolerance of each other.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec2d_equalTolerance
(
DVec2dCP pVector1,
DVec2dCP pVector2,
double                  tolerance
)
    {
    bool                    result;

    if (pVector1 && pVector2)
        {
        result = fabs(pVector1->x - pVector2->x) <= tolerance &&
                 fabs(pVector1->y - pVector2->y) <= tolerance;
        }
    else if (pVector1)
        {
        result = fabs(pVector1->x) <= tolerance &&
                 fabs(pVector1->y) <= tolerance;
        }
    else if (pVector2)
        {
        result = fabs(pVector2->x) <= tolerance &&
                 fabs(pVector2->y) <= tolerance;
        }
    else /* Both are null. Call them equal. */
        result = true;

    return  result;
    }
END_BENTLEY_NAMESPACE
