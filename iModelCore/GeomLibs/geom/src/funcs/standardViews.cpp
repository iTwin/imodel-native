/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/standardViews.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



// Yeah.  This is flakey.
static RotMatrix s_standardViewMatrix[8] = {
    {{   // Standard view matrix 1 (zero based 0)
         {1.000000000000000,  0.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  1.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  0.00000000000000000,  1.00000000000000000}
    }},
    {{   // Standard view matrix 2 (zero based 1)
         {1.000000000000000,  0.00000000000000000,  0.00000000000000000},
         {0.000000000000000, -1.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  0.00000000000000000, -1.00000000000000000}
    }},
    {{   // Standard view matrix 3 (zero based 2)
         {0.000000000000000, -1.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  0.00000000000000000,  1.00000000000000000},
         {-1.000000000000000,  0.00000000000000000,  0.00000000000000000}
    }},
    {{   // Standard view matrix 4 (zero based 3)
         {0.000000000000000,  1.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  0.00000000000000000,  1.00000000000000000},
         {1.000000000000000,  0.00000000000000000,  0.00000000000000000}
    }},
    {{   // Standard view matrix 5 (zero based 4)
         {1.000000000000000,  0.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  0.00000000000000000,  1.00000000000000000},
         {0.000000000000000, -1.00000000000000000,  0.00000000000000000}
    }},
    {{   // Standard view matrix 6 (zero based 5)
         {-1.000000000000000,  0.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  0.00000000000000000,  1.00000000000000000},
         {0.000000000000000,  1.00000000000000000,  0.00000000000000000}
    }},
    {{   // Standard view matrix 7 (zero based 6)
         {0.707106781186548, -0.70710678118654757,  0.00000000000000000},
         {0.408248290463863,  0.40824829046386302,  0.81649658092772603},
         {-0.577350269189626, -0.57735026918962573,  0.57735026918962573}
    }},
    {{   // Standard view matrix 8 (zero based 7)
         {0.707106781186548,  0.70710678118654757,  0.00000000000000000},
         {-0.408248290463863,  0.40824829046386302,  0.81649658092772603},
         {0.577350269189626, -0.57735026918962573,  0.57735026918962573}
    }}
    };


Public GEOMDLLIMPEXP bool    bsiRotMatrix_getStandardRotation
(
RotMatrixP pInstance,
int          selector
)
    {

    int index = selector - 1;
    if (index < 0 || index >= 8)
        {
        bsiRotMatrix_initIdentity (pInstance);
        return  false;
        }
    *pInstance = s_standardViewMatrix [index];
    return  true;
    }

/*-----------------------------------------------------------------*//**
* @description return a nearby matrix with only rotation and scaling.
* @param pResult <= corrected matrix
* @param pMatrix => input matrix.
* @return false if SVD fails (impossible?)
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_cleanupNearPerpendicularColumns
(
RotMatrixP pResult,
RotMatrixCP pMatrix
)
    {
    RotMatrix Q0, Q1;
    DPoint3d scalePoint;
    double det = pMatrix->Determinant ();
    RotMatrix workMatrix;
    *pResult = *pMatrix;


    double scale = pow (fabs (det), 1.0/3.0);
    if (det < 0.0)
        scale = -scale;

    if (scale == 0.0)
        return false;

    double inverseScale = 1.0 / scale;
    workMatrix.ScaleColumns (*pMatrix, inverseScale, inverseScale, inverseScale);
    // This has positive determinant !!

    if (!bsiRotMatrix_factorRotateScaleRotate (&workMatrix, &Q0, &scalePoint, &Q1))
        return false;

    bsiRotMatrix_multiplyRotMatrixRotMatrix (pResult, &Q0, &Q1);
    double abc = scalePoint.x * scalePoint.y * scalePoint.z;


    double e = pow (abc, 1.0 / 3.0);    // should be 1.0???
    pResult->ScaleColumns (*pResult, e, e, e);

    pResult->ScaleColumns (*pResult, scale, scale, scale);
    return true;
    }



/*-----------------------------------------------------------------*//**
* @description Factor a matrix (linear transformation) as a product of
two rotations and a (sandwiched) scale, i.e.
<pre>
                  matrix = Rotation1 * Scale * Rotation2
</pre>

<p> where Rotation1 and Rotation2 are pure rotation about the origin
   and Scale is a principle axis scale.
</p>

<p>
Once again:
<ul>
<li>R1 is a pure rotation matrix. It is orthogonal. Each column has magnitude 1.
                    Its determinant is 1. It defines a right handed coordinate system.</li>
<li>Scale conatains x, y, and z scale factors.  If converted to a matrix, the scale factors
                appear on the diagonal and the off-diagonals are zero.
                The largest absolute value scale factor is the x scale, and the smallest
                absolute value scale factor is the z scale.  The z scale (but no others)
                may be negative, which means that the transformation performs reflection.
                </li>
<li>R2 is a pure rotation matrix. It is orthogonal. Each column has magnitude 1.
                    Its determinant is 1.   It defines a right handed coordinate system.</li>
</p>

<p> If the composite matrix is applied to a vector, the equational form is</p>
<pre>
                   matrix * vector = R1 * Scale * R2 * vector
</pre>
<p>
If you read this from right to left, the interpretation is that the vector is rotated (by R2)
around the global origin, scaled around the global origin, and rotated again (by R1).
</p>

<p>The rotations and scales are internally arranged so that the largest scale factors
are first in the scale point.</p>

<p>If you have perservered this far, you may recognize that the product Rotation1*Scale*Rotation2
    is a singular value decomposition.  (With small caveat that in some definitions of the
    singular values, all the scales are non-negative and the reflection effects make one
    of the rotations into a reflection.)
</p>

* @param sourceMatrix IN matrix being analyzed.
* @param rotation1 OUT One of the rotation parts, as a RotMatrix.
* @param scalePoint OUT scale factors, as xyz parts of a DPoint3d.
* @param rotation2 OUT The other rotation part, as a RotMatrix.

* @bsihdr                                       EarlinLutz      03/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_factorRotateScaleRotate
(
RotMatrixCP pSourceMatrix,
RotMatrixP pRotation1,
DPoint3dP  pScalePoint,
RotMatrixP pRotation2
)
    {
    RotMatrix rotation1, rotation2;
    DPoint3d scalePoint;
    int rank = pSourceMatrix->FactorRotateScaleRotate (rotation1, scalePoint, rotation2);
    if (pRotation1)
        *pRotation1 = rotation1;
    if (pRotation2)
        *pRotation2 = rotation2;
    if (pScalePoint)
        *pScalePoint = scalePoint;
    return rank != 0.0;
    }

/*-----------------------------------------------------------------*//**
* @description Factor an affine transformation as a product of

<pre>
                  transform = Translation*R1*Scale*R2
</pre>

<p> where Translation is pure translation, R1 and R2 are pure rotation about the origin
   and Scale is a principle axis scale.
</p>

<p>
Once again:
<ul>
<li>Translation is a translation returned as
<li>R1 is a pure rotation matrix. It is orthogonal. Each column has magnitude 1.
                    Its determinant is 1. It defines a right handed coordinate system.</li>
<li>Scale conatains x, y, and z scale factors.  If converted to a matrix, the scale factors
                appear on the diagonal and the off-diagonals are zero.
                The largest absolute value scale factor is the x scale, and the smallest
                absolute value scale factor is the z scale.  The z scale (but no others)
                may be negative, which means that the transformation performs reflection.
                </li>
<li>R2 is a pure rotation matrix. It is orthogonal. Each column has magnitude 1.
                    Its determinant is 1.   It defines a right handed coordinate system.</li>
</p>

<p> If the composite transform is applied to a point, the equational form is</p>
<pre>
                   transform * point = Translation * R1 * Scale * R2 * Point
</pre>
<p>
If you read this from right to left, the interpretation is that the point is rotated (by R1)
around the global origin, scaled around the global origin, rotated again (by R1),
and translated.
</p>

<p>The rotations and scales are internally arranged so that the largest scale factors
are first in the scale point.</p>

<p>If you have perservered this far, you may recognize that the product Rotation1*Scale*Rotation2
    is a singular value decomposition.  (With small caveat that in some definitions of the
    singular values, all the scales are non-negative and the reflection effects make one
    of the rotations into a reflection.)
</p>

* @param pTransform IN transform being analyzed.
* @param pTranslation OUT translation part, as a bare DPoint3d.
*       Note that this point is not a FIXED point of the transformation, it is
*       the translation term directly out of the transformation.  A fixed point (if any)
*       of the transformation may be inquired via ~mbsiTransform_getAnyFixedPoint.
* @param pRotation1 OUT One of the rotation parts, as a RotMatrix.
* @param pScalePoint OUT scale factors, as xyz parts of a DPoint3d.
* @param pRotation2 OUT The other rotation part, as a RotMatrix.

* @bsihdr                                       EarlinLutz      03/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_factorTranslateRotateScaleRotate

(
TransformCP pTransform,
DPoint3dP pTranslation,
RotMatrixP pRotation1,
DPoint3dP pScalePoint,
RotMatrixP pRotation2
)
    {
    RotMatrix matrixPart;
    bool    boolstat;

    bsiRotMatrix_initFromTransform (&matrixPart, pTransform);
    boolstat = bsiRotMatrix_factorRotateScaleRotate (&matrixPart,
                    pRotation1, pScalePoint, pRotation2);
    if (pTranslation)
        bsiTransform_getTranslation (pTransform, pTranslation);

    return boolstat;
    }

/// <summary>Test if matrixRV is a specified single-axis rotation of matrixV</summary>
static bool    parseRotationFromMatrix

(
RotMatrixCP pMatrixRV,
int pivotRow, // Row expected to be unchanged
int viewId,     // standard id of view to test
int *pViewIdOut,
int *pAxisIdOut,
double *pRadiansOut
)
    {
    int j, mX, mY, mZ;
    double tol = bsiTrig_smallAngle ();
    double myAngle;
    RotMatrix matrixR, matrixRXY;
    RotMatrix *pMatrixV = &s_standardViewMatrix[viewId - 1];    // ASSUME .... viewId is one based and valid!!!

    // Rotation "around pivotRow" leaves the pivot row unchanged.
    // Deviation from any element of this row is an immediate out.
    // This fact makes this whole thing cheap.

    for (j = 0; j < 3; j++)
        if (fabs (pMatrixRV->form3d[pivotRow][j] - pMatrixRV->form3d[pivotRow][j]) > tol)
            return false;

    bsiRotMatrix_multiplyRotMatrixRotMatrixTranspose (&matrixR, pMatrixRV, pMatrixV);

    // We have an easy test for xy rotation.
    // Do diagonal swap of other cases to xy for testing ...
    mZ  = pivotRow;  // Becomes both row and column pivot.
    mX = bsiGeom_cyclic3dAxis (mZ + 1);
    mY = bsiGeom_cyclic3dAxis (mZ + 2);
    bsiRotMatrix_initFromRowValues (&matrixRXY,
                matrixR.form3d[mX][mX], matrixR.form3d[mX][mY], matrixR.form3d[mX][mZ],
                matrixR.form3d[mY][mX], matrixR.form3d[mY][mY], matrixR.form3d[mY][mZ],
                matrixR.form3d[mZ][mX],  matrixR.form3d[mZ][mY],  matrixR.form3d[mZ][mZ]
                );

    if (bsiRotMatrix_isXYRotation (&matrixRXY, &myAngle))
        {
        *pViewIdOut = viewId;
        *pAxisIdOut = pivotRow;
        *pRadiansOut = myAngle;
        return true;
        }
    return false;
    }


/// <summary>Test if a matrix can be factored as R*V, where R is a primary axis rotation
///         and V is a standard microstation view matrix.</summary>
/// <returns>true if the matrix is a rotation of the specified form.</returns>
/// <param name="pMatrix">Matrix to factor</param>
/// <param name="pRotationAxis">0,1, or 2 for rotation around x,y, or z respectively. </param>
/// <param name="pRadians">The rotation angle.</param>
/// <param name="pViewIndex">The view that the rotation starts from.</param>
/// <param name="bTestX">true to look for x axis rotations</param>
/// <param name="bTestY">true to look for y axis rotations</param>
/// <param name="bTestz">true to look for z axis rotations</param>
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isRotationFromStandardView

(
RotMatrixCP pMatrix,
int *pRotationAxis,
double *pRadians,
int *pViewIndex,
bool    bTestX,
bool    bTestY,
bool    bTestZ
)
    {
#ifndef STANDARDVIEW_Top
#define     STANDARDVIEW_Top        1
#define     STANDARDVIEW_Bottom     2
#define     STANDARDVIEW_Left       3
#define     STANDARDVIEW_Right      4
#define     STANDARDVIEW_Front      5
#define     STANDARDVIEW_Back       6
#define     STANDARDVIEW_Iso        7
#define     STANDARDVIEW_RightIso   8
#endif
   // Look for Z rotations first ....
    bool    boolstat =
        (bTestZ &&
            (  parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Top,      pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Front,    pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Right,    pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Left,     pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Back,     pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Bottom,   pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Iso,      pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_RightIso, pViewIndex, pRotationAxis, pRadians)
            ))
    // Then X
        || (bTestX &&
            (  parseRotationFromMatrix (pMatrix, 0, STANDARDVIEW_Top,    pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 0, STANDARDVIEW_Right,  pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 0, STANDARDVIEW_Left,   pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 0, STANDARDVIEW_Back,   pViewIndex, pRotationAxis, pRadians)
            ))
    // Then Y
        || (bTestY &&
            (  parseRotationFromMatrix (pMatrix, 1, STANDARDVIEW_Top,    pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 1, STANDARDVIEW_Front,  pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 1, STANDARDVIEW_Bottom, pViewIndex, pRotationAxis, pRadians)
            ))
        ;
    return boolstat;
    }

/// <summary>Initialize a matrix of the form R*V, where R is a primary axis rotation
///         and V is a standard microstation view matrix.</summary>
/// <returns>true if the view select is valid.</returns>
/// <param name="rotationAxis">0,1, or 2 for rotation around x,y, or z respectively. </param>
/// <param name="radians">The rotation angle.</param>
/// <param name="viewIndex">The view that the rotation starts from.</param>
Public GEOMDLLIMPEXP bool    bsiRotMatrix_initRotationFromStandardView

(
RotMatrixP pMatrix,
int rotationAxis,
double radians,
int viewIndex
)
    {
    RotMatrix matrixR, matrixV;
    if (bsiRotMatrix_getStandardRotation (&matrixV, viewIndex))
        {
        bsiRotMatrix_initFromAxisAndRotationAngle (&matrixR, rotationAxis, radians);
        bsiRotMatrix_multiplyRotMatrixRotMatrix (pMatrix, &matrixR, &matrixV);
        return true;
        }
    bsiRotMatrix_initIdentity (pMatrix);
    return false;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
