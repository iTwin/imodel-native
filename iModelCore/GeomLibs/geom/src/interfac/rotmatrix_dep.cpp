/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/interfac/rotmatrix_dep.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------
This file contains DEPRECATED functions originally in dmatrix3d.c
+----------------------------------------------------------------------*/


/*----------------------------------------------------------------------+
|                                                                       |
|   Include Files                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE





/*-----------------------------------------------------------------*//**
* Computes the product {M*P} where M is this instance matrix and P the input pPoint.
* The result overwrites the previous coordinates in pPoint.
*
* @instance pMatrix => The matrix to apply
* @param pPoint <=> The point to be updated
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyDPoint3d
(
const RotMatrix     *pMatrix,
      DPoint3d      *pPoint
)
    {
    DPoint3d    inPoint;

    inPoint = *pPoint;

    pPoint->x =   pMatrix->form3d[0][0] * inPoint.x
                + pMatrix->form3d[0][1] * inPoint.y
                + pMatrix->form3d[0][2] * inPoint.z;

    pPoint->y =   pMatrix->form3d[1][0] * inPoint.x
                + pMatrix->form3d[1][1] * inPoint.y
                + pMatrix->form3d[1][2] * inPoint.z;

    pPoint->z =   pMatrix->form3d[2][0] * inPoint.x
                + pMatrix->form3d[2][1] * inPoint.y
                + pMatrix->form3d[2][2] * inPoint.z;
    }

/* VBSUB(Point3dFromMatrix3dTimesPoint3d) */
/* CSVFUNC(multiply) */

/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix times a point.
* @instance pMatrix => The matrix.
* @param pResult <= result of the multiplication.
* @param pPoint => The known point.
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixDPoint3d
(
const RotMatrix     *pMatrix,
      DPoint3d      *pResult,
const DPoint3d      *pPoint
)
    {
    DPoint3d    inPoint;

    inPoint = *pPoint;

    pResult->x =  pMatrix->form3d[0][0] * inPoint.x
                + pMatrix->form3d[0][1] * inPoint.y
                + pMatrix->form3d[0][2] * inPoint.z;

    pResult->y =  pMatrix->form3d[1][0] * inPoint.x
                + pMatrix->form3d[1][1] * inPoint.y
                + pMatrix->form3d[1][2] * inPoint.z;

    pResult->z =  pMatrix->form3d[2][0] * inPoint.x
                + pMatrix->form3d[2][1] * inPoint.y
                + pMatrix->form3d[2][2] * inPoint.z;
    }

/* VBSUB(Point3dFromMatrix3dTransposeTimesPoint3d) */
/* CSVFUNC(multiplyTranspose) */

/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix transpose times a point.
* @instance pMatrix => The the matrix.
* @param pResult <= result of the multiplication.
* @param pPoint => The known point.
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixTransposeDPoint3d
(
const RotMatrix     *pMatrix,
      DPoint3d      *pResult,
const DPoint3d      *pPoint
)
    {
    DPoint3d    inPoint;

    inPoint = *pPoint;

    pResult->x =  pMatrix->form3d[0][0] * inPoint.x
                + pMatrix->form3d[1][0] * inPoint.y
                + pMatrix->form3d[2][0] * inPoint.z;

    pResult->y =  pMatrix->form3d[0][1] * inPoint.x
                + pMatrix->form3d[1][1] * inPoint.y
                + pMatrix->form3d[2][1] * inPoint.z;

    pResult->z =  pMatrix->form3d[0][2] * inPoint.x
                + pMatrix->form3d[1][2] * inPoint.y
                + pMatrix->form3d[2][2] * inPoint.z;
    }


/* VBSUB(Point3dFromMatrix3dTimesXYZ) */
/* CSVFUNC(multiply) */

/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix times a point,
*           with the point given as separate components.
*
* @instance pMatrix => The matrix to apply
* @param pResult <= result of multiplication
* @param x => The x component of input point
* @param y => The y component of input point
* @param z => The z component of input point
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyComponents
(
const RotMatrix     *pMatrix,
      DPoint3d      *pResult,
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




/*-----------------------------------------------------------------*//**
* @description Computes the product {P*M} where M is this instance matrix and P is the input point
* The result overwrites the previous coordinates in the point.
* @instance pMatrix => The matrix to apply
* @param pPoint <=> The point to be multiplied
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyTransposeDPoint3d
(
const RotMatrix     *pMatrix,
      DPoint3d      *pPoint
)
    {
    DPoint3d    inPoint;

    inPoint = *pPoint;

    pPoint->x =   pMatrix->form3d[0][0]*inPoint.x
                + pMatrix->form3d[1][0]*inPoint.y
                + pMatrix->form3d[2][0]*inPoint.z;

    pPoint->y =   pMatrix->form3d[0][1]*inPoint.x
                + pMatrix->form3d[1][1]*inPoint.y
                + pMatrix->form3d[2][1]*inPoint.z;

    pPoint->z =   pMatrix->form3d[0][2]*inPoint.x
                + pMatrix->form3d[1][2]*inPoint.y
                + pMatrix->form3d[2][2]*inPoint.z;

    }

/* VBSUB(Point3dFromMatrix3dTransposeTimesXYZ) */
/* CSVFUNC(multiplyTranspose) */

/*-----------------------------------------------------------------*//**
* @description Returns the product P = [x,y,z]*M where M is the input matrix and P is
*           the product point.
* @instance pMatrix => The matrix to apply
* @param pPoint <= product point
* @param x => The x component
* @param y => The y component
* @param z => The z component
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyTransposeComponents
(
const RotMatrix     *pMatrix,
      DPoint3d      *pResult,
      double        x,
      double        y,
      double        z
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
* Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
* Each result is placed in the corresponding entry in the output array pResult.   The same array
* may be named for the input and output arrays.
* @param pMatrix => The matrix to apply
* @param pResult <= output points
* @param pPoint => The input points
* @param numPoint => The number of points
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyDPoint3dArray
(
const RotMatrix     *pMatrix,
DPoint3d      *pResult,
const DPoint3d      *pPoint,
      int            numPoint
)
    {
    int     i;
    double x,y,z;
    DPoint3d *pResultPoint;

    for (i = 0, pResultPoint = pResult;
         i < numPoint;
         i++, pResultPoint++
        )
        {
        x = pPoint[i].x;
        y = pPoint[i].y;
        z = pPoint[i].z;

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
* Computes {P[i]*M} where M is this instance matrix and each P[i] is a point
*   in the input array pPoint, transposed into row form.  (Equivalently, multiply M^*p[i], where M^ indicates transpose.)
* Each result is placed in the corresponding entry in the output array pResult.   The same array
* may be named for the input and output arrays.
* @param pMatrix => The matrix to apply
* @param pResult <= output points
* @param pPoint => The input points
* @param numPoint => The number of points
* @indexVerb multiply
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyTransposeDPoint3dArray
(
const RotMatrix     *pMatrix,
DPoint3d      *pResult,
const DPoint3d      *pPoint,
      int            numPoint
)
    {
    int     i;
    double x,y,z;
    DPoint3d *pResultPoint;

    for (i = 0, pResultPoint = pResult;
         i < numPoint;
         i++, pResultPoint++
        )
        {
        x = pPoint[i].x;
        y = pPoint[i].y;
        z = pPoint[i].z;

        pResultPoint->x = pMatrix->form3d[0][0] * x
                + pMatrix->form3d[1][0] * y
                + pMatrix->form3d[2][0] * z;

        pResultPoint->y = pMatrix->form3d[0][1] * x
                + pMatrix->form3d[1][1] * y
                + pMatrix->form3d[2][1] * z;

        pResultPoint->z = pMatrix->form3d[0][2] * x
                + pMatrix->form3d[1][2] * y
                + pMatrix->form3d[2][2] * z;

        }
    }

/* VBSUB(Point3dFromMatrix3dInverseTimesPoint3d) */


/*-----------------------------------------------------------------*//**
* @description Return the product of a matrix inverse and a point.
* @vbexception Throws an exception if the matrix is singular (can't be inverted).
* @instance pMatrix     => The matrix
* @param    pResult   <= the unknown point
* @param    pPoint     => The The known point
* @return false if this instance is singular.
* @bsihdr                                                       DavidAssaf      1/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_solveDPoint3d
(
const RotMatrix     *pMatrix,
      DPoint3d      *pResult,
const DPoint3d      *pPoint
)
    {
    DVec3d      col0, col1, col2, point;
    double      det, inverseDet;

    bsiRotMatrix_getColumns (pMatrix, &col0, &col1, &col2);

    det = bsiDPoint3d_tripleProduct (&col0, &col1, &col2);

    if (det != 0.0)
        {
        inverseDet = 1.0 / det;

        point.x = bsiDPoint3d_tripleProduct (&col1, &col2, pPoint) * inverseDet;
        point.y = bsiDPoint3d_tripleProduct (&col2, &col0, pPoint) * inverseDet;
        point.z = bsiDPoint3d_tripleProduct (&col0, &col1, pPoint) * inverseDet;

        *pResult = point;
        }
    else
        {
        /* Matrix is singular.  Treat it as the identity. */
        *pResult = *pPoint;
        return false;
        }

    return true;
    }

/* VBSUB(Point3dFromMatrix3dInverseTransposeTimesPoint3d) */


/*-----------------------------------------------------------------*//**
* @description Return the product of a matrix inverse transpose and a point.
* @vbexception Throws an exception if the matrix is singular (can't be inverted).
* @instance pMatrix => The matrix whose transpose is inverted
* @param    pResult <= result of the multiplication
* @param    pPoint => The known point multipled by the matrix inverse.
* @return false if this instance is singular.
* @bsihdr                                                       DavidAssaf      1/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_solveDPoint3dTranspose
(
const RotMatrix     *pMatrix,
      DPoint3d      *pResult,
const DPoint3d      *pPoint
)
    {
    DVec3d      row0, row1, row2, point;
    double      det, inverseDet;

    bsiRotMatrix_getRows (pMatrix, &row0, &row1, &row2);

    det = bsiDPoint3d_tripleProduct (&row0, &row1, &row2);

    if (det != 0.0)
        {
        inverseDet = 1.0 / det;

        point.x = bsiDPoint3d_tripleProduct (&row1, &row2, pPoint) * inverseDet;
        point.y = bsiDPoint3d_tripleProduct (&row2, &row0, pPoint) * inverseDet;
        point.z = bsiDPoint3d_tripleProduct (&row0, &row1, pPoint) * inverseDet;

        *pResult = point;
        }
    else
        {
        /* Matrix is singular.  Treat it as the identity. */
        *pResult = *pPoint;
        return false;
        }

    return true;
    }



/*-----------------------------------------------------------------*//**
* @description Solves the matrix equation AX=B, where A is this instance, B is the matrix
* of numPoints input points and X is the matrix of numPoints output points.
* pPoint and pResult may have identical addresses.
*
* @instance pMatrix             => The matrix
* @param    pResult   <= column points of solution matrix to system
* @param    pPoint    => The column points of constant matrix of system
* @param    numPoints   => The number of input/output points
* @return false if this instance is singular.
* @bsihdr                                                       DavidAssaf      1/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_solveDPoint3dArray
(
const RotMatrix     *pMatrix,
      DPoint3d      *pResult,
const DPoint3d      *pPoint,
      int           numPoints
)
    {
    const DPoint3d  *pIn;
    DVec3d          col0, col1, col2, row0, row1, row2, point;
    double          det, inverseDet;
    int             i;

    bsiRotMatrix_getColumns (pMatrix, &col0, &col1, &col2);

    det = bsiDPoint3d_tripleProduct (&col0, &col1, &col2);

    if (det != 0.0)
        {
        inverseDet = 1.0 / det;

        /* construct rows of inverse of this instance */
        bsiDPoint3d_crossProduct (&row0, &col1, &col2);
        bsiDPoint3d_scaleInPlace (&row0, inverseDet);
        bsiDPoint3d_crossProduct (&row1, &col2, &col0);
        bsiDPoint3d_scaleInPlace (&row1, inverseDet);
        bsiDPoint3d_crossProduct (&row2, &col0, &col1);
        bsiDPoint3d_scaleInPlace (&row2, inverseDet);

        for (i = 0, pIn = pPoint; i < numPoints; i++, pIn++)
            {
            point.x = bsiDPoint3d_dotProduct (&row0, pIn);
            point.y = bsiDPoint3d_dotProduct (&row1, pIn);
            point.z = bsiDPoint3d_dotProduct (&row2, pIn);
            pResult[i] = point;
            }
        }
    else
        {
        /* Matrix is singular. Treat it as the identity. */
        bsiDPoint3d_copyArray (pResult, pPoint, numPoints);
        return false;
        }

    return true;
    }

END_BENTLEY_NAMESPACE
