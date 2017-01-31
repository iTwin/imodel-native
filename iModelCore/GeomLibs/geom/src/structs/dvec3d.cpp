/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dvec3d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------*//**
* @description @struct DVec3d
* A DVec3d structure holds cartesian components of a 3D vector
* @fields
* @field double x x component of vector
* @field double y y component of vector
* @field double z z component of vector
* @endfields
* @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE


/* VBSUB(Vec3dCrossProduct) */
/* CSVFUNC(crossProduct) */

/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
* @instance pCrossProduct <= cross product of vector1 and vector2
* @param pVector1 => The first vector
* @param pVector2 => The second vector
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_crossProduct

(
DVec3dP pCrossProduct,
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    double xx = pVector1->y * pVector2->z - pVector1->z * pVector2->y;
    double yy = pVector1->z * pVector2->x - pVector1->x * pVector2->z;
    double zz = pVector1->x * pVector2->y - pVector1->y * pVector2->x;
    pCrossProduct->x = xx;
    pCrossProduct->y = yy;
    pCrossProduct->z = zz;
    }

/* VBSUB(Vec3dCrossProduct) */
/* CSVFUNC(crossProduct) */

/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
* @instance pCrossProduct <= cross product of vector1 and vector2
* @param pVector1 => The first vector
* @param pPoint2 => The second vector, given as a point.
        The point's xyz are understood to be a vector from the origin.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_crossProductDVec3dDPoint3d

(
DVec3dP pCrossProduct,
DVec3dCP pVector1,
DPoint3dCP pPoint2
)
    {
    bsiDVec3d_crossProduct (pCrossProduct, pVector1, (DVec3d*)pPoint2);
    }

/* VBSUB(Vec3dCrossProduct) */
/* CSVFUNC(crossProduct) */

/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
* @instance pCrossProduct <= cross product of vector1 and vector2
* @param pPoint1 => The first vector, givenn as a point.
        The point's xyz are understood to be a vector from the origin.
* @param pVector2 => The second vector.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_crossProductDPoint3dDVec3d

(
DVec3dP pCrossProduct,
DPoint3dCP pPoint1,
DVec3dCP pVector2
)
    {
    bsiDVec3d_crossProduct (pCrossProduct, (DVec3d*)pPoint1, pVector2);
    }

/* VBSUB(Vec3dDotProduct) */
/* CSVFUNC(dotProduct) */

/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @param pVector1 => The first vector
* @param pVector2 => The second vector, given as point.
        The point's xyz are understood to be a vector from the origin.
* @return dot product of the two vectors.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_dotProductDVec3dDPoint3d

(
DVec3dCP pVector1,
DPoint3dCP pPoint2
)
    {
    return bsiDVec3d_dotProduct (pVector1, (DVec3d*)pPoint2);
    }

/* VBSUB(Vec3dCrossProduct3Points) */
/* CSVFUNC(crossProductToPoints) */

/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
*   The vectors are computed from the Origin to Target1 and Target2.
* @instance pCrossProduct <= product
* @param pOrigin => The base point for computing vectors.
* @param pTarget1 => The target point for the first vector.
* @param pTarget2 => The target point for the second vector.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_crossProduct3DPoint3d

(
DVec3dP pCrossProduct,
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



/* VBSUB(Vec3dCrossProductXY) */
/* CSVFUNC(crossProductXY) */

/*-----------------------------------------------------------------*//**
* @description Return the (scalar) cross product of the xy parts of two vectors.
* @param pVector1 => The first vector
* @param pVector2 => The second vector
* @return The 2d cross product.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_crossProductXY

(
DVec3dCP pVector1,
DVec3dCP pVector2
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_normalizedCrossProduct

(
DVec3dP pCrossProduct,
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    bsiDVec3d_crossProduct( pCrossProduct, pVector1, pVector2 );
    return bsiDVec3d_normalizeInPlace( pCrossProduct );
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_sizedCrossProduct

(
DVec3dP pProduct,
DVec3dCP pVector1,
DVec3dCP pVector2,
double      productLength
)
    {
    double length;
    bsiDVec3d_crossProduct( pProduct, pVector1, pVector2 );
    length = bsiDVec3d_magnitude (pProduct);
    if (length != 0)
        {
        bsiDVec3d_scale (pProduct, pProduct, productLength / length);
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_geometricMeanCrossProduct

(
DVec3dP pProduct,
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    double length;
    double aa, bb, c;
    bsiDVec3d_crossProduct( pProduct, pVector1, pVector2 );
    length = bsiDVec3d_magnitude (pProduct);
    aa = bsiDVec3d_magnitudeSquared (pVector1);
    bb = bsiDVec3d_magnitudeSquared (pVector2);
    if (length != 0)
        {
        c = sqrt (sqrt (aa * bb));
        bsiDVec3d_scale (pProduct, pProduct, c / length);
        }
    return length;
    }

/* VBSUB(Vec3dTripleProduct) */
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_tripleProduct

(
DVec3dCP pVector1,
DVec3dCP pVector2,
DVec3dCP pVector3
)
    {
    return
          pVector1->x * ( pVector2->y * pVector3->z - pVector2->z * pVector3->y )
        + pVector1->y * ( pVector2->z * pVector3->x - pVector2->x * pVector3->z )
        + pVector1->z * ( pVector2->x * pVector3->y - pVector2->y * pVector3->x )
        ;
    }


/* VBSUB(Vec3dDotProduct) */
/* CSVFUNC(dotProduct) */

/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The dot product of the two vectors
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_dotProduct

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    return (pVector1->x*pVector2->x + pVector1->y*pVector2->y + pVector1->z*pVector2->z);
    }


/* VBSUB(Vec3dDotProductXY) */
/* CSVFUNC(dotProductXY) */

/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of xy parts of two vectors.
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The dot product of the xy parts of the two vectors
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_dotProductXY

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    return (pVector1->x*pVector2->x + pVector1->y*pVector2->y);
    }

/* VBSUB(Vec3dDotProductXYZ) */
/* CSVFUNC(dotProduct) */

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
Public GEOMDLLIMPEXP double bsiDVec3d_dotXYZ

(
DVec3dCP pVector,
double    ax,
double    ay,
double    az
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec3d_getTriad

(
DVec3dCP pGivenAxis,
DVec3dP pXAxis,
DVec3dP pYAxis,
DVec3dP pZAxis
)
    {
    DVec3d      vector;
    double      length = bsiDVec3d_magnitude (pGivenAxis);
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

    bsiDVec3d_crossProduct (pXAxis, &vector, pZAxis);
    bsiDVec3d_crossProduct (pYAxis, pZAxis, pXAxis);
    bsiDVec3d_normalizeInPlace (pXAxis);
    bsiDVec3d_normalizeInPlace (pYAxis);
    bsiDVec3d_normalizeInPlace (pZAxis);
    bsiDVec3d_scale (pXAxis, pXAxis, length);
    bsiDVec3d_scale (pYAxis, pYAxis, length);
    bsiDVec3d_scale (pZAxis, pZAxis, length);

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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec3d_getNormalizedTriad

(
DVec3dCP pGivenAxis,
DVec3dP pXAxis,
DVec3dP pYAxis,
DVec3dP pZAxis
)
    {
    DVec3d      vector;
    double      length = bsiDVec3d_magnitude( pGivenAxis );
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

    bsiDVec3d_crossProduct (pXAxis, &vector, pZAxis);
    bsiDVec3d_crossProduct (pYAxis, pZAxis, pXAxis);
    bsiDVec3d_normalizeInPlace (pXAxis);
    bsiDVec3d_normalizeInPlace (pYAxis);
    bsiDVec3d_normalizeInPlace (pZAxis);

    return  boolStat;
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
Public GEOMDLLIMPEXP void bsiDVec3d_applyGivensRotation

(
DVec3dP pOut0,
DVec3dP pOut1,
DVec3dCP pIn0,
DVec3dCP pIn1,
double          c,
double          s
)
    {
    double a, b;
    int i;
    double *p0 = (double*)pIn0;
    double *p1 = (double*)pIn1;
    double *q0 = (double*)pOut0;
    double *q1 = (double*)pOut1;

    for (i = 0; i < 3; i++)
        {
        a = p0[i];
        b = p1[i];
        q0[i] = a * c + b * s;
        q1[i] = b * c - a * s;
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
Public GEOMDLLIMPEXP void bsiDVec3d_applyHyperbolicReflection

(
DVec3dP pOut0,
DVec3dP pOut1,
DVec3dCP pIn0,
DVec3dCP pIn1,
double          secant,
double          tangent
)
    {
    double a, b;
    int i;
    double *p0 = (double*)pIn0;
    double *p1 = (double*)pIn1;
    double *q0 = (double*)pOut0;
    double *q1 = (double*)pOut1;

    for (i = 0; i < 3; i++)
        {
        a = p0[i];
        b = p1[i];
        q0[i] = a * secant + b * tangent;
        q1[i] = b * secant + a * tangent;
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
Public GEOMDLLIMPEXP double bsiDVec3d_computeNormal

(
DVec3dP pVector,
DPoint3dCP pTarget,
DPoint3dCP pOrigin
)
    {
    pVector->x = pTarget->x - pOrigin->x;
    pVector->y = pTarget->y - pOrigin->y;
    pVector->z = pTarget->z - pOrigin->z;
    return bsiDVec3d_normalizeInPlace (pVector);
    }

/* VBSUB(Vec3dAngleBetweenVectors) */
/* CSVFUNC(angleTo) */

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
Public GEOMDLLIMPEXP double bsiDVec3d_angleBetweenVectors

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    DVec3d   crossProd;
    double cross, dot;
    bsiDVec3d_crossProduct (&crossProd, pVector1, pVector2);
    cross   = bsiDVec3d_magnitude (&crossProd);
    dot     = bsiDVec3d_dotProduct (pVector1, pVector2);
    return  bsiTrig_atan2 (cross, dot);
    }

/* VBSUB(Vec3dSmallerAngleBetweenUnorientedVectors) */
/* CSVFUNC(smallerUnorientedAngleTo) */

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
Public GEOMDLLIMPEXP double bsiDVec3d_smallerAngleBetweenUnorientedVectors

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    DVec3d   crossProd;
    double cross, dot;
    bsiDVec3d_crossProduct (&crossProd, pVector1, pVector2);
    cross   = bsiDVec3d_magnitude (&crossProd);
    dot     = bsiDVec3d_dotProduct (pVector1, pVector2);
    return  bsiTrig_atan2 (cross, fabs (dot));
    }

/* VBSUB(Vec3dIsVectorInSmallerSector) */
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec3d_isVectorInSmallerSector

(
DVec3dCP pTestVector,
DVec3dCP pVector0,
DVec3dCP pVector1
)
    {
    DVec3d   cross01;
    bsiDVec3d_crossProduct (&cross01, pVector0, pVector1);
    return      bsiDVec3d_tripleProduct (pVector0, pTestVector, &cross01) > 0.0
            &&  bsiDVec3d_tripleProduct (pTestVector, pVector1, &cross01) > 0.0;
    }


/* VBSUB(Vec3dIsVectorInCCWSector) */
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec3d_isVectorInCCWSector

(
DVec3dCP pTestVector,
DVec3dCP pVector0,
DVec3dCP pVector1,
DVec3dCP pUpVector
)
    {
    DVec3d  cross01;
    double  dot;

    bsiDVec3d_crossProduct (&cross01, pVector0, pVector1);

    if (bsiDVec3d_equal (NULL, &cross01))
        {
        dot = bsiDVec3d_dotProduct (pVector0, pVector1);
        if (dot > 0.0)
            return false;
        }

    if (pUpVector)
        dot = bsiDVec3d_dotProduct (&cross01, pUpVector);
    else
        dot = cross01.z;

    if (dot > 0.0)
        return  bsiDVec3d_tripleProduct (pVector0, pTestVector, &cross01) > 0.0
            &&  bsiDVec3d_tripleProduct (pTestVector, pVector1, &cross01) > 0.0;
    else
        return  bsiDVec3d_tripleProduct (pVector0, pTestVector, &cross01) < 0.0
            ||  bsiDVec3d_tripleProduct (pTestVector, pVector1, &cross01) < 0.0;
    }

/* VBSUB(Vec3dIsVectorInCCWXYSector) */
/*-----------------------------------------------------------------*//**
* @description Test if the test vector vector is "between" vector0 and vector1, in CCW sense using only xy-components of the 3 vectors.
* @remarks The containment test is strict in the sense that if the xy-components of the test vector equal those of a boundary vector, this function returns false.
* @instance pTestVector => The vector to test
* @param pVector0 => The boundary vector
* @param pVector1 => The boundary vector
* @return true if test vector is within the angle.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec3d_isVectorInCCWXYSector

(
DVec3dCP pTestVector,
DVec3dCP pVector0,
DVec3dCP pVector1
)
    {
    double cross = bsiDVec3d_crossProductXY (pVector0, pVector1);

    if (cross == 0.0)
        {
        double dot   = bsiDVec3d_dotProductXY (pVector0, pVector1);
        if (dot > 0.0)
            return false;
        }

    if (cross > 0.0)
        return  bsiDVec3d_crossProductXY (pVector0, pTestVector) > 0.0
            &&  bsiDVec3d_crossProductXY (pTestVector, pVector1) > 0.0;
    else
        return  bsiDVec3d_crossProductXY (pVector0, pTestVector) > 0.0
            ||  bsiDVec3d_crossProductXY (pTestVector, pVector1) > 0.0;
    }

/* VBSUB(Vec3dAngleBetweenVectorsXY) */
/* CSVFUNC(angleToXY) */

/*-----------------------------------------------------------------*//**
* @description Returns the angle from Vector1 to Vector2 using only xy parts.
*  This angle is between -pi and +pi.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The angle between vectors.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_angleBetweenVectorsXY

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    double cross, dot;
    cross = bsiDVec3d_crossProductXY (pVector1, pVector2);
    dot     = bsiDVec3d_dotProductXY (pVector1, pVector2);
    return  bsiTrig_atan2 (cross, dot);
    }

/* VBSUB(Vec3dSmallerAngleBetweenUnorientedVectorsXY) */
/* CSVFUNC(smallerUnorientedAngleToXY) */

/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors, considering both
*   the vectors and their negations and choosing the smaller.
*   This angle is between 0 and pi/2.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The angle between vectors.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_smallerAngleBetweenUnorientedVectorsXY

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    double cross, dot;
    cross = bsiDVec3d_crossProductXY (pVector1, pVector2);
    dot     = bsiDVec3d_dotProductXY (pVector1, pVector2);
    return  bsiTrig_atan2 (fabs (cross), fabs(dot));
    }


/* VBSUB(Vec3dRotateXY) */
/* CSVFUNC(rotateXY) */

/*-----------------------------------------------------------------*//**
* @description Rotate a vector around the z axis.
* @instance pRotatedVector <= rotated vector
* @param theta   => The rotation angle.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_rotateXY

(
DVec3dP pRotatedVector,
DVec3dCP pVector,
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_rotateXYInPlace

(
DVec3dP pVector,
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

/* VBSUB(Vec3dSignedAngleBetweenVectors) */
/* CSVFUNC(signedAngleTo) */

/*-----------------------------------------------------------------*//**
* @description Computes the signed angle from one vector to another, in the plane
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_signedAngleBetweenVectors

(
DVec3dCP pVector1,
DVec3dCP pVector2,
DVec3dCP pOrientationVector
)
    {
    DVec3d   crossProd;
    double cross, dot, theta;
    bsiDVec3d_crossProduct (&crossProd, pVector1, pVector2);
    cross   = bsiDVec3d_magnitude (&crossProd);
    dot     = bsiDVec3d_dotProduct (pVector1, pVector2);
    theta   = bsiTrig_atan2 (cross, dot);

    if (bsiDVec3d_dotProduct (&crossProd, pOrientationVector) < 0.0)
        return  -theta;
    else
        return  theta;
    }

/* VBSUB(Vec3dPlanarAngleBetweenVectors) */
/* CSVFUNC(planarAngleTo) */

/*-----------------------------------------------------------------*//**
* @description Computes the signed angle between the projection of two vectors
*       onto a plane with given normal.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @param pPlaneNormal => The plane normal vector
* @return The angle in plane
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_planarAngleBetweenVectors

(
DVec3dCP pVector1,
DVec3dCP pVector2,
DVec3dCP pPlaneNormal
)
    {
    DVec3d      projection1, projection2;
    double      square = bsiDVec3d_dotProduct (pPlaneNormal, pPlaneNormal);
    double      projectionFactor1, projectionFactor2;
    double      factor;

    if (square == 0.0)
        return 0.0;

    factor = 1.0 / square;

    projectionFactor1 = - bsiDVec3d_dotProduct (pVector1, pPlaneNormal) * factor;
    projectionFactor2 = - bsiDVec3d_dotProduct (pVector2,  pPlaneNormal) * factor;

    bsiDVec3d_addScaled (&projection1, pVector1, pPlaneNormal, projectionFactor1);
    bsiDVec3d_addScaled (&projection2, pVector2, pPlaneNormal, projectionFactor2);

    return  bsiDVec3d_signedAngleBetweenVectors (&projection1, &projection2, pPlaneNormal);
    }

/*-----------------------------------------------------------------*//**
* @description Scale each (homogeneous) point by the other's weight and subtract, i.e. form
* (point1 * point2.w - point2 * point1.w).  The weight term
* vanishes.   Copy the xyz parts back as a vector.
*
* @instance pDifference <= The difference vector
* @param pHPoint1 => The first homogeneous point
* @param pHPoint2 => The second homogeneous point.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_weightedDifference

(
DVec3dP pDifference,
DPoint4dCP pHPoint1,
DPoint4dCP pHPoint2
)
    {
    double w2 = pHPoint2->w;
    double w1 = pHPoint1->w;
    pDifference->x = pHPoint1->x * w2 - pHPoint2->x * w1;
    pDifference->y = pHPoint1->y * w2 - pHPoint2->y * w1;
    pDifference->z = pHPoint1->z * w2 - pHPoint2->z * w1;
    }


/*-----------------------------------------------------------------*//**
* @description Form the cross product of the weighted differences from base poitn
    to two targets
*
* @instance pProduct <= cross product result
* @param pBasePoint => The common base point (second point for differences)
* @param pTarget1 => The first target point.
* @param pTarget2 => The second target point.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_weightedDifferenceCrossProduct

(
DVec3dP pProduct,
DPoint4dCP pBasePoint,
DPoint4dCP pTarget1,
DPoint4dCP pTarget2
)
    {
    DVec3d   U, V;
    bsiDVec3d_weightedDifference (&U, pTarget1, pBasePoint);
    bsiDVec3d_weightedDifference (&V, pTarget2, pBasePoint);
    bsiDVec3d_crossProduct (pProduct, &U, &V);
    }

/* VBSUB(Vec3dMagnitudeSquared) */


/*-----------------------------------------------------------------*//**
* @description Computes the squared magnitude of a vector.
*
* @instance pVector => The vector whose magnitude is computed.
* @return The squared magnitude of the vector.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_magnitudeSquared

(
DVec3dCP pVector
)
    {
    return   pVector->x * pVector->x
           + pVector->y * pVector->y
           + pVector->z * pVector->z;
    }

/* VBSUB(Vec3dMagnitudeXY) */
/* CSVFUNC(magnitudeXY) */

/*-----------------------------------------------------------------*//**
* @description Computes the magnitude of the xy part of a vector.
* @instance pVector => The vector
* @return The magnitude of the xy parts of the given vector.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_magnitudeXY

(
DVec3dCP pVector
)
    {
    return sqrt (pVector->x * pVector->x + pVector->y * pVector->y);
    }

/* VBSUB(Vec3dMagnitudeSquaredXY) */
/* CSVFUNC(magnitudeSquaredXY) */

/*-----------------------------------------------------------------*//**
* @description Computes the squared magnitude of the xy part of a vector.
* @instance pVector => The vector
* @return The squared magnitude of the xy parts of the given vector.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_magnitudeSquaredXY

(
DVec3dCP pVector
)
    {
    return pVector->x * pVector->x + pVector->y * pVector->y;
    }

/* VBSUB(Vec3dUnitPerpendicularXY) */

/*---------------------------------------------------------------------------------**//**
* @description Compute a unit vector perpendicular to the xy parts of given vector.
*
* @remarks Input may equal output.
*
* @param pRotatedVector <=      The rotated and scaled vector.  Z-coordinate is zero.
* @param pVector        =>      The source vector
* @return true if the input vector has nonzero length
* @bsimethod                                                    EarlinLutz      03/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDVec3d_unitPerpendicularXY

(
DVec3dP      pRotatedVector,
DVec3dCP      pVector
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
* @param pRotatedVector <=      The rotated and scaled vector.  Z-coordinate is zero.
* @param pVector        =>      The source vector
* @param bRightHanded   =>      Whether the returned vector points to the right of the projection of the given vector in the xy-plane.
* @return true if the input vector has nonzero length
* @bsimethod                                                    DavidAssaf      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDVec3d_unitPerpendicularXYWithHandedness

(
DVec3dP      pRotatedVector,
DVec3dCP      pVector,
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

/* VBSUB(Vec3dMagnitude) */


/*-----------------------------------------------------------------*//**
* @description Computes the magnitude of a vector.
* @instance pVector => The vector
* @return The length of the vector
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_magnitude

(
DVec3dCP pVector
)
    {
    return  sqrt ( pVector->x*pVector->x
                 + pVector->y*pVector->y
                 + pVector->z*pVector->z);
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
Public GEOMDLLIMPEXP bool     bsiDVec3d_normalEqualTolerance

(
DVec3dCP pNormal0,
double          dot0,
DVec3dCP pNormal1,
double          dot1,
double          eps2
)
    {
    DVec3d   cross;
    double mag2, dot;

    if (dot0 <= 0.0)
        dot0 = bsiDVec3d_magnitudeSquared (pNormal0);
    if (dot1 <= 0.0)
        dot1 = bsiDVec3d_magnitudeSquared (pNormal1);

    if (0.0 == dot0 && 0.0 == dot1)
        return true;

    bsiDVec3d_crossProduct (&cross, pNormal0, pNormal1);
    mag2 = bsiDVec3d_magnitudeSquared (&cross);
    dot = bsiDVec3d_dotProduct (pNormal0, pNormal1);

    return ((mag2 <= eps2 * dot0 * dot1) && (dot > 0.0)) ? true : false;
    }


/* VBSUB(Vec3dScale) */
/* CSVFUNC(scale) */

/*-----------------------------------------------------------------*//**
* @description Multiplies a vector by a scale factor.
* @instance pScaledVector <= The scaled vector.
* @param pVector => The vector to be scaled.
* @param scale => The scale factor.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_scale

(
DVec3dP pScaledVector,
DVec3dCP pVector,
double       scale
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_scaleInPlace

(
DVec3dP pVector,
double       scale
)
    {
    pVector->x *= scale;
    pVector->y *= scale;
    pVector->z *= scale;
    }

/* VBSUB(Vec3dNegate) */
/* CSVFUNC(negate) */

/*-----------------------------------------------------------------*//**
* @description Computes a negated (opposite) vector.
*
* @instance pNegated <= The negated vector.
* @param pVector => The vector to be negated.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_negate

(
DVec3dP pNegatedVector,
DVec3dCP pVector
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_negateInPlace

(
DVec3dP pVector
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_normalize

(
DVec3dP pUnitVector,
DVec3dCP pVector
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_scaleToLength

(
DVec3dP pScaledVector,
DVec3dCP pVector,
double    length
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_scaleToLengthInPlace

(
DVec3dP pScaledVector,
double    length
)
    {
    return bsiDVec3d_scaleToLength (pScaledVector, pScaledVector, length);
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
Public GEOMDLLIMPEXP double bsiDVec3d_normalizeInPlace

(
DVec3dP pVector
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


/* VBSUB(Vec3dNormalize) */
/* CSVFUNC(normalize) */
/*-----------------------------------------------------------------*//**
* @description Computes a unit vector in the direction of a given vector.
* If the input vector length is 0, the output vector is a zero vector.
*
* @instance pUnitVector <= The normalized vector.
* @instance pVector => The vector to normalize.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDVec3d_unitVector

(
DVec3dP pUnitVector,
DVec3dCP pVector
)
    {
    bsiDVec3d_normalize (pUnitVector, pVector);
    }


/* VBSUB(Vec3dAreVectorsParallel) */
/* CSVFUNC(isParallelTo) */

/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are parallel.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return true if the vectors are parallel within tolerance
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec3d_areParallel

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    DVec3d      vecC;
    double      a2 = bsiDVec3d_dotProduct (pVector1, pVector1);
    double      b2 = bsiDVec3d_dotProduct (pVector2, pVector2);
    double      cross;
    double      eps = bsiTrig_smallAngle(); /* small angle tolerance (in radians) */
    bsiDVec3d_crossProduct (&vecC, pVector1, pVector2);
    cross = bsiDVec3d_magnitudeSquared (&vecC);

    /* a2,b2,c2 are squared lengths of respective vectors */
    /* c2 = sin^2(theta) * a2 * b2 */
    /* For small theta, sin^2(theta)~~theta^2 */
    /* Zero-length vector case falls through as equality */

    return  cross <= eps * eps * a2 * b2;
    }

/* VBSUB(Vec3dAreVectorsPerpendicular) */
/* CSVFUNC(isPerpendicularTo) */

/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are perpendicular.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return true if vectors are perpendicular within tolerance
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec3d_arePerpendicular

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    double      aa = bsiDVec3d_dotProduct (pVector1, pVector1);
    double      bb = bsiDVec3d_dotProduct (pVector2, pVector2);
    double      ab = bsiDVec3d_dotProduct (pVector1, pVector2);
    double      eps = bsiTrig_smallAngle();

    return  ab * ab <= eps * eps * aa * bb;
    }

/*---------------------------------------------------------------------------------**//**
* @description Tests if two vectors are parallel.
* @remarks Use bsiTrig_smallAngle() for tolerance corresponding to bsiDVec3d_areParallel.
*
* @param pVector1   IN      the first vector
* @param pVector2   IN      the second vector
* @param tolerance  IN      radian tolerance for angle between vectors
* @return true if the vectors are parallel within tolerance
* @bsimethod                                                    DavidAssaf      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDVec3d_areParallelTolerance

(
DVec3dCP    pVector1,
DVec3dCP    pVector2,
double          tolerance
)
    {
    DVec3d      cross;
    double      a2 = bsiDVec3d_dotProduct (pVector1, pVector1);
    double      b2 = bsiDVec3d_dotProduct (pVector2, pVector2);
    double      c2;

    bsiDVec3d_crossProduct (&cross, pVector1, pVector2);

    c2 = bsiDVec3d_dotProduct (&cross, &cross);

    /* a2,b2,c2 are squared lengths of respective vectors */
    /* c2 = sin^2(theta) * a2 * b2 */
    /* For small theta, sin^2(theta)~~theta^2 */
    /* Zero-length vector case falls through as equality */

    return c2 <= tolerance * tolerance * a2 * b2;
    }

/*---------------------------------------------------------------------------------**//**
* @description Tests if two vectors are perpendicular.
* @remarks Use bsiTrig_smallAngle() for tolerance corresponding to bsiDVec3d_arePerpendicular.
*
* @param pVector1   IN      the first vector
* @param pVector2   IN      the second vector
* @param tolerance  IN      radian tolerance for angle between vectors
* @return true if the vectors are perpendicular within tolerance
* @bsimethod                                                    DavidAssaf      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDVec3d_arePerpendicularTolerance

(
DVec3dCP    pVector1,
DVec3dCP    pVector2,
double          tolerance
)
    {
    double  aa = bsiDVec3d_dotProduct (pVector1, pVector1);
    double  bb = bsiDVec3d_dotProduct (pVector2, pVector2);
    double  ab = bsiDVec3d_dotProduct (pVector1, pVector2);

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
Public GEOMDLLIMPEXP bool    bsiDVec3d_safeDivide

(
DVec3dP pScaledVector,
DVec3dCP pVector,
double  denominator
)
    {
    static double s_relTol = 1.0e-12;
    double absD = fabs (denominator);
    double tol = s_relTol * bsiDVec3d_maxAbs (pVector);

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
/* VBSUB(Vec3dPolarAngle) */

/*-----------------------------------------------------------------*//**
* @description Return the angle component of the polar coordinates
*           form of a vector.  Z part of the vector is ignored.
* @instance pVector => vector in xyz form.
* @return angle from x axis to this vector, using only xy parts.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_polarAngle

(
DVec3dCP pVector
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
Public GEOMDLLIMPEXP void bsiDVec3d_zero

(
DVec3dP pVector
)
    {
    pVector->x = 0.0;
    pVector->y = 0.0;
    pVector->z = 0.0;
    }

/* VBSUB(Vec3dOne) */

/*-----------------------------------------------------------------*//**
* @description Returns a vector with all components 1.0.
* @instance pVector <= The initialized vector.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_one

(
DVec3dP pVector
)
    {
    pVector->x = 1.0;
    pVector->y = 1.0;
    pVector->z = 1.0;
    }



/*-----------------------------------------------------------------*//**
* @description Copies doubles from a 3 component array to the x,y, and z components
* of a DVec3d
*
* @instance pVector <= vector whose components are set
* @param pXyz => x, y, z components
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_fromArray

(
DVec3dP pVector,
const   double      *pXyz
)
    {
    pVector->x = pXyz[0];
    pVector->y = pXyz[1];
    pVector->z = pXyz[2];
    }

// COPY BLOCK 2
/* VBSUB(Vec3dFromXYZ) */
/* CSVFUNC(fromXYZ) */

/*-----------------------------------------------------------------*//**
* @description Sets the x,y, and z components of a vector
*
* @instance pVector <= vector whose componts are set.
* @param ax => The x component.
* @param ay => The y component.
* @param az => The z component.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_setXYZ

(
DVec3dP pVector,
double       ax,
double       ay,
double       az
)
    {
    pVector->x = ax;
    pVector->y = ay;
    pVector->z = az;
    }

/* VBSUB(Vec3dFromPoint3d) */

/*-----------------------------------------------------------------*//**
* @description Initialize a vector from a point (treating the point
            as a vector from its origin).
* @instance pVector <= vector whose componts are set.
* @param pPoint => the point.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_fromDPoint3d

(
DVec3dP pVector,
DPoint3dCP pPoint
)
    {
    pVector->x = pPoint->x;
    pVector->y = pPoint->y;
    pVector->z = pPoint->z;
    }


/* VBSUB(Vec3dFromXY) */
/* CSVFUNC(FromXY) */

/*-----------------------------------------------------------------*//**
* @description Sets the x, and y components of a vector. Sets z to zero.
*
* @instance pVector <= vector whose componts are set.
* @param ax => The x component.
* @param ax => The x component.
* @param ay => The y component
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_setXY

(
DVec3dP pVector,
double       ax,
double       ay
)
    {
    pVector->x = ax;
    pVector->y = ay;
    pVector->z = 0.0;
    }

/* VBSUB(Vec3dFromXYAngleAndMagnitude) */

/*-----------------------------------------------------------------*//**
* @description Sets a vector from given angle and distance in xy plane.
*       Z part is set to zero.
*
* @instance pVector <= vector whose componts are set.
* @param theta => Angle from X axis to the vector, in the xy plane.
* @param magnitude => Vector magitude
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_fromXYAngleAndMagnitude

(
DVec3dP pVector,
double       theta,
double       magnitude
)
    {
    pVector->x = magnitude * cos (theta);
    pVector->y = magnitude * sin (theta);
    pVector->z = 0.0;
    }




/*-----------------------------------------------------------------*//**
* @description Sets the x,y, and z components of a DVec3d structure from the
* corresponding parts of a DPoint4d.  Weight part of DPoint4d is not used.
*
* @instance pVector <= vector whose components are set
* @param pHPoint => The homogeneous point
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_getXYZ

(
DVec3dP pVector,
DPoint4dCP pHPoint
)
    {
    pVector->x = pHPoint->x;
    pVector->y = pHPoint->y;
    pVector->z = pHPoint->z;
    }




/*-----------------------------------------------------------------*//**
* @description Set one of three components (x,y,z) of the vector.
*
* @instance pVector <= vector whose component is set.
* @param a => The component value.
* @param index => Selects the the axis: 0=x, 1=y, 2=z, others cyclic.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_setComponent

(
DVec3dP pVector,
double       a,
int         index
)
    {
    if (index > 2)
        index = index % 3;
    if (index == 0)
        {
        pVector->x = a;
        }
    else if (index == 1)
        {
        pVector->y = a;
        }
    else if (index == 2)
        {
        pVector->z = a;
        }
    else /* index < 0.*/
        {
        bsiDVec3d_setComponent (pVector, a, 3 - ( (-index) % 3));
        }
    }

/* VBSUB(Vec3dGetComponent) */


/*-----------------------------------------------------------------*//**
* @description Gets a single component of a vector.  If the index is out of
* range 0,1,2, it is interpreted cyclically.
*
* @instance pVector => vector whose component is accessed.
* @param index => Indicates which component is accessed.  The values
*                       are 0=x, 1=y, 2=z.  Other values are treated cyclically.
* @return The specified component of the vector.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_getComponent

(
DVec3dCP pVector,
int         index
)
    {
    if (index > 2)
        index = index % 3;
    if (index == 0)
        {
        return pVector->x;
        }
    else if (index == 1)
        {
        return pVector->y;
        }
    else if (index == 2)
        {
        return pVector->z;
        }
    else /* index < 0.*/
        {
        return bsiDVec3d_getComponent (pVector, 3 - ( (-index) % 3));
        }
    }




/*-----------------------------------------------------------------*//**
* @description Copies x,y,z components from a vector to individual variables.
*
* @instance pVector => source vector
* @param pXCoord <= x component
* @param pYCoord <= y component
* @param pZCoord <= z component
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_getComponents

(
DVec3dCP pVector,
double      *pXCoord,
double      *pYCoord,
double      *pZCoord
)
    {
    if (pXCoord)
        *pXCoord = pVector->x;
    if (pYCoord)
        *pYCoord = pVector->y;
    if (pZCoord)
        *pZCoord = pVector->z;
    }
/* VBSUB(Vec3dAdd) */
/* CSVFUNC(Add) */

/*-----------------------------------------------------------------*//**
* @description Compute the sum of two vectors or vectors.
*
* @instance pSum <= The computed sum.
* @param pVector1 => The the first vector
* @param pVector2 => The second vector
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_add

(
DVec3dP pSum,
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    pSum->x = pVector1->x + pVector2->x;
    pSum->y = pVector1->y + pVector2->y;
    pSum->z = pVector1->z + pVector2->z;
    }



/*-----------------------------------------------------------------*//**
* @description Adds a vector to a pointer or vector, returns the result in place.
*
* @instance pSum <=> The vector to be modified.
* @param pVector => The vector to add.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_addInPlace

(
DVec3dP pSum,
DVec3dCP pVector
)
    {
    pSum->x += pVector->x;
    pSum->y += pVector->y;
    pSum->z += pVector->z;
    }


/*-----------------------------------------------------------------*//**
* @description Subtract two vectors, and return the result in
*           place of the first.
*
* @instance pVector2 <=> The vector to be modified.
* @param pVector2 => The vector to subtract.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_subtractInPlace

(
DVec3dP pVector1,
DVec3dCP pVector2
)
    {
    pVector1->x -= pVector2->x;
    pVector1->y -= pVector2->y;
    pVector1->z -= pVector2->z;
    }

/* VBSUB(Vec3dAddScaled) */
/* CSVFUNC(Add) */

/*-----------------------------------------------------------------*//**
* @description Adds an origin and a scaled vector.
*
* @instance pSum <= The sum
* @param pOrigin => Origin for the sum.
* @param pVector => The vector to be added.
* @param scale => The scale factor.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_addScaled

(
DVec3dP pSum,
DVec3dCP pOrigin,
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


/* VBSUB(Vec3dInterpolate) */
/* CSVFUNC(interpolate) */

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
Public GEOMDLLIMPEXP void bsiDVec3d_interpolate

(
DVec3dP pSum,
DVec3dCP pVector0,
double       fractionParameter,
DVec3dCP pVector1
)
    {
    pSum->x = pVector0->x + fractionParameter * (pVector1->x - pVector0->x);
    pSum->y = pVector0->y + fractionParameter * (pVector1->y - pVector0->y);
    pSum->z = pVector0->z + fractionParameter * (pVector1->z - pVector0->z);
    }


/* VBSUB(Vec3dAdd2Scaled) */
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_add2ScaledDVec3d

(
DVec3dP pSum,
DVec3dCP pOrigin,
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

/* VBSUB(Vec3dAdd3Scaled) */
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
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_add3ScaledDVec3d

(
DVec3dP pSum,
DVec3dCP pOrigin,
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


// COPY 3
/* VBSUB(Vec3dSubtract) */
/* CSVFUNC(Subtract) */

/*-----------------------------------------------------------------*//**
* @description Subtract coordinates of two vectors. (Compute Vector1 - Vector2)
*
* @instance pVector <= The difference vector
* @param pVector1 => The first vector
* @param pVector2 => The second (subtracted) vector
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_subtract

(
DVec3dP pVector,
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    pVector->x = pVector1->x - pVector2->x;
    pVector->y = pVector1->y - pVector2->y;
    pVector->z = pVector1->z - pVector2->z;
    }

/* VBSUB(Vec3dSubtractPoint3dPoint3d) */

/*-----------------------------------------------------------------*//**
* @description Subtract coordinates of two points. (Compute Point1 - Point2)
*
* @instance pVector <= The difference vector
* @param pTarget => The target point
* @param pBase => The base point
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_subtractDPoint3dDPoint3d

(
DVec3dP pVector,
DPoint3dCP pTarget,
DPoint3dCP pBase
)
    {
    pVector->x = pTarget->x - pBase->x;
    pVector->y = pTarget->y - pBase->y;
    pVector->z = pTarget->z - pBase->z;
    }

/* VBSUB(Vec3dDistance) */
/* CSVFUNC(distance) */

/*-----------------------------------------------------------------*//**
* @description Computes the (cartesian) distance between two vectors
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The distance between vector.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_distance

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    double      xdist, ydist, zdist;

    xdist = pVector2->x - pVector1->x;
    ydist = pVector2->y - pVector1->y;
    zdist = pVector2->z - pVector1->z;

    return (sqrt (xdist*xdist + ydist*ydist + zdist*zdist));
    }
/* VBSUB(Vec3dDistanceSquared) */
/* CSVFUNC(distanceSquared) */

/*-----------------------------------------------------------------*//**
* @description Computes the squared distance between two vectors.
*
* @instance pVector1 => The first vector.
* @param pVector2 => The second vector.
* @return The squared distance between the vectors.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_distanceSquared

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    double      xdist, ydist, zdist;

    xdist = (pVector2->x - pVector1->x);
    ydist = (pVector2->y - pVector1->y);
    zdist = (pVector2->z - pVector1->z);

    return (xdist*xdist + ydist*ydist + zdist*zdist);
    }

/* VBSUB(Vec3dDistanceSquaredXY) */
/* CSVFUNC(distanceSquaredXY) */

/*-----------------------------------------------------------------*//**
* @description Computes the squared distance between two vectors, using only the
*       xy parts.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The squared distance between the XY projections of the two vectors.
*               (i.e. any z difference is ignored)
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_distanceSquaredXY

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    double      xdist, ydist;

    xdist = pVector2->x - pVector1->x;
    ydist = pVector2->y - pVector1->y;

    return (xdist*xdist + ydist*ydist);
    }

/* VBSUB(Vec3dDistanceXY) */
/* CSVFUNC(distanceXY) */

/*-----------------------------------------------------------------*//**
* @description Computes the distance between two vectors, using
*   only x and y components.
*
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The distance between the XY projections of the two vectors.
*               (i.e. any z difference is ignored)
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_distanceXY

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    double      xdist, ydist;

    xdist = pVector2->x - pVector1->x;
    ydist = pVector2->y - pVector1->y;

    return sqrt (xdist*xdist + ydist*ydist);
    }

/* VBSUB(Vec3dMaxAbs) */


/*-----------------------------------------------------------------*//**
* @description Finds the largest absolute value among the components of a vector.
* @instance pVector => The vector
* @return The largest absolute value among vector coordinates.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDVec3d_maxAbs

(
DVec3dCP pVector
)
    {
    double maxVal = fabs (pVector->x);

    if (fabs (pVector->y) > maxVal)
        maxVal = fabs (pVector->y);

    if (fabs (pVector->z) > maxVal)
        maxVal = fabs (pVector->z);

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
Public GEOMDLLIMPEXP double bsiDVec3d_maxAbsDifference

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    DVec3d diff;
    diff.x = pVector1->x - pVector2->x;
    diff.y = pVector1->y - pVector2->y;
    diff.z = pVector1->z - pVector2->z;

    return bsiDVec3d_maxAbs (&diff);
    }

/* VBSUB(Vec3dEqual) */
/* CSVFUNC(isEqual) */

/*-----------------------------------------------------------------*//**
* @description Test for exact equality between all components of two vectors.
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return true if the vectors are identical.
* @bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec3d_equal

(
DVec3dCP pVector1,
DVec3dCP pVector2
)
    {
    bool                    result;

    if (pVector1 && pVector2)
        {
        result =
                   pVector1->x == pVector2->x
                && pVector1->y == pVector2->y
                && pVector1->z == pVector2->z;
        }
    else if (pVector1)
        {
        result =
                   pVector1->x == 0.0
                && pVector1->y == 0.0
                && pVector1->z == 0.0;
        }
    else if (pVector2)
        {
        result =
                   0.0 == pVector2->x
                && 0.0 == pVector2->y
                && 0.0 == pVector2->z;
        }
    else /* Both are null. Call them equal. */
        result = true;

    return  result;
    }

/* VBSUB(Vec3dEqualTolerance) */
/* CSVFUNC(isEqual) */

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
Public GEOMDLLIMPEXP bool    bsiDVec3d_equalTolerance
(
DVec3dCP pVector1,
DVec3dCP pVector2,
double                  tolerance
)
    {
    bool                    result;

    if (pVector1 && pVector2)
        {
        result = fabs(pVector1->x - pVector2->x) <= tolerance &&
                 fabs(pVector1->y - pVector2->y) <= tolerance &&
                 fabs(pVector1->z - pVector2->z) <= tolerance;
        }
    else if (pVector1)
        {
        result = fabs(pVector1->x) <= tolerance &&
                 fabs(pVector1->y) <= tolerance &&
                 fabs(pVector1->z) <= tolerance;
        }
    else if (pVector2)
        {
        result = fabs(pVector2->x) <= tolerance &&
                 fabs(pVector2->y) <= tolerance &&
                 fabs(pVector2->z) <= tolerance;
        }
    else /* Both are null. Call them equal. */
        result = true;

    return  result;
    }


/*-----------------------------------------------------------------*//**
@description Decompose a vector into two perpendicular parts.  One part is
   parallel to a reference vector, the other is perpendicular.
@instance pVector IN vector to decompose.
@param pParallelPart OUT part parallel to reference vector
@param pPerpendicularPart OUT part perpendicular to reference vector
@param pFraction IN size of the parallel part as a fraction of the reference vector.
@param pRefVector IN reference vector.
@bsihdr                               EarlinLutz      03/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec3d_perpendicularParts
(
DVec3dCP pVector,
DVec3dP  pParallelPart,
DVec3dP  pPerpendicularPart,
double*  pFraction,
DVec3dCP pRefVector
)
    {
    double UdotV = bsiDVec3d_dotProduct (pVector, pRefVector);
    double UdotU = bsiDVec3d_magnitudeSquared (pRefVector);
    double fraction;
    bool    boolstat = bsiTrig_safeDivide (&fraction, UdotV, UdotU, 0.0);
    DVec3d parallelPart;
    bsiDVec3d_scale (&parallelPart, pRefVector, fraction);
    if (pFraction)
        *pFraction = fraction;
    if (pParallelPart)
        *pParallelPart = parallelPart;
    if (pPerpendicularPart)
        bsiDVec3d_addScaled (pPerpendicularPart, pVector, pRefVector, -fraction);
    return  boolstat;
    }

/*-----------------------------------------------------------------*//**
@description Sweep the vector along a specified direction until it is perependicular to a plane normal.
@instance pVectorIN known vector
@param pResult OUT initialized vector
@param pSweepMultiplier OUT muliplier of sweep vector.
@param pSweepDirection IN direction to project.  If NULL, the plane normal is used.
@param pPlane IN the target plane.
@return false if projection direction is parallel to the plane.  The result
    is then a copy of the source.
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDVec3d_sweepToPlane
(
DVec3dCP    pVector,
DVec3dP     pResult,
double *    pSweepMultiplier,
DVec3dCP    pSweepDirection,
DPlane3dCP  pPlane
)
    {
    DVec3d sweepDirection = NULL != pSweepDirection ? *pSweepDirection : pPlane->normal;
    double dot0 = bsiDVec3d_dotProduct (pVector, &pPlane->normal);
    double dot1 = bsiDVec3d_dotProduct (&sweepDirection, &pPlane->normal);
    double f;
    bool    boolstat = bsiTrig_safeDivide (&f, - dot0, dot1, 0.0);
    if (pSweepMultiplier)
        *pSweepMultiplier = f;
    if (pResult)
        bsiDVec3d_addScaled (pResult, pVector, &sweepDirection, f);
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute a normalized vector that lies between the given vectors at a fraction of the (smaller) angle spanned by the vectors.
* @remarks In general, the vector returned by this function differs from the vector returned by ~mbsiDVec3d_interpolate.
* @remarks Input vectors do not have to be normalized.
* @remarks If the given parameter is <=0 (>=1), the interpolant is bit-copied from pVector0 (pVector1).
* @remarks If the vectors are not diametrically opposed, their cross product is the rotation axis; otherwise, the rotation axis is the cross
*       product of the first with the third vector.
*
* @param pSum                       OUT     the interpolated, normalized vector
* @param pVector0                   IN      vector corresponding to parameter 0.0
* @param angularFractionParameter   IN      the angular fraction in [0,1] at which to construct the interpolant
* @param pVector1                   IN      vector corresponding to parameter 1.0
* @param pDefaultPlaneVector        IN      vector in default rotation plane (used only if given vectors are antiparallel)
* @return false iff a given vector is zero.
* @bsimethod                                                    DavidAssaf      03/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDVec3d_interpolateNormals

(
DVec3dP  pSum,
DVec3dCP  pVector0,
double  angularFractionParameter,
DVec3dCP  pVector1,
DVec3dCP  pDefaultPlaneVector
)
    {
    // Direct construction for theta in (0,pi) and angularFractionParameter > 0:
    // if (bsiTrig_safeDivide (&cotAlphaTheta, 1.0, tan (angularFractionParameter * theta), 0.0) &&
    //    bsiTrig_safeDivide (&lambda, 1.0, 1.0 - cos (theta) + sin (theta) * cotAlphaTheta, 0.0))
    //    bsiDVec3d_interpolate (pSum, pVector0, lambda, pVector1);

    bsiDVec3d_zero (pSum);

    if (angularFractionParameter <= 0.0)
        *pSum = *pVector0;
    else if (angularFractionParameter >= 1.0)
        *pSum = *pVector1;
    else
        {
        double  theta = bsiDVec3d_angleBetweenVectors (pVector0, pVector1);

        if (theta <= 0.0)
            *pSum = *pVector0;
        else
            {
            RotMatrix   rotation;
            DVec3d      axis;

            if (theta < msGeomConst_pi)
                bsiDVec3d_crossProduct (&axis, pVector0, pVector1);
            else
                bsiDVec3d_crossProduct (&axis, pVector0, pDefaultPlaneVector);

            bsiRotMatrix_initFromVectorAndRotationAngle (&rotation, &axis, angularFractionParameter * theta);
            bsiDVec3d_multiplyRotMatrixDVec3d (pSum, &rotation, pVector0);
            }
        }

    return bsiDVec3d_normalizeInPlace (pSum) > 0.0;
    }


END_BENTLEY_NAMESPACE
