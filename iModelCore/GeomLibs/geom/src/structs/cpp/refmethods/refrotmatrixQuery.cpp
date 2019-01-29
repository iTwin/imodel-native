/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/cpp/refmethods/refrotmatrixQuery.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE


/*-----------------------------------------------------------------*//**
* @description Copies from columns of this instance matrix to corresponding points.
* @param [out] vec3dX first column
* @param [out] vec3dY second column
* @param [out] vec3dZ third column
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::GetColumns
(

DVec3dR vec3dX,
DVec3dR vec3dY,
DVec3dR vec3dZ

) const
    {
    vec3dX.x = form3d[0][0];
    vec3dX.y = form3d[1][0];
    vec3dX.z = form3d[2][0];

    vec3dY.x = form3d[0][1];
    vec3dY.y = form3d[1][1];
    vec3dY.z = form3d[2][1];

    vec3dZ.x = form3d[0][2];
    vec3dZ.y = form3d[1][2];
    vec3dZ.z = form3d[2][2];
    }


/*-----------------------------------------------------------------*//**
* @description Copies from rows of this instance matrix to corresponding points.
* @param [out] vec3dX first row
* @param [out] vec3dY second row
* @param [out] vec3dZ third row
* @bsimethod                                                    DavidAssaf      01/99
+----------------------------------------------------------------------*/
void RotMatrix::GetRows
(
DVec3dR vec3dX,
DVec3dR vec3dY,
DVec3dR vec3dZ

) const
    {
    vec3dX.x = form3d[0][0];
    vec3dX.y = form3d[0][1];
    vec3dX.z = form3d[0][2];

    vec3dY.x = form3d[1][0];
    vec3dY.y = form3d[1][1];
    vec3dY.z = form3d[1][2];

    vec3dZ.x = form3d[2][0];
    vec3dZ.y = form3d[2][1];
    vec3dZ.z = form3d[2][2];
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RotMatrix::GetRowValues
(
double          &x00,
double          &x01,
double          &x02,
double          &x10,
double          &x11,
double          &x12,
double          &x20,
double          &x21,
double          &x22
) const
    {
    x00 = form3d[0][0];
    x01 = form3d[0][1];
    x02 = form3d[0][2];

    x10 = form3d[1][0];
    x11 = form3d[1][1];
    x12 = form3d[1][2];

    x20 = form3d[2][0];
    x21 = form3d[2][1];
    x22 = form3d[2][2];
    }





/*-----------------------------------------------------------------*//**
* @description Set the components in a column.
* @param [in] point new values
* @param [in] col The index of column to change. Column indices are 0, 1, 2.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::SetColumn
(

DVec3dCR point,
int      col

)
    {
    col = Angle::Cyclic3dAxis (col);
    form3d[0][col] = point.x;
    form3d[1][col] = point.y;
    form3d[2][col] = point.z;
    }

void RotMatrix::SetColumn
(
int      col,
double x,
double y,
double z
)
    {
    col = Angle::Cyclic3dAxis (col);
    form3d[0][col] = x;
    form3d[1][col] = y;
    form3d[2][col] = z;
    }

/*-----------------------------------------------------------------*//**
* @description Set the components in a row.
* @param [in] point new values
* @param [in] row The index of row to change. Row indices are 0, 1, 2.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::SetRow
(

DVec3dCR point,
int      row

)
    {
    row = Angle::Cyclic3dAxis (row);

    if (row == 0)
        {
        form3d[0][0] = point.x;
        form3d[0][1] = point.y;
        form3d[0][2] = point.z;
        }
    else if (row == 1)
        {
        form3d[1][0] = point.x;
        form3d[1][1] = point.y;
        form3d[1][2] = point.z;
        }
    else if (row == 2)
        {
        form3d[2][0] = point.x;
        form3d[2][1] = point.y;
        form3d[2][2] = point.z;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Returns a point taken from a column of a matrix.
* @param [out] point filled point
* @param [in] col The index of column to extract. Column indices are 0, 1, 2.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::GetColumn
(

DVec3dR point,
int     col

) const
    {
    col = Angle::Cyclic3dAxis (col);
    point.x = form3d[0][col];
    point.y = form3d[1][col];
    point.z = form3d[2][col];
    }


/*-----------------------------------------------------------------*//**
* @description Returns a value from a specified row and column of the matrix.
* @param [in] row The index of row to read. Row indices are 0, 1, 2.
* @param [in] col The index of column to read.  Column indices are 0, 1, 2.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double RotMatrix::GetComponentByRowAndColumn
(

int  row,
int  col

) const
    {
    double value;
    row = Angle::Cyclic3dAxis (row);
    col = Angle::Cyclic3dAxis (col);

    value = form3d[row][col];
    return value;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a value from a specified row and column of the matrix.
* @param [in] row The index of row to read. Row indices are 0, 1, 2.
* @param [in] col The index of column to read.  Column indices are 0, 1, 2.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::SetComponentByRowAndColumn
(
int  row,
int  col,
double value
)
    {
    row = Angle::Cyclic3dAxis (row);
    col = Angle::Cyclic3dAxis (col);

    form3d[row][col] = value;
    }


/*-----------------------------------------------------------------*//**
* @bsimethod                                EarlinLutz      09/12
+----------------------------------------------------------------------*/
void RotMatrix::GetRowValuesXY
(
double data[4]
) const
    {
    data[0] = form3d[0][0];
    data[1] = form3d[0][1];
    data[2] = form3d[1][0];
    data[3] = form3d[1][1];
    }
/*-----------------------------------------------------------------*//**
* @description Returns a vector taken from a column of a matrix.
* @param [out] point filled point
* @param [in] row The index of row to extract.  Row indices are 0, 1, and 2.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::GetRow
(

DVec3dR point,
int     row

) const
    {
    row = Angle::Cyclic3dAxis (row);

    if ( row == 0)
        {
        point.x = form3d[0][0];
        point.y = form3d[0][1];
        point.z = form3d[0][2];
        }
    else if (row == 1)
        {
        point.x = form3d[1][0];
        point.y = form3d[1][1];
        point.z = form3d[1][2];
        }
    else if (row == 2)
        {
        point.x = form3d[2][0];
        point.y = form3d[2][1];
        point.z = form3d[2][2];
        }
    }



/*-----------------------------------------------------------------*//**
* @description Tests for equality between two matrices
* "Equality" means relative error less than 1.0e-12, in the sense that each
* component-wise difference is less than 1.0e-12 times the largest absolute
* value of the components of one matrix.
*
* @param [in] matrix2 The second matrix
* @return   true if the matrices are identical.
* @bsimethod                                                    DavidAssaf      6/98
+----------------------------------------------------------------------*/
bool RotMatrix::IsEqual
(

RotMatrixCR matrix2

) const
    {
    double tol = 1.0e-12;
    bool result;

    result = (this->MaxDiff (matrix2)
                    <= tol * this->MaxAbs () );
    
    return result;
    }


/*-----------------------------------------------------------------*//**
* @description Tests for equality between two matrices.
*
* @param [in] matrix2 The second matrix
* @param [in] tolerance The relative error tolerance.  Absolute tolerance is
*           this (relative) tolerance times the largest absolute value in Matrix1.
* @return true if the matrices are identical within tolerance.
* @bsimethod                                                    DavidAssaf      6/98
+----------------------------------------------------------------------*/
bool RotMatrix::IsEqual
(

RotMatrixCR matrix2,
double      tolerance

) const
    {
    bool result;

    result = (this->MaxDiff (matrix2)
                    <= tolerance * this->MaxAbs () );
    
    return result;
    }



/*-----------------------------------------------------------------*//**
* @description Returns {Matrix0 + Matrix1*s1+Matrix2*s2}, i.e. the sum of matrix M0,
* matrix M1 multiplied by scale s1, and matrix M2 multiplied by scale s2.
* Any combination of the matrix pointers may have identical addresses.
* Any of the input matrix pointers may be NULL.
* @param [in] matrix0 The matrix0 of formula
* @param [in] matrix1 The matrix1 of formula
* @param [in] scale1 The scale factor to apply to Matrix1
* @param [in] matrix2 The matrix2 of formula
* @param [in] scale2 The scale factor to apply to Matrix2
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::SumOf
(

RotMatrixCR matrix0,
RotMatrixCR matrix1,
double      scale1,
RotMatrixCR matrix2,
double      scale2

)
    {
    int i;
    RotMatrix sum;
    double const *pFrom;
    double *pTo;


    sum = matrix0;

    for (i = 0, pFrom = (double const*)&matrix1, pTo = (double *)&sum;
        i < 9; i++, pFrom++, pTo++)
        {
        *pTo += *pFrom * scale1;
        }

    for (i = 0, pFrom = (double const*)&matrix2, pTo = (double *)&sum;
             i < 9; i++, pFrom++, pTo++)
        {
        *pTo += *pFrom * scale2;
        }
    *this = sum;
    }


/*-----------------------------------------------------------------*//**
* @description Return the sum of squares of coefficients in a matrix.
* @return Sum of squares of all entries in matrix
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double RotMatrix::SumSquares
(

) const
    {
    int i;
    double const*pij = (double const*) this;
    double sum = 0.0;

    for (i = 0; i < 9; i++, pij++)
        {
        sum += *pij * *pij;
        }

    return  sum;
    }


/*-----------------------------------------------------------------*//**
* @description Find the largest absolute value of entries in the matrix.
* @return largest absolute value in matrix
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double RotMatrix::MaxAbs
(

) const
    {
    int i;
    double const*pij = (double const*) this;
    double maxAbs = 0.0;

    for (i = 0; i < 9; i++, pij++)
        {
        if (fabs (*pij) > maxAbs)
            maxAbs = fabs (*pij);
        }
    return  maxAbs;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the largest absolute value difference between
*   corresponding coefficients in Matrix1 and Matrix2.
* @param [in] matrix2 The matrix to compare to
* @return largest absolute difference between the two matrices.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double RotMatrix::MaxDiff
(

RotMatrixCR matrix2

) const
    {
    int i;
    double const*pA = (double const*) this;
    double const*pB = (double const*) &matrix2;
    double absDiff;
    double maxDiff = 0.0;

    for (i = 0; i < 9; i++, pA++, pB++)
        {
        absDiff = fabs (*pA - *pB);
        if (absDiff > maxDiff)
            maxDiff = absDiff;
        }
    return  maxDiff;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the sum of the squares of the diagonal entries of this instance matrix.
* @return Sum of squares of diagonal entries
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double RotMatrix::SumDiagonalSquares
(

) const
    {
    return
        form3d[0][0] * form3d[0][0] +
        form3d[1][1] * form3d[1][1] +
        form3d[2][2] * form3d[2][2]
        ;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the sum of the squares of the off-diagonal entries of this instance matrix.
* @return sum of square of off-diagonal entries of the matrix.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double RotMatrix::SumOffDiagonalSquares
(

) const
    {
    return
        form3d[1][0] * form3d[1][0] +
        form3d[2][0] * form3d[2][0] +
        form3d[0][1] * form3d[0][1] +
        form3d[2][1] * form3d[2][1] +
        form3d[0][2] * form3d[0][2] +
        form3d[1][2] * form3d[1][2]
        ;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RotMatrix::IsNearRigidScale (RotMatrixR dest, int primaryAxis, double reltol) const
    {
    dest = *this;
    
    if (IsNearSignedPermutation (dest, reltol))
        return true;
        
    RotMatrix candidate;
    candidate.SquareAndNormalizeColumns (*this, primaryAxis, primaryAxis + 1);
    if (reltol < Angle::SmallAngle ())
        reltol = Angle::SmallAngle ();
    RotMatrix product;
    product.InitProductRotMatrixTransposeRotMatrix (*this, candidate);
    double e0, e1;
    DVec3d diagonals = DVec3d::From (product.form3d[0][0], product.form3d[1][1], product.form3d[2][2]);
    double maxScale = diagonals.MaxAbs ();
    double maxScaleDiff = DoubleOps::MaxAbs (fabs (diagonals.x) - maxScale, fabs (diagonals.y) - maxScale, fabs (diagonals.z) - maxScale);
    product.OffDiagonalSignedRange (e0, e1);
    double zeroTol = reltol * maxScale;
    int ix, iy, iz;
    Angle::Cyclic3dAxes(ix, iy, iz, primaryAxis);
    DVec3d primaryVector;       // save to get exact bit copies ....
    this->GetColumn (primaryVector, ix);
    double dx = diagonals.GetComponent (ix);
    double dy = diagonals.GetComponent (iy);
    double dz = diagonals.GetComponent (iz);
    if (DoubleOps::MaxAbs (e0, e1) < zeroTol    // off diagonals are zero
        && maxScaleDiff < zeroTol               // diagonals are close
        && dx > 0.0                             // x direction preserved
        && dy > 0.0                             // y direction preserved
        )
        {
        bool mirrored = dz < 0.0;
        if (mirrored)
            {
            // SquareAndNormalize built the right handed system that goes the right way for x and y, but z is flipped.
            double scales[3] = {1.0, 1.0, 1.0};
            scales[iz] = -1.0;
            candidate.ScaleColumns (scales[0], scales[1], scales[2]);
            }
        // the candidate has unit columns.
        if (fabs (maxScale - 1.0) < zeroTol)
            {
            // use the fully normalized candidate.
            dest = candidate;
            return true;
            }
        else
            {
            // rescale the candidate to the size of the primary axis (dx);
            dest.ScaleColumns (candidate, dx, dx, dx);
            // reinstall true original primary axis .
            dest.SetColumn (primaryVector, ix);
            return true;
            }
        }
    return false;
    }



END_BENTLEY_NAMESPACE
