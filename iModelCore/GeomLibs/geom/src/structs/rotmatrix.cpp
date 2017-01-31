/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/rotmatrix.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/



/*-----------------------------------------------------------------*//**
@struct RotMatrix
A RotMatrix is a classic microstation linear transformation.
@bsistruct                                          EarlinLutz      09/00
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE



#define __RotMatrix__
#ifdef __RotMatrix__
static const RotMatrix sm_identityMatrix =
        {{
            {1.0, 0.0, 0.0},
            {0.0, 1.0, 0.0},
            {0.0, 0.0, 1.0}
        }};
#else
static const RotMatrix sm_identityMatrix =
        {{
            {1.0, 0.0, 0.0},
            {0.0, 1.0, 0.0},
            {0.0, 0.0, 1.0}
        }};
#endif

//static const double s_tightRelTol = 1.0e-15;
static const double s_mediumRelTol = 1.0e-12;

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
static void copyDoubleThroughPointer


(
double *pDouble,
double myDouble
)
    {
    if (pDouble)
        *pDouble = myDouble;
    }


/*-----------------------------------------------------------------*//**
Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
Each result is placed in the corresponding entry in the output array pResult.   The same array
may be named for the input and output arrays.
@param pMatrix => The matrix to apply
@param pResult <= output points
@param pPoint => The input points
@param numPoint => The number of points
@group "RotMatrix Multiplication"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyDPoint4dArray


(
RotMatrixCP pMatrix,
DPoint4dP pResult,
DPoint4dCP pPoint,
int            numPoint
)
    {
    int     i;
    double x,y,z;
    DPoint4d *pResultPoint;

    for (i = 0, pResultPoint = pResult;
         i < numPoint;
         i++, pResultPoint++
        )
        {
        x = pPoint[i].x;
        y = pPoint[i].y;
        z = pPoint[i].z;
        pResultPoint->w = pPoint[i].w;

        pResultPoint->x = pMatrix->form3d[0][0] * x
                + pMatrix->form3d[0][1] * y
                + pMatrix->form3d[0][2] * z;

        pResultPoint->y = pMatrix->form3d[1][0] * x
                + pMatrix->form3d[1][1] * y
                + pMatrix->form3d[1][2] * z;

        pResultPoint->z = pMatrix->form3d[2][0] * x
                + pMatrix->form3d[2][1] * y
                + pMatrix->form3d[2][2] * z;

        }
    }



/*-----------------------------------------------------------------*//**
Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
Each result is placed in the corresponding entry in the output array pResult.   The same array
may be named for the input and output arrays.
@param pMatrix => The matrix to apply
@param pResult <= output points
@param pPoint => The input points
@param numPoint => The number of points
@group "RotMatrix Multiplication"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyDPoint2dArray


(
RotMatrixCP pMatrix,
DPoint2dP pResult,
DPoint2dCP pPoint,
int            numPoint
)
    {
    int     i;
    double x,y;
    DPoint2d *pResultPoint;

    for (i = 0, pResultPoint = pResult;
         i < numPoint;
         i++, pResultPoint++
        )
        {
        x = pPoint[i].x;
        y = pPoint[i].y;

        pResultPoint->x = pMatrix->form3d[0][0] * x
                + pMatrix->form3d[0][1] * y;

        pResultPoint->y = pMatrix->form3d[1][0] * x
                + pMatrix->form3d[1][1] * y;
        }
    }


/*-----------------------------------------------------------------*//**
Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
Each result is placed in the corresponding entry in the output array pResult.   The same array
may be named for the input and output arrays.
@param pMatrix => The matrix to apply
@param pResult <= output points
@param pPoint => The input points
@param numPoint => The number of points
@group "RotMatrix Multiplication"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyFPoint3dArray


(
RotMatrixCP pMatrix,
FPoint3dP pResult,
FPoint3dCP pPoint,
int            numPoint
)
    {
    int     i;
    double x,y,z;
    FPoint3d *pResultPoint;

    for (i = 0, pResultPoint = pResult;
         i < numPoint;
         i++, pResultPoint++
        )
        {
        x = pPoint[i].x;
        y = pPoint[i].y;
        z = pPoint[i].z;

        pResultPoint->x = (float)(pMatrix->form3d[0][0] * x
                + pMatrix->form3d[0][1] * y
                + pMatrix->form3d[0][2] * z);

        pResultPoint->y = (float)(pMatrix->form3d[1][0] * x
                + pMatrix->form3d[1][1] * y
                + pMatrix->form3d[1][2] * z);

        pResultPoint->z = (float)(pMatrix->form3d[2][0] * x
                + pMatrix->form3d[2][1] * y
                + pMatrix->form3d[2][2] * z);

        }
    }


/*-----------------------------------------------------------------*//**
Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
Each result is placed in the corresponding entry in the output array pResult.   The same array
may be named for the input and output arrays.
@param pMatrix => The matrix to apply
@param pResult <= output points
@param pPoint => The input points
@param numPoint => The number of points
@group "RotMatrix Multiplication"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyFPoint2dArray


(
RotMatrixCP pMatrix,
FPoint2dP pResult,
FPoint2dCP pPoint,
int            numPoint
)
    {
    int     i;
    double x,y;
    FPoint2d *pResultPoint;

    for (i = 0, pResultPoint = pResult;
         i < numPoint;
         i++, pResultPoint++
        )
        {
        x = pPoint[i].x;
        y = pPoint[i].y;

        pResultPoint->x = (float)(pMatrix->form3d[0][0] * x
                + pMatrix->form3d[0][1] * y);

        pResultPoint->y = (float)(pMatrix->form3d[1][0] * x
                + pMatrix->form3d[1][1] * y);
        }
    }



/*-----------------------------------------------------------------*//**
* Matrix times matrix, using only the xy rows (but z clumn) of B.
* @instance pProduct <= product matrix
@param pA => The first factor
@param pB => The second factor.  z row treated as zero.
@group "RotMatrix Multiplication"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixRotMatrixXY


(
RotMatrixP pProduct,
RotMatrixCP pA,
RotMatrixCP pB
)
    {
    int       j;
    RotMatrix AB;

    for (j = 0; j < 3; j++)
        {
        AB.form3d[0][j]
                       = pA->form3d[0][0] * pB->form3d[0][j]
                       + pA->form3d[0][1] * pB->form3d[1][j];

        AB.form3d[1][j]
                       = pA->form3d[1][0] * pB->form3d[0][j]
                       + pA->form3d[1][1] * pB->form3d[1][j];

        AB.form3d[2][j]
                       = pA->form3d[2][0] * pB->form3d[0][j]
                       + pA->form3d[2][1] * pB->form3d[1][j];
        }
    }



/* VBSUB(Matrix3dFromMatrix3dTimesMatrix3d) */
/* CSVFUNC(multiply) */

/*-----------------------------------------------------------------*//**
@description Returns the product {A*B} of two matrices.
@instance pProduct  <= product matrix
@param pA => The first factor
@param pB => The second factor
@group "RotMatrix Multiplication"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixRotMatrix


(
RotMatrixP pProduct,
RotMatrixCP pA,
RotMatrixCP pB
)
    {
    int       j;
    RotMatrix AB;
    AB.Zero ();
    for (j = 0; j < 3; j++)
        {
        AB.form3d[0][j]
                       = pA->form3d[0][0] * pB->form3d[0][j]
                       + pA->form3d[0][1] * pB->form3d[1][j]
                       + pA->form3d[0][2] * pB->form3d[2][j];

        AB.form3d[1][j]
                       = pA->form3d[1][0] * pB->form3d[0][j]
                       + pA->form3d[1][1] * pB->form3d[1][j]
                       + pA->form3d[1][2] * pB->form3d[2][j];

        AB.form3d[2][j]
                       = pA->form3d[2][0] * pB->form3d[0][j]
                       + pA->form3d[2][1] * pB->form3d[1][j]
                       + pA->form3d[2][2] * pB->form3d[2][j];
        }
    *pProduct = AB;
    }

/*-----------------------------------------------------------------*//**
@description Returns the product {AT*B} of a transposed matrix and a matrix.
@instance pProduct  <= product matrix
@param pA => The first factor, to be tranposed.
@param pB => The second factor
@group "RotMatrix Multiplication"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixTransposeRotMatrix


(
RotMatrixP pProduct,
RotMatrixCP pA,
RotMatrixCP pB
)
    {
    int       j;
    RotMatrix ATB;
    ATB.Zero ();
    for (j = 0; j < 3; j++)
        {
        ATB.form3d[0][j]
                       = pA->form3d[0][0] * pB->form3d[0][j]
                       + pA->form3d[1][0] * pB->form3d[1][j]
                       + pA->form3d[2][0] * pB->form3d[2][j];

        ATB.form3d[1][j]
                       = pA->form3d[0][1] * pB->form3d[0][j]
                       + pA->form3d[1][1] * pB->form3d[1][j]
                       + pA->form3d[2][1] * pB->form3d[2][j];

        ATB.form3d[2][j]
                       = pA->form3d[0][2] * pB->form3d[0][j]
                       + pA->form3d[1][2] * pB->form3d[1][j]
                       + pA->form3d[2][2] * pB->form3d[2][j];
        }
    *pProduct = ATB;
    }

/*-----------------------------------------------------------------*//**
@description Returns the product {A*BT} of a matrix and a transposed matrix.
@instance pProduct  <= product matrix
@param pA => The first factor
@param pB => The second factor, to be transposed
@group "RotMatrix Multiplication"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixRotMatrixTranspose


(
RotMatrixP pProduct,
RotMatrixCP pA,
RotMatrixCP pB
)
    {
    RotMatrix BT;
    bsiRotMatrix_transpose (&BT, pB);
    bsiRotMatrix_multiplyRotMatrixRotMatrix (pProduct, pA, &BT);
    }

/* VBSUB(Matrix3dFromMatrix3dTimesMatrix3dTimesMatrix3d) */
/* CSVFUNC(multiply) */

/*-----------------------------------------------------------------*//**
@description Returns the product of three matrices.
@instance pProduct  <= product matrix
@param pA => The first factor
@param pB => The second factor
@param pC => The third factor
@group "RotMatrix Multiplication"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixRotMatrixRotMatrix


(
RotMatrixP pProduct,
RotMatrixCP pA,
RotMatrixCP pB,
RotMatrixCP pC
)
    {
    bsiRotMatrix_multiplyRotMatrixRotMatrix (pProduct, pA, pB);
    bsiRotMatrix_multiplyRotMatrixRotMatrix (pProduct, pProduct, pC);
    }



/* CSVFUNC(multiply) */

/*-----------------------------------------------------------------*//**
Sets this instance to the product of matrix pA times the matrix part
of transform pB.
In this 'post multiplication' the matrix part of pB multiplies pA ON THE RIGHT
and the translation part of pB is ignored.
* @instance pAB <= product matrix
@param pA => The first factor (matrix)
@param pB => The second factor (transform)
@group "RotMatrix Multiplication"
 @bsimethod                                                     DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixTransform


(
RotMatrixP pAB,
RotMatrixCP pA,
TransformCP pB
)
    {
    RotMatrix   AB;
    AB.Zero ();
    if (pA && pB)
        {
        int j;
        for (j = 0; j < 3; j++)
            {
            AB.form3d[0][j]
                           = pA->form3d[0][0] * pB->form3d[0][j]
                           + pA->form3d[0][1] * pB->form3d[1][j]
                           + pA->form3d[0][2] * pB->form3d[2][j];

            AB.form3d[1][j]
                           = pA->form3d[1][0] * pB->form3d[0][j]
                           + pA->form3d[1][1] * pB->form3d[1][j]
                           + pA->form3d[1][2] * pB->form3d[2][j];

            AB.form3d[2][j]
                           = pA->form3d[2][0] * pB->form3d[0][j]
                           + pA->form3d[2][1] * pB->form3d[1][j]
                           + pA->form3d[2][2] * pB->form3d[2][j];
            }
        *pAB = AB;
        }

    else if (pA)
        {
        *pAB = *pA;
        }
    else if (pB)
        {
        bsiRotMatrix_initFromTransform (pAB, pB);
        }
    else
        {
        bsiRotMatrix_initIdentity (pAB);
        }
    }



/* CSVFUNC(multiply) */

/*-----------------------------------------------------------------*//**
@description Returns to the product of the matrix part of transform pA
times matrix pB.
In this 'pre multiplication' the matrix part of pA multiplies pB ON THE LEFT
and the translation part of pA is ignored.
* @instance pAB <= product matrix
@param pA => The first factor (transform)
@param pB => The second factor (matrix)
@group "RotMatrix Multiplication"
 @bsimethod                                                     DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyTransformRotMatrix


(
RotMatrixP pAB,
TransformCP pA,
RotMatrixCP pB
)
    {
    RotMatrix   AB;
    AB.Zero ();
    if (pA && pB)
        {
        int j;
        for (j = 0; j < 3; j++)
            {
            AB.form3d[0][j]
                           = pA->form3d[0][0] * pB->form3d[0][j]
                           + pA->form3d[0][1] * pB->form3d[1][j]
                           + pA->form3d[0][2] * pB->form3d[2][j];

            AB.form3d[1][j]
                           = pA->form3d[1][0] * pB->form3d[0][j]
                           + pA->form3d[1][1] * pB->form3d[1][j]
                           + pA->form3d[1][2] * pB->form3d[2][j];

            AB.form3d[2][j]
                           = pA->form3d[2][0] * pB->form3d[0][j]
                           + pA->form3d[2][1] * pB->form3d[1][j]
                           + pA->form3d[2][2] * pB->form3d[2][j];
            }
        *pAB = AB;
        }

    else if (pA)
        {
        bsiRotMatrix_initFromTransform (pAB, pA);
        }
    else if (pB)
        {
        *pAB = *pB;
        }
    else
        {
        bsiRotMatrix_initIdentity (pAB);
        }
    }




/*-----------------------------------------------------------------*//**
@description Returns the (scalar) LeftPoint*Matrix*RightPoint
@param pLeftPoint => The point applied on left of matrix
@param pA => The matrix
@param pRightPoint => The point applied on right of matrix
@return evaluated quadratic form
@group "RotMatrix Multiplication"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRotMatrix_quadraticForm


(
DPoint3dCP pLeftPoint,
RotMatrixCP pA,
DPoint3dCP pRightPoint
)
    {
    return
        pLeftPoint->x * (pA->form3d[0][0] * pRightPoint->x + pA->form3d[0][1] * pRightPoint->y + pA->form3d[0][2] * pRightPoint->z)
      + pLeftPoint->y * (pA->form3d[1][0] * pRightPoint->x + pA->form3d[1][1] * pRightPoint->y + pA->form3d[1][2] * pRightPoint->z)
      + pLeftPoint->z * (pA->form3d[2][0] * pRightPoint->x + pA->form3d[2][1] * pRightPoint->y + pA->form3d[2][2] * pRightPoint->z);
    }






/*-----------------------------------------------------------------*//**
computes the 8 corner points of the range cube defined by the two points
of pRange, multiplies matrix times each point, and computes the
twopoint range limits (min max) of the resulting rotated box.
Note that the volume of the rotated box is generally larger than
that of the input because the postrotation minmax operation expands
the box.
Ranges that are null as defined by pInRange->isNull are left
unchanged.
@instance pMatrix => The matrix to apply
@param pOutRange <= range of transformed cube
@param pInRange => The any two corners of original cube
@group "RotMatrix Multiplication"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRange


(
RotMatrixCP pMatrix,
DRange3dP pOutRange,
DRange3dCP pInRange
)
    {
    DPoint3d    corner[8];

    if (bsiDRange3d_isNull (pInRange))
        {
        *pOutRange = *pInRange;
        }
    else
        {
        bsiDRange3d_box2Points(pInRange, corner);
        bsiRotMatrix_multiplyDPoint3dArray (pMatrix, corner, corner, 8);
        bsiDRange3d_initFromArray (pOutRange, corner, 8);
        }
    }


/* VBSUB(Matrix3dIdentity) */

/*-----------------------------------------------------------------*//**
@description Returns an identity matrix.
@instance pMatrix <= identity matrix (1 on diagonal, 0 off diagonal)
@group "RotMatrix Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initIdentity


(
RotMatrixP pMatrix
)
    {
    *pMatrix = sm_identityMatrix;
    }

/* VBSUB(Matrix3dZero) */

/*-----------------------------------------------------------------*//**
@description returns a zero matrix.
@instance pMatrix <= a matrix of zeros.
@group "RotMatrix Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_zero


(
RotMatrixP pMatrix
)
    {
#if defined (__jmdl)
    *((_dMatrix3d *)pMatrix) = sm_zeroMatrix;
#else
    memset (pMatrix, 0, sizeof (RotMatrix));
#endif
    }



/* VBSUB(Matrix3dFromScaleFactors) */
/* CSVFUNC(FromScaleFactors) */

/*-----------------------------------------------------------------*//**
@description Returns a scaling matrix with respective x, y, and z
scaling factors.
@instance pMatrix <= scale matrix
@param xscale => The x direction scale factor
@param yscale => The y direction scale factor
@param zscale => The z direction scale factor
@group "RotMatrix Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromScaleFactors


(
RotMatrixP pMatrix,
double         xscale,
double         yscale,
double         zscale
)
    {

    bsiRotMatrix_zero (pMatrix);
    pMatrix->form3d[0][0] = xscale;
    pMatrix->form3d[1][1] = yscale;
    pMatrix->form3d[2][2] = zscale;
    }

/* VBSUB(Matrix3dFromScale) */
/* CSVFUNC(fromScale) */

/*-----------------------------------------------------------------*//**
@description Returns a uniform scaling matrix.
@instance pMatrix <= scale matrix
@param scale => The scale factor.
@group "RotMatrix Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromScale


(
RotMatrixP pMatrix,
double      scale
)
    {
    bsiRotMatrix_initFromScaleFactors (pMatrix, scale, scale, scale);
    }




/*-----------------------------------------------------------------*//**
* @description Returns a 'rank one' matrix defined as a scale factor times
the 'vector outer product' of two vectors, i.e. as the matrix {s*U*V^T}
* @param pMatrix <= constructed matrix
@param pVector1 => The column vector U
@param pVector2 => The row vector V
@param scale => The scale factor
@group "RotMatrix Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromScaledOuterProduct


(
RotMatrixP pMatrix,
DVec3dCP pVector1,
DVec3dCP pVector2,
double         scale
)
    {
    pMatrix->form3d[0][0] = scale * pVector1->x * pVector2->x;
    pMatrix->form3d[1][0] = scale * pVector1->y * pVector2->x;
    pMatrix->form3d[2][0] = scale * pVector1->z * pVector2->x;

    pMatrix->form3d[0][1] = scale * pVector1->x * pVector2->y;
    pMatrix->form3d[1][1] = scale * pVector1->y * pVector2->y;
    pMatrix->form3d[2][1] = scale * pVector1->z * pVector2->y;

    pMatrix->form3d[0][2] = scale * pVector1->x * pVector2->z;
    pMatrix->form3d[1][2] = scale * pVector1->y * pVector2->z;
    pMatrix->form3d[2][2] = scale * pVector1->z * pVector2->z;
    }


/*-----------------------------------------------------------------*//**
@description Returns a 'rank one' matrix defined as a scale factor times
        the 'vector outer product' of two vectors, i.e. as the matrix {s*U*V^T}
@param pMatrix <= constructed matrix
@param pVector1 => The column vector U
@param pVector2 => The row vector V
@param scale => The scale factor
@group "RotMatrix Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_addScaledOuterProductInPlace


(
RotMatrixP pMatrix,
DVec3dCP pVector1,
DVec3dCP pVector2,
double         scale
)
    {
    RotMatrix matrixA;
    bsiRotMatrix_initFromScaledOuterProduct (&matrixA, pVector1, pVector2, scale);
    bsiRotMatrix_addRotMatrixInPlace (pMatrix, &matrixA);
    }



/*-----------------------------------------------------------------*//**
Copies double arrays into corresponding columns of a matrix.
@instance pMatrix <= initialized matrix.
@param pXVector => The array to insert in column 0
@param pYVector => The array to insert in column 1
@param pZVector => The array to insert in column 2
@group "RotMatrix Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromColumnArrays


(
RotMatrixP pMatrix,
const double        *pXVector,
const double        *pYVector,
const double        *pZVector
)
    {
    pMatrix->form3d[0][0] = pXVector[0];
    pMatrix->form3d[1][0] = pXVector[1];
    pMatrix->form3d[2][0] = pXVector[2];

    pMatrix->form3d[0][1] = pYVector[0];
    pMatrix->form3d[1][1] = pYVector[1];
    pMatrix->form3d[2][1] = pYVector[2];

    pMatrix->form3d[0][2] = pZVector[0];
    pMatrix->form3d[1][2] = pZVector[1];
    pMatrix->form3d[2][2] = pZVector[2];
    }

/* VBSUB(Matrix3dFromRowValues) */


/*-----------------------------------------------------------------*//**
@description Returns a matrix with the 9 specified coefficients given
in "row major" order.
@instance pMatrix <= initialized matrix.
@param x00 => The 00 entry
@param x01 => The 01 entry
@param x02 => The 02 entry
@param x10 => The 10 entry
@param x11 => The 11 entry
@param x12 => The 12 entry
@param x20 => The 20 entry
@param x21 => The 21 entry
@param x22 => The 22 entry
@group "RotMatrix Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromRowValues


(
RotMatrixP pMatrix,
double        x00,
double        x01,
double        x02,
double        x10,
double        x11,
double        x12,
double        x20,
double        x21,
double        x22
)
    {
    pMatrix->form3d[0][0] = x00;
    pMatrix->form3d[1][0] = x10;
    pMatrix->form3d[2][0] = x20;

    pMatrix->form3d[0][1] = x01;
    pMatrix->form3d[1][1] = x11;
    pMatrix->form3d[2][1] = x21;

    pMatrix->form3d[0][2] = x02;
    pMatrix->form3d[1][2] = x12;
    pMatrix->form3d[2][2] = x22;
    }

/* VBSUB(Matrix3dFromPoint3dColumns) */


/*-----------------------------------------------------------------*//**
@description Returns a matrix with 3 points copied to respective columns.
@instance pMatrix <= initialized matrix.
@param pXVector => The vector to insert in column 0
@param pYVector => The vector to insert in column 1
@param pZVector => The vector to insert in column 2
@group "RotMatrix Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromColumnVectors


(
RotMatrixP pMatrix,
DVec3dCP pXVector,
DVec3dCP pYVector,
DVec3dCP pZVector
)
    {
    pMatrix->form3d[0][0] = pXVector->x;
    pMatrix->form3d[1][0] = pXVector->y;
    pMatrix->form3d[2][0] = pXVector->z;

    pMatrix->form3d[0][1] = pYVector->x;
    pMatrix->form3d[1][1] = pYVector->y;
    pMatrix->form3d[2][1] = pYVector->z;

    pMatrix->form3d[0][2] = pZVector->x;
    pMatrix->form3d[1][2] = pZVector->y;
    pMatrix->form3d[2][2] = pZVector->z;
    }




/*-----------------------------------------------------------------*//**
Copies double arrays into corresponding rows of a matrix.
@instance pMatrix <= initialized matrix.
@param pXVector => The array to insert in row 0
@param pYVector => The array to insert in row 1
@param pZVector => The array to insert in row 2
@group "RotMatrix Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromRowArrays


(
RotMatrixP pMatrix,
const double        *pXVector,
const double        *pYVector,
const double        *pZVector
)
    {
    int i;
    for (i = 0; i < 3; i++)
        {
        pMatrix->form3d[0][i] = pXVector[i];
        pMatrix->form3d[1][i] = pYVector[i];
        pMatrix->form3d[2][i] = pZVector[i];
        }
    }


/* VBSUB(Matrix3dFromPoint3dRows) */


/*-----------------------------------------------------------------*//**
@description Returns a matrix with 3 points copied to respective rows.
@instance pMatrix <= initialized matrix.
@param pXVector => The vector to insert in row 0
@param pYVector => The vector to insert in row 1
@param pZVector => The vector to insert in row 2
@group "RotMatrix Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromRowVectors


(
RotMatrixP pMatrix,
DVec3dCP pXVector,
DVec3dCP pYVector,
DVec3dCP pZVector
)
    {
    pMatrix->form3d[0][0] = pXVector->x;
    pMatrix->form3d[1][0] = pYVector->x;
    pMatrix->form3d[2][0] = pZVector->x;

    pMatrix->form3d[0][1] = pXVector->y;
    pMatrix->form3d[1][1] = pYVector->y;
    pMatrix->form3d[2][1] = pZVector->y;

    pMatrix->form3d[0][2] = pXVector->z;
    pMatrix->form3d[1][2] = pYVector->z;
    pMatrix->form3d[2][2] = pZVector->z;
    }

/* VBSUB(Matrix3dFromVectorAndRotationAngle) */


/*-----------------------------------------------------------------*//**
@description Returns a matrix representing rotation around a vector.
@instance pMatrix <= rotation matrix
@param pAxis => The axis of rotation
@param radians => The rotation angle
@group "RotMatrix Rotations"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromVectorAndRotationAngle


(
RotMatrixP pMatrix,
DVec3dCP pAxis,
double         radians
)
    {
    double c = cos(radians);
    double s = sin(radians);
    double v = 1.0 - c;

    DVec3d dirVec;
    double mag = bsiDVec3d_normalize (&dirVec, pAxis);

    if (mag == 0.0)
        {
        bsiRotMatrix_initIdentity (pMatrix);
        return;
        }

    bsiRotMatrix_initFromScaledOuterProduct (pMatrix, &dirVec, &dirVec, v );

    pMatrix->form3d[0][0] += c;
    pMatrix->form3d[0][1] -= s * dirVec.z;
    pMatrix->form3d[0][2] += s * dirVec.y;

    pMatrix->form3d[1][0] += s * dirVec.z;
    pMatrix->form3d[1][1] += c;
    pMatrix->form3d[1][2] -= s * dirVec.x;

    pMatrix->form3d[2][0] -= s * dirVec.y;
    pMatrix->form3d[2][1] += s * dirVec.x;
    pMatrix->form3d[2][2] += c;
    }

/* VBSUB(Matrix3dFromDirectionAndScale) */
/* CSVFUNC(fromDirectionAndScale) */

/*-----------------------------------------------------------------*//**
@description Returns a matrix which scales along a vector direction.
@param pMatrix <= scale matrix.
@param pVector => The scaling direction
@param scale => The scale factor
@group "RotMatrix Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromDirectionAndScale


(
RotMatrixP pMatrix,
DVec3dCP pVector,
double         scale
)
    {
    double denominator = bsiDVec3d_dotProduct (pVector, pVector);
    if (denominator == 0.0)
        {
        bsiRotMatrix_initIdentity (pMatrix);
        }
    else
        {
        bsiRotMatrix_initFromScaledOuterProduct (pMatrix,
                pVector, pVector, (scale - 1.0)/denominator);
        pMatrix->form3d[0][0] += 1.0;
        pMatrix->form3d[1][1] += 1.0;
        pMatrix->form3d[2][2] += 1.0;
        }
    }

/* VBSUB(Matrix3dFromAxisAndRotationAngle) */
/* CSVFUNC(fromAxisAndRotationAngle) */

/*-----------------------------------------------------------------*//**
@description Returns a matrix of rotation  about the x,y, or z axis


(indicated by axis = 0,1, or 2) by an angle in radians.
* @instance pMatrix <= rotation matrix
@param axis => The axis index 0=x, 1=y, 2=z
@param radians => The rotation angle in radians
@group "RotMatrix Rotations"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromAxisAndRotationAngle


(
RotMatrixP pMatrix,
int            axis,
double         radians
)
    {
    double c = cos(radians);
    double s = sin(radians);
    bsiRotMatrix_initIdentity (pMatrix);

    if (2 == axis)
       {
       pMatrix->form3d[0][0] = pMatrix->form3d[1][1] = c;
       pMatrix->form3d[1][0] = s;
       pMatrix->form3d[0][1] = -s;
      }
    else if (1 == axis)
       {
       pMatrix->form3d[0][0] = pMatrix->form3d[2][2] = c;
       pMatrix->form3d[0][2] =  s;
       pMatrix->form3d[2][0] = -s;
       }
    else if (0 == axis)
       {
       pMatrix->form3d[1][1] = pMatrix->form3d[2][2] = c;
       pMatrix->form3d[1][2] = -s;
       pMatrix->form3d[2][1] =  s;
       }
    }

/*======================================================================+
|SECTION translationPart Modifications to translation part.             |
+======================================================================*/


/* VBSUB(Matrix3dTranspose) */


/*-----------------------------------------------------------------*//**
@description Returns the transpose of a matrix.
@param pTranspose <= transposed matrix
@param pMatrix => The input matrix
@group "RotMatrix Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_transpose


(
RotMatrixP pTranspose,
RotMatrixCP pMatrix
)
    {

    RotMatrix tempTranspose;

    /* EDL: Base MS version did not allow inplace transpose */
    if (pTranspose == pMatrix)
        {
        tempTranspose = *pMatrix;
        pMatrix = &tempTranspose;
        }

    pTranspose->form3d[0][0] = pMatrix->form3d[0][0];
    pTranspose->form3d[1][0] = pMatrix->form3d[0][1];
    pTranspose->form3d[2][0] = pMatrix->form3d[0][2];

    pTranspose->form3d[0][1] = pMatrix->form3d[1][0];
    pTranspose->form3d[1][1] = pMatrix->form3d[1][1];
    pTranspose->form3d[2][1] = pMatrix->form3d[1][2];

    pTranspose->form3d[0][2] = pMatrix->form3d[2][0];
    pTranspose->form3d[1][2] = pMatrix->form3d[2][1];
    pTranspose->form3d[2][2] = pMatrix->form3d[2][2];
    }



/*-----------------------------------------------------------------*//**
@description Transposes a matrix in place.
@instance pMatrix <=> The transposed matrix
@group "RotMatrix In Place Modification"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_transposeInPlace


(
RotMatrixP pMatrix
)
    {
    bsiRotMatrix_transpose (pMatrix, pMatrix);
    }

/* VBSUB(Matrix3dInverse) */



/*-----------------------------------------------------------------*//**
@description Compute the inverse of the matrix.
@param pInverse <= inverted matrix
@param pForward => The input matrix
@return true if the matrix is invertible.
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_invertRotMatrix


(
RotMatrixP pInverse,
RotMatrixCP pForward
)
    {
    DVec3d vectorU, vectorV, vectorW;
    DVec3d vector0, vector1, vector2;
    RotMatrix forward = *pForward;
    double det;
    double inverseDet;

    bool    boolStat;
    double mag = bsiRotMatrix_maxAbs (pForward);
    double scale;
    /* relTol1 identifies obviously safe matrices, based on
        simple determinant after scaling so largest entry is 1.
    */
    static double s_relTol1 = 1.0e-8;
    /* relTol2 identifies truly bad matrices, based on singular
        values. */
    static double s_relTol2 = 1.0e-15;

    bsiRotMatrix_getColumns (pForward, &vectorU, &vectorV, &vectorW);
    if (mag == 0.0)
        {
        /* The matrix is singular.  Fill in the identity */
        bsiRotMatrix_initIdentity (pInverse);
        return false;
        }

    scale = 1.0 / mag;
    bsiDVec3d_scaleInPlace (&vectorU, scale);
    bsiDVec3d_scaleInPlace (&vectorV, scale);
    bsiDVec3d_scaleInPlace (&vectorW, scale);

    det = bsiDVec3d_tripleProduct( &vectorU, &vectorV, &vectorW);

    /* Largest entry in matrix is now 1.
       Allow cramers rule if determinant is clearly nonzero
    */
    if (fabs (det) > s_relTol1)
        {
        bsiDVec3d_crossProduct (&vector0, &vectorV, &vectorW);
        bsiDVec3d_crossProduct (&vector1, &vectorW, &vectorU);
        bsiDVec3d_crossProduct (&vector2, &vectorU, &vectorV);
        /* do the work of initFromColumnVectors and transpose all at once! */
        bsiRotMatrix_initFromRowVectors (pInverse, &vector0, &vector1, &vector2);
        inverseDet = 1.0 / (mag * det);

        bsiRotMatrix_scaleColumns (pInverse, pInverse, inverseDet, inverseDet, inverseDet);
        boolStat = true;
        }
    else
        {
        /* The matrix is badly conditioned. Use singular values
            to make the hard decision */
        RotMatrix inverse;
        double condition;
        if (  bsiRotMatrix_invertRotMatrixByOrthogonalFactors (&forward, &inverse, &condition)
           && condition > s_relTol2
           )
            {
            *pInverse = inverse;
            boolStat = true;
            }
        else
            {
            bsiRotMatrix_initIdentity (pInverse);
            boolStat = false;
            }
        }


    return boolStat;
    }

/* VBSUB(Matrix3dHasInverse) */

/*-----------------------------------------------------------------*//**
@description Tests if a matrix can be inverted
@param pMatrix => The matrix to test.
@return true if the matrix is invertible.
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_hasInverse


(
RotMatrixCP pMatrix
)
    {
    RotMatrix inverse;
    return bsiRotMatrix_invertRotMatrix (&inverse, pMatrix);
    }




/*-----------------------------------------------------------------*//**
@description Inverts this instance matrix in place.
@instance pMatrix <=> The inverted matrix
@return true if the matrix is invertible.
@group "RotMatrix In Place Modification"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_invertInPlace


(
RotMatrixP pMatrix
)
    {
    return  bsiRotMatrix_invertRotMatrix (pMatrix, pMatrix);
    }




/*-----------------------------------------------------------------*//**
@description Applies scale factors to corresponding rows of the input matrix, and places the result
in this instance matrix.
@param pScaledMatrix <= scaled matrix
@param pInMatrix => The initial matrix
@param xScale => The scale factor for row 0
@param yScale => The scale factor for row 1
@param zScale => The scale factor for row 2
@group "RotMatrix Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_scaleRows


(
RotMatrixP pScaledMatrix,
RotMatrixCP pInMatrix,
double         xScale,
double         yScale,
double         zScale
)
    {
    int     i;

    if (pInMatrix)
        {
        *pScaledMatrix = *pInMatrix;
        for (i=0; i<3; i++)
            {
            pScaledMatrix->form3d[0][i] *= xScale;
            pScaledMatrix->form3d[1][i] *= yScale;
            pScaledMatrix->form3d[2][i] *= zScale;
            }
        }
    else
        {
        bsiRotMatrix_initFromScaleFactors (pScaledMatrix, xScale, yScale, zScale);
        }

    }




/*-----------------------------------------------------------------*//**
@description Applies scale factors to corresponding columns of the input matrix, and places the result
in this instance matrix.
@instance pScaledMatrix <= scaled matrix
@param pIn => The initial matrix
@param xs => The scale factor for column 0
@param ys => The scale factor for column 1
@param zs => The scale factor for column 2
@group "RotMatrix Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_scaleColumns


(
RotMatrixP pScaledMatrix,
RotMatrixCP pIn,
double         xs,
double         ys,
double         zs
)
    {
    pScaledMatrix->form3d[0][0] = pIn->form3d[0][0] * xs;
    pScaledMatrix->form3d[1][0] = pIn->form3d[1][0] * xs;
    pScaledMatrix->form3d[2][0] = pIn->form3d[2][0] * xs;

    pScaledMatrix->form3d[0][1] = pIn->form3d[0][1] * ys;
    pScaledMatrix->form3d[1][1] = pIn->form3d[1][1] * ys;
    pScaledMatrix->form3d[2][1] = pIn->form3d[2][1] * ys;

    pScaledMatrix->form3d[0][2] = pIn->form3d[0][2] * zs;
    pScaledMatrix->form3d[1][2] = pIn->form3d[1][2] * zs;
    pScaledMatrix->form3d[2][2] = pIn->form3d[2][2] * zs;
    }


/*-----------------------------------------------------------------*//**
@description Returns a matrix formed from a scaling matrix which is
multiplied on the left and/or right with other matrices.
That is, form LeftMatrix * ScaleMatrix * RightMatrix
where the ScaleMatrix is constructed from the given scale factors.
If LeftMatrix is null, this has the effect of scaling the rows
of the right matrix.  If RightMatrix is null this has the effect
of scaling the columns of the left matrix.
@instance pScaledMatrix <= scaled matrix
@param pLeftMatrix => The matrix on left of product
@param xs => The x scale factor
@param ys => The y scale factor
@param zs => The z scale factor
@param pRightMatrix => The matrix on right of product
@group "RotMatrix Multiplication"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixScaleRotMatrix


(
RotMatrixP pScaledMatrix,
RotMatrixCP pLeftMatrix,
double         xs,
double         ys,
double         zs,
RotMatrixCP pRightMatrix
)
    {
    if (pLeftMatrix && pRightMatrix)
        {
        RotMatrix intermediateMatrix;
        bsiRotMatrix_scaleColumns
                    (
                    &intermediateMatrix,
                    pLeftMatrix,
                    xs, ys, zs);
        bsiRotMatrix_multiplyRotMatrixRotMatrix (pScaledMatrix, &intermediateMatrix, pRightMatrix);

        }
    else if (pLeftMatrix)
        {
        bsiRotMatrix_scaleColumns (pScaledMatrix, pLeftMatrix, xs, ys, zs);
        }
    else if (pRightMatrix)
        {
        bsiRotMatrix_scaleRows (pScaledMatrix, pRightMatrix, xs, ys, zs);
        }
    else
        {
        bsiRotMatrix_initFromScaleFactors (pScaledMatrix, xs, ys, zs);
        }
    }






/*-----------------------------------------------------------------*//**
@description Returns the product {RX*RY*RZ*M} where RX, RY, and RZ are rotations (in radians)
around X, Y, and Z axes, and M is the input matrix.
@instance pRotatedMatrix <= rotated matrix
@param pInMatrix => The optional prior matrix
@param xrot => The x axis rotation
@param yrot => The y axis rotation
@param zrot => The z axis rotation
@group "RotMatrix Rotations"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_rotateByPrincipleAngles


(
RotMatrixP pRotatedMatrix,
RotMatrixCP pInMatrix,
double         xrot,
double         yrot,
double         zrot
)
    {
    RotMatrix factor;
    RotMatrix product;

    if (xrot != 0.0)
        {
        bsiRotMatrix_initFromAxisAndRotationAngle ( &product, 0, xrot);
        }
    else
        {
        bsiRotMatrix_initIdentity (&product);
        }

    if (yrot != 0.0)
        {
        bsiRotMatrix_initFromAxisAndRotationAngle ( &factor, 1, yrot);
        bsiRotMatrix_multiplyRotMatrixRotMatrix (&product, &product, &factor);
        }

    if (zrot != 0.0)
        {
        bsiRotMatrix_initFromAxisAndRotationAngle ( &factor, 2, zrot);
        bsiRotMatrix_multiplyRotMatrixRotMatrix (&product, &product, &factor);
        }


    if (pInMatrix)
        {
        bsiRotMatrix_multiplyRotMatrixRotMatrix (pRotatedMatrix, &product, pInMatrix);
        }
    else
        {
        *pRotatedMatrix = product;
        }
    }




/*-----------------------------------------------------------------*//**
@description Copies from columns of this instance matrix to corresponding points.
@instance pMatrix => The input matrix
@param pX <= first column
@param pY <= second column
@param pZ <= third column
@group "RotMatrix Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_getColumns


(
RotMatrixCP pMatrix,
DVec3dP pX,
DVec3dP pY,
DVec3dP pZ
)
    {
    if (pX)
        {
        pX->x = pMatrix->form3d[0][0];
        pX->y = pMatrix->form3d[1][0];
        pX->z = pMatrix->form3d[2][0];
        }

    if (pY)
        {
        pY->x = pMatrix->form3d[0][1];
        pY->y = pMatrix->form3d[1][1];
        pY->z = pMatrix->form3d[2][1];
        }

    if (pZ)
        {
        pZ->x = pMatrix->form3d[0][2];
        pZ->y = pMatrix->form3d[1][2];
        pZ->z = pMatrix->form3d[2][2];
        }
    }



/*-----------------------------------------------------------------*//**
@description Copies from rows of this instance matrix to corresponding points.
@instance pMatrix => The input matrix
@param pX <= first row
@param pY <= second row
@param pZ <= third row
@group "RotMatrix Queries"
 @bsimethod                                                     DavidAssaf      01/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_getRows


(
RotMatrixCP pMatrix,
DVec3dP pX,
DVec3dP pY,
DVec3dP pZ
)
    {
    if (pX)
        {
        pX->x = pMatrix->form3d[0][0];
        pX->y = pMatrix->form3d[0][1];
        pX->z = pMatrix->form3d[0][2];
        }

    if (pY)
        {
        pY->x = pMatrix->form3d[1][0];
        pY->y = pMatrix->form3d[1][1];
        pY->z = pMatrix->form3d[1][2];
        }

    if (pZ)
        {
        pZ->x = pMatrix->form3d[2][0];
        pZ->y = pMatrix->form3d[2][1];
        pZ->z = pMatrix->form3d[2][2];
        }
    }



/*-----------------------------------------------------------------*//**
@description Set the components in a column.
@instance pMatrix <=> The matrix to change.
@param pPoint => new values
@param col => The index of column to change. Column indices are 0, 1, 2.
@group "RotMatrix Initialization"
@group "RotMatrix In Place Modification"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_setColumn


(
RotMatrixP pMatrix,
DVec3dCP pPoint,
int            col
)
    {
    col = bsiGeom_cyclic3dAxis (col);
    pMatrix->form3d[0][col] = pPoint->x;
    pMatrix->form3d[1][col] = pPoint->y;
    pMatrix->form3d[2][col] = pPoint->z;
    }



/*-----------------------------------------------------------------*//**
@description Set the components in a row.
@instance pMatrix <=> The matrix to change.
@param pPoint => new values
@param row => The index of row to change. Row indices are 0, 1, 2.
@group "RotMatrix In Place Modification"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_setRow


(
RotMatrixP pMatrix,
DVec3dCP pPoint,
int            row
)
    {
    row = bsiGeom_cyclic3dAxis (row);

    if (row == 0)
        {
        pMatrix->form3d[0][0] = pPoint->x;
        pMatrix->form3d[0][1] = pPoint->y;
        pMatrix->form3d[0][2] = pPoint->z;
        }
    else if (row == 1)
        {
        pMatrix->form3d[1][0] = pPoint->x;
        pMatrix->form3d[1][1] = pPoint->y;
        pMatrix->form3d[1][2] = pPoint->z;
        }
    else if (row == 2)
        {
        pMatrix->form3d[2][0] = pPoint->x;
        pMatrix->form3d[2][1] = pPoint->y;
        pMatrix->form3d[2][2] = pPoint->z;
        }

    }

/* VBSUB(Point3dFromMatrix3dColumn) */
/* CSVFUNC(getColumn) */

/*-----------------------------------------------------------------*//**
@description Returns a point taken from a column of a matrix.
@instance pMatrix => The input matrix
@param pPoint <= filled point
@param col => The index of column to extract. Column indices are 0, 1, 2.
@group "RotMatrix Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_getColumn


(
RotMatrixCP pMatrix,
DVec3dP pPoint,
int            col
)
    {
    col = bsiGeom_cyclic3dAxis (col);
    pPoint->x = pMatrix->form3d[0][col];
    pPoint->y = pMatrix->form3d[1][col];
    pPoint->z = pMatrix->form3d[2][col];
    }

/* VBSUB(Matrix3dGetComponentByRowAndColumn) */


/*-----------------------------------------------------------------*//**
@description Returns a value from a specified row and column of the matrix.
@instance pMatrix => The input matrix.
@param row => The index of row to read. Row indices are 0, 1, 2.
@param col => The index of column to read.  Column indices are 0, 1, 2.
@return specified component of matrix
@group "RotMatrix Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRotMatrix_getComponentByRowAndColumn


(
RotMatrixCP pMatrix,
int            row,
int            col
)
    {
    double value;
    row = bsiGeom_cyclic3dAxis (row);
    col = bsiGeom_cyclic3dAxis (col);

#ifdef __RotMatrix__
    value = pMatrix->form3d[row][col];
#else
    value = bsiDVec3d_getComponent (&pMatrix->column[col], row);
#endif
    return value;
    }



/*-----------------------------------------------------------------*//**
@description Sets a value at a specified row and column of the matrix.
@instance pMatrix => The input matrix.
@param row => The index of row to read.  Row indices are 0, 1, and 2.
@param col => The index of column to read.  Column indices are 0, 1, and 2.
@param value  => the value to set.
@group "RotMatrix In Place Modification"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_setComponentByRowAndColumn


(
RotMatrixP pMatrix,
int            row,
int            col,
double         value
)
    {
    row = bsiGeom_cyclic3dAxis (row);
    col = bsiGeom_cyclic3dAxis (col);

#ifdef __RotMatrix__
    pMatrix->form3d[row][col] = value;
#else
    bsiDVec3d_setComponent (&pMatrix->column[col], value, row);
#endif
    }

/* VBSUB(Point3dFromMatrix3dRow) */
/* CSVFUNC(getRow) */

/*-----------------------------------------------------------------*//**
@description Returns a vector taken from a column of a matrix.
@instance pMatrix => The input matrix
@param pPoint <= filled point
@param row => The index of row to extract.  Row indices are 0, 1, and 2.
@group "RotMatrix Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_getRow


(
RotMatrixCP pMatrix,
DVec3dP pPoint,
int            row
)
    {
    row = bsiGeom_cyclic3dAxis (row);

    if ( row == 0)
        {
        pPoint->x = pMatrix->form3d[0][0];
        pPoint->y = pMatrix->form3d[0][1];
        pPoint->z = pMatrix->form3d[0][2];
        }
    else if (row == 1)
        {
        pPoint->x = pMatrix->form3d[1][0];
        pPoint->y = pMatrix->form3d[1][1];
        pPoint->z = pMatrix->form3d[1][2];
        }
    else if (row == 2)
        {
        pPoint->x = pMatrix->form3d[2][0];
        pPoint->y = pMatrix->form3d[2][1];
        pPoint->z = pMatrix->form3d[2][2];
        }
    }




/*-----------------------------------------------------------------*//**
@description Returns a matrix whose rows are unit vectors in the same
drection as corresponding rows of the input matrix.
Also (optionally) stores the original row magnitudes as components of the point.
@instance pMatrix <= matrix with normalized rows
@param pInMatrix => The input matrix
@param pScaleVector <= length of original rows
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_getNormalizedRows


(
RotMatrixP pMatrix,
RotMatrixCP pInMatrix,
DVec3dP pScaleVector
)
    {
    DVec3d xRow, yRow, zRow, mag;

    bsiRotMatrix_getRow (pInMatrix, &xRow, 0);
    mag.x = bsiDVec3d_magnitude (&xRow);
    bsiRotMatrix_getRow (pInMatrix, &yRow, 1);
    mag.y = bsiDVec3d_magnitude (&yRow);
    bsiRotMatrix_getRow (pInMatrix, &zRow, 2);
    mag.z = bsiDVec3d_magnitude (&zRow);

    if (pScaleVector)
        *pScaleVector = mag;

    if (pMatrix)
        {
        if (mag.x > 0.0)
            bsiDVec3d_scale (&xRow, &xRow, 1.0/mag.x);
        if (mag.y > 0.0)
            bsiDVec3d_scale (&yRow, &yRow, 1.0/mag.y);
        if (mag.z > 0.0)
            bsiDVec3d_scale (&zRow, &zRow, 1.0/mag.z);

        bsiRotMatrix_initFromRowVectors (pMatrix, &xRow, &yRow, &zRow);
        }
    }



/*-----------------------------------------------------------------*//**
@description Returns a matrix whose rows are unit vectors in the same
drection as corresponding columns of the input matrix.
Also (optionally) stores the original column magnitudes as components of the point.
@instance pMatrix <= matrix with normalized columns
@param pInMatrix => The input matrix
@param pScaleVector <= length of original columns
@group "RotMatrix Queries"
 @bsimethod                                                     DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_getNormalizedColumns


(
RotMatrixP pMatrix,
RotMatrixCP pInMatrix,
DVec3dP pScaleVector
)
    {
    DVec3d    xCol, yCol, zCol, mag;

    bsiRotMatrix_getColumn (pInMatrix, &xCol, 0);
    mag.x = bsiDVec3d_magnitude (&xCol);
    bsiRotMatrix_getColumn (pInMatrix, &yCol, 1);
    mag.y = bsiDVec3d_magnitude (&yCol);
    bsiRotMatrix_getColumn (pInMatrix, &zCol, 2);
    mag.z = bsiDVec3d_magnitude (&zCol);

    if (pScaleVector)
        *pScaleVector = mag;

    if (pMatrix)
        {
        if (mag.x > 0.0)
            bsiDVec3d_scale (&xCol, &xCol, 1.0/mag.x);
        if (mag.y > 0.0)
            bsiDVec3d_scale (&yCol, &yCol, 1.0/mag.y);
        if (mag.z > 0.0)
            bsiDVec3d_scale (&zCol, &zCol, 1.0/mag.z);

        bsiRotMatrix_initFromColumnVectors (pMatrix, &xCol, &yCol, &zCol);
        }
    }


/* VBSUB(Matrix3dDeterminant) */


/*-----------------------------------------------------------------*//**
@description Returns the determinant of the matrix.
@param pMatrix => The matrix to query
@return determinant of the matrix.
@group "RotMatrix Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRotMatrix_determinant


(
RotMatrixCP pMatrix
)
    {
    int i, j, k;
    double d = 0.0;
    for (i = 0; i < 3 ; i++)
        {
        j = (i + 1) % 3;
        k = (j + 1) % 3;
        d += pMatrix->form3d[0][i] *
                (  pMatrix->form3d[1][j] * pMatrix->form3d[2][k]
                 - pMatrix->form3d[2][j] * pMatrix->form3d[1][k]);
        }
    return d;
    }





/*-----------------------------------------------------------------*//**
Computes an estimate of the condition of this instance matrix.  Values near 0
are bad.
@param pMatrix => The input matrix
@return estimated condition number.
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRotMatrix_conditionNumber


(
RotMatrixCP pMatrix
)
    {
    RotMatrix rotation1, rotation2;
    DPoint3d scalePoint;

    bsiRotMatrix_factorRotateScaleRotate (pMatrix, &rotation1, &scalePoint, &rotation2);
    double aMax = fabs (scalePoint.x);
    double aMin = aMax;
    double a = fabs (scalePoint.y);
    double result;
    if (a > aMax)
        aMax = a;
    if (a < aMin)
        aMin = a;
    a = fabs (scalePoint.z);
    if (a > aMax)
        aMax = a;
    if (a < aMin)
        aMin = a;
    bsiTrig_safeDivide (&result, aMin, aMax, 0.0);
    return result;
    }



/* VBSUB(Matrix3dIsIdentity) */


/*-----------------------------------------------------------------*//**
@description Tests if a matrix is the identity matrix.
@param pMatrix => The matrix to test
@return true if matrix is approximately an identity.
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isIdentity


(
RotMatrixCP pMatrix
)
    {
    /* Entries in an identity matrix are strictly in the 0..1 range,*/
    /* hence a simple tolerance near machine precision is warranted.*/
    double tol = s_mediumRelTol;

    if (
           fabs( pMatrix->form3d[0][0] - 1.0 )   <= tol
        && fabs( pMatrix->form3d[1][0]       )   <= tol
        && fabs( pMatrix->form3d[2][0]       )   <= tol

        && fabs( pMatrix->form3d[0][1]       )   <= tol
        && fabs( pMatrix->form3d[1][1] - 1.0 )   <= tol
        && fabs( pMatrix->form3d[2][1]       )   <= tol
        && fabs( pMatrix->form3d[0][2]       )   <= tol
        && fabs( pMatrix->form3d[1][2]       )   <= tol
        && fabs( pMatrix->form3d[2][2] - 1.0 )   <= tol
        )
        return true;

    return false;
    }


/* VBSUB(Matrix3dIsDiagonal) */


/*-----------------------------------------------------------------*//**
@description Tests if a matrix has small offdiagonal entries compared to
diagonals.   The specific test condition is that the
largest off diagonal absolute value is less than a tight tolerance
fraction times the largest diagonal entry.
@param pMatrix => The matrix to test
@return true if matrix is approximately diagonal
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isDiagonal


(
RotMatrixCP pMatrix
)
    {
    double dmax, dmin;
    double smax, smin;
    bsiRotMatrix_diagonalAbsRange (pMatrix, &dmin, &dmax);
    bsiRotMatrix_offDiagonalAbsRange (pMatrix, &smin, &smax);

    return smax < s_mediumRelTol * dmax;
    }


/*-----------------------------------------------------------------*//**
@description Tests if a matrix has (nearly) equal diagaonal entries and


(nearly) zero off diagonals.  Tests use a tight relative tolerance.
@param pMatrix => The matrix to test
@param pMaxScale <= the largest diagaonal entry
@return true if matrix is approximately diagonal
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isUniformScale


(
RotMatrixCP pMatrix,
double        *pMaxScale
)
    {
    double dmax, dmin;
    double smax, smin;
    bool    isUniform = false;
    bsiRotMatrix_diagonalAbsRange (pMatrix, &dmin, &dmax);
    bsiRotMatrix_offDiagonalAbsRange (pMatrix, &smin, &smax);
    isUniform =  smax < s_mediumRelTol * dmax
              && dmax - dmin < s_mediumRelTol * dmax;
    copyDoubleThroughPointer (pMaxScale, dmax);
    return isUniform;
    }


/*-----------------------------------------------------------------*//**
@description Return the (signed) range of entries on the diagonal.
@param pMatrix => The matrix to test
@param pMinValue <= smallest signed value
@param pMaxValue <= largest signed value
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_diagonalSignedRange


(
RotMatrixCP pMatrix,
double              *pMinValue,
double              *pMaxValue
)
    {
    double minValue = pMatrix->form3d[0][0];
    double maxValue = minValue;
    updateExtrema (&minValue, &maxValue, pMatrix->form3d[1][1]);
    updateExtrema (&minValue, &maxValue, pMatrix->form3d[2][2]);
    copyDoubleThroughPointer (pMaxValue, maxValue);
    copyDoubleThroughPointer (pMinValue, minValue);
    }


/*-----------------------------------------------------------------*//**
@description Return the (absolute value) range of entries on the diagonal.
@param pMatrix => The matrix to test
@param pMinValue <= smallest absolute value
@param pMaxValue <= largest absolute value
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_diagonalAbsRange


(
RotMatrixCP pMatrix,
double              *pMinValue,
double              *pMaxValue
)
    {
    double minValue = fabs (pMatrix->form3d[0][0]);
    double maxValue = minValue;
    updateExtrema (&minValue, &maxValue, fabs (pMatrix->form3d[1][1]));
    updateExtrema (&minValue, &maxValue, fabs (pMatrix->form3d[2][2]));
    copyDoubleThroughPointer (pMaxValue, maxValue);
    copyDoubleThroughPointer (pMinValue, minValue);
    }


/*-----------------------------------------------------------------*//**
@description Return the (signed) range of entries off the diagonal.
@param pMatrix => The matrix to test
@param pMinValue <= smallest signed value
@param pMaxValue <= largest signed value
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_offDiagonalSignedRange


(
RotMatrixCP pMatrix,
double              *pMinValue,
double              *pMaxValue
)
    {
    double minValue = pMatrix->form3d[1][0];
    double maxValue = minValue;
    updateExtrema (&minValue, &maxValue, pMatrix->form3d[2][0]);
    updateExtrema (&minValue, &maxValue, pMatrix->form3d[0][1]);
    updateExtrema (&minValue, &maxValue, pMatrix->form3d[2][1]);
    updateExtrema (&minValue, &maxValue, pMatrix->form3d[1][2]);
    updateExtrema (&minValue, &maxValue, pMatrix->form3d[0][2]);
    copyDoubleThroughPointer (pMaxValue, maxValue);
    copyDoubleThroughPointer (pMinValue, minValue);
    }


/*-----------------------------------------------------------------*//**
@description Return the (absolute value) range of entries off the diagonal.
@param pMatrix => The matrix to test
@param pMinValue <= smallest absolute value
@param pMaxValue <= largest absolute value
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_offDiagonalAbsRange


(
RotMatrixCP pMatrix,
double              *pMinValue,
double              *pMaxValue
)
    {
    double minValue = fabs (pMatrix->form3d[0][1]);
    double maxValue = minValue;
    updateExtrema (&minValue, &maxValue, fabs (pMatrix->form3d[0][2]));
    updateExtrema (&minValue, &maxValue, fabs (pMatrix->form3d[1][0]));
    updateExtrema (&minValue, &maxValue, fabs (pMatrix->form3d[1][2]));
    updateExtrema (&minValue, &maxValue, fabs (pMatrix->form3d[2][0]));
    updateExtrema (&minValue, &maxValue, fabs (pMatrix->form3d[2][1]));

    copyDoubleThroughPointer (pMaxValue, maxValue);
    copyDoubleThroughPointer (pMinValue, minValue);
    }

/* VBSUB(Matrix3dIsSignedPermutation) */


/*-----------------------------------------------------------------*//**
@description Test if this instance matrix does nothing more
than exchange and possibly negate principle axes.
@param pMatrix => The input matrix
@return true if the matrix is a permutation of the principle axes.
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isSignedPermutation


(
RotMatrixCP pMatrix
)
    {
    double zeroTol = 1.0e-14;
    double a, b, c, d;

    int k, k1, k2;

    /* Is the x row two zeros and a +-1?*/
    a = fabs (pMatrix->form3d[0][0]);
    b = fabs (pMatrix->form3d[0][1]);
    c = fabs (pMatrix->form3d[0][2]);

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
    if (   fabs (pMatrix->form3d[1][k]) > zeroTol
        || fabs (pMatrix->form3d[2][k]) > zeroTol)
        return false;

    a = fabs(pMatrix->form3d[1][k1]);
    b = fabs(pMatrix->form3d[1][k2]);
    c = fabs(pMatrix->form3d[2][k1]);
    d = fabs(pMatrix->form3d[2][k2]);

    /* The valid configurations of the 2x2 matrix after eliminating*/
    /* row zero and column k are of the form*/
    /*   [ a b ]    =       [ 0 1 ]   or    [ 1 0 ]*/
    /*   [ c d ]            [ 1 0 ]         [ 0 1 ]*/

    if ( a < zeroTol)
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



/* VBSUB(Matrix3dIsRigid) */


/*-----------------------------------------------------------------*//**
@description Test if a matrix is a rigid body rotation,
i.e. its transpose is its inverse and it has a positive determinant.
@instance pMatrix => The matrix to test
@return true if the matrix is a rigid body rotation.
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isRigid


(
RotMatrixCP pMatrix
)
    {
    RotMatrix transpose;
    RotMatrix product;
    bsiRotMatrix_transpose (&transpose, pMatrix);
    bsiRotMatrix_multiplyRotMatrixRotMatrix (&product, &transpose, pMatrix);
    return      bsiRotMatrix_isIdentity (&product)
            &&  (bsiRotMatrix_determinant (pMatrix) > 0.0);
    }

/* VBSUB(Matrix3dIsOrthogonal) */


/*-----------------------------------------------------------------*//**
@description Test if this instance matrix is orthogonal, i.e. its transpose is its inverse.
This class of matrices includes both rigid body rotations and reflections.
@instance pMatrix => The matrix to test
@return true if the matrix is orthogonal.
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isOrthogonal


(
RotMatrixCP pMatrix
)
    {
    RotMatrix transpose;
    RotMatrix product;
    bsiRotMatrix_transpose (&transpose, pMatrix);
    bsiRotMatrix_multiplyRotMatrixRotMatrix (&product, &transpose, pMatrix);
    return      bsiRotMatrix_isIdentity (&product);
    }


/*-----------------------------------------------------------------*//**
@description Test if this instance matrix has orthonormal columns, i.e. its columns
are all perpendicular to one another.
@instance pMatrix => The matrix to test
@param pColumns <= (optional) matrix containing the unit vectors along the columns.
@param pAxisScales <= (optional) point whose x, y, and z components are the magnitudes of the
original columns.
@param pAxisRatio <= (optional) smallest axis length divided by largest.
@return true if the matrix is orthonormal.
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_areColumnsOrthonormal


(
RotMatrixCP pMatrix,
RotMatrixP pColumns,
DVec3dP pAxisScales,
double      *pAxisRatio
)
    {
    DVec3d axisScales;
    DVec3d column[3];
    RotMatrix matrixR, matrixRT, matrixRTR;
    double aMin, aMax;

    bsiRotMatrix_getColumns (pMatrix, &column[0], &column[1], &column[2]);
    axisScales.x = bsiDVec3d_normalizeInPlace (&column[0]);
    axisScales.y = bsiDVec3d_normalizeInPlace (&column[1]);
    axisScales.z = bsiDVec3d_normalizeInPlace (&column[2]);
    bsiRotMatrix_initFromColumnVectors (&matrixR, &column[0], &column[1], &column[2]);
    bsiRotMatrix_transpose (&matrixRT, &matrixR);
    bsiRotMatrix_multiplyRotMatrixRotMatrix (&matrixRTR, &matrixRT, &matrixR);

    if (pColumns)
        *pColumns = matrixR;
    if (pAxisScales)
        *pAxisScales = axisScales;

    if (pAxisRatio)
        {
        aMin = aMax = axisScales.x;

        if (axisScales.y < aMin)
            aMin = axisScales.y;
        else if (axisScales.y > aMax)
            aMax = axisScales.y;

        if (axisScales.z < aMin)
            aMin = axisScales.z;
        else if (axisScales.z > aMax)
            aMax = axisScales.z;

        if (aMax > 0.0)
            *pAxisRatio = aMin / aMax;
        else
            *pAxisRatio = 0.0;
        }

    return      bsiRotMatrix_isIdentity (&matrixRTR);
    }


/*-----------------------------------------------------------------*//**
@description Test if this instance matrix is composed of only rigid rotation and a uniform
positive scale.
@instance pMatrix => The matrix to test
@param pColumns <= (optional) matrix containing the unit vectors along the columns.
@param pScale <= largest axis scale factor.  If function value is true,
    the min scale is the same.  Use areColumnsOrthonormal to get
    separate column scales.
@return true if the matrix is orthonormal with positive determinant and same length of all columns.
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isRigidScale


(
RotMatrixCP pMatrix,
RotMatrixP pColumns,
double      *pScale
)
    {
    double almostOne = 1.0 - s_mediumRelTol;
    DVec3d axisScales;
    double ratio;

    bool    ortho = bsiRotMatrix_areColumnsOrthonormal (pMatrix, pColumns,
                        &axisScales, &ratio);

    if (pScale)
        {
        *pScale = axisScales.x;
        if (axisScales.y > *pScale)
            *pScale = axisScales.y;
        if (axisScales.z > *pScale)
            *pScale = axisScales.z;
        }

    return ortho && ratio > almostOne && bsiRotMatrix_determinant (pMatrix) > 0.0;
    }



/*-----------------------------------------------------------------*//**
@description Tests if this instance matrix has no effects perpendicular to any plane with the given normal.  This
will be true if the matrix represents a combination of (a) scaling perpencicular to the normal
and (b) rotation around the normal.

@instance pMatrix => The input matrix
@param pNormal => The plane normal
@return true if the matrix has no effects perpendicular to any plane
with the given normal.
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isPlanar


(
RotMatrixCP pMatrix,
DVec3dCP pNormal
)
    {
    DVec3d vector;
    double norm2, vec2;
    double tolSquared = 1.0E-20;
    vector = *pNormal;
    bsiRotMatrix_multiplyTransposeDPoint3d (pMatrix, &vector);
    bsiDVec3d_subtractDPoint3dDPoint3d (&vector, &vector, pNormal);
    norm2 = bsiDVec3d_dotProduct (pNormal, pNormal);
    vec2  = bsiDVec3d_dotProduct (&vector, &vector);
    return vec2 <= tolSquared * norm2;
    }

/*======================================================================+
|SECTION complexInitializations Initialization from points and vectors  |
| These functions initialize so that the coordinate system described    |
| by a matrix or transform aligns with given points and vectors.        |
+======================================================================*/




/*-----------------------------------------------------------------*//**
@description Initializes this instance matrix so that the indicated axis (axis = 0,1,or 2)
    is aligned with the vector pDir.   The normalize flag selects between
    normalized axes (all matrix columns of unit length) and unnormalized
    axes (all matrix columns of same length as the pDir vector).
@instance pMatrix <= computed matrix
@param pDir => The fixed direction vector
@param axis => The axis column to be aligned with direction
@param normalize => true to have normalized columns
@return true if the direction vector is nonzero.
@group "RotMatrix Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_initFrom1Vector


(
RotMatrixP pMatrix,
DVec3dCP pDir,
int            axis,
bool               normalize
)
    {
    int ix, iy, iz;
    bool    boolStat;
    DVec3d vector[3];

    bsiGeom_cyclic3dAxes (&iz, &ix, &iy, axis);


    if (normalize)
        {
        boolStat = bsiDVec3d_getNormalizedTriad(pDir,
                        &vector[ix], &vector[iy], &vector[iz]);
        }
    else
        {
        boolStat = bsiDVec3d_getTriad(pDir,
                        &vector[ix], &vector[iy], &vector[iz]);
        }
    bsiRotMatrix_initFromColumnVectors (pMatrix, &vector[0], &vector[1], &vector[2]);
    return  boolStat;
    }




/*-----------------------------------------------------------------*//**
@description Copies vectors into the x and y columns of this instance matrix
and sets the z column to their cross product.
Use squareAndNormalizeColumns to force these (possibly
skewed and scaled) coordinate axes to be perpendicular
* @instance pMatrix <= computed matrix
@param pXVector => The vector to place in column 0
@param pYVector => The vector to place in column 1
@group "RotMatrix Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initFrom2Vectors


(
RotMatrixP pMatrix,
DVec3dCP pXVector,
DVec3dCP pYVector
)
    {
    DVec3d WVector;
    bsiDVec3d_crossProduct (&WVector, pXVector, pYVector);
    bsiRotMatrix_initFromColumnVectors (pMatrix, pXVector, pYVector, &WVector);
    }

/* VBSUB(Matrix3dRotationFromPoint3dOriginXY) */

/*-----------------------------------------------------------------*//**
@description Set this instance matrix to be an orthogonal (rotation) matrix with column 0 in the direction
from the origin to the x point, column 1 in the plane of the 3 points, directed so the Y point
on the positive side, and column 2 as their cross product.
@instance pMatrix <= computed matrix
@param pOrigin => The reference point
@param pXPoint => The x axis target point
@param pYPoint => The 3rd point defining xy plane.
@return true if the 3 points are non-colinear
@group "RotMatrix Rotations"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_initRotationFromOriginXY


(
RotMatrixP pMatrix,
DPoint3dCP pOrigin,
DPoint3dCP pXPoint,
DPoint3dCP pYPoint
)
    {
    DVec3d U, V, W;
    double a;
    double relTol = s_mediumRelTol;
    bsiDVec3d_computeNormal (&U, pXPoint, pOrigin);
    bsiDVec3d_computeNormal (&V, pYPoint, pOrigin);
    a = bsiDVec3d_normalizedCrossProduct (&W, &U, &V);

    if  (a <= relTol)
        {
        bsiRotMatrix_initIdentity (pMatrix);
        return  false;
        }
    else
        {
        bsiDVec3d_normalizedCrossProduct (&V, &W, &U);
        bsiRotMatrix_initFromColumnVectors (pMatrix, &U, &V,&W);
        return  true;
        }
    }




/*-----------------------------------------------------------------*//**
@description Adjust the direction and length of columns of the input matrix to
produce an instance matrix which has perpendicular, unit length columns.
The column whose index is primaryAxis (i.e. 0,1,2 for x,y,z axis of
coordinate frame) is normalized to unit length in its current
direction.
The column whose index is secondaryAxis is unit length and perpendicular
to the primaryAxis column, and lies in the same plane as that
defined by the original primary and secondary columns.
To preserve X axis and XY plane, call with axis id's 0 and 1.
To preserve Z axis and ZX plane, call with axis id's 2 and 0.
pInMatrix and pMatrix may be the same address.
* @instance pMatrix <= normalized matrix
@param pInMatrix => The input matrix
@param primaryAxis => The axis id (0, 1, 2) which is to be normalized but left
in its current direction
@param secondaryAxis => The axis id (0, 1, 2) which is to be kept within the
plane of the primary and secondary axis.
@return false if primaryAxis and secondaryAxis are the same, or either axis has zero length
@group "RotMatrix Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_squareAndNormalizeColumns


(
RotMatrixP pMatrix,
RotMatrixCP pInMatrix,
int            primaryAxis,
int            secondaryAxis
)
    {
    int ix = bsiGeom_cyclic3dAxis (primaryAxis);
    int iy = bsiGeom_cyclic3dAxis (secondaryAxis);
    int iz = 3 - (ix + iy);
    double lx, ly;
    DVec3d vector[3];
    bool    boolStat = false;

    if (ix == iy)
        {
        *pMatrix = *pInMatrix;
        return false;
        }

    bsiRotMatrix_getColumns (pInMatrix, &vector[0], &vector[1], &vector[2]);

    lx = bsiDVec3d_normalizeInPlace (&vector[ix]);
    bsiDVec3d_normalizedCrossProduct (&vector[iz], &vector[ix], &vector[iy]);
    ly = bsiDVec3d_normalizedCrossProduct (&vector[iy], &vector[iz], &vector[ix]);

    if (ly == 0.0)
        {
        if (lx == 0.0)
            {
            bsiRotMatrix_initIdentity (pMatrix);
            boolStat = false;
            }
        else
            {
            bsiDVec3d_getNormalizedTriad (&vector[ix], &vector[iy], &vector[iz], &vector[ix]);
            bsiRotMatrix_initFromColumnVectors (pMatrix, &vector[0], &vector[1], &vector[2]);
            boolStat = false;
            }
        }
    else
        {
        /* negate z-axis if x follows y (this prevents negative determinant) */
        if ((iy + 1) % 3 == ix)
            bsiDVec3d_negateInPlace (&vector[iz]);
        bsiRotMatrix_initFromColumnVectors (pMatrix, &vector[0], &vector[1], &vector[2]);
        boolStat = true;
        }

    return boolStat;
    }





/*-----------------------------------------------------------------*//**
@description Returns an orthogonal matrix that preserves aligns with
the columns of pInMatrix.  This is done by
trying various combinations of primary and secondary axes until one
succeeds in squareAndNormalizeColumns.
* @param pMatrix <= normalized matrix
@param pInMatrix => The input matrix
@return true if a valid pair of primary and secondary axes was found.
@group "RotMatrix Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_squareAndNormalizeColumnsAnyOrder


(
RotMatrixP pMatrix,
RotMatrixCP pInMatrix
)
    {
    RotMatrix tempMatrix;
    bool    boolStat;
    int i, j;

    boolStat = false;
    tempMatrix = *pInMatrix;
    for (i = 0; i < 3 && !boolStat; i++)
        {
        j = (i + 1) % 3;
        boolStat = bsiRotMatrix_squareAndNormalizeColumns(pMatrix,
                        &tempMatrix,
                        i, j);
        }
    return boolStat;
    }






/*-----------------------------------------------------------------*//**
@description Moves columns 0, 1, 2 of the input matrix into columns i0, i1, i2
of the instance.
@param pMatrix <= shuffled matrix
@param pInMatrix => The input matrix
@param i0 => The column to receive input column 0
@param i1 => The column to receive input column 1
@param i2 => The column to receive input column 2
@group "RotMatrix Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_shuffleColumns


(
RotMatrixP pMatrix,
RotMatrixCP pInMatrix,
int            i0,
int            i1,
int            i2
)
    {
    RotMatrix A = *pInMatrix;
    int index[3];
    int i, j;
    index[0] = bsiGeom_cyclic3dAxis (i0);
    index[1] = bsiGeom_cyclic3dAxis (i1);
    index[2] = bsiGeom_cyclic3dAxis (i2);

    for (i = 0; i < 3; i++)
        {
        j = index[i];
        pMatrix->form3d[0][j] = A.form3d[0][i];
        pMatrix->form3d[1][j] = A.form3d[1][i];
        pMatrix->form3d[2][j] = A.form3d[2][i];
        }
    }


/*======================================================================+
| Additive operations                                                   |
| Addition/subtraction of matrices is fairly unusual.   (e.g. a test    |
| program may want to compare 2 results of different computations.)     |
| The strategy in the following routines is to have a small number      |
| of routines with complex argument lists that can be used in many      |
| ways by an informed user, rather than the multiple-function approach  |
| used in the broad-usage functions above.                              |
+======================================================================*/

/* VBSUB(Matrix3dAdd2Scaled) */
/* CSVFUNC(add) */

/*-----------------------------------------------------------------*//**
@description Returns {Matrix0 + Matrix1*s1+Matrix2*s2}, i.e. the sum of matrix M0,
matrix M1 multiplied by scale s1, and matrix M2 multiplied by scale s2.
Any combination of the matrix pointers may have identical addresses.
Any of the input matrix pointers may be NULL.
@instance pSum <= sum of matrices
@param pMatrix0 => The matrix0 of formula
@param pMatrix1 => The matrix1 of formula
@param scale1 => The scale factor to apply to Matrix1
@param pMatrix2 => The matrix2 of formula
@param scale2 => The scale factor to apply to Matrix2
@group "RotMatrix Addition"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_add2ScaledRotMatrix


(
RotMatrixP pSum,
RotMatrixCP pMatrix0,
RotMatrixCP pMatrix1,
double         scale1,
RotMatrixCP pMatrix2,
double         scale2
)
    {
    int i;
    RotMatrix sum;
    double *pFrom, *pTo;


    if (pMatrix0)
        sum = *pMatrix0;
    else
        bsiRotMatrix_zero (&sum);


    if (pMatrix1)
        {
        for (i = 0, pFrom = (double *)pMatrix1, pTo = (double *)&sum;
             i < 9; i++, pFrom++, pTo++)
                {
                *pTo += *pFrom * scale1;
                }
        }

    if (pMatrix2)
        {
        for (i = 0, pFrom = (double *)pMatrix2, pTo = (double *)&sum;
             i < 9; i++, pFrom++, pTo++)
                {
                *pTo += *pFrom * scale2;
                }
        }
    *pSum = sum;
    }



/*-----------------------------------------------------------------*//**
@description Add a matrix (componentwise, in place).
* @instance pMatrix  <=> The  pMatrix + pDelta
@param pDelta => The matrix to add
@group "RotMatrix Addition"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_addRotMatrixInPlace


(
RotMatrixP pMatrix,
RotMatrixCP pDelta
)
    {
    int i;
    double *pA = (double *)pMatrix;
    double *pB = (double *)pDelta;
    for (i = 0; i < 9; i++, pA++, pB++)
        {
        *pA += *pB;
        }
    }



/*-----------------------------------------------------------------*//**
@description Subtract a matrix (componentwise, in place).
* @instance pMatrix  <= pMatrix - pDelta
@param pDelta => The matrix to subtract
@group "RotMatrix Addition"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_subtract


(
RotMatrixP pMatrix,
RotMatrixCP pDelta
)
    {
    int i;
    double *pA = (double *)pMatrix;
    double *pB = (double *)pDelta;
    for (i = 0; i < 9; i++, pA++, pB++)
        {
        *pA -= *pB;
        }
    }


/* VBSUB(Matrix3dSumSquares) */


/*-----------------------------------------------------------------*//**
@description Return the sum of squares of coefficients in a matrix.
@instance pMatrix => The matrix to sum
@return Sum of squares of all entries in matrix
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRotMatrix_sumSquares


(
RotMatrixCP pMatrix
)
    {
    int i;
    double *pij = (double *) pMatrix;
    double sum = 0.0;

    for (i = 0; i < 9; i++, pij++)
        {
        sum += *pij * *pij;
        }

    return  sum;
    }



/* VBSUB(Matrix3dMaxAbs) */


/*-----------------------------------------------------------------*//**
@description Find the largest absolute value of entries in the matrix.
@instance pMatrix => The matrix to inspect
@return largest absolute value in matrix
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRotMatrix_maxAbs


(
RotMatrixCP pMatrix
)
    {
    int i;
    double *pij = (double *) pMatrix;
    double maxAbs = 0.0;

    for (i = 0; i < 9; i++, pij++)
        {
        if (fabs (*pij) > maxAbs)
            maxAbs = fabs (*pij);
        }
    return  maxAbs;
    }

/* VBSUB(Matrix3dMaxDiff) */
/* CSVFUNC(maxDiff) */

/*-----------------------------------------------------------------*//**
@description Returns the largest absolute value difference between
corresponding coefficients in Matrix1 and Matrix2.
@instance pMatrix1 => The matrix to inspect
@param pMatrix2 => The matrix to compare to
@return largest absolute difference between the two matrices.
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRotMatrix_maxDiff


(
RotMatrixCP pMatrix1,
RotMatrixCP pMatrix2
)
    {
    int i;
    double *pA = (double *) pMatrix1;
    double *pB = (double *) pMatrix2;
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
@description Computes the sum of the squares of the diagonal entries of this instance matrix.
@instance pMatrix => The matrix to inspect
@return Sum of squares of diagonal entries
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRotMatrix_sumDiagonalSquares


(
RotMatrixCP pMatrix
)
    {
    return
        pMatrix->form3d[0][0] * pMatrix->form3d[0][0] +
        pMatrix->form3d[1][1] * pMatrix->form3d[1][1] +
        pMatrix->form3d[2][2] * pMatrix->form3d[2][2]
        ;
    }






/*-----------------------------------------------------------------*//**
@description Computes the sum of the squares of the off-diagonal entries of this instance matrix.
@instance pMatrix => The matrix to inspect
@return sum of square of off-diagonal entries of the matrix.
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRotMatrix_sumOffDiagonalSquares


(
RotMatrixCP pMatrix
)
    {
    return
        pMatrix->form3d[1][0] * pMatrix->form3d[1][0] +
        pMatrix->form3d[2][0] * pMatrix->form3d[2][0] +
        pMatrix->form3d[0][1] * pMatrix->form3d[0][1] +
        pMatrix->form3d[2][1] * pMatrix->form3d[2][1] +
        pMatrix->form3d[0][2] * pMatrix->form3d[0][2] +
        pMatrix->form3d[1][2] * pMatrix->form3d[1][2]
        ;
    }






extern double computeQuatTerm (
double numerator,
double denomCoff,
double reciprocal,
double diagSum
);



/*-----------------------------------------------------------------*//**
@instance pMatrix  => The matrix to be converted to quaternion
@param pQuatAsDoubleArray <= quaternion, stored as (w,x,y,z) in an array of doubles.
@param   transpose => true if matrix is stored transposed
@group Quaternions
@bsimethod                                                      EarlinLutz      3/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_toQuaternionAsDoubleArray


(
RotMatrixCP pMatrix,
double              *pQuatAsDoubleArray,
bool                transpose
)
    {
    DPoint4d quat;
    bsiRotMatrix_toQuaternion (pMatrix, &quat, transpose);
    pQuatAsDoubleArray[0] = quat.w;
    pQuatAsDoubleArray[1] = quat.x;
    pQuatAsDoubleArray[2] = quat.y;
    pQuatAsDoubleArray[3] = quat.z;
    }


/*-----------------------------------------------------------------*//**
@instance pMatrix <= matrix computed from quaternion
@param pQuatAsDoubleArray => The quaternion, stored as (w,x,y,z) in an array of doubles.
@group Quaternions
@bsimethod                                                      EarlinLutz      3/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_fromQuaternionAsDoubleArray


(
RotMatrixP pMatrix,
const double        *pQuatAsDoubleArray
)
    {
    DPoint4d quat;
    quat.x = pQuatAsDoubleArray[1];
    quat.y = pQuatAsDoubleArray[2];
    quat.z = pQuatAsDoubleArray[3];
    quat.w = pQuatAsDoubleArray[0];
    bsiRotMatrix_fromQuaternion (pMatrix, &quat);
    }

/*---------------------------------------------------------------------------------**//**
* @description Implementation of slerp: spherical linear interpolation of quaternions.
* @remarks It is assumed that the input matrices are pure rotations.
* @param pSum               OUT     The interpolated rotation
* @param pMatrix0           IN      The rotation corresponding to fractionParameter of 0.0
* @param fractionParameter  IN      The interpolation parameter in [0,1]
* @param pMatrix1           IN      The rotation corresponding to fractionParameter of 1.0
* @bsimethod                                                    DavidAssaf      07/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiRotMatrix_interpolateRotations


(
RotMatrixP   pSum,
RotMatrixCP   pRotation0,
double      fractionParameter,
RotMatrixCP   pRotation1
)
    {
    DPoint4d    q0, q1, q;
    bsiRotMatrix_toQuaternion (pRotation0, &q0, false);
    bsiRotMatrix_toQuaternion (pRotation1, &q1, false);
    bsiDPoint4d_interpolateQuaternions (&q, &q0, fractionParameter, &q1);
    bsiRotMatrix_fromQuaternion (pSum, &q);
    }



/*-----------------------------------------------------------------*//**
Returns the angle of rotation of this instance and sets pAxis to be the
normalized vector about which this instance rotates.
NOTE: this instance is assumed to be a (rigid body, i.e. orthogonal) rotation
matrix.
Since negating both angle and axis produces an identical rotation,
calculations are simplified by assuming (and returning) the angle in [0,Pi].
* @instance pM      => The input matrix (assumed to be orthogonal)
@param    pAxis   <= normalized axis of rotation
@return rotation angle (in radians) between 0 and Pi, inclusive
@group "RotMatrix Rotations"
 @bsimethod                                     DavidAssaf      6/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRotMatrix_getRotationAngleAndVector


(
RotMatrixCP pM,
DVec3dP pAxis
)
    {
    DPoint4d        quat;

    bsiRotMatrix_toQuaternion (pM, &quat, false);
    return bsiDPoint4d_getRotationAngleAndVectorFromQuaternion (&quat, pAxis);
    }




/*-----------------------------------------------------------------*//**
Sets this instance matrix by copying the matrix part of the trasnform.
@param pMatrix <= matrix part of transformation
@param pTransform => The transformation whose matrix part is returned
@group "RotMatrix Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromTransform


(
RotMatrixP pMatrix,
TransformCP pTransform
)
    {
#ifdef __RotMatrix__
    int i, j;
    for (i = 0; i < 3; i++)
        {
        for (j = 0; j < 3; j++)
            {
            pMatrix->form3d[i][j] = pTransform->form3d[i][j];
            }
        }
#else
    *pMatrix = pTransform->matrix;
#endif
    }



/*-----------------------------------------------------------------*//**
@description Sets this instance matrix by copying from the matrix parameter.
@instance pMatrix  <= copied data
@param pIn => The source matrix
@group "RotMatrix Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_copy


(
RotMatrixP pMatrix,
RotMatrixCP pIn
)
    {
    if  (pMatrix)
        if  (pIn)
            *pMatrix = *pIn;
        else
            bsiRotMatrix_initIdentity (pMatrix);
    }

/* VBSUB(Matrix3dEqual) */
/* CSVFUNC(isEqual) */

/*-----------------------------------------------------------------*//**
@description Tests for equality between two matrices
"Equality" means relative error less than 1.0e-12, in the sense that each
component-wise difference is less than 1.0e-12 times the largest absolute
value of the components of one matrix.
* @instance pMatrix1 => The first matrix
@param    pMatrix2   => The second matrix
@return   true if the matrices are identical.
@group "RotMatrix Queries"
@bsimethod                                                      DavidAssaf      6/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_matrixEqual


(
RotMatrixCP pMatrix1,
RotMatrixCP pMatrix2
)
    {
    double tol = 1.0e-12;
    bool    result;

    if (pMatrix1 && pMatrix2)
        {
        result = (bsiRotMatrix_maxDiff (pMatrix1, pMatrix2)
                    <= tol * bsiRotMatrix_maxAbs (pMatrix1) );
        }
    else if (pMatrix1)
        {
        result = (bsiRotMatrix_maxAbs (pMatrix1) <= tol);
        }
    else if (pMatrix2)
        {
        result = (bsiRotMatrix_maxAbs (pMatrix2) <= tol);
        }
    else /* Both are null. Call them equal. */
        {
        result = true;
        }
    return result;
    }

/* VBSUB(Matrix3dEqualTolerance) */
/* CSVFUNC(isEqual) */

/*-----------------------------------------------------------------*//**
@description Tests for equality between two matrices.
* @instance pMatrix1 => The first matrix
@param    pMatrix2   => The second matrix
@param    tolerance => The relative error tolerance.  Absolute tolerance is
this (relative) tolerance times the largest absolute value in Matrix1.
@return true if the matrices are identical within tolerance.
@group "RotMatrix Queries"
@bsimethod                                                      DavidAssaf      6/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_matrixEqualTolerance


(
RotMatrixCP pMatrix1,
RotMatrixCP pMatrix2,
double              tolerance
)
    {
    bool    result;

    if (pMatrix1 && pMatrix2)
        {
        result = (bsiRotMatrix_maxDiff (pMatrix1, pMatrix2)
                    <= tolerance * bsiRotMatrix_maxAbs (pMatrix1) );
        }
    else if (pMatrix1)
        {
        result = (bsiRotMatrix_maxAbs (pMatrix1) <= tolerance);
        }
    else if (pMatrix2)
        {
        result = (bsiRotMatrix_maxAbs (pMatrix2) <= tolerance);
        }
    else /* Both are null. Call them equal. */
        {
        result = true;
        }
    return result;
    }


/*---------------------------------------------------------------------------------**//**
Apply a Givens "row operation", i.e. pre-multiply by a Givens rotation matrix.
The Givens matrix is an identity except for the 4 rotational entries, viz
R(i0,i0)=R(i1,i1)=c
R(i0,i1)=s
R(i1,i0)=-s
* @instance pProduct      <= product of givens rotation and input matrix.
@param    c   => The cosine of givens rotation.
@param    s   => The sine of givens rotation.
@param    i0  => The index of the first affected row.
@param    i1  => The index of the second affected row.
@param    pMatrix => The input matrix
@group "RotMatrix Rotations"
@bsimethod                                                      EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void        bsiRotMatrix_givensRowOp


(
RotMatrixP pProduct,
double        c,
double        s,
int           i0,
int           i1,
RotMatrixCP pMatrix
)
    {
    int j;
    double a, b;
    i0 = bsiGeom_cyclic3dAxis (i0);
    i1 = bsiGeom_cyclic3dAxis (i1);

#ifdef __RotMatrix__
    for (j = 0; j < 3; j++)
        {
        a = pProduct->form3d[i0][j];
        b = pProduct->form3d[i1][j];
        pProduct->form3d[i0][j] = a * c - b * s;
        pProduct->form3d[i1][j] = b * c + a * s;
        }
#else
    for (j = 0; j < 3; j++)
        {
        double *pCol = (double *)&pMatrix->column[j];
        a = pCol[i0];
        b = pCol[i1];
        pCol[i0] = a * c - b * s;
        pCol[i1] = b * c + a * s;
        }
#endif
    }


/*---------------------------------------------------------------------------------**//**
Apply a Givens "column operation", i.e. post-multiply by a Givens rotation matrix.
The Givens matrix is an identity except for the 4 rotational entries, viz
R(i0,i0)=R(i1,i1)=c
R(i0,i1)=-s
R(i1,i0)=s
* @instance pProduct      <= product of matrix and a givens rotation.
@param    pMatrix => The input matrix
@param    c   => The cosine of givens rotation.
@param    s   => The sine of givens rotation.
@param    i0  => The index of the first affected row.
@param    i1  => The index of the second affected row.
@group "RotMatrix Rotations"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void        bsiRotMatrix_givensColumnOp


(
RotMatrixP pProduct,
RotMatrixCP pMatrix,
double        c,
double        s,
int           i0,
int           i1
)
    {
    int i;
    double a, b;
    i0 = bsiGeom_cyclic3dAxis (i0);
    i1 = bsiGeom_cyclic3dAxis (i1);

#ifdef __RotMatrix__
    for (i = 0; i < 3; i++)
        {
        a = pProduct->form3d[i][i0];
        b = pProduct->form3d[i][i1];
        pProduct->form3d[i][i0] = a * c + b * s;
        pProduct->form3d[i][i1] = b * c - a * s;
        }
#else
        {
        double *pCol0 = (double *) &pMatrix->column[i0];
        double *pCol1 = (double *) &pMatrix->column[i1];

        for (i = 0; i < 3; i++)
            {
            a = pCol0[i];
            b = pCol1[i];
            pCol0[i] = a * c + b * s;
            pCol1[i] = b * c - a * s;
            }
        }
#endif
    }


/*---------------------------------------------------------------------------------**//**
Apply a hyperbolic "row operation", i.e. pre-multiply by a hyperbolic reflection matrix
The matrix is an identity except for the 4 entries
R(i0,i0)=R(i1,i1)=secant
R(i0,i1)=R(i1,i0)=tangent
* @instance pProduct      <= product of givens rotation and input matrix.
@param    secant   => The cosine of reflection.
@param    tangent   => The sine of reflection.
@param    i0  => The index of the first affected row.
@param    i1  => The index of the second affected row.
@param    pMatrix => The input matrix
@group "RotMatrix Rotations"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void        bsiRotMatrix_hyperbolicRowOp


(
RotMatrixP pProduct,
double        secant,
double        tangent,
int           i0,
int           i1,
RotMatrixCP pMatrix
)
    {
    int j;
    double a, b;
    i0 = bsiGeom_cyclic3dAxis (i0);
    i1 = bsiGeom_cyclic3dAxis (i1);

#ifdef __RotMatrix__
    for (j = 0; j < 3; j++)
        {
        a = pProduct->form3d[i0][j];
        b = pProduct->form3d[i1][j];
        pProduct->form3d[i0][j] = a * secant + b * tangent;
        pProduct->form3d[i1][j] = b * secant + a * tangent;
        }
#else
    for (j = 0; j < 3; j++)
        {
        double *pCol = (double *)&pMatrix->column[j];
        a = pCol[i0];
        b = pCol[i1];
        pCol[i0] = a * secant + b * tangent;
        pCol[i1] = b * secant + a * tangent;
        }
#endif
    }


/*---------------------------------------------------------------------------------**//**
Apply a hyperbolic "column operation", i.e. pre-multiply by a hyperbolic reflection matrix
The matrix is an identity except for the 4 entries
R(i0,i0)=R(i1,i1)=secant
R(i0,i1)=R(i1,i0)=tangent
* @instance pProduct      <= product of givens rotation and input matrix.
@param    secant   => The cosine of reflection.
@param    tangent   => The sine of reflection.
@param    i0  => The index of the first affected row.
@param    i1  => The index of the second affected row.
@param    pMatrix => The input matrix
@group "RotMatrix Rotations"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void        bsiRotMatrix_hyperbolicColumnOp


(
RotMatrixP pProduct,
RotMatrixCP pMatrix,
double        secant,
double        tangent,
int           i0,
int           i1
)
    {
    int i;
    double a, b;
    i0 = bsiGeom_cyclic3dAxis (i0);
    i1 = bsiGeom_cyclic3dAxis (i1);

#ifdef __RotMatrix__
    for (i = 0; i < 3; i++)
        {
        a = pProduct->form3d[i][i0];
        b = pProduct->form3d[i][i1];
        pProduct->form3d[i][i0] = a * secant + b * tangent;
        pProduct->form3d[i][i1] = b * secant + a * tangent;
        }
#else
        {
        double *pCol0 = (double *) &pMatrix->column[i0];
        double *pCol1 = (double *) &pMatrix->column[i1];

        for (i = 0; i < 3; i++)
            {
            a = pCol0[i];
            b = pCol1[i];
            pCol0[i] = a * secant + b * tangent;
            pCol1[i] = b * secant + a * tangent;
            }
        }
#endif
    }

/* METHOD(RotMatrix,none,augmentRank)
/*---------------------------------------------------------------------------------**//**
@description Compute a full rank matrix close to the input matrix.  Specifically,
form a singular value decomposition
pMatrix = U * Sigma * VT
For discussion, assume the null singular values are last.
If pMatrix has rank 1, pMatrix = U0 * sigma0 * V0T.   Form 2 vector B1, B2 perpendicular
to U0*sigma0.  Add B1 * V1T + V2 * V2T.
If pMatrix has rank 2, pMatrix = U0 * sigma0 * V0T + U1 * sigma1 * V1T.
Form B2 = (U0 * sigma0) cross (U1 * sigma1).  Add back B2 * V2T.
@instance pFullRankMatrix <= augmented matrix.
@instance pMatrix => original matrix.
@return number of deficient directions added in.
@group "RotMatrix Initialization"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int         bsiRotMatrix_augmentRank


(
RotMatrixP pFullRankMatrix,
RotMatrixCP pMatrix
)
    {
    RotMatrix matrixB, matrixV;
    DVec3d Bcol[3];
    DVec3d Ccol[3];
    DVec3d Vcol[3];
    double sigma[3];
    bool    isZero[3];
    double sum;
    int lastZero = 0;
    int lastNonZero = 0;
    int numZero;
    int i;
    double reltol = bsiTrig_smallAngle ();

    bsiRotMatrix_orthogonalizeColumns (pMatrix, &matrixB, &matrixV);

    bsiRotMatrix_getColumns (&matrixB, &Bcol[0], &Bcol[1], &Bcol[2]);
    bsiRotMatrix_getColumns (&matrixV, &Vcol[0], &Vcol[1], &Vcol[2]);

    for (i = 0, sum = 0.0; i < 3; i++)
        {
        sigma[i] = bsiDVec3d_magnitude (&Bcol[i]);
        sum += sigma[i];
        }

    for (i = numZero = 0; i < 3; i++)
        {
        isZero[i] = sigma[i] <= reltol * sum;
        if (isZero[i])
            {
            lastZero = i;
            numZero++;
            }
        else
            {
            lastNonZero = i;
            }
        }

    switch (numZero)
        {
        case 0:
            *pFullRankMatrix = *pMatrix;
            break;
        case 1:
            {
            DVec3d perpVector;
            int i0, i1;
            /* Only 1 zero column -- use the cross product of the other
                two columns of B to augment rank. */
            i0 = (lastZero + 1) % 3;
            i1 = (i0 + 1) % 3;
            *pFullRankMatrix = *pMatrix;
            bsiDVec3d_geometricMeanCrossProduct (&perpVector, &Bcol[i0], &Bcol[i1]);
            bsiRotMatrix_addScaledOuterProductInPlace (pFullRankMatrix, &perpVector, &Vcol[lastZero], 1.0);
            break;
            }
        case 2:
            {
            int i0, i1;
            /* Only 1 nonzero column */
            bsiDVec3d_getTriad (&Ccol[0], &Ccol[1], &Ccol[2], &Bcol[lastNonZero]);
            i0 = (lastNonZero + 1) % 3;
            i1 = (i0 + 1) % 3;
            *pFullRankMatrix = *pMatrix;
            bsiRotMatrix_addScaledOuterProductInPlace (pFullRankMatrix, &Ccol[0], &Vcol[i0], 1.0);
            bsiRotMatrix_addScaledOuterProductInPlace (pFullRankMatrix, &Ccol[1], &Vcol[i1], 1.0);
            break;
            }

        case 3:
            /* Matrix is full of zeros */
            bsiRotMatrix_initIdentity (pFullRankMatrix);
            break;
        }
    return 3 - numZero;
    }

/*-----------------------------------------------------------------*//**
@description Factor a matrix as a product of a rotation and a skew factor.
    The rotation is defined (via parameters) as a coordiante frame with its xy axes
        aligned with specified columns of the input matrix.
@instance pMatrixA IN matrix to query
@param pRotationMatrix <= the rotation factor.
@param pSkewMatrix <= the skew factor.
@param primaryAxis => this axis direction is preserved exactly.  (usually 0, x axis)
@param secondaryAxis => this axis direction is kept in plane of original primary and secondary
axes.  (usually 1, for  y axis)
@return false if singular matrix.
@group "RotMatrix Queries"
 @bsimethod                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_rotateAndSkewFactors


(
RotMatrixCP pMatrixA,
RotMatrixP pRotationMatrix,
RotMatrixP pSkewMatrix,
int         primaryAxis,
int         secondaryAxis
)
    {
    RotMatrix rotation, skewFactor;
    bool    boolstat = false;

    if (bsiRotMatrix_squareAndNormalizeColumns (&rotation, pMatrixA, primaryAxis, secondaryAxis))
        {
        bsiRotMatrix_multiplyRotMatrixTransposeRotMatrix (&skewFactor, &rotation, pMatrixA);
        boolstat = true;
        }
    else
        {
        bsiRotMatrix_initIdentity (&rotation);
        skewFactor = *pMatrixA;
        boolstat = false;
        }
    if (pRotationMatrix)
        *pRotationMatrix = rotation;
    if (pSkewMatrix)
        *pSkewMatrix = skewFactor;
    return boolstat;
    }

/*-----------------------------------------------------------------*//**
Return a crude 5-case summary of the z-direction components of a matrix.
@param pMatrix => matrix to analyze.
@return
<ul>
<li>  1 if complete effect of z entries is identity.</li>
<li>  -1 if complete effect of z entries is to negate z</li>
<li>  2 if complete effect of z entries is non-unit positive scale on z (zeros off diagonal)</li>
<li>  -2 if complete efect of z entries is non-unit negative scale on z (zeros off diagonal)</li>
<li>  0 if there are any off-diagonal nonzeros, so the matrix has z effects other than simple scaling.</li>
</ul>
@group "RotMatrix Queries"
 @bsimethod                                     EarlinLutz      02/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiRotMatrix_summaryZEffects


(
RotMatrixCP pMatrix
)
    {
    double tol = bsiTrig_smallAngle ();
    double zz = pMatrix->form3d[2][2];
    if (    fabs (pMatrix->form3d[2][0]) > tol
       ||   fabs (pMatrix->form3d[2][1]) > tol
       ||   fabs (pMatrix->form3d[0][2]) > tol
       ||   fabs (pMatrix->form3d[1][2]) > tol
       ||   fabs (zz) < tol
       )
        {
        return 0;
        }
    if (fabs (zz - 1.0) < tol)
        return 1;
    if (fabs (zz + 1.0) < tol)
        return -1;
    if (zz > 0.0)
        return 2;
    if (zz < 0.0)
        return -2;
    return 0;   /* Can't get here. */
    }

/*-----------------------------------------------------------------*//**
Test if a matrix is "just" a rotation around z (i.e. in the xy plane)
@param pMatrix => matrix to analyze.
@param pRadians <= angle in radians.  This angle is the direction of column 0
of the matrix.
@return false if there are any non-rotational effects, or rotation is around any other axis.
@group "RotMatrix Rotations"
 @bsimethod                                     EarlinLutz      02/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isXYRotation


(
RotMatrixCP pMatrix,
double  *pRadians
)
    {
    double radians = 0.0;
    bool    result = false;
    if (   bsiRotMatrix_isOrthogonal (pMatrix)
       &&  1 == bsiRotMatrix_summaryZEffects (pMatrix)
       &&   bsiRotMatrix_determinant (pMatrix) > 0.0
       )
        {
        radians = atan2 (pMatrix->form3d[1][0], pMatrix->form3d[0][0]);
        result = true;
        }
    if (pRadians)
        *pRadians = radians;
    return result;
    }

/*-----------------------------------------------------------------*//**
@Description Constructs a matrix from xy rotation, skew, and scale factors.
In this matrix, the first two columns represent the X and Y axes of a local coordinate
system.  These differ from global X and Y axes in both direction and length.
@instance pMatrix OUT result matrix
@param xAxisAngle  IN Angle (in radians) from global x axis to local x axis.
@param yAxisSkewAngle IN (in radians) from a hypothetical y axis 90 CCW from the local axis
to the actual y axis represented in the matrix.
@param xScale IN (signed) scale factor to apply to x direction
@param yScale IN (signed) scale factor to apply to y direction.
@param zScale IN (signed) scale factor to apply to z direction.
@return Constructed matrix.
@remarks Various reflection (mirroring) effects can be introduced into the coordinate system
via negative xScale, negative yScale, or by ySkewAngle between 90 and 270 degrees


(or, between -90 and -270)., or by negative zScale.
@group "RotMatrix Rotations"
@bsimethod                                      EarlinLutz      02/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiRotMatrix_initFromXYRotationSkewAndScale


(
RotMatrixP pMatrix,
double xAxisAngle,
double yAxisSkewAngle,
double xScale,
double yScale,
double zScale
)
    {
    double ux = cos (xAxisAngle);
    double uy = sin (xAxisAngle);

    double yAxisAngle = xAxisAngle + yAxisSkewAngle;

    double vx = -sin (yAxisAngle);
    double vy =  cos (yAxisAngle);

    bsiRotMatrix_initFromRowValues
                (
                pMatrix,
                ux * xScale, vx * yScale, 0.0,
                uy * xScale, vy * yScale, 0.0,
                0.0,         0.0,         zScale
                );
    }

/*------------------------------------------------------------------------------------------*//**
@Description Reinterprets the matrix as a combination of XY rotation, skew and scaling
    and Z scaling.
@param pMatrix IN matrix being analyzed.
@param pXAxisAngle OUT The angle, in radians counterclockwise, from the global x axis to the x axis (first column)
    of this matrix.
@param pYAxisSkewAngle OUT The angle, in radians counterclockwise, by which the Y axis of the matrix is skewed away from
    the usual position of 90 degrees counterclockwise from X.
@param pXScale OUT The length of the x axis vector of this matrix.  This scale factor is always positive.
@param pYScale OUT The length of the y axis vector of this matrix.  This scale factor may be positive or negative.
    If negative, the matrix operation includes mirroring (reflection).
@param pZScale OUT The (signed) scale factor which the matrix applies to z data.
    This is typically 1 or -1.
@return true if the matrix can be decomposed in this manner.  false if there are out-of-plane effects
    indicated by values in the z parts of the matrix.
@remarks If the matrix is "just" rotation around z, the two rotation angles will be identical and both scale
    factors are 1.0.
@remarks If the matrix is "just" rotation around z and uniform scaling, the two rotation angles will be identical
    and the scale factors will be identical to each other but different from 1.
@remarks In the non-mirrored case, the x direction is given by the first column of the matrix; the nominal
    y direction is 90 degrees counter clockwise, and the returned skew angle is the angle from that
    nominal y axis direction to the second column of the matrix.  In the mirrored case, the mirroring
    is indicated by a negated y scale factor, and the skew angle is measured from the same nominal
    y axis (90 degrees counterclockwise from first column) to the NEGATED second column.  That is,
    the returned numbers always represent a skewed-but-righthanded system, with reflection rolled into
    the y scale factor.
@group "RotMatrix Queries"
@bsimethod                                      EarlinLutz      02/02
------------------------------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isXYRotationSkewAndScale


(
RotMatrixP pMatrix,
double          *pXAxisAngle,
double          *pYAxisSkewAngle,
double          *pXScale,
double          *pYScale,
double          *pZScale
)
    {
    double ux, uy, vx, vy, vx0, vy0;
    double ztol = bsiTrig_smallAngle ();
    double ax, ay, dot;
    bool    bHasZEffects;

    /* Column 0 is u axis */
    ux = pMatrix->form3d[0][0];
    uy = pMatrix->form3d[1][0];

    /* 90 degree rotation is nominal y axis. */
    vx0 = -uy;
    vy0 = ux;

    /* Column 1 is real axis (possibly negated) */
    vx = pMatrix->form3d[0][1];
    vy = pMatrix->form3d[1][1];
    ax = sqrt (ux * ux + uy * uy);
    ay = sqrt (vx * vx + vy * vy);
    dot = vx0 * vx + vy0 * vy;

    if (dot < 0.0)
        {
        vx = - vx;
        vy = - vy;
        ay = -ay;
        }

    *pXAxisAngle = atan2 (uy, ux);
    *pYAxisSkewAngle = atan2 (vy, vx) - atan2 (vy0, vx0);
    *pXScale = ax;
    *pYScale = ay;
    *pZScale = pMatrix->form3d[2][2];

    bHasZEffects =
           fabs (pMatrix->form3d[2][0]) > ztol
        || fabs (pMatrix->form3d[2][1]) > ztol
        || fabs (pMatrix->form3d[0][2]) > ztol
        || fabs (pMatrix->form3d[1][2]) > ztol;

    return !bHasZEffects;
    }
/**
@description Compute angles such that a matrix is decomposed into a product
    of simple X, Y, and Z rotations and a uniform scale, i.e.
        <p>
    Rotate(X,thetaX) * Rotate(Y,thetaY) * Rotate(Z, thetaZ) * uniformScale
        </p>
        where Rotate(axis,angle) is rotation matrix for specified axis and angle.
        This decompositon is sometimes called "Euler angles".  The user should be aware that
        Euler angles are of very limited value for computation.
@param pMatrix IN matrix to analyze.
@param pThetaX OUT rotation around X.
@param pThetaY OUT rotation around Y.
@param pThetaZ OUT rotation around Z.
@param pScale OUT scale factor.
@group "RotMatrix Queries"
@return true if the matrix is constructed of rotation and uniform scale.
    false if non-uniform scale.
*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isXRotationYRotationZRotationScale


(
RotMatrixCP pMatrix,
double *pThetaX,
double *pThetaY,
double *pThetaZ,
double *pScale
)
    {
    double s_tol = bsiTrig_smallAngle ();

    double thetaX = 0.0, thetaY = 0.0, thetaZ = 0.0;
    double scale = 1.0;
    double rescale;

    double cz, sz;
    double cx, sx;
    double cy, sy;

    double cosSqY, tolTest;
    bool    boolstat = false;
    RotMatrix R, R1;
    double det;
    double maxDiff;
    det = bsiRotMatrix_determinant (pMatrix);

    if (fabs (det) > 0.0)
        {
        scale = pow (fabs (det), 1.0 / 3.0);
        if (det < 0.0)
            scale = -scale;
        rescale = 1.0 / scale;

        /* Descale the matrix.
        The descaled matrix has positive determinant. */
        bsiRotMatrix_scaleColumns (&R, pMatrix, rescale, rescale, rescale);

        cosSqY = sqrt (R.form3d[0][0] * R.form3d[0][0] + R.form3d[0][1] * R.form3d[0][1]);
        tolTest = 1.0 - cosSqY;

        if (tolTest < 1.0)  // is cosine large enough to pull us away form +- Y axis?
            {
            RotMatrix RotZInv, RotXRotY;
            thetaZ = atan2 (-R.form3d[0][1],R.form3d[0][0]);
            cz = cos (thetaZ);
            sz = sin (thetaZ);
            RotZInv.form3d[0][0] = RotZInv.form3d[1][1] = cz;
            RotZInv.form3d[2][2] = 1.0;
            RotZInv.form3d[0][1] = sz;
            RotZInv.form3d[1][0] = -sz;
            RotZInv.form3d[0][2] = RotZInv.form3d[1][2] =0.0;
            RotZInv.form3d[2][0] = RotZInv.form3d[2][1] =0.0;
            bsiRotMatrix_multiplyRotMatrixRotMatrix (&RotXRotY, &R, &RotZInv);
            thetaY = atan2 (RotXRotY.form3d[0][2], RotXRotY.form3d[0][0]);
            thetaX = atan2 (RotXRotY.form3d[2][1], RotXRotY.form3d[1][1]);
            }
        else
            {
            thetaZ = 0.0;
            cz = 1.0;
            sz = 0.0;
            thetaX = atan2 (R.form3d[1][0], R.form3d[1][1]);
            thetaY = 2.0 * atan (1.0);
            if (R.form3d[0][2] < 0.0)
                {
                thetaX = -thetaX;
                thetaY = -thetaY;
                }
            }

            /* Reassemble the matrix from composite rotation formula.
               cx, sx were computed along the way.
               This confirms that the input was a true rotation.
            */

        cy = cos (thetaY);
        sy = sin (thetaY);
        cx = cos (thetaX);
        sx = sin (thetaX);

        bsiRotMatrix_initFromRowValues (&R1,
                cy * cz,                - cy * sz,                  sy,
                sx * sy * cz +  cx * sz, -sx * sy * sz + cx * cz,  -sx * cy,
                -cx * sy * cz + sx * sz,  cx * sy * sz + sx * cz,   cx * cy
                );
        maxDiff = bsiRotMatrix_maxDiff (&R, &R1);
        if (maxDiff <= s_tol)
            boolstat = true;
        }

    if (pThetaX)
        *pThetaX = thetaX;
    if (pThetaY)
        *pThetaY = thetaY;
    if (pThetaZ)
        *pThetaZ = thetaZ;
    if (pScale)
        *pScale = scale;

    return boolstat;
    }

END_BENTLEY_NAMESPACE
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
* @param pEigenvectors <= matrix of eigenvectors.
* @param pEigenvalues  => eigenvalues corresponding to columns of the eigenvector matrix.
* @param pInstance      => matrix whose eigenvectors and eigenvalues are computed.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_symmetricEigensystem
(
RotMatrixP pEigenvectors,
DPoint3dP pEigenvalues,
RotMatrixCP pInstance
)
    {
    RotMatrix workMatrix = *pInstance;
    int numIteration;
    bsiGeom_jacobi3X3 (
              (double*)pEigenvalues,
              (double (*)[3])pEigenvectors,
              &numIteration,
              (double (*)[3])&workMatrix
              );
        }
#ifdef CompileAll
/*-----------------------------------------------------------------*//**
* @description Factor matrix A as a product A = B * VT
*   where VT is orthogonal and B has orthonormal columns.
*   If A is singular, B will have a zero column.  If all columns
*   of B are nonzero, the inverse of A can be constructed accurately.
* @param pMatrixA => the matrix to factor
* @param pB <= the matrix with perpendicular but not normalized columns.
* @param pV <= the orthogonal matrix.
*
* @return true factorization completed.  Numerical authorities claim
*       the iteration will always succeed.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static bool    bsiRotMatrix_orthogonalizeColumnsSymmetric

(
RotMatrixCP pMatrixA,
RotMatrixP pB,
RotMatrixP pV
)
    {
    RotMatrix matrixB, matrixBTB, matrixQ, matrixV;
    bool    boolstat = false;
    DPoint3d lambda;
    double s0, s1;
    int i;
    static int maxIteration = 10;
    static double s_squareTol = 1.0e-30;

    matrixB = *pMatrixA;
    bsiRotMatrix_initIdentity (&matrixV);
    /* Basic step: Form eigenvectors of BT*B.
       Replace (B,V) by (B*Q, V*Q).
    */
    for (i = 0; i < maxIteration; i++)
        {
        bsiRotMatrix_multiplyRotMatrixTransposeRotMatrix
                        (&matrixBTB, &matrixB, &matrixB);
        s0 = bsiRotMatrix_sumOffDiagonalSquares (&matrixBTB);
        s1 = bsiRotMatrix_sumDiagonalSquares (&matrixBTB);

        if (s0 <= s_squareTol * s1)
            {
            boolstat = true;
            break;
            }
        bsiRotMatrix_symmetricEigensystem
                        (&matrixQ, &lambda, &matrixBTB);
        bsiRotMatrix_multiplyRotMatrixRotMatrix
                        (&matrixB, &matrixB, &matrixQ);
        bsiRotMatrix_multiplyRotMatrixRotMatrix
                        (&matrixV, &matrixV, &matrixQ);
        }

    if (pB)
        *pB = matrixB;
    if (pV)
        *pV = matrixV;
    return boolstat;
    }
#endif

#ifdef CheckOrthogonalization
static void CheckBV
(
DVec3dCR U,
DVec3dCR V,
DVec3dCR W,
DVec3dCR QU,
DVec3dCR QV,
DVec3dCR QW,
RotMatrixCP pMatrixA
)
    {
    RotMatrix  B, C;
    B.InitFromColumnVectors (U, V, W);
    C.InitFromColumnVectors (QU, QV, QW);
    RotMatrix BCT, BTB, CTC;
    bsiRotMatrix_multiplyRotMatrixRotMatrixTranspose (&BCT, &B, &C);
    bsiRotMatrix_multiplyRotMatrixTransposeRotMatrix (&BTB, &B, &B);
    bsiRotMatrix_multiplyRotMatrixTransposeRotMatrix (&CTC, &C, &C);
    bool diagonal = BTB.IsDiagonal ();
    bool identity = CTC.IsIdentity ();
    double delta = BCT.MaxDiff (*pMatrixA);
    }
#endif

#ifdef CheckOrthogonalization
Public GEOMDLLIMPEXP bool    bsiRotMatrix_orthogonalizeColumnsDual
(
RotMatrixCP pMatrixA,
RotMatrixP pB,
RotMatrixP pV
)
    {
    RotMatrix B, V;
    bool    stat;
    stat = bsiRotMatrix_orthogonalizeColumnsSymmetric (pMatrixA, &B, &V);
    stat = bsiRotMatrix_orthogonalizeColumnsDirect (pMatrixA, pB, pV);
    double dB = B.MaxDiff (*pB);
    double dV = V.MaxDiff (*pV);
    return stat;
    }
#endif




static void doubleIndexBubbleStep

(
int             *pOrder,
double const    *pValues,
int             i0,
int             i1
)
    {
    if (pValues[pOrder[i0]] > pValues[pOrder[i1]])
        {
        int temp = pOrder[i0];
        pOrder[i0] = pOrder[i1];
        pOrder[i1] = temp;
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE

BEGIN_BENTLEY_NAMESPACE
/*-----------------------------------------------------------------*//**
* @param pEigenvectors <=> Eigenvectors of matrix
* @param pEigenvalues <=> corresponding Eigenvalues
* @param pOrder => pOrder[i] = original position of final i'th eigenvalue
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_sortEigensystem
(
RotMatrixP pEigenvectors,
DPoint3dP pEigenvalues,
int       *pOrder
)
    {
    int i;
    double vector[3];
    DVec3d uu[3];
    uu[0] = DVec3d::FromColumn (*pEigenvectors, 0);
    uu[1] = DVec3d::FromColumn (*pEigenvectors, 1);
    uu[2] = DVec3d::FromColumn (*pEigenvectors, 2);

    vector[0] = pEigenvalues->x;
    vector[1] = pEigenvalues->y;
    vector[2] = pEigenvalues->z;

    for (i = 0; i < 3; i++)
        pOrder[i] = i;

    doubleIndexBubbleStep (pOrder, vector, 0, 1);
    doubleIndexBubbleStep (pOrder, vector, 0, 2);
    doubleIndexBubbleStep (pOrder, vector, 1, 2);

    *pEigenvectors = RotMatrix::FromColumnVectors (uu[pOrder[0]], uu[pOrder[1]], uu[pOrder[2]]);

    pEigenvalues->x = vector[pOrder[0]];
    pEigenvalues->y = vector[pOrder[1]];
    pEigenvalues->z = vector[pOrder[2]];
    }



END_BENTLEY_NAMESPACE
