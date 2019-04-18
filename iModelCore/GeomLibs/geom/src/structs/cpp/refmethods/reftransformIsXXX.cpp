/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*-----------------------------------------------------------------*//**
* @description Analyze the invariant space of a transform.
* @param pTransform IN transform to analyze
* @param pInvariantSpace OUT transform in which leading columns are vector basis
*           corresponding to degrees of freedom in the invariant space.
* @param pInvariantVectorCount OUT number of free vectors.  If the invariant space
*           is a single point, the vector count is zero and the translation
*           in the basis is the point.
* @param pTranslationShift OUT vector change which must be added to the translation
*           part of the transform to make it have a invariant space.
* @param relTol IN relative tolerance for identifying zeros in scale matrix.
* @return true if there is an invariant space.   Note that a pure translation transform
*           has no invariant space, so false returns are common.
* @bsihdr                                       EarlinLutz      03/02
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiTransform_invariantSpaceBasis

(
TransformCP pTransform,
TransformP pInvariantSpace,
int       *pInvariantVectorCount,
RotMatrixP pVariantSpace,
int       *pVariantVectorCount,
DPoint3dP pTranslationShift,
double    relTol
)
    {
    RotMatrix rotation1, rotation2, matrixPart;
    DPoint3d scalePoint;
    DVec3d column;
    DVec3d invariantColumn[3];
    DVec3d variantColumn[3];
    DPoint3d translation;
    DVec3d errorVector;
    DVec3d errorDirection;
    int numDof, rank, i;
    bool    boolstat = false;
    DPoint3d refPoint;
    double *pScaleComponent;
    double *pTranslationComponent;
    double alpha;
    static double s_minRelTol = 1.0e-16;
    double a;

    if (relTol < s_minRelTol)
        relTol = s_minRelTol;

    pTransform->GetMatrix (matrixPart);

    for (i = 0; i < 3; i++)
        matrixPart.form3d[i][i] -= 1.0;

    pTransform->GetTranslation (translation);
    translation.Negate ();

    if (matrixPart.FactorRotateScaleRotate (rotation1, scalePoint, rotation2))
        {
        rotation1.MultiplyTranspose (translation);
        rank = 0;
        pScaleComponent = &scalePoint.x;
        pTranslationComponent = &translation.x;
        a = relTol * fabs (scalePoint.x);
        for (i = 0; i < 3 && fabs (pScaleComponent[i]) > a; i++)
            {
            rank++;
            }
        numDof = 3 - rank;
        boolstat = true;
        refPoint.Zero ();
        errorVector.Zero ();

        for (i = 0; i < 3; i++)
            {
            rotation2.GetRow (column, i);
            invariantColumn[i].Zero ();
            variantColumn[i].Zero ();
            if (i < rank)
                {
                alpha = pTranslationComponent[i] / pScaleComponent[i];
                refPoint.SumOf (refPoint, column, alpha);
                variantColumn[i] = column;
                }
            else
                {
                invariantColumn[i - rank] = column;
                rotation1.GetColumn (errorDirection, i);
                errorVector.SumOf (errorVector, errorDirection, pTranslationComponent[i]);
                }
            }
        if (pInvariantSpace)
            {
            pInvariantSpace->InitFromOriginAndVectors
                        (
                        refPoint,
                        invariantColumn[0],
                        invariantColumn[1],
                        invariantColumn[2]
                        );
            }

        if (pInvariantVectorCount)
            *pInvariantVectorCount = numDof;

        if (pVariantSpace)
            {
            pVariantSpace->InitFromColumnVectors
                        (
                        variantColumn[0],
                        variantColumn[1],
                        variantColumn[2]
                        );
            }

        if (pVariantVectorCount)
            *pVariantVectorCount = rank;

        if (pTranslationShift)
            *pTranslationShift = errorVector;
        }

    return boolstat;
    }

END_BENTLEY_GEOMETRY_NAMESPACE

BEGIN_BENTLEY_NAMESPACE


/*-----------------------------------------------------------------*//**
* @description Returns true if the transform is the identity transform.
* @return true if the transformation is within tolerance of the identity.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool Transform::IsIdentity
(

) const
    {
    DPoint3d  origin;
    RotMatrix M;

    GetMatrix (M);

    if (!M.IsIdentity())
        return false;

#define IS_ZERO(a) ((a < 1.0E-10) && (a > - 1.0E-10))
    GetTranslation (origin);
    if (IS_ZERO(origin.x) &&
        IS_ZERO(origin.y) &&
        IS_ZERO(origin.z))
        return true;

    return false;
    }



/*-----------------------------------------------------------------*//**
* @description Returns true if the matrix part of a transform is a rigid body rotation,
* i.e. its transpose is its inverse and it has a positive determinant.
*
* @return true if the transformation is rigid (no scale or shear in the matrix part)
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool Transform::IsRigid () const
    {
    RotMatrix M;
    GetMatrix (M);
    return M.IsRigid ();
    }
/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      04/13
+----------------------------------------------------------------------*/
bool Transform::IsRigidScale (double &scale) const
    {
    RotMatrix M;
    GetMatrix (M);
    RotMatrix M1;
    return M.IsRigidScale (M1, scale);
    }


/*-----------------------------------------------------------------*//**
* @description Returns true if transformation effects are entirely within the plane
*       with given normal.
*
* @param [in] normal The plane normal
* @return true if the transform has no effects perpendicular to planes with the given normal.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool Transform::IsPlanar
(

DVec3dCR normal

) const
    {
    double norm2, tran2, dot;
    DPoint3d origin;
    RotMatrix M;
    static double tolSquared = 1.0E-20;
    GetTranslation (origin);

    dot = normal.DotProduct (origin);

    tran2 = origin.MagnitudeSquared ();
    norm2 = normal.MagnitudeSquared();
    if (tran2 > tolSquared * norm2 && dot * dot > tolSquared * norm2 * tran2)
        return false;
    GetMatrix (M);
    return M.IsPlanar (normal);
    }


/*-----------------------------------------------------------------*//**
* @description Returns true if two transforms have exact (bitwise) equality.
*
* @param [in] transform2 The second transform
* @return   true if the transforms are identical
* @bsimethod                                                    EarlinLutz      05/00
+----------------------------------------------------------------------*/
bool Transform::IsEqual (TransformCR transform2) const
    {
    const double *p0 = (const double *)this;
    const double *p1 = (const double *)&transform2;
    int i;

    for (i = 0; i < 12; i++)
        {
        if (p0[i] != p1[i])
            return false;
        }
    return true;
    }


/*-----------------------------------------------------------------*//**
* @description Returns true if two transformations are equal within tolerance, using
*       separate tolerances for the matrix and point parts.
* @indexVerb
* @bsimethod                                                    EarlinLutz      05/00
+----------------------------------------------------------------------*/
bool Transform::IsEqual
(

TransformCR transform1,
double                  matrixTolerance,
double                  pointTolerance

) const
    {
    RotMatrix matrix0, matrix1;
    DPoint3d  point0,  point1;
    GetMatrix (matrix0);
    transform1.GetMatrix (matrix1);

    if (!matrix0.IsEqual (matrix1, matrixTolerance))
        return false;
    GetTranslation (point0);
    transform1.GetTranslation (point1);
    return point0.IsEqual (point1, pointTolerance);
    }



/*-----------------------------------------------------------------*//**
* @description Returns true if the transform is a simple translation.
* @param [in] pTransform The transformation to test
* @param [out] translation the translation vector. Zero of not a translation transform.
* @return true if the transformation is a pure translation.
* @bsimethod                            EarlinLutz      03/02
+----------------------------------------------------------------------*/
bool Transform::IsTranslate
(

DPoint3dR translation

) const
    {
    RotMatrix matrix;
    GetMatrix (matrix);

    if (matrix.IsIdentity ())
        {
        GetTranslation (translation);
        return true;
        }
    else
        {
        translation.Zero ();
        return false;
        }
    }

//! Return true if the transform a combination of only 2 thing: (1) move origin, (2) rotate around Z
//! @param [out] origin origin of frame.
//! @param [out] radians positive rotation around Z
bool Transform::IsTranslateScaleRotateAroundZ
(
	DPoint3dR origin,
	RotMatrixR rigidAxes,
	double &scale,
	double &radians
) const
    {

	RotMatrix matrix;
	GetMatrix(matrix);
	GetTranslation(origin);
	scale = 1.0;
	rigidAxes.InitIdentity();
	radians = 0.0;
	if (matrix.IsRigidScale(rigidAxes, scale))
	    {
		DVec3d xVec, yVec, zVec;
		rigidAxes.GetColumns(xVec, yVec, zVec);
		DVec3d globalZ = DVec3d::From(0, 0, 1);
		if (globalZ.AlmostEqual(zVec))
		    {
			radians = atan2(xVec.y, xVec.x);
			return true;
	    	}
	    }
	return false;
    }

/*-----------------------------------------------------------------*//**
* @description Returns true if the transform is a uniform scale with scale factor other than 1.0.
* @param [in] pTransform The transformation to test
* @param [out] fixedPoint (If function result is true) the (one) point which
*                           remains in place in the transformation.
* @param [out] scale The scale factor.  If the transform is not a scale, this is returned as 1.0.
* @return true if the transformation is a uniform scale.
* @bsimethod                            EarlinLutz      03/02
+----------------------------------------------------------------------*/
bool Transform::IsUniformScale
(

DPoint3dR fixedPoint,
        double          &scale

) const
    {
    DPoint3d  translation;
    RotMatrix matrix;
    double a;
    bool    isUniformScale = false;

    GetMatrix (matrix);
    fixedPoint.Zero ();
    scale = 1.0;

    if (matrix.IsUniformScale (scale))
        {
        GetTranslation (translation);
        a = 1.0 - scale;
        isUniformScale = fixedPoint.SafeDivide (translation, a);
        }
    else
        {
        scale = 1.0;
        }

    return isUniformScale;
    }


/*-----------------------------------------------------------------*//**
* @description Returns true if the transform is a non-zero rotation around a line.
* @param [in] pTransform The transformation to test
* @param [out] pLinePoint a point on the line.
* @param [out] directionVector vector in the line direction.
* @param [out] radians rotation angle in radians.
* @return true if the transformation is a non-zero rotation.
* @bsimethod                            EarlinLutz      03/02
+----------------------------------------------------------------------*/
bool Transform::IsRotateAroundLine
(
DPoint3dR fixedPoint,
DVec3dR directionVector,
double  &radians
) const
    {
    Transform basis;
    RotMatrix matrix;
    int nullSpaceDimension;
    bool    result = false;
    DVec3d errorVector;

    GetMatrix (matrix);
    directionVector.Zero ();
    fixedPoint.Zero ();
    radians = 0.0;

    if (    matrix.IsRigid ()
        && !matrix.IsIdentity ()
        && bsiTransform_invariantSpaceBasis (this, &basis, &nullSpaceDimension,
                    NULL, NULL, &errorVector, 0.0)
        )
        {
        basis.GetTranslation (fixedPoint);
        radians = matrix.GetRotationAngleAndVector (directionVector);
        result = true;
        }

    return result;
    }


/*-----------------------------------------------------------------*//**
* @description Returns true if the transform is a mirror with respect to
*       a plane.
* @param [in] pTransform The transformation to test
* @param [out] planePoint Some point on the plane.
* @param [out] pPlaneNormal unit vector perpendicular to the plane.
* @return true if the transformation is a mirror.
* @bsimethod                            EarlinLutz      03/02
+----------------------------------------------------------------------*/
bool Transform::IsMirrorAboutPlane (DPoint3dR planePoint, DVec3dR unitNormal) const
    {
    bool    result = false;
    RotMatrix matrix;
    DPoint3d translation;
    DVec3d planeNormal;
    Transform invariantSpace;
    RotMatrix variantSpace;
    int invariantDimension, variantDimension;
    DPoint3d translationShift;
    DVec3d mirrorVector, mirrorImage;

    static double s_scaleTol = 1.0e-15;
    planePoint.Zero ();
    planeNormal.Zero ();

    GetMatrix (matrix);
    GetTranslation (translation);

    if (bsiTransform_invariantSpaceBasis (this, &invariantSpace, &invariantDimension,
                                            &variantSpace, &variantDimension, &translationShift, s_scaleTol)
        && variantDimension == 1    // the variant dimension is the plane normal
        && invariantDimension == 2  // the invariant dimensions are the plane tangents
        )
        {
        double a;
        variantSpace.GetColumn (mirrorVector, 0);
        MultiplyMatrixOnly (mirrorImage, mirrorVector);
        a = mirrorImage.DotProduct (mirrorVector);
        if (fabs (a + 1.0) <= s_scaleTol)
            {
            result = true;
            invariantSpace.GetTranslation (planePoint);
            unitNormal = mirrorVector;
            }
        }

    return result;
    }


/*-----------------------------------------------------------------*//**
* @description Returns true if the transform is a uniform scale combined with
*       a rotation.  One, but not both, of the two steps may be null (unit scale or no rotation)
*
* @param [in] pTransform The transformation to test
* @param [out] fixedPoint fixed point of scaling.  This is also a point on the
*               line.
* @param [out] directionVector vector in the direction of the rotation.
* @param [out] radians rotation angle
* @param [out] scale scale factor.
* @return true if the transformation has at least one of the scale, rotate effects.
* @bsimethod                            EarlinLutz      03/02
+----------------------------------------------------------------------*/
bool Transform::IsUniformScaleAndRotateAroundLine
(

DPoint3dR fixedPoint,
DVec3dR directionVector,
        double          &radians,
        double          &scale

) const
    {
    Transform basis;
    RotMatrix matrix, rotationPart;
    int nullSpaceDimension;
    bool    result = false;
    DPoint3d errorVector;

    GetMatrix (matrix);
    directionVector.Init (1,0,0);
    fixedPoint.Zero ();
    radians = 0.0;

    if (    matrix.IsRigidScale (rotationPart, scale)
        && !matrix.IsIdentity ()
        && bsiTransform_invariantSpaceBasis (this, &basis, &nullSpaceDimension,
                    NULL, NULL, &errorVector, 0.0)
        )
        {
        basis.GetTranslation (fixedPoint);
        radians = rotationPart.GetRotationAngleAndVector (directionVector);
        result = true;
        }

    return result;
    }



/*-----------------------------------------------------------------*//**
* @description Compute any single point that remains unchanged by action of
*       a transform.   Note that a pure translation has no fixed points,
*       while any other transformation does.
* @param [in] pTransform The transformation to test
* @param [out] fixedPoint Point that is not changed by the transformation.
* @return true if the transformation has a fixed point.
* @bsimethod                            EarlinLutz      03/02
+----------------------------------------------------------------------*/
bool Transform::GetAnyFixedPoint
(
DPoint3dR fixedPoint
) const
    {
    Transform fixedSpace;
    int nullSpaceDimension;
    bool    boolstat = bsiTransform_invariantSpaceBasis
                    (
                    this,
                    &fixedSpace, &nullSpaceDimension,
                    NULL, NULL,
                    NULL,
                    0.0
                    );
    if (!boolstat)
        {
        fixedPoint.Zero ();
        }
    else
        {
        fixedSpace.GetTranslation (fixedPoint);
        }
    return boolstat;
    }


/*-----------------------------------------------------------------*//**
* @description Compute the line (if any) of points that are not affected by
*       this transformation.  Returns false if the fixed point set for the
*       transform is empty, a single point, a plane, or all points.
* @param [in] pTransform The transformation to test
* @param [out] fixedPoint A point on the line.
* @param [out] directionVector vector along the line.
* @return true if the transformation has a fixed point.
* @bsimethod                            EarlinLutz      03/02
+----------------------------------------------------------------------*/
bool Transform::GetFixedLine
(
DPoint3dR fixedPoint,
DVec3dR   directionVector
) const
    {
    Transform fixedSpace;
    int nullSpaceDimension;
    bool    boolstat = bsiTransform_invariantSpaceBasis
                    (
                    this,
                    &fixedSpace, &nullSpaceDimension,
                    NULL, NULL,
                    NULL, 0.0
                    );
    if (boolstat && nullSpaceDimension == 1)
        {
        fixedSpace.GetTranslation (fixedPoint);
        fixedSpace.GetMatrixColumn (directionVector, 0);
        }
    else
        {
        fixedPoint.Zero ();
        directionVector.Init (1,0,0);
        }
    return boolstat;
    }


/*-----------------------------------------------------------------*//**
* @description Compute the plane (if any) of points that are not affected by
*       this transformation.  Returns false if the fixed point set for the
*       transform is empty, a single point, a line, or all points.
* @param [in] pTransform The transformation to test
* @param [out] fixedPoint A point on the line.
* @param [out] pDirectionVectorX a unit vector in the plane.
* @param [out] pDirectionVectorY another unit vector in the plane,
*               perpendicular to pDirectionVectorX.
* @return true if the transformation has a fixed point.
* @bsimethod                            EarlinLutz      03/02
+----------------------------------------------------------------------*/
bool Transform::GetFixedPlane
(

DPoint3dR fixedPoint,
DVec3dR planeVectorX,
DVec3dR planeVectorY

) const
    {
    Transform fixedSpace;
    int nullSpaceDimension;
    bool    boolstat = bsiTransform_invariantSpaceBasis
                    (
                    this,
                    &fixedSpace, &nullSpaceDimension,
                    NULL, NULL,
                    NULL, 0.0
                    );

    if (boolstat && nullSpaceDimension == 1)
        {
        fixedSpace.GetTranslation (fixedPoint);
        fixedSpace.GetMatrixColumn (planeVectorX, 0);
        fixedSpace.GetMatrixColumn (planeVectorY, 1);
        }
    else
        {
        fixedPoint.Zero ();
        planeVectorX.Init (1,0,0);
        planeVectorY.Init (0,1,0);
        boolstat = false;
        }

    return boolstat;
    }


/*-----------------------------------------------------------------*//**
* @description Factor a combination of a mirror part and a non-mirror part,
*           attempting to use a common fixed point for the two parts.
* Equationally, the old transform T0 becomes
<pre>
            T0 = T1 * M
</pre>
* where M is the mirror, and T1 is a mirror-free transform.
* The mirror transform is returned in both matrix form and as plane point with normal.
* In an order of operations view, a point X is transformed as
<pre>
            T0 * X = T1 * M * X
</pre>
* That is, X mirrored first, followed by rotation and scaling in the residual transform.
*
* @param [in] pTransform The transformation to test
* @param [out] residualTransform the residual transform.
* @param [out] mirrorTransform the mirror transform.
* @param [out] pFixedPoint A fixed point of the mirror transform.
* @param [out] planeNormal Unit normal for the mirror plane.
* @return false if the transform has no mirror effects.  In this case the mirror transform is the identity,
*       the residual transform is the original transform, and the fixed point and normal are both zero.
* @bsimethod                            EarlinLutz      03/02
+----------------------------------------------------------------------*/
bool Transform::GetFixedPlane
(
TransformR residualTransform,
TransformR mirrorTransform,
DPoint3dR planePoint,
DVec3dR planeNormal
) const
    {
    bool    boolstat = false;
    RotMatrix matrix;
    double det;
    GetMatrix (matrix);
    det = Determinant ();

    if (det >= 0.0)
        {
        mirrorTransform.InitIdentity ();
        residualTransform = *this;
        planePoint.Zero ();
        planeNormal.Zero ();
        boolstat = false;
        }
    else if (IsMirrorAboutPlane (planePoint, planeNormal))
        {
        // simple mirror case....
        residualTransform.InitIdentity ();
        mirrorTransform = *this;
        boolstat = true;
        }
    else if (GetAnyFixedPoint (planePoint))
        {
        // mirror and something else.
        planeNormal.Init (0,0,1);
        mirrorTransform.InitFromMirrorPlane (planePoint, planeNormal);
        // Mirror transform is its own inverse, so
        // T = T * M * M, and (T*M) has positive determinant and is the residual.
        residualTransform.InitProduct (*this, mirrorTransform);
        boolstat = true;
        }
    else
        {
        // No fixed point for the whole transform. (Is this even possible?)  Just mirror around origin.
        planePoint.Zero ();
        planeNormal.Init (0,0,1);
        mirrorTransform.InitFromMirrorPlane (planePoint, planeNormal);
        // Mirror transform is its own inverse, so
        // T = T * M * M, and (T*M) has positive determinant and is the residual.
        residualTransform.InitProduct (*this, mirrorTransform);
        boolstat = true;
        }

    return boolstat;
    }


END_BENTLEY_NAMESPACE
