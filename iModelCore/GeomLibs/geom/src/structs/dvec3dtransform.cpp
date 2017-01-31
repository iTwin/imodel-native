/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dvec3dtransform.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/* VBSUB(Vector3dFromTransform3dTimesVector3d) */

/*-----------------------------------------------------------------*//**
* @description Returns the product of a (matrix part of a) transform times a vector.
* @param pResult OUT result of the multiplication.
* @param pTransform IN The transform
* @param pVector IN The known vector.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_multiplyTransformDVec3d

(
DVec3dP pResult,
TransformCP pTransform,
DVec3dCP pVector
)
    {
    DVec3d    inVec;

    inVec = *pVector;

    pResult->x =  pTransform->form3d[0][0] * inVec.x
                + pTransform->form3d[0][1] * inVec.y
                + pTransform->form3d[0][2] * inVec.z;

    pResult->y =  pTransform->form3d[1][0] * inVec.x
                + pTransform->form3d[1][1] * inVec.y
                + pTransform->form3d[1][2] * inVec.z;

    pResult->z =  pTransform->form3d[2][0] * inVec.x
                + pTransform->form3d[2][1] * inVec.y
                + pTransform->form3d[2][2] * inVec.z;
    }

/* VBSUB(Vector3dFromTransform3dTransposeTimesVector3d) */

/*-----------------------------------------------------------------*//**
* @description Returns the product of a  (matrix part of a) transform transpose times a vector.
* @param pResult OUT result of the multiplication.
* @param pTransform IN The transform
* @param pVector IN The known vector.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_multiplyTransformTransposeDVec3d

(
DVec3dP pResult,
TransformCP pTransform,
DVec3dCP pVector
)
    {
    DVec3d    inVec;

    inVec = *pVector;

    pResult->x =  pTransform->form3d[0][0] * inVec.x
                + pTransform->form3d[1][0] * inVec.y
                + pTransform->form3d[2][0] * inVec.z;

    pResult->y =  pTransform->form3d[0][1] * inVec.x
                + pTransform->form3d[1][1] * inVec.y
                + pTransform->form3d[2][1] * inVec.z;

    pResult->z =  pTransform->form3d[0][2] * inVec.x
                + pTransform->form3d[1][2] * inVec.y
                + pTransform->form3d[2][2] * inVec.z;
    }

/* VBSUB(Vector3dFromTransform3dTimesXYZ) */

/*-----------------------------------------------------------------*//**
* @description Returns the product of a  (matrix part of a) transform times a vector,
*           with the vector given as separate components.
*
* @param pResult OUT result of multiplication
* @param pTransform IN The transform
* @param x IN The x component of input vector
* @param y IN The y component of input vector
* @param z IN The z component of input vector
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_multiplyTransformXYZ

(
DVec3dP pResult,
TransformCP pTransform,
double         x,
double         y,
double         z
)
    {
    pResult->x =  pTransform->form3d[0][0] * x
                + pTransform->form3d[0][1] * y
                + pTransform->form3d[0][2] * z;

    pResult->y =  pTransform->form3d[1][0] * x
                + pTransform->form3d[1][1] * y
                + pTransform->form3d[1][2] * z;

    pResult->z =  pTransform->form3d[2][0] * x
                + pTransform->form3d[2][1] * y
                + pTransform->form3d[2][2] * z;
    }

/* VBSUB(Vector3dFromTransform3dTransposeTimesXYZ) */

/*-----------------------------------------------------------------*//**
* @description Returns the product of a  (matrix part of a) transform transposed times a vector,
*           with the vector given as separate components.
* @param pVector OUT product vector
* @param pTransform IN The transform
* @param x IN The x component
* @param y IN The y component
* @param z IN The z component
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_multiplyTransformTransposeXYZ

(
DVec3dP pResult,
TransformCP pTransform,
double      x,
double      y,
double      z
)
    {
    pResult->x =  pTransform->form3d[0][0] * x
                + pTransform->form3d[1][0] * y
                + pTransform->form3d[2][0] * z;

    pResult->y =  pTransform->form3d[0][1] * x
                + pTransform->form3d[1][1] * y
                + pTransform->form3d[2][1] * z;

    pResult->z =  pTransform->form3d[0][2] * x
                + pTransform->form3d[1][2] * y
                + pTransform->form3d[2][2] * z;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of a (matrix part of a) transform times an array of vectors.
* @param pResultArray OUT array of n multiplication results.
* @param pTransform IN The transform
* @param pResultArray IN The array of vactors
* @param count IN number of vectors
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_multiplyTransformDVec3dArray

(
DVec3dP pResultArray,
TransformCP pTransform,
DVec3dCP pVectorArray,
int          count
)
    {
    int i;
    for (i = 0; i < count; i++)
        {
        bsiDVec3d_multiplyTransformDVec3d (pResultArray + i, pTransform, pVectorArray);
        }
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of a (matrix part of a) transform times an array of vectors.
* @param pResultArray OUT array of n multiplication results.
* @param pTransform IN The transform
* @param pResultArray IN The array of vactors
* @param count IN number of vectors
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_multiplyTransformTransposeDVec3dArray

(
DVec3dP pResultArray,
TransformCP pTransform,
DVec3dCP pVectorArray,
int          count
)
    {
    int i;
    for (i = 0; i < count; i++)
        {
        bsiDVec3d_multiplyTransformTransposeDVec3d (pResultArray + i, pTransform, pVectorArray);
        }
    }

/* VBSUB(Vector3dFromTransform3dColumn) */

/*-----------------------------------------------------------------*//**
* @description Extracts a column of the matrix part of a transform
*   Note that this addresses only the matrix part.
* @param pVector OUT the vector
* @param pTransform IN The transform
* @param col IN column index. Columns are numbered 0,1,2.  Others
        indices are reduced cyclically.
* @bsihdr                                       EarlinLutz      04/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_fromTransformColumn

(
DVec3dP pVector,
TransformCP pTransform,
int         col
)
    {
    col = bsiGeom_cyclic3dAxis (col);
    pVector->x = pTransform->form3d[0][col];
    pVector->y = pTransform->form3d[1][col];
    pVector->z = pTransform->form3d[2][col];
    }

/* VBSUB(Vector3dFromTransform3dTranslation) */

/*-----------------------------------------------------------------*//**
* @description Extracts the translation part of the transform as a vector
* @param pVector OUT the vector
* @param pTransform IN The transform
* @bsihdr                                       EarlinLutz      04/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_fromTransformTranslation

(
DVec3dP pVector,
TransformCP pTransform
)
    {
    pVector->x = pTransform->form3d[0][3];
    pVector->y = pTransform->form3d[1][3];
    pVector->z = pTransform->form3d[2][3];
    }

/* VBSUB(Vector3dFromTransform3dRow) */

/*-----------------------------------------------------------------*//**
* @description Extracts a row of the matrix part of a transform.
* @param pVector OUT the vector
* @param pTransform IN The transform
* @param row IN row index. Rows are numbered 0,1,2.  Others
        indices are reduced cyclically.
* @bsihdr                                       EarlinLutz      04/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDVec3d_fromTransformRow

(
DVec3dP pVector,
TransformCP pTransform,
int            row
)
    {
    row = bsiGeom_cyclic3dAxis (row);
    pVector->x = pTransform->form3d[row][0];
    pVector->y = pTransform->form3d[row][1];
    pVector->z = pTransform->form3d[row][2];
    }

END_BENTLEY_GEOMETRY_NAMESPACE
