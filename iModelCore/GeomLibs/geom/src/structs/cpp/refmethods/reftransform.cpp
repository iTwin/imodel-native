/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/cpp/refmethods/reftransform.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE

/*-----------------------------------------------------------------*//**
* @description returns an identity transformation, i.e. zero
* translation part and identity matrix part.
*
+----------------------------------------------------------------------*/
Transform Transform::FromIdentity
(
)
    {
    Transform transform;
    transform.InitIdentity ();
    return transform;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a transformation with the given matrix part and a zero
*           translation part.
* @param [in] matrix The matrix part
+----------------------------------------------------------------------*/
Transform Transform::From
(
RotMatrixCR     matrix
)
    {
    Transform transform;
    transform.InitFrom (matrix);
    return transform;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a transformation with the given 2d origin and xVector.
*   The y vector is a a 90 degree CCW rotation of the xVector.  The zVector is unit Z
+----------------------------------------------------------------------*/
Transform Transform::FromOriginAndXVector
(
DPoint2dCR origin, 
DVec2dCR xVector
)
    {
    double x = xVector.x;
    double y = xVector.y;
    return Transform::FromRowValues
        (
        x, -y, 0, origin.x,
        y,  x, 0, origin.y,
        0,  0, 1, 0.0
        );
    }

//! @description Returns a transformation with
//!<ul>
//!<li>translation xyz are the origin
//!<li>xAxis in direction of bearing radians (parallel to xy plane)
//!<li>yAxis perpenedicular to xAxis and also parallel to the xy plane.
//!<li>zAxis is global (0,0,1)
//!<ul>
Transform Transform::FromOriginAndBearingXY
(
DPoint3dCR origin, 
double bearingRadians
)
    {
    double x = cos (bearingRadians);
    double y = sin (bearingRadians);
    return Transform::FromRowValues
        (
        x, -y, 0, origin.x,
        y,  x, 0, origin.y,
        0,  0, 1, origin.z
        );
    }

#ifndef MinimalRefMethods
/*-----------------------------------------------------------------*//**
* @description Returns a transformation with the given matrix part and a zero
*           translation part.
* @param [in] matrix The matrix part
+----------------------------------------------------------------------*/
bool Transform::TryRangeMapping (DRange2dCR sourceRange, DRange2dCR destRange, TransformR transform)
    {
    double sx, sy, ax, ay;
    transform.InitIdentity ();

    if (   !sourceRange.IsNull ()
        && !destRange.IsNull ()
        && DoubleOps::LinearTransform (sourceRange.low.x, sourceRange.high.x, destRange.low.x, destRange.high.x, ax, sx)
        && DoubleOps::LinearTransform (sourceRange.low.y, sourceRange.high.y, destRange.low.y, destRange.high.y, ay, sy)
        )
        {
        transform.InitFromRowValues (
                sx, 0, 0, ax,
                0, sy, 0, ay,
                0,  0, 1, 0
                );
        return true;
        }
    return false;
    }

/*-----------------------------------------------------------------*//**
+----------------------------------------------------------------------*/
bool Transform::TryUniformScaleXYRangeFit (DRange3dCR rangeA, DRange3dCR rangeB, TransformR AToB, TransformR BToA)
    {
    if (   !rangeA.IsNull ()
        && !rangeB.IsNull ()
        )
        {
        DPoint3d centerA = DPoint3d::FromInterpolate (rangeA.low, 0.5, rangeA.high);
        DPoint3d centerB = DPoint3d::FromInterpolate (rangeB.low, 0.5, rangeB.high);
        DVec3d diagonalA = DVec3d::FromStartEnd (rangeA.low, rangeA.high);
        DVec3d diagonalB = DVec3d::FromStartEnd (rangeB.low, rangeB.high);
        double scaleX, scaleY;
        DoubleOps::SafeDivide (scaleX, diagonalB.x, diagonalA.x, 1.0);
        DoubleOps::SafeDivide (scaleY, diagonalB.y, diagonalA.y, 1.0);
        double s = DoubleOps::Min (scaleX, scaleY);
        if (s == 0.0)
            s = 1.0;

        AToB.InitFromRowValues (
                s, 0, 0, centerB.x - s * centerA.x,
                0, s, 0, centerB.y - s * centerA.y,
                0,  0, s, centerB.z - s * centerA.z
                );
        double q;
        DoubleOps::SafeDivide (q, 1.0, s, 1.0);
        BToA.InitFromRowValues (
                q, 0, 0, centerA.x - q * centerB.x,
                0, q, 0, centerA.y - q * centerB.y,
                0,  0, q, centerA.z - q * centerB.z
                );                
        return true;
        }
    AToB.InitIdentity ();
    BToA.InitIdentity ();
    return false;
    }


#endif
/*-----------------------------------------------------------------*//**
* @description Returns a transformation with the given matrix and translation parts.
* @param [in] matrix The matrix part
* @param [in] translation The translation part
+----------------------------------------------------------------------*/
Transform Transform::From
(
RotMatrixCR     matrix,
DPoint3dCR      translation
)
    {
    Transform transform;
    transform.InitFrom (matrix, translation);
    return transform;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a transformation with identity matrix part and
*           given translation part.
* @param [in] translation The translation part
+----------------------------------------------------------------------*/
Transform Transform::From
(
DPoint3dCR      translation
)
    {
    Transform transform;
    transform.InitFrom (translation);
    return transform;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a transformation with identity matrix part and
*       translation part given as x, y, and z components.
* @param [in] x The x-coordinate of translation part
* @param [in] y The y-coordinate of translation part
* @param [in] z The z-coordinate of translation part
+----------------------------------------------------------------------*/
Transform Transform::From
(
double          x,
double          y,
double          z
)
    {
    Transform transform;
    transform.InitFrom (x, y, z);
    return transform;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a transformation copying the double values directly into the rows of this instance.
*
* @param [in] x00 The (0,0) entry of the matrix (row, column)
* @param [in] x01 The (0,1) entry
* @param [in] x02 The (0,2) entry
* @param [in] tx The x-coordinate of the translation part
* @param [in] x10 The (1,0) entry
* @param [in] x11 The (1,1) entry
* @param [in] x12 The (1,2) entry
* @param [in] ty The y-coordinate of the translation part
* @param [in] x20 The (2,0) entry
* @param [in] x21 The (2,1) entry
* @param [in] x22 The (2,2) entry
* @param [in] tz The z-coordinate of the translation part
+----------------------------------------------------------------------*/
Transform Transform::FromRowValues
(
double          x00,
double          x01,
double          x02,
double          tx,
double          x10,
double          x11,
double          x12,
double          ty,
double          x20,
double          x21,
double          x22,
double          tz
)
    {
    Transform transform;
    transform.InitFromRowValues (x00, x01, x02, tx, x10, x11, x12, ty, x20, x21, x22, tz);
    return transform;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a transformation of rotation about a specified line.
* @param [in] point0 The start point of the line
* @param [in] point1 The end point of the line
* @param [in] radians The rotation angle
+----------------------------------------------------------------------*/
Transform Transform::FromLineAndRotationAngle
(
DPoint3dCR      point0,
DPoint3dCR      point1,
double          radians
)
    {
    Transform transform;
    transform.InitFromLineAndRotationAngle (point0, point1, radians);
    return transform;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
Transform Transform::FromAxisAndRotationAngle
(
DRay3dCR    axis,
double      radians,
TransformR  derivativeTransform
)
    {
    Transform transform;
    RotMatrix matrix, derivativeMatrix;
    matrix = RotMatrix::FromVectorAndRotationAngle (axis.direction, radians, derivativeMatrix);

    
    /* We count on the vector rotation returning a valid identity matrix even if
        the vector is null */
    transform.InitFromMatrixAndFixedPoint (matrix, axis.origin);

    DVec3d vectorW;
    derivativeMatrix.MultiplyComponents (vectorW, -axis.origin.x, -axis.origin.y, -axis.origin.z);
    derivativeTransform.InitFrom (derivativeMatrix, vectorW);
    return transform;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
Transform Transform::FromAxisAndRotationAngle
(
DRay3dCR    axis,
double      radians
)
    {
    Transform transform;
    RotMatrix matrix;
    matrix = RotMatrix::FromVectorAndRotationAngle (axis.direction, radians);
    /* We count on the vector rotation returning a valid identity matrix even if
        the vector is null */
    transform.InitFromMatrixAndFixedPoint (matrix, axis.origin);
    return transform;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a transformation with given matrix part, and translation
*   part computed from the matrix and a given fixed point.  This translation part
*   is generally different from the fixed point itself.   The resulting transformation
*   will leave the fixed point unchanged and apply whatever effects are contained in the
*   matrix as if the fixed point is the origin.
* @param [in] matrix The matrix part
* @param [in] origin The point that is to remain fixed when multiplied by the transformation.
+----------------------------------------------------------------------*/
Transform Transform::FromMatrixAndFixedPoint
(
RotMatrixCR     matrix,
DPoint3dCR      origin
)
    {
    Transform transform;
    transform.InitFromMatrixAndFixedPoint (matrix, origin);
    return transform;
    }
/*-----------------------------------------------------------------*//**
* Sets this instance to the transformation obtained by premultiplying
* inTransform by 3 matrix rotations about principle axes, given by the
* angles xrot, yrot and zrot.
* inTransform may be the same as this instance.
* Symbolically, given transform [M t] and rotation
* matrices X,Y,Z, the resulting transform is [X*Y*Z*M X*Y*Z*t]
*
* @param [in] inTransform The base transformation
* @param [in] xrot The x axis rotation, in radians
* @param [in] yrot The y axis rotation, in radians
* @param [in] zrot The z axis rotation, in radians
+----------------------------------------------------------------------*/
Transform Transform::FromPrincipleAxisRotations
(
TransformCR     inTransform,
double          xrot,
double          yrot,
double          zrot
)
    {
    Transform transform;
    transform.InitFromPrincipleAxisRotations (inTransform, xrot, yrot, zrot);
    return transform;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a transformation with origin at origin, x-axis
* xVector, y-axis yVector, and z-axis zVector.
* All axes are unnormalized.
* There is no effort to detect zero length axes or degenerate points that
* define only a line or plane but not a full coordinate system.
* The axes may be skewed.
*
* @param [in] origin The origin of transformed coordinates
* @param [in] xVector The 100 point of transformed coordinates
* @param [in] yVector The 010 point of transformed coordinates
* @param [in] zVector The 001 point of transformed coordinates
+----------------------------------------------------------------------*/
Transform Transform::FromOriginAndVectors
(
DPoint3dCR      origin,
DVec3dCR      xVector,
DVec3dCR      yVector,
DVec3dCR      zVector
)
    {
    Transform transform;
    transform.InitFromOriginAndVectors (origin, xVector, yVector, zVector);
    return transform;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a transformation with origin at origin, x-axis from
* origin to xPoint, y-axis from origin to yPoint,
* and z-axis from origin to zPoint.
* All axes are unnormalized.
* There is no effort to detect zero length axes or degenerate points that
* define only a line or plane but not a full coordinate system.
* The axes may be skewed.
*
* @param [in] origin The origin of transformed coordinates
* @param [in] xPoint The 100 point of transformed coordinates
* @param [in] yPoint The 010 point of transformed coordinates
* @param [in] zPoint The 001 point of transformed coordinates
+----------------------------------------------------------------------*/
Transform Transform::From4Points
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
DPoint3dCR      yPoint,
DPoint3dCR      zPoint
)
    {
    Transform transform;
    transform.InitFrom4Points (origin, xPoint, yPoint, zPoint);
    return transform;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a transformation with origin at origin, x-axis from
* origin to xPoint, y-axis from origin to yPoint,
* and z-axis equal to the cross product of x and y axes.
* All axes are unnormalized.
* There is no effort to detect zero length axes or degenerate points that
* define only a line or plane but not a full coordinate system.

* @param [in] origin The origin of coordinate system
* @param [in] xPoint The 100 point of coordinate system
* @param [in] yPoint The 010 point of coordinate system
+----------------------------------------------------------------------*/
Transform Transform::FromPlaneOf3Points
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
DPoint3dCR      yPoint
)
    {
    Transform transform;
    transform.InitFromPlaneOf3Points (origin, xPoint, yPoint);
    return transform;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a transformation with x,y,z scales around a fixed point.

* @param [in] origin The fixed point.
* @param [in] xScale x direction scale factor.
* @param [in] yScale y direction scale factor.
* @param [in] zScale z direction scale factor.
+----------------------------------------------------------------------*/
Transform Transform::FromFixedPointAndScaleFactors
(
DPoint3dCR      origin,
double          xScale,
double          yScale,
double          zScale
)
    {
    return FromRowValues (
        xScale, 0, 0, (1.0 - xScale) * origin.x,
        0, yScale, 0, (1.0 - yScale) * origin.y,
        0, 0, zScale, (1.0 - zScale) * origin.z
        );
    }


/*-----------------------------------------------------------------*//**
* @description Returns a (possibly skewed) transformation with origin
* origin, the axis axisId towards xPoint, and other axes perpendicular.
* If normalize is false, all axes have length equal to the distance
* between the two origin and xPoint.
* The axes may be skewed.
*
* @param [in] origin The origin of coordinate system
* @param [in] xPoint The target point of axis axisId of coordinate system
* @param [in] axisId The axis that points from origin to xPoint
* @param [in] normalize true to have coordinate system normalized
+----------------------------------------------------------------------*/
Transform Transform::FromPlaneNormalToLine
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
int             axisId,
bool            normalize
)
    {
    Transform transform;
    transform.InitFromPlaneNormalToLine (origin, xPoint, axisId, normalize);
    return transform;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool Transform::InitFromMirrorPlane (DPoint3dCR origin, DVec3dCR normal)
    {
    return InitFromScalePerpendicularToPlane (origin, normal, -1.0); 
    }
    
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool Transform::InitFromProjectionToPlane (DPoint3dCR origin, DVec3dCR normal)
    {
    return InitFromScalePerpendicularToPlane (origin, normal, 0.0);
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2012
+--------------------------------------------------------------------------------------*/
bool Transform::IsNearRigidScale (TransformR dest, int primaryAxis, double tolerance) const
    {
    dest = *this;
    RotMatrix Q, Q1;
    this->GetMatrix (Q);
    if (!Q.IsNearRigidScale (Q1, primaryAxis, tolerance))
        return false;
    dest.SetMatrix (Q1);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool Transform::InitFromScalePerpendicularToPlane (DPoint3dCR origin, DVec3dCR normal, double scale)
    {
    DVec3d unitNormal;
    bool    boolstat;
    double magnitude = unitNormal.Normalize (normal);
    double phi = 1.0 - scale;
    RotMatrix phiNNT = RotMatrix::FromScaledOuterProduct (unitNormal, unitNormal, phi);
    RotMatrix reflector = RotMatrix::FromIdentity ();
    reflector.AddScaledOuterProductInPlace (unitNormal, unitNormal, -phi);
    DPoint3d B;
    phiNNT.Multiply (B, origin);
    if (magnitude == 0.0)
        {
        InitIdentity ();
        boolstat = false;
        }
    else
        {
        InitFrom (reflector, B);
        boolstat = true;
        }

    return boolstat;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/2012
+--------------------------------------------------------------------------------------*/
bool Transform::InitForwardAndInverseFromAxesAndOrigin
(
TransformR localToWorld,
TransformR worldToLocal,
RotMatrixCR axes,
DPoint3dCR origin
)
    {
    localToWorld.InitFrom (axes, origin);
    return worldToLocal.InverseOf (localToWorld);
    }


/*-----------------------------------------------------------------*//**
* @description Returns a transformation in the xy-plane
* with origin origin, axis axisId towards xPoint, and the other axis
* perpendicular.
* If normalize is false, both axes have length equal to the distance
* between origin and xPoint.
*
* @param [in] origin The origin of coordinate system (or null)
* @param [in] xPoint The target point of axis axisId of coordinate system
* @param [in] axisId The axis (x=0, y=1) that points from origin to xPoint
* @param [in] normalize true to have coordinate system normalized
+----------------------------------------------------------------------*/
Transform Transform::From2Points
(
DPoint2dCR      origin,
DPoint2dCR      xPoint,
int             axisId,
bool            normalize
)
    {
    Transform transform;
    transform.InitFrom2Points (origin, xPoint, axisId, normalize);
    return transform;
    }

//! @description Returns the product of two transforms.
Transform Transform::FromProduct (TransformCR transformA, TransformCR transformB)
    {
    Transform result;
    result.InitProduct (transformA, transformB);
    return result;
    }

//! @description Returns the product of two transforms.
Transform Transform::FromProduct (TransformCR transformA, TransformCR transformB, TransformCR transformC)
    {
    Transform AB, ABC;
    AB.InitProduct (transformA, transformB);
    ABC.InitProduct (AB, transformC);
    return ABC;
    }



//! @description Returns the product of two transforms.
Transform Transform::FromProduct (RotMatrixCR matrixA, TransformCR transformB)
    {
    Transform result;
    result.InitProduct (matrixA, transformB);
    return result;
    }

//! @description Returns the product of two transforms.
Transform Transform::FromProduct (TransformCR transformA, RotMatrixCR matrixB)
    {
    Transform result;
    result.InitProduct (transformA, matrixB);
    return result;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a value from a specified row and column of the matrix part of the transformation.
* @param [in] row The index of row to read.  Row indices are 0, 1, 2.
* @param [in] col The index of column to read.  Column indices are 0, 1, 2.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double Transform::GetFromMatrixByRowAndColumn (int row, int col) const
    {
    row = Angle::Cyclic3dAxis (row);
    col = Angle::Cyclic3dAxis (col);
    return form3d[row][col];
    }

void Transform::SetMatrixByRowAndColumn (int row, int col, double value) 
    {
    row = Angle::Cyclic3dAxis (row);
    col = Angle::Cyclic3dAxis (col);
    form3d[row][col] = value;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a value from a specified component of the point (translation) part of the transformation.
* @param [in] row The index of point component to read.  Indices are 0, 1, 2 for x, y, z
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double Transform::GetPointComponent
(

      int            row

) const
    {
    row = Angle::Cyclic3dAxis (row);
    return form3d[row][3];
    }



/*-----------------------------------------------------------------*//**
* @description returns an identity transformation, i.e. zero
* translation part and identity matrix part.
*
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitIdentity
(

)
    {
    memset (this, 0, sizeof (Transform));
    form3d[0][0] =
    form3d[1][1] =
    form3d[2][2] = 1.0;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a transformation with the given matrix part and a zero
*           translation part.
* @param [in] matrix The matrix part
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitFrom (RotMatrixCR matrix)
    {
    int j;
    for (j = 0; j < 3; j++      )
        {
        form3d[0][j] = matrix.form3d[0][j];
        form3d[1][j] = matrix.form3d[1][j];
        form3d[2][j] = matrix.form3d[2][j];
        }

    form3d[0][3] =
    form3d[1][3] =
    form3d[2][3] = 0.0;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a transformation with the given matrix part and a zero
*           translation part.
* @param [in] matrix The matrix part
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitFromScaleFactors
(
double xScale,
double yScale,
double zScale
)
    {
    *this = Transform::FromScaleFactors (xScale, yScale, zScale);
    }

/*-----------------------------------------------------------------*//**
* @description Returns a transformation with the given matrix part and a zero
*           translation part.
* @param [in] matrix The matrix part
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
Transform Transform::FromScaleFactors
(
double xScale,
double yScale,
double zScale
)
    {
    return Transform::FromRowValues
                (
                xScale, 0,0,0,
                0, yScale,0,0,
                0,0, zScale,0
                );
    }                



/*-----------------------------------------------------------------*//**
* @description Returns a transformation with the given matrix and translation parts.
* @param [in] matrix The matrix part
* @param [in] translation The translation part
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitFrom
(

RotMatrixCR matrix,
DPoint3dCR translation

)
    {
    this->InitFrom (matrix);
       

    form3d[0][3] = translation.x;
    form3d[1][3] = translation.y;
    form3d[2][3] = translation.z;

    }


/*-----------------------------------------------------------------*//**
* @description Returns a transformation with identity matrix part and
*           given translation part.
* @param [in] translation The translation part
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitFrom
(

DPoint3dCR translation

)
    {
    this->InitIdentity ();

    form3d[0][3] = translation.x;
    form3d[1][3] = translation.y;
    form3d[2][3] = translation.z;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a transformation with identity matrix part and
*       translation part given as x, y, and z components.
* @param [in] x The x-coordinate of translation part
* @param [in] y The y-coordinate of translation part
* @param [in] z The z-coordinate of translation part
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitFrom
(

      double        x,
      double        y,
      double        z

)
    {
    this->InitIdentity ();
    form3d[0][3] = x;
    form3d[1][3] = y;
    form3d[2][3] = z;
    }


/*-----------------------------------------------------------------*//**
* Copies the double values directly into the rows of this instance.
*
* @param [in] x00 The (0,0) entry of the matrix (row, column)
* @param [in] x01 The (0,1) entry
* @param [in] x02 The (0,2) entry
* @param [in] tx The x-coordinate of the translation part
* @param [in] x10 The (1,0) entry
* @param [in] x11 The (1,1) entry
* @param [in] x12 The (1,2) entry
* @param [in] ty The y-coordinate of the translation part
* @param [in] x20 The (2,0) entry
* @param [in] x21 The (2,1) entry
* @param [in] x22 The (2,2) entry
* @param [in] tz The z-coordinate of the translation part
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitFromRowValues
(

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
    form3d[0][0] = x00;
    form3d[1][0] = x10;
    form3d[2][0] = x20;
    form3d[0][3] = tx;

    form3d[0][1] = x01;
    form3d[1][1] = x11;
    form3d[2][1] = x21;
    form3d[1][3] = ty;

    form3d[0][2] = x02;
    form3d[1][2] = x12;
    form3d[2][2] = x22;
    form3d[2][3] = tz;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a transformation of rotation about a specified line.
* @param [in] point0 The start point of the line
* @param [in] point1 The end point of the line
* @param [in] radians The rotation angle
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitFromLineAndRotationAngle
(

DPoint3dCR point0,
DPoint3dCR point1,
      double         radians

)
    {
    DVec3d vector;
    RotMatrix matrix;
    vector.DifferenceOf (point1, point0);
    matrix.InitFromVectorAndRotationAngle (vector, radians);
    /* We count on the vector rotation returning a valid identity matrix even if
        the vector is null */
    this->InitFromMatrixAndFixedPoint (matrix, point0);
    assert (DoubleOps::AlmostEqual (Determinant (), 1.0));
    }


/*-----------------------------------------------------------------*//**
* Sets this instance to a transformation that has the same matrix part
* as transform in and a translation part that is the SUM of the
* translation part of in plus the product of the
* matrix part of in times the given point.
* If the translation part of in is interpreted as the
* origin of a coordinate system (whose axis directions and sizes
* are given by the columns of the matrix part), this instance
* becomes a coordinate frame with the same axis directions and sizes,
* but with the origin shifted to point x,y,z of the in
* system.  That is, x,y,z are the local coordinates of the new
* origin, and the translation part of this instance becomes the global
* coordinates of the new origin.
* in may be identical to this instance.
* Symbolically, if in is the transform [R t] and the local
* origin coordinates x,y,z are in column vector p, the result is
* the transformation [R t+R*p].
*
* @param [in] in The input transformation
* @param [in] x The x-coordinate of the local origin
* @param [in] y The y-coordinate of the local origin
* @param [in] z The z-coordinate of the local origin
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::TranslateInLocalCoordinates
(
TransformCR in,
      double         x,
      double         y,
      double         z
)
    {
    DPoint3d point;
    point.x = x;
    point.y = y;
    point.z = z;
    in.Multiply (point);
    *this = in;
    SetTranslation (point);
    }


/*-----------------------------------------------------------------*//**
* @description Overwrites the matrix part of a preexisting transformation.
*   The translation part is unchanged.
* @param [in] matrix The matrix to insert
* @bsimethod                                                    DavidAssaf      10/98
+----------------------------------------------------------------------*/
void Transform::SetMatrix (RotMatrixCR matrix)
    {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            form3d[i][j] = matrix.form3d[i][j];
    }


/*-----------------------------------------------------------------*//**
* Sets the translation part of this instance to point.  The prior
* translation part is overwritten, and the matrix part is unchanged.
* Symbolically, if point is u then this instance [R t] becomes the
* transformation [R u].
*
* @param [in] point The vector to insert
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::SetTranslation (DPoint3dCR point)
    {
    form3d[0][3] = point.x;
    form3d[1][3] = point.y;
    form3d[2][3] = point.z;
    }


/*-----------------------------------------------------------------*//**
* Sets the translation part of this instance to zero.  The prior
* translation part is overwritten, and the matrix part is unchanged.
* Symbolically, this instance [R t] becomes the transformation [R 0].
*
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::ZeroTranslation ()
    {
    form3d[0][3] = 0.0;
    form3d[1][3] = 0.0;
    form3d[2][3] = 0.0;
    }


/*-----------------------------------------------------------------*//**
* Sets the translation part of this instance so that it leaves point
* point unchanged, i.e. so that this instance may be interpreted as
* applying its matrix part as a rotation or scaling about point.
* Symbolically, given transform [R t] and column vector p,
* the returned transform is [R p-R*p].
* (The prior translation part is destroyed, and does not affect the
* result in any way.)
*
* @param [in] point The point that is to remain fixed when multiplied by the modified transformation
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::SetFixedPoint
(
DPoint3dCR point
)
    {
    form3d[0][3] = point.x
                - form3d[0][0] * point.x
                - form3d[0][1] * point.y
                - form3d[0][2] * point.z;

    form3d[1][3] = point.y
                - form3d[1][0] * point.x
                - form3d[1][1] * point.y
                - form3d[1][2] * point.z;

    form3d[2][3] = point.z
                - form3d[2][0] * point.x
                - form3d[2][1] * point.y
                - form3d[2][2] * point.z;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a transformation with given matrix part, and translation
*   part computed from the matrix and a given fixed point.  This translation part
*   is generally different from the fixed point itself.   The resulting transformation
*   will leave the fixed point unchanged and apply whatever effects are contained in the
*   matrix as if the fixed point is the origin.
* @param [in] matrix The matrix part
* @param [in] origin The point that is to remain fixed when multiplied by the transformation.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitFromMatrixAndFixedPoint
(

RotMatrixCR matrix,
DPoint3dCR origin

)
    {
    InitFrom (matrix);
    SetFixedPoint (origin);
    }


/*-----------------------------------------------------------------*//**
* Sets this instance to the transformation obtained by premultiplying
* inTransform by 3 matrix rotations about principle axes, given by the
* angles xrot, yrot and zrot.
* inTransform may be the same as this instance.
* Symbolically, given transform [M t] and rotation
* matrices X,Y,Z, the resulting transform is [X*Y*Z*M X*Y*Z*t]
*
* @param [in] inTransform The base transformation
* @param [in] xrot The x axis rotation, in radians
* @param [in] yrot The y axis rotation, in radians
* @param [in] zrot The z axis rotation, in radians
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitFromPrincipleAxisRotations
(

TransformCR inTransform,
double      xrot,
double      yrot,
double      zrot

)
    {
    RotMatrix rotation = RotMatrix::FromIdentity();
    rotation.InitFromPrincipleAxisRotations (rotation, xrot, yrot, zrot);
    this->InitProduct (rotation, inTransform);
    }


/*-----------------------------------------------------------------*//**
* Scale the columns of the matrix part by respective factors.
* Translation part is unaffected.  (See also scaleMatrixRows, scaleTransformRows)
* @param [in] transform The input transform.  If NULL, an identity transform is implied.
* @param [in] xscale The x column scale factor
* @param [in] yscale The y column scale factor
* @param [in] zscale The z column scale factor
* @bsimethod                                                    EarlinLutz      12/00
+----------------------------------------------------------------------*/
void Transform::ScaleMatrixColumns
(

TransformCR transform,
      double         xscale,
      double         yscale,
      double         zscale

)
    {
    *this = transform;
    ScaleMatrixColumns (xscale, yscale, zscale);
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void Transform::ScaleMatrixColumns
(
double         xscale,
double         yscale,
double         zscale
)
    {
    for (int i = 0; i < 3; i++)
        {
        form3d[i][0] *= xscale;
        form3d[i][1] *= yscale;
        form3d[i][2] *= zscale;
        }
    }




/*-----------------------------------------------------------------*//**
* Scale the rows of the matrix part by respective factors.
* Translation part is unaffected.  (See also scaleMatrixColumns, scaleTransformRows)
* @param [in] transform The input transform.  If NULL, an identity transform is implied.
* @param [in] xscale The x column scale factor
* @param [in] yscale The y column scale factor
* @param [in] zscale The z column scale factor
* @bsimethod                                                    EarlinLutz      12/00
+----------------------------------------------------------------------*/
void Transform::ScaleMatrixRows
(
TransformCR transform,
double      xscale,
double      yscale,
double      zscale
)
    {
    for (int i = 0; i < 3; i++)
        {
        form3d[0][i] = transform.form3d[0][i] * xscale;
        form3d[1][i] = transform.form3d[0][i] * yscale;
        form3d[2][i] = transform.form3d[0][i] * zscale;
        }
    }


/*-----------------------------------------------------------------*//**
* Scale the complete rows by respective factors.
* (See also scaleMatrixColumns, scaleMatrixRows, which only act on the matrix part)
* @param [in] transform The input transform.  If NULL, an identity transform is implied.
* @param [in] xscale The x column scale factor
* @param [in] yscale The y column scale factor
* @param [in] zscale The z column scale factor
* @bsimethod                                                    EarlinLutz      12/00
+----------------------------------------------------------------------*/
void Transform::ScaleCompleteRows
(
TransformCR transform,
double xscale,
double yscale,
double zscale
)
    {
    for (int i = 0; i < 4; i++)
        {
        form3d[0][i] = transform.form3d[0][i] * xscale;
        form3d[1][i] = transform.form3d[1][i] * yscale;
        form3d[2][i] = transform.form3d[2][i] * zscale;
        }
    }






/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void Transform::CorrectCoordinateFrameXYRange
(
TransformR localToWorld,
TransformR worldToLocal,
DRange3dR  localRange,
LocalCoordinateSelect frameType,
TransformP appliedTransformOldLocalToNewLocal
)
    {
    DPoint3d localOrigin;
    localToWorld.GetTranslation (localOrigin);
    DVec3d diagonal = DVec3d::FromStartEnd (localRange.low, localRange.high);
    DVec3d originToHigh = DVec3d::FromStartEnd (localOrigin, localRange.high);

    bool lowerLeftOrigin = frameType != LOCAL_COORDINATE_SCALE_UnitAxesAtStart;

    if (NULL != appliedTransformOldLocalToNewLocal)
        appliedTransformOldLocalToNewLocal->InitIdentity ();
    if (lowerLeftOrigin && !DVec3dOps::AlmostEqual (diagonal, originToHigh))
        {
        DVec3d translation = DVec3d::From (localRange.low);
        worldToLocal.MultiplyTranslationTransform (translation, -1.0, worldToLocal);
        localToWorld.MultiplyTransformTranslation (localToWorld, translation, 1.0);
        localRange.low.Zero ();
        localRange.high = diagonal;
        if (appliedTransformOldLocalToNewLocal)
            appliedTransformOldLocalToNewLocal->InitFrom (-translation.x, -translation.y, -translation.z);
        }
    double ax = 1.0;
    double ay = 1.0;
    double bx = 1.0;
    double by = 1.0;
    if (frameType == LOCAL_COORDINATE_SCALE_01RangeBothAxes)
        {
        ax = diagonal.x;
        ay = diagonal.y;
        }
    else if (frameType == LOCAL_COORDINATE_SCALE_01RangeLargerAxis)
        {
        ax = ay = DoubleOps::MaxAbs (diagonal.x, diagonal.y);
        }
    else
        {
        ax = 1.0;
        ay = 1.0;
        }
    
    double az = sqrt (ax * ay);
    double bz = 1.0;
    if (   (ax != 1.0 || ay != 1.0)
        && DoubleOps::SafeDivide (bx, 1.0, ax, 1.0)
        && DoubleOps::SafeDivide (by, 1.0, ay, 1.0)
        && DoubleOps::SafeDivide (bz, 1.0, az, 1.0)
        )
        {
        localToWorld.ScaleMatrixColumns (localToWorld, ax, ay, az);
        worldToLocal.ScaleCompleteRows (worldToLocal, bx, by, bz);
        if (appliedTransformOldLocalToNewLocal)
            appliedTransformOldLocalToNewLocal->ScaleCompleteRows (*appliedTransformOldLocalToNewLocal, bx, by, bz);
        localRange.low.x *= bx;
        localRange.low.y *= by;
        localRange.low.z *= bz;
        localRange.high.x *= bx;
        localRange.high.y *= by;
        localRange.high.z *= bz;
        }
    }



/*-----------------------------------------------------------------*//**
* @description Returns the translation (point) part of a transformation.
*
* @param [out] point vector part of transformation
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::GetTranslation
(

DPoint3dR point

) const
    {
    point.x = form3d[0][3];
    point.y = form3d[1][3];
    point.z = form3d[2][3];
    }

/** Return the matrix part */
RotMatrix Transform::Matrix () const
    {
    return RotMatrix::FromRowValues (
            form3d[0][0], form3d[0][1], form3d[0][2],
            form3d[1][0], form3d[1][1], form3d[1][2],
            form3d[2][0], form3d[2][1], form3d[2][2]
            );
    }


/** Return the origin (aka translation) part as a DPoint3d. */
DPoint3d  Transform::Origin () const
    {
    return DPoint3d::From (form3d[0][3], form3d[1][3], form3d[2][3]);
    }

/** Return the translation (aka origin) part as a DVec3d. */
DVec3d    Transform::Translation () const
    {
    return DVec3d::From (form3d[0][3], form3d[1][3], form3d[2][3]);
    }

/*-----------------------------------------------------------------*//**
* @description Returns a column from the matrix part of the transformation.
*
* @param [out] pPoint column of matrix part.
* @param [in] index
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::GetMatrixColumn
(
DVec3dR column,
int           index

) const
    {
    index = Angle::Cyclic3dAxis (index);
    column.x = form3d[0][index];
    column.y = form3d[1][index];
    column.z = form3d[2][index];
    }

DVec3d Transform::GetMatrixColumn (int index) const
    {
    index = Angle::Cyclic3dAxis (index);
    return DVec3d::From (form3d[0][index], form3d[1][index], form3d[2][index]);
    }

/*-----------------------------------------------------------------*//**
* @description Set  column
*
* @param [out] column data
* @param [in] index
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::SetMatrixColumn
(
DVec3dCR column,
int           index
)
    {
    index = Angle::Cyclic3dAxis (index);
    form3d[0][index] = column.x;
    form3d[1][index] = column.y;
    form3d[2][index] = column.z;
    }

void Transform::SetMatrixRow
(
DVec3dCR row,
int           index
)
    {
    index = Angle::Cyclic3dAxis (index);
    form3d[index][0] = row.x;
    form3d[index][1] = row.y;
    form3d[index][2] = row.z;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a column from the matrix part of the transformation.
*
* @param [out] pPoint column of matrix part.
* @param [in] index
* @bsimethod                                                    EarlinLutz      09/12
+----------------------------------------------------------------------*/
double Transform::MatrixColumnMagnitude (int index) const
    {
    DVec3d column;
    GetMatrixColumn (column, index);
    return column.Magnitude ();
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      09/12
+----------------------------------------------------------------------*/
void Transform::ScaleDoubleArrayByXColumnMagnitude (double *data, int n) const
    {
    if (n > 0)
        DoubleOps::ScaleArray (data, (size_t)n, MatrixColumnMagnitude (0));
    }

/*-----------------------------------------------------------------*//**
* @description Returns a row from the matrix part of the transformation.
*
* @param [out] row row of matrix part.
* @param [in] index
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::GetMatrixRow
(
DVec3dR data,
int           index
) const
    {
    index = Angle::Cyclic3dAxis (index);
    data.x = form3d[index][0];
    data.y = form3d[index][1];
    data.z = form3d[index][2];
    }


/*-----------------------------------------------------------------*//**
* @descriptioin Returns the matrix part of a transformation.
*
* @param [out] matrix matrix part of transformation
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::GetMatrix
(
RotMatrixR matrix
) const
    {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            matrix.form3d[i][j] = form3d[i][j];
    }


/*-----------------------------------------------------------------*//**
* Adds column i of the matrix part of this instance to point in and places
* the result in out.
*
* @param [out] out sum of in and column i
* @param [in] in The base point for sum
* @param [in] i The column index of matrix
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::OffsetPointByColumn
(

DPoint3dR out,
DPoint3dCR in,
        int             i

) const
    {
    out.x = in.x + form3d[0][i];
    out.y = in.y + form3d[1][i];
    out.z = in.z + form3d[2][i];
    }


/*-----------------------------------------------------------------*//**
* Sets point0 to the origin (translation part), and sets point1, point2
* point3 to the x, y and z points (translations of columns
* of matrix part by origin) from this instance.
*
* @param [out] point0 origin of transform coordinates
* @param [out] point1 100 point of transform coordinates
* @param [out] point2 010 point of transform coordinates
* @param [out] point3 001 point of transform coordinates
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Get4Points
(

DPoint3dR point0,
DPoint3dR point1,
DPoint3dR point2,
DPoint3dR point3

) const
    {
    point0.x = form3d[0][3];
    point0.y = form3d[1][3];
    point0.z = form3d[2][3];

    point1.x = point0.x + form3d[0][0];
    point1.y = point0.y + form3d[1][0];
    point1.z = point0.z + form3d[2][0];

    point2.x = point0.x + form3d[0][1];
    point2.y = point0.y + form3d[1][1];
    point2.z = point0.z + form3d[2][1];

    point3.x = point0.x + form3d[0][2];
    point3.y = point0.y + form3d[1][2];
    point3.z = point0.z + form3d[2][2];
    }


/*-----------------------------------------------------------------*//**
* Sets the four points in array to the origin (translation part), x, y and z
* points (translations of columns of matrix part by origin) from this
* instance.
*
* @param [in] pTransform The input transformation
* @param [out] array origin, 100, 010, 001 points as an array
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Get4Points
(

DPoint3dP points

) const
    {
    GetTranslation (points[0]);
    for (int i = 0; i < 3; i++)
        {
        points[i+1].x = points[0].x + form3d[0][i];
        points[i+1].y = points[0].y + form3d[1][i];
        points[i+1].z = points[0].z + form3d[2][i];
        }
    }


/*-----------------------------------------------------------------*//**
* Sets origin to the translation part, and sets vector0, vector1
* vector2 to the columns of this instance.
*
* @param [out] origin origin of transform coordinates
* @param [out] vector0 100 vector of transform coordinates
* @param [out] vector1 010 vector of transform coordinates
* @param [out] vector2 001 vector of transform coordinates
* @indexVerb
* @bsimethod                                                    DavidAssaf      5/99
+----------------------------------------------------------------------*/
void Transform::GetOriginAndVectors
(

DPoint3dR origin,
DVec3dR vector0,
DVec3dR vector1,
DVec3dR vector2

) const
    {
    origin.x = form3d[0][3];
    origin.y = form3d[1][3];
    origin.z = form3d[2][3];

    vector0.x = form3d[0][0];
    vector0.y = form3d[1][0];
    vector0.z = form3d[2][0];

    vector1.x = form3d[0][1];
    vector1.y = form3d[1][1];
    vector1.z = form3d[2][1];

    vector2.x = form3d[0][2];
    vector2.y = form3d[1][2];
    vector2.z = form3d[2][2];
    }



//! Return the determinant of the matrix part.
double Transform::Determinant () const
    {
    DPoint3d origin;
    DVec3d vectorX, vectorY, vectorZ;
    GetOriginAndVectors (origin, vectorX, vectorY, vectorZ);
    return vectorX.TripleProduct (vectorY, vectorZ);
    }

//! Return magnitude of X column.  This is commonly considered as the scale factor of the transform.
double Transform::ColumnXMagnitude () const
    {
    return sqrt (
              form3d[0][0] * form3d[0][0]
            + form3d[1][0] * form3d[1][0]
            + form3d[2][0] * form3d[2][0]
            );
    }


/*-----------------------------------------------------------------*//**
* @description Returns a transformation with origin at origin, x-axis
* xVector, y-axis yVector, and z-axis zVector.
* All axes are unnormalized.
* There is no effort to detect zero length axes or degenerate points that
* define only a line or plane but not a full coordinate system.
* The axes may be skewed.
*
* @param [in] origin The origin of transformed coordinates
* @param [in] xVector The 100 point of transformed coordinates
* @param [in] yVector The 010 point of transformed coordinates
* @param [in] zVector The 001 point of transformed coordinates
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitFromOriginAndVectors
(

DPoint3dCR origin,
DVec3dCR xVector,
DVec3dCR yVector,
DVec3dCR zVector

)
    {
    form3d[0][3] = origin.x;
    form3d[1][3] = origin.y;
    form3d[2][3] = origin.z;

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
* @description Returns a transformation with origin at origin, x-axis from
* origin to xPoint, y-axis from origin to yPoint,
* and z-axis from origin to zPoint.
* All axes are unnormalized.
* There is no effort to detect zero length axes or degenerate points that
* define only a line or plane but not a full coordinate system.
* The axes may be skewed.
*
* @param [in] origin The origin of transformed coordinates
* @param [in] xPoint The 100 point of transformed coordinates
* @param [in] yPoint The 010 point of transformed coordinates
* @param [in] zPoint The 001 point of transformed coordinates
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitFrom4Points
(

DPoint3dCR origin,
DPoint3dCR xPointIN,
DPoint3dCR yPointIN,
DPoint3dCR zPointIN

)
    {
    DVec3d xVector, yVector, zVector;

    xVector.DifferenceOf (xPointIN, origin);
    yVector.DifferenceOf (yPointIN, origin);
    zVector.DifferenceOf (zPointIN, origin);
    this->InitFromOriginAndVectors (origin, xVector, yVector, zVector);
    }


/*-----------------------------------------------------------------*//**
* @description Returns a transformation with origin at origin, x-axis from
* origin to xPoint, y-axis from origin to yPoint,
* and z-axis equal to the cross product of x and y axes.
* All axes are unnormalized.
* There is no effort to detect zero length axes or degenerate points that
* define only a line or plane but not a full coordinate system.

* @param [in] origin The origin of coordinate system
* @param [in] xPoint The 100 point of coordinate system
* @param [in] yPoint The 010 point of coordinate system
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitFromPlaneOf3Points
(
DPoint3dCR origin,
DPoint3dCR xPoint,
DPoint3dCR yPoint
)
    {
    DVec3d xVector, yVector, zVector;

    xVector.DifferenceOf (xPoint, origin);
    yVector.DifferenceOf (yPoint, origin);
    zVector.CrossProduct (xVector, yVector);
    this->InitFromOriginAndVectors (origin, xVector, yVector, zVector);
    }

//! @description Returns a transformation with origin, x axis direction, and xy plane
//! All axes are normalized and perpendicular.
//!
//! @param [in] origin The origin of transformed coordinates
//! @param [in] xVector direction for x axis
//! @param [in] yVector "In plane" direction for y axis.
//!
bool Transform::InitFromOriginXVectorYVectorSquareAndNormalize
(
DPoint3dCR      origin,
DVec3dCR      xVector,
DVec3dCR      yVector
)
    {
    RotMatrix matrix;
    matrix.InitFromColumnVectors (xVector, yVector, DVec3d::From (0,0,0));
    if (matrix.SquareAndNormalizeColumns (matrix, 0, 1))
        {
        InitFrom (matrix, origin);
        return true;
        }
    InitFrom (origin);
    return false;
    }

Transform Transform::FromPlaneOf3PointsZeroZ
(
DPoint3dCR origin,
DPoint3dCR xPoint,
DPoint3dCR yPoint
)
    {
    DVec3d U, V;

    U.DifferenceOf (xPoint, origin);
    V.DifferenceOf (yPoint, origin);
    return Transform::FromRowValues
        (
        U.x, V.x, 0, origin.x,
        U.y, V.y, 0, origin.y,
        U.z, V.z, 0, origin.z
        );
    }




/*-----------------------------------------------------------------*//**
* @description Returns a (possibly skewed) transformation with origin
* origin, the axis axisId towards xPoint, and other axes perpendicular.
* If normalize is false, all axes have length equal to the distance
* between the two origin and xPoint.
* The axes may be skewed.
*
* @param [in] origin The origin of coordinate system
* @param [in] xPoint The target point of axis axisId of coordinate system
* @param [in] axisId The axis that points from origin to xPoint
* @param [in] normalize true to have coordinate system normalized
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InitFromPlaneNormalToLine
(

DPoint3dCR origin,
DPoint3dCR xPoint,
int        axisId,
bool       normalize

)
    {
    DVec3d vector;
    RotMatrix matrix;
    vector.DifferenceOf (xPoint, origin);
    matrix.InitFrom1Vector (vector, axisId, normalize);
    this->InitFrom (matrix, origin);
    }


/*-----------------------------------------------------------------*//**
* Initializes a Transform from the affine part of one of the
* matrices in an DMap4d.
* If the scale factor is 0, the transform is returned as an identity.
* If the scale factor is other than 1, the all coefficients are divided
* by the scale factor.
* Return code is false if the 4'th row is significantly different from
* (0,0,0,A).
*
* @param [in] hMap The source mapping
* @param [in] inverse true to extract the inverse, false to extract the forward
* @return true if the DMatrix4d can be collapsed to affine form.
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool Transform::InitFrom
(
DMap4dCR hMap,
      int             inverse
)
    {
    return InitFrom (inverse ? hMap.M1 : hMap.M0);
    }


/*-----------------------------------------------------------------*//**
* @description Returns a copy of a transformation.
*
* @param [in] source The source transform
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Copy (TransformCR source)
    {
    *this = source;
    }



/*-----------------------------------------------------------------*//**
*
* Transform the a,b,c,d components for an implicit plane.
* The plane equation format is ax+by+cz=d.

* @param [out] aOut x coefficient in transformed plane equation
* @param [out] bOut y coefficient in transformed plane equation
* @param [out] cOut z coefficient in transformed plane equation
* @param [out] cOut transformed right hand side constant
* @param [in] a The x coefficient in plane equation
* @param [in] b The y coefficient in plane equation
* @param [in] c The z coefficient in plane equation
* @param [in] d The constant on right hand side of plane equation
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/98
+----------------------------------------------------------------------*/
bool Transform::TransformImplicitPlane
(

      double        &aOut,
      double        &bOut,
      double        &cOut,
      double        &dOut,
      double        a,
      double        b,
      double        c,
      double        d

) const
    {
    Transform inverse;

    if (inverse.InverseOf (*this))
        {
        aOut =      a * inverse.form3d[0][0]
                    + b * inverse.form3d[1][0]
                    + c * inverse.form3d[2][0];

        bOut =      a * inverse.form3d[0][1]
                    + b * inverse.form3d[1][1]
                    + c * inverse.form3d[2][1];

        cOut =      a * inverse.form3d[0][2]
                    + b * inverse.form3d[1][2]
                    + c * inverse.form3d[2][2];

        dOut =      -(a * inverse.form3d[0][3]
                    + b * inverse.form3d[1][3]
                    + c * inverse.form3d[2][3]
                    - d);
        return true;
        }
    return false;
    }



/*-----------------------------------------------------------------*//**
* Computes the 8 corner points of the range cube defined by the two points
* of pRange, multiplies pTransform times each point, and computes the
* twopoint range limits (min max) of the resulting rotated box.
* Note that the volume of the rotated box is generally larger than
* that of the input because the postrotation minmax operation expands
* the box.
*
* @param [out] outRange range of transformed cube
* @param [in] inRange The any two corners of original cube
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::Multiply
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
        Multiply (corner, 8);
        outRange.InitFrom (corner, 8);
        }
    }



/*-----------------------------------------------------------------*//**
* @description Returns a transformation in the xy-plane with origin origin
* and x,y-axes of given lengths.
* The z-coordinate of the origin is zero and the z-axis is unscaled.
*
* @param [in] origin origin of coordinate system (or null)
* @param [in] xAxisLength The length of x-axis
* @param [in] yAxisLength The length of y-axis
* @indexVerb
* @bsimethod                                                    DavidAssaf      12/98
+----------------------------------------------------------------------*/
void Transform::InitFromOriginAndLengths
(

DPoint2dCR origin,
double     xAxisLength,
double     yAxisLength

)
    {
    form3d[0][0] = xAxisLength;
    form3d[1][0] = 0.0;
    form3d[2][0] = 0.0;

    form3d[0][1] = 0.0;
    form3d[1][1] = yAxisLength;
    form3d[2][1] = 0.0;

    form3d[0][2] = 0.0;
    form3d[1][2] = 0.0;
    form3d[2][2] = 1.0;

    form3d[0][3] = origin.x;
    form3d[1][3] = origin.y;
    form3d[2][3] = 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a transformation in the xy-plane with    origin origin and
* x,y-axes of the given lengths rotated counter-clockwise from standard position
* by the given angle.
* The z-coordinate of the origin is zero and the z-axis is unscaled.
*
* @param [in] origin origin of coordinate system (or null)
* @param [in] xAxisAngleRadians The ccw angle     separating x-axis from its standard position
* @param [in] xAxisLength The length of x-axis
* @param [in] yAxisLength The length of y-axis
* @indexVerb
* @bsimethod                                                    DavidAssaf      12/98
+----------------------------------------------------------------------*/
void Transform::InitFromOriginAngleAndLengths
(

DPoint2dCR origin,
double     xAxisAngleRadians,
double     xAxisLength,
double     yAxisLength

)
    {
    double c = cos (xAxisAngleRadians);
    double s = sin (xAxisAngleRadians);

    form3d[0][0] = xAxisLength * c;
    form3d[1][0] = xAxisLength * s;
    form3d[2][0] = 0.0;

    form3d[0][1] = yAxisLength * -s;
    form3d[1][1] = yAxisLength * c;
    form3d[2][1] = 0.0;

    form3d[0][2] = 0.0;
    form3d[1][2] = 0.0;
    form3d[2][2] = 1.0;

    form3d[0][3] = origin.x;
    form3d[1][3] = origin.y;
    form3d[2][3] = 0.0;
    }


/*-----------------------------------------------------------------*//**
* Sets the translation part of this instance so that it leaves the given point
* unchanged, i.e. so that this instance may be interpreted as
* applying its matrix part as a rotation or scaling (in the xy-plane)
* about the point.
* Symbolically, given transform [R t] and column vector p,
* the returned transform is [R p-R*p].
* (The prior translation part is destroyed, and does not affect the
* result in any way.)
*
* @param [in] point The point that is to remain fixed when multiplied by the modified transformation
* @indexVerb
* @bsimethod                                                    DavidAssaf      12/98
+----------------------------------------------------------------------*/
void Transform::SetFixedPoint
(
DPoint2dCR point
)
    {
    form3d[0][3] = point.x
                - form3d[0][0] * point.x
                - form3d[0][1] * point.y;
    form3d[1][3] = point.y
                - form3d[1][0] * point.x
                - form3d[1][1] * point.y;
    form3d[2][3] = 0.0;
    }


/*-----------------------------------------------------------------*//**
* Sets the translation part of this instance to the given point.
* The prior translation part is overwritten (and z-coord set to zero) and the
* matrix part is unchanged.
* Symbolically, if point is u then this instance [R t] becomes the
* transformation [R u].
*
* @param [in] point The point to insert
* @indexVerb
* @bsimethod                                                    DavidAssaf      12/98
+----------------------------------------------------------------------*/
void Transform::SetTranslation
(

DPoint2dCR point

)
    {
    form3d[0][3] = point.x;
    form3d[1][3] = point.y;
    form3d[2][3] = 0.0;
    }


/*-----------------------------------------------------------------*//**
* Sets the given point to the x- and y-coordinates of the translation part
* of this instance.
*
* @param [out] point translation part of transformation
* @indexVerb
* @bsimethod                                                    DavidAssaf      12/98
+----------------------------------------------------------------------*/
void Transform::GetTranslation
(

DPoint2dR point

) const
    {
    point.x = form3d[0][3];
    point.y = form3d[1][3];
    }


/*-----------------------------------------------------------------*//**
* Sets this instance to a transformation in the xy-plane with origin at
* origin, x-axis xVector and y-axis yVector.
* All axes are unnormalized.
* There is no effort to detect zero length axes or degenerate points that
* define a line but not a full coordinate system.
* The axes may be skewed.
*
* @param [in] origin The origin of transformed coordinates (or null)
* @param [in] xVector The 10 point of transformed coordinates
* @param yVector=> 01 point of transformed coordinates
* @indexVerb
* @bsimethod                                                    DavidAssaf      12/98
+----------------------------------------------------------------------*/
void Transform::InitFromOriginAndVectors
(
DPoint2dCR origin,
DVec2dCR xVector,
DVec2dCR yVector
)
    {
    form3d[0][0] = xVector.x;
    form3d[1][0] = xVector.y;
    form3d[2][0] = 0.0;

    form3d[0][1] = yVector.x;
    form3d[1][1] = yVector.y;
    form3d[2][1] = 0.0;

    form3d[0][2] = 0.0;
    form3d[1][2] = 0.0;
    form3d[2][2] = 1.0;

    form3d[0][3] = origin.x;
    form3d[1][3] = origin.y;
    form3d[2][3] = 0.0;
    }

/*-----------------------------------------------------------------*//**
* Returns an instance with with origin at
* origin, x-axis xVector and y-axis yVector.
* All axes are unnormalized.
* There is no effort to detect zero length axes or degenerate points that
* define a line but not a full coordinate system.
* The axes may be skewed.
*
* @param [in] origin The origin of transformed coordinates (or null)
* @param [in] xVector The 10 point of transformed coordinates
* @param yVector=> 01 point of transformed coordinates
* @indexVerb
* @bsimethod                                                    DavidAssaf      12/98
+----------------------------------------------------------------------*/
Transform Transform::FromOriginAndVectors
(
DPoint2dCR origin,
DVec2dCR xVector,
DVec2dCR yVector
)
    {
    Transform result;
    result.InitFromOriginAndVectors (origin, xVector, yVector);
    return result;
    }

/*-----------------------------------------------------------------*//**
* Sets this instance to a transformation in the xy-plane with origin at origin,
* x-axis from origin to xPoint and y-axis from origin to yPoint.
* All axes are unnormalized.
* There is no effort to detect zero length axes or degenerate points that
* define a line but not a full coordinate system.
* The axes may be skewed.
*
* @param [in] origin The origin of transformed coordinates (or null)
* @param [in] xPoint The 10 point of transformed coordinates
* @param [in] yPoint The 01 point of transformed coordinates
* @indexVerb
* @bsimethod                                                    DavidAssaf      12/98
+----------------------------------------------------------------------*/
void Transform::InitFrom3Points
(
DPoint2dCR origin,
DPoint2dCR xPoint,
DPoint2dCR yPoint
)
    {
    form3d[0][0] = xPoint.x - origin.x;
    form3d[1][0] = xPoint.y - origin.y;
    form3d[2][0] = 0.0;

    form3d[0][1] = yPoint.x - origin.x;
    form3d[1][1] = yPoint.y - origin.y;
    form3d[2][1] = 0.0;

    form3d[0][2] = 0.0;
    form3d[1][2] = 0.0;
    form3d[2][2] = 1.0;

    form3d[0][3] = origin.x;
    form3d[1][3] = origin.y;
    form3d[2][3] = 0.0;
    }



//! @description Attempt to set up a coordinate frame origin at origin, x-axis from
//! origin to xPoint, y-axis in plane with yPoint,
//! All axes are normalized.
//! Return false with identity at origin if unable to do cross products.
//! @param [in] origin The origin of coordinate system
//! @param [in] xPoint The 100 point of coordinate system
//! @param [in] yPoint The 010 point of coordinate system
bool Transform::InitNormalizedFrameFromOriginXPointYPoint
(
DPoint3dCR origin,
DPoint3dCR xPoint,
DPoint3dCR yPoint
)
    {
    DVec3d vectorU, vectorV, vectorW;
    double relTol = Angle::SmallAngle ();
    double u = vectorU.NormalizedDifference (xPoint, origin);
    double v = vectorV.NormalizedDifference (yPoint, origin);
    // What's zero?  Really hard to tell.  BUT.. 
    if (    u > relTol * v
        &&  v > relTol * u
        )
        {
        double w = vectorW.NormalizedCrossProduct (vectorU, vectorV);
        if (w > relTol)
            {
            vectorV.NormalizedCrossProduct (vectorW, vectorU);
            InitFromOriginAndVectors (origin, vectorU, vectorV, vectorW);
            return true;
            }
        }
    InitFrom (origin);
    return false;
    }



/*-----------------------------------------------------------------*//**
* Initialize from first three rows of DMatrix4d, divided by weight entry.
* ALWAYS copy the data, even if final row has invalid data.
* @return false if 4th row is other than 0001
* @indexVerb
* @bsimethod                                                    DavidAssaf      12/98
+----------------------------------------------------------------------*/
bool Transform::InitFrom (DMatrix4dCR matrix)
    {
    double scale = matrix.coff[3][3];
    static double relTol = 1.0e-12;
    bool boolStat = true;
    int i;
    double sum;

    if (scale == 0.0)
        {
        boolStat = false;
        this->InitIdentity ();
        }
    else
        {
        if (scale == 1.0)
            {
            /* This is really the most common case.*/
            for (i = 0; i < 3; i++)
                {
                form3d[0][i] = matrix.coff[0][i];
                form3d[1][i] = matrix.coff[1][i];
                form3d[2][i] = matrix.coff[2][i];
                }
            form3d[0][3] = matrix.coff[0][3];
            form3d[1][3] = matrix.coff[1][3];
            form3d[2][3] = matrix.coff[2][3];
            }
        else
            {
            double a = 1.0 / scale;
            for (i = 0; i < 3; i++)
                {
                form3d[0][i] = a * matrix.coff[0][i];
                form3d[1][i] = a * matrix.coff[1][i];
                form3d[2][i] = a * matrix.coff[2][i];
                }
            form3d[0][3] = a * matrix.coff[0][3];
            form3d[1][3] = a * matrix.coff[1][3];
            form3d[2][3] = a * matrix.coff[2][3];
            }
        sum =
                      fabs (matrix.coff[3][0])
                    + fabs (matrix.coff[3][1])
                    + fabs (matrix.coff[3][2]);
        if (sum >= relTol * scale)
            boolStat = false;
        }
    return boolStat;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a transformation in the xy-plane
* with origin origin, axis axisId towards xPoint, and the other axis
* perpendicular.
* If normalize is false, both axes have length equal to the distance
* between origin and xPoint.
*
* @param [in] origin The origin of coordinate system (or null)
* @param [in] xPoint The target point of axis axisId of coordinate system
* @param [in] axisId The axis (x=0, y=1) that points from origin to xPoint
* @param [in] normalize true to have coordinate system normalized
* @indexVerb
* @bsimethod                                                    DavidAssaf      12/98
+----------------------------------------------------------------------*/
void Transform::InitFrom2Points
(

DPoint2dCR origin,
DPoint2dCR xPoint,
int        axisId,
bool       normalize

)
    {
    double mag, magRecip;
    DVec2d vector1, vector2;

    vector1.x = xPoint.x - origin.x;
    vector1.y = xPoint.y - origin.y;

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

        this->InitFromOriginAndVectors (origin, vector1, vector2);
        }
    else                /* y-axis in direction vector1 */
        {
        vector2.x =  vector1.y;
        vector2.y = -vector1.x;

        this->InitFromOriginAndVectors (origin, vector2, vector1);
        }
    }


/*-----------------------------------------------------------------*//**
* Sets origin to the translation part, and sets vector0 and vector1
* to the columns of this 2D instance.
*
* @param [out] origin origin of transform coordinates
* @param [out] vector0 10 vector of transform coordinates
* @param [out] vector1 01 vector of transform coordinates
* @indexVerb
* @bsimethod                                                    DavidAssaf      5/99
+----------------------------------------------------------------------*/
void Transform::GetOriginAndVectors
(

DPoint2dR origin,
DVec2dR vector0,
DVec2dR vector1

) const
    {
    origin.x = form3d[0][3];
    origin.y = form3d[1][3];

    vector0.x = form3d[0][0];
    vector0.y = form3d[1][0];

    vector1.x = form3d[0][1];
    vector1.y = form3d[1][1];
    }


/*-----------------------------------------------------------------*//**
* @description Construct a transform which preserves both a primary column directon
* and a secondary column plane.   Scale all columns to length of
* primary axis.
* @param [in] transform original matrix.
* @param [in] primaryAxis axis to be retained.
* @param [in] secondaryAxis axis defining plane to be maintained.
* @bsimethod                            EarlinLutz          10/01
+----------------------------------------------------------------------*/
bool Transform::InitUniformScaleApproximation
(

TransformCR transform,
int         primaryAxis,
int         secondaryAxis

)
    {
    RotMatrix matrixPart, rotationPart, skewPart;
    DPoint3d translation;
    bool boolstat = false;
    double scale;
    matrixPart.InitFrom (transform);
    transform.GetTranslation (translation);
    if (matrixPart.RotateAndSkewFactors
                    (
                    rotationPart,
                    skewPart,
                    primaryAxis,
                    secondaryAxis
                    ))
        {
        scale = skewPart.GetComponentByRowAndColumn (primaryAxis, primaryAxis);
        rotationPart.ScaleColumns (rotationPart, scale, scale, scale);
        boolstat = true;
        }
    else
        {
        matrixPart.InitIdentity ();
        }

    this->InitFrom (rotationPart, translation);
    return boolstat;
    }



/*-----------------------------------------------------------------*//**
* @description Returns the largest absolute value difference between
*   corresponding coefficients
* @param [in] otherTransform
* @return largest absolute difference between the two transforms
* @bsimethod                                                    EarlinLutz     03/10
+----------------------------------------------------------------------*/
double Transform::MaxDiff
(
TransformCR otherTransform
) const
    {
    double a = 0.0;
    double b;
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < 3; i++)
            {
            b = fabs (form3d[i][j] - otherTransform.form3d[i][j]);
            if (b > a)
                a = b;
            }
    return a;
    }

//! return a transform that sweeps points along a vector until they hit a plane.
//! Note that this transform is singular.
//! @returns sweep transform.  This is invalid if the plane or sweep vector is zero.
ValidatedTransform Transform::FromSweepToPlane
(
DVec3dCR sweepVector,   //!< [in] sweep direction
DPlane3dCR targetPlane  //!< [in] target plane where sweep stops
)
    {
    RotMatrix Q = RotMatrix::FromIdentity ();
    auto s = DoubleOps::ValidatedDivide (1.0, sweepVector.DotProduct (targetPlane.normal));
    if (!s.IsValid ())
        {
        return ValidatedTransform (Transform::FromIdentity (), false);
        }
    Q.AddScaledOuterProductInPlace (sweepVector, targetPlane.normal, -s);
    return ValidatedTransform (
            Transform::From (Q,
                DVec3d::FromScale (sweepVector, targetPlane.normal.DotProduct (targetPlane.origin) * s)),
            true);
    }

DVec3d Transform::ColumnX () const {return DVec3d::From (form3d[0][0], form3d[1][0], form3d[2][0]);}
DVec3d Transform::ColumnY () const {return DVec3d::From (form3d[0][1], form3d[1][1], form3d[2][1]);}
DVec3d Transform::ColumnZ () const {return DVec3d::From (form3d[0][2], form3d[1][2], form3d[2][2]);}


END_BENTLEY_NAMESPACE
