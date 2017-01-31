/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dmatrix4d.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*//**
* @struct DMatrix4d
* A DMatrix3d holds a 4d linear transformation.  This is most commonly used
*   as for weighted (homogeneous) 3d viewing transformations.
* @fields
* @field double coff[4][4] coefficients in 4x4 row-major order.
* @endfields
* @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
* @class DMatrix4d
*
* A DMatrix4d contains a 4x4 matrix, such as for perspective viewing
* transformations.
*
* @author   EarlinLutz
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+===============+===============+===============+===============+======*/


/*-----------------------------------------------------------------*//**
* Copies the double values directly into the rows of this instance.
*
* @instance pInstance <= constructed transform
* @param x00 => (0,0) entry of matrix (row, column)
* @param x01 => (0,1) entry
* @param x02 => (0,2) entry
* @param x03 => (0,3) entry
* @param x10 => (1,0) entry of matrix (row, column)
* @param x11 => (1,1) entry
* @param x12 => (1,2) entry
* @param x13 => (1,3) entry
* @param x20 => (2,0) entry of matrix (row, column)
* @param x21 => (2,1) entry
* @param x22 => (2,2) entry
* @param x23 => (2,3) entry
* @param x30 => (3,0) entry of matrix (row, column)
* @param x31 => (3,1) entry
* @param x32 => (3,2) entry
* @param x33 => (3,3) entry
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_initFromRowValues

(
DMatrix4dP pInstance,
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
    pInstance->coff[0][0] = x00;
    pInstance->coff[0][1] = x01;
    pInstance->coff[0][2] = x02;
    pInstance->coff[0][3] = x03;

    pInstance->coff[1][0] = x10;
    pInstance->coff[1][1] = x11;
    pInstance->coff[1][2] = x12;
    pInstance->coff[1][3] = x13;

    pInstance->coff[2][0] = x20;
    pInstance->coff[2][1] = x21;
    pInstance->coff[2][2] = x22;
    pInstance->coff[2][3] = x23;

    pInstance->coff[3][0] = x30;
    pInstance->coff[3][1] = x31;
    pInstance->coff[3][2] = x32;
    pInstance->coff[3][3] = x33;
    }


/*-----------------------------------------------------------------*//**
*
* Transpose a 4x4 matrix.
*
* @instance pA <= transposed matrix
* @param pB => original matrix
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_transpose

(
DMatrix4dP pA,
DMatrix4dCP pB
)

    {
    int i,j;
    if ( pA == pB )
        {
        DMatrix4d C = *(DMatrix4d*)pB;
        bsiDMatrix4d_transpose(pA,&C);
        }
    else
        {
        for ( i = 0; i < 4; i++ )
            for ( j = 0; j < 4; j++ )
                {
                pA->coff[i][j] = pB->coff[j][i];
                }
        }
    }



/*-----------------------------------------------------------------*//**
*
* Matrix multiplication, using all components of both the matrix
* and the points.
*
* @instance pA => Matrix term of multiplication.
* @param pOutPoint <= Array of homogeneous products A*pInPoint[i]
* @param pInPoint => Array of homogeneous points
* @param n => number of points
* @see bsiDMatrix4d_multiplyAndRenormalize
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiply4dPoints

(
DMatrix4dCP pA,
DPoint4dP pOutPoint,
DPoint4dCP pInPoint,
int n
)
    {
    int i;
    const DPoint4d *pCurrPoint;
    DPoint4d *pDest;
    double x,y,z,w;

    for ( i = 0 , pCurrPoint = pInPoint , pDest = pOutPoint ;
          i < n ;
          i++, pCurrPoint++, pDest++ )
        {
        x = pCurrPoint->x;
        y = pCurrPoint->y;
        z = pCurrPoint->z;
        w = pCurrPoint->w;
        pDest->x =
            pA->coff[0][0] * x +
            pA->coff[0][1] * y +
            pA->coff[0][2] * z +
            pA->coff[0][3] * w ;
        pDest->y =
            pA->coff[1][0] * x +
            pA->coff[1][1] * y +
            pA->coff[1][2] * z +
            pA->coff[1][3] * w ;
        pDest->z =
            pA->coff[2][0] * x +
            pA->coff[2][1] * y +
            pA->coff[2][2] * z +
            pA->coff[2][3] * w ;
        pDest->w =
            pA->coff[3][0] * x +
            pA->coff[3][1] * y +
            pA->coff[3][2] * z +
            pA->coff[3][3] * w ;
        }

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
*
* Multiply a matrix times points.
*
* @instance pA => Matrix term of multiplication.
* @param pOutPoint <= Array of graphics points A*pInPoint[i]
* @param pInPoint => Array of graphics points
* @param n => number of points
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyGrapicsPointArray

(
DMatrix4dCP pA,
GraphicsPointP pOutPoint,
GraphicsPointCP pInPoint,
int n
)
    {
    int i;
    const GraphicsPoint *pCurrPoint;
    GraphicsPoint *pDest;
    double x,y,z,w;

    for ( i = 0 , pCurrPoint = pInPoint , pDest = pOutPoint ;
          i < n ;
          i++, pCurrPoint++, pDest++ )
        {
        x = pCurrPoint->point.x;
        y = pCurrPoint->point.y;
        z = pCurrPoint->point.z;
        w = pCurrPoint->point.w;
        pDest->point.x =
            pA->coff[0][0] * x +
            pA->coff[0][1] * y +
            pA->coff[0][2] * z +
            pA->coff[0][3] * w ;
        pDest->point.y =
            pA->coff[1][0] * x +
            pA->coff[1][1] * y +
            pA->coff[1][2] * z +
            pA->coff[1][3] * w ;
        pDest->point.z =
            pA->coff[2][0] * x +
            pA->coff[2][1] * y +
            pA->coff[2][2] * z +
            pA->coff[2][3] * w ;
        pDest->point.w =
            pA->coff[3][0] * x +
            pA->coff[3][1] * y +
            pA->coff[3][2] * z +
            pA->coff[3][3] * w ;
        }
    }


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
* Multiply an array of points by a matrix, using all components of both the matrix
* and the points.
*
* @instance pA => matrix term of product.
* @param pOutPoint <= Array of products A*pPoint[i] renormalized
* @param pInPoint => Array of points points
* @param n => number of points
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray

(
DMatrix4dCP pA,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
int n
)
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
            pA->coff[0][0] * x +
            pA->coff[0][1] * y +
            pA->coff[0][2] * z +
            pA->coff[0][3];
        rY =
            pA->coff[1][0] * x +
            pA->coff[1][1] * y +
            pA->coff[1][2] * z +
            pA->coff[1][3];
        rZ =
            pA->coff[2][0] * x +
            pA->coff[2][1] * y +
            pA->coff[2][2] * z +
            pA->coff[2][3];
        rW =
            pA->coff[3][0] * x +
            pA->coff[3][1] * y +
            pA->coff[3][2] * z +
            pA->coff[3][3];

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
* Matrix*point multiplication, using full 4d points but assuming the
* matrix is affine, i.e. assume 0001 4th row.

* @instance pA => matrix  4th row to be ignored
* @param pOutPoint <= Array of homogeneous products A*pPoint[i]
* @param pInPoint => Array of homogeneous points
* @param n => number of points
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyAffineMatrix4dPoints

(
DMatrix4dCP pA,
DPoint4dP pOutPoint,
DPoint4dCP pInPoint,
int   n
)
    {
    int i;
    const DPoint4d *pCurrPoint;
    DPoint4d *pDest;
    double x,y,z,w;

    for ( i = 0 , pCurrPoint = pInPoint , pDest = pOutPoint ;
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
                pA->coff[0][0] * x +
                pA->coff[0][1] * y +
                pA->coff[0][2] * z +
                pA->coff[0][3] ;
            pDest->y =
                pA->coff[1][0] * x +
                pA->coff[1][1] * y +
                pA->coff[1][2] * z +
                pA->coff[1][3] ;
            pDest->z =
                pA->coff[2][0] * x +
                pA->coff[2][1] * y +
                pA->coff[2][2] * z +
                pA->coff[2][3] ;
            pDest->w = 1.0;
            }
        else
            {
            pDest->x =
                pA->coff[0][0] * x +
                pA->coff[0][1] * y +
                pA->coff[0][2] * z +
                pA->coff[0][3] * w;
            pDest->y =
                pA->coff[1][0] * x +
                pA->coff[1][1] * y +
                pA->coff[1][2] * z +
                pA->coff[1][3] * w;
            pDest->z =
                pA->coff[2][0] * x +
                pA->coff[2][1] * y +
                pA->coff[2][2] * z +
                pA->coff[2][3] * w;
            pDest->w = w;
            }
        }
    }


/*-----------------------------------------------------------------*//**
* Matrix*point multiplication, using full 4d points but assuming the
* matrix is has 3D only scaling and translation.

* @instance pA => matrix.
* @param pOutPoint <= Array of homogeneous products A*pPoint[i]
* @param pInPoint => Array of homogeneous points
* @param n => number of points
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_scaleAndTranslate4dPoints

(
DMatrix4dCP pA,
DPoint4dP pOutPoint,
DPoint4dCP pInPoint,
int   n
)
    {
    int i;
    const DPoint4d *pCurrPoint;
    DPoint4d *pDest;
    double x,y,z,w;
    double sx, sy, sz, tx, ty, tz;

    sx = pA->coff[0][0];
    sy = pA->coff[1][1];
    sz = pA->coff[2][2];

    tx = pA->coff[0][3];
    ty = pA->coff[1][3];
    tz = pA->coff[2][3];

    for ( i = 0 , pCurrPoint = pInPoint , pDest = pOutPoint ;
          i < n ;
          i++, pCurrPoint++, pDest++ )
        {
        x = pCurrPoint->x;
        y = pCurrPoint->y;
        z = pCurrPoint->z;
        w = pDest->w = pCurrPoint->w;
        if ( pDest->w == 1.0 )
            {
            /* Both the input and output have w=1 */
            pDest->x = sx * x + tx;
            pDest->y = sy * y + ty;
            pDest->z = sz * z + tz;
            }
        else
            {
            pDest->x = sx * x + w * tx;
            pDest->y = sy * y + w * ty;
            pDest->z = sz * z + w * tz;
            }
        }
    }


/*-----------------------------------------------------------------*//**
* Matrix times vector multiplication, assume 0001 4th row
*
* @instance pA => matrix  4th row to be ignored
* @param pOut <= Destination array
* @param pIn => Source array
* @param n => number of vectors
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyAffineMatrix3dPoints

(
DMatrix4dCP pA,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
int   n
)
    {
    int i;
    const DPoint3d *pCurrPoint;
    DPoint3d *pDest;
    double x,y,z;

    for ( i = 0 , pCurrPoint = pInPoint , pDest = pOutPoint ;
          i < n ;
          i++, pCurrPoint++, pDest++ )
        {
        x = pCurrPoint->x;
        y = pCurrPoint->y;
        z = pCurrPoint->z;
        pDest->x =
            pA->coff[0][0] * x +
            pA->coff[0][1] * y +
            pA->coff[0][2] * z +
            pA->coff[0][3] ;
        pDest->y =
            pA->coff[1][0] * x +
            pA->coff[1][1] * y +
            pA->coff[1][2] * z +
            pA->coff[1][3] ;
        pDest->z =
            pA->coff[2][0] * x +
            pA->coff[2][1] * y +
            pA->coff[2][2] * z +
            pA->coff[2][3] ;
        }
    }


/*------------------------------------------------------------------*//**
* Matrix times vector multiplication, assume 0001 4th row and padding
* 3d data with 0 weight.
*
* @instance pA => matrix  4th row to be ignored
* @param pOut <= Destination array
* @param pIn => Source array
* @param n => number of vectors
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyAffineMatrix3dVectors

(
DMatrix4dCP pA,
DPoint3dP pOut,
DPoint3dCP pIn,
int   n
)
    {
    int i;
    const DPoint3d *pCurr;
    DPoint3d *pDest;
    double x,y,z;

    for ( i = 0 , pCurr = pIn , pDest = pOut ;
          i < n ;
          i++, pCurr++, pDest++ )
        {
        x = pCurr->x;
        y = pCurr->y;
        z = pCurr->z;
        pDest->x =
            pA->coff[0][0] * x +
            pA->coff[0][1] * y +
            pA->coff[0][2] * z;
        pDest->y =
            pA->coff[1][0] * x +
            pA->coff[1][1] * y +
            pA->coff[1][2] * z;
        pDest->z =
            pA->coff[2][0] * x +
            pA->coff[2][1] * y +
            pA->coff[2][2] * z;
        }
    }


/*-----------------------------------------------------------------*//**
* Matrix*point multiplication, using only scale and translate entries from the
* matrix.

* @instance pA => matrix.
* @param pOutPoint <= Array of products A*pPoint[i]
* @param pInPoint => Array of input points
* @param n => number of points
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_scaleAndTranslate3dPoints

(
DMatrix4dCP pA,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
int   n
)
    {
    int i;
    const DPoint3d *pCurrPoint;
    DPoint3d *pDest;
    double x,y,z;
    double sx, sy, sz, tx, ty, tz;

    sx = pA->coff[0][0];
    sy = pA->coff[1][1];
    sz = pA->coff[2][2];

    tx = pA->coff[0][3];
    ty = pA->coff[1][3];
    tz = pA->coff[2][3];

    for ( i = 0 , pCurrPoint = pInPoint , pDest = pOutPoint;
          i < n ;
          i++, pCurrPoint++, pDest++ )
        {
        x = pCurrPoint->x;
        y = pCurrPoint->y;
        z = pCurrPoint->z;
        pDest->x = sx * x + tx;
        pDest->y = sy * y + ty;
        pDest->z = sz * z + tz;
        }
    }


/*-----------------------------------------------------------------*//**
* Matrix*point multiplication, with input points represented by
* separate DPoint3d and weight arrays.

* @instance pA => matrix
* @param pHPoint <= Array of homogeneous products A*pPoint[i]
* @param pPoint => Array of xyz coordinates
* @param pWeight => weight array. If NULL, unit weight is used
* @param n => number of points
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyWeightedDPoint3dArray

(
DMatrix4dCP pA,
DPoint4dP pHPoint,
DPoint3dCP pPoint,
const double *pWeight,
int n
)
    {
    int i;
    const DPoint3d *pCurrPoint;
    double w;
    if ( !pWeight )
        {
        for ( i = 0 , pCurrPoint = pPoint ; i < n; i++, pCurrPoint++ )
            {
            pHPoint[i].x =
                pA->coff[0][0] * pCurrPoint->x +
                pA->coff[0][1] * pCurrPoint->y +
                pA->coff[0][2] * pCurrPoint->z +
                pA->coff[0][3];
            pHPoint[i].y =
                pA->coff[1][0] * pCurrPoint->x +
                pA->coff[1][1] * pCurrPoint->y +
                pA->coff[1][2] * pCurrPoint->z +
                pA->coff[1][3];
            pHPoint[i].z =
                pA->coff[2][0] * pCurrPoint->x +
                pA->coff[2][1] * pCurrPoint->y +
                pA->coff[2][2] * pCurrPoint->z +
                pA->coff[2][3];
            pHPoint[i].w =
                pA->coff[3][0] * pCurrPoint->x +
                pA->coff[3][1] * pCurrPoint->y +
                pA->coff[3][2] * pCurrPoint->z +
                pA->coff[3][3];
            }
        }
    else
        {
        for ( i = 0 , pCurrPoint = pPoint ; i < n; i++, pCurrPoint++ )
            {
            w = pWeight[i];
            pHPoint[i].x =
                pA->coff[0][0] * pCurrPoint->x +
                pA->coff[0][1] * pCurrPoint->y +
                pA->coff[0][2] * pCurrPoint->z +
                pA->coff[0][3] * w;
            pHPoint[i].y =
                pA->coff[1][0] * pCurrPoint->x +
                pA->coff[1][1] * pCurrPoint->y +
                pA->coff[1][2] * pCurrPoint->z +
                pA->coff[1][3] * w;
            pHPoint[i].z =
                pA->coff[2][0] * pCurrPoint->x +
                pA->coff[2][1] * pCurrPoint->y +
                pA->coff[2][2] * pCurrPoint->z +
                pA->coff[2][3] * w;
            pHPoint[i].w =
                pA->coff[3][0] * pCurrPoint->x +
                pA->coff[3][1] * pCurrPoint->y +
                pA->coff[3][2] * pCurrPoint->z +
                pA->coff[3][3] * w;
            }
        }
    }




/*-----------------------------------------------------------------*//**
*
* Multiply a matrix times a single homogeneous point.
*
* @instance pA => matrix
* @param pOutPoint <= product A*P, where P is a column vector
* @param pInPoint => column vector
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyMatrixPoint

(
DMatrix4dCP pA,
DPoint4dP pOutPoint,
DPoint4dCP pInPoint
)
    {
    double result[4];
    int i;
    for ( i = 0; i < 4; i++ )
        result[i] =
                pA->coff[i][0] * pInPoint->x
             +  pA->coff[i][1] * pInPoint->y
             +  pA->coff[i][2] * pInPoint->z
             +  pA->coff[i][3] * pInPoint->w;
    pOutPoint->x = result[0];
    pOutPoint->y = result[1];
    pOutPoint->z = result[2];
    pOutPoint->w = result[3];
    }



/*-----------------------------------------------------------------*//**
* Multiply the transformed matrix times points. (Equivalent to
* multiplying transposed points times the matrix.)

* @instance pA => matrix
* @param pOutPoint <= Array of homogeneous products A^T *pPoint[i]
* @param pInPoint => Array of homogeneous points
* @param n => number of points
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyTransposePoints

(
DMatrix4dCP pA,
DPoint4dP pOutPoint,
DPoint4dCP pInPoint,
int n
)
    {
    int i, pointIndex;
    double result[4];

    for (pointIndex = 0; pointIndex < n; pointIndex++, pInPoint++, pOutPoint++)
        {
        for ( i = 0; i < 4; i++ )
            result[i] =
                    pInPoint->x * pA->coff[0][i]
                 +  pInPoint->y * pA->coff[1][i]
                 +  pInPoint->z * pA->coff[2][i]
                 +  pInPoint->w * pA->coff[3][i];
        pOutPoint->x = result[0];
        pOutPoint->y = result[1];
        pOutPoint->z = result[2];
        pOutPoint->w = result[3];
        }

    }


/*-----------------------------------------------------------------*//**
* @description Transform an ellipse.
* @param pMatrix => 4x4 matrix
* @param pOutEllipse <= transformed ellipse (can be same as pInEllipse)
* @param pInEllipse => untransformed ellipse
* @group "DMatrix4d Multiplication"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyDEllipse4d

(
DMatrix4dCP pMatrix,
DEllipse4dP pOutEllipse,
DEllipse4dCP pInEllipse
)
    {
    if (pOutEllipse != pInEllipse)
        *pOutEllipse = *pInEllipse;
    bsiDMatrix4d_multiplyMatrixPoint (pMatrix, &pOutEllipse->center  , &pOutEllipse->center  );
    bsiDMatrix4d_multiplyMatrixPoint (pMatrix, &pOutEllipse->vector0 , &pOutEllipse->vector0 );
    bsiDMatrix4d_multiplyMatrixPoint (pMatrix, &pOutEllipse->vector90, &pOutEllipse->vector90);
    }


/*-----------------------------------------------------------------*//**
* Install c0, c1, c2, c3 in an indicated row of an DMatrix4d.

* @instance pA <=> Matrix whose row is being set
* @param i => index of row 0 <= i < 4 whose values are to be set
* @param c0 => column 0 value
* @param c1 => column 1 value
* @param c2 => column 2 value
* @param c3 => column 3 value
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_setRow

(
DMatrix4dP pA,
int i,
double c0,
double c1,
double c2,
double c3
)

    {
    pA->coff[i][0] = c0;
    pA->coff[i][1] = c1;
    pA->coff[i][2] = c2;
    pA->coff[i][3] = c3;
    }



/*-----------------------------------------------------------------*//**
* Install r0, r1, r2, r3 in an indicated column of an DMatrix4d.

* @instance pA <=> Matrix whose column is being set
* @param i => index of column 0 <= i < 4  whose values are to be set
* @param r0 => row 0 value
* @param r1 => row 1 value
* @param r2 => row 2 value
* @param r3 => row 3 value
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_setColumn

(
DMatrix4dP pA,
int i,
double r0,
double r1,
double r2,
double r3
)

    {
    if ( i >= 0 && i < 4 )
        {
        pA->coff[0][i] = r0;
        pA->coff[1][i] = r1;
        pA->coff[2][i] = r2;
        pA->coff[3][i] = r3;
        }
    }


/*-----------------------------------------------------------------*//**
* Install a DPoint4d in an indicated column of an DMatrix4d.

* @instance pA <=> Matrix whose column is being set
* @param i => index of column 0 <= i < 4  whose values are to be set
* @param pPoint => column values
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_setColumnDPoint4d

(
DMatrix4dP pA,
int i,
DPoint4dCP pPoint
)

    {
    if ( i >= 0 && i < 4 )
        {
        pA->coff[0][i] = pPoint->x;
        pA->coff[1][i] = pPoint->y;
        pA->coff[2][i] = pPoint->z;
        pA->coff[3][i] = pPoint->w;
        }
    }


/*-----------------------------------------------------------------*//**
* Install a DPoint4d in an indicated column of an DMatrix4d.
*
* @instance pA <=> Matrix whose colums are swapped
* @param i => first column index
* @param j => second column index
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_swapColumns

(
DMatrix4dP pA,
int i,
int j
)

    {
    double temp;
    int k;
    if ( i >= 0 && i < 4 && j >= 0 && j < 4)
        {
        for (k = 0; k < 4; k++)
            {
            temp = pA->coff[k][i];
            pA->coff[k][i]= pA->coff[k][j];
            pA->coff[k][j]= temp;
            }
        }
    }


/*-----------------------------------------------------------------*//**
* Install a DPoint4d in an indicated column of an DMatrix4d.
*
* @instance pA <=> Matrix whose colums are swapped
* @param i => first column index
* @param j => second column index
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_swapRows

(
DMatrix4dP pA,
int i,
int j
)

    {
    double temp;
    int k;
    if ( i >= 0 && i < 4 && j >= 0 && j < 4)
        {
        for (k = 0; k < 4; k++)
            {
            temp = pA->coff[i][k];
            pA->coff[i][k]= pA->coff[j][k];
            pA->coff[j][k]= temp;
            }
        }
    }


/*-----------------------------------------------------------------*//**
* Copy data from a matrix column to a DPoint4d structure.
*
* @instance pMatrix => matrix whose column is retrieved
* @param pVec <= point copied from column
* @param i => index of column 0 <= i < 4  whose values are to be set
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_getColumnDPoint4d

(
DMatrix4dCP pMatrix,
DPoint4dP pVec,
int i
)

    {
    if ( i >= 0 && i < 4 )
        {
        pVec->x = pMatrix->coff[0][i];
        pVec->y = pMatrix->coff[1][i];
        pVec->z = pMatrix->coff[2][i];
        pVec->w = pMatrix->coff[3][i];
        }
    }


/*-----------------------------------------------------------------*//**
* Copy data from a matrix rows to DPoint4d structures.
*
* @instance pMatrix => matrix whose rows are retrieved
* @param pRow0 <= row 0 data. May be NULL.
* @param pRow1 <= row 1 data. May be NULL.
* @param pRow2 <= row 2 data. May be NULL.
* @param pRow3 <= row 3 data. May be NULL.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_getRowsDPoint4d

(
DMatrix4dCP pMatrix,
DPoint4dP pRow0,
DPoint4dP pRow1,
DPoint4dP pRow2,
DPoint4dP pRow3
)

    {
    int i;
    DPoint4d *pRow[4];

    pRow[0] = pRow0;
    pRow[1] = pRow1;
    pRow[2] = pRow2;
    pRow[3] = pRow3;

    for (i = 0; i < 4; i++)
        {
        if (pRow[i])
            {
            pRow[i]->x = pMatrix->coff[i][0];
            pRow[i]->y = pMatrix->coff[i][1];
            pRow[i]->z = pMatrix->coff[i][2];
            pRow[i]->w = pMatrix->coff[i][3];
            }
        }
    }


/*-----------------------------------------------------------------*//**
* initialize an identity matrix.

* @instance pA <= matrix initialized as an identity
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_initIdentity

(
DMatrix4dP pA
)

    {
    int i,j;
    for ( i = 0;  i < 4 ; i++ )
        for ( j = 0; j < 4; j++ )
            pA->coff[i][j] = 0.0;
    pA->coff[0][0] = pA->coff[1][1] = pA->coff[2][2] = pA->coff[3][3] = 1.0;
    }


/*-----------------------------------------------------------------*//**
* Fill the affine part using xyz vectors for each row of the basis
* part and an xyz vector for the translation
*
* @instance pA <= matrix initialized as an identity
* @param pRow0 => data for row 0 of leading 3x3 submatrix
* @param pRow1 => data for row 1 of leading 3x3 submatrix
* @param pRow2 => data for row 2 of leading 3x3 submatrix
* @param pTranslation => data for translation part of matrix
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_initAffineRows

(
DMatrix4dP pA,
DPoint3dCP pRow0,
DPoint3dCP pRow1,
DPoint3dCP pRow2,
DPoint3dCP pTranslation
)
    {
    bsiDMatrix4d_setRow( pA, 0, pRow0->x, pRow0->y, pRow0->z, pTranslation->x );
    bsiDMatrix4d_setRow( pA, 1, pRow1->x, pRow1->y, pRow1->z, pTranslation->y );
    bsiDMatrix4d_setRow( pA, 2, pRow2->x, pRow2->y, pRow2->z, pTranslation->z );
    bsiDMatrix4d_setRow( pA, 3, 0.0, 0.0, 0.0, 1.0 );
    }



/*-----------------------------------------------------------------*//**
* Fill the scale and translate entries in an otherwise identity matrix

* @param pA <= matrix initialized as an identity
* @param pScale => scale factor for each diagonal of leading 3x3 submatrix
* @param pTranslation => translation vector
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_initScaleAndTranslate

(
DMatrix4dP pA,
DPoint3dCP pScale,
DPoint3dCP pTranslation
)

    {
    bsiDMatrix4d_setRow( pA, 0, pScale->x, 0.0, 0.0, pTranslation->x );
    bsiDMatrix4d_setRow( pA, 1, 0.0, pScale->y, 0.0, pTranslation->y );
    bsiDMatrix4d_setRow( pA, 2, 0.0, 0.0, pScale->z, pTranslation->z );
    bsiDMatrix4d_setRow( pA, 3, 0.0, 0.0, 0.0,       1.0           );
    }



/*-----------------------------------------------------------------*//**
* Fill the affine part using xyz vectors for each column of the basis
* part and an xyz vector for the translation

* @instance pA <= matrix initialized as an identity
* @param pCol0 => data for column 0 of leading 3x3 submatrix
* @param pCol1 => data for column 1 of leading 3x3 submatrix
* @param pCol2 => data for column 2 of leading 3x3 submatrix
* @param pTranslation => data for translation part of matrix
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_initAffineColumns

(
DMatrix4dP pA,
DPoint3dCP pCol0,
DPoint3dCP pCol1,
DPoint3dCP pCol2,
DPoint3dCP pTranslation
)

    {
    bsiDMatrix4d_setRow( pA, 0, pCol0->x, pCol1->x, pCol2->x, pTranslation->x );
    bsiDMatrix4d_setRow( pA, 1, pCol0->y, pCol1->y, pCol2->y, pTranslation->y );
    bsiDMatrix4d_setRow( pA, 2, pCol0->z, pCol1->z, pCol2->z, pTranslation->z );
    bsiDMatrix4d_setRow( pA, 3, 0.0, 0.0, 0.0, 1.0 );
    }



/*-----------------------------------------------------------------*//**
*
* Copy a RotMatrix into corresponding parts of a 4x4 matrix with
* 4th row and column both 0001.
*
* @instance pA <= matrix initialized with 0 translate, given 3x3 part
* @param pB => 3x3 part to fill
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_initFromRotMatrix

(
DMatrix4dP pA,
RotMatrixCP pB
)

    {
    bsiDMatrix4d_initFromRowValues
                (
                pA,
                pB->form3d[0][0], pB->form3d[0][1], pB->form3d[0][2], 0.0,
                pB->form3d[1][0], pB->form3d[1][1], pB->form3d[1][2], 0.0,
                pB->form3d[2][0], pB->form3d[2][1], pB->form3d[2][2], 0.0,
                0.0,              0.0,              0.0,              1.0
                );
    }



/*-----------------------------------------------------------------*//**
*
* Copy a Transform into corresponding parts of a 4x4 matrix with
* 4th row 0001.
*
* @instance pA <= matrix initialized with 0 translate, given 3x3 part
* @param pTransfrom => transform to copy
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_initFromTransform

(
DMatrix4dP pA,
TransformCP pTransform
)

    {
    int i, j;
    for (j = 0; j < 4; j++)
        {
        for (i = 0; i < 3; i++)
            pA->coff[i][j] = pTransform->form3d[i][j];
        pA->coff[3][j] = 0.0;
        }
    pA->coff[3][3] = 1.0;
    }




/*-----------------------------------------------------------------*//**
* Fill a 4x4 matrix with a given translation vector and otherwise
* an identity.

* @instance pA <= matrix initialized as pure translation
* @param tx => x component
* @param ty => y component
* @param tz => z component
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_initTranslate

(
DMatrix4dP pA,
double tx,
double ty,
double tz
)

    {
    bsiDMatrix4d_initIdentity(pA);
    pA->coff[0][3] = tx;
    pA->coff[1][3] = ty;
    pA->coff[2][3] = tz;
    }



/*-----------------------------------------------------------------*//**
* Fill a matrix with entries in the perspective row, otherwise an
* identity matrix.

* @instance pA <= matrix initialized as perspective
* @param px => x component
* @param py => y component
* @param pz => z component
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_initPerspective

(
DMatrix4dP pA,
double px,
double py,
double pz
)

    {
    bsiDMatrix4d_initIdentity(pA);
    pA->coff[3][0] = px;
    pA->coff[3][1] = py;
    pA->coff[3][2] = pz;
    }


/*-----------------------------------------------------------------*//**
*
* Apply perspective to a matrix.  This is a fast matrix product C = P * A
* where A is an arbitrary matrix, P is identity except for
* perspective effects Pzz=taper and Pwz = taper - 1
* @instance <= Matrix with z perspective applied.
* @param    taper => taper factor in perspective
* @param    pA    => pre-perpsective matrix.
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDMatrix4d_premultiplyByZTaper

(
DMatrix4dP pInstance,   /* <= matrix after applying perspective */
double  taper,                  /* => perspective taper. 1.0 flat projection (no effect)
                                    0.0 has eyepoint on z=1 plane. */
const   DMatrix4d *pA           /* => pre-persepctive matrix */
)

    {
    double a32 = taper - 1.0;
    static double tol = 1.0e-12;
    int j;
    /* Filter out the flat projection case. */
    if  (a32 < tol)
        {
        /* Avoid the copy if the operation is in place */
        if  (pInstance != pA)
            *pInstance = *pA;
        }
    else
        {
        /* First two rows are unchanged.  Rows 3 and 4 are multiplied by
            [f    0]
            [f-1  1]
           Replace row 3 first, then row 2 so it works for in-place transformation. */
        for (j = 0; j < 4; j++)
            {
            pInstance->coff[3][j] += a32 * pA->coff[2][2];
            pInstance->coff[2][j] *= taper;
            }
        }
    }


/*-----------------------------------------------------------------*//**
* Premultiply A by a matrix with sx,sy,sz,1 on diagonal, tx,ty,tz,1 in last column
* @param tx => 03 entry (x translate) of mutliplier.
* @param ty => 13 entry (y translate) of multiplier.
* @param tz => 23 entry (z translate) of multiplier.
* @param sx => 00 entry (x scale) of multiplier.
* @param sy => 11 entry (y scale) of multiplier.
* @param sz => 22 entry (z scale) of multiplier.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDMatrix4d_premultiplyByScaleAndTranslate

(
DMatrix4dP pInstance,
double tx,
double ty,
double tz,
double sx,
double sy,
double sz,
DMatrix4dCP pA
)

    {
    int i, j;
    double tv[3];
    double sv[3];

    if  (pA != NULL)
        {
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
                pInstance->coff[i][j]
                        = sv[i] * pInstance->coff[i][j]
                        + tv[i] * pInstance->coff[3][j];
                }
            }
        }
    else
        {
        memset (pInstance, 0, sizeof (DMatrix3d));
        bsiDMatrix4d_initIdentity (pInstance);
        pInstance->coff[0][0] = sx;
        pInstance->coff[1][1] = sy;
        pInstance->coff[2][2] = sz;
        pInstance->coff[3][3] = 1.0;
        pInstance->coff[0][3] = tx;
        pInstance->coff[1][3] = ty;
        pInstance->coff[2][3] = tz;
        }
    }

/* METHOD(DMatrix4d,none,RMS)
/*-----------------------------------------------------------------*//**
* @instance pA => matrix whose RMS is being computed
* @see
* @return Root mean square of entries in the matrix, i.e. square root of
*       S/16 where S is the sum of squares.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDMatrix4d_RMS

(
DMatrix4dCP pA
)
    {
    double s = 0;
    int i,j;
    for ( i = 0; i < 4; i++ )
        for ( j = 0; j < 4; j++ )
            {
            s +=  pA->coff[i][j] * pA->coff[i][j];
            }
    return sqrt (s/16.0);
    }



/*-----------------------------------------------------------------*//**
* Form the product of two 4x4 matrices.
* @param pA => first matrix of product A*B
* @param pB => second matrix of product A*B
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiply

(
DMatrix4dP pC,
DMatrix4dCP pA,
DMatrix4dCP pB
)

    {
    if ( pA == pC )
        {
        DMatrix4d D = *(DMatrix4d*)pA;
        if ( pB == pC ) /* really doing C := C*C */
            {
            bsiDMatrix4d_multiply( pC, &D, &D);
            }
        else            /* really doing C := C * B */
            {
            bsiDMatrix4d_multiply( pC, &D, pB );
            }
        }
    else if ( pB == pC )        /* really doing C = A * C */
        {
        DMatrix4d D = *(DMatrix4d*)pB;
        bsiDMatrix4d_multiply( pC, pA, &D);
        }
    else
        {
        int i,j;
        for ( i = 0; i < 4; i++ )
            for ( j = 0; j < 4; j++  )
                {
                pC->coff[i][j] = pA->coff[i][0] * pB->coff[0][j]
                    + pA->coff[i][1] * pB->coff[1][j]
                    + pA->coff[i][2] * pB->coff[2][j]
                    + pA->coff[i][3] * pB->coff[3][j];
                }
        }
    }



/*-----------------------------------------------------------------*//**
* Form the product of three 4x4 matrices.

* @instance pABC <= Product of A and B
* @param pA => first matrix of product A*B*C
* @param pB => second matrix of product A*B*C
* @param pC => third matrix of product A*B*C
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiply3

(
DMatrix4dP pABC,
DMatrix4dCP pA,
DMatrix4dCP pB,
DMatrix4dCP pC
)

    {
    DMatrix4d AB, ABC;
    int i, j;
    ABC.InitIdentity ();
    for ( i = 0; i < 4; i++ )
        for ( j = 0; j < 4; j++  )
            {
            AB.coff[i][j] =   pA->coff[i][0] * pB->coff[0][j]
                            + pA->coff[i][1] * pB->coff[1][j]
                            + pA->coff[i][2] * pB->coff[2][j]
                            + pA->coff[i][3] * pB->coff[3][j];
            }

    for ( i = 0; i < 4; i++ )
        for ( j = 0; j < 4; j++  )
            {
            ABC.coff[i][j] =  AB.coff[i][0] * pC->coff[0][j]
                            + AB.coff[i][1] * pC->coff[1][j]
                            + AB.coff[i][2] * pC->coff[2][j]
                            + AB.coff[i][3] * pC->coff[3][j];
            }
    *pABC = ABC;
    }



/*-----------------------------------------------------------------*//**
*
* Subtract two 4x4 matrices.
*
* @instance pC <= result A - B
* @param pA => A matrix of A - B
* @param pB => B matrix of A - B
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_subtractDMatrix4d

(
DMatrix4dP pC,
DMatrix4dCP pA,
DMatrix4dCP pB
)

    {
    int i,j;
    for ( i = 0;  i < 4 ; i++ )
        for ( j = 0; j < 4; j++ )
            pC->coff[i][j] = pA->coff[i][j] - pB->coff[i][j];
    }

/*-----------------------------------------------------------------*//**
* Add a matrix (componentwise) to the instance.
*
* @instance <= pInstance + pDelta
* @param pVector => matrix to add
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_addDMatrix4dInPlace

(
DMatrix4dP pInstance,
DMatrix4dCP pDelta
)
    {
    int i;
    double *pA = (double *)pInstance;
    double *pB = (double *)pDelta;
    for (i = 0; i < 16; i++, pA++, pB++)
        {
        *pA += *pB;
        }
    }


/*-----------------------------------------------------------------*//**
* Subtract a matrix (componentwise) from the instance
*
* @instance <= pInstance - pDelta
* @param pDelta => matrix to subtract
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_subtract

(
DMatrix4dP pInstance,
DMatrix4dCP pDelta
)
    {
    int i;
    double *pA = (double *)pInstance;
    double *pB = (double *)pDelta;
    for (i = 0; i < 16; i++, pA++, pB++)
        {
        *pA -= *pB;
        }
    }

/*----------------------------------------------------------------------+
| TITLE QR Homogeneous Matrix Inversion and Factoring                   |
| These functions provide inversion and factoring of DMatrix4d structures,|
| both as single-step operations and as lower level components          |
| that can be used for other manipulations.                             |
|                                                                       |
| Inversion and factoring are done with the QR method, i.e. application |
| of a sequence of 4-dimensional reflection (mirror) operations which   |
| lead to factored forms with high numerical stability.                 |
|                                                                       |
| The single step functions are:                                        |
|<UL>                                                                   |
|<LI>jmdlDMatrix4d_invertQR(&matrixB, &matrixA) -- returns in matrixB   |
|       the inverse of matrixA.                                         |
|<LI>jmdlDMatrix4d_solve (&xVector, &matrixA, &bVector) -- solves               |
|       the linear system of equations A*x = b where A is a homogeneous |
|       matrix, b is a given homogeneous vector, and x is the           |
|       homogeneous solution vector.                                    |
|</UL>                                                                  |
|                                                                       |
| The following functions are the building blocks for QR manipulations: |
|<UL>                                                                   |
|<LI>bsiDMatrix4d_partialColumnDotProduct(..) -- compute a dot product  |
|       between vectors defined as portions of columns in two matrices. |
|<LI>jmdlDMatrix4d_partialColumnUpate (..) -- subtract a scaled column  |
|       from a given column, updating in place.                         |
|<LI>jmdlDMatrix4d_constructPartialColumnReflection(..) -- construct    |
|       the Householder vector that reflects a given portion of         |
|       a column to the corresponding pivot axis                        |
|<LI>jmdlDMatrix4d_factorQR(..) -- factor a matrix as a product of a    |
|       sequence of orthogonal reflections and an upper-triangular      |
|       matrix.   R is returned as an explicit upper triangular matrix, |
|       Q as a collection of reflection vectors.                        |
|<LI>bsiDMatrix4d_expandPartialColumnReflection -- expand a single      |
|       reflection vector to its full matrix form                       |
|<LI>jmdlDMatrix4d_backSubstitute -- solve a triangular system          |
|<LI>bsiDMatrix4d_expandReflections -- expand the full sequence of      |
|       reflections (from factorization) to full matrix form            |
|</UL>                                                                  |
+----------------------------------------------------------------------*/
#define HDIM 4


/*-----------------------------------------------------------------*//**
*
* Explode a 4x4 matrix into a 3x3 matrix, two 3D vectors, and a scalar, with
* the scalar location defined by a pivot index along the diagonal.
*
* @param pMatrix <= 3x3 submatrix
* @param pRow <= off-diagonals in row
* @param pColumn <= off-diagonals in column
* @param pPivot <= pivot entry
* @param pHMat => Original matrix
* @param pivot => pivot index
* @see
* @return true if pivot index is valid.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_extractAroundPivot

(
DMatrix4dCP pHMat,
DMatrix3dP pMatrix,
DPoint3dP pRow,
DPoint3dP pColumn,
double        *pPivot,
int           pivot
)
    {
    int i, j;
    bool    boolStat = false;
    int index[3];
    int i0, i1, i2, jj;
    if (pivot >= 0 && pivot < HDIM)
        {
        if (pPivot)
            *pPivot = pHMat->coff[pivot][pivot];

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

        if (pRow)
            {
            bsiDPoint3d_setXYZ (pRow,
                        pHMat->coff[pivot][i0],
                        pHMat->coff[pivot][i1],
                        pHMat->coff[pivot][i2]);
            }

        if (pColumn)
            {
            bsiDPoint3d_setXYZ (pColumn,
                        pHMat->coff[i0][pivot],
                        pHMat->coff[i1][pivot],
                        pHMat->coff[i2][pivot]);
            }

        if (pMatrix)
            {
            for (j = 0; j < 3; j++)
                {
                jj = index[j];
                bsiDPoint3d_setXYZ (&pMatrix->column[j],
                        pHMat->coff[i0][jj],
                        pHMat->coff[i1][jj],
                        pHMat->coff[i2][jj]
                        );
                }
            }
        boolStat = true;
        }
    return boolStat;
    }



/*-----------------------------------------------------------------*//**
*
* Explode a 4x4 matrix into a 3x3 matrix, two 3D vectors, and a scalar, with
* the scalar location defined by a pivot index along the diagonal.
*
* @param pMatrix <= 3x3 submatrix
* @param pRow <= off-diagonals in row
* @param pColumn <= off-diagonals in column
* @param pPivot <= pivot entry
* @param pHMat => Original matrix
* @param pivot => pivot index
* @see
* @return true if pivot index is valid.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_extractAroundPivot

(
DMatrix4dCP pHMat,
RotMatrixP pMatrix,
DPoint3dP pRow,
DPoint3dP pColumn,
double        *pPivot,
int           pivot
)
    {
    int i, j;
    bool    boolStat = false;
    int index[3];
    int i0, i1, i2, jj;
    if (pivot >= 0 && pivot < HDIM)
        {
        if (pPivot)
            *pPivot = pHMat->coff[pivot][pivot];

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

        if (pRow)
            {
            bsiDPoint3d_setXYZ (pRow,
                        pHMat->coff[pivot][i0],
                        pHMat->coff[pivot][i1],
                        pHMat->coff[pivot][i2]);
            }

        if (pColumn)
            {
            bsiDPoint3d_setXYZ (pColumn,
                        pHMat->coff[i0][pivot],
                        pHMat->coff[i1][pivot],
                        pHMat->coff[i2][pivot]);
            }

        if (pMatrix)
            {
            for (j = 0; j < 3; j++)
                {
                jj = index[j];
                pMatrix->SetColumn (
                        DVec3d::From (pHMat->coff[i0][jj], pHMat->coff[i1][jj], pHMat->coff[i2][jj]),
                        j
                        );
                }
            }
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
* @instance pA <= Bt*D*B
* @param pSigma => entries of diagonal matrix D
* @param pB => matrix B
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_symmetricProduct

(
DMatrix4dP pA,
DPoint4dCP pSigma,
DMatrix4dCP pB
)

    {
    DMatrix4d C;
    double factor;
    int i,j;
    const double *pDiag = (const double *)pSigma;
    /* Form C = B^t * D */
    for (j = 0; j < HDIM; j++)
        {
        factor = pDiag[j];
        for (i = 0; i < HDIM; i++)
            C.coff[i][j] = factor * pB->coff[j][i];
        }

    bsiDMatrix4d_multiply (pA, &C, pB);
    }



/*-----------------------------------------------------------------*//**
*
* Search the matrix for the entry with the largest absolute value.
*
* @instance pMatrix => matrix to examine
* @see
* @return max absolute value entry in the matrix.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDMatrix4d_maxAbs

(
DMatrix4dCP pMatrix
)

    {
    return DoubleOps::MaxAbs ((const double *) pMatrix, HDIM * HDIM);
    }



/*-----------------------------------------------------------------*//**
* Tests if pInstance is an identity transform and returns the bool
* indicator.
* The matrix is considered an identity if the sum of squared components
* after subtracting 1 from the diagonals is less than a small tolerance
*
* @param => matrix to test
* @see
* @return true if matrix is approximately an identity.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_isIdentity

(
DMatrix4dCP pInstance
)
    {
    /* Entries in an identity matrix are strictly in the 0..1 range,*/
    /* hence a simple tolerance near machine precision is warranted.*/
    double tol = 1.0e-12;

    if (
           fabs( pInstance->coff[0][0] - 1.0 )   <= tol
        && fabs( pInstance->coff[1][1] - 1.0 )   <= tol
        && fabs( pInstance->coff[2][2] - 1.0 )   <= tol
        && fabs( pInstance->coff[3][3] - 1.0 )   <= tol

        && fabs( pInstance->coff[0][1])   <= tol
        && fabs( pInstance->coff[0][2])   <= tol
        && fabs( pInstance->coff[0][3])   <= tol

        && fabs( pInstance->coff[1][0])   <= tol
        && fabs( pInstance->coff[1][2])   <= tol
        && fabs( pInstance->coff[1][3])   <= tol

        && fabs( pInstance->coff[2][0])   <= tol
        && fabs( pInstance->coff[2][1])   <= tol
        && fabs( pInstance->coff[2][3])   <= tol

        && fabs( pInstance->coff[3][0])   <= tol
        && fabs( pInstance->coff[3][1])   <= tol
        && fabs( pInstance->coff[3][2])   <= tol
        )
        return true;

    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
