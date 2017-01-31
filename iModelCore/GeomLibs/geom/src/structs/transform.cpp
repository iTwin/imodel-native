/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/transform.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*//**
 @struct Transform
 A Transform is a classic microstation affine transformation.

 In original mdlTMatrix functions, the runtime 2d/3d switch determined
 whether the transforms would be addressed through the
 "form2d" union as a 2x3 matrix, or through the
 "form3d" union as a 3x4 matrix.

 In jmdlTransform functions, the transform is always addressed through
 the "form3d" union.  This ensures universal 3d behavior within the
 jmdlTransform functions, but beware that passing the same Transform
 (by address) to mdl functions will produce unpredictable results if
 the masterfile mode is 2d.
 @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE


#define __Transform__
/* VBSUB(Transform3dGetMatrixComponentByRowAndColumn) */


/*-----------------------------------------------------------------*//**
 @description Returns a value from a specified row and column of the matrix part of the transformation.
 @instance pTransform => The input transformation.
 @param row => The index of row to read.  Row indices are 0, 1, 2.
 @param col => The index of column to read.  Column indices are 0, 1, 2.
 @return requested component of the matrix
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRotMatrix_getMatrixComponentByRowAndColumn
(
TransformCP pTransform,
int            row,
int            col
)
    {
    double value;
    row = bsiGeom_cyclic3dAxis (row);
    col = bsiGeom_cyclic3dAxis (col);

#ifdef __Transform__
    value = pTransform->form3d[row][col];
#else
    value = bsiDPoint3d_getComponent (&pTransform->matrix.column[col], row);
#endif
    return value;
    }



/*-----------------------------------------------------------------*//**
 @description Sets a value at a specified row and column of the matrix part of the transformation
 @instance pTransform => The input transformation.
 @param row => The index of row to set.
 @param col => The index of column to set.
 @param value  => the value to set.
 @group "Transform In Place Modification"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_setMatrixComponentByRowAndColumn
(
TransformP pTransform,
int            row,
int            col,
double         value
)
    {
    row = bsiGeom_cyclic3dAxis (row);
    col = bsiGeom_cyclic3dAxis (col);

#ifdef __Transform__
    pTransform->form3d[row][col] = value;
#else
    bsiDPoint3d_setComponent (&pTransform->matrix.column[col], value, row);
#endif
    }

/* VBSUB(Transform3dGetPointComponent) */


/*-----------------------------------------------------------------*//**
 @description Returns a value from a specified component of the point (translation) part of the transformation.
 @instance pTransform => The input transformation.
 @param row => The index of point component to read.  Indices are 0, 1, 2 for x, y, z
 @return requested component of translation
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRotMatrix_getTranslationComponent
(
TransformCP pTransform,
int            row
)
    {
    double value;
    row = bsiGeom_cyclic3dAxis (row);

#ifdef __Transform__
    value = pTransform->form3d[row][3];
#else
    value = bsiDPoint3d_getComponent (&pTransform->translation, row);
#endif
    return value;
    }



/*-----------------------------------------------------------------*//**
 @description Sets a value at a specified row of the point (translation) part of the transformation.
 @instance pTransform => The input transformation.
 @param row => The index of the point component to set
 @param value  => the value to set.
 @group "Transform In Place Modification"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_setTranslationComponent
(
TransformP pTransform,
int            row,
double         value
)
    {
    row = bsiGeom_cyclic3dAxis (row);

#ifdef __Transform__
    pTransform->form3d[row][3] = value;
#else
    bsiDPoint3d_setComponent (&pTransform->translation, value, row);
#endif
    }


/*-----------------------------------------------------------------*//**
 @description Multiplies a point by a transform, returning the result in
   place of the input point.
 @instance pTransform => The transformation matrix
 @param pPoint <=> point to be updated
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyDPoint3dInPlace
(
TransformCP pTransform,
DPoint3dP pPoint
)
    {
    bsiTransform_multiplyDPoint3d (pTransform, pPoint, pPoint);
    }

/* VBSUB(Point3dFromTransform3dTimesPoint3d) */
/* CSVFUNC(multiply) */

/*-----------------------------------------------------------------*//**
 @description Returns the product of a transform times a point.
 @instance pTransform => The transform to apply.
 @param pResult <= returned point.
 @param pPoint => The input point.
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyDPoint3d
(
TransformCP pTransform,
DPoint3dP pResult,
DPoint3dCP pPoint
)
    {
    DPoint3d    inPoint;

    inPoint = *pPoint;

    pResult->x =  pTransform->form3d[0][0]*inPoint.x
                + pTransform->form3d[0][1]*inPoint.y
                + pTransform->form3d[0][2]*inPoint.z
                + pTransform->form3d[0][3];

    pResult->y =  pTransform->form3d[1][0]*inPoint.x
                + pTransform->form3d[1][1]*inPoint.y
                + pTransform->form3d[1][2]*inPoint.z
                + pTransform->form3d[1][3];

    pResult->z =  pTransform->form3d[2][0]*inPoint.x
                + pTransform->form3d[2][1]*inPoint.y
                + pTransform->form3d[2][2]*inPoint.z
                + pTransform->form3d[2][3];
    }



/*-----------------------------------------------------------------*//**
 Multiplies a "weighted point" in place.  That is, the point is input and output as
   (wx,wy,wz,w) where x,y,z are the cartesian image coordinates.

 @instance pTransform => The transformation matrix
 @param pPoint <=> point to be updated
 @param weight => The weight
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyWeightedDPoint3dInPlace
(
TransformCP pTransform,
DPoint3dP pPoint,
double          weight
)
    {
    DPoint3d    inPoint;

    inPoint = *pPoint;

    pPoint->x =   pTransform->form3d[0][0]*inPoint.x
                + pTransform->form3d[0][1]*inPoint.y
                + pTransform->form3d[0][2]*inPoint.z
                + pTransform->form3d[0][3] * weight;

    pPoint->y =   pTransform->form3d[1][0]*inPoint.x
                + pTransform->form3d[1][1]*inPoint.y
                + pTransform->form3d[1][2]*inPoint.z
                + pTransform->form3d[1][3] * weight;

    pPoint->z =   pTransform->form3d[2][0]*inPoint.x
                + pTransform->form3d[2][1]*inPoint.y
                + pTransform->form3d[2][2]*inPoint.z
                + pTransform->form3d[2][3] * weight;
    }


/*-----------------------------------------------------------------*//**
 Multiplies an array of "weighted points" in place.  That is, the point is input and output as
   (wx,wy,wz,w) where x,y,z are the cartesian image coordinates.

 @instance pTransform => The transformation matrix
 @param pOutPoint  => The transformed points.
 @param pInPoint   => The original points
 @param pWeight    => The weight array.  If null, unit weight is used.
 @param numPoint   => number of points in arrays.
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyWeightedDPoint3dArray
(
TransformCP pTransform,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
const double    *pWeight,
int             numPoint
)
    {
    DPoint3d    inPoint;
    double weight;

    int i;

    if (!pWeight)
        {
        bsiTransform_multiplyDPoint3dArray (pTransform, pOutPoint, pInPoint, numPoint);
        }
    else
        {
        for (i = 0; i < numPoint; i++)
            {
            inPoint = pInPoint[i];
            weight  = pWeight[i];

            pOutPoint[i].x = pTransform->form3d[0][0] * inPoint.x
                           + pTransform->form3d[0][1] * inPoint.y
                           + pTransform->form3d[0][2] * inPoint.z
                           + pTransform->form3d[0][3] * weight;

            pOutPoint[i].y = pTransform->form3d[1][0] * inPoint.x
                           + pTransform->form3d[1][1] * inPoint.y
                           + pTransform->form3d[1][2] * inPoint.z
                           + pTransform->form3d[1][3] * weight;

            pOutPoint[i].z = pTransform->form3d[2][0] * inPoint.x
                           + pTransform->form3d[2][1] * inPoint.y
                           + pTransform->form3d[2][2] * inPoint.z
                           + pTransform->form3d[2][3] * weight;
            }
        }
    }


/*-----------------------------------------------------------------*//**
 Multiplies this instance times the column vector pPoint and replaces pPoint
 with the result, using the transpose of the matrix part of this instance in
 the multiplication.
 Symbolically, this is equivalent to being given transform [R t] and row
 vector p, and returning the point p*R + t.

 @instance pTransform => The transformation matrix
 @param pPoint <=> point to be updated
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyTransposePoint
(
TransformCP pTransform,
DPoint3dP pPoint
)
    {
    DPoint3d    inPoint;

    inPoint = *pPoint;

    pPoint->x =   pTransform->form3d[0][0]*inPoint.x
                + pTransform->form3d[1][0]*inPoint.y
                + pTransform->form3d[2][0]*inPoint.z
                + pTransform->form3d[0][3];

    pPoint->y =   pTransform->form3d[0][1]*inPoint.x
                + pTransform->form3d[1][1]*inPoint.y
                + pTransform->form3d[2][1]*inPoint.z
                + pTransform->form3d[1][3];

    pPoint->z =   pTransform->form3d[0][2]*inPoint.x
                + pTransform->form3d[1][2]*inPoint.y
                + pTransform->form3d[2][2]*inPoint.z
                + pTransform->form3d[2][3];
    }

/* VBSUB(Point3dFromTransform3dTimesXYZ) */
/* CSVFUNC(multiply) */

/*-----------------------------------------------------------------*//**
 @description Returns the product of a matrix times a point, with the point
       specified as explict x, y, and z values.
 @instance pTransform => The transformation to apply
 @param pPoint <= result of transformation * point operation
 @param x => The x component of the point
 @param y => The y component of the point
 @param z => The z component of the point
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyComponents
(
TransformCP pTransform,
DPoint3dP pPoint,
double         x,
double         y,
double         z
)
    {
    pPoint->x =   pTransform->form3d[0][3]
                + pTransform->form3d[0][0] * x
                + pTransform->form3d[0][1] * y
                + pTransform->form3d[0][2] * z;

    pPoint->y =   pTransform->form3d[1][3]
                + pTransform->form3d[1][0] * x
                + pTransform->form3d[1][1] * y
                + pTransform->form3d[1][2] * z;

    pPoint->z =   pTransform->form3d[2][3]
                + pTransform->form3d[2][0] * x
                + pTransform->form3d[2][1] * y
                + pTransform->form3d[2][2] * z;
    }



/*-----------------------------------------------------------------*//**
 Multiplies this instance times the column vector constructed from components
 x,y,z, using the transpose of the matrix part of this instance in the
 multiplication.
 Symbolically, this is equivalent to being given transform [R t] and row
 vector p, and returning the point p*R + t.

 @instance pTransform => The transformation to apply
 @param pPoint <= result of transformation * point operation
 @param x => The x component of the point
 @param y => The y component of the point
 @param z => The z component of the point
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyTransposeComponents
(
TransformCP pTransform,
DPoint3dP pPoint,
double         x,
double         y,
double         z
)
    {
    pPoint->x =   pTransform->form3d[0][0] * x
                + pTransform->form3d[1][0] * y
                + pTransform->form3d[2][0] * z
                + pTransform->form3d[0][3];

    pPoint->y =   pTransform->form3d[0][1] * x
                + pTransform->form3d[1][1] * y
                + pTransform->form3d[2][1] * z
                + pTransform->form3d[1][3];

    pPoint->z =   pTransform->form3d[0][2] * x
                + pTransform->form3d[1][2] * y
                + pTransform->form3d[2][2] * z
                + pTransform->form3d[2][3];
    }


/*-----------------------------------------------------------------*//**

 Transform the a,b,c,d components for an implicit plane.
 The plane equation format is ax+by+cz=d.

 @instance pTransform => The transformation to apply
 @param paOut <= x coefficient in transformed plane equation
 @param pbOut <= y coefficient in transformed plane equation
 @param pcOut <= z coefficient in transformed plane equation
 @param pdOut <= transformed right hand side constant
 @param a => The x coefficient in plane equation
 @param b => The y coefficient in plane equation
 @param c => The z coefficient in plane equation
 @param d => The constant on right hand side of plane equation
 @return value of plane equation.
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_transformImplicitPlane
(
TransformCP pTransform,
double        *paOut,
double        *pbOut,
double        *pcOut,
double        *pdOut,
double        a,
double        b,
double        c,
double        d
)
    {
    Transform inverse;

    if (bsiTransform_invertTransform (&inverse, pTransform))
        {
        *paOut =      a * inverse.form3d[0][0]
                    + b * inverse.form3d[1][0]
                    + c * inverse.form3d[2][0];

        *pbOut =      a * inverse.form3d[0][1]
                    + b * inverse.form3d[1][1]
                    + c * inverse.form3d[2][1];

        *pcOut =      a * inverse.form3d[0][2]
                    + b * inverse.form3d[1][2]
                    + c * inverse.form3d[2][2];

        *pdOut = -(   a * inverse.form3d[0][3]
                    + b * inverse.form3d[1][3]
                    + c * inverse.form3d[2][3]
                    - d);
        return true;
        }
    return false;
    }

/* CSVFUNC(multiplyMatrixPart) */

/*-----------------------------------------------------------------*//**
 Multiplies the matrix part of this instance times the column vector
 constructed from components x,y,z.
 Symbolically, given transform [R t] and column vector p,
 the returned point is R*p.

 @instance pTransform => The transformation to apply
 @param pPoint <= result of matrix * point operation
 @param x => The x component of the point
 @param y => The y component of the point
 @param z => The z component of the point
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyComponentsByMatrixPart
(
TransformCP pTransform,
DPoint3dP pPoint,
double         x,
double         y,
double         z
)
    {
    pPoint->x =   pTransform->form3d[0][0] * x
                + pTransform->form3d[0][1] * y
                + pTransform->form3d[0][2] * z;

    pPoint->y =   pTransform->form3d[1][0] * x
                + pTransform->form3d[1][1] * y
                + pTransform->form3d[1][2] * z;

    pPoint->z =   pTransform->form3d[2][0] * x
                + pTransform->form3d[2][1] * y
                + pTransform->form3d[2][2] * z;
    }
/* CSVFUNC(multiplyMatrixPart) */

/*-----------------------------------------------------------------*//**
 Multiplies the matrix part of this instance times column vector pInPoint.
 Symbolically, given transform [R t] and column vector p,
 the returned point is R*p.

 @instance pTransform => The transformation to apply
 @param pOutPoint <= result of matrix * point operation
 @param pInPoint => The point by which matrix is multiplied
 @group "Transform Multiplication"
 @bsimethod                                                       DavidAssaf      7/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyDPoint3dByMatrixPart
(
TransformCP pTransform,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint
)
    {
    DPoint3d inPoint;
    inPoint = *pInPoint;
    pOutPoint->x =  pTransform->form3d[0][0] * inPoint.x
                  + pTransform->form3d[0][1] * inPoint.y
                  + pTransform->form3d[0][2] * inPoint.z;

    pOutPoint->y =  pTransform->form3d[1][0] * inPoint.x
                  + pTransform->form3d[1][1] * inPoint.y
                  + pTransform->form3d[1][2] * inPoint.z;

    pOutPoint->z =  pTransform->form3d[2][0] * inPoint.x
                  + pTransform->form3d[2][1] * inPoint.y
                  + pTransform->form3d[2][2] * inPoint.z;
    }

/*-----------------------------------------------------------------*//**
 Multiplies the matrix part of this instance times column vector pPoint and
 replaces pPoint with the result.
 Symbolically, given transform [R t] and column vector p,
 the returned point is R*p.

 @instance pTransform => The transformation to apply
 @param pPoint <=> point to be updated
 @group "Transform Multiplication"
@bsimethod                                                       DavidAssaf      7/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyDPoint3dByMatrixPartInPlace
(
TransformCP pTransform,
DPoint3dP pPoint
)
    {
    DPoint3d    inPoint;

    inPoint = *pPoint;

    pPoint->x =  pTransform->form3d[0][0] * inPoint.x
               + pTransform->form3d[0][1] * inPoint.y
               + pTransform->form3d[0][2] * inPoint.z;

    pPoint->y =  pTransform->form3d[1][0] * inPoint.x
               + pTransform->form3d[1][1] * inPoint.y
               + pTransform->form3d[1][2] * inPoint.z;

    pPoint->z =  pTransform->form3d[2][0] * inPoint.x
               + pTransform->form3d[2][1] * inPoint.y
               + pTransform->form3d[2][2] * inPoint.z;
    }
/* CSVFUNC(multiplyMatrixPartTranspose) */

/*-----------------------------------------------------------------*//**
 Multiplies the row vector constructed from components x,y,z times the matrix
 part of this instance.
 Symbolically, given transform [R t] and row vector p,
 the returned point is p*R.

 @instance pTransform => The transformation to apply
 @param pPoint <= result of point * matrix operation
 @param x => The x component of the point
 @param y => The y component of the point
 @param z => The z component of the point
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyTransposeComponentsByMatrixPart
(
TransformCP pTransform,
DPoint3dP pPoint,
double         x,
double         y,
double         z
)
    {
    pPoint->x =   pTransform->form3d[0][0] * x
                + pTransform->form3d[1][0] * y
                + pTransform->form3d[2][0] * z;

    pPoint->y =   pTransform->form3d[0][1] * x
                + pTransform->form3d[1][1] * y
                + pTransform->form3d[2][1] * z;

    pPoint->z =   pTransform->form3d[0][2] * x
                + pTransform->form3d[1][2] * y
                + pTransform->form3d[2][2] * z;
    }
/* CSVFUNC(multiplyMatrixPartTranspose) */

/*-----------------------------------------------------------------*//**
 Multiplies the row vector pInPoint times the matrix
 part of this instance.
 Symbolically, given transform [R t] and row vector p,
 the returned point is p*R.

 @instance pTransform => The transformation to apply
 @param pOutPoint <= result of point * matrix operation
 @param pInPoint => The point which multiplies matrix
 @group "Transform Multiplication"
 @bsimethod                                                       DavidAssaf      7/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyTransposePointByMatrixPart
(
TransformCP pTransform,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint
)
    {
    DPoint3d inPoint;
    inPoint = *pInPoint;
    pOutPoint->x =  pTransform->form3d[0][0] * inPoint.x
                  + pTransform->form3d[1][0] * inPoint.y
                  + pTransform->form3d[2][0] * inPoint.z;

    pOutPoint->y =  pTransform->form3d[0][1] * inPoint.x
                  + pTransform->form3d[1][1] * inPoint.y
                  + pTransform->form3d[2][1] * inPoint.z;

    pOutPoint->z =  pTransform->form3d[0][2] * inPoint.x
                  + pTransform->form3d[1][2] * inPoint.y
                  + pTransform->form3d[2][2] * inPoint.z;
    }


/*-----------------------------------------------------------------*//**
 Multiplies the row vector pPoint times the matrix
 part of this instance, and replaces pPoint with the result.
 Symbolically, given transform [R t] and row vector p,
 the returned point is p*R.

 @instance pTransform => The transformation to apply
 @param pPoint <=> point to be updated
 @group "Transform Multiplication"
 @bsimethod                                                       DavidAssaf      7/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyTransposePointByMatrixPartInPlace
(
TransformCP pTransform,
DPoint3dP pPoint
)
    {
    DPoint3d    inPoint;

    inPoint = *pPoint;

    pPoint->x =  pTransform->form3d[0][0] * inPoint.x
               + pTransform->form3d[1][0] * inPoint.y
               + pTransform->form3d[2][0] * inPoint.z;

    pPoint->y =  pTransform->form3d[0][1] * inPoint.x
               + pTransform->form3d[1][1] * inPoint.y
               + pTransform->form3d[2][1] * inPoint.z;

    pPoint->z =  pTransform->form3d[0][2] * inPoint.x
               + pTransform->form3d[1][2] * inPoint.y
               + pTransform->form3d[2][2] * inPoint.z;
    }


/*-----------------------------------------------------------------*//**
 Multiplies this instance times each column vector in pPointArray
 and replaces pPointArray with the resulting points.
 Symbolically, given transform [R t],
 each returned point is of the form R*p + t, where p is a column vector.

 @instance pTransform => The transformation to apply
 @param pPointArray <=> array of points to be multiplied
 @param numPoint => The number of points
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyDPoint3dArrayInPlace
(
TransformCP pTransform,
DPoint3dP pPointArray,
int            numPoint
)
    {
    int     i;
    double x,y,z;
    DPoint3d *pPoint;

    for (i = 0, pPoint = pPointArray;
         i < numPoint;
         i++, pPoint++
        )
        {
        x = pPoint->x;
        y = pPoint->y;
        z = pPoint->z;

        pPoint->x =   pTransform->form3d[0][3]
                + pTransform->form3d[0][0] * x
                + pTransform->form3d[0][1] * y
                + pTransform->form3d[0][2] * z;

        pPoint->y =   pTransform->form3d[1][3]
                + pTransform->form3d[1][0] * x
                + pTransform->form3d[1][1] * y
                + pTransform->form3d[1][2] * z;

        pPoint->z =   pTransform->form3d[2][3]
                + pTransform->form3d[2][0] * x
                + pTransform->form3d[2][1] * y
                + pTransform->form3d[2][2] * z;
        }
    }


/*-----------------------------------------------------------------*//**
 Multiplies this instance times each column vector in pPointArray,
 using the transpose of the matrix part of this instance in the multiplications,
 and replaces pPointArray with the resulting points.
 Symbolically, given transform [R t], each returned point has the equivalent
 form p*R + t, where p is a row vector.

 @instance pTransform => The transformation to apply
 @param pPointArray <=> array of points to be multiplied
 @param numPoint => The number of points
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyTransposeDPoint3dArrayInPlace
(
TransformCP pTransform,
DPoint3dP pPointArray,
int            numPoint
)
    {
    int     i;
    double x,y,z;
    DPoint3d *pPoint;

    for (i = 0, pPoint = pPointArray;
         i < numPoint;
         i++, pPoint++
        )
        {
        x = pPoint->x;
        y = pPoint->y;
        z = pPoint->z;

        pPoint->x =   pTransform->form3d[0][3]
                + pTransform->form3d[0][0] * x
                + pTransform->form3d[1][0] * y
                + pTransform->form3d[2][0] * z;

        pPoint->y =   pTransform->form3d[1][3]
                + pTransform->form3d[0][1] * x
                + pTransform->form3d[1][1] * y
                + pTransform->form3d[2][1] * z;

        pPoint->z =   pTransform->form3d[2][3]
                + pTransform->form3d[0][2] * x
                + pTransform->form3d[1][2] * y
                + pTransform->form3d[2][2] * z;
        }
    }


/*-----------------------------------------------------------------*//**
 Multiplies this instance times each column vector in pInPoint and places the
 resulting points in pOutPoint.
 pInPoint and pOutPoint may be the same.
 Symbolically, given transform [R t], each returned point has the
 form R*p + t*w (with weight w), where p is a column vector and w is its
 weight.

 @instance pTransform => The transformation to apply
 @param pOutPoint <= transformed points
 @param pInPoint => The input points
 @param numPoint => The number of points
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyDPoint4dArray
(
TransformCP pTransform,
DPoint4dP pOutPoint,
DPoint4dCP pInPoint,
int            numPoint
)
    {
    int     i;
    double x,y,z,w;
    DPoint4d *pPoint;

    for (i = 0, pPoint = pOutPoint;
         i < numPoint;
         i++, pPoint++
        )
        {
        x = pInPoint[i].x;
        y = pInPoint[i].y;
        z = pInPoint[i].z;
        w = pInPoint[i].w;
        if (w == 1.0)
            {
            pPoint->x =   pTransform->form3d[0][3]
                    + pTransform->form3d[0][0] * x
                    + pTransform->form3d[0][1] * y
                    + pTransform->form3d[0][2] * z;

            pPoint->y =   pTransform->form3d[1][3]
                    + pTransform->form3d[1][0] * x
                    + pTransform->form3d[1][1] * y
                    + pTransform->form3d[1][2] * z;

            pPoint->z =   pTransform->form3d[2][3]
                    + pTransform->form3d[2][0] * x
                    + pTransform->form3d[2][1] * y
                    + pTransform->form3d[2][2] * z;
            pPoint->w = 1.0;
            }
        else
            {
            pPoint->x =   pTransform->form3d[0][3] * w
                    + pTransform->form3d[0][0] * x
                    + pTransform->form3d[0][1] * y
                    + pTransform->form3d[0][2] * z;

            pPoint->y =   pTransform->form3d[1][3] * w
                    + pTransform->form3d[1][0] * x
                    + pTransform->form3d[1][1] * y
                    + pTransform->form3d[1][2] * z;

            pPoint->z =   pTransform->form3d[2][3] * w
                    + pTransform->form3d[2][0] * x
                    + pTransform->form3d[2][1] * y
                    + pTransform->form3d[2][2] * z;
            pPoint->w = w;
            }
        }
    }

/*-----------------------------------------------------------------*//**
 Multiplies this instance times each column vector in pInPoint and places the
 resulting points in pOutPoint.
 pInPoint and pOutPoint may be the same.
 Symbolically, given transform [R t], each returned point has the
 form R*p + t, where p is a column vector.

 @instance pTransform => The transformation to apply
 @param pOutPoint <= transformed points
 @param pInPoint => The input points
 @param numPoint => The number of points
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyDPoint3dArray
(
TransformCP pTransform,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
int            numPoint
)
    {
    int     i;
    double x,y,z;
    DPoint3d *pPoint;

    for (i = 0, pPoint = pOutPoint;
         i < numPoint;
         i++, pPoint++
        )
        {
        x = pInPoint[i].x;
        y = pInPoint[i].y;
        z = pInPoint[i].z;

        pPoint->x =   pTransform->form3d[0][3]
                + pTransform->form3d[0][0] * x
                + pTransform->form3d[0][1] * y
                + pTransform->form3d[0][2] * z;

        pPoint->y =   pTransform->form3d[1][3]
                + pTransform->form3d[1][0] * x
                + pTransform->form3d[1][1] * y
                + pTransform->form3d[1][2] * z;

        pPoint->z =   pTransform->form3d[2][3]
                + pTransform->form3d[2][0] * x
                + pTransform->form3d[2][1] * y
                + pTransform->form3d[2][2] * z;

        }
    }


/*-----------------------------------------------------------------*//**
 Multiplies this instance times each column vector in pInPoint and places the
 resulting points in pOutPoint.
 pInPoint and pOutPoint may be the same.
 Symbolically, given transform [R t], each returned point has the
 form R*p + t, where p is a column vector.

 @instance pTransform => The transformation to apply
 @param pOutPoint <= transformed points
 @param pInPoint => The input points
 @param numPoint => The number of points
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyFPoint3dArray
(
TransformCP pTransform,
FPoint3dP pOutPoint,
FPoint3dCP pInPoint,
int            numPoint
)
    {
    int     i;
    double x,y,z;
    FPoint3d *pPoint;

    for (i = 0, pPoint = pOutPoint;
         i < numPoint;
         i++, pPoint++
        )
        {
        x = pInPoint[i].x;
        y = pInPoint[i].y;
        z = pInPoint[i].z;

        pPoint->x = (float)(pTransform->form3d[0][3]
                + pTransform->form3d[0][0] * x
                + pTransform->form3d[0][1] * y
                + pTransform->form3d[0][2] * z);

        pPoint->y = (float)(pTransform->form3d[1][3]
                + pTransform->form3d[1][0] * x
                + pTransform->form3d[1][1] * y
                + pTransform->form3d[1][2] * z);

        pPoint->z = (float)(pTransform->form3d[2][3]
                + pTransform->form3d[2][0] * x
                + pTransform->form3d[2][1] * y
                + pTransform->form3d[2][2] * z);
        }
    }


/*-----------------------------------------------------------------*//**
 Multiplies this instance times each column vector in pInPoint,
 using the transpose of the matrix part of this instance in the multiplications,
 and places the resulting points in pOutPoint.
 Symbolically, given transform [R t], each returned point has the equivalent
 form p*R + t, where p is a row vector.
 pInPoint and pOutPoint may be the same.

 @instance pTransform => The transformation to apply
 @param pOutPoint <= transformed points
 @param pInPoint => The input points
 @param numPoint => The number of points
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyTransposeDPoint3dArray
(
TransformCP pTransform,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
int            numPoint
)
    {
    int     i;
    double x,y,z;
    DPoint3d *pPoint;

    for (i = 0, pPoint = pOutPoint;
         i < numPoint;
         i++, pPoint++
        )
        {
        x = pInPoint[i].x;
        y = pInPoint[i].y;
        z = pInPoint[i].z;

        pPoint->x =   pTransform->form3d[0][3]
                + pTransform->form3d[0][0] * x
                + pTransform->form3d[1][0] * y
                + pTransform->form3d[2][0] * z;

        pPoint->y =   pTransform->form3d[1][3]
                + pTransform->form3d[0][1] * x
                + pTransform->form3d[1][1] * y
                + pTransform->form3d[2][1] * z;

        pPoint->z =   pTransform->form3d[2][3]
                + pTransform->form3d[0][2] * x
                + pTransform->form3d[1][2] * y
                + pTransform->form3d[2][2] * z;

        }
    }


/*-----------------------------------------------------------------*//**
 Multiplies this instance times each column vector in pInPoint and places the
 resulting points in pOutPoint.
 pInPoint and pOutPoint may be the same.
 All z parts (i.e. last row and column) of this instance are ignored.

 @instance pTransform => The transformation to apply
 @param pOutPoint <= transformed points
 @param pInPoint => The input points
 @param numPoint => The number of points
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyDPoint2dArray
(
TransformCP pTransform,
DPoint2dP pOutPoint,
DPoint2dCP pInPoint,
int            numPoint
)
    {
    int     i;
    double x,y;
    DPoint2d *pPoint;

    for (i = 0, pPoint = pOutPoint;
         i < numPoint;
         i++, pPoint++
        )
        {
        x = pInPoint[i].x;
        y = pInPoint[i].y;

        pPoint->x =   pTransform->form3d[0][3]
                + pTransform->form3d[0][0] * x
                + pTransform->form3d[0][1] * y;

        pPoint->y =   pTransform->form3d[1][3]
                + pTransform->form3d[1][0] * x
                + pTransform->form3d[1][1] * y;
        }
    }


/*-----------------------------------------------------------------*//**
 Multiplies this instance times each column vector in pInPoint and places the
 resulting points in pOutPoint.
 pInPoint and pOutPoint may be the same.
 All z parts (i.e. last row and column) of this instance are ignored.

 @instance pTransform => The transformation to apply
 @param pOutPoint <= transformed points
 @param pInPoint => The input points
 @param numPoint => The number of points
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyFPoint2dArray
(
TransformCP pTransform,
FPoint2dP pOutPoint,
FPoint2dCP pInPoint,
int            numPoint
)
    {
    int     i;
    double x,y;
    FPoint2d *pPoint;

    for (i = 0, pPoint = pOutPoint;
         i < numPoint;
         i++, pPoint++
        )
        {
        x = pInPoint[i].x;
        y = pInPoint[i].y;

        pPoint->x = (float)(pTransform->form3d[0][3]
                + pTransform->form3d[0][0] * x
                + pTransform->form3d[0][1] * y);

        pPoint->y = (float)(pTransform->form3d[1][3]
                + pTransform->form3d[1][0] * x
                + pTransform->form3d[1][1] * y);
        }
    }



/*-----------------------------------------------------------------*//**
 Multiplies this instance times each column vector in pInPoint and places the
 resulting points in pOutPoint.
 Each input point is given a z=0.

 @param pTransform => The transformation to apply
 @param pOutPoint <= transformed points
 @param pInPoint => The input points
 @param numPoint => The number of points
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyDPoint2dArrayTo3d
(
TransformCP pTransform,
DPoint3dP pOutPoint,
DPoint2dCP pInPoint,
int            numPoint
)
    {
    int     i;
    double x,y;
    DPoint3d *pPoint;

    for (i = 0, pPoint = pOutPoint;
         i < numPoint;
         i++, pPoint++
        )
        {
        x = pInPoint[i].x;
        y = pInPoint[i].y;

        pPoint->x =   pTransform->form3d[0][3]
                + pTransform->form3d[0][0] * x
                + pTransform->form3d[0][1] * y;

        pPoint->y =   pTransform->form3d[1][3]
                + pTransform->form3d[1][0] * x
                + pTransform->form3d[1][1] * y;

        pPoint->z =   pTransform->form3d[2][3]
                + pTransform->form3d[2][0] * x
                + pTransform->form3d[2][1] * y;

        }
    }


/*-----------------------------------------------------------------*//**
 Multiplies this instance times each column vector in pInPoint and places the
 x and y components of the resulting points in pOutPoint.

 @instance pTransform => The transformation to apply
 @param pOutPoint <= transformed points
 @param pInPoint => The input points
 @param numPoint => The number of points
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyDPoint3dArrayTo2d
(
TransformCP pTransform,
DPoint2dP pOutPoint,
DPoint3dCP pInPoint,
int            numPoint
)
    {
    int     i;
    double x,y,z;
    DPoint2d *pPoint;

    for (i = 0, pPoint = pOutPoint;
         i < numPoint;
         i++, pPoint++
        )
        {
        x = pInPoint[i].x;
        y = pInPoint[i].y;
        z = pInPoint[i].z;

        pPoint->x =   pTransform->form3d[0][3]
                + pTransform->form3d[0][0] * x
                + pTransform->form3d[0][1] * y
                + pTransform->form3d[0][2] * z;

        pPoint->y =   pTransform->form3d[1][3]
                + pTransform->form3d[1][0] * x
                + pTransform->form3d[1][1] * y
                + pTransform->form3d[1][2] * z;

        }
    }
/* VBSUB(Transform3dFromTransform3dTimesTransform3d) */
/* CSVFUNC(Multiply) */

/*-----------------------------------------------------------------*//**
 @description Returns the product of two transformations.
 Symbolically, given transforms [R t] and [S u], return the product transform
 [R t][S u] = [R*S t+R*u].

 @instance pProduct <= product transformation
 @param pTransform1 => The first factor
 @param pTransform2 => The second factor
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyTransformTransform
(
TransformP pProduct,
TransformCP pTransform1,
TransformCP pTransform2
)
    {
    Transform AB;
    if (pTransform1 && pTransform2)
        {
        int j;
        AB.form3d[0][3] = pTransform1->form3d[0][3]
                         + pTransform1->form3d[0][0] * pTransform2->form3d[0][3]
                         + pTransform1->form3d[0][1] * pTransform2->form3d[1][3]
                         + pTransform1->form3d[0][2] * pTransform2->form3d[2][3];

        AB.form3d[1][3] = pTransform1->form3d[1][3]
                         + pTransform1->form3d[1][0] * pTransform2->form3d[0][3]
                         + pTransform1->form3d[1][1] * pTransform2->form3d[1][3]
                         + pTransform1->form3d[1][2] * pTransform2->form3d[2][3];

        AB.form3d[2][3] = pTransform1->form3d[2][3]
                         + pTransform1->form3d[2][0] * pTransform2->form3d[0][3]
                         + pTransform1->form3d[2][1] * pTransform2->form3d[1][3]
                         + pTransform1->form3d[2][2] * pTransform2->form3d[2][3];

        for (j = 0; j < 3; j++)
            {

            AB.form3d[0][j]
                           = pTransform1->form3d[0][0] * pTransform2->form3d[0][j]
                           + pTransform1->form3d[0][1] * pTransform2->form3d[1][j]
                           + pTransform1->form3d[0][2] * pTransform2->form3d[2][j];

            AB.form3d[1][j]
                           = pTransform1->form3d[1][0] * pTransform2->form3d[0][j]
                           + pTransform1->form3d[1][1] * pTransform2->form3d[1][j]
                           + pTransform1->form3d[1][2] * pTransform2->form3d[2][j];

            AB.form3d[2][j]
                           = pTransform1->form3d[2][0] * pTransform2->form3d[0][j]
                           + pTransform1->form3d[2][1] * pTransform2->form3d[1][j]
                           + pTransform1->form3d[2][2] * pTransform2->form3d[2][j];
            }
        *pProduct = AB;

        }
    else if (pTransform1)
        {
        *pProduct = *pTransform1;
        }
    else if (pTransform2)
        {
        *pProduct = *pTransform2;
        }
    else
        {
        bsiTransform_initIdentity (pProduct);
        }
    }

/* VBSUB(Transform3dFromTransform3dTimesTransform3dTimesTransform3d) */
/* CSVFUNC(Multiply) */

/*-----------------------------------------------------------------*//**
 @description Returns the product of three transformations.
 Symbolically, given transforms [R t], [S u] and [T v], return the product transform
 [R t][S u][T v] = [R*S*T t+R*(u+S*v)].

 @instance pProduct <= product transformation
 @param pTransform1 => The first factor (R)
 @param pTransform2 => The second factor (S)
 @param pTransform3 => The third factor (T)
 @group "Transform Multiplication"
 @bsimethod                                       DavidAssaf      03/04
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyTransformTransformTransform
(
TransformP pProduct,
TransformCP pTransform1,
TransformCP pTransform2,
TransformCP pTransform3
)
    {
    Transform AB;
    bsiTransform_multiplyTransformTransform (&AB, pTransform1, pTransform2);
    bsiTransform_multiplyTransformTransform (pProduct, &AB, pTransform3);
    }

/* VBSUB(Transform3dFromMatrix3dTimesTransform3d) */
/* CSVFUNC(Multiply) */

/*-----------------------------------------------------------------*//**
 @description Returns the product of a matrix and a transformation.
 The matrix acts like a transformation with a zero point as its translation part.

 @instance pProduct <= product transformation
 @param pMatrix => The first factor (matrix)
 @param pTransform => The second factor (transform)
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyRotMatrixTransform
(
TransformP pProduct,
RotMatrixCP pMatrix,
TransformCP pTransform
)
    {
    Transform AB;
    if (pMatrix && pTransform)
        {
        int j;
        AB.form3d[0][3] = pMatrix->form3d[0][0] * pTransform->form3d[0][3]
                         + pMatrix->form3d[0][1] * pTransform->form3d[1][3]
                         + pMatrix->form3d[0][2] * pTransform->form3d[2][3];

        AB.form3d[1][3] = pMatrix->form3d[1][0] * pTransform->form3d[0][3]
                         + pMatrix->form3d[1][1] * pTransform->form3d[1][3]
                         + pMatrix->form3d[1][2] * pTransform->form3d[2][3];

        AB.form3d[2][3] = pMatrix->form3d[2][0] * pTransform->form3d[0][3]
                         + pMatrix->form3d[2][1] * pTransform->form3d[1][3]
                         + pMatrix->form3d[2][2] * pTransform->form3d[2][3];

        for (j = 0; j < 3; j++)
            {

            AB.form3d[0][j]
                           = pMatrix->form3d[0][0] * pTransform->form3d[0][j]
                           + pMatrix->form3d[0][1] * pTransform->form3d[1][j]
                           + pMatrix->form3d[0][2] * pTransform->form3d[2][j];

            AB.form3d[1][j]
                           = pMatrix->form3d[1][0] * pTransform->form3d[0][j]
                           + pMatrix->form3d[1][1] * pTransform->form3d[1][j]
                           + pMatrix->form3d[1][2] * pTransform->form3d[2][j];

            AB.form3d[2][j]
                           = pMatrix->form3d[2][0] * pTransform->form3d[0][j]
                           + pMatrix->form3d[2][1] * pTransform->form3d[1][j]
                           + pMatrix->form3d[2][2] * pTransform->form3d[2][j];
            }
        *pProduct = AB;

        }
    else if (pMatrix)
        {
        bsiTransform_initFromMatrix (pProduct, pMatrix);
        }
    else if (pTransform)
        {
        *pProduct = *pTransform;
        }
    else
        {
        bsiTransform_initIdentity (pProduct);
        }
    }
/* VBSUB(Transform3dFromTransform3dTimesMatrix3d) */
/* CSVFUNC(Multiply) */

/*-----------------------------------------------------------------*//**
 @description returns the product of a transformation times a matrix, treating
   the matrix as a transformation with zero as its translation components.
 @instance pProduct <= product transformation
 @param pTransform => The first factor (transform)
 @param pMatrix => The second factor (matrix)
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyTransformRotMatrix
(
TransformP pProduct,
TransformCP pTransform,
RotMatrixCP pMatrix
)
    {
    Transform AB;
    if (pTransform && pMatrix)
        {
        int j;
        AB.form3d[0][3] = pTransform->form3d[0][3];
        AB.form3d[1][3] = pTransform->form3d[1][3];
        AB.form3d[2][3] = pTransform->form3d[2][3];

        for (j = 0; j < 3; j++)
            {

            AB.form3d[0][j]
                           = pTransform->form3d[0][0] * pMatrix->form3d[0][j]
                           + pTransform->form3d[0][1] * pMatrix->form3d[1][j]
                           + pTransform->form3d[0][2] * pMatrix->form3d[2][j];

            AB.form3d[1][j]
                           = pTransform->form3d[1][0] * pMatrix->form3d[0][j]
                           + pTransform->form3d[1][1] * pMatrix->form3d[1][j]
                           + pTransform->form3d[1][2] * pMatrix->form3d[2][j];

            AB.form3d[2][j]
                           = pTransform->form3d[2][0] * pMatrix->form3d[0][j]
                           + pTransform->form3d[2][1] * pMatrix->form3d[1][j]
                           + pTransform->form3d[2][2] * pMatrix->form3d[2][j];
            }
        *pProduct = AB;

        }
    else if (pTransform)
        {
        *pProduct = *pTransform;
        }
    else if (pMatrix)
        {
        bsiTransform_initFromMatrix (pProduct, pMatrix);
        }
    else
        {
        bsiTransform_initIdentity (pProduct);
        }
    }


/*-----------------------------------------------------------------*//**
 Computes the 8 corner points of the range cube defined by the two points
 of pRange, multiplies pTransform times each point, and computes the
 twopoint range limits (min max) of the resulting rotated box.
 Note that the volume of the rotated box is generally larger than
 that of the input because the postrotation minmax operation expands
 the box.

 @instance pTransform => The transform to apply
 @param pOutRange <= range of transformed cube
 @param pInRange => The any two corners of original cube
 @group "Transform Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyRange
(
TransformCP pTransform,
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
        bsiDRange3d_box2Points (pInRange, corner);

        bsiTransform_multiplyDPoint3dArrayInPlace (pTransform, corner, 8);
        bsiDRange3d_initFromArray (pOutRange, corner, 8);
        }
    }
/* VBSUB(Transform3dIdentity) */

/*-----------------------------------------------------------------*//**
 @description returns an identity transformation, i.e. zero
 translation part and identity matrix part.

 @instance pTransform <= identity transformation
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initIdentity
(
TransformP pTransform
)
    {
    memset (pTransform, 0, sizeof (Transform));
    pTransform->form3d[0][0] =
    pTransform->form3d[1][1] =
    pTransform->form3d[2][2] = 1.0;
    }

/* VBSUB(Transform3dFromMatrix3d) */
/* CSVFUNC(FromRotMatrix) */

/*-----------------------------------------------------------------*//**
 @description Returns a transformation with the given matrix part and a zero
           translation part.
 @instance pTransform <= constructed transformation
 @param pMatrix => The matrix part
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFromMatrix
(
TransformP pTransform,
RotMatrixCP pMatrix
)
    {
    int j;
    for (j = 0; j < 3; j++      )
        {
        pTransform->form3d[0][j] = pMatrix->form3d[0][j];
        pTransform->form3d[1][j] = pMatrix->form3d[1][j];
        pTransform->form3d[2][j] = pMatrix->form3d[2][j];
        }

    pTransform->form3d[0][3] =
    pTransform->form3d[1][3] =
    pTransform->form3d[2][3] = 0.0;
    }
/* VBSUB(Transform3dFromMatrix3dPoint3d) */
/* CSVFUNC(FromRotMatrixDPoint3d) */

/*-----------------------------------------------------------------*//**
 @description Returns a transformation with the given matrix and translation parts.
 @instance pTransform <= constructed transformation
 @param pMatrix => The matrix part
 @param pTranslation => The translation part
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFromMatrixAndTranslation
(
TransformP pTransform,
RotMatrixCP pMatrix,
DPoint3dCP pTranslation
)
    {
    if (pMatrix)
        {
        bsiTransform_initFromMatrix (pTransform, pMatrix);
        }
    else
        {
        bsiTransform_initIdentity (pTransform);
        }

    if (pTranslation)
        {
        pTransform->form3d[0][3] = pTranslation->x;
        pTransform->form3d[1][3] = pTranslation->y;
        pTransform->form3d[2][3] = pTranslation->z;
        }
    else
        {
        pTransform->form3d[0][3] =
        pTransform->form3d[1][3] =
        pTransform->form3d[2][3] = 0.0;
        }
    }
/* VBSUB(Transform3dFromPoint3d) */
/* CSVFUNC(FromDPoint3d) */

/*-----------------------------------------------------------------*//**
 @description Returns a transformation with identity matrix part and
           given translation part.
 @instance pTransform <= transformation with identity matrix, given translation
 @param pTranslation => The translation part
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFromTranslation
(
TransformP pTransform,
DPoint3dCP pTranslation
)
    {
    bsiTransform_initIdentity (pTransform);

    if (pTranslation)
        {
        pTransform->form3d[0][3] = pTranslation->x;
        pTransform->form3d[1][3] = pTranslation->y;
        pTransform->form3d[2][3] = pTranslation->z;
        }
    }
/* VBSUB(Transform3dFromXYZ) */
/* CSVFUNC(FromXYZ) */

/*-----------------------------------------------------------------*//**
 @description Returns a transformation with identity matrix part and
       translation part given as x, y, and z components.
 @instance pTransform <= transformation with identity matrix, given translation
 @param x => The x-coordinate of translation part
 @param y => The y-coordinate of translation part
 @param z => The z-coordinate of translation part
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFromTranslationXYZ
(
TransformP pTransform,
double        x,
double        y,
double        z
)
    {
    bsiTransform_initIdentity (pTransform);
    pTransform->form3d[0][3] = x;
    pTransform->form3d[1][3] = y;
    pTransform->form3d[2][3] = z;
    }
/* VBSUB(Transform3dFromRowValues) */
/* CSVFUNC(FromRowValues) */

/*-----------------------------------------------------------------*//**
 Copies the double values directly into the rows of this instance.

 @instance pTransform <= constructed transform
 @param x00 => The (0,0) entry of the matrix (row, column)
 @param x01 => The (0,1) entry
 @param x02 => The (0,2) entry
 @param tx  => The x-coordinate of the translation part
 @param x10 => The (1,0) entry
 @param x11 => The (1,1) entry
 @param x12 => The (1,2) entry
 @param ty  => The y-coordinate of the translation part
 @param x20 => The (2,0) entry
 @param x21 => The (2,1) entry
 @param x22 => The (2,2) entry
 @param tz  => The z-coordinate of the translation part
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFromRowValues
(
TransformP pTransform,
double        x00,
double        x01,
double        x02,
double        tx,
double        x10,
double        x11,
double        x12,
double        ty,
double        x20,
double        x21,
double        x22,
double        tz
)
    {
    pTransform->form3d[0][0] = x00;
    pTransform->form3d[1][0] = x10;
    pTransform->form3d[2][0] = x20;
    pTransform->form3d[0][3] = tx;

    pTransform->form3d[0][1] = x01;
    pTransform->form3d[1][1] = x11;
    pTransform->form3d[2][1] = x21;
    pTransform->form3d[1][3] = ty;

    pTransform->form3d[0][2] = x02;
    pTransform->form3d[1][2] = x12;
    pTransform->form3d[2][2] = x22;
    pTransform->form3d[2][3] = tz;
    }

/* VBSUB(Transform3dFromLineAndRotationAngle) */
/* CSVFUNC(FromLineAndRotationAngle) */

/*-----------------------------------------------------------------*//**
 @description Returns a transformation of rotation about a specified line.
 @instance pTransform <= rotation transformation
 @param pPoint0 => The start point of the line
 @param pPoint1 => The end point of the line
 @param radians => The rotation angle
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFromLineAndRotationAngle
(
TransformP pTransform,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1,
double         radians
)
    {
    DVec3d vector;
    RotMatrix matrix;
    bsiDVec3d_subtractDPoint3dDPoint3d (&vector, pPoint1, pPoint0);
    bsiRotMatrix_initFromVectorAndRotationAngle (&matrix,
                                &vector, radians);
    /* We count on the vector rotation returning a valid identity matrix even if
        the vector is null */
    bsiTransform_initFromMatrixAndFixedPoint (pTransform, &matrix, pPoint0);
    }


/*-----------------------------------------------------------------*//**
 Sets this instance to a transformation that has the same matrix part
 as transform pIn and a translation part that is the SUM of the
 translation part of pIn plus the product of the
 matrix part of pIn times the given point.
 If the translation part of pIn is interpreted as the
 origin of a coordinate system (whose axis directions and sizes
 are given by the columns of the matrix part), this instance
 becomes a coordinate frame with the same axis directions and sizes,
 but with the origin shifted to point x,y,z of the pIn
 system.  That is, x,y,z are the local coordinates of the new
 origin, and the translation part of this instance becomes the global
 coordinates of the new origin.
 pIn may be identical to this instance.
 Symbolically, if pIn is the transform [R t] and the local
 origin coordinates x,y,z are in column vector p, the result is
 the transformation [R t+R*p].

 @instance pOut <= output transformation
 @param pIn => The input transformation
 @param x => The x-coordinate of the local origin
 @param y => The y-coordinate of the local origin
 @param z => The z-coordinate of the local origin
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_translateInLocalCoordinates
(
TransformP pOut,
TransformCP pIn,
double         x,
double         y,
double         z
)
    {
    DPoint3d point;

    if (pIn)
        {
        point.x = x;
        point.y = y;
        point.z = z;
        *pOut = *pIn;
        bsiTransform_multiplyDPoint3dInPlace (pIn, &point);
        bsiTransform_setTranslation (pOut, &point);
        }
    else
        {
        bsiTransform_initFromTranslationXYZ (pOut, x, y, z);
        }
    }




/*-----------------------------------------------------------------*//**
 @description Overwrites the matrix part of a preexisting transformation.
   The translation part is unchanged.
 @instance pTransform <=> transformation whose matrix part is modified
 @param pMatrix => The matrix to insert
 @group "Transform In Place Modification"
 @bsimethod                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_setMatrix
(
TransformP pTransform,
RotMatrixCP pMatrix
)
    {
    int j;
    for (j = 0; j < 3; j++)
        {
        pTransform->form3d[0][j] = pMatrix->form3d[0][j];
        pTransform->form3d[1][j] = pMatrix->form3d[1][j];
        pTransform->form3d[2][j] = pMatrix->form3d[2][j];
        }
    }




/*-----------------------------------------------------------------*//**
 Sets the translation part of this instance to pPoint.  The prior
 translation part is overwritten, and the matrix part is unchanged.
 Symbolically, if pPoint is u then this instance [R t] becomes the
 transformation [R u].

 @instance pTransform <=> transformation whose vector part is modified
 @param pPoint => The vector to insert
 @group "Transform In Place Modification"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_setTranslation
(
TransformP pTransform,
DPoint3dCP pPoint
)
    {
    pTransform->form3d[0][3] = pPoint->x;
    pTransform->form3d[1][3] = pPoint->y;
    pTransform->form3d[2][3] = pPoint->z;
    }



/*-----------------------------------------------------------------*//**
 Sets the translation part of this instance to zero.  The prior
 translation part is overwritten, and the matrix part is unchanged.
 Symbolically, this instance [R t] becomes the transformation [R 0].

 @instance pTransform <=> transformation whose vector is zeroed
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_zeroTranslation
(
TransformP pTransform
)
    {
    pTransform->form3d[0][3] =
    pTransform->form3d[1][3] =
    pTransform->form3d[2][3] = 0.0;
    }



/*-----------------------------------------------------------------*//**
 Sets the translation part of this instance so that it leaves point
 pPoint unchanged, i.e. so that this instance may be interpreted as
 applying its matrix part as a rotation or scaling about pPoint.
 Symbolically, given transform [R t] and column vector p,
 the returned transform is [R p-R*p].
 (The prior translation part is destroyed, and does not affect the
 result in any way.)

 @instance pTransform <=> matrix whose vector part is modified
 @param pPoint => The point that is to remain fixed when multiplied by the modified transformation
 @group "Transform In Place Modification"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_setFixedPoint
(
TransformP pTransform,
DPoint3dCP pPoint
)
    {
    if (!pPoint)
        {
        bsiTransform_zeroTranslation (pTransform);
        return;
        }

    pTransform->form3d[0][3] = pPoint->x
                - pTransform->form3d[0][0] * pPoint->x
                - pTransform->form3d[0][1] * pPoint->y
                - pTransform->form3d[0][2] * pPoint->z;

    pTransform->form3d[1][3] = pPoint->y
                - pTransform->form3d[1][0] * pPoint->x
                - pTransform->form3d[1][1] * pPoint->y
                - pTransform->form3d[1][2] * pPoint->z;

    pTransform->form3d[2][3] = pPoint->z
                - pTransform->form3d[2][0] * pPoint->x
                - pTransform->form3d[2][1] * pPoint->y
                - pTransform->form3d[2][2] * pPoint->z;

    }
/* VBSUB(Transform3dFromMatrix3dAndFixedPoint3d) */
/* CSVFUNC(FromRotMatrixAndFixedDPoint3d) */

/*-----------------------------------------------------------------*//**
 @description Returns a transformation with given matrix part, and translation
   part computed from the matrix and a given fixed point.  This translation part
   is generally different from the fixed point itself.   The resulting transformation
   will leave the fixed point unchanged and apply whatever effects are contained in the
   matrix as if the fixed point is the origin.
 @instance pTransform <= transform to be initialized
 @param pMatrix => The matrix part
 @param pOrigin => The point that is to remain fixed when multiplied by the transformation.
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFromMatrixAndFixedPoint
(
TransformP pTransform,
RotMatrixCP pMatrix,
DPoint3dCP pOrigin
)
    {
    bsiTransform_initFromMatrix (pTransform, pMatrix);
    bsiTransform_setFixedPoint (pTransform, pOrigin);
    }


/*-----------------------------------------------------------------*//**

 Sets this instance to a matrix which is the inverse of pIn
 IN THE SPECIAL CASE WHERE pIn HAS ONLY PURE ROTATION OR
 MIRRORING IN ITS ROTATIONAL PART.   These special conditions allow
 the 'inversion' to be done by only a transposition and one
 matrix-times-point multiplication, rather than the full effort of
 inverting a general transformation. It is the caller's responsibility
 to be sure that these special conditions hold.  This usually occurs
 when the caller has just constructed the transform by a sequence of
 translations and rotations.
 If the caller has received the matrix from nonverified external
 sources and therefore does not know if the special conditions apply,
 the <CODE>inverseOf</CODE> method should be used instead.
 pIn may be the same as this instance.
 The specific computations in this special-case inversion are (1) the
 output transform's translation is the input transform's
 matrix times the negative of the input transform's translation, and (2) the
 output transform's matrix part is the tranpose of the input transform's
 matrix part.
 Symbolically, given transform [R t] return transform [R^ (R^)*(-t)]
 where ^ indicates transposition.

 @instance pOut <= inverted transformation
 @param    pIn  => The input transformation
 @group "Transform Inverse"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_invertAsRotation
(
TransformP pOut,
TransformCP pIn
)
    {
    DPoint3d point;
    Transform A;

    A = *pIn;

    pOut->form3d[0][0] = A.form3d[0][0];
    pOut->form3d[1][0] = A.form3d[0][1];
    pOut->form3d[2][0] = A.form3d[0][2];

    pOut->form3d[0][1] = A.form3d[1][0];
    pOut->form3d[1][1] = A.form3d[1][1];
    pOut->form3d[2][1] = A.form3d[1][2];

    pOut->form3d[0][2] = A.form3d[2][0];
    pOut->form3d[1][2] = A.form3d[2][1];
    pOut->form3d[2][2] = A.form3d[2][2];

    bsiTransform_multiplyComponentsByMatrixPart (pOut, &point,
                                            -A.form3d[0][3],
                                            -A.form3d[1][3],
                                            -A.form3d[2][3]
                                            );

    bsiTransform_setTranslation (pOut, &point);
    }
/* VBSUB(Transform3dInverse) */


/*-----------------------------------------------------------------*//**
 @description Compute the inverse transform of pIn.
 @remarks pIn may be the same as pOut.
 @remarks This is a modestly expensive floating point computation (33 multiplies, 14 adds).
 @remarks Symbolically, given transform [R t] return transform [Q Q*(-t)] where Q is the inverse of matrix R.
 @param pOut <= inverted transformation
 @param pIn => The input transformation
 @return true if transform is invertible
 @group "Transform Inverse"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_invertTransform
(
TransformP pOut,
TransformCP pIn
)
    {
    RotMatrix M;
    DPoint3d point;
    bool    boolStat;
    bsiTransform_getMatrix (pIn, &M);
    boolStat = bsiRotMatrix_invertRotMatrix(&M, &M);
    if  (boolStat)
        {
        bsiRotMatrix_multiplyComponents (&M, &point,
                    -pIn->form3d[0][3],
                    -pIn->form3d[1][3],
                    -pIn->form3d[2][3]
                    );
        bsiTransform_initFromMatrixAndTranslation (pOut, &M, &point);
        }
    else
        {
        bsiTransform_initIdentity (pOut);
        }
    return boolStat;
    }

/* VBSUB(Transform3dHasInverse) */

/*-----------------------------------------------------------------*//**
 @description Test if a transform has an inverse.
 @param pTransform => The transform to test.
 @return true if the matrix has an inverse.
 @group "Transform Inverse"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_hasInverse
(
TransformCP pTransform
)
    {
    Transform inverse;
    return bsiTransform_invertTransform (&inverse, pTransform);
    }


/*-----------------------------------------------------------------*//**
 Solves the linear system Tx=b, where T is this instance, b is the input
 point and x is the output point.  No simplifying assumptions are made
 regarding the matrix part of T.  Symbolically, if T = [M t], then
 x = Q (b - t), where Q is the inverse of M (i.e., the system is equivalent
 to Mx = b - t).  pInPoint and pOutPoint may have identical addresses.

 @instance pTransform  => The matrix and translation
 @param    pOutPoint   <= solution to system
 @param    pInPoint    => The constant point of the system

 @return false if the matrix part of this instance is singular.
 @group "Transform Inverse"
 @bsimethod                                                       DavidAssaf      1/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_solveDPoint3d
(
TransformCP pTransform,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint
)
    {
    RotMatrix   M;
    DPoint3d    t;

    bsiTransform_getMatrix (pTransform, &M);
    bsiTransform_getTranslation (pTransform, &t);
    bsiDPoint3d_subtractDPoint3dDPoint3d (pOutPoint, pInPoint, &t);

    if (!bsiRotMatrix_solveDPoint3d (&M, pOutPoint, pOutPoint))
        {
        /* treat transform as if it were the identity */
        *pOutPoint = *pInPoint;
        return false;
        }
    else
        return true;
    }



/*-----------------------------------------------------------------*//**
 Solves the linear systems TX=B, where T is this instance, B is the matrix
 of numPoints input points and X is the matrix of numPoints output points.
 No simplifying assumptions are made regarding the matrix part of T.
 Symbolically, if T = [M t], then for each input/output point i,
 X[i] = Q (B[i] - t), where Q is the inverse of M (i.e., the i_th system is
 equivalent to MX[i] = B[i] - t).  pInPoint and pOutPoint may have identical
 addresses.

 @instance pTransform  => matrix and translation
 @param    pOutPoint   <= column points of solution matrix to system
 @param    pInPoint    => The column points of constant matrix of system
 @param    numPoints   => The number of input/output points

 @return false if the matrix part of this instance is singular.
 @group "Transform Inverse"
 @bsimethod                                                       DavidAssaf      1/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_solveDPoint3dArray
(
TransformCP pTransform,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
int           numPoints
)
    {
    RotMatrix   M;
    DPoint3d    t;

    bsiTransform_getMatrix (pTransform, &M);
    bsiTransform_getTranslation (pTransform, &t);
    bsiDPoint3d_copyArray (pOutPoint, pInPoint, numPoints);
    bsiDPoint3d_subtractDPoint3dArray (pOutPoint, &t, numPoints);

    if (!bsiRotMatrix_solveDPoint3dArray (&M, pOutPoint, pOutPoint, numPoints))
        {
        /* treat transform as if it were the identity */
        bsiDPoint3d_copyArray (pOutPoint, pInPoint, numPoints);
        return false;
        }
    else
        return true;
    }



/*-----------------------------------------------------------------*//**
 Sets this instance to the transformation obtained by premultiplying
 pInTransform by 3 matrix rotations about principle axes, given by the
 angles xrot, yrot and zrot.
 pInTransform may be the same as this instance.
 Symbolically, given transform [M t] and rotation
 matrices X,Y,Z, the resulting transform is [X*Y*Z*M X*Y*Z*t]

 @instance pOutTransform <= rotated transformation
 @param pInTransform => The base transformation
 @param xrot => The x axis rotation, in radians
 @param yrot => The y axis rotation, in radians
 @param zrot => The z axis rotation, in radians
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_rotateByPrincipleAngles
(
TransformP pOutTransform,
TransformCP pInTransform,
double         xrot,
double         yrot,
double         zrot
)
    {
    RotMatrix rotation;
    bsiRotMatrix_rotateByPrincipleAngles ( &rotation, NULL, xrot, yrot, zrot);
    if (pInTransform)
        {
        bsiTransform_multiplyRotMatrixTransform (pOutTransform, &rotation, pInTransform);
        }
    else
        {
        bsiTransform_initFromMatrix( pOutTransform, &rotation);
        }
    }



/*-----------------------------------------------------------------*//**
 Scale the columns of the matrix part by respective factors.
 Translation part is unaffected.  (See also scaleMatrixRows, scaleTransformRows)
 @instance pResult <= output transform
 @param pTransform => The input transform.  If NULL, an identity transform is implied.
 @param xscale => The x column scale factor
 @param yscale => The y column scale factor
 @param zscale => The z column scale factor
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_scaleMatrixColumns
(
TransformP pResult,
TransformCP pTransform,
double         xscale,
double         yscale,
double         zscale
)
    {
    if (pTransform)
        {
        *pResult = *pTransform;
        pResult->form3d[0][0] *= xscale;
        pResult->form3d[1][0] *= xscale;
        pResult->form3d[2][0] *= xscale;

        pResult->form3d[0][1] *= yscale;
        pResult->form3d[1][1] *= yscale;
        pResult->form3d[2][1] *= yscale;

        pResult->form3d[0][2] *= zscale;
        pResult->form3d[1][2] *= zscale;
        pResult->form3d[2][2] *= zscale;
        }
    else
        {
        bsiTransform_initIdentity (pResult);
        pResult->form3d[0][0] = xscale;
        pResult->form3d[1][1] = yscale;
        pResult->form3d[2][2] = zscale;
        }
    }



/*-----------------------------------------------------------------*//**
 Scale the rows of the matrix part by respective factors.
 Translation part is unaffected.  (See also scaleMatrixColumns, scaleTransformRows)
 @instance pResult <= output transform
 @param pTransform => The input transform.  If NULL, an identity transform is implied.
 @param xscale => The x column scale factor
 @param yscale => The y column scale factor
 @param zscale => The z column scale factor
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_scaleMatrixRows
(
TransformP pResult,
TransformCP pTransform,
double         xscale,
double         yscale,
double         zscale
)
    {
    if (pTransform)
        {
        *pResult = *pTransform;
        pResult->form3d[0][0] *= xscale;
        pResult->form3d[1][0] *= yscale;
        pResult->form3d[2][0] *= zscale;

        pResult->form3d[0][1] *= xscale;
        pResult->form3d[1][1] *= yscale;
        pResult->form3d[2][1] *= zscale;

        pResult->form3d[0][2] *= xscale;
        pResult->form3d[1][2] *= yscale;
        pResult->form3d[2][2] *= zscale;
        }
    else
        {
        bsiTransform_initIdentity (pResult);
        pResult->form3d[0][0] = xscale;
        pResult->form3d[1][1] = yscale;
        pResult->form3d[2][2] = zscale;
        }
    }



/*-----------------------------------------------------------------*//**
 Scale the complete rows by respective factors.
 (See also scaleMatrixColumns, scaleMatrixRows, which only act on the matrix part)
 @instance pA <= output transform
 @param pTransform => The input transform.  If NULL, an identity transform is implied.
 @param xscale => The x column scale factor
 @param yscale => The y column scale factor
 @param zscale => The z column scale factor
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_scaleCompleteRows
(
TransformP pA,
TransformCP pTransform,
double         xscale,
double         yscale,
double         zscale
)
    {
    if (pTransform)
        {
        *pA = *pTransform;
        pA->form3d[0][0] *= xscale;
        pA->form3d[1][0] *= yscale;
        pA->form3d[2][0] *= zscale;

        pA->form3d[0][1] *= xscale;
        pA->form3d[1][1] *= yscale;
        pA->form3d[2][1] *= zscale;

        pA->form3d[0][2] *= xscale;
        pA->form3d[1][2] *= yscale;
        pA->form3d[2][2] *= zscale;

        pA->form3d[0][3] *= xscale;
        pA->form3d[1][3] *= yscale;
        pA->form3d[2][3] *= zscale;

        }
    else
        {
        bsiTransform_initIdentity (pA);
        pA->form3d[0][0] = xscale;
        pA->form3d[1][1] = yscale;
        pA->form3d[2][2] = zscale;
        }
    }

/* VBSUB(Point3dFromTransform3d) */

/*-----------------------------------------------------------------*//**
 @description Returns the translation (point) part of a transformation.

 @instance pTransform => The transformation whose vector part is returned
 @param pPoint <= vector part of transformation
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_getTranslation
(
TransformCP pTransform,
DPoint3dP pPoint
)
    {
    pPoint->x = pTransform->form3d[0][3];
    pPoint->y = pTransform->form3d[1][3];
    pPoint->z = pTransform->form3d[2][3];
    }


/*-----------------------------------------------------------------*//**
 @description Returns a column from the matrix part of the transformation.

 @instance pTransform => The transformation being accessed.
 @param pColumn <= column of matrix part.
 @param index => index of matrix column.  If outside 012, this is adjusted cyclically.
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_getMatrixColumn
(
TransformCP pTransform,
DPoint3dP pColumn,
int           index
)
    {
    if  (index < 0 || index > 2)
        index = bsiGeom_cyclic3dAxis (index);

    pColumn->x = pTransform->form3d[0][index];
    pColumn->y = pTransform->form3d[1][index];
    pColumn->z = pTransform->form3d[2][index];
    }
/* VBSUB(Matrix3dFromTransform3d) */

/*-----------------------------------------------------------------*//**
 @description Returns the matrix part of a transformation.

 @instance pTransform => The transformation whose matrix part is returned
 @param pMatrix <= matrix part of transformation
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_getMatrix
(
TransformCP pTransform,
RotMatrixP pMatrix
)
    {
    int i;
    for (i = 0; i < 3; i++)
        {
        pMatrix->form3d[0][i] = pTransform->form3d[0][i];
        pMatrix->form3d[1][i] = pTransform->form3d[1][i];
        pMatrix->form3d[2][i] = pTransform->form3d[2][i];
        }
    }


/*-----------------------------------------------------------------*//**
 Adds column i of the matrix part of this instance to point pIn and places
 the result in pOut.

 @instance pTransform => The input transform
 @param pOut <= sum of pIn and column i
 @param pIn  => The base point for sum
 @param i    => The column index of matrix
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_offsetPointByColumn
(
TransformCP pTransform,
DPoint3dP pOut,
DPoint3dCP pIn,
int             i
)
    {
    if  (i < 0 || i > 2)
        i = bsiGeom_cyclic3dAxis (i);

    pOut->x = pIn->x + pTransform->form3d[0][i];
    pOut->y = pIn->y + pTransform->form3d[1][i];
    pOut->z = pIn->z + pTransform->form3d[2][i];
    }


/*-----------------------------------------------------------------*//**
 Sets pPoint0 to the origin (translation part), and sets pPoint1, pPoint2
 pPoint3 to the x, y and z points (translations of columns
 of matrix part by origin) from this instance.

 @instance pTransform => The input transform
 @param pPoint0 <= origin of transform coordinates
 @param pPoint1 <= 100 point of transform coordinates
 @param pPoint2 <= 010 point of transform coordinates
 @param pPoint3 <= 001 point of transform coordinates
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_get4Points
(
TransformCP pTransform,
DPoint3dP pPoint0,
DPoint3dP pPoint1,
DPoint3dP pPoint2,
DPoint3dP pPoint3
)
    {
    bsiTransform_getTranslation (pTransform, pPoint0);
    if (pPoint1)
        bsiTransform_offsetPointByColumn (pTransform, pPoint1, pPoint0, 0);

    if (pPoint2)
        bsiTransform_offsetPointByColumn (pTransform, pPoint2, pPoint0, 1);

    if (pPoint3)
        bsiTransform_offsetPointByColumn (pTransform, pPoint3, pPoint0, 2);
    }


/*-----------------------------------------------------------------*//**
 Sets the four points in pArray to the origin (translation part), x, y and z
 points (translations of columns of matrix part by origin) from this
 instance.

 @param pTransform => The input transformation
 @param pArray <= origin, 100, 010, 001 points as an array
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_get4PointArray
(
TransformCP pTransform,
DPoint3dP pArray
)
    {
    bsiTransform_get4Points (pTransform,
                        pArray, pArray + 1, pArray + 2, pArray + 3);
    }



/*-----------------------------------------------------------------*//**
 Sets pOrigin to the translation part, and sets pVector0, pVector1
 pVector2 to the columns of this instance.

 @instance pTransform => The input transform
 @param pOrigin <= origin of transform coordinates
 @param pVector0 <= 100 vector of transform coordinates
 @param pVector1 <= 010 vector of transform coordinates
 @param pVector2 <= 001 vector of transform coordinates
 @group "Transform Queries"
 @bsimethod                                                       DavidAssaf      5/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_getOriginAndVectors
(
TransformCP pTransform,
DPoint3dP pOrigin,
DPoint3dP pVector0,
DPoint3dP pVector1,
DPoint3dP pVector2
)
    {
    if (pOrigin)
        bsiTransform_getTranslation (pTransform, pOrigin);

    if (pVector0)
        {
        pVector0->x = pTransform->form3d[0][0];
        pVector0->y = pTransform->form3d[1][0];
        pVector0->z = pTransform->form3d[2][0];
        }

    if (pVector1)
        {
        pVector1->x = pTransform->form3d[0][1];
        pVector1->y = pTransform->form3d[1][1];
        pVector1->z = pTransform->form3d[2][1];
        }

    if (pVector2)
        {
        pVector2->x = pTransform->form3d[0][2];
        pVector2->y = pTransform->form3d[1][2];
        pVector2->z = pTransform->form3d[2][2];
        }
    }



/*-----------------------------------------------------------------*//**
 Sets the four points in pArray to the translation part and the columns
 of this instance.

 @instance pTransform => The input transformation
 @param pArray <= origin, 100, 010, 001 vectors as an array
 @group "Transform Queries"
 @bsimethod                                                       DavidAssaf      5/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_getOriginAndVectorArray
(
TransformCP pTransform,
DPoint3dP pArray
)
    {
    bsiTransform_getOriginAndVectors (pTransform,
                        pArray, pArray + 1, pArray + 2, pArray + 3);
    }

/* VBSUB(Transform3dIsIdentity) */


/*-----------------------------------------------------------------*//**
 @description Returns true if the transform is the identity transform.
 @instance pTransform => The transformation to test
 @return true if the transformation is within tolerance of the identity.
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_isIdentity
(
TransformCP pTransform
)
    {
    DPoint3d  origin;
    RotMatrix M;

    bsiTransform_getMatrix (pTransform, &M);

    if (!bsiRotMatrix_isIdentity(&M))
        return false;

#define IS_ZERO(a) ((a < 1.0E-10) && (a > - 1.0E-10))

    bsiTransform_getTranslation (pTransform, &origin);
    if (IS_ZERO(origin.x) &&
        IS_ZERO(origin.y) &&
        IS_ZERO(origin.z))
        return true;

    return false;
    }

/* VBSUB(Transform3dIsRigid) */


/*-----------------------------------------------------------------*//**
 @description Returns true if the matrix part of a transform is a rigid body rotation,
 i.e. its transpose is its inverse and it has a positive determinant.

 @instance pTransform => The transformation to test
 @return true if the transformation is rigid (no scale or shear in the matrix part)
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_isRigid
(
TransformCP pTransform
)
    {
    RotMatrix M;

    bsiTransform_getMatrix (pTransform, &M);
    return  bsiRotMatrix_isRigid (&M);;
    }
/* VBSUB(Transform3dIsPlanar) */


/*-----------------------------------------------------------------*//**
 @description Returns true if transformation effects are entirely within the plane
       with given normal.

 @instance pTransform => The transform to be tested for out-of-plane effects.
 @param pNormal => The plane normal

 @return true if the transform has no effects perpendicular to planes with the given normal.
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_isPlanar
(
TransformCP pTransform,
DVec3dCP pNormal
)
    {
    double norm2, tran2, dot;
    DPoint3d origin;
    RotMatrix M;
    static double tolSquared = 1.0E-20;

    bsiTransform_getTranslation (pTransform, &origin);

    dot = bsiDPoint3d_dotProduct (pNormal, &origin);

    tran2 = bsiDPoint3d_dotProduct (&origin, &origin);
    norm2 = bsiDPoint3d_dotProduct (pNormal, pNormal);
    if (tran2 > tolSquared * norm2 && dot * dot > tolSquared * norm2 * tran2)
        return false;

    bsiTransform_getMatrix (pTransform, &M);
    return bsiRotMatrix_isPlanar (&M, pNormal);
    }

/* VBSUB(Transform3dEqual) */
/* CSVFUNC(IsEqual) */

/*-----------------------------------------------------------------*//**
 @description Returns true if two transforms have exact (bitwise) equality.

 @instance pTransform1 => The first transform
 @param    pTransform2 => The second transform
 @return   true if the transforms are identical
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      05/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_isEqual
(
TransformCP pTransform1,
TransformCP pTransform2
)
    {
    const double *p0 = (const double *)pTransform1;
    const double *p1 = (const double *)pTransform2;
    int i;

    for (i = 0; i < 12; i++)
        {
        if (p0[i] != p1[i])
            return false;
        }
    return true;
    }

/* VBSUB(Transform3dEqualTolerance) */
/* CSVFUNC(IsEqual) */

/*-----------------------------------------------------------------*//**
 @description Returns true if two transformations are equal within tolerance, using
       separate tolerances for the matrix and point parts.
 @param pTransform1 IN first matrix
 @param pTransform2 IN second matrix
 @param matrixTolerance IN tolerance for comparing matrix components
 @param pointTolerance IN tolerance for comparing point components.
 @return   true if the transforms are identical
 @group "Transform Queries"
 @bsimethod                                                       EarlinLutz      05/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_isEqualTolerance
(
TransformCP pTransform1,
TransformCP pTransform2,
double                  matrixTolerance,
double                  pointTolerance
)
    {
    RotMatrix matrix0, matrix1;
    DPoint3d  point0,  point1;
    bsiTransform_getMatrix (pTransform1, &matrix0);
    bsiTransform_getMatrix (pTransform2, &matrix1);

    if (!bsiRotMatrix_matrixEqualTolerance (&matrix0, &matrix1, matrixTolerance))
        return false;
    bsiTransform_getTranslation (pTransform1, &point0);
    bsiTransform_getTranslation (pTransform2, &point1);
    return bsiDPoint3d_pointEqualTolerance (&point0, &point1, pointTolerance);
    }



/*-----------------------------------------------------------------*//**
 @description Returns a transformation with origin at pOrigin, x-axis
 pXVector, y-axis pYVector, and z-axis pZVector.
 All axes are unnormalized.
 There is no effort to detect zero length axes or degenerate points that
 define only a line or plane but not a full coordinate system.
 The axes may be skewed.

 @instance pTransform <= computed transformation
 @param pOrigin => The origin of transformed coordinates
 @param pXVector => The 100 point of transformed coordinates
 @param pYVector => The 010 point of transformed coordinates
 @param pZVector => The 001 point of transformed coordinates
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFromOriginAndVectors
(
TransformP pTransform,
DPoint3dCP pOrigin,
DPoint3dCP pXVector,
DPoint3dCP pYVector,
DPoint3dCP pZVector
)
    {
    pTransform->form3d[0][3] = pOrigin->x;
    pTransform->form3d[1][3] = pOrigin->y;
    pTransform->form3d[2][3] = pOrigin->z;

    pTransform->form3d[0][0] = pXVector->x;
    pTransform->form3d[1][0] = pXVector->y;
    pTransform->form3d[2][0] = pXVector->z;

    pTransform->form3d[0][1] = pYVector->x;
    pTransform->form3d[1][1] = pYVector->y;
    pTransform->form3d[2][1] = pYVector->z;

    pTransform->form3d[0][2] = pZVector->x;
    pTransform->form3d[1][2] = pZVector->y;
    pTransform->form3d[2][2] = pZVector->z;
    }


/*-----------------------------------------------------------------*//**
 @description Returns a transformation with origin at pOrigin, x-axis from
 pOrigin to pXPoint, y-axis from pOrigin to pYPoint,
 and z-axis from pOrigin to pZPoint.
 All axes are unnormalized.
 There is no effort to detect zero length axes or degenerate points that
 define only a line or plane but not a full coordinate system.
 The axes may be skewed.

 @instance pTransform <= computed transformation
 @param pOrigin => The origin of transformed coordinates
 @param pXPoint => The 100 point of transformed coordinates
 @param pYPoint => The 010 point of transformed coordinates
 @param pZPoint => The 001 point of transformed coordinates
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFrom4Points
(
TransformP pTransform,
DPoint3dCP pOrigin,
DPoint3dCP pXPoint,
DPoint3dCP pYPoint,
DPoint3dCP pZPoint
)
    {
    DPoint3d xVector, yVector, zVector;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&xVector, pXPoint, pOrigin);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&yVector, pYPoint, pOrigin);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&zVector, pZPoint, pOrigin);
    bsiTransform_initFromOriginAndVectors (pTransform,
                pOrigin, &xVector, &yVector, &zVector);
    }


/*-----------------------------------------------------------------*//**
 @description Returns a transformation with origin at pOrigin, x-axis from
 pOrigin to pXPoint, y-axis from pOrigin to pYPoint,
 and z-axis equal to the cross product of x and y axes.
 All axes are unnormalized.
 There is no effort to detect zero length axes or degenerate points that
 define only a line or plane but not a full coordinate system.

 @instance pTransform <= computed transformation
 @param pOrigin => The origin of coordinate system
 @param pXPoint => The 100 point of coordinate system
 @param pYPoint => The 010 point of coordinate system
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFrom3DPoint3d
(
TransformP pTransform,
DPoint3dCP pOrigin,
DPoint3dCP pXPoint,
DPoint3dCP pYPoint
)
    {
    DPoint3d xVector, yVector, zVector;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&xVector, pXPoint, pOrigin);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&yVector, pYPoint, pOrigin);
    bsiDPoint3d_crossProduct (&zVector, &xVector, &yVector);
    bsiTransform_initFromOriginAndVectors (pTransform,
                pOrigin, &xVector, &yVector, &zVector);
    }


/*-----------------------------------------------------------------*//**
 Approximate a coordinate frame through a set of points.
 The xy plane is determined by planeThroughPoints.
 The xy axes are arbitrary within that plane, and z is perpendicular.
 @param pTransform <= transformation
 @param pPoint => The point array
 @param numPoint => The number of points

 @return true if the points define a clear plane.
 @group "Transform Initialization"
 @bsimethod                                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiTransform_initFromPlaneOfDPoint3dArray

(
TransformP pTransform,
DPoint3dCP pPoint,
int             numPoint
)
    {
    DPoint3d normal;
    DPoint3d origin;
    DPoint3d vector0, vector1, vector2;
    bool    boolstat = false;
    if (bsiGeom_planeThroughPoints (&normal, &origin, pPoint, numPoint))
        {
        boolstat = bsiDPoint3d_getNormalizedTriad (&normal,
                    &vector0, &vector1, &vector2);
        bsiTransform_initFromOriginAndVectors (pTransform,
                        &origin, &vector0, &vector1, &vector2);
        }
    return boolstat;
    }



/*-----------------------------------------------------------------*//**
 @description Returns a (possibly skewed) transformation with origin
 pOrigin, the axis axisId towards pXPoint, and other axes perpendicular.
 If normalize is false, all axes have length equal to the distance
 between the two pOrigin and pXPoint.
 The axes may be skewed.

 @instance pTransform <= computed transformation
 @param pOrigin => The origin of coordinate system
 @param pXPoint => The target point of axis axisId of coordinate system
 @param axisId => The axis that points from pOrigin to pXPoint
 @param normalize => true to have coordinate system normalized
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFrom2Points
(
TransformP pTransform,
DPoint3dCP pOrigin,
DPoint3dCP pXPoint,
int            axisId,
bool           normalize
)
    {
    DVec3d vector;
    RotMatrix matrix;
    bsiDVec3d_subtractDPoint3dDPoint3d (&vector, pXPoint, pOrigin);
    bsiRotMatrix_initFrom1Vector(&matrix, &vector, axisId, normalize);
    bsiTransform_initFromMatrixAndTranslation (pTransform, &matrix, pOrigin);
    }


/*-----------------------------------------------------------------*//**
 Sets pNpcToGlobal to the transformation from 000...111 to the
 globallyaligned cube with diagonally opposite corners pMin and pMax.
 Sets pGlobalToNpc to the inverse of this transform.
 The diagonal component for any degenerate direction is 1.
 A NULL for either transformation parameter indicates that that
 transformation is not needed.

 @param pNpcToGlobal <= transformation from unit cube to given cube
 @param pGlobalToNpc <= transformation from given cube to unit cube
 @param pMin => The 000 corner of cube
 @param pMax => The 111 corner of cube
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFromRange
(
TransformP pNpcToGlobal,
TransformP pGlobalToNpc,
DPoint3dCP pMin,
DPoint3dCP pMax
)
    {
    DPoint3d diag;
    RotMatrix matrix;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&diag, pMax, pMin);

    if (diag.x == 0.0)
        diag.x = 1.0;
    if (diag.y == 0.0)
        diag.y = 1.0;
    if (diag.z == 0.0)
        diag.z = 1.0;
    if (pNpcToGlobal)
        {
        bsiRotMatrix_initFromScaleFactors (&matrix, diag.x, diag.y, diag.z);
        bsiTransform_initFromMatrixAndTranslation (pNpcToGlobal, &matrix, pMin);

        }

    if (pGlobalToNpc)
        {
        DPoint3d origin;

        origin.x = - pMin->x / diag.x;
        origin.y = - pMin->y / diag.y;
        origin.z = - pMin->z / diag.z;

        bsiRotMatrix_initFromScaleFactors (&matrix, 1.0 / diag.x, 1.0 / diag.y, 1.0 /diag.z);
        bsiTransform_initFromMatrixAndTranslation (pGlobalToNpc, &matrix, &origin);
        }
    }


/*-----------------------------------------------------------------*//**
 Initializes a transformation that takes pMin1 and pMax1 to
 pMin2 and pMax2 (respectively).   Initializes pTransform21 to the inverse
 of this transform.

 The diagonal component for any degenerate direction is 1.
 A NULL for either transformation parameter indicates that that
 transformation is not needed.

 @param pTransform12 <= forward transformation
 @param pTransform21 <= inverse transformation
 @param pMin1 => The 000 corner of cube 1
 @param pMax1 => The 111 corner of cube 1
 @param pMin2 => The 000 corner of cube 2
 @param pMax2 => The 111 corner of cube 2
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFromRangePair
(
TransformP pTransform12,
TransformP pTransform21,
DPoint3dCP pMin1,
DPoint3dCP pMax1,
DPoint3dCP pMin2,
DPoint3dCP pMax2
)
    {
    DPoint3d diag1, diag2, diag;
    DPoint3d translate;
    RotMatrix matrix;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&diag1, pMax1, pMin1);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&diag2, pMax2, pMin2);

    if (diag1.x == 0.0)
        diag1.x = 1.0;
    if (diag1.y == 0.0)
        diag1.y = 1.0;
    if (diag1.z == 0.0)
        diag1.z = 1.0;

    if (diag2.x == 0.0)
        diag2.x = 1.0;
    if (diag2.y == 0.0)
        diag2.y = 1.0;
    if (diag2.z == 0.0)
        diag2.z = 1.0;
    if (pTransform12)
        {
        diag.x = diag1.x / diag2.x;
        diag.y = diag1.y / diag2.y;
        diag.z = diag1.z / diag2.z;

        translate.x = pMin2->x - diag.x * pMin1->x;
        translate.y = pMin2->y - diag.y * pMin1->y;
        translate.z = pMin2->z - diag.z * pMin1->z;

        bsiRotMatrix_initFromScaleFactors (&matrix, diag.x, diag.y, diag.z);
        bsiTransform_initFromMatrixAndTranslation (pTransform12, &matrix, &translate);
        }

    if (pTransform21)
        {
        diag.x = diag2.x / diag1.x;
        diag.y = diag2.y / diag1.y;
        diag.z = diag2.z / diag1.z;

        translate.x = pMin1->x - diag.x * pMin2->x;
        translate.y = pMin1->y - diag.y * pMin2->y;
        translate.z = pMin1->z - diag.z * pMin2->z;
        bsiRotMatrix_initFromScaleFactors (&matrix, diag.x, diag.y, diag.z);
        bsiTransform_initFromMatrixAndTranslation (pTransform21, &matrix, &translate);
        }
    }

/*-----------------------------------------------------------------*//**
 Initializes a Transform from the affine part a 4x4 matrix.
 If the scale factor is 0, the transform is returned as an identity.
 If the scale factor is other than 1, the all coefficients are divided
 by the scale factor.
 Return code is false if the 4'th row is significantly different from
 (0,0,0,A).

 @param pTransform <= extracted transform
 @param pMatrix => The source matrix.

 @return true if the DMatrix4d can be collapsed to affine form.
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_initFromHMatrix
(
TransformP pTransform,
DMatrix4dCP pMatrix
)
    {
    double scale = pMatrix->coff[3][3];
    static double relTol = 1.0e-12;
    bool    boolStat = true;
    int i;
    double sum;

    if (scale == 0.0)
        {
        boolStat = false;
        bsiTransform_initIdentity (pTransform);
        }
    else
        {
        if (scale == 1.0)
            {
            /* This is really the most common case.*/
            for (i = 0; i < 3; i++)
                {
                pTransform->form3d[0][i] = pMatrix->coff[0][i];
                pTransform->form3d[1][i] = pMatrix->coff[1][i];
                pTransform->form3d[2][i] = pMatrix->coff[2][i];
                }
            pTransform->form3d[0][3] = pMatrix->coff[0][3];
            pTransform->form3d[1][3] = pMatrix->coff[1][3];
            pTransform->form3d[2][3] = pMatrix->coff[2][3];
            }
        else
            {
            double a = 1.0 / scale;
            for (i = 0; i < 3; i++)
                {
                pTransform->form3d[0][i] = a * pMatrix->coff[0][i];
                pTransform->form3d[1][i] = a * pMatrix->coff[1][i];
                pTransform->form3d[2][i] = a * pMatrix->coff[2][i];
                }
            pTransform->form3d[0][3] = a * pMatrix->coff[0][3];
            pTransform->form3d[1][3] = a * pMatrix->coff[1][3];
            pTransform->form3d[2][3] = a * pMatrix->coff[2][3];
            }
        sum =
                      fabs (pMatrix->coff[3][0])
                    + fabs (pMatrix->coff[3][1])
                    + fabs (pMatrix->coff[3][2]);
        if (sum >= relTol * scale)
            boolStat = false;
        }
    return boolStat;
    }


/*-----------------------------------------------------------------*//**
 Initializes a Transform from the affine part of one of the
 matrices in an DMap4d.
 If the scale factor is 0, the transform is returned as an identity.
 If the scale factor is other than 1, the all coefficients are divided
 by the scale factor.
 Return code is false if the 4'th row is significantly different from
 (0,0,0,A).

 @instance pTransform <= extracted transform
 @param pHMap => The source mapping
 @param inverse => true to extract the inverse, false to extract the forward

 @return true if the DMatrix4d can be collapsed to affine form.
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_initFromHMap
(
TransformP pTransform,
DMap4dCP            pHMap,
int             inverse
)
    {
    return bsiTransform_initFromHMatrix(
                    pTransform,
                    inverse ? &pHMap->M1 : &pHMap->M0
                    );
    }


/*-----------------------------------------------------------------*//**
 @description Returns a copy of a transformation.

 @instance pDest <= copied data
 @param pSource => The source transform
 @group "Transform Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_copy
(
TransformP pDest,
TransformCP pSource
)
    {
    if  (pDest)
        if  (pSource)
            *pDest = *pSource;
        else
            bsiTransform_initIdentity (pDest);
    }

/*-----------------------------------+
|
| 2D TRANSFORM ROUTINES
|
+-----------------------------------*/



/*-----------------------------------------------------------------*//**
 @description Returns a transformation in the xy-plane with origin pOrigin
 and x,y-axes of given lengths.
 The z-coordinate of the origin is zero and the z-axis is unscaled.

 @instance pTransform   <= 2D transform with scaled identity matrix and translation pOrigin
 @param pOrigin        => origin of coordinate system (or null)
 @param xAxisLength    => The length of x-axis
 @param yAxisLength    => The length of y-axis
 @group "Transform Initialization"
 @bsimethod                                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFromDPoint2dOriginAndLengths
(
TransformP pTransform,
DPoint2dCP pOrigin,
double          xAxisLength,
double          yAxisLength
)
    {
    pTransform->form3d[0][0] = xAxisLength;
    pTransform->form3d[1][0] = 0.0;
    pTransform->form3d[2][0] = 0.0;

    pTransform->form3d[0][1] = 0.0;
    pTransform->form3d[1][1] = yAxisLength;
    pTransform->form3d[2][1] = 0.0;

    pTransform->form3d[0][2] = 0.0;
    pTransform->form3d[1][2] = 0.0;
    pTransform->form3d[2][2] = 1.0;

    if (pOrigin)
        {
        pTransform->form3d[0][3] = pOrigin->x;
        pTransform->form3d[1][3] = pOrigin->y;
        }
    else
        {
        pTransform->form3d[0][3] =
        pTransform->form3d[1][3] = 0.0;
        }

    pTransform->form3d[2][3] = 0.0;
    }



/*-----------------------------------------------------------------*//**
 @description Returns a transformation in the xy-plane with    origin pOrigin and
 x,y-axes of the given lengths rotated counter-clockwise from standard position
 by the given angle.
 The z-coordinate of the origin is zero and the z-axis is unscaled.

 @instance pTransform   <= 2D transform with scaled and rotated matrix, and translation pOrigin
 @param pOrigin        => origin of coordinate system (or null)
 @param xAxisAngleRadians => The ccw angle     separating x-axis from its standard position
 @param xAxisLength    => The length of x-axis
 @param yAxisLength    => The length of y-axis
 @group "Transform Initialization"
 @bsimethod                                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFromDPoint2dOriginAngleAndLengths
(
TransformP pTransform,
DPoint2dCP pOrigin,
double          xAxisAngleRadians,
double          xAxisLength,
double          yAxisLength
)
    {
    double c = cos(xAxisAngleRadians);
    double s = sin(xAxisAngleRadians);

    pTransform->form3d[0][0] = xAxisLength * c;
    pTransform->form3d[1][0] = xAxisLength * s;
    pTransform->form3d[2][0] = 0.0;

    pTransform->form3d[0][1] = yAxisLength * -s;
    pTransform->form3d[1][1] = yAxisLength * c;
    pTransform->form3d[2][1] = 0.0;

    pTransform->form3d[0][2] = 0.0;
    pTransform->form3d[1][2] = 0.0;
    pTransform->form3d[2][2] = 1.0;

    if (pOrigin)
        {
        pTransform->form3d[0][3] = pOrigin->x;
        pTransform->form3d[1][3] = pOrigin->y;
        }
    else
        {
        pTransform->form3d[0][3] =
        pTransform->form3d[1][3] = 0.0;
        }

    pTransform->form3d[2][3] = 0.0;
    }



/*-----------------------------------------------------------------*//**
 Sets the translation part of this instance so that it leaves the given point
 unchanged, i.e. so that this instance may be interpreted as
 applying its matrix part as a rotation or scaling (in the xy-plane)
 about the point.
 Symbolically, given transform [R t] and column vector p,
 the returned transform is [R p-R*p].
 (The prior translation part is destroyed, and does not affect the
 result in any way.)

 @instance pTransform <=> 2D transformation whose translation part is modified
 @param pPoint => The point that is to remain fixed when multiplied by the modified transformation
 @group "Transform In Place Modification"
 @bsimethod                                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_setDPoint2dFixedPoint
(
TransformP pTransform,
DPoint2dCP pPoint
)
    {
    pTransform->form3d[0][3] = pPoint->x
                - pTransform->form3d[0][0] * pPoint->x
                - pTransform->form3d[0][1] * pPoint->y;

    pTransform->form3d[1][3] = pPoint->y
                - pTransform->form3d[1][0] * pPoint->x
                - pTransform->form3d[1][1] * pPoint->y;

    pTransform->form3d[2][3] = 0.0;
    }



/*-----------------------------------------------------------------*//**
 Sets the translation part of this instance to the given point.
 The prior translation part is overwritten (and z-coord set to zero) and the
 matrix part is unchanged.
 Symbolically, if pPoint is u then this instance [R t] becomes the
 transformation [R u].

 @instance pTransform <=> transformation whose vector part is modified
 @param pPoint => The point to insert
 @group "Transform In Place Modification"
 @bsimethod                                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_setDPoint2dTranslation
(
TransformP pTransform,
DPoint2dCP pPoint
)
    {
    pTransform->form3d[0][3] = pPoint->x;
    pTransform->form3d[1][3] = pPoint->y;
    pTransform->form3d[2][3] = 0.0;
    }



/*-----------------------------------------------------------------*//**
 Sets the given point to the x- and y-coordinates of the translation part
 of this instance.

 @instance pTransform => The transformation whose translation part is returned
 @param pPoint <= translation part of transformation
 @group "Transform Queries"
 @bsimethod                                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_getDPoint2dTranslation
(
TransformCP pTransform,
DPoint2dP pPoint
)
    {
    pPoint->x = pTransform->form3d[0][3];
    pPoint->y = pTransform->form3d[1][3];
    }



/*-----------------------------------------------------------------*//**
 Sets this instance to a transformation in the xy-plane with origin at
 pOrigin, x-axis pXVector and y-axis pYVector.
 All axes are unnormalized.
 There is no effort to detect zero length axes or degenerate points that
 define a line but not a full coordinate system.
 The axes may be skewed.

 @instance pTransform <= computed 2D transformation
 @param pOrigin => The origin of transformed coordinates (or null)
 @param pXVector => The 10 point of transformed coordinates
 @param pYVector => 01 point of transformed coordinates
 @group "Transform Initialization"
 @bsimethod                                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFromDPoint2dOriginAndVectors
(
TransformP pTransform,
DPoint2dCP pOrigin,
DPoint2dCP pXVector,
DPoint2dCP pYVector
)
    {
    pTransform->form3d[0][0] = pXVector->x;
    pTransform->form3d[1][0] = pXVector->y;
    pTransform->form3d[2][0] = 0.0;

    pTransform->form3d[0][1] = pYVector->x;
    pTransform->form3d[1][1] = pYVector->y;
    pTransform->form3d[2][1] = 0.0;

    pTransform->form3d[0][2] = 0.0;
    pTransform->form3d[1][2] = 0.0;
    pTransform->form3d[2][2] = 1.0;

    if (pOrigin)
        {
        pTransform->form3d[0][3] = pOrigin->x;
        pTransform->form3d[1][3] = pOrigin->y;
        }
    else
        {
        pTransform->form3d[0][3] =
        pTransform->form3d[1][3] = 0.0;
        }

    pTransform->form3d[2][3] = 0.0;
    }



/*-----------------------------------------------------------------*//**
 Sets this instance to a transformation in the xy-plane with origin at pOrigin,
 x-axis from pOrigin to pXPoint and y-axis from pOrigin to pYPoint.
 All axes are unnormalized.
 There is no effort to detect zero length axes or degenerate points that
 define a line but not a full coordinate system.
 The axes may be skewed.

 @instance pTransform <= computed 2D transformation
 @param pOrigin => The origin of transformed coordinates (or null)
 @param pXPoint => The 10 point of transformed coordinates
 @param pYPoint => The 01 point of transformed coordinates
 @group "Transform Initialization"
 @bsimethod                                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFrom3DPoint2d
(
TransformP pTransform,
DPoint2dCP pOrigin,
DPoint2dCP pXPoint,
DPoint2dCP pYPoint
)
    {
    if (!pOrigin)
        bsiTransform_initFromDPoint2dOriginAndVectors
                (pTransform, NULL, pXPoint, pYPoint);
    else
        {
        pTransform->form3d[0][0] = pXPoint->x - pOrigin->x;
        pTransform->form3d[1][0] = pXPoint->y - pOrigin->y;
        pTransform->form3d[2][0] = 0.0;

        pTransform->form3d[0][1] = pYPoint->x - pOrigin->x;
        pTransform->form3d[1][1] = pYPoint->y - pOrigin->y;
        pTransform->form3d[2][1] = 0.0;

        pTransform->form3d[0][2] = 0.0;
        pTransform->form3d[1][2] = 0.0;
        pTransform->form3d[2][2] = 1.0;

        pTransform->form3d[0][3] = pOrigin->x;
        pTransform->form3d[1][3] = pOrigin->y;
        pTransform->form3d[2][3] = 0.0;
        }
    }


/*-----------------------------------------------------------------*//**
 Initialize from first three rows of DMatrix4d, divided by weight entry.
 ALWAYS copy the data, even if final row has invalid data.
 @instance pTransform OUT initialized transform
 @param pMatrix IN 4x4 matrix
 @return false if 4th row is other than 0001
 @group "Transform Initialization"
 @bsimethod                                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_initFromDMatrix4d
(
TransformP pTransform,
DMatrix4dCP pMatrix
)
    {
    return bsiTransform_initFromHMatrix (pTransform, pMatrix);
    }


/*-----------------------------------------------------------------*//**
 @description Returns a transformation in the xy-plane
 with origin pOrigin, axis axisId towards pXPoint, and the other axis
 perpendicular.
 If normalize is false, both axes have length equal to the distance
 between pOrigin and pXPoint.

 @instance pTransform <= computed 2D transformation
 @param pOrigin => The origin of coordinate system (or null)
 @param pXPoint => The target point of axis axisId of coordinate system
 @param axisId => The axis (x=0, y=1) that points from pOrigin to pXPoint
 @param normalize => true to have coordinate system normalized
 @group "Transform Initialization"
 @bsimethod                                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_initFrom2DPoint2d
(
TransformP pTransform,
DPoint2dCP pOrigin,
DPoint2dCP pXPoint,
int            axisId,
bool           normalize
)
    {
    double mag, magRecip;
    DPoint2d vector1, vector2;

    if (pOrigin)
        {
        vector1.x = pXPoint->x - pOrigin->x;
        vector1.y = pXPoint->y - pOrigin->y;
        }
    else
        {
        vector1.x = pXPoint->x;
        vector1.y = pXPoint->y;
        }

    mag = sqrt (vector1.x * vector1.x + vector1.y * vector1.y);

    if (mag > 0.0)
        {
        if (normalize)
            {
            magRecip = 1.0 / mag;
            vector1.x *= magRecip;
            vector1.y *= magRecip;
            }
        }
    else                /* ensure identity matrix if pXPoint is zero vector */
        {
        vector1.x = (axisId == 0) ? 1.0 : 0.0;
        vector1.y = (axisId == 0) ? 0.0 : 1.0;
        }

    if (axisId == 0)    /* x-axis in direction of vector1 */
        {
        vector2.x = -vector1.y;
        vector2.y =  vector1.x;

        bsiTransform_initFromDPoint2dOriginAndVectors
                (pTransform, pOrigin, &vector1, &vector2);
        }
    else                /* y-axis in direction vector1 */
        {
        vector2.x =  vector1.y;
        vector2.y = -vector1.x;

        bsiTransform_initFromDPoint2dOriginAndVectors
                (pTransform, pOrigin, &vector2, &vector1);
        }
    }



/*-----------------------------------------------------------------*//**
 Sets pOrigin to the translation part, and sets pVector0 and pVector1
 to the columns of this 2D instance.

 @instance pTransform => The input 2D transform
 @param pOrigin <= origin of transform coordinates
 @param pVector0 <= 10 vector of transform coordinates
 @param pVector1 <= 01 vector of transform coordinates
 @group "Transform Queries"
 @bsimethod                                                       DavidAssaf      5/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_getDPoint2dOriginAndVectors
(
TransformCP pTransform,
DPoint2dP pOrigin,
DPoint2dP pVector0,
DPoint2dP pVector1
)
    {
    if (pOrigin)
        bsiTransform_getDPoint2dTranslation (pTransform, pOrigin);

    if (pVector0)
        {
        pVector0->x = pTransform->form3d[0][0];
        pVector0->y = pTransform->form3d[1][0];
        }

    if (pVector1)
        {
        pVector1->x = pTransform->form3d[0][1];
        pVector1->y = pTransform->form3d[1][1];
        }
    }



/*-----------------------------------------------------------------*//**
 Sets the three points in pArray to the translation part and the columns
 of this 2D instance.

 @instance pTransform => The input 2D transformation
 @param pArray <= origin, 10, 01 vectors as an array
 @group "Transform Queries"
 @bsimethod                                                       DavidAssaf      5/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_getDPoint2dOriginAndVectorArray
(
TransformCP pTransform,
DPoint2dP pArray
)
    {
    bsiTransform_getDPoint2dOriginAndVectors (pTransform,
                        pArray, pArray + 1, pArray + 2);
    }

/* VBSUB(Transform3dFromSquaredTransform3d) */

/*-----------------------------------------------------------------*//**
 @description Construct a transform which preserves both a primary column directon
 and a secondary column plane.   Scale all columns to length of
 primary axis.
 @instance pSquaredColumns <= matrix with squared columns.
 @param pTransform => original matrix.
 @param primaryAxis => axis to be retained.
 @param secondaryAxis => axis defining plane to be maintained.
 @return true if the primary and secondary axes are independent.
 @group "Transform Initialization"
 @bsimethod                               EarlinLutz          10/01
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_initFromSquaredColumns
(
TransformP pSquaredColumns,
TransformCP pTransform,
int             primaryAxis,
int             secondaryAxis
)
    {
    RotMatrix matrixPart, rotationPart, skewPart;
    DPoint3d translation;
    bool    boolstat = false;
    double scale;
    bsiRotMatrix_initFromTransform(&matrixPart, pTransform);
    bsiTransform_getTranslation (pTransform, &translation);
    if (bsiRotMatrix_rotateAndSkewFactors
                    (
                    &matrixPart,
                    &rotationPart,
                    &skewPart,
                    primaryAxis,
                    secondaryAxis
                    ))
        {
        scale = bsiRotMatrix_getComponentByRowAndColumn (&skewPart, primaryAxis, primaryAxis);
        bsiRotMatrix_scaleColumns (&rotationPart, &rotationPart, scale, scale, scale);
        boolstat = true;
        }
    else
        {
        bsiRotMatrix_initIdentity (&matrixPart);
        }

    bsiTransform_initFromMatrixAndTranslation (pSquaredColumns, &rotationPart, &translation);
    return boolstat;
    }

/* VBSUB(Transform3dFromMirrorPlane) */
/*-----------------------------------------------------------------*//**
 @description Construct a transform which mirrors about a plane.
 @instance pTransform  <= mirror transform.
 @param pOrigin => any point on plane.
 @param pNormal => normal vector.
 @return false if the plane normal is 000.
 @group "Transform Initialization"
 @bsimethod                               EarlinLutz          10/01
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_initFromMirrorPlane
(
TransformP pTransform,
DPoint3dCP pOrigin,
DPoint3dCP pNormal
)
    {
    DVec3d unitNormal;
    RotMatrix matrixPart;
    DPoint3d translationPart;
    bool    boolstat;
    double magnitude = bsiDPoint3d_normalize (&unitNormal, pNormal);

    if (magnitude == 0.0)
        {
        bsiTransform_initIdentity (pTransform);
        boolstat = false;
        }
    else
        {
        bsiRotMatrix_initIdentity (&matrixPart);
        bsiRotMatrix_addScaledOuterProductInPlace (&matrixPart, &unitNormal, &unitNormal, -2.0);
        bsiDPoint3d_scale (&translationPart, &unitNormal, 2.0 * bsiDPoint3d_dotProduct (&unitNormal, pOrigin));
        bsiTransform_initFromMatrixAndTranslation (pTransform, &matrixPart, &translationPart);
        boolstat = true;
        }

    return boolstat;
    }


/*-----------------------------------------------------------------*//**
 @description Returns true if the transform is a simple translation.
 @param pTransform => The transformation to test
 @param pTranslation <= the translation vector. Zero of not a translation transform.
 @return true if the transformation is a pure translation.
 @group "Transform Queries"
 @bsimethod                               EarlinLutz      03/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_isTranslate
(
TransformCP pTransform,
DPoint3dP pTranslation
)
    {
    RotMatrix matrix;
    bsiTransform_getMatrix (pTransform, &matrix);

    if (bsiRotMatrix_isIdentity(&matrix))
        {
        if (pTranslation)
            bsiTransform_getTranslation (pTransform, pTranslation);
        return true;
        }
    else
        {
        if (pTranslation)
            bsiDPoint3d_zero (pTranslation);
        return false;
        }
    }


/*-----------------------------------------------------------------*//**
 @description Returns true if the transform is a uniform scale with scale factor other than 1.0.
 @param pTransform => The transformation to test
 @param pFixedPoint <= (If function result is true) the (one) point which
                           remains in place in the transformation.
 @param pScale <= The scale factor.  If the transform is not a scale, this is returned as 1.0.
 @return true if the transformation is a uniform scale.
 @group "Transform Queries"
 @bsimethod                               EarlinLutz      03/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_isUniformScale
(
TransformCP pTransform,
DPoint3dP pFixedPoint,
double          *pScale
)
    {
    DPoint3d  fixedPoint;
    DPoint3d  translation;
    RotMatrix matrix;
    double scale;
    double a;
    bool    isUniformScale = false;

    bsiTransform_getMatrix (pTransform, &matrix);
    bsiDPoint3d_zero (&fixedPoint);
    scale = 1.0;

    if (bsiRotMatrix_isUniformScale (&matrix, &scale))
        {
        bsiTransform_getTranslation (pTransform, &translation);
        a = 1.0 - scale;
        isUniformScale = bsiDPoint3d_safeDivide (&fixedPoint, &translation, a);
        }
    else
        {
        scale = 1.0;
        }

    if (pFixedPoint)
        *pFixedPoint = fixedPoint;
    if (pScale)
        *pScale = scale;
    return isUniformScale;
    }


/*-----------------------------------------------------------------*//**
 @description Returns true if the transform is a non-zero rotation around a line.
 @param pTransform => The transformation to test
 @param pFixedPoint <= a point on the line.
 @param pDirectionVector <= vector in the line direction.
 @param pRadians <= rotation angle in radians.
 @return true if the transformation is a non-zero rotation.
 @group "Transform Queries"
 @bsimethod                               EarlinLutz      03/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_isRotateAroundLine
(
TransformCP pTransform,
DPoint3dP pFixedPoint,
DPoint3dP pDirectionVector,
double          *pRadians
)
    {
    DPoint3d  fixedPoint;
    Transform basis;
    DVec3d directionVector;
    RotMatrix matrix;
    double radians;
    int nullSpaceDimension;
    bool    result = false;
    DPoint3d errorVector;

    bsiTransform_getMatrix (pTransform, &matrix);
    bsiDPoint3d_setXYZ (&directionVector, 0.0, 0.0, 1.0);
    bsiDPoint3d_zero (&fixedPoint);
    radians = 0.0;

    if (    bsiRotMatrix_isRigid (&matrix)
        && !bsiRotMatrix_isIdentity (&matrix)
        && bsiTransform_invariantSpaceBasis (pTransform, &basis, &nullSpaceDimension,
                    NULL, NULL, &errorVector, 0.0)
        )
        {
        bsiTransform_getTranslation (&basis, &fixedPoint);
        radians = bsiRotMatrix_getRotationAngleAndVector (&matrix, &directionVector);
        result = true;
        }

    if (pFixedPoint)
        *pFixedPoint = fixedPoint;
    if (pDirectionVector)
        *pDirectionVector = directionVector;
    if (pRadians)
        *pRadians = radians;
    return result;
    }


/*-----------------------------------------------------------------*//**
 @description Returns true if the transform is a mirror with respect to
       a plane.
 @param pTransform => The transformation to test
 @param pPlanePoint <= Some point on the plane.
 @param pUnitNormal <= unit vector perpendicular to the plane.
 @return true if the transformation is a mirror.
 @group "Transform Queries"
 @bsimethod                               EarlinLutz      03/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_isMirrorAboutPlane
(
TransformCP pTransform,
DPoint3dP pPlanePoint,
DPoint3dP pUnitNormal
)
    {
    bool    result = false;
    RotMatrix matrix;
    DPoint3d translation;
    DPoint3d refPoint, planeOrigin, imagePoint;
    DPoint3d planeNormal;
    Transform invariantSpace;
    RotMatrix variantSpace;
    int invariantDimension, variantDimension;
    DPoint3d translationShift;
    DVec3d mirrorVector, mirrorImage;

    static double s_scaleTol = 1.0e-15;

    bsiDPoint3d_zero (&refPoint);
    bsiDPoint3d_zero (&planeOrigin);
    bsiDPoint3d_zero (&planeNormal);

    bsiTransform_getMatrix (pTransform, &matrix);
    bsiTransform_getTranslation (pTransform, &translation);
    bsiTransform_multiplyDPoint3d (pTransform, &imagePoint, &refPoint);

    if (bsiTransform_invariantSpaceBasis (pTransform, &invariantSpace, &invariantDimension,
                                            &variantSpace, &variantDimension, &translationShift, s_scaleTol)
        && variantDimension == 1    // the variant dimension is the plane normal
        && invariantDimension == 2  // the invariant dimensions are the plane tangents
        )
        {
        double a;
        bsiRotMatrix_getColumn (&variantSpace, &mirrorVector, 0);
        bsiTransform_multiplyDPoint3dByMatrixPart (pTransform, &mirrorImage, &mirrorVector);
        a = bsiDVec3d_dotProduct (&mirrorImage, &mirrorVector) ;
        if (fabs (a + 1.0) <= s_scaleTol)
            {
            result = true;
            bsiTransform_getTranslation (&invariantSpace, &planeOrigin);
            planeNormal = mirrorVector;
            }
        }
    if (pPlanePoint)
        *pPlanePoint = planeOrigin;
    if (pUnitNormal)
        *pUnitNormal = planeNormal;

    return result;
    }


/*-----------------------------------------------------------------*//**
 @description Returns true if the transform is a uniform scale combined with
       a rotation.  One, but not both, of the two steps may be null (unit scale or no rotation)

 @param pTransform => The transformation to test
 @param pFixedPoint <= fixed point of scaling.  This is also a point on the
               line.
 @param pDirectionVector <= vector in the direction of the rotation.
 @param pRadians <= rotation angle
 @param pScale <= scale factor.
 @return true if the transformation has at least one of the scale, rotate effects.
 @group "Transform Queries"
 @bsimethod                               EarlinLutz      03/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_isUniformScaleAndRotateAroundLine
(
TransformCP pTransform,
DPoint3dP pFixedPoint,
DPoint3dP pDirectionVector,
double          *pRadians,
double          *pScale
)
    {
    DPoint3d  fixedPoint;
    Transform basis;
    DVec3d directionVector;
    RotMatrix matrix, rotationPart;
    double scale;
    double radians;
    int nullSpaceDimension;
    bool    result = false;
    DPoint3d errorVector;

    bsiTransform_getMatrix (pTransform, &matrix);
    bsiDPoint3d_setXYZ (&directionVector, 0.0, 0.0, 1.0);
    bsiDPoint3d_zero (&fixedPoint);
    radians = 0.0;

    if (    bsiRotMatrix_isRigidScale (&matrix, &rotationPart, &scale)
        && !bsiRotMatrix_isIdentity (&matrix)
        && bsiTransform_invariantSpaceBasis (pTransform, &basis, &nullSpaceDimension,
                    NULL, NULL, &errorVector, 0.0)
        )
        {
        bsiTransform_getTranslation (&basis, &fixedPoint);
        radians = bsiRotMatrix_getRotationAngleAndVector (&rotationPart, &directionVector);
        result = true;
        }

    if (pFixedPoint)
        *pFixedPoint = fixedPoint;
    if (pDirectionVector)
        *pDirectionVector = directionVector;
    if (pRadians)
        *pRadians = radians;
    if (pScale)
        *pScale = scale;
    return result;
    }


/*-----------------------------------------------------------------*//**
 @description Compute any single point that remains unchanged by action of
       a transform.   Note that a pure translation has no fixed points,
       while any other transformation does.
 @param pTransform => The transformation to test
 @param pFixedPoint <= Point that is not changed by the transformation.
 @return true if the transformation has a fixed point.
 @group "Transform Queries"
 @bsimethod                               EarlinLutz      03/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_getAnyFixedPoint
(
TransformCP pTransform,
DPoint3dP pFixedPoint
)
    {
    Transform fixedSpace;
    int nullSpaceDimension;
    bool    boolstat = bsiTransform_invariantSpaceBasis
                    (
                    pTransform,
                    &fixedSpace, &nullSpaceDimension,
                    NULL, NULL,
                    NULL,
                    0.0
                    );
    if (!boolstat)
        {
        if (pFixedPoint)
            bsiDPoint3d_zero (pFixedPoint);
        }
    else
        {
        if (pFixedPoint)
            bsiTransform_getTranslation (&fixedSpace, pFixedPoint);
        }
    return boolstat;
    }


/*-----------------------------------------------------------------*//**
 @description Compute the line (if any) of points that are not affected by
       this transformation.  Returns false if the fixed point set for the
       transform is empty, a single point, a plane, or all points.
 @param pTransform => The transformation to test
 @param pFixedPoint <= A point on the line.
 @param pDirectionVector <= vector along the line.
 @return true if the transformation has a fixed point.
 @group "Transform Queries"
 @bsimethod                               EarlinLutz      03/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_getFixedLine
(
TransformCP pTransform,
DPoint3dP pFixedPoint,
DPoint3dP pDirectionVector
)
    {
    Transform fixedSpace;
    int nullSpaceDimension;
    bool    boolstat = bsiTransform_invariantSpaceBasis
                    (
                    pTransform,
                    &fixedSpace, &nullSpaceDimension,
                    NULL, NULL,
                    NULL, 0.0
                    );
    if (boolstat && nullSpaceDimension == 1)
        {
        if (pFixedPoint)
            bsiTransform_getTranslation (&fixedSpace, pFixedPoint);
        if (pDirectionVector)
            bsiTransform_getMatrixColumn (&fixedSpace, pDirectionVector, 0);
        }
    else
        {
        if (pFixedPoint)
            bsiDPoint3d_zero (pFixedPoint);
        if (pDirectionVector)
            bsiDPoint3d_setXYZ (pDirectionVector, 1.0, 0.0, 0.0);
        }
    return boolstat;
    }


/*-----------------------------------------------------------------*//**
 @description Compute the plane (if any) of points that are not affected by
       this transformation.  Returns false if the fixed point set for the
       transform is empty, a single point, a line, or all points.
 @param pTransform => The transformation to test
 @param pFixedPoint <= A point on the line.
 @param pPlaneVectorX <= a unit vector in the plane.
 @param pPlaneVectorY <= another unit vector in the plane,
               perpendicular to pDirectionVectorX.
 @return true if the transformation has a fixed point.
 @group "Transform Queries"
 @bsimethod                               EarlinLutz      03/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_getFixedPlane
(
TransformCP pTransform,
DPoint3dP pFixedPoint,
DPoint3dP pPlaneVectorX,
DPoint3dP pPlaneVectorY
)
    {
    Transform fixedSpace;
    int nullSpaceDimension;
    bool    boolstat = bsiTransform_invariantSpaceBasis
                    (
                    pTransform,
                    &fixedSpace, &nullSpaceDimension,
                    NULL, NULL,
                    NULL, 0.0
                    );

    if (boolstat && nullSpaceDimension == 1)
        {
        if (pFixedPoint)
            bsiTransform_getTranslation (&fixedSpace, pFixedPoint);
        if (pPlaneVectorX)
            bsiTransform_getMatrixColumn (&fixedSpace, pPlaneVectorX, 0);
        if (pPlaneVectorY)
            bsiTransform_getMatrixColumn (&fixedSpace, pPlaneVectorY, 1);
        }
    else
        {
        if (pFixedPoint)
            bsiDPoint3d_zero (pFixedPoint);
        if (pPlaneVectorX)
            bsiDPoint3d_setXYZ (pPlaneVectorX, 1.0, 0.0, 0.0);
        if (pPlaneVectorY)
            bsiDPoint3d_setXYZ (pPlaneVectorY, 1.0, 0.0, 0.0);
        }

    return boolstat;
    }


/*-----------------------------------------------------------------*//**
 @description Factor a combination of a mirror part and a non-mirror part,
           attempting to use a common fixed point for the two parts.
 Equationally, the old transform T0 becomes
<pre>
            T0 = T1 * M
</pre>
 where M is the mirror, and T1 is a mirror-free transform.
 The mirror transform is returned in both matrix form and as plane point with normal.
 In an order of operations view, a point X is transformed as
<pre>
            T0 * X = T1 * M * X
</pre>
 That is, X mirrored first, followed by rotation and scaling in the residual transform.

 @param pTransform => The transformation to test
 @param pResidualTransform <= the residual transform.
 @param pMirrorTransform <= the mirror transform.
 @param pPlanePoint <= A fixed point of the mirror transform.
 @param pPlaneNormal <= Unit normal for the mirror plane.
 @return false if the transform has no mirror effects.  In this case the mirror transform is the identity,
       the residual transform is the original transform, and the fixed point and normal are both zero.
 @group "Transform Queries"
 @bsimethod                               EarlinLutz      03/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_factorMirror
(
TransformCP pTransform,
TransformP pResidualTransform,
TransformP pMirrorTransform,
DPoint3dP pPlanePoint,
DPoint3dP pPlaneNormal
)
    {
    bool    boolstat = false;
    RotMatrix matrix;
    DPoint3d planePoint;
    DPoint3d planeNormal;
    Transform mirrorTransform;
    Transform residualTransform;
    double det;

    bsiTransform_getMatrix (pTransform, &matrix);
    det = bsiRotMatrix_determinant (&matrix);

    if (det >= 0.0)
        {
        bsiTransform_initIdentity (&mirrorTransform);
        residualTransform = *pTransform;
        bsiDPoint3d_zero (&planePoint);
        bsiDPoint3d_zero (&planeNormal);
        boolstat = false;
        }
    else if (bsiTransform_isMirrorAboutPlane (pTransform, &planePoint, &planeNormal))
        {
        bsiTransform_initIdentity (&residualTransform);
        mirrorTransform = *pTransform;
        bsiDPoint3d_zero (&planePoint);
        bsiDPoint3d_zero (&planeNormal);
        boolstat = true;
        }
    else if (bsiTransform_getAnyFixedPoint (pTransform, &planePoint))
        {
        bsiDPoint3d_setXYZ (&planeNormal, 0.0, 0.0, 1.0);
        bsiTransform_initFromMirrorPlane (&mirrorTransform, &planePoint, &planeNormal);
        // Mirror transform is its own inverse, so
        // T = T * M * M, and (T*M) has positive determinant and is the residual.
        bsiTransform_multiplyTransformTransform (&residualTransform, pTransform, &mirrorTransform);
        boolstat = true;
        }
    else
        {
        // No fixed point for the whole transform. (Is this even possible?)  Just mirror around origin.
        bsiDPoint3d_zero (&planePoint);
        bsiDPoint3d_setXYZ (&planeNormal, 0.0, 0.0, 1.0);
        bsiTransform_initFromMirrorPlane (&mirrorTransform, &planePoint, &planeNormal);
        // Mirror transform is its own inverse, so
        // T = T * M * M, and (T*M) has positive determinant and is the residual.
        bsiTransform_multiplyTransformTransform (&residualTransform, pTransform, &mirrorTransform);
        boolstat = true;
        }

    if (pResidualTransform)
        *pResidualTransform = residualTransform;
    if (pMirrorTransform)
        *pMirrorTransform = mirrorTransform;
    if (pPlanePoint)
        *pPlanePoint = planePoint;
    if (pPlaneNormal)
        *pPlaneNormal = planeNormal;

    return boolstat;
    }

/*-----------------------------------------------------------------*//**
@description Compute the composite transformation
    Transform = Xlate(putdownPoint) . ( putdownAxes . Inverse (pickupAxes) ) . S (s,s,s) . Xlate(-pickupPoint)
In common usage, geometry flows from right to left as follows:
<ul>
<li>Treat the pickup point as the origin.</li>
<li>Scale about the (local) origin.</li>
<li>Using the same origin, Rotate from the pickup axes to the putdown axes.</li>
<li>Move the pickup point to the putdown point.</li>
</ul>

@param pResult OUT composite transformation.
@param pPutdownPoint IN putdown point
@param pPutdownAxes IN matrix whose columns are axes of putdown orientation.  Usually, but not necessarily,
    a rotation.
@param pScale IN scale factor to apply.
@param pPickupAxes IN matrix whose columns are axes of pickup orientation.  Usually, but not necessarily,
    a rotation.
@param pPickupPoint IN pickup origin
@return false if pPickupAxes was not invertible.  In this case the computation completes using the global
principal axes as pickup axes.
 @group "Transform Initialization"
 @bsimethod                               EarlinLutz      03/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTransform_composeLocalOriginOperations
(
TransformP pResult,
DPoint3dCP pPutdownPoint,
RotMatrixCP pPutdownAxes,
const double    *pScale,
RotMatrixCP pPickupAxes,
DPoint3dCP pPickupPoint
)
    {
    Transform        _temp;
    bool    boolstat = true;
    RotMatrix inversePickup;

    if (pResult == NULL)
        pResult = &_temp;

    /* Build incrementally from left to right. */
    if (pPutdownPoint)
        bsiTransform_initFromTranslationXYZ (pResult, pPutdownPoint->x, pPutdownPoint->y, pPutdownPoint->z);
    else
        bsiTransform_initIdentity (pResult);
    if (pPutdownAxes)
        bsiTransform_multiplyTransformRotMatrix (pResult, pResult, pPutdownAxes);

    if (pPickupAxes)
        {
        if (bsiRotMatrix_invertRotMatrix (&inversePickup, pPickupAxes))
            bsiTransform_multiplyTransformRotMatrix (pResult, pResult, &inversePickup);
        else
            boolstat = false;
        }

    if (pScale)
        bsiTransform_scaleMatrixColumns (pResult, pResult, *pScale, *pScale, *pScale);

    if (pPickupPoint)
        {
        Transform   toZero;
        bsiTransform_initFromTranslationXYZ (&toZero, - pPickupPoint->x, - pPickupPoint->y, - pPickupPoint->z);
        bsiTransform_multiplyTransformTransform (pResult, pResult, &toZero);
        }

    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* @description Construct a transform which flattens to a plane.
* @param pTransform  <= flatten transform
* @param pOrigin => any point on plane
* @param pNormal => normal vector
* @return true if the normal is nonzero
* @group "Transform Initialization"
* @bsimethod                                                    David.Assaf     09/07
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiTransform_initFromFlattenPlane
(
TransformP  pTransform,
DPoint3dCP  pOrigin,
DVec3dCP    pNormal
)
    {
    if (!pTransform || !pNormal)
        return false;

    // The projection of vector x onto the subspace perpendicular to n is
    //      x - (n.x)n = x - n (n.x) = x - n (n^x) = x - (nn^)x,
    // so the matrix operator we seek is I - nn^.   For general planes with
    // normal n and containing point p, we sandwich this operator between
    // translations to/from the origin:
    //      (I - nn^)(x-p) + p = (I - nn^)x + (nn^)p = (I - nn^)x + n (n.p).
    // Note the similarity to bsiTransform_initFromMirrorPlane, which just
    // doubles the length of the subtracted vector in the projection operator
    // to punch the vector all the way through to the other side of the plane!

    DVec3d  unitNormal;
    double  magnitude = bsiDVec3d_normalize (&unitNormal, pNormal);
    if (magnitude <= 0.0)
        {
        bsiTransform_initIdentity (pTransform);
        return false;
        }

    RotMatrix   matrixPart;
    bsiRotMatrix_initIdentity (&matrixPart);
    bsiRotMatrix_addScaledOuterProductInPlace (&matrixPart, &unitNormal, &unitNormal, -1.0);

    DPoint3d    translationPart;
    bsiDPoint3d_scale (&translationPart, &unitNormal, bsiDVec3d_dotProductDVec3dDPoint3d (&unitNormal, pOrigin));
    bsiTransform_initFromMatrixAndTranslation (pTransform, &matrixPart, &translationPart);
    return true;
    }

END_BENTLEY_NAMESPACE
