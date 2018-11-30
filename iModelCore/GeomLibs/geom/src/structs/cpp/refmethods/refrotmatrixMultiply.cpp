/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/cpp/refmethods/refrotmatrixMultiply.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE


/*-----------------------------------------------------------------*//**
* @description Add a matrix (componentwise, in place).
*
* @param [in] delta The matrix to add
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::Add (RotMatrixCR delta)
    {
    int i;
    double *pA = (double *)this;
    double *pB = (double *)&delta;
    for (i = 0; i < 9; i++, pA++, pB++)
        {
        *pA += *pB;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Applies scale factors to corresponding rows of the input matrix, and places the result
* in this instance matrix.
* @param [out] pScaledMatrix scaled matrix
* @param [in] inMatrix The initial matrix
* @param [in] xScale The scale factor for row 0
* @param [in] yScale The scale factor for row 1
* @param [in] zScale The scale factor for row 2
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::ScaleRows
(

RotMatrixCR inMatrix,
double      xScale,
double      yScale,
double      zScale

)
    {
    int i;

    *this = inMatrix;
    for (i=0; i<3; i++)
        {
        form3d[0][i] *= xScale;
        form3d[1][i] *= yScale;
        form3d[2][i] *= zScale;
        }
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RotMatrix::ScaleColumns
(
RotMatrixCR in,
double      xs,
double      ys,
double      zs
)
    {
    form3d[0][0] = in.form3d[0][0] * xs;
    form3d[1][0] = in.form3d[1][0] * xs;
    form3d[2][0] = in.form3d[2][0] * xs;

    form3d[0][1] = in.form3d[0][1] * ys;
    form3d[1][1] = in.form3d[1][1] * ys;
    form3d[2][1] = in.form3d[2][1] * ys;

    form3d[0][2] = in.form3d[0][2] * zs;
    form3d[1][2] = in.form3d[1][2] * zs;
    form3d[2][2] = in.form3d[2][2] * zs;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RotMatrix::ScaleColumns
(
double      xs,
double      ys,
double      zs
)
    {
    form3d[0][0] *= xs;
    form3d[1][0] *= xs;
    form3d[2][0] *= xs;

    form3d[0][1] *= ys;
    form3d[1][1] *= ys;
    form3d[2][1] *= ys;

    form3d[0][2] *= zs;
    form3d[1][2] *= zs;
    form3d[2][2] *= zs;
    }






/*-----------------------------------------------------------------*//**
* @description Subtract a matrix (componentwise, in place).
*
* @param [in] delta The matrix to subtract
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::Subtract
(

RotMatrixCR delta

)
    {
    int i;
    double *pA = (double *)this;
    double *pB = (double *)&delta;
    for (i = 0; i < 9; i++, pA++, pB++)
        {
        *pA -= *pB;
        }
    }





/*-----------------------------------------------------------------*//**
* Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array point.
* Each result is placed in the corresponding entry in the output array result.   The same array
* may be named for the input and output arrays.
* @param [in] pMatrix The matrix to apply
* @param [out] result output points
* @param [in] point The input points
* @param [in] numPoint The number of points
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::Multiply
(

DPoint4dP  result,
DPoint4dCP point,
int        numPoint

) const
    {
    int     i;
    double x,y,z;
    DPoint4d *resultPoint;

    for (i = 0, resultPoint = result;
         i < numPoint;
         i++, resultPoint++
        )
        {
        x = point[i].x;
        y = point[i].y;
        z = point[i].z;
        resultPoint->w = point[i].w;

        resultPoint->x = form3d[0][0] * x
                       + form3d[0][1] * y
                       + form3d[0][2] * z;

        resultPoint->y = form3d[1][0] * x
                       + form3d[1][1] * y
                       + form3d[1][2] * z;

        resultPoint->z = form3d[2][0] * x
                       + form3d[2][1] * y
                       + form3d[2][2] * z;

        }
    }


/*-----------------------------------------------------------------*//**
* Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array point.
* Each result is placed in the corresponding entry in the output array result.   The same array
* may be named for the input and output arrays.
* @param [in] pMatrix The matrix to apply
* @param [out] result output points
* @param [in] point The input points
* @param [in] numPoint The number of points
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::Multiply
(

DPoint2dP result,
DPoint2dCP point,
      int            numPoint

) const
    {
    int     i;
    double x,y;
    DPoint2d *resultPoint;

    for (i = 0, resultPoint = result;
         i < numPoint;
         i++, resultPoint++
        )
        {
        x = point[i].x;
        y = point[i].y;

        resultPoint->x = form3d[0][0] * x
                       + form3d[0][1] * y;

        resultPoint->y = form3d[1][0] * x
                       + form3d[1][1] * y;
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RotMatrix::InitProductRotMatrixTransposeRotMatrix
(
RotMatrixCR rotMatrixA,
RotMatrixCR rotMatrixB
)
    {
    int       j;
    RotMatrix ATB;
    ATB.Zero ();
    for (j = 0; j < 3; j++)
        {
        ATB.form3d[0][j]
                       = rotMatrixA.form3d[0][0] * rotMatrixB.form3d[0][j]
                       + rotMatrixA.form3d[1][0] * rotMatrixB.form3d[1][j]
                       + rotMatrixA.form3d[2][0] * rotMatrixB.form3d[2][j];

        ATB.form3d[1][j]
                       = rotMatrixA.form3d[0][1] * rotMatrixB.form3d[0][j]
                       + rotMatrixA.form3d[1][1] * rotMatrixB.form3d[1][j]
                       + rotMatrixA.form3d[2][1] * rotMatrixB.form3d[2][j];

        ATB.form3d[2][j]
                       = rotMatrixA.form3d[0][2] * rotMatrixB.form3d[0][j]
                       + rotMatrixA.form3d[1][2] * rotMatrixB.form3d[1][j]
                       + rotMatrixA.form3d[2][2] * rotMatrixB.form3d[2][j];
        }
    *this = ATB;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RotMatrix::InitProductRotMatrixRotMatrixTranspose
(
RotMatrixCR rotMatrixA,
RotMatrixCR rotMatrixB
)
    {
    RotMatrix BT;
    BT.TransposeOf (rotMatrixB);
    InitProduct (rotMatrixA, BT);
    }



/*-----------------------------------------------------------------*//**
* @description Returns the product {A*B} of two matrices.
* @param [in] rotMatrixA The first factor
* @param [in] rotMatrixB The second factor
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitProduct
(

RotMatrixCR rotMatrixA,
RotMatrixCR rotMatrixB

)
    {
    int       j;
    RotMatrix AB;
    AB.InitIdentity ();
    for (j = 0; j < 3; j++)
        {
        AB.form3d[0][j]
                       = rotMatrixA.form3d[0][0] * rotMatrixB.form3d[0][j]
                       + rotMatrixA.form3d[0][1] * rotMatrixB.form3d[1][j]
                       + rotMatrixA.form3d[0][2] * rotMatrixB.form3d[2][j];

        AB.form3d[1][j]
                       = rotMatrixA.form3d[1][0] * rotMatrixB.form3d[0][j]
                       + rotMatrixA.form3d[1][1] * rotMatrixB.form3d[1][j]
                       + rotMatrixA.form3d[1][2] * rotMatrixB.form3d[2][j];

        AB.form3d[2][j]
                       = rotMatrixA.form3d[2][0] * rotMatrixB.form3d[0][j]
                       + rotMatrixA.form3d[2][1] * rotMatrixB.form3d[1][j]
                       + rotMatrixA.form3d[2][2] * rotMatrixB.form3d[2][j];
        }
    *this = AB;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of three matrices.
* @param [in] rotMatrixA The first factor
* @param [in] rotMatrixB The second factor
* @param [in] rotMatrixC The third factor
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitProduct
(

RotMatrixCR rotMatrixA,
RotMatrixCR rotMatrixB,
RotMatrixCR rotMatrixC

)
    {
    this->InitProduct (rotMatrixA, rotMatrixB);
    this->InitProduct (*this, rotMatrixC);
    }


/*-----------------------------------------------------------------*//**
* Sets this instance to the product of matrix rotMatrixA times the matrix part
* of transform transformB.
* In this 'post multiplication' the matrix part of transformB multiplies rotMatrixA ON THE RIGHT
* and the translation part of transformB is ignored.
*
* @param [in] rotMatrixA The first factor (matrix)
* @param [in] transformB The second factor (transform)
* @indexVerb multiply
* @bsimethod                                                    DavidAssaf      10/98
+----------------------------------------------------------------------*/
void RotMatrix::InitProduct
(

RotMatrixCR rotMatrixA,
TransformCR transformB

)
    {
    RotMatrix   AB;
    AB.InitIdentity ();
    int j;
    for (j = 0; j < 3; j++)
        {
        AB.form3d[0][j]
                       = rotMatrixA.form3d[0][0] * transformB.form3d[0][j]
                       + rotMatrixA.form3d[0][1] * transformB.form3d[1][j]
                       + rotMatrixA.form3d[0][2] * transformB.form3d[2][j];

        AB.form3d[1][j]
                       = rotMatrixA.form3d[1][0] * transformB.form3d[0][j]
                       + rotMatrixA.form3d[1][1] * transformB.form3d[1][j]
                       + rotMatrixA.form3d[1][2] * transformB.form3d[2][j];

        AB.form3d[2][j]
                       = rotMatrixA.form3d[2][0] * transformB.form3d[0][j]
                       + rotMatrixA.form3d[2][1] * transformB.form3d[1][j]
                       + rotMatrixA.form3d[2][2] * transformB.form3d[2][j];
        }
    *this = AB;
    }


/*-----------------------------------------------------------------*//**
* @description Returns to the product of the matrix part of transform transformA
* times matrix rotMatrixB.
* In this 'pre multiplication' the matrix part of transformA multiplies rotMatrixB ON THE LEFT
* and the translation part of transformA is ignored.
*
* @param [in] transformA The first factor (transform)
* @param [in] rotMatrixB The second factor (matrix)
* @indexVerb multiply
* @bsimethod                                                    DavidAssaf      10/98
+----------------------------------------------------------------------*/
void RotMatrix::InitProduct
(

TransformCR transformA,
RotMatrixCR rotMatrixB

)
    {
    RotMatrix   AB;
    AB.InitIdentity ();
    int j;
    for (j = 0; j < 3; j++)
        {
        AB.form3d[0][j]
                        = transformA.form3d[0][0] * rotMatrixB.form3d[0][j]
                        + transformA.form3d[0][1] * rotMatrixB.form3d[1][j]
                        + transformA.form3d[0][2] * rotMatrixB.form3d[2][j];

        AB.form3d[1][j]
                        = transformA.form3d[1][0] * rotMatrixB.form3d[0][j]
                        + transformA.form3d[1][1] * rotMatrixB.form3d[1][j]
                        + transformA.form3d[1][2] * rotMatrixB.form3d[2][j];

        AB.form3d[2][j]
                        = transformA.form3d[2][0] * rotMatrixB.form3d[0][j]
                        + transformA.form3d[2][1] * rotMatrixB.form3d[1][j]
                        + transformA.form3d[2][2] * rotMatrixB.form3d[2][j];
        }
    *this = AB;
    }


/*-----------------------------------------------------------------*//**
* computes the 8 corner points of the range cube defined by the two points
* of pRange, multiplies matrix times each point, and computes the
* twopoint range limits (min max) of the resulting rotated box.
* Note that the volume of the rotated box is generally larger than
* that of the input because the postrotation minmax operation expands
* the box.
* Ranges that are null as defined by inRange->isNull are left
* unchanged.
* @param [out] outRange range of transformed cube
* @param [in] inRange The any two corners of original cube
* @indexVerb multiply
* @indexVerb range
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::Multiply
(

DRange3dR outRange,
DRange3dCR inRange

) const
    {
    DPoint3d    corner[8];

    if (inRange.IsNull ())
        {
        outRange = inRange;
        }
    else
        {
        inRange.Get8Corners (corner);
        this->Multiply (corner, corner, 8);
        outRange.InitFrom (corner, 8);
        }
    }


/*-----------------------------------------------------------------*//**
* Computes the product {M*P} where M is this instance matrix and P the input point.
* The result overwrites the previous coordinates in point.
*
* @param [in,out] point The point to be updated
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::Multiply
(

DPoint3dR point

) const
    {
    DPoint3d  inPoint;

    inPoint = point;

    point.x =   form3d[0][0] * inPoint.x
              + form3d[0][1] * inPoint.y
              + form3d[0][2] * inPoint.z;

    point.y =   form3d[1][0] * inPoint.x
              + form3d[1][1] * inPoint.y
              + form3d[1][2] * inPoint.z;

    point.z =   form3d[2][0] * inPoint.x
              + form3d[2][1] * inPoint.y
              + form3d[2][2] * inPoint.z;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix times a point.
* @param [out] result result of the multiplication.
* @param [in] point The known point.
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::Multiply
(

DPoint3dR result,
DPoint3dCR point

) const
    {
    DPoint3d  inPoint;

    inPoint = point;

    result.x =  form3d[0][0] * inPoint.x
              + form3d[0][1] * inPoint.y
              + form3d[0][2] * inPoint.z;

    result.y =  form3d[1][0] * inPoint.x
              + form3d[1][1] * inPoint.y
              + form3d[1][2] * inPoint.z;

    result.z =  form3d[2][0] * inPoint.x
              + form3d[2][1] * inPoint.y
              + form3d[2][2] * inPoint.z;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix transpose times a point.
* @param [out] result result of the multiplication.
* @param [in] point The known point.
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::MultiplyTranspose
(

DPoint3dR result,
DPoint3dCR point

) const
    {
    DPoint3d  inPoint;

    inPoint = point;

    result.x =  form3d[0][0] * inPoint.x
              + form3d[1][0] * inPoint.y
              + form3d[2][0] * inPoint.z;

    result.y =  form3d[0][1] * inPoint.x
              + form3d[1][1] * inPoint.y
              + form3d[2][1] * inPoint.z;

    result.z =  form3d[0][2] * inPoint.x
              + form3d[1][2] * inPoint.y
              + form3d[2][2] * inPoint.z;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix times a point,
*           with the point given as separate components.
*
* @param [out] result result of multiplication
* @param [in] x The x component of input point
* @param [in] y The y component of input point
* @param [in] z The z component of input point
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::MultiplyComponents
(

DPoint3dR result,
double         x,
double         y,
double         z

) const
    {
    result.x =  form3d[0][0] * x
              + form3d[0][1] * y
              + form3d[0][2] * z;

    result.y =  form3d[1][0] * x
              + form3d[1][1] * y
              + form3d[1][2] * z;

    result.z =  form3d[2][0] * x
              + form3d[2][1] * y
              + form3d[2][2] * z;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the product {P*M} where M is this instance matrix and P is the input point
* The result overwrites the previous coordinates in the point.
* @param [in,out] point The point to be multiplied
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::MultiplyTranspose
(

DPoint3dR point

) const
    {
    DPoint3d  inPoint;

    inPoint = point;

    point.x =   form3d[0][0]*inPoint.x
              + form3d[1][0]*inPoint.y
              + form3d[2][0]*inPoint.z;

    point.y =   form3d[0][1]*inPoint.x
              + form3d[1][1]*inPoint.y
              + form3d[2][1]*inPoint.z;

    point.z =   form3d[0][2]*inPoint.x
              + form3d[1][2]*inPoint.y
              + form3d[2][2]*inPoint.z;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product P = [x,y,z]*M where M is the input matrix and P is
*           the product point.
* @param [out] pPoint product point
* @param [in] x The x component
* @param [in] y The y component
* @param [in] z The z component
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::MultiplyTransposeComponents
(

DPoint3dR result,
double        x,
double        y,
double        z

) const
    {
    result.x =  form3d[0][0] * x
              + form3d[1][0] * y
              + form3d[2][0] * z;

    result.y =  form3d[0][1] * x
              + form3d[1][1] * y
              + form3d[2][1] * z;

    result.z =  form3d[0][2] * x
              + form3d[1][2] * y
              + form3d[2][2] * z;
    }


/*-----------------------------------------------------------------*//**
* Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array point.
* Each result is placed in the corresponding entry in the output array result.   The same array
* may be named for the input and output arrays.
* @param [out] result output points
* @param [in] point The input points
* @param [in] numPoint The number of points
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::Multiply
(

DPoint3dP result,
DPoint3dCP point,
      int            numPoint

) const
    {
    int     i;
    double x,y,z;
    DPoint3d *resultPoint;

    for (i = 0, resultPoint = result;
         i < numPoint;
         i++, resultPoint++
        )
        {
        x = point[i].x;
        y = point[i].y;
        z = point[i].z;

        resultPoint->x = form3d[0][0] * x
                       + form3d[0][1] * y
                       + form3d[0][2] * z;

        resultPoint->y = form3d[1][0] * x
                       + form3d[1][1] * y
                       + form3d[1][2] * z;

        resultPoint->z = form3d[2][0] * x
                       + form3d[2][1] * y
                       + form3d[2][2] * z;

        }
    }


/*-----------------------------------------------------------------*//**
* Computes {P[i]*M} where M is this instance matrix and each P[i] is a point
*   in the input array point, transposed into row form.  (Equivalently, multiply M^*p[i], where M^ indicates transpose.)
* Each result is placed in the corresponding entry in the output array result.   The same array
* may be named for the input and output arrays.
* @param [in] pMatrix The matrix to apply
* @param [out] result output points
* @param [in] point The input points
* @param [in] numPoint The number of points
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::MultiplyTranspose
(

DPoint3dP  result,
DPoint3dCP point,
int        numPoint

) const
    {
    int     i;
    double x,y,z;
    DPoint3d *resultPoint;

    for (i = 0, resultPoint = result;
         i < numPoint;
         i++, resultPoint++
        )
        {
        x = point[i].x;
        y = point[i].y;
        z = point[i].z;

        resultPoint->x = form3d[0][0] * x
                       + form3d[1][0] * y
                       + form3d[2][0] * z;

        resultPoint->y = form3d[0][1] * x
                       + form3d[1][1] * y
                       + form3d[2][1] * z;

        resultPoint->z = form3d[0][2] * x
                       + form3d[1][2] * y
                       + form3d[2][2] * z;

        }
    }


END_BENTLEY_NAMESPACE
