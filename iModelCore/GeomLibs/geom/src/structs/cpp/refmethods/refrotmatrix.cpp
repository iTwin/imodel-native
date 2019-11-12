/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE
#define CompileAllxxx



static const double s_mediumRelTol = 1.0e-12;


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void updateExtrema
(
double *pMin,
double *pMax,
double  a
)
    {
    if (a < *pMin)
        *pMin = a;
    if (a > *pMax)
        *pMax = a;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool RotMatrix::RotateAndSkewFactors
(
RotMatrixR rotation,
RotMatrixR skewFactor,
int         primaryAxis,
int         secondaryAxis
) const
    {
    bool    boolstat = false;

    if (rotation.SquareAndNormalizeColumns (*this, primaryAxis, secondaryAxis))
        {
        skewFactor.InitProductRotMatrixTransposeRotMatrix (rotation, *this);
        boolstat = true;
        }
    else
        {
        skewFactor = *this;
        rotation.InitIdentity ();
        boolstat = false;
        }
    return boolstat;
    }

/*-----------------------------------------------------------------*//**
* @param [in] quat The quaternion, stored as (xyzw)
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::FromQuaternion
(
DPoint4dCR      quat
)
    {
    RotMatrix matrix;
    matrix.InitFromQuaternion (quat);

    return matrix;
    }

/*-----------------------------------------------------------------*//**
* @param [in] pQuatAsDoubleArray The quaternion, stored as (w,x,y,z) in an array of doubles.
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::FromQuaternion
(
const double    *pQuatAsDoubleArray
)
    {
    RotMatrix matrix;
    matrix.InitFromQuaternion (pQuatAsDoubleArray);

    return matrix;
    }



/*-----------------------------------------------------------------*//**
* @description Returns a matrix formed from a scaling matrix which is
*       multiplied on the left and/or right with other matrices.
*       That is, form LeftMatrix * ScaleMatrix * RightMatrix
*       where the ScaleMatrix is constructed from the given scale factors.
*       If LeftMatrix is null, this has the effect of scaling the rows
*       of the right matrix.  If RightMatrix is null this has the effect
*       of scaling the columns of the left matrix.
* @param [in] leftMatrix The matrix on left of product
* @param [in] xs The x scale factor
* @param [in] ys The y scale factor
* @param [in] zs The z scale factor
* @param [in] leftMatrix The matrix on right of product
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::Scale
(

RotMatrixCR leftMatrix,
      double         xs,
      double         ys,
      double         zs,
RotMatrixCR rightMatrix

)
    {
    RotMatrix intermediateMatrix;
    intermediateMatrix.ScaleColumns (leftMatrix, xs, ys, zs);
    this->InitProduct (intermediateMatrix, rightMatrix);
    }


/*-----------------------------------------------------------------*//**
* @description Returns a matrix whose rows are unit vectors in the same
*   drection as corresponding rows of the input matrix.
*   Also (optionally) stores the original row magnitudes as components of the point.
* @param [in] inMatrix The input matrix
* @param [out] scaleVector length of original rows
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::NormalizeRowsOf
(

RotMatrixCR inMatrix,
DVec3dR scaleVector

)
    {
    DVec3d xRow, yRow, zRow, mag;

    inMatrix.GetRow (xRow, 0);
    mag.x = xRow.Magnitude ();
    inMatrix.GetRow (yRow, 1);
    mag.y = yRow.Magnitude ();
    inMatrix.GetRow (zRow, 2);
    mag.z = zRow.Magnitude ();

    scaleVector = mag;

    if (mag.x > 0.0)
        xRow.Scale (xRow, 1.0/mag.x);
    if (mag.y > 0.0)
        yRow.Scale (yRow, 1.0/mag.y);
    if (mag.z > 0.0)
        zRow.Scale (zRow, 1.0/mag.z);

    this->InitFromRowVectors (xRow, yRow, zRow);
    }


/*-----------------------------------------------------------------*//**
* @description Returns a matrix whose rows are unit vectors in the same
*   drection as corresponding columns of the input matrix.
*   Also (optionally) stores the original column magnitudes as components of the point.
* @param [in] inMatrix The input matrix
* @param [out] scaleVector length of original columns
* @bsimethod                                                    DavidAssaf      10/98
+----------------------------------------------------------------------*/
void RotMatrix::NormalizeColumnsOf
(

RotMatrixCR inMatrix,
DVec3dR scaleVector

)
    {
    DVec3d    xCol, yCol, zCol, mag;

    inMatrix.GetColumn (xCol, 0);
    mag.x = xCol.Magnitude ();
    inMatrix.GetColumn (yCol, 1);
    mag.y = yCol.Magnitude ();
    inMatrix.GetColumn (zCol, 2);
    mag.z = zCol.Magnitude ();

    scaleVector = mag;

    if (mag.x > 0.0)
        xCol.Scale (xCol, 1.0/mag.x);
    if (mag.y > 0.0)
        yCol.Scale (yCol, 1.0/mag.y);
    if (mag.z > 0.0)
        zCol.Scale (zCol, 1.0/mag.z);

    this->InitFromColumnVectors (xCol, yCol, zCol);
    }


/*-----------------------------------------------------------------*//**
* @description Returns the determinant of the matrix.
* @param [in] pMatrix The matrix to query
* @return determinant of the matrix.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double RotMatrix::Determinant
(

) const
    {
    return
          form3d[0][0] * form3d[1][1] * form3d[2][2]
        + form3d[0][1] * form3d[1][2] * form3d[2][0]
        + form3d[0][2] * form3d[1][0] * form3d[2][1]
        - form3d[0][0] * form3d[1][2] * form3d[2][1]
        - form3d[0][1] * form3d[1][0] * form3d[2][2]
        - form3d[0][2] * form3d[1][1] * form3d[2][0];
    }


/*-----------------------------------------------------------------*//**
* Computes an estimate of the condition of this instance matrix.  Values near 0
* are bad.
*
* @param [in] pMatrix The input matrix
* @return estimated condition number.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double RotMatrix::ConditionNumber
(

) const
    {
    /* Estimate the condition number as the ratio of the determinant
        (volume of tetrahedron) to 1/3 the product of the 3 column
        lengths (volume of tetrahedron on orthogonal vectors of
        same length)
    */
    double result;
    DVec3d vector[3];
    double numerator = fabs (this->Determinant ());
    double denominator = 1.0;
    int i;

    this->GetColumns (vector[0], vector[1], vector[2]);
    for (i = 0; i < 3; i++)
        {
        denominator *= vector[i].Magnitude ();
        }
    if (denominator > 0.0 )
        result = numerator / denominator;
    else
        result = 0.0;
    return result;
    }


/*-----------------------------------------------------------------*//**
* @description Tests if a matrix is the identity matrix.
* @param [in] pMatrix The matrix to test
* @return true if matrix is approximately an identity.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::IsIdentity
(

) const
    {
    /* Entries in an identity matrix are strictly in the 0..1 range,*/
    /* hence a simple tolerance near machine precision is warranted.*/
    double tol = s_mediumRelTol;

    if (
           fabs(form3d[0][0] - 1.0)   <= tol
        && fabs(form3d[1][0]      )   <= tol
        && fabs(form3d[2][0]      )   <= tol

        && fabs(form3d[0][1]      )   <= tol
        && fabs(form3d[1][1] - 1.0)   <= tol
        && fabs(form3d[2][1]      )   <= tol
        && fabs(form3d[0][2]      )   <= tol
        && fabs(form3d[1][2]      )   <= tol
        && fabs(form3d[2][2] - 1.0)   <= tol
        )
        return true;

    return false;
    }


/*-----------------------------------------------------------------*//**
* @description Tests if a matrix has small offdiagonal entries compared to
*                   diagonals.   The specific test condition is that the
*                   largest off diagonal absolute value is less than a tight tolerance
*                   fraction times the largest diagonal entry.
* @param [in] pMatrix The matrix to test
* @return true if matrix is approximately diagonal
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::IsDiagonal
(

) const
    {
    double dmax, dmin;
    double smax, smin;
    this->DiagonalAbsRange (dmin, dmax);
    this->OffDiagonalAbsRange (smin, smax);

    return smax < s_mediumRelTol * dmax;
    }


/*-----------------------------------------------------------------*//**
* @description Tests if a matrix has (nearly) equal diagaonal entries and
*           (nearly) zero off diagonals.  Tests use a tight relative tolerance.
* @param [in] pMatrix The matrix to test
* @param [out] maxScale the largest diagaonal entry
* @return true if matrix is approximately diagonal
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::IsUniformScale
(
double        &maxScale
) const
    {
    double dmax, dmin;
    double smax, smin;
    bool isUniform = false;
    this->DiagonalSignedRange (dmin, dmax);
    this->OffDiagonalAbsRange (smin, smax);
    double delta = dmax - dmin;
    double absD = DoubleOps::MaxAbs (dmin, dmax);
    isUniform =  smax < s_mediumRelTol * absD
              && delta < s_mediumRelTol * fabs (dmax);
    maxScale = fabs (dmin) > fabs (dmax) ? dmin : dmax;
    return isUniform;
    }


/*-----------------------------------------------------------------*//**
* @description Return the (signed) range of entries on the diagonal.
* @param [in] pMatrix The matrix to test
* @param [out] minValue smallest signed value
* @param [out] maxValue largest signed value
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::DiagonalSignedRange
(
double              &minValue,
double              &maxValue
) const
    {
    minValue = maxValue = form3d[0][0];
    updateExtrema (&minValue, &maxValue, form3d[1][1]);
    updateExtrema (&minValue, &maxValue, form3d[2][2]);
    }


/*-----------------------------------------------------------------*//**
* @description Return the (absolute value) range of entries on the diagonal.
* @param [in] pMatrix The matrix to test
* @param [out] minValue smallest absolute value
* @param [out] maxValue largest absolute value
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::DiagonalAbsRange
(
double  &minValue,
double  &maxValue
) const
    {
    minValue = maxValue = fabs (form3d[0][0]);
    updateExtrema (&minValue, &maxValue, fabs (form3d[1][1]));
    updateExtrema (&minValue, &maxValue, fabs (form3d[2][2]));
    }

DRange1d RotMatrix::LowerTriangleAbsRange ()
    {
    return DRange1d::From (
                fabs (form3d[2][0]),
                fabs (form3d[2][1]),
                fabs (form3d[1][0])
                );
    }
DRange1d RotMatrix::UpperTriangleAbsRange ()
    {
    return DRange1d::From (
                fabs (form3d[0][1]),
                fabs (form3d[0][2]),
                fabs (form3d[1][2])
                );
    }
DRange1d RotMatrix::DiagonalAbsRange ()
    {
    return DRange1d::From (
                fabs (form3d[0][0]),
                fabs (form3d[1][1]),
                fabs (form3d[2][2])
                );
    }
double RotMatrix::LowerTriangleMaxAbs ()
    {
    return DoubleOps::Max (
                fabs (form3d[2][0]),
                fabs (form3d[2][1]),
                fabs (form3d[1][0])
                );
    }
double RotMatrix::UpperTriangleMaxAbs ()
    {
    return DoubleOps::Max (
                fabs (form3d[0][1]),
                fabs (form3d[0][2]),
                fabs (form3d[1][2])
                );
    }
double RotMatrix::DiagonalMaxAbs ()
    {
    return DoubleOps::Max (
                fabs (form3d[0][0]),
                fabs (form3d[1][1]),
                fabs (form3d[2][2])
                );
    }

/*-----------------------------------------------------------------*//**
* @description Return the (signed) range of entries off the diagonal.
* @param [in] pMatrix The matrix to test
* @param [out] minValue smallest signed value
* @param [out] maxValue largest signed value
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::OffDiagonalSignedRange
(
double              &minValue,
double              &maxValue
) const
    {
    minValue = maxValue = form3d[1][0];
    updateExtrema (&minValue, &maxValue, form3d[2][0]);
    updateExtrema (&minValue, &maxValue, form3d[0][1]);
    updateExtrema (&minValue, &maxValue, form3d[2][1]);
    updateExtrema (&minValue, &maxValue, form3d[1][2]);
    updateExtrema (&minValue, &maxValue, form3d[0][2]);
    }


/*-----------------------------------------------------------------*//**
* @description Return the (absolute value) range of entries off the diagonal.
* @param [in] pMatrix The matrix to test
* @param [out] pMinValue smallest absolute value
* @param [out] pMaxValue largest absolute value
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::OffDiagonalAbsRange
(
double              &minValue,
double              &maxValue
) const
    {
    minValue = maxValue = form3d[0][1];
    updateExtrema (&minValue, &maxValue, fabs (form3d[0][2]));
    updateExtrema (&minValue, &maxValue, fabs (form3d[1][0]));
    updateExtrema (&minValue, &maxValue, fabs (form3d[1][2]));
    updateExtrema (&minValue, &maxValue, fabs (form3d[2][0]));
    updateExtrema (&minValue, &maxValue, fabs (form3d[2][1]));
    }


/*-----------------------------------------------------------------*//**
* @description Test if this instance matrix does nothing more
* than exchange and possibly negate principle axes.
* @param [in] pMatrix The input matrix
* @return true if the matrix is a permutation of the principle axes.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::IsSignedPermutation
(

) const
    {
    double zeroTol = 1.0e-14;
    double a, b, c, d;

    int k, k1, k2;

    /* Is the x row two zeros and a +-1?*/
    a = fabs (form3d[0][0]);
    b = fabs (form3d[0][1]);
    c = fabs (form3d[0][2]);

    if (fabs( a - 1.0) < zeroTol)
        {
        if (   b > zeroTol
            || c > zeroTol )
            return false;
        k  = 0;
        k1 = 1;
        k2 = 2;
        }
    else if (fabs (b - 1.0) < zeroTol)
        {
        if (   c > zeroTol
            || a > zeroTol )
            return false;
        k  = 1;
        k1 = 2;
        k2 = 0;
        }
    else if (fabs (c - 1.0) < zeroTol)
        {
        if (   a > zeroTol
            || b > zeroTol )
            return false;
        k  = 2;
        k1 = 0;
        k2 = 1;
        }
    else
        return false;

    /* Check the rest of the column.*/
    if (   fabs (form3d[1][k]) > zeroTol
        || fabs (form3d[2][k]) > zeroTol)
        return false;

    a = fabs (form3d[1][k1]);
    b = fabs (form3d[1][k2]);
    c = fabs (form3d[2][k1]);
    d = fabs (form3d[2][k2]);

    /* The valid configurations of the 2x2 matrix after eliminating*/
    /* row zero and column k are of the form*/
    /*   [ a b ]    =       [ 0 1 ]   or    [ 1 0 ]*/
    /*   [ c d ]            [ 1 0 ]         [ 0 1 ]*/

    if (a < zeroTol)
        {
        if (   d < zeroTol
            && fabs (b - 1.0) < zeroTol
            && fabs (c - 1.0) < zeroTol
           )
            return true;
        }
    else if (fabs (a - 1.0) < zeroTol)
        {
        if (   fabs (d - 1.0) < zeroTol
            && b < zeroTol
            && c < zeroTol
           )
            return true;
        }

    return false;
    }

static void CopySignedOne (RotMatrixR dest, int i, int j, RotMatrixCR source)
    {
    dest.form3d[i][j] = source.form3d[i][j] > 0.0 ? 1.0 : -1.0;
    }
// ASSUME row and column are safe !!!
static bool nearUnitRow (RotMatrixCR matrix, int row, int unitCol, int j0, int j1, double tolerance)
    {
    double e0 = fabs (matrix.form3d[row][unitCol]) - 1.0;
    return    fabs (e0) < tolerance
           && fabs (matrix.form3d[row][j0]) < tolerance
           && fabs (matrix.form3d[row][j1]) < tolerance
           ;
    }
/*-----------------------------------------------------------------*//**
* @bsimethod                                  EarlinLutz      09/12
+----------------------------------------------------------------------*/
bool RotMatrix::IsNearSignedPermutation (RotMatrixR result, double tolerance) const
    {
    result = *this;
    if (tolerance < 1.0e-16)
        tolerance = 1.0e-16;
    int ix, iy, iz;
    for (int i = 0; i < 3; i++)
        {
        Angle::Cyclic3dAxes (ix, iy, iz, i);
        if (nearUnitRow (*this, 0, ix, iy, iz, tolerance))
            {
            if (   nearUnitRow (*this, 1, iy, iz, ix, tolerance)
                && nearUnitRow (*this, 2, iz, ix, iy, tolerance))
                {
                result.Zero ();
                CopySignedOne (result, 0, ix, *this);
                CopySignedOne (result, 1, iy, *this);
                CopySignedOne (result, 2, iz, *this);
                return true;
                }
            if (   nearUnitRow (*this, 2, iy, iz, ix, tolerance)
                && nearUnitRow (*this, 1, iz, ix, iy, tolerance))
                {
                result.Zero ();
                CopySignedOne (result, 0, ix, *this);
                CopySignedOne (result, 2, iy, *this);
                CopySignedOne (result, 1, iz, *this);
                return true;
                }
             }
        }
    return false;
    }


/*-----------------------------------------------------------------*//**
* @description Test if a matrix is a rigid body rotation,
*   i.e. its transpose is its inverse and it has a positive determinant.
* @return true if the matrix is a rigid body rotation.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::IsRigid
(

) const
    {
    RotMatrix transpose;
    RotMatrix product;
    transpose.TransposeOf (*this);
    product.InitProduct (transpose, *this);
    return      product.IsIdentity ()
            &&  (this->Determinant () > 0.0);
    }


/*-----------------------------------------------------------------*//**
* @description Test if this instance matrix is orthogonal, i.e. its transpose is its inverse.
*   This class of matrices includes both rigid body rotations and reflections.
* @return true if the matrix is orthogonal.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::IsOrthogonal
(

) const
    {
    RotMatrix transpose;
    RotMatrix product;
    transpose.TransposeOf (*this);
    product.InitProduct (transpose, *this);
    return      product.IsIdentity ();
    }


/*-----------------------------------------------------------------*//**
* @description Test if this instance matrix has orthonormal columns, i.e. its columns
* are all perpendicular to one another.
* @param [out] columns (optional) matrix containing the unit vectors along the columns.
* @param [out] axisScales (optional) point whose x, y, and z components are the magnitudes of the
*           original columns.
* @param *axisRatio <= (optional) smallest axis length divided by largest.
* @return true if the matrix is orthonormal.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::IsOrthonormal
(
RotMatrixR columns,
DVec3dR    axisScales,
double     &axisRatio

) const
    {
    DVec3d column[3];
    RotMatrix matrixR, matrixRT, matrixRTR;

    this->GetColumns (column[0], column[1], column[2]);
    axisScales.x = column[0].Normalize ();
    axisScales.y = column[1].Normalize ();
    axisScales.z = column[2].Normalize ();
    matrixR.InitFromColumnVectors (column[0], column[1], column[2]);
    matrixRT.TransposeOf (matrixR);
    matrixRTR.InitProduct (matrixRT, matrixR);

    columns = matrixR;
    DRange1d scaleRange = axisScales.ComponentRange ();
    if (scaleRange.high > 0.0)
        axisRatio = scaleRange.low / scaleRange.high;
    else
        axisRatio = 0.0;

    return matrixRTR.IsIdentity ();
    }


/*-----------------------------------------------------------------*//**
* @description Test if this instance matrix is composed of only rigid rotation and scaling.
* @param [out] columns (optional) matrix containing the unit vectors along the columns.
* @param [out] scale largest axis scale factor.  If function value is true,
*       the min scale is the same.  Use areColumnsOrthonormal to get
*       separate column scales.
* @return true if the matrix is orthonormal.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::IsRigidScale
(
RotMatrixR columns,
double     &scale
) const
    {
    double almostOne = 1.0 - s_mediumRelTol;
    DVec3d axisScales;
    double ratio;

    bool ortho = this->IsOrthonormal (columns, axisScales, ratio);

    scale = axisScales.x;
    if (axisScales.y > scale)
        scale = axisScales.y;
    if (axisScales.z > scale)
        scale = axisScales.z;

    return ortho && ratio > almostOne && this->Determinant () > 0.0;
    }

/*-----------------------------------------------------------------*//**
* @description Test if this instance matrix is composed of only rigid rotation and (possibly negative) scaling
* @param [out] columns (optional) matrix containing the unit vectors along the columns.
* @param [out] scale largest axis scale factor.  If function value is true,
*       the min scale is the same.  Use areColumnsOrthonormal to get
*       separate column scales.
* @return true if the matrix is orthonormal.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::IsRigidSignedScale
(
RotMatrixR columns,
double     &scale
) const
    {
    double almostOne = 1.0 - s_mediumRelTol;
    DVec3d axisScales;
    double ratio;

    bool ortho = this->IsOrthonormal (columns, axisScales, ratio);
    // Start scale as largest absolute scale ...
    scale = axisScales.x;
    if (axisScales.y > scale)
        scale = axisScales.y;
    if (axisScales.z > scale)
        scale = axisScales.z;
    // adjust for determinant ..
    if (Determinant () < 0.0)
        scale = - scale;
    double s1 = 1.0 / scale;
    columns.ScaleColumns (*this, s1, s1, s1);
    return ortho && ratio > almostOne;
    }

/*-----------------------------------------------------------------*//**
* @description Tests if this instance matrix has no effects perpendicular to any plane with the given normal.  This
* will be true if the matrix represents a combination of (a) scaling perpencicular to the normal
* and (b) rotation around the normal.

* @param [in] normal The plane normal
* @return true if the matrix has no effects perpendicular to any plane
*   with the given normal.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::IsPlanar
(

DVec3dCR normal

) const
    {
    DVec3d vector;
    double norm2, vec2;
    double tolSquared = 1.0E-20;
    vector = normal;
    this->Multiply (vector);
    vector.DifferenceOf (vector, normal);
    norm2 = normal.DotProduct (normal);
    vec2  = vector.DotProduct (vector);
    return vec2 <= tolSquared * norm2;
    }


/*-----------------------------------------------------------------*//**
* @description Adjust the direction and length of columns of the input matrix to
* produce an instance matrix which has perpendicular, unit length columns.
* The column whose index is primaryAxis (i.e. 0,1,2 for x,y,z axis of
* coordinate frame) is normalized to unit length in its current
* direction.
* The column whose index is secondaryAxis is unit length and perpendicular
* to the primaryAxis column, and lies in the same plane as that
* defined by the original primary and secondary columns.
* To preserve X axis and XY plane, call with axis id's 0 and 1.
* To preserve Z axis and ZX plane, call with axis id's 2 and 0.
* inMatrix and pMatrix may be the same address.
*
* @param [in] inMatrix The input matrix
* @param [in] primaryAxis The axis id (0, 1, 2) which is to be normalized but left
*           in its current direction
* @param [in] secondaryAxis The axis id (0, 1, 2) which is to be kept within the
*           plane of the primary and secondary axis.
* @return false if primaryAxis and secondaryAxis are the same, or either axis has zero length
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::SquareAndNormalizeColumns
(

RotMatrixCR inMatrix,
int         primaryAxis,
int         secondaryAxis

)
    {
    int ix = Angle::Cyclic3dAxis (primaryAxis);
    int iy = Angle::Cyclic3dAxis (secondaryAxis);
    int iz = 3 - (ix + iy);
    double lx, ly;
    DVec3d vector[3];
    bool boolStat = false;

    if (ix == iy)
        {
        *this = inMatrix;
        return false;
        }

    inMatrix.GetColumns (vector[0], vector[1], vector[2]);

    lx = vector[ix].Normalize ();
    double lz = vector[iz].NormalizedCrossProduct (vector[ix], vector[iy]);
    if (0.0 == lz)
        {
        *this = inMatrix;
        return false;
        }
    ly = vector[iy].NormalizedCrossProduct (vector[iz], vector[ix]);

    if (ly == 0.0)
        {
        if (lx == 0.0)
            {
            this->InitIdentity ();
            boolStat = false;
            }
        else
            {
            vector[ix].GetNormalizedTriad (vector[iy], vector[iz], vector[ix]);
            this->InitFromColumnVectors (vector[0], vector[1], vector[2]);
            boolStat = false;
            }
        }
    else
        {
        /* negate z-axis if x follows y (this prevents negative determinant) */
        if ((iy + 1) % 3 == ix)
            vector[iz].Negate ();
        this->InitFromColumnVectors (vector[0], vector[1], vector[2]);
        boolStat = true;
        }

    return boolStat;
    }


bool RotMatrix::SquareAndNormalizeColumns
(

RotMatrixCR inMatrix,
int         primaryAxis,
int         secondaryAxis,
int         preferredOrientation
)
    {
    int ix = Angle::Cyclic3dAxis (primaryAxis);
    int iy = Angle::Cyclic3dAxis (secondaryAxis);
    int iz = 3 - (ix + iy);

    double d0 = inMatrix.Determinant ();
    if (!SquareAndNormalizeColumns (inMatrix, primaryAxis, secondaryAxis))
        return false;
    if (preferredOrientation > 0)
        return true;
    DVec3d columnZ;
    GetColumn (columnZ, iz);
    if (preferredOrientation < 0)
        {
        columnZ.Negate ();
        }
    else
        {
        double d1 = Determinant ();
        if (!DoubleOps::AlmostEqual (d0, 0.0) && d0 * d1 < 0.0)
            columnZ.Negate ();
        }
    SetColumn (columnZ, iz);
    return true;
    }

/*-----------------------------------------------------------------*//**
* @description Returns an orthogonal matrix that preserves aligns with
* the columns of inMatrix.  This is done by
* trying various combinations of primary and secondary axes until one
* succeeds in squareAndNormalizeColumns.
*
* @param [out] pMatrix normalized matrix
* @param [in] inMatrix The input matrix
* @param [in] preferredOrientation 
* <pre>
* <ul>
* <li>1 for right handed system
* <li>-1 for left handed system
* <li>0 to match orientation of input (but default to right handed if input is singular)
* </ul>
* </pre>
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::SquareAndNormalizeColumnsAnyOrder
(

RotMatrixCR inMatrix,
int preferredOrientation
)
    {
    RotMatrix tempMatrix;
    bool boolStat;
    int i, j;

    boolStat = false;
    tempMatrix = inMatrix;
    for (i = 0; i < 3 && !boolStat; i++)
        {
        j = (i + 1) % 3;
        boolStat = this->SquareAndNormalizeColumns(tempMatrix, i, j, preferredOrientation);
        }
    return boolStat;
    }

bool RotMatrix::SquareAndNormalizeColumnsAnyOrder (RotMatrixCR inMatrix)
    {
    return SquareAndNormalizeColumnsAnyOrder (inMatrix, 1);
    }
/*-----------------------------------------------------------------*//**
* @description Moves columns 0, 1, 2 of the input matrix into columns i0, i1, i2
* of the instance.
* @param [out] pMatrix shuffled matrix
* @param [in] inMatrix The input matrix
* @param [in] i0 The column to receive input column 0
* @param [in] i1 The column to receive input column 1
* @param [in] i2 The column to receive input column 2
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::ShuffleColumnsOf
(

RotMatrixCR inMatrix,
      int            i0,
      int            i1,
      int            i2

)
    {
    RotMatrix A = inMatrix;
    int index[3];
    int i, j;
    index[0] = Angle::Cyclic3dAxis (i0);
    index[1] = Angle::Cyclic3dAxis  (i1);
    index[2] = Angle::Cyclic3dAxis  (i2);

    for (i = 0; i < 3; i++)
        {
        j = index[i];
        form3d[0][j] = A.form3d[0][i];
        form3d[1][j] = A.form3d[1][i];
        form3d[2][j] = A.form3d[2][i];
        }
    }


/*-----------------------------------------------------------------*//**
@description Choose and carry out computation of a quaternion coefficient by one of two formulas.
@remarks The product formula is: numerator * reciprocal
@remarks The diagonal sum formula is: sqrt (diagSum) / 2.0.  This form is better if diagSum is large.
@param numerator => The numerator of product formula
@param denomCoff => The denominator coefficient of product formula
@param reciprocal => 1 / (4 * denomCoff)
@param diagSum => The alternate formula sum
@return quaternion coefficient
@group RotMatrix
@bsimethod                                                      EarlinLutz      03/98
+----------------------------------------------------------------------*/
double computeQuatTerm


(
double numerator,
double denomCoff,
double reciprocal,
double diagSum
)
    {
    double coff;
    static double diagTol = 0.500;
    if (diagSum > diagTol)
        {
        coff = sqrt (diagSum) * 0.5;
        if (denomCoff * numerator < 0.0)
            coff = - coff;
        }
    else
        {
        coff = numerator * reciprocal;
        }
    return coff;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| Conversion between 3D matrix (rotation) and quaternion (stored as     |
| DPoint4d).                                                            |
|                                                                       |
+----------------------------------------------------------------------*/



/*-----------------------------------------------------------------*//**
@instance pMatrix       <= matrix to be computed from quaternion
@param pQuat     => The quaternion, stored as (xyzw)
@param   transpose          => true if matrix is stored transposed
@group Quaternions
@bsimethod                                                      EarlinLutz      12/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_fromQuaternion
(
RotMatrixP pMatrix,
DPoint4dCP pQuat
)
    {
    double      qqw, qqx, qqy, qqz, mag2;

    qqx = pQuat->x * pQuat->x;
    qqy = pQuat->y * pQuat->y;
    qqz = pQuat->z * pQuat->z;
    qqw = pQuat->w * pQuat->w;

    mag2 = qqx + qqy + qqz + qqw;

    if (mag2 == 0.0)
        {
        pMatrix->InitIdentity ();
        }
    else
        {
        double a = 1.0 / mag2;

        pMatrix->form3d[0][0] = a * (qqw + qqx - qqy - qqz);
        pMatrix->form3d[1][0] = 2.0 * a * (  pQuat->w * pQuat->z
                                            + pQuat->x * pQuat->y);
        pMatrix->form3d[2][0] = 2.0 * a * (  pQuat->x * pQuat->z
                                            - pQuat->w * pQuat->y);

        pMatrix->form3d[0][1] = 2.0 * a * (  pQuat->x * pQuat->y
                                            - pQuat->w * pQuat->z);
        pMatrix->form3d[1][1] = a * (qqw - qqx + qqy - qqz);
        pMatrix->form3d[2][1] = 2.0 * a * (  pQuat->w * pQuat->x
                                            + pQuat->y * pQuat->z);

        pMatrix->form3d[0][2] = 2.0 * a * (  pQuat->x * pQuat->z
                                            + pQuat->w * pQuat->y);
        pMatrix->form3d[1][2] = 2.0 * a * (  pQuat->y * pQuat->z
                                            - pQuat->w * pQuat->x);
        pMatrix->form3d[2][2] = a * (qqw - qqx - qqy + qqz);
        }

    }


/*-----------------------------------------------------------------*//**
* @param [in] pQuatAsDoubles The quaternion, stored as (xyzw)
* @param [in] transpose true if matrix is stored transposed
* @indexVerb quaternion
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitFromQuaternion
(

DPoint4dCR quat

)
    {
    bsiRotMatrix_fromQuaternion (this, (&quat));
    }





/*-----------------------------------------------------------------*//**
@instance pMatrix  => The matrix to be converted to quaternion
@param pQuat   <= quaternion, stored as xyzw
@param transpose   => true if matrix is stored transposed
@group Quaternions
@bsimethod                                                      EarlinLutz      3/98
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_toQuaternion


(
RotMatrixCP pMatrix,
DPoint4dP pQuat,
bool                transpose
)
    {
    double xx = pMatrix->form3d[0][0];
    double yy = pMatrix->form3d[1][1];
    double zz = pMatrix->form3d[2][2];
    double dSum[4];
    double denom;
    int maxIndex;
    int i;

    dSum[0] = 1.0 + xx - yy - zz;
    dSum[1] = 1.0 - xx + yy - zz;
    dSum[2] = 1.0 - xx - yy + zz;
    dSum[3] = 1.0 + xx + yy + zz;

    maxIndex = 0;
    for (i = 1; i < 4; i++)
        {
        if (dSum[i] > dSum[maxIndex])
            maxIndex = i;
        }


    if (maxIndex == 0)
        {
        pQuat->x = 0.5 * sqrt (dSum[0]);
        denom = 1.0 / (4.0 * pQuat->x);

        pQuat->y = computeQuatTerm
                    (
                    pMatrix->form3d[0][1] + pMatrix->form3d[1][0],
                    pQuat->x,
                    denom,
                    dSum[1]
                    );

        pQuat->z = computeQuatTerm
                    (
                    pMatrix->form3d[0][2] + pMatrix->form3d[2][0],
                    pQuat->x,
                    denom,
                    dSum[2]
                    );

        pQuat->w = computeQuatTerm
                    (
                    pMatrix->form3d[2][1] - pMatrix->form3d[1][2],
                    pQuat->x,
                    denom,
                    dSum[3]
                    );

        }
    else if (maxIndex == 1)
        {
        pQuat->y = 0.5 * sqrt (dSum[1]);
        denom = 1.0 / (4.0 * pQuat->y);
        pQuat->x = computeQuatTerm
                    (
                    pMatrix->form3d[0][1] + pMatrix->form3d[1][0],
                    pQuat->y,
                    denom,
                    dSum[0]
                    );

        pQuat->z = computeQuatTerm
                    (
                    pMatrix->form3d[1][2] + pMatrix->form3d[2][1],
                    pQuat->y,
                    denom,
                    dSum[2]
                    );

        pQuat->w = computeQuatTerm
                    (
                    pMatrix->form3d[0][2] - pMatrix->form3d[2][0],
                    pQuat->y,
                    denom,
                    dSum[3]
                    );

        }
    else if (maxIndex == 2)
        {
        pQuat->z = 0.5 * sqrt (dSum[2]);
        denom = 1.0 / (4.0 * pQuat->z);

        pQuat->x = computeQuatTerm
                    (
                    pMatrix->form3d[0][2] + pMatrix->form3d[2][0],
                    pQuat->z,
                    denom,
                    dSum[0]
                    );

        pQuat->y = computeQuatTerm
                    (
                    pMatrix->form3d[1][2] + pMatrix->form3d[2][1],
                    pQuat->z,
                    denom,
                    dSum[1]
                    );

        pQuat->w = computeQuatTerm
                    (
                    pMatrix->form3d[1][0] - pMatrix->form3d[0][1],
                    pQuat->z,
                    denom,
                    dSum[3]
                    );

    }
    else
        {
        pQuat->w = 0.5 * sqrt (dSum[3]);
        denom = 1.0 / (4.0 * pQuat->w);
        pQuat->x = computeQuatTerm
                    (
                    pMatrix->form3d[2][1] - pMatrix->form3d[1][2],
                    pQuat->w,
                    denom,
                    dSum[0]
                    );

        pQuat->y = computeQuatTerm
                    (
                    pMatrix->form3d[0][2] - pMatrix->form3d[2][0],
                    pQuat->w,
                    denom,
                    dSum[1]
                    );

        pQuat->z = computeQuatTerm
                    (
                    pMatrix->form3d[1][0] - pMatrix->form3d[0][1],
                    pQuat->w,
                    denom,
                    dSum[2]
                    );

        }

    if (transpose)
        pQuat->w = - pQuat->w;
    }



/*-----------------------------------------------------------------*//**
* @param [out] pQuatAsDoubles quaternion, stored as xyzw
* @param [in] transpose true if matrix is stored transposed
* @indexVerb quaternion
* @bsimethod                                                    EarlinLutz      3/98
+----------------------------------------------------------------------*/
void RotMatrix::GetQuaternion
(

DPoint4dR quat,
bool transpose

) const
    {
    RotMatrix trustedRotation;
    trustedRotation.SquareAndNormalizeColumns (*this, 0, 1);
    bsiRotMatrix_toQuaternion (this, (&quat),(transpose ? true : false));
    }


/*-----------------------------------------------------------------*//**
* @param [out] pQuatAsDoubles quaternion, stored as (w,x,y,z) in an array of doubles.
* @param [in] transpose true if matrix is stored transposed
* @indexVerb quaternion
* @bsimethod                                                    EarlinLutz      3/98
+----------------------------------------------------------------------*/
void RotMatrix::GetQuaternion
(

double              *pQuatAsDoubleArray,
bool transpose

) const
    {
    DPoint4d quat;
    RotMatrix trustedRotation;
    trustedRotation.SquareAndNormalizeColumns (*this, 0, 1);
    bsiRotMatrix_toQuaternion (&trustedRotation, &quat, transpose);
    pQuatAsDoubleArray[0] = quat.w;
    pQuatAsDoubleArray[1] = quat.x;
    pQuatAsDoubleArray[2] = quat.y;
    pQuatAsDoubleArray[3] = quat.z;
    }


/*-----------------------------------------------------------------*//**
* @param [in] pQuatAsDoubles The quaternion, stored as (w,x,y,z) in an array of doubles.
* @indexVerb quaternion
* @bsimethod                                                    EarlinLutz      3/98
+----------------------------------------------------------------------*/
void RotMatrix::InitFromQuaternion
(

const double        *pQuatAsDoubleArray

)
    {
    DPoint4d quat;
    quat.x = pQuatAsDoubleArray[1];
    quat.y = pQuatAsDoubleArray[2];
    quat.z = pQuatAsDoubleArray[3];
    quat.w = pQuatAsDoubleArray[0];
    bsiRotMatrix_fromQuaternion (this, &quat);
    }

void RotMatrix::InitTransposedFromQuaternionWXYZ
(
const double        *pQuatAsDoubleArray
)
    {
    InitFromQuaternion (pQuatAsDoubleArray);
    Transpose ();
    }




/*-----------------------------------------------------------------*//**
* Returns the angle of rotation of this instance and sets axis to be the
* normalized vector about which this instance rotates.
* NOTE: this instance is assumed to be a (rigid body, i.e. orthogonal) rotation
* matrix.
* Since negating both angle and axis produces an identical rotation,
* calculations are simplified by assuming (and returning) the angle in [0,Pi].
*
* @param [out] axis normalized axis of rotation
* @return rotation angle (in radians) between 0 and Pi, inclusive
* @bsimethod                                    DavidAssaf      6/98
+----------------------------------------------------------------------*/
double RotMatrix::GetRotationAngleAndVector (DVec3dR axis) const
    {
    DPoint4d quat;
    GetQuaternion (quat, false);
    return quat.GetRotationAngleAndVectorFromQuaternion(axis);
    }

//!
//! Returns the (0 or positive) angle from (1,0) to the XY vector in the first column.
//! @return rotation angle (in radians) between 0 and 2Pi
//!
double RotMatrix::ColumnXAngleXY () const
    {
    double theta = Angle::Atan2 (form3d[1][0], form3d[0][0]);
    if (theta < 0.0)
        theta += Angle::TwoPi ();
    return theta;
    }

/*-----------------------------------------------------------------*//**
* @description Sets this instance matrix by copying from the matrix parameter.
* @param [in] in The source matrix
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::Copy
(

RotMatrixCR in

)
    {
    *this = in;
    }


/*---------------------------------------------------------------------------------**//**
* Apply a Givens "row operation", i.e. pre-multiply by a Givens rotation matrix.
* The Givens matrix is an identity except for the 4 rotational entries, viz
*       R(i0,i0)=R(i1,i1)=c
*       R(i0,i1)=s
*       R(i1,i0)=-s
*
* @param [in] c The cosine of givens rotation.
* @param [in] s The sine of givens rotation.
* @param [in] i0 The index of the first affected row.
* @param [in] i1 The index of the second affected row.
* @param [in] matrix The input matrix
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
void RotMatrix::GivensRowOp
(
double        c,
double        s,
int           i0,
int           i1
)
    {
    int j;
    double a, b;
    i0 = Angle::Cyclic3dAxis (i0);
    i1 = Angle::Cyclic3dAxis  (i1);

    for (j = 0; j < 3; j++)
        {
        a = form3d[i0][j];
        b = form3d[i1][j];
        form3d[i0][j] = a * c - b * s;
        form3d[i1][j] = b * c + a * s;
        }
    }




/*---------------------------------------------------------------------------------**//**
* Apply a Givens "column operation", i.e. post-multiply by a Givens rotation matrix.
* The Givens matrix is an identity except for the 4 rotational entries, viz
*       R(i0,i0)=R(i1,i1)=c
*       R(i0,i1)=-s
*       R(i1,i0)=s
*
* @param [in] matrix The input matrix
* @param [in] c The cosine of givens rotation.
* @param [in] s The sine of givens rotation.
* @param [in] i0 The index of the first affected row.
* @param [in] i1 The index of the second affected row.
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
void RotMatrix::GivensColumnOp
(
double        c,
double        s,
int           i0,
int           i1
)
    {
    int i;
    double a, b;
    i0 = Angle::Cyclic3dAxis (i0);
    i1 = Angle::Cyclic3dAxis  (i1);
    for (i = 0; i < 3; i++)
        {
        a = form3d[i][i0];
        b = form3d[i][i1];
        form3d[i][i0] = a * c + b * s;
        form3d[i][i1] = b * c - a * s;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Apply a hyperbolic "row operation", i.e. pre-multiply by a hyperbolic reflection matrix
* The matrix is an identity except for the 4 entries
*       R(i0,i0)=R(i1,i1)=secant
*       R(i0,i1)=R(i1,i0)=tangent
*
* @param [in] secant The cosine of reflection.
* @param [in] tangent The sine of reflection.
* @param [in] i0 The index of the first affected row.
* @param [in] i1 The index of the second affected row.
* @param [in] matrix The input matrix
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
void RotMatrix::HyperbolicRowOp
(
double        secant,
double        tangent,
int           i0,
int           i1
)
    {
    int j;
    double a, b;
    i0 = Angle::Cyclic3dAxis (i0);
    i1 = Angle::Cyclic3dAxis  (i1);

    for (j = 0; j < 3; j++)
        {
        a = form3d[i0][j];
        b = form3d[i1][j];
        form3d[i0][j] = a * secant + b * tangent;
        form3d[i1][j] = b * secant + a * tangent;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Apply a hyperbolic "column operation", i.e. pre-multiply by a hyperbolic reflection matrix
* The matrix is an identity except for the 4 entries
*       R(i0,i0)=R(i1,i1)=secant
*       R(i0,i1)=R(i1,i0)=tangent
*
* @param [in] secant The cosine of reflection.
* @param [in] tangent The sine of reflection.
* @param [in] i0 The index of the first affected row.
* @param [in] i1 The index of the second affected row.
* @param [in] matrix The input matrix
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
void RotMatrix::HyperbolicColumnOp
(
double        secant,
double        tangent,
int           i0,
int           i1
)
    {
    int i;
    double a, b;
    i0 = Angle::Cyclic3dAxis (i0);
    i1 = Angle::Cyclic3dAxis  (i1);

    for (i = 0; i < 3; i++)
        {
        a = form3d[i][i0];
        b = form3d[i][i1];
        form3d[i][i0] = a * secant + b * tangent;
        form3d[i][i1] = b * secant + a * tangent;
        }
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static double rotate
(
DVec3dR U,
DVec3dR V,
DVec3dR QU,
DVec3dR QV
)
    {
    double c, s;
    DVec3d Q, R;
    bsiTrig_constructOneSided3DJacobiRotation (&c, &s, &U, &V);
    Q.SumOf (U, c, V, s);
    R.SumOf (U, -s, V, c);
    U = Q;
    V = R;

    Q.SumOf (QU, c, QV, s);
    R.SumOf (QU, -s, QV, c);
    QU = Q;
    QV = R;
#ifdef CheckOrthogonalization
    double uu = U.DotProduct (U);
    double uv = U.DotProduct (V);
    double vv = V.DotProduct (V);
#endif
    return fabs (s);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool RotMatrix::FactorOrthogonalColumns
(
RotMatrixR matrixB,
RotMatrixR matrixV
) const
    {
    DVec3d U, V, W;
    GetColumns (U, V, W);
    DVec3d QU, QV, QW;
    QU = DVec3d::From (1.0, 0.0, 0.0);
    QV = DVec3d::From (0.0, 1.0, 0.0);
    QW = DVec3d::From (0.0, 0.0, 1.0);
    bool    boolstat = false;
    int i;
    static int maxIteration = 10;
    static double s_sineTol = 1.0e-14;;
    double s;
    for (i = 0; i < maxIteration; i++)
        {
        s  = rotate (U, V, QU, QV);
        s += rotate (V, W, QV, QW);
        s += rotate (U, W, QU, QW);
        if (s <= s_sineTol)
            {
#ifdef CheckOrthogonalization
            CheckBV (U, V, W, QU, QV, QW, pMatrixA);
#endif
            boolstat = true;
            break;
            }
        }

    matrixB.InitFromColumnVectors (U, V, W);
    matrixV.InitFromColumnVectors (QU, QV, QW);
    return boolstat;
    }



/*-----------------------------------------------------------------*//**
* @description Solves the matrix equation AX=B, where A is this instance, B is the matrix
* of numPoints input points and X is the matrix of numPoints output points.
* point and result may have identical addresses.
*
* @param [out] result column points of solution matrix to system
* @param [in] point The column points of constant matrix of system
* @param [in] numPoints The number of input/output points
* @return false if this instance is singular.
* @bsimethod                                                    DavidAssaf      1/99
+----------------------------------------------------------------------*/
bool RotMatrix::SolveArray
(

DPoint3dP result,
DPoint3dCP point,
      int           numPoints

) const
    {
    RotMatrix inverse;
    if (inverse.InverseOf (*this))
        {
        inverse.Multiply (result, point, numPoints);
        return true;
        }
    for (int i = 0; i < numPoints; i++)
        result[i] = point[i];
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
int RotMatrix::FactorRotateScaleRotate
(
RotMatrixR  rotation1,
DPoint3dR   scalePoint,
RotMatrixR  rotation2
) const
    {
    RotMatrix BB, VV;
    int rank = 0;

    FactorOrthogonalColumns (BB, VV);

    DVec3d column[3];
    BB.GetColumns (column[0], column[1], column[2]);

    scalePoint.x = column[0].Magnitude ();
    scalePoint.y = column[1].Magnitude ();
    scalePoint.z = column[2].Magnitude ();

    // !! We know that the orthogonalizer is left heavy ...
    double det = BB.Determinant ();

    if (det < 0.0)
        scalePoint.z = - scalePoint.z;

    if (scalePoint.z != 0.0)
        {
        // Just scale and be gone.
        rotation1.ScaleColumns (BB, 1.0 / scalePoint.x, 1.0 / scalePoint.y, 1.0 / scalePoint.z);
        rank = 3;
        }
    else if (scalePoint.y != 0.0)
        {
        column[0].Scale (1.0 / scalePoint.x);
        column[1].Scale (1.0 / scalePoint.y);
        column[2].NormalizedCrossProduct (column[0], column[1]);
        rotation1.InitFromColumnVectors (column[0], column[1], column[2]);
        rank = 2;
        }
    else if (scalePoint.x != 0.0)
        {
        column[0].Scale (1.0 / scalePoint.x);
        column[0].GetNormalizedTriad (column[1], column[2], column[0]);
        rotation1.InitFromColumnVectors (column[0], column[1], column[2]);
        rank = 1;
        }
    else
        {
        rotation1.InitIdentity ();
        rank = 0;
        }
    rotation2.TransposeOf (VV);
    return rank;
    }

//! @description Returns a (rotation) that is a right-handed signed permutation of axes.
//!<ul>
//!<li>The transform is described by directing the local u and v axes along positive or negative direction of any combination 
//!     of global axes.
//!<li>(0,1,0,1) creates an identity -- u along positive x, v along positive y.
//!<li>)0,1,2,1) points u axis along x, v axis along negative z.  {w=u cross v} is positive y.
//!<li>if the uAxisId and vAxisId are identical, the result is invalid (but will have u along that direction, v along the cyclic successor)
//!<ul>
//! @param [in] uAxisId the id (0,1,2) of the axis where the local u column points.
//! @param [in] uSign Any positive number to point the u column in the forward direction of uAxisId, negative to point backward.
//! @param [in] vAxisId the id (0,1,2) of the axis where the local v column points.
//! @param [in] ySign Any positive number to point the v column in the forward direction of xAxisId, negative to point backward.
ValidatedRotMatrix RotMatrix::FromPrimaryAxisDirections
(
int uAxisId,
int uAxisSign,
int vAxisId,
int vAxisSign
)
    {
    uAxisId = Angle::Cyclic3dAxis (uAxisId);
    vAxisId = Angle::Cyclic3dAxis (vAxisId);
    bool valid = true;
    if (uAxisId == vAxisId)
        {
        valid = false;
        vAxisId = Angle::Cyclic3dAxis (uAxisId + 1);
        }
    double u = uAxisSign >= 0.0 ? 1.0 : -1.0;
    double v = vAxisSign >= 0.0 ? 1.0 : -1.0;
    DVec3d vectorU, vectorV;
    vectorU.Zero ();
    vectorV.Zero ();
    vectorU.SetComponent (u, uAxisId);
    vectorV.SetComponent (v, vAxisId);
    DVec3d vectorW = DVec3d::FromCrossProduct (vectorU, vectorV);
    return ValidatedRotMatrix (RotMatrix::FromColumnVectors (vectorU, vectorV, vectorW), valid);
    }

END_BENTLEY_NAMESPACE
