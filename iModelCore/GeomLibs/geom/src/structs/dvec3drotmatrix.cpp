/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dvec3drotmatrix.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/* VBSUB(Vec3dFromMatrix3dTimesVec3d) */

/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix times a vector.
* @param pResult <= result of the multiplication.
* @param pMatrix => The matrix.
* @param pVector => The known vector.
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_multiplyRotMatrixDVec3d

(
DVec3dP pResult,
RotMatrixCP pMatrix,
DVec3dCP pVector
)
    {
    DVec3d    inVec;

    inVec = *pVector;

    pResult->x =  pMatrix->form3d[0][0] * inVec.x
                + pMatrix->form3d[0][1] * inVec.y
                + pMatrix->form3d[0][2] * inVec.z;

    pResult->y =  pMatrix->form3d[1][0] * inVec.x
                + pMatrix->form3d[1][1] * inVec.y
                + pMatrix->form3d[1][2] * inVec.z;

    pResult->z =  pMatrix->form3d[2][0] * inVec.x
                + pMatrix->form3d[2][1] * inVec.y
                + pMatrix->form3d[2][2] * inVec.z;
    }

/* VBSUB(Vec3dFromMatrix3dTransposeTimesVec3d) */

/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix transpose times a vector.
* @param pResult <= result of the multiplication.
* @param pMatrix => The the matrix.
* @param pVector => The known vector.
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_multiplyRotMatrixTransposeDVec3d

(
DVec3dP pResult,
RotMatrixCP pMatrix,
DVec3dCP pVector
)
    {
    DVec3d    inVec;

    inVec = *pVector;

    pResult->x =  pMatrix->form3d[0][0] * inVec.x
                + pMatrix->form3d[1][0] * inVec.y
                + pMatrix->form3d[2][0] * inVec.z;

    pResult->y =  pMatrix->form3d[0][1] * inVec.x
                + pMatrix->form3d[1][1] * inVec.y
                + pMatrix->form3d[2][1] * inVec.z;

    pResult->z =  pMatrix->form3d[0][2] * inVec.x
                + pMatrix->form3d[1][2] * inVec.y
                + pMatrix->form3d[2][2] * inVec.z;
    }


/* VBSUB(Vec3dFromMatrix3dTimesXYZ) */

/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix times a vector,
*           with the vector given as separate components.
*
* @param pResult <= result of multiplication
* @param pMatrix => The matrix to apply
* @param x => The x component of input vector
* @param y => The y component of input vector
* @param z => The z component of input vector
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_multiplyRotMatrixXYZ

(
DVec3dP pResult,
RotMatrixCP pMatrix,
double         x,
double         y,
double         z
)
    {
    pResult->x =  pMatrix->form3d[0][0] * x
                + pMatrix->form3d[0][1] * y
                + pMatrix->form3d[0][2] * z;

    pResult->y =  pMatrix->form3d[1][0] * x
                + pMatrix->form3d[1][1] * y
                + pMatrix->form3d[1][2] * z;

    pResult->z =  pMatrix->form3d[2][0] * x
                + pMatrix->form3d[2][1] * y
                + pMatrix->form3d[2][2] * z;
    }



/* VBSUB(Vec3dFromMatrix3dTransposeTimesXYZ) */

/*-----------------------------------------------------------------*//**
* @description Returns the product P = [x,y,z]*M where M is the input matrix and P is
*           the product vector.
* @param pVector <= product vector
* @param pMatrix => The matrix to apply
* @param x => The x component
* @param y => The y component
* @param z => The z component
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_multiplyRotMatrixTransposeXYZ

(
DVec3dP pResult,
RotMatrixCP pMatrix,
double      x,
double      y,
double      z
)
    {
    pResult->x =  pMatrix->form3d[0][0] * x
                + pMatrix->form3d[1][0] * y
                + pMatrix->form3d[2][0] * z;

    pResult->y =  pMatrix->form3d[0][1] * x
                + pMatrix->form3d[1][1] * y
                + pMatrix->form3d[2][1] * z;

    pResult->z =  pMatrix->form3d[0][2] * x
                + pMatrix->form3d[1][2] * y
                + pMatrix->form3d[2][2] * z;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix times an array of vectors.
* @param pResultArray <= array of n multiplication results.
* @param pMatrix => The matrix.
* @param pResultArray => The array of vactors
* @param count => number of vectors
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_multiplyRotMatrixDVec3dArray

(
DVec3dP pResultArray,
RotMatrixCP pMatrix,
DVec3dCP pVectorArray,
int          count
)
    {
    int i;
    for (i = 0; i < count; i++)
        {
        bsiDVec3d_multiplyRotMatrixDVec3d (pResultArray + i, pMatrix, pVectorArray);
        }
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix times an array of vectors.
* @param pResultArray <= array of n multiplication results.
* @param pMatrix => The matrix.
* @param pResultArray => The array of vactors
* @param count => number of vectors
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_multiplyRotMatrixTransposeDVec3dArray

(
DVec3dP pResultArray,
RotMatrixCP pMatrix,
DVec3dCP pVectorArray,
int          count
)
    {
    int i;
    for (i = 0; i < count; i++)
        {
        bsiDVec3d_multiplyRotMatrixTransposeDVec3d (pResultArray + i, pMatrix, pVectorArray);
        }
    }

/* VBSUB(Vec3dFromMatrix3dColumn) */

/*-----------------------------------------------------------------*//**
* @description Extracts a column of a matrix.
* @param pVector <= the vector
* @param pMatrix => The matrix.
* @param col => column index. Columns are numbered 0,1,2.  Others
        indices are reduced cyclically.
* @indexVerb multiply
* @bsihdr                                       EarlinLutz      04/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_fromRotMatrixColumn

(
DVec3dP pVector,
RotMatrixCP pMatrix,
int         col
)
    {
    col = bsiGeom_cyclic3dAxis (col);
    pVector->x = pMatrix->form3d[0][col];
    pVector->y = pMatrix->form3d[1][col];
    pVector->z = pMatrix->form3d[2][col];
    }

/* VBSUB(Vec3dFromMatrix3dRow) */

/*-----------------------------------------------------------------*//**
* @description Extracts a row of a matrix.
* @param pVector <= the vector
* @param pMatrix => The matrix.
* @param row => row index. Rows are numbered 0,1,2.  Others
        indices are reduced cyclically.
* @indexVerb multiply
* @bsihdr                                       EarlinLutz      04/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_fromRotMatrixRow

(
DVec3dP pVector,
RotMatrixCP pMatrix,
int            row
)
    {
    row = bsiGeom_cyclic3dAxis (row);
    /* Ugly case branches, but necessary for RotMatrix/DMatrix3d code sharing. */
    if ( row == 0)
        {
        pVector->x = pMatrix->form3d[0][0];
        pVector->y = pMatrix->form3d[0][1];
        pVector->z = pMatrix->form3d[0][2];
        }
    else if (row == 1)
        {
        pVector->x = pMatrix->form3d[1][0];
        pVector->y = pMatrix->form3d[1][1];
        pVector->z = pMatrix->form3d[1][2];
        }
    else if (row == 2)
        {
        pVector->x = pMatrix->form3d[2][0];
        pVector->y = pMatrix->form3d[2][1];
        pVector->z = pMatrix->form3d[2][2];
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE
