/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Initialize as sum of (possibly zero) prior matrix and explicit
//! coefficients of a quadratic form.
//!
//! @param <= quadric coefficients
//! @param pAB0 IN      prior quadric coefficients.  If NULL, assumed 0
//! @param cxx IN      xx coefficient
//! @param cxy IN      xy coefficient
//! @param cyy IN      yy coefficient
//! @param cx IN      x coefficient
//! @param cy IN      y coefficient
//! @param c1 IN      1 coefficient
//!
Public GEOMDLLIMPEXP void bsiQCoff_loadCoefficients
(
DMatrix3dP pInstance,
DMatrix3dP pAB0,
double      cxx,
double      cxy,
double      cyy,
double      cx,
double      cy,
double      c1
);

//!
//! Compute B so X'BX = X'AX and B is symmetric.
//!
//! @param pA OUT     symmetric coefficients
//! @param pB IN      nonsymmetric coefficients
//!
Public GEOMDLLIMPEXP void bsiQCoff_symmetrize
(
DMatrix3dP pA,
DMatrix3dCP pB
);

//!
//! Compute a matrix A such that
//!   A*(c s 1)' = H * B where
//!  H is the matrix
//! [ 0 -1 -s][bx]        [c]
//! [ 1  0  c][by] == A * [s]
//! [ s -c  0][bz]        [1]
//!
//! @param pA OUT     coefficient matrix
//! @param pVecB IN      vector
//!
Public GEOMDLLIMPEXP void bsiQCoff_HOperator
(
DMatrix3dP pA,
DPoint3dCP pVecB
);

//!
//! Compute the matrix of a quadric section whose intersections with
//! the unit circle are the cosine and sine of the angles where pPoint
//! projects to the quadric.
//! That is,
//!   A = sym(D*B'* (I - QW') * B)
//! Where sym is the symmetrizing operator and  B, D, Q, and W are things
//! that need some explanation.
//!
//! @param pA OUT     matrix of new quadric section
//! @param pB IN      matrix of existing quadric section
//! @param pPoint IN      point being projected to matrix B.
//! @param pPoint
//!
Public GEOMDLLIMPEXP void bsiQCoff_projectToEllipse
(
RotMatrixP pA,
RotMatrixCP pB,
DPoint3dCP pPoint
);


//!
//! Find the angles at which an unbounded ellipse comes close to a point, measuring in
//! projected space.
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_closeApproachUnbounded2d
(
DPoint3dP pTrigPoint,
int                 *pNumApproach,
DEllipse4dCP pHEllipse,
int                 xIndex,
int                 yIndex,
int                 wIndex,
DPoint4dP pTestPoint
);

//!
//! @param pIntersections         OUT     up to 4 intersection points
//! @param pNumIntersections      OUT     number of intersections
//! @param pA                     IN      first conic section [center,vector0,vector90][x,y,w]
//! @param pB                     IN      second conic section
//! @return true if intersections computed normally.
//!
Public GEOMDLLIMPEXP bool    bsiQCoff_intersectQuadricSections
(
DPoint2dP pIntersections,
int         *pNumIntersections,
RotMatrixCR matrixA,
RotMatrixCR matrixB
);

//!
//! @param pLineInt * @param pNumLineInt * @param pLineCoff * @param pQCoff * @see
//! @return SUCCESS if
//!
Public GEOMDLLIMPEXP StatusInt bsiQCoff_intersectLine
(
DPoint2dP pLineInt,
int                 *pNumLineInt,
DPoint3dCP pLineCoff,
RotMatrixCP pQCoff
);


/*----------------------------------------------------------------------+
|                                                                       |
| name          jacobi3X3                                               |
|                                                                       |
        Diagonalize a 3X3 symmetric matrix by Jacobi's method.
        Return eigenvalues and normalized eigenvectors (column vectors).

        The method is iterative, but it achieves machine precision --
        it's NOT approximate.

    Pre-requisites & errors:
        This algorithm only applies to symmetric matrices -- it will
        signal an assertion failure if input is not symmetric.

        (Given that the matrix is symmetric, we can be sure that there
        are 3 mutually orthogonal eigenvectors and, therefore, that all
        the eigenvalues are distinct. This algorithm is guaranteed to
        find them ... unless the numbers involved are completely
        degenerate from the start. No explicit error returned in that
        case.)

    Notes:
        Taken directly from Numerical Recipes in C (adapted to index from 0).
        The method is iterative, but NRCEE claims it usually finishes in
        5 or 6 iterations.
        ROTATE macro is TRICKY!  It deliberatlely changes the values of g and h
                                as a side effect each time it is used.

|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiGeom_jacobi3X3
(
double  d[],
double  v[3][3],
int     *nRot,
double  a[3][3]
);

Public GEOMDLLIMPEXP void bsiRotMatrix_symmetricEigensystem
(
RotMatrixP pEigenvectors,
DPoint3dP pEigenvalues,
RotMatrixCP pInstance
);
//!
//! @param selector = coordinate system identifier
//!           1   top
//!           2   bottom
//!           3   left
//!           4   right
//!           5   front
//!           6   back
//!           7   left iso
//!           8   right iso
//! WARNING: The matrix may be tranposed from what you expect.
//! @return true if index is valid
//!

//!
//! @param selector = coordinate system identifier
//!           1   top
//!           2   bottom
//!           3   left
//!           4   right
//!           5   front
//!           6   back
//!           7   left iso
//!           8   right iso
//! @return true if index is valid
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_getStandardRotation
(
RotMatrixP pInstance,
int          selector
);



//!
//! @description Factor matrix A as a product A = B * VT
//!   where VT is orthogonal and B has orthonormal columns.
//!   If A is singular, B will have a zero column.  If all columns
//!   of B are nonzero, the inverse of A can be constructed accurately.
//! @param pMatrixA IN      the matrix to factor
//! @param pB OUT     the matrix with perpendicular but not normalized columns.
//! @param pV OUT     the orthogonal matrix.
//!
//! @return true factorization completed.  Numerical authorities claim
//!       the iteration will always succeed.
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_orthogonalizeColumns
(
RotMatrixCP pMatrixA,
RotMatrixP pB,
RotMatrixP pV
);


//!
//! @description Invert a matrix by constructing orthogonal factorization.
//!   This is more expensive than Cramer's rule or elimination, but provides
//!   a more accurate estimate of condition.   This function considers
//!   any nonzero condition to be nonsingular; caller may wish to
//!   apply a stricter test to the condition.
//! @param pMatrixA IN      matrix to invert.
//! @param pAInv OUT     the computed inverse.
//! @param pCondition IN      the smallest singular value divided by the largest.
//! @return true if the matrix is invertible.
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_invertRotMatrixByOrthogonalFactors
(
RotMatrixCP pMatrixA,
RotMatrixP pMatrixAInv,
double      *pCondition
);

//!
//! @description return a nearby matrix with only rotation and scaling.
//! @param pResult <= corrected matrix
//! @param pMatrix => input matrix.
//! @return false if SVD fails (impossible?)
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_cleanupNearPerpendicularColumns
(
RotMatrixP pResult,
RotMatrixCP pMatrix
);

//!
//! @description Factor a matrix (linear transformation) as a product of
//! two rotations and a (sandwiched) scale, i.e.
//! <pre>
//!                  matrix = Rotation1 * Scale * Rotation2
//! </pre>
//!
//! <p> where Rotation1 and Rotation2 are pure rotation about the origin
//!   and Scale is a principle axis scale.
//! </p>
//!
//! <p>
//! Once again:
//! <ul>
//! <li>R1 is a pure rotation matrix. It is orthogonal. Each column has magnitude 1.
//!                    Its determinant is 1. It defines a right handed coordinate system.
//! <li>Scale conatains x, y, and z scale factors.  If converted to a matrix, the scale factors
//!                appear on the diagonal and the off-diagonals are zero.
//!                The largest absolute value scale factor is the x scale, and the smallest
//!                absolute value scale factor is the z scale.  The z scale (but no others)
//!                may be negative, which means that the transformation performs reflection.
//! <li>R2 is a pure rotation matrix. It is orthogonal. Each column has magnitude 1.
//!                    Its determinant is 1.   It defines a right handed coordinate system.
//! </p>
//!
//! <p> If the composite matrix is applied to a vector, the equational form is</p>
//! <pre>
//!                   matrix * vector = R1 * Scale * R2 * vector
//! </pre>
//! <p>
//! If you read this from right to left, the interpretation is that the vector is rotated (by R1)
//! around the global origin, scaled around the global origin, and rotated again (by R1).
//! </p>
//!
//! <p>The rotations and scales are internally arranged so that the largest scale factors
//! are first in the scale point.</p>
//!
//! <p>If you have perservered this far, you may recognize that the product Rotation1*Scale*Rotation2
//!    is a singular value decomposition.  (With small caveat that in some definitions of the
//!    singular values, all the scales are non-negative and the reflection effects make one
//!    of the rotations into a reflection.)
//! </p>
//!
//! @param pMatrix IN matrix being analyzed.
//! @param pRotation1 OUT One of the rotation parts, as a RotMatrix.
//! @param pScalePoint OUT scale factors, as xyz parts of a DPoint3d.
//! @param pRotation2 OUT The other rotation part, as a RotMatrix.
//!
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_factorRotateScaleRotate
(
RotMatrixCP pMatrix,
RotMatrixP pRotation1,
DPoint3dP pScalePoint,
RotMatrixP pRotation2
);


//!
//! @description Factor an affine transformation as a product of
//!
//! <pre>
//!                  transform = Translation*R1*Scale*R2
//! </pre>
//!
//! <p> where Translation is pure translation, R1 and R2 are pure rotation about the origin
//!   and Scale is a principle axis scale.
//! </p>
//!
//! <p>
//! Once again:
//! <ul>
//! <li>Translation is a translation returned as
//! <li>R1 is a pure rotation matrix. It is orthogonal. Each column has magnitude 1.
//!                    Its determinant is 1. It defines a right handed coordinate system.
//! <li>Scale conatains x, y, and z scale factors.  If converted to a matrix, the scale factors
//!                appear on the diagonal and the off-diagonals are zero.
//!                The largest absolute value scale factor is the x scale, and the smallest
//!                absolute value scale factor is the z scale.  The z scale (but no others)
//!                may be negative, which means that the transformation performs reflection.
//! <li>R2 is a pure rotation matrix. It is orthogonal. Each column has magnitude 1.
//!                    Its determinant is 1.   It defines a right handed coordinate system.
//! </p>
//!
//! <p> If the composite transform is applied to a point, the equational form is</p>
//! <pre>
//!                   transform * point = Translation * R1 * Scale * R2 * Point
//! </pre>
//! <p>
//! If you read this from right to left, the interpretation is that the point is rotated (by R1)
//! around the global origin, scaled around the global origin, rotated again (by R1),
//! and translated.
//! </p>
//!
//! <p>The rotations and scales are internally arranged so that the largest scale factors
//! are first in the scale point.</p>
//!
//! <p>If you have perservered this far, you may recognize that the product Rotation1*Scale*Rotation2
//!    is a singular value decomposition.  (With small caveat that in some definitions of the
//!    singular values, all the scales are non-negative and the reflection effects make one
//!    of the rotations into a reflection.)
//! </p>
//!
//! @param pTransform IN transform being analyzed.
//! @param pTranslation OUT translation part, as a bare DPoint3d.
//!       Note that this point is not a FIXED point of the transformation, it is
//!       the translation term directly out of the transformation.  A fixed point (if any)
//!       of the transformation may be inquired via ~mbsiTransform_getAnyFixedPoint.
//! @param pRotation1 OUT One of the rotation parts, as a RotMatrix.
//! @param pScalePoint OUT scale factors, as xyz parts of a DPoint3d.
//! @param pRotation2 OUT The other rotation part, as a RotMatrix.
//!
//!
Public GEOMDLLIMPEXP bool    bsiTransform_factorTranslateRotateScaleRotate
(
TransformCP pTransform,
DPoint3dP pTranslation,
RotMatrixP pRotation1,
DPoint3dP pScalePoint,
RotMatrixP pRotation2
);

Public GEOMDLLIMPEXP bool    bsiRotMatrix_isRotationFromStandardView
(
RotMatrixCP pMatrix,
int *pRotationAxis,
double *pRadians,
int *pViewIndex,
bool    bTestX,
bool    bTestY,
bool    bTestZ
);

Public GEOMDLLIMPEXP bool    bsiRotMatrix_initRotationFromStandardView
(
RotMatrixP pMatrix,
int rotationAxis,
double radians,
int viewIndex
);

//!
//! @description Analyze the invariant space of a transform.
//! @param pTransform IN transform to analyze
//! @param pInvariantSpace OUT transform in which leading columns are vector basis
//!           corresponding to degrees of freedom in the invariant space.
//! @param pInvariantVectorCount OUT number of free vectors.  If the invariant space
//!           is a single point, the vector count is zero and the translation
//!           in the basis is the point.
//! @param pTranslationShift OUT vector change which must be added to the translation
//!           part of the transform to make it have a invariant space.
//! @param relTol IN relative tolerance for identifying zeros in scale matrix.
//! @return true if there is an invariant space.   Note that a pure translation transform
//!           has no invariant space, so false returns are common.
//!
Public GEOMDLLIMPEXP bool     bsiTransform_invariantSpaceBasis
(
TransformCP pTransform,
TransformP pInvariantSpace,
int       *pInvariantVectorCount,
RotMatrixP pVariantSpace,
int       *pVariantVectorCount,
DPoint3dP pTranslationShift,
double    relTol
);


END_BENTLEY_GEOMETRY_NAMESPACE

