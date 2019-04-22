/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE
static const RotMatrix s_identityMatrix =
        {{
            {1.0, 0.0, 0.0},
            {0.0, 1.0, 0.0},
            {0.0, 0.0, 1.0}
        }};

static const RotMatrix s_zeroMatrix = {{{0,0,0},{0,0,0},{0,0,0}}};
static const double s_mediumRelTol = 1.0e-12;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
RotMatrix RotMatrix::FromIdentity
(
)
    {
    return s_identityMatrix;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RotMatrix::InitIdentity ()   {*this = s_identityMatrix;}



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RotMatrix::Zero () {*this = s_zeroMatrix;}


/*-----------------------------------------------------------------*//**
* @description Returns a scaling matrix with respective x, y, and z
*   scaling factors.
* @param 'xscale => The x direction scale factor
* @param [in] yscale The y direction scale factor
* @param [in] zscale The z direction scale factor
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitFromScaleFactors
(

double xscale,
double yscale,
double zscale

)
    {
    this->Zero ();
    form3d[0][0] = xscale;
    form3d[1][1] = yscale;
    form3d[2][2] = zscale;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a uniform scaling matrix.
* @param [in] scale The scale factor.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitFromScale
(

double scale

)
    {
    this->InitFromScaleFactors (scale, scale, scale);
    }


/*-----------------------------------------------------------------*//**
*
* @description Returns a 'rank one' matrix defined as a scale factor times
* the 'vector outer product' of two vectors, i.e. as the matrix {s*U*V^T}
*
* @param [out] pMatrix constructed matrix
* @param [in] vector1 The column vector U
* @param [in] vector2 The row vector V
* @param [in] scale The scale factor
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitFromScaledOuterProduct
(

DVec3dCR vector1,
DVec3dCR vector2,
double   scale

)
    {
    form3d[0][0] = scale * vector1.x * vector2.x;
    form3d[1][0] = scale * vector1.y * vector2.x;
    form3d[2][0] = scale * vector1.z * vector2.x;

    form3d[0][1] = scale * vector1.x * vector2.y;
    form3d[1][1] = scale * vector1.y * vector2.y;
    form3d[2][1] = scale * vector1.z * vector2.y;

    form3d[0][2] = scale * vector1.x * vector2.z;
    form3d[1][2] = scale * vector1.y * vector2.z;
    form3d[2][2] = scale * vector1.z * vector2.z;
    }


/*-----------------------------------------------------------------*//**
*
* @description Returns a 'rank one' matrix defined as a scale factor times
* the 'vector outer product' of two vectors, i.e. as the matrix {s*U*V^T}
*
* @param [out] pMatrix constructed matrix
* @param [in] vector1 The column vector U
* @param [in] vector2 The row vector V
* @param [in] scale The scale factor
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::AddScaledOuterProductInPlace
(

DVec3dCR vector1,
DVec3dCR vector2,
      double         scale

)
    {
    RotMatrix matrixA;
    matrixA.InitFromScaledOuterProduct (vector1, vector2, scale);
    this->Add (matrixA);
    }


/*-----------------------------------------------------------------*//**
* @description Returns a matrix with the 9 specified coefficients given
*   in "row major" order.
* @param [in] x00 The 00 entry
* @param [in] x01 The 01 entry
* @param [in] x02 The 02 entry
* @param [in] x10 The 10 entry
* @param [in] x11 The 11 entry
* @param [in] x12 The 12 entry
* @param [in] x20 The 20 entry
* @param [in] x21 The 21 entry
* @param [in] x22 The 22 entry
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitFromRowValues
(

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
    form3d[0][0] = x00;
    form3d[1][0] = x10;
    form3d[2][0] = x20;

    form3d[0][1] = x01;
    form3d[1][1] = x11;
    form3d[2][1] = x21;

    form3d[0][2] = x02;
    form3d[1][2] = x12;
    form3d[2][2] = x22;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                EarlinLutz      09/12
+----------------------------------------------------------------------*/
void RotMatrix::InitFromRowValuesXY
(
double        x00,
double        x01,
double        x10,
double        x11
)
    {
    form3d[0][0] = x00;
    form3d[1][0] = x10;
    form3d[2][0] = 0.0;

    form3d[0][1] = x01;
    form3d[1][1] = x11;
    form3d[2][1] = 0.0;

    form3d[0][2] = 0.0;
    form3d[1][2] = 0.0;
    form3d[2][2] = 1.0;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                EarlinLutz      09/12
+----------------------------------------------------------------------*/
void RotMatrix::InitFromRowValuesXY
(
double const *data
)
    {
    form3d[0][0] = data[0];
    form3d[1][0] = data[2];
    form3d[2][0] = 0.0;

    form3d[0][1] = data[1];
    form3d[1][1] = data[3];
    form3d[2][1] = 0.0;

    form3d[0][2] = 0.0;
    form3d[1][2] = 0.0;
    form3d[2][2] = 1.0;
    }



/*-----------------------------------------------------------------*//**
* @description Returns a matrix with 3 points copied to respective columns.
* @param [in] xVector The vector to insert in column 0
* @param [in] yVector The vector to insert in column 1
* @param [in] zVector The vector to insert in column 2
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitFromColumnVectors
(

DVec3dCR xVector,
DVec3dCR yVector,
DVec3dCR zVector

)
    {
    form3d[0][0] = xVector.x;
    form3d[1][0] = xVector.y;
    form3d[2][0] = xVector.z;

    form3d[0][1] = yVector.x;
    form3d[1][1] = yVector.y;
    form3d[2][1] = yVector.z;

    form3d[0][2] = zVector.x;
    form3d[1][2] = zVector.y;
    form3d[2][2] = zVector.z;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a matrix with 3 points copied to respective rows.
* @param [in] xVector The vector to insert in row 0
* @param [in] yVector The vector to insert in row 1
* @param [in] zVector The vector to insert in row 2
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitFromRowVectors
(

DVec3dCR xVector,
DVec3dCR yVector,
DVec3dCR zVector

)
    {
    form3d[0][0] = xVector.x;
    form3d[1][0] = yVector.x;
    form3d[2][0] = zVector.x;

    form3d[0][1] = xVector.y;
    form3d[1][1] = yVector.y;
    form3d[2][1] = zVector.y;

    form3d[0][2] = xVector.z;
    form3d[1][2] = yVector.z;
    form3d[2][2] = zVector.z;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a matrix representing rotation around a vector.
* @param [in] axis The axis of rotation
* @param [in] radians The rotation angle
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitFromVectorAndRotationAngle
(

DVec3dCR axis,
double   radians

)
    {
    double c = cos(radians);
    double s = sin(radians);
    double v = 1.0 - c;

    DVec3d dirVec;
    double mag = dirVec.Normalize (axis);

    if (mag == 0.0)
        {
        this->InitIdentity ();
        return;
        }

    this->InitFromScaledOuterProduct (dirVec, dirVec, v);

    form3d[0][0] += c;
    form3d[0][1] -= s * dirVec.z;
    form3d[0][2] += s * dirVec.y;

    form3d[1][0] += s * dirVec.z;
    form3d[1][1] += c;
    form3d[1][2] -= s * dirVec.x;

    form3d[2][0] -= s * dirVec.y;
    form3d[2][1] += s * dirVec.x;
    form3d[2][2] += c;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
RotMatrix RotMatrix::FromVectorAndRotationAngle
(
DVec3dCR axis,
double   radians,
RotMatrixR derivativeMatrix
)
    {
    RotMatrix matrix;
    double c = cos(radians);
    double s = sin(radians);
    double v = 1.0 - c;

    DVec3d dirVec;
    double mag = dirVec.Normalize (axis);

    if (mag == 0.0)
        {
        matrix.InitIdentity ();
        derivativeMatrix.Zero ();
        return matrix;
        }

    matrix.InitFromScaledOuterProduct (dirVec, dirVec, v);

    DVec3d vectorU = DVec3d::FromScale (dirVec, s);

    matrix.form3d[0][0] += c;
    matrix.form3d[0][1] -= vectorU.z;
    matrix.form3d[0][2] += vectorU.y;

    matrix.form3d[1][0] += vectorU.z;
    matrix.form3d[1][1] += c;
    matrix.form3d[1][2] -= vectorU.x;

    matrix.form3d[2][0] -= vectorU.y;
    matrix.form3d[2][1] += vectorU.x;
    matrix.form3d[2][2] += c;

    DVec3d vectorV = DVec3d::FromScale (dirVec, c);
    derivativeMatrix.InitFromRowValues (
        -s, -vectorV.z, vectorV.y,
        vectorV.z, -s, -vectorV.x,
        -vectorV.y, vectorV.x, -s
        );
    derivativeMatrix.AddScaledOuterProductInPlace (dirVec, dirVec, s);
    return matrix;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
RotMatrix RotMatrix::FromCrossingVector (DVec3dCR vector)
    {
    return RotMatrix::FromRowValues
        (
        0.0, -vector.z, vector.y,
        vector.z, 0.0, -vector.x,
        -vector.y, vector.x, 0.0
        );
// 3 positive, 3 negative.
// x,y,z each appear once positive and once negative.
// Each row and column has one positive, one negative
// skew partners are negated
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
RotMatrix RotMatrix::FromRotate90 (DVec3dCR axis)
    {
    RotMatrix matrix;
    DVec3d dirVec;
    double mag = dirVec.Normalize (axis);

    if (mag == 0.0)
        {
        matrix.InitIdentity ();
        return matrix;
        }


    double xy = dirVec.x * dirVec.y;
    double xz = dirVec.x * dirVec.z;
    double yz = dirVec.y * dirVec.z;
    matrix.form3d[0][0] = dirVec.x * dirVec.x;
    matrix.form3d[0][1] = xy - dirVec.z;
    matrix.form3d[0][2] = xz + dirVec.y;

    matrix.form3d[1][0] = xy + dirVec.z;
    matrix.form3d[1][1] = dirVec.y * dirVec.y;
    matrix.form3d[1][2] = yz - dirVec.x;

    matrix.form3d[2][0] = xz - dirVec.y;
    matrix.form3d[2][1] = yz + dirVec.x;
    matrix.form3d[2][2] = dirVec.z * dirVec.z;

    return matrix;
    }





/*-----------------------------------------------------------------*//**
* @description Returns a matrix which scales along a vector direction.
* @param [out] pMatrix scale matrix.
* @param [in] vector The scaling direction
* @param [in] scale The scale factor
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitFromDirectionAndScale
(

DVec3dCR vector,
double   scale

)
    {
    double denominator = vector.DotProduct (vector);
    if (denominator == 0.0)
        {
        this->InitIdentity ();
        }
    else
        {
        this->InitFromScaledOuterProduct (vector, vector, (scale - 1.0)/denominator);
        form3d[0][0] += 1.0;
        form3d[1][1] += 1.0;
        form3d[2][2] += 1.0;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Returns a matrix of rotation  about the x,y, or z axis
* (indicated by axis = 0,1, or 2) by an angle in radians.
*
* @param [in] axis The axis index 0=x, 1=y, 2=z
* @param [in] radians The rotation angle in radians
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitFromAxisAndRotationAngle
(

int     axis,
double  radians

)
    {
    double c = cos(radians);
    double s = sin(radians);
    this->InitIdentity ();

    if (2 == axis)
       {
       form3d[0][0] = form3d[1][1] = c;
       form3d[1][0] = s;
       form3d[0][1] = -s;
      }
    else if (1 == axis)
       {
       form3d[0][0] = form3d[2][2] = c;
       form3d[0][2] =  s;
       form3d[2][0] = -s;
       }
    else if (0 == axis)
       {
       form3d[1][1] = form3d[2][2] = c;
       form3d[1][2] = -s;
       form3d[2][1] =  s;
       }
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RotMatrix::TransposeOf (RotMatrixCR matrix)
    {
    RotMatrix matrix0 = matrix;

    /* EDL: Base MS version did not allow inplace transpose */
    

    form3d[0][0] = matrix0.form3d[0][0];
    form3d[1][0] = matrix0.form3d[0][1];
    form3d[2][0] = matrix0.form3d[0][2];

    form3d[0][1] = matrix0.form3d[1][0];
    form3d[1][1] = matrix0.form3d[1][1];
    form3d[2][1] = matrix0.form3d[1][2];

    form3d[0][2] = matrix0.form3d[2][0];
    form3d[1][2] = matrix0.form3d[2][1];
    form3d[2][2] = matrix0.form3d[2][2];
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
RotMatrix RotMatrix::FromTransposeOf (RotMatrixCR matrix)
    {
    RotMatrix result;
    result.TransposeOf (matrix);
    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RotMatrix::Transpose ()
    {
    this->TransposeOf (*this);
    }




/*-----------------------------------------------------------------*//**
* Sets this instance matrix by copying the matrix part of the trasnform.
* @param [out] pMatrix matrix part of transformation
* @param [in] transform The transformation whose matrix part is returned
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitFrom
(

TransformCR transform

)
    {
    int i, j;
    for (i = 0; i < 3; i++)
        {
        for (j = 0; j < 3; j++)
            {
            form3d[i][j] = transform.form3d[i][j];
            }
        }
    }




/*-----------------------------------------------------------------*//**
* @description Returns the product {RX*RY*RZ*M} where RX, RY, and RZ are rotations (in radians)
* around X, Y, and Z axes, and M is the input matrix.
* @param [in] inMatrix The optional prior matrix
* @param [in] xrot The x axis rotation
* @param [in] yrot The y axis rotation
* @param [in] zrot The z axis rotation
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitFromPrincipleAxisRotations
(

RotMatrixCR inMatrix,
double      xrot,
double      yrot,
double      zrot

)
    {
    RotMatrix factor;
    RotMatrix product;

    if (xrot != 0.0)
        {
        product.InitFromAxisAndRotationAngle (0, xrot);
        }
    else
        {
        product.InitIdentity ();
        }

    if (yrot != 0.0)
        {
        factor.InitFromAxisAndRotationAngle (1, yrot);
        product.InitProduct (product, factor);
        }

    if (zrot != 0.0)
        {
        factor.InitFromAxisAndRotationAngle (2, zrot);
        product.InitProduct (product, factor);
        }

    this->InitProduct (product, inMatrix);
    }




/*-----------------------------------------------------------------*//**
* @description Initializes this instance matrix so that the indicated axis (axis = 0,1,or 2)
* is aligned with the vector dir.   The normalize flag selects between
* normalized axes (all matrix columns of unit length) and unnormalized
* axes (all matrix columns of same length as the dir vector).
*
* @param [in] dir The fixed direction vector
* @param [in] axis The axis column to be aligned with direction
* @param [in] normalize true to have normalized columns
* @return true if the direction vector is nonzero.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::InitFrom1Vector
(

DVec3dCR dir,
int      axis,
bool     normalize

)
    {
    int ix, iy, iz;
    bool boolStat;
    DVec3d vector[3];

    Angle::Cyclic3dAxes(iz, ix, iy, axis);


    if (normalize)
        {
        boolStat = dir.GetNormalizedTriad (vector[ix], vector[iy], vector[iz]);
        }
    else
        {
        boolStat = dir.GetTriad (vector[ix], vector[iy], vector[iz]);
        }
    this->InitFromColumnVectors (vector[0], vector[1], vector[2]);
    return  boolStat;
    }


/*-----------------------------------------------------------------*//**
* @description Copies vectors into the x and y columns of this instance matrix
* and sets the z column to their cross product.
* Use squareAndNormalizeColumns to force these (possibly
* skewed and scaled) coordinate axes to be perpendicular
*
* @param [in] xVector The vector to place in column 0
* @param [in] yVector The vector to place in column 1
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void RotMatrix::InitFrom2Vectors
(

DVec3dCR xVector,
DVec3dCR yVector

)
    {
    DVec3d wVector;
    wVector.CrossProduct (xVector, yVector);
    this->InitFromColumnVectors (xVector, yVector, wVector);
    }


/*-----------------------------------------------------------------*//**
* @description Set this instance matrix to be an orthogonal (rotation) matrix with column 0 in the direction
* from the origin to the x point, column 1 in the plane of the 3 points, directed so the Y point
* on the positive side, and column 2 as their cross product.
* @param [in] origin The reference point
* @param [in] xPoint The x axis target point
* @param [in] yPoint The 3rd point defining xy plane.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::InitRotationFromOriginXY
(

DPoint3dCR origin,
DPoint3dCR xPoint,
DPoint3dCR yPoint

)
    {
    DVec3d U, V, W;
    double a;
    double relTol = s_mediumRelTol;
    U.NormalizedDifference (xPoint, origin);
    V.NormalizedDifference (yPoint, origin);
    a = W.NormalizedCrossProduct (U, V);

    if  (a <= relTol)
        {
        this->InitIdentity ();
        return  false;
        }
    else
        {
        V.NormalizedCrossProduct (W, U);
        this->InitFromColumnVectors (U, V, W);
        return  true;
        }
    }




/*-----------------------------------------------------------------*//**
* @description Returns a scaling matrix with respective x, y, and z
*   scaling factors.
* @param [in] xscale The x direction scale factor
* @param [in] yscale The y direction scale factor
* @param [in] zscale The z direction scale factor
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::FromScaleFactors
(
double          xscale,
double          yscale,
double          zscale
)
    {
    RotMatrix matrix;
    matrix.InitFromScaleFactors (xscale, yscale, zscale);

    return matrix;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a uniform scaling matrix.
* @param [in] scale The scale factor.
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::FromScale
(
double          scale
)
    {
    RotMatrix matrix;
    matrix.InitFromScale (scale);

    return matrix;
    }

/*-----------------------------------------------------------------*//**
*
* @description Returns a 'rank one' matrix defined as a scale factor times
* the 'vector outer product' of two vectors, i.e. as the matrix {s*U*V^T}
*
* @param [in] vector1 The column vector U
* @param [in] vector2 The row vector V
* @param [in] scale The scale factor
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::FromScaledOuterProduct
(
DVec3dCR        vector1,
DVec3dCR        vector2,
double          scale
)
    {
    RotMatrix matrix;
    matrix.InitFromScaledOuterProduct (vector1, vector2, scale);

    return matrix;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a matrix with the 9 specified coefficients given
*   in "row major" order.
* @param [in] x00 The 00 entry
* @param [in] x01 The 01 entry
* @param [in] x02 The 02 entry
* @param [in] x10 The 10 entry
* @param [in] x11 The 11 entry
* @param [in] x12 The 12 entry
* @param [in] x20 The 20 entry
* @param [in] x21 The 21 entry
* @param [in] x22 The 22 entry
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::FromRowValues
(
double          x00,
double          x01,
double          x02,
double          x10,
double          x11,
double          x12,
double          x20,
double          x21,
double          x22
)
    {
    RotMatrix matrix;
    matrix.InitFromRowValues (x00, x01, x02, x10, x11, x12, x20, x21, x22);

    return matrix;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a matrix with 3 points copied to respective columns.
* @param [in] xVector The vector to insert in column 0
* @param [in] yVector The vector to insert in column 1
* @param [in] zVector The vector to insert in column 2
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::FromColumnVectors
(
DVec3dCR        xVector,
DVec3dCR        yVector,
DVec3dCR        zVector
)
    {
    RotMatrix matrix;
    matrix.InitFromColumnVectors (xVector, yVector, zVector);

    return matrix;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a matrix with 3 points copied to respective rows.
* @param [in] xVector The vector to insert in row 0
* @param [in] yVector The vector to insert in row 1
* @param [in] zVector The vector to insert in row 2
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::FromRowVectors
(
DVec3dCR        xVector,
DVec3dCR        yVector,
DVec3dCR        zVector
)
    {
    RotMatrix matrix;
    matrix.InitFromRowVectors (xVector, yVector, zVector);

    return matrix;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a matrix representing rotation around a vector.
* @param [in] axis The axis of rotation
* @param [in] radians The rotation angle
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::FromVectorAndRotationAngle
(
DVec3dCR        axis,
double          radians
)
    {
    RotMatrix matrix;
    matrix.InitFromVectorAndRotationAngle (axis, radians);

    return matrix;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a matrix which scales along a vector direction.
* @param [in] vector The scaling direction
* @param [in] scale The scale factor
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::FromDirectionAndScale
(
DVec3dCR        vector,
double          scale
)
    {
    RotMatrix matrix;
    matrix.InitFromDirectionAndScale (vector, scale);

    return matrix;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a matrix of rotation  about the x,y, or z axis
* (indicated by axis = 0,1, or 2) by an angle in radians.
*
* @param [in] axis The axis index 0=x, 1=y, 2=z
* @param [in] radians The rotation angle in radians
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::FromAxisAndRotationAngle
(
int             axis,
double          radians
)
    {
    RotMatrix matrix;
    matrix.InitFromAxisAndRotationAngle (axis, radians);

    return matrix;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product {RX*RY*RZ*M} where RX, RY, and RZ are rotations (in radians)
* around X, Y, and Z axes, and M is the input matrix.
* @param [in] inMatrix The optional prior matrix
* @param [in] xrot The x axis rotation
* @param [in] yrot The y axis rotation
* @param [in] zrot The z axis rotation
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::FromPrincipleAxisRotations
(
RotMatrixCR     inMatrix,
double          xrot,
double          yrot,
double          zrot
)
    {
    RotMatrix matrix;
    matrix.InitFromPrincipleAxisRotations (inMatrix, xrot, yrot, zrot);

    return matrix;
    }



/*-----------------------------------------------------------------*//**
* @description Initializes this instance matrix so that the indicated axis (axis = 0,1,or 2)
* is aligned with the vector dir.   The normalize flag selects between
* normalized axes (all matrix columns of unit length) and unnormalized
* axes (all matrix columns of same length as the dir vector).
*
* @param [in] dir The fixed direction vector
* @param [in] axis The axis column to be aligned with direction
* @param [in] normalize true to have normalized columns
* @return true if the direction vector is nonzero.
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::From1Vector
(
DVec3dCR        dir,
int             axis,
bool            normalize
)
    {
    RotMatrix matrix;
    matrix.InitFrom1Vector (dir, axis, normalize);

    return matrix;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a matrix copying vectors into the x and y columns of this instance matrix
* and sets the z column to their cross product.
* Use squareAndNormalizeColumns to force these (possibly
* skewed and scaled) coordinate axes to be perpendicular
*
* @param [in] xVector The vector to place in column 0
* @param [in] yVector The vector to place in column 1
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::From2Vectors
(
DVec3dCR        xVector,
DVec3dCR        yVector
)
    {
    RotMatrix matrix;
    matrix.InitFrom2Vectors (xVector, yVector);

    return matrix;
    }

/*-----------------------------------------------------------------*//**
* @description Set this instance matrix to be an orthogonal (rotation) matrix with column 0 in the direction
* from the origin to the x point, column 1 in the plane of the 3 points, directed so the Y point
* on the positive side, and column 2 as their cross product.
* @param [in] origin The reference point
* @param [in] xPoint The x axis target point
* @param [in] yPoint The 3rd point defining xy plane.
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::RotationFromOriginXY
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
DPoint3dCR      yPoint
)
    {
    RotMatrix matrix;
    matrix.InitRotationFromOriginXY (origin, xPoint, yPoint);

    return matrix;
    }
/*-----------------------------------------------------------------*//**
* @description Returns a matrix copying the matrix part of the trasnform.
* @param [in] transform The transformation whose matrix part is returned
+----------------------------------------------------------------------*/
RotMatrix RotMatrix::From
(
TransformCR     transform
)
    {
    RotMatrix matrix;
    matrix.InitFrom (transform);

    return matrix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             06/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool RotMatrix::InitRotationFromVectorToVector
(
DVec3dCR startVector,  /* xyz components of start vector */
DVec3dCR endVector     /* xyz components of end vector */
)
    {
    double radians;
    DVec3d axis;
    if (startVector.AngleAndAxisOfRotationFromVectorToVector (endVector, axis, radians))
        {
        InitFromVectorAndRotationAngle (axis, radians);
        return true;
        }
    else
        {
        InitIdentity ();
        return false;
        }
    }

END_BENTLEY_NAMESPACE
