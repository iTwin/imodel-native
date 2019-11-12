/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "../DeprecatedFunctions.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define HDIM 4

/*-----------------------------------------------------------------*//**
* Evaluate pA*X for m*n points X arranged in a grid.
* The homogeneous coordinates of the i,j point in the grid is
*               (x0 + i, y0 + j, 0, 1)
* The returned point pGrid[i * m + j] is the xy components of the image
* of grid poitn ij AFTER normalization.
*
* @instance pA => Viewing transformation
* @param pGrid <= Array of mXn mapped, normalized points
* @param x00 => grid origin x
* @param y00 => grid origin y
* @param m => number of grid points in x direction
* @param n => number of grid points in y direction
* @param tol => relative tolerance for 0-weight tests.
                                        If 0, 1.0e-10 is used *
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_evaluateImageGrid

(
DMatrix4dCP pA,
DPoint2dP pGrid,
double x00,
double y00,
int m,
int n,
double tol
)

    {
#define DEFAULT_PERSPECTIVE_TOL 1.0e-10
    /* For efficiency:
        1 -- we never care about z in the output.
        2 -- once the origin is transformed, subsequent points are
            obtained by adding (x,y,w) from a single column of the matrix
    */
    int i,j,k;

    double  x0,  y0,  w0;
    double  xi,  yi,  wi;
    double  x,   y,   w;
    double dxi, dyi, dwi;
    double dxj, dyj, dwj;
    double wTol;

    if (tol <= 0.0)
        tol = DEFAULT_PERSPECTIVE_TOL;

    x0 =
        pA->coff[0][0] * x00 +
        pA->coff[0][1] * y00 +
        pA->coff[0][3];
    y0 =
        pA->coff[1][0] * x00 +
        pA->coff[1][1] * y00 +
        pA->coff[1][3];
    w0 =
        pA->coff[3][0] * x00 +
        pA->coff[3][1] * y00 +
        pA->coff[3][3];

    dxi = pA->coff[0][0];
    dyi = pA->coff[1][0];
    dwi = pA->coff[3][0];

    dxj = pA->coff[0][1];
    dyj = pA->coff[1][1];
    dwj = pA->coff[3][1];

    if ( fabs(dwi) + fabs(dwj) < tol * fabs (w0))
        {

        /* w0 is strictly nonzero -- we can divide */
        if (fabs(w0 - 1.0) > tol)
            {
            /* Scale all coordinates and steps back to w0=1 */
            x0 /= w0;
            y0 /= w0;
            dxi /= w0;
            dyi /= w0;
            dxj /= w0;
            dyj /= w0;
            }

        k = 0;
        xi = x0;
        yi = y0;

        for (j = 0; j < n; j++)
            {
            x = pGrid[k].x = xi;
            y = pGrid[k].y = yi;
            k++;
            for (i = 1; i < m; i++)
                {
                pGrid[k].x = (x += dxi);
                pGrid[k].y = (y += dyi);
                k++;
                }
            xi += dxj;
            yi += dyj;
            }
        }
    else
        {
        /* w0 varies -- we have to increment it and do the *&^*&^ division */
        k = 0;
        /* Use the overall variation of w to estimate a near-zero tolerance */
        wTol = tol * (fabs (w0) + m * fabs (dwi) + n * fabs (dwj) );
        xi = x0;
        yi = y0;
        wi = w0;
        for (j = 0; j < n; j++)
            {
            x = xi;
            y = yi;
            w = wi;
            if (w < wTol)
                    return false;

            pGrid[k].x = x / w;
            pGrid[k].y = y / w;
            k++;
            for (i = 1; i < m; i++)
                {
                x += dxi;
                y += dyi;
                w += dwi;
                if (w < wTol)
                    return false;
                pGrid[k].x = x / w;
                pGrid[k].y = y / w;
                k++;
                }
            xi += dxj;
            yi += dyj;
            wi += dwj;
            }
        }
    return true;
    }

/*-----------------------------------------------------------------*//**
* Compute eigenvectors, assuming A is symmetric.
* @param pQ <= orthogonal, unit eigenvectors.
* @param pD <= corresponding eigenvalues.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_symmetricEigenvectors

(
DMatrix4dCP pA,
DMatrix4dP pQ,
DPoint4dP pD
)
    {
    DMatrix4d A = *pA;
    DMatrix4d Q;
    DPoint4d D, B, Z;
    bsiLinAlg_symmetricJacobiEigensystem
                    (
                    (double*)&A,
                    (double*)&Q,
                    (double*)&D,
                    (double*)&B,
                    (double*)&Z,
                    4
                    );
    if (pQ)
        *pQ = Q;
    if (pD)
        *pD = D;
    }
/*-----------------------------------------------------------------*//**
* @param pMatrixA => matrix containing first column
* @param columnA => column index within matrix A
* @param pMatrixB => matrix containing second column
* @param columnB => column index within matrix B
* @param firstRow => row where partial column data begins
* @param lastRowPlus1 => 1 more than the last row to use
* @see
* @return dot product of the trailing part of the indicated columns
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double bsiDMatrix4d_partialColumnDotProduct

(
DMatrix4dCP      pMatrixA,
int                 columnA,
DMatrix4dCP      pMatrixB,
int                 columnB,
int                 firstRow,
int                 lastRowPlus1
)
    {
    double dot = 0.0;
    const double *pA;
    const double *pB;
    int n;
    pA = &pMatrixA->coff[firstRow][columnA];
    pB = &pMatrixB->coff[firstRow][columnB];
    for (n = lastRowPlus1 - firstRow; n--; pA += HDIM, pB += HDIM)
        {
        dot += *pA * *pB;
        }
    return dot;
    }


/*-----------------------------------------------------------------*//**
* Subtract a scaled multiple of a (partial) column of B to a (partial)
*   column of A.
* A[*] = factor * B[*]
* where ** ranges over the respective column entries.

* @param pMatrixA <=> matrix being updated
* @param columnA => affected column of pMatrixA
* @param factor => scale factor
* @param pMatrixB => added matrix
* @param columnB => source column in B
* @param firstRow => first row of updated
* @param lastRowPlus1 => 1 more than the last row of update
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_partialColumnUpdate

(
DMatrix4dP    pMatrixA,
int         columnA,
double      factor,
DMatrix4dCP    pMatrixB,
int         columnB,
int         firstRow,
int         lastRowPlus1
)
    {
    double *pA;
    const double *pB;
    int n;
    pA = &pMatrixA->coff[firstRow][columnA];
    pB = &pMatrixB->coff[firstRow][columnB];
    for (n = lastRowPlus1 - firstRow; n--; pA += HDIM, pB += HDIM)
        {
        *pA -= factor * *pB;
        }
    }

/*-----------------------------------------------------------------*//**
* Construct a unitary reflection (Householder) transform defined
* by the reflection vector.

* @param pMatrixA <= matrix where reflection vector is placed
* @param columnA => column where vector is placed
* @param pMatrixB <=> matrix containing original vector.
                                        The original vector is replaced by
                                        the result of applying the transform,
                                        i.e. +-a 0 0 ... where a is the
                                        length of the original
* @param columnB => column of B containing original vector
* @param firstRow => first row of vectors
* @param lastRowPlus1 => 1 more than the last row of vectors
* @see
* @return true if vector was computed
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_constructPartialColumnReflection

(
DMatrix4dP    pMatrixA,
int         columnA,
DMatrix4dP    pMatrixB,
int         columnB,
int         firstRow,
int         lastRowPlus1
)
    {
    //double dot = 0.0;
    double *pA, *pA0;
    double *pB, *pB0;
    double mag2, temp, mag;
    int n, nSave;
    double scale;
    nSave = lastRowPlus1 - firstRow;
    pA0 = &pMatrixA->coff[firstRow][columnA];
    pB0 = &pMatrixB->coff[firstRow][columnB];
    /* Copy the vector and accumulate the magnitude */
    mag2 = 0.0;
    pB = pB0;
    pA = pA0;
    for (n = nSave; n--; pA += HDIM, pB += HDIM)
        {
        temp = *pA = *pB;
        *pB = 0.0;
        mag2 += temp * temp;
        }
    mag = sqrt (mag2);
    if (mag2 == 0.0)
        {
        return false;
        }
    if (*pA0 < 0.0)
        mag = -mag;
    *pB0 = -mag;
    *pA0 += mag;
    scale = 1.0 / sqrt (mag * *pA0);

    for (pA = pA0, n = nSave; n--; pA += HDIM)
        {
        *pA *= scale;
        }
    return true;
    }


/*-----------------------------------------------------------------*//**
* Factor a matrix by Householder reflections

* @param pMatrixQ <= Matrix of reflection vectors
* @param pMatrixR <= Upper triangular factor
* @param pMatrixA => Matrix to factor
* @see
* @return true if invertible
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_factorQR

(
DMatrix4dP    pMatrixQ,
DMatrix4dP    pMatrixR,
DMatrix4dCP    pMatrixA
)

    {
    int pivot,j;
    double dot;
    *pMatrixR = *pMatrixA;
    memset (pMatrixQ, 0, sizeof (DMatrix4d));
    for (pivot = 0; pivot < HDIM - 1; pivot++)
        {
        if (!bsiDMatrix4d_constructPartialColumnReflection(
                        pMatrixQ, pivot, pMatrixR, pivot, pivot, HDIM))
            return false;
        for (j = pivot + 1; j < HDIM; j++)
            {
            dot = bsiDMatrix4d_partialColumnDotProduct (
                        pMatrixQ, pivot, pMatrixR, j, pivot, HDIM);
            bsiDMatrix4d_partialColumnUpdate (
                        pMatrixR, j,
                        dot, pMatrixQ, pivot, pivot, HDIM);
            }
        }
    return true;
    }

/*-----------------------------------------------------------------*//**
*
* Expands a partial column reflection into its complete matrix.
*
* @param pMatrixM <= Full matrix form
* @param pMatrixQ => Compresed form
* @param colQ => column where reflection vector is found
* @param firstRow => start of vector
* @param lastRowPlus1 => 1 more than last index of vector
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_expandPartialColumnReflection

(
DMatrix4dP    pMatrixM,
DMatrix4dCP    pMatrixQ,
int         colQ,
int         firstRow,
int         lastRowPlus1
)
    {
    int i,j;
    double qi;
    const double *pQ, *pQ0;
    pMatrixM->InitIdentity ();
    pQ0 = &pMatrixQ->coff[firstRow][colQ];
    for (i = firstRow; i < lastRowPlus1; i++)
        {
        qi = pMatrixQ->coff[i][colQ];
        for (j = firstRow, pQ = pQ0; j < lastRowPlus1; j++, pQ += 4)
            pMatrixM->coff[i][j] -= qi * *pQ;
        }
    }

/*-----------------------------------------------------------------*//**
* @param pMatrixB <=> Matrix whose column colB is to be replaced
* @param colB => column to replace
* @param pMatrixR => triangular matrix
* @see
* @return true if diagonals are are nonzero
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_backSubstitute

(
DMatrix4dP    pMatrixB,
int         colB,
DMatrix4dP    pMatrixR
)
    {
    double denom;
    double a;
    int i;
    for (i = HDIM - 1; i >= 0; i--)
        {
        denom = pMatrixR->coff[i][i];
        if (denom == 0.0)
            {
            return false;
            }
        a = (pMatrixB->coff[i][colB] /= denom);
        bsiDMatrix4d_partialColumnUpdate(pMatrixB, colB, a, pMatrixR, i, 0, i);
        }
    return true;
    }


/*-----------------------------------------------------------------*//**
*
* Expand reflection vectors to full matrix form.
*
* @param pMatrixB <= Expanded matrix
* @param pMatrixQ => Matrix of reflection vectors
* @param order => 0 or positive for Q0*Q1*Q2...
                                              negative for ..Q2*Q1*Q0
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_expandReflections

(
DMatrix4dP              pMatrixB,
DMatrix4dCP        pMatrixQ,
int                     order
)
    {
    int j, pivot, pivotStep, nPivot;
    double dot;
    /* Construction process only knows how to apply Q from left, hence
        column processing order is reverse of appearance in the product.
    */

    if (order >= 0)
        {
        pivot = 2;
        pivotStep = -1;
        }
    else
        {
        pivot = 0;
        pivotStep = 1;
        }
    nPivot = 3;

    bsiDMatrix4d_expandPartialColumnReflection(pMatrixB, pMatrixQ,
                                pivot, pivot, HDIM);

    for (pivot+= pivotStep, nPivot-- ;nPivot--; pivot += pivotStep)
        {
        for (j = (order > 0 ? pivot : 0); j < HDIM; j++)
            {
            dot = bsiDMatrix4d_partialColumnDotProduct (pMatrixQ, pivot,
                        pMatrixB, j, pivot, HDIM);
            bsiDMatrix4d_partialColumnUpdate (pMatrixB, j,
                        dot, pMatrixQ, pivot, pivot, HDIM);
            }
        }
    }


/*-----------------------------------------------------------------*//**
* Invert a matrix using QR (Householder reflection) factors.
*
* @param pMatrixB <= Inverse matrix
* @param pMatrixA => Original matrix
* @see
* @return true if inversion completed normally.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_invertQR

(
DMatrix4dP    pMatrixB,
DMatrix4dCP    pMatrixA
)

    {
    int j;
    DMatrix4d matrixQ, matrixR;
    //bool    boolStat = false;
        /* The process iz..
            .. Need to find B such that
                 AB = I
            ..  Premultuply by Q0, Q1, and Q2
                Q2*Q1*Q0*A*B = Q2*Q1*Q0
            .. and the Q's were chosen so Q2*Q1*Q0*A became upper triangular R, hence
                R*B = Q2*Q1*Q0
            .. and this is directly solvable by back substition
        */

    if (bsiDMatrix4d_factorQR(&matrixQ, &matrixR, pMatrixA))
        {
        bsiDMatrix4d_expandReflections ( pMatrixB, &matrixQ, -1);
        for (j = 0; j < HDIM; j++)
            {
            bsiDMatrix4d_backSubstitute(pMatrixB, j, &matrixR);
            }
        return true;
        }
    return false;
    }







/*-----------------------------------------------------------------*//**
* Copies the double values directly into the rows of this instance.
*
* @param [in] x00 (0,0) entry of matrix (row, column)
* @param [in] x01 (0,1) entry
* @param [in] x02 (0,2) entry
* @param [in] x03 (0,3) entry
* @param [in] x10 (1,0) entry of matrix (row, column)
* @param [in] x11 (1,1) entry
* @param [in] x12 (1,2) entry
* @param [in] x13 (1,3) entry
* @param [in] x20 (2,0) entry of matrix (row, column)
* @param [in] x21 (2,1) entry
* @param [in] x22 (2,2) entry
* @param [in] x23 (2,3) entry
* @param [in] x30 (3,0) entry of matrix (row, column)
* @param [in] x31 (3,1) entry
* @param [in] x32 (3,2) entry
* @param [in] x33 (3,3) entry
+----------------------------------------------------------------------*/
DMatrix4d DMatrix4d::FromRowValues
(
double          x00,
double          x01,
double          x02,
double          x03,
double          x10,
double          x11,
double          x12,
double          x13,
double          x20,
double          x21,
double          x22,
double          x23,
double          x30,
double          x31,
double          x32,
double          x33
)
    {
    DMatrix4d matrix;
    matrix.InitFromRowValues (x00, x01, x02, x03, x10, x11, x12, x13, x20, x21, x22, x23, x30, x31, x32, x33);
    return matrix;

    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DMatrix4d DMatrix4d::From101WeightedProduct
(
TransformCR transformA,
RotMatrixCR matrixB,
TransformCR transformC
)
    {
    RotMatrix matrixA, matrixC;
    DVec3d  translationC, translationP;       // pointA is zeroed by weight entry in B!!!
    transformA.GetMatrix (matrixA);
    transformC.GetMatrix (matrixC);
    transformC.GetTranslation (translationC);
    RotMatrix matrixAB, matrixABC;
    matrixAB.InitProduct (matrixA, matrixB);
    matrixABC.InitProduct (matrixA, matrixB, matrixC);
    matrixAB.Multiply (translationP, translationC);

    return DMatrix4d::FromRowValues (
            matrixABC.form3d[0][0], matrixABC.form3d[0][1], matrixABC.form3d[0][2], translationP.x,
            matrixABC.form3d[1][0], matrixABC.form3d[1][1], matrixABC.form3d[1][2], translationP.y,
            matrixABC.form3d[2][0], matrixABC.form3d[2][1], matrixABC.form3d[2][2], translationP.z,
            0,0,0,0
            );
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DMatrix4d DMatrix4d::FromSandwichProduct
(
TransformCR     transform,
DMatrix4dCR     momentMatrix,
double          scaleFactor
)
    {
    DMatrix4d AB;   // transform * momentMatrix
    for (int i = 0; i < 3; i++)
        {
        for (int j = 0; j < 4; j++)
            {
            AB.coff[i][j] = 
                  transform.form3d[i][0] * momentMatrix.coff[0][j]
                + transform.form3d[i][1] * momentMatrix.coff[1][j]
                + transform.form3d[i][2] * momentMatrix.coff[2][j]
                + transform.form3d[i][3] * momentMatrix.coff[3][j];
            }
        }
    for (int j = 0; j < 4; j++)
        AB.coff[3][j] = momentMatrix.coff[3][j];

    DMatrix4d ABC = DMatrix4d::FromZero ();
    for (int i = 0; i < 4; i++)
        {
        for (int j = 0; j < 3; j++)
            {
            ABC.coff[i][j] = scaleFactor * (
                  AB.coff[i][0] * transform.form3d[j][0] 
                + AB.coff[i][1] * transform.form3d[j][1] 
                + AB.coff[i][2] * transform.form3d[j][2] 
                + AB.coff[i][3] * transform.form3d[j][3]);
            }
        ABC.coff[i][3] = scaleFactor * AB.coff[i][3];
        }
    return ABC;
    }





/*-----------------------------------------------------------------*//**
* Fill the scale and translate entries in an otherwise identity matrix

* @param [in] scale scale factor for each diagonal of leading 3x3 submatrix
* @param [in] translation translation vector
+----------------------------------------------------------------------*/
DMatrix4d DMatrix4d::FromScaleAndTranslation
(
DPoint3dCR      scale,
DPoint3dCR      translation
)
    {
    DMatrix4d matrix;
    matrix.InitFromScaleAndTranslation (scale, translation);
    return matrix;

    }

/*-----------------------------------------------------------------*//**
* Fill the affine part using xyz vectors for each column of the basis
* part and an xyz vector for the translation

* @param [in] col0 data for column 0 of leading 3x3 submatrix
* @param [in] col1 data for column 1 of leading 3x3 submatrix
* @param [in] col2 data for column 2 of leading 3x3 submatrix
* @param [in] translation data for translation part of matrix
+----------------------------------------------------------------------*/
DMatrix4d DMatrix4d::FromColumnVectors
(
DVec3dCR      col0,
DVec3dCR      col1,
DVec3dCR      col2,
DPoint3dCR      translation
)
    {
    DMatrix4d matrix;
    matrix.InitFromColumnVectors (col0, col1, col2, translation);
    return matrix;

    }
/*-----------------------------------------------------------------*//**
*
* Copy a RotMatrix into corresponding parts of a 4x4 matrix with
* 4th row and column both 0001.
*
* @param [in] B 3x3 part to fill
+----------------------------------------------------------------------*/
DMatrix4d DMatrix4d::From
(
RotMatrixCR     B
)
    {
    DMatrix4d matrix;
    matrix.InitFrom (B);
    return matrix;

    }

/*-----------------------------------------------------------------*//**
*
* Copy a DTransform3d into corresponding parts of a 4x4 matrix with
* 4th row 0001.
*
* @param [in] transfrom transform to copy
+----------------------------------------------------------------------*/
DMatrix4d DMatrix4d::From
(
TransformCR  transfrom
)
    {
    DMatrix4d matrix;
    matrix.InitFrom (transfrom);
    return matrix;

    }


/*-----------------------------------------------------------------*//**
* Fill a 4x4 matrix with a given translation vector and otherwise
* an identity.

* @param [in] tx x component
* @param [in] ty y component
* @param [in] tz z component
+----------------------------------------------------------------------*/
DMatrix4d DMatrix4d::FromTranslation
(
double          tx,
double          ty,
double          tz
)
    {
    DMatrix4d matrix;
    matrix.InitFromTranslation (tx, ty, tz);
    return matrix;

    }

/*-----------------------------------------------------------------*//**
* Fill a matrix with entries in the perspective row, otherwise an
* identity matrix.

* @param [in] px x component
* @param [in] py y component
* @param [in] pz z component
+----------------------------------------------------------------------*/
DMatrix4d DMatrix4d::FromPerspective
(
double          px,
double          py,
double          pz
)
    {
    DMatrix4d matrix;
    matrix.InitFromPerspective (px, py, pz);
    return matrix;

    }




/*-----------------------------------------------------------------*//**
* Copies the double values directly into the rows of this instance.
*
* @param [in] x00 (0,0) entry of matrix (row, column)
* @param [in] x01 (0,1) entry
* @param [in] x02 (0,2) entry
* @param [in] x03 (0,3) entry
* @param [in] x10 (1,0) entry of matrix (row, column)
* @param [in] x11 (1,1) entry
* @param [in] x12 (1,2) entry
* @param [in] x13 (1,3) entry
* @param [in] x20 (2,0) entry of matrix (row, column)
* @param [in] x21 (2,1) entry
* @param [in] x22 (2,2) entry
* @param [in] x23 (2,3) entry
* @param [in] x30 (3,0) entry of matrix (row, column)
* @param [in] x31 (3,1) entry
* @param [in] x32 (3,2) entry
* @param [in] x33 (3,3) entry
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::InitFromRowValues
(

      double        x00,
      double        x01,
      double        x02,
      double        x03,
      double        x10,
      double        x11,
      double        x12,
      double        x13,
      double        x20,
      double        x21,
      double        x22,
      double        x23,
      double        x30,
      double        x31,
      double        x32,
      double        x33

)
    {
    coff[0][0] = x00;   coff[0][1] = x01;   coff[0][2] = x02;   coff[0][3] = x03;
    coff[1][0] = x10;   coff[1][1] = x11;   coff[1][2] = x12;   coff[1][3] = x13;
    coff[2][0] = x20;   coff[2][1] = x21;   coff[2][2] = x22;   coff[2][3] = x23;
    coff[3][0] = x30;   coff[3][1] = x31;   coff[3][2] = x32;   coff[3][3] = x33;
    }


/*-----------------------------------------------------------------*//**
*
* Transpose a 4x4 matrix.
*
* @param [in] B original matrix
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::TransposeOf (DMatrix4dCR B)
    {
    // hmphh.  If this and B are idential (aliased), in place requires 6 swaps (18 assignments, each with indexing.), but diagonal can be left untouched.
    // If different, 16 assigments with indexing.
    // Just make a copy (which copies 16 sequential values) and do the 16 indexed assignments.
    DMatrix4d C = B;
    for (int i = 0; i < 4; i++)
        {
        for (int j = 0; j < 4; j++)
            coff[i][j] = C.coff[j][i];
        }
    }


/*-----------------------------------------------------------------*//**
*
* Matrix multiplication, using all components of both the matrix
* and the points.
*
* @param [out] outPoint Array of homogeneous products A*inPoint[i]
* @param [in] inPoint Array of homogeneous points
* @param [in] n number of points
* @see DMatrix4d#multiplyAndRenormalize
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::Multiply
(
DPoint4dP       outPoint,
DPoint4dCP      inPoint,
      int n
) const
    {
    int i;
    const DPoint4d *pCurrPoint;
    DPoint4d *pDest;
    double x,y,z,w;

    for ( i = 0 , pCurrPoint = inPoint , pDest = outPoint;
          i < n ;
          i++, pCurrPoint++, pDest++ )
        {
        x = pCurrPoint->x;
        y = pCurrPoint->y;
        z = pCurrPoint->z;
        w = pCurrPoint->w;
        pDest->x =
            coff[0][0] * x +
            coff[0][1] * y +
            coff[0][2] * z +
            coff[0][3] * w ;
        pDest->y =
            coff[1][0] * x +
            coff[1][1] * y +
            coff[1][2] * z +
            coff[1][3] * w ;
        pDest->z =
            coff[2][0] * x +
            coff[2][1] * y +
            coff[2][2] * z +
            coff[2][3] * w ;
        pDest->w =
            coff[3][0] * x +
            coff[3][1] * y +
            coff[3][2] * z +
            coff[3][3] * w ;
        }

    }


/*-----------------------------------------------------------------*//**
*
* Matrix multiplication, using all components of both the matrix
* and the points.
*
+----------------------------------------------------------------------*/
void DMatrix4d::MultiplyAndRenormalize (bvector<DPoint3d> &points) const
    {

    for (auto &xyz : points)
        {
        double x = xyz.x;
        double y = xyz.y;
        double z = xyz.z;

        double rX =
            this->coff[0][0] * x +
            this->coff[0][1] * y +
            this->coff[0][2] * z +
            this->coff[0][3];
        double rY =
            this->coff[1][0] * x +
            this->coff[1][1] * y +
            this->coff[1][2] * z +
            this->coff[1][3];
        double rZ =
            this->coff[2][0] * x +
            this->coff[2][1] * y +
            this->coff[2][2] * z +
            this->coff[2][3];
        double rW =
            this->coff[3][0] * x +
            this->coff[3][1] * y +
            this->coff[3][2] * z +
            this->coff[3][3];

        if (rW == 1.0 || rW == 0.0)
            {
            xyz.x = rX;
            xyz.y = rY;
            xyz.z = rZ;
            }
        else
            {
            double a = 1.0 / rW;
            xyz.x = rX * a;
            xyz.y = rY * a;
            xyz.z = rZ * a;
            }
        }

    }




#ifndef SmallGeomLib

/*-----------------------------------------------------------------*//**
* Compute eigenvectors, assuming A is symmetric.
* @param [out] Q orthogonal, unit eigenvectors.
* @param [out] D corresponding eigenvalues.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::SymmetricEigenvectors
(

DMatrix4dR Q,
DPoint4dR D

) const
    {
    bsiDMatrix4d_symmetricEigenvectors (this, (&Q),(&D));
    }


/*-----------------------------------------------------------------*//**
* Evaluate pA*X for m*n points X arranged in a grid.
* The homogeneous coordinates of the i,j point in the grid is
*               (x0 + i, y0 + j, 0, 1)
* The returned point grid[i * m + j] is the xy components of the image
* of grid poitn ij AFTER normalization.
*
* @param [out] grid Array of mXn mapped, normalized points
* @param [in] x00 grid origin x
* @param [in] y00 grid origin y
* @param [in] m number of grid points in x direction
* @param [in] n number of grid points in y direction
* @param [in] tol relative tolerance for 0-weight tests.
                                        If 0, 1.0e-10 is used *
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMatrix4d::EvaluateImageGrid
(

DPoint2dP       grid,
double          x00,
double          y00,
int             m,
int             n,
double          tol

) const
    {
    return bsiDMatrix4d_evaluateImageGrid (this, grid, x00, y00, m, n, tol) ? true : false;
    }

#endif

/*-----------------------------------------------------------------*//**
* Multiply an array of points by a matrix, using all components of both the matrix
* and the points.
*
* @param [out] outPoint Array of products A*pPoint[i] renormalized
* @param [in] inPoint Array of points points
* @param [in] n number of points
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::MultiplyAndRenormalize
(
DPoint3dP       pOutPoint,
DPoint3dCP      pInPoint,
int             n
) const
    {
    int i;
    const DPoint3d *sourceP;
    DPoint3d *pDest;
    double x,y,z;
    double rX, rY, rZ, rW;

    for ( i = 0 , sourceP = pInPoint , pDest = pOutPoint ;
          i < n ;
          i++, sourceP++, pDest++ )
        {
        x = sourceP->x;
        y = sourceP->y;
        z = sourceP->z;
        rX =
            this->coff[0][0] * x +
            this->coff[0][1] * y +
            this->coff[0][2] * z +
            this->coff[0][3];
        rY =
            this->coff[1][0] * x +
            this->coff[1][1] * y +
            this->coff[1][2] * z +
            this->coff[1][3];
        rZ =
            this->coff[2][0] * x +
            this->coff[2][1] * y +
            this->coff[2][2] * z +
            this->coff[2][3];
        rW =
            this->coff[3][0] * x +
            this->coff[3][1] * y +
            this->coff[3][2] * z +
            this->coff[3][3];

        if (rW == 1.0 || rW == 0.0)
            {
            pDest->x = rX;
            pDest->y = rY;
            pDest->z = rZ;
            }
        else
            {
            double a = 1.0 / rW;
            pDest->x = rX * a;
            pDest->y = rY * a;
            pDest->z = rZ * a;
            }
        }
    }
/*-----------------------------------------------------------------*//**
* Multiply an array of points by a matrix, using all components of both the matrix
* and the points.
*
* @param [out] outPoint Array of products A*pPoint[i] renormalized
* @param [in] inPoint Array of points points
* @param [in] n number of points
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::MultiplyAndRenormalize
(
DPoint3dR       outPoint,
DPoint3dCR      inPoint
) const
    {
    MultiplyAndRenormalize (&outPoint, &inPoint, 1);
    }


/*-----------------------------------------------------------------*//**
* Matrix*point multiplication, using full 4d points but assuming the
* matrix is affine, i.e. assume 0001 4th row.

* @param [out] outPoint Array of homogeneous products A*pPoint[i]
* @param [in] inPoint Array of homogeneous points
* @param [in] n number of points
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::MultiplyAffine
(

DPoint4dP outPoint,
DPoint4dCP inPoint,
int   n

) const
    {
    int i;
    const DPoint4d *pCurrPoint;
    DPoint4d *pDest;
    double x,y,z,w;

    for ( i = 0 , pCurrPoint = inPoint , pDest = outPoint ;
          i < n ;
          i++, pCurrPoint++, pDest++ )
        {
        x = pCurrPoint->x;
        y = pCurrPoint->y;
        z = pCurrPoint->z;
        w = pCurrPoint->w;
        if ( w == 1.0 )
            {
            /* Both the input and output have w=1 */
            pDest->x =
                this->coff[0][0] * x +
                this->coff[0][1] * y +
                this->coff[0][2] * z +
                this->coff[0][3] ;
            pDest->y =
                this->coff[1][0] * x +
                this->coff[1][1] * y +
                this->coff[1][2] * z +
                this->coff[1][3] ;
            pDest->z =
                this->coff[2][0] * x +
                this->coff[2][1] * y +
                this->coff[2][2] * z +
                this->coff[2][3] ;
            pDest->w = 1.0;
            }
        else
            {
            pDest->x =
                this->coff[0][0] * x +
                this->coff[0][1] * y +
                this->coff[0][2] * z +
                this->coff[0][3] * w;
            pDest->y =
                this->coff[1][0] * x +
                this->coff[1][1] * y +
                this->coff[1][2] * z +
                this->coff[1][3] * w;
            pDest->z =
                this->coff[2][0] * x +
                this->coff[2][1] * y +
                this->coff[2][2] * z +
                this->coff[2][3] * w;
            pDest->w = w;
            }
        }
    }



/*-----------------------------------------------------------------*//**
* Matrix times vector multiplication, assume 0001 4th row
*
* @param [out] pOut Destination array
* @param [in] pIn Source array
* @param [in] n number of vectors
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::MultiplyAffine
(

DPoint3dP outPoint,
DPoint3dCP inPoint,
int   n

) const
    {
    int i;
    const DPoint3d *pCurrPoint;
    DPoint3d *pDest;
    double x,y,z;

    for ( i = 0 , pCurrPoint = inPoint , pDest = outPoint ;
          i < n ;
          i++, pCurrPoint++, pDest++ )
        {
        x = pCurrPoint->x;
        y = pCurrPoint->y;
        z = pCurrPoint->z;
        pDest->x =
            this->coff[0][0] * x +
            this->coff[0][1] * y +
            this->coff[0][2] * z +
            this->coff[0][3] ;
        pDest->y =
            this->coff[1][0] * x +
            this->coff[1][1] * y +
            this->coff[1][2] * z +
            this->coff[1][3] ;
        pDest->z =
            this->coff[2][0] * x +
            this->coff[2][1] * y +
            this->coff[2][2] * z +
            this->coff[2][3] ;
        }
    }


/*------------------------------------------------------------------*//**
* Matrix times vector multiplication, assume 0001 4th row and padding
* 3d data with 0 weight.
*
* @param [out] out Destination array
* @param [in] in Source array
* @param [in] n number of vectors
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::MultiplyAffineVectors
(
DPoint3dP out,
DPoint3dCP in,
int   n
) const
    {
    int i;
    const DPoint3d *pCurr;
    DPoint3d *pDest;
    double x,y,z;

    for ( i = 0 , pCurr = in , pDest = out ;
          i < n ;
          i++, pCurr++, pDest++ )
        {
        x = pCurr->x;
        y = pCurr->y;
        z = pCurr->z;
        pDest->x =
            this->coff[0][0] * x +
            this->coff[0][1] * y +
            this->coff[0][2] * z;
        pDest->y =
            this->coff[1][0] * x +
            this->coff[1][1] * y +
            this->coff[1][2] * z;
        pDest->z =
            this->coff[2][0] * x +
            this->coff[2][1] * y +
            this->coff[2][2] * z;
        }
    }

/*------------------------------------------------------------------*//**
* ASSUME the 4x4 has 0001 in last row.
* Mutliply a transform.
* @param [out] out Destination Transform
* @param [in] in Source Transform
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::MultiplyAffine
(
TransformR out,
TransformCR in
) const
    {
    double x, y, z, w;

    double rowW[4] = {0,0,0,1};
    for (int j = 0; j < 4; j++)
        {
        x = in.form3d[0][j];
        y = in.form3d[1][j];
        z = in.form3d[2][j];
        w = rowW[j];
        out.form3d[0][j] =
            coff[0][0] * x +
            coff[0][1] * y +
            coff[0][2] * z +
            coff[0][3] * w;
        out.form3d[1][j] =
            coff[1][0] * x +
            coff[1][1] * y +
            coff[1][2] * z +
            coff[1][3] * w;
        out.form3d[2][j] =
            coff[2][0] * x +
            coff[2][1] * y +
            coff[2][2] * z +
            coff[2][3] * w;
        }
    }




/*-----------------------------------------------------------------*//**
* Matrix*point multiplication, with input points represented by
* separate DPoint3d and weight arrays.

* @param [out] hPoint Array of homogeneous products A*point[i]
* @param [in] point Array of xyz coordinates
* @param [in] pWeight weight array. If NULL, unit weight is used
* @param [in] n number of points
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::Multiply
(

DPoint4dP hPoint,
DPoint3dCP point,
const double *pWeight,
int         n

) const
    {
    int i;
    const DPoint3d *pCurrPoint;
    double w;
    if ( !pWeight )
        {
        for ( i = 0 , pCurrPoint = point ; i < n; i++, pCurrPoint++ )
            {
            hPoint[i].x =
                this->coff[0][0] * pCurrPoint->x +
                this->coff[0][1] * pCurrPoint->y +
                this->coff[0][2] * pCurrPoint->z +
                this->coff[0][3];
            hPoint[i].y =
                this->coff[1][0] * pCurrPoint->x +
                this->coff[1][1] * pCurrPoint->y +
                this->coff[1][2] * pCurrPoint->z +
                this->coff[1][3];
            hPoint[i].z =
                this->coff[2][0] * pCurrPoint->x +
                this->coff[2][1] * pCurrPoint->y +
                this->coff[2][2] * pCurrPoint->z +
                this->coff[2][3];
            hPoint[i].w =
                this->coff[3][0] * pCurrPoint->x +
                this->coff[3][1] * pCurrPoint->y +
                this->coff[3][2] * pCurrPoint->z +
                this->coff[3][3];
            }
        }
    else
        {
        for ( i = 0 , pCurrPoint = point ; i < n; i++, pCurrPoint++ )
            {
            w = pWeight[i];
            hPoint[i].x =
                this->coff[0][0] * pCurrPoint->x +
                this->coff[0][1] * pCurrPoint->y +
                this->coff[0][2] * pCurrPoint->z +
                this->coff[0][3] * w;
            hPoint[i].y =
                this->coff[1][0] * pCurrPoint->x +
                this->coff[1][1] * pCurrPoint->y +
                this->coff[1][2] * pCurrPoint->z +
                this->coff[1][3] * w;
            hPoint[i].z =
                this->coff[2][0] * pCurrPoint->x +
                this->coff[2][1] * pCurrPoint->y +
                this->coff[2][2] * pCurrPoint->z +
                this->coff[2][3] * w;
            hPoint[i].w =
                this->coff[3][0] * pCurrPoint->x +
                this->coff[3][1] * pCurrPoint->y +
                this->coff[3][2] * pCurrPoint->z +
                this->coff[3][3] * w;
            }
        }
    }


/*-----------------------------------------------------------------*//**
*
* Multiply a matrix times a single homogeneous point.
*
* @param [out] outPoint product A*P, where P is a column vector
* @param [in] inPoint column vector
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::Multiply
(

DPoint4dR outPoint,
DPoint4dCR inPoint

) const
    {
    double result[4];
    int i;
    for ( i = 0; i < 4; i++ )
        result[i] =
                this->coff[i][0] * inPoint.x
             +  this->coff[i][1] * inPoint.y
             +  this->coff[i][2] * inPoint.z
             +  this->coff[i][3] * inPoint.w;
    outPoint.x = result[0];
    outPoint.y = result[1];
    outPoint.z = result[2];
    outPoint.w = result[3];
    }

/*-----------------------------------------------------------------*//**
* Multiply a matrix times a single homogeneous point.
+----------------------------------------------------------------------*/
DPoint4d DMatrix4d::Multiply (DPoint3dCR xyz, double w) const
    {
    double result[4];
    int i;
    for ( i = 0; i < 4; i++ )
        result[i] =
                this->coff[i][0] * xyz.x
             +  this->coff[i][1] * xyz.y
             +  this->coff[i][2] * xyz.z
             +  this->coff[i][3] * w;

    DPoint4d outPoint;
    outPoint.x = result[0];
    outPoint.y = result[1];
    outPoint.z = result[2];
    outPoint.w = result[3];
    return outPoint;
    }

/*-----------------------------------------------------------------*//**
* Multiply the transformed matrix times points. (Equivalent to
* multiplying transposed points times the matrix.)

* @param [out] outPoint Array of homogeneous products A^T *pPoint[i]
* @param [in] inPoint Array of homogeneous points
* @param [in] n number of points
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::MultiplyTranspose
(
DPoint4dP outPoint,
DPoint4dCP inPoint,
      int n
) const
    {
    int i, pointIndex;
    double result[4];

    for (pointIndex = 0; pointIndex < n; pointIndex++, inPoint++, outPoint++)
        {
        for ( i = 0; i < 4; i++ )
            result[i] =
                    inPoint->x * this->coff[0][i]
                 +  inPoint->y * this->coff[1][i]
                 +  inPoint->z * this->coff[2][i]
                 +  inPoint->w * this->coff[3][i];
        outPoint->x = result[0];
        outPoint->y = result[1];
        outPoint->z = result[2];
        outPoint->w = result[3];
        }
    }


/*-----------------------------------------------------------------*//**
* Install c0, c1, c2, c3 in an indicated row of an DMatrix4d.

* @param [in] i index of row 0 <= i < 4 whose values are to be set
* @param [in] c0 column 0 value
* @param [in] c1 column 1 value
* @param [in] c2 column 2 value
* @param [in] c3 column 3 value
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::SetRow (int i, double c0, double c1, double c2, double c3)
    {
    if ( i >= 0 && i < 4 )
        {
        this->coff[i][0] = c0;
        this->coff[i][1] = c1;
        this->coff[i][2] = c2;
        this->coff[i][3] = c3;
        }
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DMatrix4d::SetRow
(
int i,
DPoint4dCR row
)
    {
    SetRow (i, row.x, row.y, row.z, row.w);
    }







/*-----------------------------------------------------------------*//**
* Install r0, r1, r2, r3 in an indicated column of an DMatrix4d.

* @param [in] i index of column 0 <= i < 4  whose values are to be set
* @param [in] r0 row 0 value
* @param [in] r1 row 1 value
* @param [in] r2 row 2 value
* @param [in] r3 row 3 value
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::SetColumn (int i, double r0, double r1, double r2, double r3)
    {
    if ( i >= 0 && i < 4 )
        {
        this->coff[0][i] = r0;
        this->coff[1][i] = r1;
        this->coff[2][i] = r2;
        this->coff[3][i] = r3;
        }
    }


/*-----------------------------------------------------------------*//**
* Install a DPoint4d in an indicated column of an DMatrix4d.

* @param [in] i index of column 0 <= i < 4  whose values are to be set
* @param [in] point column values
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::SetColumn (int i, DPoint4dCR point)
    {
    SetColumn (i, point.x, point.y, point.z, point.w);
    }


/*-----------------------------------------------------------------*//**
* Install a DPoint4d in an indicated column of an DMatrix4d.
*
* @param [in] i first column index
* @param [in] j second column index
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::SwapColumns
(

int i,
int j

)
    {
    double temp;
    i = (i & 3);
    j = (j & 3);

    for (int k = 0; k < 4; k++)
        {
        temp = this->coff[k][i];
        this->coff[k][i]= this->coff[k][j];
        this->coff[k][j]= temp;
        }
    }


/*-----------------------------------------------------------------*//**
* Install a DPoint4d in an indicated column of an DMatrix4d.
*
* @param [in] i first column index
* @param [in] j second column index
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::SwapRows (int i, int j)
    {
    i = (i & 3);
    j = (j & 3);
    double temp;
    for (int k = 0; k < 4; k++)
        {
        temp = this->coff[i][k];
        this->coff[i][k]= this->coff[j][k];
        this->coff[j][k]= temp;
        }
    }


/*-----------------------------------------------------------------*//**
* Copy data from a matrix column to a DPoint4d structure.
*
* @param [out] vec point copied from column
* @param [in] i index of column 0 <= i < 4  whose values are to be set
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::GetColumn
(
DPoint4dR vec,
      int i
) const
    {
    if ( i >= 0 && i < 4 )
        {
        vec.x = coff[0][i];
        vec.y = coff[1][i];
        vec.z = coff[2][i];
        vec.w = coff[3][i];
        }
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DMatrix4d::GetRow
(
DPoint4dR vec,
      int i
) const
    {
    if ( i >= 0 && i < 4 )
        {
        vec.x = coff[i][0];
        vec.y = coff[i][1];
        vec.z = coff[i][2];
        vec.w = coff[i][3];
        }
    }


/*-----------------------------------------------------------------*//**
* Copy data from a matrix rows to DPoint4d structures.
*
* @param [out] row0 row 0 data. 
* @param [out] row1 row 1 data. 
* @param [out] row2 row 2 data. 
* @param [out] row3 row 3 data. 
* @bsimethod                                                    EarlinLutz      01/17
+----------------------------------------------------------------------*/
void DMatrix4d::GetRows
(
DPoint4dR row0,
DPoint4dR row1,
DPoint4dR row2,
DPoint4dR row3
) const
    {
    GetRow (row0, 0);
    GetRow (row1, 1);
    GetRow (row2, 2);
    GetRow (row3, 3);
    }

/*-----------------------------------------------------------------*//**
* Copy data from a matrix columns to DPoint4d structures.
*
* @param [out] column0 column 0 data. 
* @param [out] column1 column 1 data. 
* @param [out] column2 column 2 data. 
* @param [out] column3 column 3 data. 
* @bsimethod                                                    EarlinLutz      01/17
+----------------------------------------------------------------------*/
void DMatrix4d::GetColumns
(
DPoint4dR column0,
DPoint4dR column1,
DPoint4dR column2,
DPoint4dR column3
) const
    {
    GetColumn (column0, 0);
    GetColumn (column1, 1);
    GetColumn (column2, 2);
    GetColumn (column3, 3);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      01/17
+----------------------------------------------------------------------*/
void DMatrix4d::GetColumnsXYZ
(
DVec3dR      col0,  //! [out] xyz part of column 0
DVec3dR      col1,  //! [out] xyz part of column 1
DVec3dR      col2,  //! [out] xyz part of column 2
DPoint3dR      translation  //! [out] translation xyz parts
) const
    {
    col0.x = coff[0][0];   col1.x = coff[0][1];    col2.x = coff[0][2];   translation.x = coff[0][3];
    col0.y = coff[1][0];   col1.y = coff[1][1];    col2.y = coff[1][2];   translation.y = coff[1][3];
    col0.z = coff[2][0];   col1.z = coff[2][1];    col2.z = coff[2][2];   translation.z = coff[2][3];
    }


static DMatrix4d s_identity = {{{1,0,0,0},   {0,1,0,0},  {0,0,1,0},  {0,0,0,1}}};

/*-----------------------------------------------------------------*//**
* initialize an identity matrix.

* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::InitIdentity ()
    {
    *this = s_identity;
    }


/*-----------------------------------------------------------------*//**
* Fill the scale and translate entries in an otherwise identity matrix

* @param [out] pA matrix initialized as an identity
* @param [in] scale scale factor for each diagonal of leading 3x3 submatrix
* @param [in] translation translation vector
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::InitFromScaleAndTranslation
(

DPoint3dCR scale,
DPoint3dCR translation

)
    {
    InitFromRowValues (
            scale.x, 0, 0, translation.x,
            0, scale.y, 0, translation.y,
            0, 0, scale.z, translation.z,
            0, 0, 0,       1
            );
    }


/*-----------------------------------------------------------------*//**
* Fill the affine part using xyz vectors for each column of the basis
* part and an xyz vector for the translation

* @param [in] col0 data for column 0 of leading 3x3 submatrix
* @param [in] col1 data for column 1 of leading 3x3 submatrix
* @param [in] col2 data for column 2 of leading 3x3 submatrix
* @param [in] translation data for translation part of matrix
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::InitFromColumnVectors
(
DVec3dCR col0,
DVec3dCR col1,
DVec3dCR col2,
DPoint3dCR translation

)
    {
    coff[0][0] = col0.x;    coff[0][1] = col1.x;    coff[0][2] = col2.x;    coff[0][3] = translation.x;
    coff[1][0] = col0.y;    coff[1][1] = col1.y;    coff[1][2] = col2.y;    coff[1][3] = translation.y;
    coff[2][0] = col0.z;    coff[2][1] = col1.z;    coff[2][2] = col2.z;    coff[2][3] = translation.z;
    coff[3][0] = 0.0;       coff[3][1] = 0.0;       coff[3][2] = 0.0;       coff[3][3] = 1.0;
    }


/*-----------------------------------------------------------------*//**
+----------------------------------------------------------------------*/
DMatrix4d DMatrix4d::FromRows
(
DPoint4dCR row0,
DPoint4dCR row1,
DPoint4dCR row2,
DPoint4dCR row3
)
    {
    DMatrix4d A;
    A.coff[0][0] = row0.x;    A.coff[0][1] = row0.y;    A.coff[0][2] = row0.z;    A.coff[0][3] = row0.w;
    A.coff[1][0] = row1.x;    A.coff[1][1] = row1.y;    A.coff[1][2] = row1.z;    A.coff[1][3] = row1.w;
    A.coff[2][0] = row2.x;    A.coff[2][1] = row2.y;    A.coff[2][2] = row2.z;    A.coff[2][3] = row2.w;
    A.coff[3][0] = row3.x;    A.coff[3][1] = row3.y;    A.coff[3][2] = row3.z;    A.coff[3][3] = row3.w;
    return A;
    }

/*-----------------------------------------------------------------*//**
+----------------------------------------------------------------------*/
DMatrix4d DMatrix4d::FromColumns
(
DPoint4dCR col0,
DPoint4dCR col1,
DPoint4dCR col2,
DPoint4dCR col3
)
    {
    DMatrix4d A;
    A.coff[0][0] = col0.x;    A.coff[0][1] = col1.x;    A.coff[0][2] = col2.x;    A.coff[0][3] = col3.x;
    A.coff[1][0] = col0.y;    A.coff[1][1] = col1.y;    A.coff[1][2] = col2.y;    A.coff[1][3] = col3.y;
    A.coff[2][0] = col0.z;    A.coff[2][1] = col1.z;    A.coff[2][2] = col2.z;    A.coff[2][3] = col3.z;
    A.coff[3][0] = col0.w;    A.coff[3][1] = col1.w;    A.coff[3][2] = col2.w;    A.coff[3][3] = col3.w;
    return A;
    }


/*-----------------------------------------------------------------*//**
*
* Copy a RotMatrix into corresponding parts of a 4x4 matrix with
* 4th row and column both 0001.
*
* @param [in] B 3x3 part to fill
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::InitFrom (RotMatrixCR B)
    {
    *this = s_identity;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            coff[i][j] = B.form3d[i][j];
    }


/*-----------------------------------------------------------------*//**
*
* Copy a DTransform3d into corresponding parts of a 4x4 matrix with
* 4th row 0001.
*
* @param [in] transfrom transform to copy
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::InitFrom
(
TransformCR transform
)
    {
    *this = s_identity;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            coff[i][j] = transform.form3d[i][j];
    }


/*-----------------------------------------------------------------*//**
* Fill a 4x4 matrix with a given translation vector and otherwise
* an identity.

* @param [in] tx x component
* @param [in] ty y component
* @param [in] tz z component
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::InitFromTranslation
(

double tx,
double ty,
double tz

)
    {
    *this = s_identity;
    coff[0][3] = tx;
    coff[1][3] = ty;
    coff[2][3] = tz;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      01/17
+----------------------------------------------------------------------*/
DMatrix4d DMatrix4d::FromTranspose (DMatrix4dCR B)
    {
    DMatrix4d BT;
    BT.TransposeOf (B);
    return BT;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      01/17
+----------------------------------------------------------------------*/
DMatrix4d DMatrix4d::FromSwap (int i, int j)
    {
    i = (i & 3);
    j = (j & 3);
    auto A = s_identity;

    // i==j falls through as identity
    A.coff[i][i] = 0.0;
    A.coff[j][j] = 0.0;

    A.coff[i][j] = 1.0;
    A.coff[j][i] = 1.0;

    return A;
    }

/*-----------------------------------------------------------------*//**
* Fill a matrix with entries in the perspective row, otherwise an
* identity matrix.

* @param [in] px x component
* @param [in] py y component
* @param [in] pz z component
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::InitFromPerspective
(

double px,
double py,
double pz

)
    {
    *this = s_identity;
    coff[3][0] = px;
    coff[3][1] = py;
    coff[3][2] = pz;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DMatrix4d DMatrix4d::FromZero ()
    {
    DMatrix4d result;
    memset (&result, 0, sizeof (result));
    return result;
    }



/*-----------------------------------------------------------------*//**
* Premultiply A by a matrix with sx,sy,sz,1 on diagonal, tx,ty,tz,1 in last column
* @param [in] tx 03 entry (x translate) of mutliplier.
* @param [in] ty 13 entry (y translate) of multiplier.
* @param [in] tz 23 entry (z translate) of multiplier.
* @param [in] sx 00 entry (x scale) of multiplier.
* @param [in] sy 11 entry (y scale) of multiplier.
* @param [in] sz 22 entry (z scale) of multiplier.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::PreMultiplyByTranslateAndScale
(
double tx,
double ty,
double tz,
double sx,
double sy,
double sz,
DMatrix4dCR matrixA
)
    {
    int i, j;
    double tv[3];
    double sv[3];
    DMatrix4d A = matrixA;   // local copy in case of alias
    tv[0] = tx;
    tv[1] = ty;
    tv[2] = tz;
    sv[0] = sx;
    sv[1] = sy;
    sv[2] = sz;
    for (j = 0; j < 4; j++)
        {
        for (i = 0; i < 3 ; i++)
            {
            coff[i][j]
                    = sv[i] * A.coff[i][j]
                    + tv[i] * A.coff[3][j];
            }
        }
    }


/*-----------------------------------------------------------------*//**
* Form the product of two 4x4 matrices.
* @param [in] A first matrix of product A*B
* @param [in] B second matrix of product A*B
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::InitProduct
(
DMatrix4dCR A,
DMatrix4dCR B
)
    {
    int i,j;
    DMatrix4d C = s_identity;
    for ( i = 0; i < 4; i++ )
        {
        for ( j = 0; j < 4; j++  )
            {
            C.coff[i][j] = A.coff[i][0] * B.coff[0][j]
                + A.coff[i][1] * B.coff[1][j]
                + A.coff[i][2] * B.coff[2][j]
                + A.coff[i][3] * B.coff[3][j];
            }
        }
    *this = C;
    }


/*-----------------------------------------------------------------*//**
* Form the product of three 4x4 matrices.

* @param [in] A first matrix of product A*B*C
* @param [in] B second matrix of product A*B*C
* @param [in] C third matrix of product A*B*C
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::InitProduct
(
DMatrix4dCR A,
DMatrix4dCR B,
DMatrix4dCR C
)
   {
    DMatrix4d AB, ABC;
    int i, j;
    ABC = s_identity;           // for compiler???
    for ( i = 0; i < 4; i++ )
        for ( j = 0; j < 4; j++  )
            {
            AB.coff[i][j] =   A.coff[i][0] * B.coff[0][j]
                            + A.coff[i][1] * B.coff[1][j]
                            + A.coff[i][2] * B.coff[2][j]
                            + A.coff[i][3] * B.coff[3][j];
            }

    for ( i = 0; i < 4; i++ )
        for ( j = 0; j < 4; j++  )
            {
            ABC.coff[i][j] =  AB.coff[i][0] * C.coff[0][j]
                            + AB.coff[i][1] * C.coff[1][j]
                            + AB.coff[i][2] * C.coff[2][j]
                            + AB.coff[i][3] * C.coff[3][j];
            }
    *this = ABC;
    }


/*-----------------------------------------------------------------*//**
*
* Subtract two 4x4 matrices.
*
* @param [in] A A matrix of A - B
* @param [in] B B matrix of A - B
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::DifferenceOf (DMatrix4dCR A, DMatrix4dCR B)
    {
    int i,j;
    for ( i = 0;  i < 4 ; i++ )
        for ( j = 0; j < 4; j++ )
            coff[i][j] = A.coff[i][j] - B.coff[i][j];
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DMatrix4d::MaxAbsDiff
(
DMatrix4dCR other,
double &matrixDiff,
double &colDiff,
double &rowDiff,
double &weightDiff
) const
    {
    matrixDiff = 0.0;
    colDiff = DoubleOps::MaxAbs (
        coff[0][3] - other.coff[0][3],
        coff[1][3] - other.coff[1][3],
        coff[2][3] - other.coff[2][3]
        );
    rowDiff = DoubleOps::MaxAbs (
        coff[3][0] - other.coff[3][0],
        coff[3][1] - other.coff[3][1],
        coff[3][2] - other.coff[3][2]
        );
    weightDiff = fabs (coff[3][3] - other.coff[3][3]);
    for (int i = 0; i < 3; i++)
        {
        for (int j = 0; j < 3; j++)
            {
            matrixDiff = DoubleOps::Max (matrixDiff, fabs (coff[i][j] - other.coff[i][j]));
            }
        }
    return DoubleOps::Max (matrixDiff, rowDiff, colDiff, weightDiff);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DMatrix4d::MaxAbs
(
double &matrixMax,
double &colMax,
double &rowMax,
double &weightAbs
) const
    {
    weightAbs = fabs (coff[3][3]);
    colMax = DoubleOps::MaxAbs (coff[0][3], coff[1][3], coff[2][3]);
    rowMax = DoubleOps::MaxAbs (coff[3][0], coff[3][1], coff[3][2]);
    matrixMax = 0.0;
    for (int i = 0; i < 3; i++)
        {
        for (int j = 0; j < 3; j++)
            {
            matrixMax = DoubleOps::Max (matrixMax, fabs (coff[i][j]));
            }
        }
    return DoubleOps::Max (matrixMax, rowMax, colMax, weightAbs);
    }


/*-----------------------------------------------------------------*//**
* Add a matrix (componentwise) to the instance.
*
* @param [in] pVector matrix to add
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::Add (DMatrix4dCR delta)
    {
    double const *source = &delta.coff[0][0];
    double *dest   = &coff[0][0];
    for (int i = 0; i < 16; i++)
        dest[i] += source[i];
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::Add (DMatrix4dCR delta, double scaleFactor)
    {
    double const *source = &delta.coff[0][0];
    double *dest   = &coff[0][0];
    for (int i = 0; i < 16; i++)
        dest[i] += source[i] * scaleFactor;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::Scale (double scaleFactor)
    {
    double *dest   = &coff[0][0];
    for (int i = 0; i < 16; i++)
        dest[i] *= scaleFactor;
    }



/*-----------------------------------------------------------------*//**
* Subtract a matrix (componentwise) from the instance
*
* @param [in] delta matrix to subtract
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::Subtract (DMatrix4dCR delta)
    {
    double const *source = &delta.coff[0][0];
    double *dest   = &coff[0][0];
    for (int i = 0; i < 16; i++)
        dest[i] -= source[i];
    }


/*-----------------------------------------------------------------*//**
* Factor a matrix by Householder reflections

* @param [out] pMatrixQ Matrix of reflection vectors
* @param [out] matrixR Upper triangular factor
* @param [in] matrixA Matrix to factor
* @return true if invertible
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMatrix4d::FactorAsQR (DMatrix4dR matrixR, DMatrix4dCR matrixA)
    {
    return bsiDMatrix4d_factorQR (this, (&matrixR),(&matrixA)) ? true : false;
    }


/*-----------------------------------------------------------------*//**
* Invert a matrix using QR (Householder reflection) factors.
*
* @param [out] pMatrixB Inverse matrix
* @param [in] matrixA Original matrix
* @return true if inversion completed normally.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMatrix4d::QrInverseOf
(

DMatrix4dCR matrixA

)
    {
    return bsiDMatrix4d_invertQR (this, (&matrixA)) ? true : false;
    }

void DMatrix4d::GetBlocks
(
RotMatrixR matrix,
DVec3dR row,
DVec3dR column,
double    &scalar
) const
    {
    matrix.InitFromRowValues (
         coff[0][0], coff[0][1], coff[0][2],
         coff[1][0], coff[1][1], coff[1][2],
         coff[2][0], coff[2][1], coff[2][2]);
    row.Init (coff[3][0], coff[3][1], coff[3][2]);
    column.Init (coff[0][3], coff[1][3], coff[2][3]);
    scalar = coff[3][3];
    }

/*-----------------------------------------------------------------*//**
*
* Explode a 4x4 matrix into a 3x3 matrix, two 3D vectors, and a scalar, with
* the scalar location defined by a pivot index along the diagonal.
*
* @param [out] matrix 3x3 submatrix
* @param [out] row off-diagonals in row
* @param [out] column off-diagonals in column
* @param [out] pivotValue pivot entry
* @param [in] pivot pivot index
* @return true if pivot index is valid.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMatrix4d::ExtractAroundPivot
(
RotMatrixR matrix,
DPoint3dR row,
DPoint3dR column,
      double        &pivotValue,
      int           pivot
) const
    {
    int i, j;
    bool    boolStat = false;
    int index[3];
    int i0, i1, i2;
    if (pivot >= 0 && pivot < HDIM)
        {
        pivotValue = coff[pivot][pivot];

        /* Get the index sequence for non-pivot rows and columns */
        j = 0;
        for (i = 0; i < HDIM;i++)
            {
            if (i != pivot)
                index[j++] = i;
            }

        i0 = index[0];
        i1 = index[1];
        i2 = index[2];

        row.Init (coff[pivot][i0], coff[pivot][i1], coff[pivot][i2]);
        column.Init (coff[i0][pivot], coff[i1][pivot], coff[i2][pivot]);

        for (j = 0; j < 3; j++)
            for (i = 0; i < 3; i++)
                matrix.form3d[i][j] = coff[index[i]][index[j]];
        boolStat = true;
        }
    return boolStat;
    }


/*-----------------------------------------------------------------*//**
* Compute A = Bt * D * B where B is a matrix and D is a diagonal matrix
* given as a vector.
* REMARK: This is a Very Bad Thing for numerical purposes.  Are you sure
* you can't get your result without forming this product?
*
* @param [in] sigma entries of diagonal matrix D
* @param [in] B matrix B
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DMatrix4d::InitSymmetricProduct
(

DPoint4dCR sigma,
DMatrix4dCR B

)
    {
    DMatrix4d BTD;
    double factor;
    int i,j;
    const double *pDiag = (const double *)&sigma;
    /* Form C = B^t * D */
    for (j = 0; j < HDIM; j++)
        {
        factor = pDiag[j];
        for (i = 0; i < HDIM; i++)
            BTD.coff[i][j] = factor * B.coff[j][i];
        }

    InitProduct (BTD, B);
    }


/*-----------------------------------------------------------------*//**
*
* Search the matrix for the entry with the largest absolute value.
*
* @return max absolute value entry in the matrix.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DMatrix4d::MaxAbs () const
    {
    return DoubleOps::MaxAbs (&coff[0][0], 16);
    }


/*-----------------------------------------------------------------*//**
* Tests if pInstance is an identity transform and returns the bool
* indicator.
* The matrix is considered an identity if the sum of squared components
* after subtracting 1 from the diagonals is less than a small tolerance
*
* @param => matrix to test
* @return true if matrix is approximately an identity.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMatrix4d::IsIdentity () const
    {
    double tol = 1.0e-12;
    double const *value0 = &coff[0][0];
    double const *value1 = &s_identity.coff[0][0];
    for (int i = 0; i < 16; i++)
        if (fabs (value0[i] - value1[i]) > tol)
            return false;
    return true;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMatrix4d::IsAffine () const
    {
    double tol = 1.0e-12;
    return fabs (coff[3][0]) < tol
        && fabs (coff[3][1]) < tol
        && fabs (coff[3][2]) < tol
        && fabs (coff[3][3] - 1.0) < tol;
    }


/*-----------------------------------------------------------------*//**
* Approximate solution of A*X=0, i.e. find an approximate null vector;
* A is assumed upper triangular.
* Method: Find the smallest diagonal.
*         Set that entry in X to 1.
*         Backsolve the linear system with RHS 0 and the chosen X component fixed.
* @instance pA => upper triangular matrix.
* @param pX <= approximate null space vector.
* @param pResidual <= estimate of error in A*X. (Squared residual.)
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiDMatrix4d_approximateNullVectorFromUpperTriangle

(
DMatrix4dCP pA,
DPoint4dP pX,
double        *pResidual
)
    {
    double bb[HDIM], yy[HDIM];
    DMatrix4d A;
    double xx[4];
    int i, j, iMin;

    int lastColumn = HDIM - 1;
    double a, aMin;
    aMin = fabs (pA->coff[0][0]);
    iMin = 0;
    A = *pA;
    for (i = 1; i < HDIM; i++)
        {
        a = fabs (A.coff[i][i]);
        if (a < aMin)
            {
            aMin = a;
            iMin = i;
            }
        }
    xx[iMin] = 1.0;

    memset (bb, 0, HDIM * sizeof (double));
    for (i = 0; i <= iMin; i++)
        bb[i] = -A.coff[i][iMin];

    /* pack columns forward */
    for (j = iMin + 1; j < HDIM; j++)
        {
        for (i = 0; i <= j; i++)
            A.coff[i][j-1] = A.coff[i][j];
        }

    for (i = 0; i < HDIM; i++)
        {
        A.coff[i][lastColumn] = bb[i];
        }

    /* Givens rotations to rediagonalize columns 0..2 */
    for (i = iMin; i < lastColumn; i++)
        {
        double cc, ss;
        int k = i + 1;
        Angle::ConstructGivensWeights (cc, ss, A.coff[i][i], A.coff[k][i]);
        for (j = i; j < HDIM; j++)
            {
            Angle::ApplyGivensWeights
                        (A.coff[i][j], A.coff[k][j], A.coff[i][j], A.coff[k][j], cc, ss);
            }
        }
    /*  Now.... A[HDIM][HDIM] is the residual.
        Solve the leading 3x3 with column 4 as right hand side.
        */
    for (j = lastColumn - 1; j >= 0; j--)
        {
        double yj;
        if (!DoubleOps::SafeDivide (yj, A.coff[j][lastColumn], A.coff[j][j], 0.0))
            return false;
        yy[j] = yj;
        for (i = 0; i < j; i++)
            A.coff[i][lastColumn] -= yj * A.coff[i][j];
        }

    for (i = 0; i < iMin; i++)
        xx[i] = yy[i];
    xx[iMin] = 1.0;
    for (i = iMin + 1; i < HDIM; i++)
        xx[i] = yy[i - 1];
    memcpy (pX, xx, sizeof(DPoint4d));
    *pResidual = A.coff[lastColumn][lastColumn];
    return true;
    }



/*-----------------------------------------------------------------*//**
* Approximate solution of A*X=0, i.e. find an approximate null vector;
* A is assumed upper triangular.
* Method: Find the smallest diagonal.
*         Set that entry in X to 1.
*         Backsolve the linear system with RHS 0 and the chosen X component fixed.
* @param [out] nullVector approximate null space vector.
* @param [out] residual estimate of error in A*X. (Squared residual.)
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DMatrix4d::ApproximateNullVectorForUpperTriangle
(

DPoint4dR nullVector,
      double        &residual

) const
    {
    return bsiDMatrix4d_approximateNullVectorFromUpperTriangle (this, (&nullVector),&residual) ? true : false;
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DMatrix4d::Multiply
(
GraphicsPointP  dest,
GraphicsPointCP source,
size_t          nPoint
) const
    {
    size_t i;
    DPoint4d xyzw;
    static const DPoint4d z0010 = {0,0,1,0};
    static const DPoint4d w0001 = {0,0,0,1};

    // 4x4 matrices are only "full" in perspective.
    // Before starting loop, check for standard special cases.
    // This gives 30 to 50% speedup for 2d views.
    if (0 == memcmp (&coff[3][0], &w0001, 4 * sizeof (double)))
        {
        if (0 == memcmp (&coff[2][0], &z0010, 4 * sizeof (double)))
            {
            if (   coff[0][2] == 0.0
                && coff[1][2] == 0.0)
                {
                for (i = 0; i < nPoint; i++)
                    {
                    xyzw = source[i].point;
                    dest[i].point.x =
                        coff[0][0] * xyzw.x
                      + coff[0][1] * xyzw.y
                      + coff[0][3] * xyzw.w;
                    dest[i].point.y =
                        coff[1][0] * xyzw.x
                      + coff[1][1] * xyzw.y
                      + coff[1][3] * xyzw.w;
                    dest[i].point.z  = xyzw.z;
                    dest[i].point.w  = xyzw.w;
                    }
                }
            else
                {
                for (i = 0; i < nPoint; i++)
                    {
                    xyzw = source[i].point;
                    dest[i].point.x =
                        coff[0][0] * xyzw.x
                      + coff[0][1] * xyzw.y
                      + coff[0][2] * xyzw.z
                      + coff[0][3] * xyzw.w;
                    dest[i].point.y =
                        coff[1][0] * xyzw.x
                      + coff[1][1] * xyzw.y
                      + coff[1][2] * xyzw.z
                      + coff[1][3] * xyzw.w;
                    dest[i].point.z  = xyzw.z;
                    dest[i].point.w  = xyzw.w;
                    }
                }
            }
        else
            {
            for (i = 0; i < nPoint; i++)
                {
                xyzw = source[i].point;
                dest[i].point.x =
                    coff[0][0] * xyzw.x
                  + coff[0][1] * xyzw.y
                  + coff[0][2] * xyzw.z
                  + coff[0][3] * xyzw.w;
                dest[i].point.y =
                    coff[1][0] * xyzw.x
                  + coff[1][1] * xyzw.y
                  + coff[1][2] * xyzw.z
                  + coff[1][3] * xyzw.w;
                dest[i].point.z =
                    coff[2][0] * xyzw.x
                  + coff[2][1] * xyzw.y
                  + coff[2][2] * xyzw.z
                  + coff[2][3] * xyzw.w;
                dest[i].point.w  = xyzw.w;
                }
            }
        }
    else
        {
        for (i = 0; i < nPoint; i++)
            {
            xyzw = source[i].point;
            dest[i].point.x =
                coff[0][0] * xyzw.x
              + coff[0][1] * xyzw.y
              + coff[0][2] * xyzw.z
              + coff[0][3] * xyzw.w;
            dest[i].point.y =
                coff[1][0] * xyzw.x
              + coff[1][1] * xyzw.y
              + coff[1][2] * xyzw.z
              + coff[1][3] * xyzw.w;
            dest[i].point.z =
                coff[2][0] * xyzw.x
              + coff[2][1] * xyzw.y
              + coff[2][2] * xyzw.z
              + coff[2][3] * xyzw.w;
            dest[i].point.w =
                coff[3][0] * xyzw.x
              + coff[3][1] * xyzw.y
              + coff[3][2] * xyzw.z
              + coff[3][3] * xyzw.w;
            }
        }
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DMatrix4d::MultiplyAndNormalize
(
DPoint3dP dest,
DPoint4dCP source,
size_t          nPoint
) const
    {
    size_t i;
    DPoint4d xyzw, hdest;
    static const DPoint4d z0010 = {0,0,1,0};
    static const DPoint4d w0001 = {0,0,0,1};

    // 4x4 matrices are only "full" in perspective.
    // Before starting loop, check for standard special cases.
    // This gives 30 to 50% speedup for 2d views.
    if (0 == memcmp (&coff[3][0], &w0001, 4 * sizeof (double)))
        {
        if (0 == memcmp (&coff[2][0], &z0010, 4 * sizeof (double)))
            {
            if (   coff[0][2] == 0.0
                && coff[1][2] == 0.0)
                {
                for (i = 0; i < nPoint; i++)
                    {
                    xyzw = source[i];
                    hdest.x =
                        coff[0][0] * xyzw.x
                      + coff[0][1] * xyzw.y
                      + coff[0][3] * xyzw.w;
                    hdest.y =
                        coff[1][0] * xyzw.x
                      + coff[1][1] * xyzw.y
                      + coff[1][3] * xyzw.w;
                    hdest.z  = xyzw.z;
                    hdest.w  = xyzw.w;
                    hdest.GetProjectedXYZ (dest[i]);
                    }
                }
            else
                {
                for (i = 0; i < nPoint; i++)
                    {
                    xyzw = source[i];
                    hdest.x =
                        coff[0][0] * xyzw.x
                      + coff[0][1] * xyzw.y
                      + coff[0][2] * xyzw.z
                      + coff[0][3] * xyzw.w;
                    hdest.y =
                        coff[1][0] * xyzw.x
                      + coff[1][1] * xyzw.y
                      + coff[1][2] * xyzw.z
                      + coff[1][3] * xyzw.w;
                    hdest.z  = xyzw.z;
                    hdest.w  = xyzw.w;
                    hdest.GetProjectedXYZ (dest[i]);
                    }
                }
            }
        else
            {
            for (i = 0; i < nPoint; i++)
                {
                xyzw = source[i];
                hdest.x =
                    coff[0][0] * xyzw.x
                  + coff[0][1] * xyzw.y
                  + coff[0][2] * xyzw.z
                  + coff[0][3] * xyzw.w;
                hdest.y =
                    coff[1][0] * xyzw.x
                  + coff[1][1] * xyzw.y
                  + coff[1][2] * xyzw.z
                  + coff[1][3] * xyzw.w;
                hdest.z =
                    coff[2][0] * xyzw.x
                  + coff[2][1] * xyzw.y
                  + coff[2][2] * xyzw.z
                  + coff[2][3] * xyzw.w;
                hdest.w  = xyzw.w;
                hdest.GetProjectedXYZ (dest[i]);
                }
            }
        }
    else
        {
        for (i = 0; i < nPoint; i++)
            {
            xyzw = source[i];
            hdest.x =
                coff[0][0] * xyzw.x
              + coff[0][1] * xyzw.y
              + coff[0][2] * xyzw.z
              + coff[0][3] * xyzw.w;
            hdest.y =
                coff[1][0] * xyzw.x
              + coff[1][1] * xyzw.y
              + coff[1][2] * xyzw.z
              + coff[1][3] * xyzw.w;
            hdest.z =
                coff[2][0] * xyzw.x
              + coff[2][1] * xyzw.y
              + coff[2][2] * xyzw.z
              + coff[2][3] * xyzw.w;
            hdest.w =
                coff[3][0] * xyzw.x
              + coff[3][1] * xyzw.y
              + coff[3][2] * xyzw.z
              + coff[3][3] * xyzw.w;
            hdest.GetProjectedXYZ (dest[i]);
            }
        }
    }


bool DMatrix4d::ConvertInertiaProductsToPrincipalMoments
(
double &volume,
DVec3dR centroid,
RotMatrixR axes,
DVec3dR momentTensorDiagonal
) const
    {
    DVec3d moment1Col, moment1Row;
    RotMatrix Q, S;
    GetBlocks (Q, moment1Row, moment1Col, volume);
    if (centroid.SafeDivide (moment1Col, volume))
        {
        // origin shift ....
        // Striclty Q - C*M^ - M*C^ + C*C^v
        // but C = M/v ===> Q - MM^/v - MM^/v + MM^/v = Q - MM^/v = Q - MC^
        Q.AddScaledOuterProductInPlace (moment1Col, centroid, -1.0);
        //Q.AddScaledOuterProductInPlace (centroid, moment1Col, -1.0);
        //Q.AddScaledOuterProductInPlace (centroid, centroid, volume);
        // convert products to tensor ...
        double xx = Q.form3d[0][0];
        double yy = Q.form3d[1][1];
        double zz = Q.form3d[2][2];
        S.ScaleColumns (Q, -1.0, -1.0, -1.0);
        S.form3d[0][0] = yy + zz;
        S.form3d[1][1] = xx + zz;
        S.form3d[2][2] = xx + yy;
        int numIteration;
        bsiGeom_jacobi3X3 (
                  (double*)&momentTensorDiagonal,
                  axes.form3d,
                  &numIteration,
                  S.form3d
                  );
        return true;
        }
    return false;
    }


bool DMatrix4d::ConvertInertiaProductsToPrincipalMoments
(
TransformCR localToWorld,
double &volume,
DVec3dR centroid,
RotMatrixR axes,
DVec3dR momentTensorDiagonal
) const
    {
    RotMatrix Q;
    DVec3d translation;
    localToWorld.GetMatrix (Q);
    localToWorld.GetTranslation (translation);
    Transform T1 = Transform::From (Q);
    // Do the eigenvalue step in after ONLY the rotate/scale parts.
    DMatrix4d worldProducts = DMatrix4d::FromSandwichProduct (T1, *this, fabs (Q.Determinant ()));
    bool stat = worldProducts.ConvertInertiaProductsToPrincipalMoments (volume, centroid, axes, momentTensorDiagonal);
    if (stat)
        {
        centroid.Add (translation);
        }
    return stat;
    }
 
bool DMatrix4d::ConvertInertiaProductsToPrincipalAreaMoments
(
TransformCR localToWorld,
double &area,
DVec3dR centroid,
RotMatrixR axes,
DVec3dR momentTensorDiagonal
) const
    {
    RotMatrix Q, Q0;
    DVec3d translation;
    localToWorld.GetMatrix (Q);
    localToWorld.GetTranslation (translation);
    Transform T1 = Transform::From (Q);
    double scale;
    
    if (!Q.IsRigidScale (Q0, scale))    // area integrals do not map commute with nonuniform scaling!!!
        return false;
    // Do the eigenvalue step in after ONLY the rotate/scale parts.
    DMatrix4d worldProducts = DMatrix4d::FromSandwichProduct (T1, *this, scale * scale);
    bool stat = worldProducts.ConvertInertiaProductsToPrincipalMoments (area, centroid, axes, momentTensorDiagonal);
    if (stat)
        {
        centroid.Add (translation);
        }
    return stat;
    }

bool DMatrix4d::ConvertInertiaProductsToPrincipalWireMoments
(
TransformCR localToWorld,
double &length,
DVec3dR centroid,
RotMatrixR axes,
DVec3dR momentTensorDiagonal
) const
    {
    RotMatrix Q, Q0;
    DVec3d translation;
    localToWorld.GetMatrix (Q);
    localToWorld.GetTranslation (translation);
    Transform T1 = Transform::From (Q);
    double scale;
    
    if (!Q.IsRigidScale (Q0, scale))    // wire integrals do not map commute with nonuniform scaling!!!
        return false;
    // Do the eigenvalue step in after ONLY the rotate/scale parts.
    DMatrix4d worldProducts = DMatrix4d::FromSandwichProduct (T1, *this, scale * scale);
    bool stat = worldProducts.ConvertInertiaProductsToPrincipalMoments (length, centroid, axes, momentTensorDiagonal);
    if (stat)
        {
        centroid.Add (translation);
        }
    return stat;
    }




void DMatrix4d::AddSymmetricScaledOuterProduct
(
DPoint3d xyz,
double scale
)
    {
    coff[0][0] += xyz.x * xyz.x * scale;
    coff[0][1] += xyz.x * xyz.y * scale;
    coff[0][2] += xyz.x * xyz.z * scale;

    coff[1][0] += xyz.y * xyz.x * scale;
    coff[1][1] += xyz.y * xyz.y * scale;
    coff[1][2] += xyz.y * xyz.z * scale;

    coff[2][0] += xyz.z * xyz.x * scale;
    coff[2][1] += xyz.z * xyz.y * scale;
    coff[2][2] += xyz.z * xyz.z * scale;
    
    coff[0][3] += xyz.x * scale;
    coff[1][3] += xyz.y * scale;
    coff[2][3] += xyz.z * scale;

    coff[3][0] += xyz.x * scale;
    coff[3][1] += xyz.y * scale;
    coff[3][2] += xyz.z * scale;

    coff[3][3] += scale;
    }

void DMatrix4d::AddSymmetricScaledOuterProduct
(
DPoint4dCR U,
DPoint4dCR V,
double scale
)
    {
    double const *pu = &U.x;
    double const *pv = &V.x;
    for (int i = 0; i < 4; i++)
        {
        for (int j = 0; j < 4; j++)
            {
            coff[i][j] += scale * (pu[i] * pv[j] + pv[i] * pu[j]);
            }
        }
    
    }

void DMatrix4d::AddSymmetricScaledOuterProduct
(
DPoint4dCR U,
double scale
)
    {
    double const *pu = &U.x;
    for (int i = 0; i < 4; i++)
        {
        for (int j = 0; j < 4; j++)
            {
            coff[i][j] += scale * (pu[i] * pu[j]);
            }
        }    
    }

void DMatrix4d::CopyUpperTriangleToLower ()
    {
    coff[1][0] = coff[0][1];
    coff[2][0] = coff[0][2];
    coff[3][0] = coff[0][3];

    coff[2][1] = coff[1][2];
    coff[3][1] = coff[1][3];
    coff[3][2] = coff[2][3];
    }



DMatrix4d DMatrix4d::SweepMomentProducts (DMatrix4dCR baseProducts, DVec3d sweepVector)
    {
    DPoint4d moment1;
    DMatrix4d products = DMatrix4d::FromZero ();
    DPoint4d U = DPoint4d::From (sweepVector, 0.0);    
    baseProducts.GetColumn (moment1, 3);
    double mass0 = baseProducts.coff[3][3];
    double a = sweepVector.Magnitude ();
    products.Add (baseProducts, a);
    products.AddSymmetricScaledOuterProduct (U, mass0 * a / 3.0);
    products.AddSymmetricScaledOuterProduct (moment1, U, a / 2.0);
    return products;
    }

END_BENTLEY_GEOMETRY_NAMESPACE

