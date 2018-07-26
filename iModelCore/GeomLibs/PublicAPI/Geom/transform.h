/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/transform.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*
#ifdef flexwiki
:Title: struct Bentley::Transform

Summary: A 3x4 matrix for defining coordinate frames and affine transformations.

Individual entries are named thusly
 [ axx axy axz tx ]
 [ ayx ayy ayz ty ]
 [ azx azy azz tz ]
 
The leading a__ parts are called the "matrix" part, and the t_ parts are called the "translation" part.

#endif
*/

/*__PUBLISH_SECTION_START__*/

#ifndef transform_H_
#define transform_H_

BEGIN_BENTLEY_NAMESPACE
/**
3x4 matrix for defining coordinate frames and affine transformations.

Useful typedefs for Transform
\code
    typdedef struct const &Transform TransformCR;
    typdedef struct &Transform TransformR;
    typdedef struct const *Transform TransformCP;
    typdedef struct *Transform TransformP;
\endcode

@ingroup GROUP_Geometry
*/
struct GEOMDLLIMPEXP Transform
{
//! 3x4 matrix.  Leading 3x3 part is 3x3 matrix commonly called "rotation part"; final column is commonly called "translation part".
double form3d[3][4];

#ifdef __cplusplus

//flex!! Construction and initialization
//flex|| computed matrix || result returned from static method || result placed in instance.  No use of prior instance contents. || modify instance in place ||

//flex|| identity  || outTransform = Transform::FromIdentity () || outTransform.InitIdentity () ||
//! @description returns an identity transformation, i.e. zero
//! translation part and identity matrix part.
static Transform FromIdentity ();
//! @description returns an identity transformation, i.e. zero
//! translation part and identity matrix part.
void InitIdentity ();

//flex|| matrix given, zero translation || outTransform = Transform::From (matrix) || ||

//! @description Returns a transformation with the given matrix part and a zero translation part.
//! @param [in] matrix The matrix part
static Transform From (RotMatrixCR matrix);
//! @description Initializes a transformation with the given matrix part and a zero translation part.
//! @param [in] matrix The matrix part
void InitFrom (RotMatrixCR matrix);

//! @description Initializes a transformation with scale factors on the diagonal
void InitFromScaleFactors (double xScale, double yScale, double zScale);

//! @description Returns a transformation with scale factors on the diagonal
static Transform FromScaleFactors (double xScale, double yScale, double zScale);

//flex|| matrix, translation given || outTransform = Transform::From (matrix, point) || ||
//! @description Returns a transformation with the given matrix and translation parts.
//! @param [in] matrix The matrix part
//! @param [in] translation The translation part
static Transform From  (RotMatrixCR matrix, DPoint3dCR translation);
//! @description Returns a transformation with the given matrix and translation parts.
//! @param [in] matrix The matrix part
//! @param [in] translation The translation part
void InitFrom  (RotMatrixCR matrix, DPoint3dCR translation);

//flex|| translation given  || outTransform = Transform::From (point) || outTransform.InitFrom (point) ||
//flex|| || outTransform = Transform::InitFrom (x, y, z) || outTransform.InitFrom (x, y, z) ||

//! @description Returns a transformation with identity matrix part and given translation part.
//! @param [in] translation The translation part
static Transform From (DPoint3dCR translation);
//! @description Returns a transformation with identity matrix part and given translation part.
//! @param [in] translation The translation part
void InitFrom (DPoint3dCR translation);

//flex|| matrix given, compute translation to hold specified fixed point || outTransform = Transform::FromFixedPoint (matrix, fixedPoint) || outTransform.InitFromMatrixAndFixedPOint (matrix, fixedPoint) ||

//! @description Returns a transformation with given matrix part, and translation
//!   part computed from the matrix and a given fixed point.  This translation part
//!   is generally different from the fixed point itself.   The resulting transformation
//!   will leave the fixed point unchanged and apply whatever effects are contained in the
//!   matrix as if the fixed point is the origin.
//! @param [in] matrix The matrix part
//! @param [in] origin The point that is to remain fixed when multiplied by the transformation.
//!
static Transform FromMatrixAndFixedPoint  (RotMatrixCR matrix, DPoint3dCR origin);

//! @description Returns a transformation with given matrix part, and translation
//!   part computed from the matrix and a given fixed point.  This translation part
//!   is generally different from the fixed point itself.   The resulting transformation
//!   will leave the fixed point unchanged and apply whatever effects are contained in the
//!   matrix as if the fixed point is the origin.
//! @param [in] matrix The matrix part
//! @param [in] origin The point that is to remain fixed when multiplied by the transformation.
//!
void InitFromMatrixAndFixedPoint  (RotMatrixCR matrix, DPoint3dCR origin);

//!
//! @description Returns a transformation with identity matrix part and
//!       translation part given as x, y, and z components.
//! @param [in] x The x-coordinate of translation part
//! @param [in] y The y-coordinate of translation part
//! @param [in] z The z-coordinate of translation part
//!
static Transform From  (double x, double y, double z);
//!
//! @description Returns a transformation with identity matrix part and
//!       translation part given as x, y, and z components.
//! @param [in] x The x-coordinate of translation part
//! @param [in] y The y-coordinate of translation part
//! @param [in] z The z-coordinate of translation part
//!
void InitFrom  (double x, double y, double z);

//flex|| Complete list of entries || outTransform = Transform::FromRowValues (axx, axy, axz, tx, ayx, ayy, ayz, ty, azx, azy, azz, tz) || outTransform.InitFromRowValues (axx etc) ||

//! @description Returns a transformation copying the double values directly into the rows of this instance.
//!
//! @param [in] x00 The (0,0) entry of the matrix (row, column)
//! @param [in] x01 The (0,1) entry
//! @param [in] x02 The (0,2) entry
//! @param [in] tx The x-coordinate of the translation part
//! @param [in] x10 The (1,0) entry
//! @param [in] x11 The (1,1) entry
//! @param [in] x12 The (1,2) entry
//! @param [in] ty The y-coordinate of the translation part
//! @param [in] x20 The (2,0) entry
//! @param [in] x21 The (2,1) entry
//! @param [in] x22 The (2,2) entry
//! @param [in] tz The z-coordinate of the translation part
//!
static Transform FromRowValues
    (
    double x00, double x01, double x02, double tx,
    double x10, double x11, double x12, double ty,
    double x20, double x21, double x22, double tz
    );



//! Copies the double values directly into the rows of this instance.
//!
//! @param [in] x00 The (0,0) entry of the matrix (row, column)
//! @param [in] x01 The (0,1) entry
//! @param [in] x02 The (0,2) entry
//! @param [in] tx The x-coordinate of the translation part
//! @param [in] x10 The (1,0) entry
//! @param [in] x11 The (1,1) entry
//! @param [in] x12 The (1,2) entry
//! @param [in] ty The y-coordinate of the translation part
//! @param [in] x20 The (2,0) entry
//! @param [in] x21 The (2,1) entry
//! @param [in] x22 The (2,2) entry
//! @param [in] tz The z-coordinate of the translation part
//!
void InitFromRowValues
    (
    double x00, double x01, double x02, double tx,
    double x10, double x11, double x12, double ty,
    double x20, double x21, double x22, double tz
    );
    
//flex|| origin and all vectors || || outTransform.InitFromOriginAndVectors (origin, xVector, yVector, zVector)    ||
//! @description Returns a transformation with origin at origin, x-axis
//! xVector, y-axis yVector, and z-axis zVector.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define only a line or plane but not a full coordinate system.
//! The axes may be skewed.
//!
//! @param [in] origin The origin of transformed coordinates
//! @param [in] xVector The 100 vector of transformed coordinates
//! @param [in] yVector The 010 vector of transformed coordinates
//! @param [in] zVector The 001 vector of transformed coordinates
//!
void InitFromOriginAndVectors
(
DPoint3dCR      origin,
DVec3dCR      xVector,
DVec3dCR      yVector,
DVec3dCR      zVector
);


//flex|| origin and vectors in xy plane || || outTransform.InitOrthogonalFromOriginXVectorYVector (origin, xVector, yVector)    ||
//! @description Returns a transformation with origin, x axis direction, and xy plane
//! All axes are normalized and perpendicular.
//!
//! @param [in] origin The origin of transformed coordinates
//! @param [in] xVector direction for x axis
//! @param [in] yVector "In plane" direction for y axis.
//! <returns>true if SquareAndOrthogonalizeColumns succeeds</returns>
bool InitFromOriginXVectorYVectorSquareAndNormalize
(
DPoint3dCR      origin,
DVec3dCR      xVector,
DVec3dCR      yVector
);

//flex|| origin, x target, y target, z target || || outTransform.InitFrom4Points (origin, xTarget, yTarget, zTarget) ||

//! @description Returns a transformation with origin at origin, x-axis from
//! origin to xPoint, y-axis from origin to yPoint,
//! and z-axis from origin to zPoint.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define only a line or plane but not a full coordinate system.
//! The axes may be skewed.
//! @param [in] origin The origin of transformed coordinates
//! @param [in] xPoint The 100 point of transformed coordinates
//! @param [in] yPoint The 010 point of transformed coordinates
//! @param [in] zPoint The 001 point of transformed coordinates
void InitFrom4Points
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
DPoint3dCR      yPoint,
DPoint3dCR      zPoint
);

//flex|| origin, x target, y target || || outTransform.InitFromPlaneOf3Points (origin, xTarget, yTarget) ||
//!
//! @description Returns a transformation with origin at origin, x-axis from
//! origin to xPoint, y-axis from origin to yPoint,
//! and z-axis equal to the cross product of x and y axes.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define only a line or plane but not a full coordinate system.
//!
//! @param [in] origin The origin of coordinate system
//! @param [in] xPoint The 100 point of coordinate system
//! @param [in] yPoint The 010 point of coordinate system
//!
void InitFromPlaneOf3Points
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
DPoint3dCR      yPoint
);

//flex|| origin, one axis along line   || || outTransform.InitFromPlaneNormalToLine (origin, targetPoint, axisIndex, bool normalizeAxes) ||
//!
//! @description Returns a (possibly skewed) transformation with origin
//! origin, the axis axisId towards xPoint, and other axes perpendicular.
//! If normalize is false, all axes have length equal to the distance
//! between the two origin and xPoint.
//! The axes may be skewed.
//!
//! @param [in] origin The origin of coordinate system
//! @param [in] xPoint The target point of axis axisId of coordinate system
//! @param [in] axisId The axis that points from origin to xPoint
//! @param [in] normalize true to have coordinate system normalized
//!
void InitFromPlaneNormalToLine
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
int             axisId,
bool            normalize
);

//flex|| remove 000w row from homogeneous, optionally invert || || bool transform.InitFrom (matrix4d, bool invert) ||
//!
//! Initializes a Transform from the affine part of one of the
//! matrices in an DMap4d.
//! If the scale factor is 0, the transform is returned as an identity.
//! If the scale factor is other than 1, the all coefficients are divided
//! by the scale factor.
//! Return code is false if the 4'th row is significantly different from
//! (0,0,0,A).
//!
//! @param [in] hMap The source mapping
//! @param [in] inverse true to extract the inverse, false to extract the forward
//! @return true if the DMatrix4d can be collapsed to affine form.
//!
bool InitFrom
(
DMap4dCR        hMap,
int             inverse
);

//flex|| direct copy || || outTransform.Copy (transform) ||

//! @description Returns a copy of a transformation.
//! @param [in] source The source transform
void Copy (TransformCR source);

//flex|| 2d origin, xy axis lengths || || outTransform.InitFromOriginAndLengths (origin2d, xLength, yLength) ||

//! @description Returns a transformation in the xy-plane with origin origin
//! and x,y-axes of given lengths.
//! The z-coordinate of the origin is zero and the z-axis is unscaled.
//!
//! @param [in] origin origin of coordinate system
//! @param [in] xAxisLength The length of x-axis
//! @param [in] yAxisLength The length of y-axis
//!
void InitFromOriginAndLengths
(
DPoint2dCR      origin,
double          xAxisLength,
double          yAxisLength
);

//flex|| 2d origin, x axis angle, xy axis lengths || || outTransform.InitFromOriginAndLengths (origin2d, xRadians, xLength, yLength) ||
//!
//! @description Returns a transformation in the xy-plane with    origin origin and
//! x,y-axes of the given lengths rotated counter-clockwise from standard position
//! by the given angle.
//! The z-coordinate of the origin is zero and the z-axis is unscaled.
//!
//! @param [in] origin origin of coordinate system
//! @param [in] xAxisAngleRadians The ccw angle     separating x-axis from its standard position
//! @param [in] xAxisLength The length of x-axis
//! @param [in] yAxisLength The length of y-axis
//!
void InitFromOriginAngleAndLengths
(
DPoint2dCR      origin,
double          xAxisAngleRadians,
double          xAxisLength,
double          yAxisLength
);
    
//!
//! Sets this instance to a transformation in the xy-plane with origin at
//! origin, x-axis xVector and y-axis yVector.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define a line but not a full coordinate system.
//! The axes may be skewed.
//!
//! @param [in] origin The origin of transformed coordinates
//! @param [in] xVector The 10 point of transformed coordinates
//! @param [in] yVector 01 point of transformed coordinates
//!
void InitFromOriginAndVectors  (DPoint2dCR origin, DVec2dCR xVector, DVec2dCR yVector);

//!
//! Returns a transformation in the xy-plane with origin at
//! origin, x-axis xVector and y-axis yVector.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define a line but not a full coordinate system.
//! The axes may be skewed.
//!
//! @param [in] origin The origin of transformed coordinates
//! @param [in] xVector The 10 point of transformed coordinates
//! @param [in] yVector 01 point of transformed coordinates
//!
static Transform FromOriginAndVectors (DPoint2dCR origin, DVec2dCR xVector, DVec2dCR yVector);

//! @description Returns a transformation with
//!<ul>
//!<li>translation xyz are the origin
//!<li>xAxis in direction of bearing radians (parallel to xy plane)
//!<li>yAxis perpenedicular to xAxis and also parallel to the xy plane.
//!<li>zAxis is global (0,0,1)
//!<ul>
static Transform FromOriginAndBearingXY
(
DPoint3dCR      origin,
double bearingRadians
);

//! Returns a transform with given origin and xVector.
//! The yVector is a CCW perpendicular to the xVector (with the same length)
//! The zVecotor is a unitZ.
static Transform FromOriginAndXVector (DPoint2dCR origin, DVec2dCR xVector);

//!
//! Sets this instance to a transformation in the xy-plane with origin at origin,
//! x-axis from origin to xPoint and y-axis from origin to yPoint.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define a line but not a full coordinate system.
//! The axes may be skewed.
//!
//! @param [in] origin The origin of transformed coordinates
//! @param [in] xPoint The 10 point of transformed coordinates
//! @param [in] yPoint The 01 point of transformed coordinates
//!
void InitFrom3Points  (DPoint2dCR origin, DPoint2dCR xPoint, DPoint2dCR yPoint);

//! @description Attempt to set up a coordinate frame origin at origin, x-axis from
//! origin to xPoint, y-axis in plane with yPoint,
//! All axes are normalized.
//! Return false with identity at origin if unable to do cross products.
//! @param [in] origin The origin of coordinate system
//! @param [in] xPoint The 100 point of coordinate system
//! @param [in] yPoint The 010 point of coordinate system
bool InitNormalizedFrameFromOriginXPointYPoint  (DPoint3dCR origin, DPoint3dCR xPoint, DPoint3dCR yPoint);

//!
//! Initialize from first three rows of DMatrix4d, divided by weight entry.
//! ALWAYS copy the data, even if final row has invalid data.
//! @return false if 4th row is other than 0001
//!
bool InitFrom (DMatrix4dCR matrix); 

//! @description Returns a transformation in the xy-plane 
//! with origin origin, axis axisId towards xPoint, and the other axis 
//! perpendicular. 
//! If normalize is false, both axes have length equal to the distance 
//! between origin and xPoint. //! //! @param [in] origin The origin of coordinate system 
//! @param [in] xPoint The target point of axis axisId of coordinate system 
//! @param [in] axisId The axis (x=0, y=1) that points from origin to xPoint 
//! @param [in] normalize true to have coordinate system normalized 
//! 
void InitFrom2Points  (DPoint2dCR origin, DPoint2dCR xPoint, int axisId, bool normalize);

//flex|| Square columns, make all match primary length. || outTrasnform.InitUniformScaleApproximation (transform, primaryAxis, secondaryAxis) || ||
//!
//! @description Construct a transform which preserves both a primary column directon
//! and a secondary column plane.   Scale all columns to length of
//! primary axis.
//! @param [in] transform original matrix.
//! @param [in] primaryAxis axis to be retained.
//! @param [in] secondaryAxis axis defining plane to be maintained.
//!
bool InitUniformScaleApproximation  (TransformCR transform, int primaryAxis, int secondaryAxis);
    
//flex|| True inverse transform || || outTransform.InverseOf (transform) || ValidatedInverse (); ||
//!
//! Sets this instance to the inverse transform of in.
//! in may be the same as this instance.
//! This is a modestly expensive floating point computation (33
//! multiplies, 14 adds).
//! Symbolically, given transform [R t] return transform [Q Q*(-t)]
//! where Q is the inverse of matrix R.
//!
//! @param [in] in The input transformation
//! @remark This is deprecated.  Preferred call is     {resutl = in.ValidatedInverse ();}
//! @return true if transform is invertible
//!
bool InverseOf (TransformCR in);

//!
//! Return the inverse of the instance transform.
//! This is a modestly expensive floating point computation (33
//! multiplies, 14 adds).
//! Symbolically, given transform [R t] return transform [Q Q*(-t)]
//! where Q is the inverse of matrix R.
//!
//! @return 
//!
ValidatedTransform ValidatedInverse () const;



//flex|| Fast Inverse transform, valid only when matrix part is known to be orthogonal || || outTransform.InvertRigidBodyTransform (transform) ||
//!
//!
//! Sets this instance to a matrix which is the inverse of in
//! IN THE SPECIAL CASE WHERE in HAS ONLY PURE ROTATION OR
//! MIRRORING IN ITS ROTATIONAL PART.   These special conditions allow
//! the 'inversion' to be done by only a transposition and one
//! matrix-times-point multiplication, rather than the full effort of
//! inverting a general transformation. It is the caller's responsibility
//! to be sure that these special conditions hold.  This usually occurs
//! when the caller has just constructed the transform by a sequence of
//! translations and rotations.
//! If the caller has received the matrix from nonverified external
//! sources and therefore does not know if the special conditions apply,
//! the <CODE>inverseOf</CODE> method should be used instead.
//! in may be the same as this instance.
//! The specific computations in this special-case inversion are (1) the
//! output transform's translation is the input transform's
//! matrix times the negative of the input transform's translation, and (2) the
//! output transform's matrix part is the tranpose of the input transform's
//! matrix part.
//! Symbolically, given transform [R t] return transform [R^ (R^)*(-t)]
//! where ^ indicates transposition.
//!
//! @param [in] in The input transformation
//!  (TransformCR)
//!
void InvertRigidBodyTransformation (TransformCR in);


//! @description Returns a transformation that maps corners of the source range to corners of the destination range.
//! @return false if either range is null or any direction of the sourceRange has zero length.
static bool TryRangeMapping (DRange2dCR sourceRange, DRange2dCR destRange, TransformR transform);

//! @description Returns a transformation that fits rangeB in rangeA, with uniform scale in XY, and Z with same scale.
//! @return false if either range is null or single point
static bool TryUniformScaleXYRangeFit
(
DRange3dCR rangeA,
DRange3dCR rangeB,
TransformR AToB,
TransformR BToA
);
//!
//! @description Returns a transformation of rotation about a specified line.
//! @param [in] point0 The start point of the line
//! @param [in] point1 The end point of the line
//! @param [in] radians The rotation angle
//!
static Transform FromLineAndRotationAngle  (DPoint3dCR point0, DPoint3dCR point1, double radians);

//!
//! @description Returns a transformation of rotation about a specified ray, and
//!    also its derivative with respect to the angle.
//! @param [in] axis axis of rotation
//! @param [in] radians The rotation angle
//! @param derivativeTransform transform mapping rotated point (anywhere) to direction vector at its destination.
//!
static Transform FromAxisAndRotationAngle  (DRay3dCR axis, double radians, TransformR derivativeTransform);

//!
//! @description Returns a transformation of rotation about a specified ray.
//! @param [in] axis axis of rotation
//! @param [in] radians The rotation angle
//! @param [in] radians The rotation angle
//!
static Transform FromAxisAndRotationAngle  (DRay3dCR axis, double radians);



//!
//! Sets this instance to the transformation obtained by premultiplying
//! inTransform by 3 matrix rotations about principle axes, given by the
//! angles xrot, yrot and zrot.
//! inTransform may be the same as this instance.
//! Symbolically, given transform M and rotation
//! matrices X,Y,Z, the resulting transform is X*Y*Z*M
//!
//! @param [in] inTransform The base transformation
//! @param [in] xrot The x axis rotation, in radians
//! @param [in] yrot The y axis rotation, in radians
//! @param [in] zrot The z axis rotation, in radians
//!
static Transform FromPrincipleAxisRotations
(
TransformCR     inTransform,
double          xrot,
double          yrot,
double          zrot
);

//!
//! @description Returns a transformation with origin at origin, x-axis
//! xVector, y-axis yVector, and z-axis zVector.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define only a line or plane but not a full coordinate system.
//! The axes may be skewed.
//!
//! @param [in] origin The origin of transformed coordinates
//! @param [in] xVector The 100 point of transformed coordinates
//! @param [in] yVector The 010 point of transformed coordinates
//! @param [in] zVector The 001 point of transformed coordinates
//!
static Transform FromOriginAndVectors
(
DPoint3dCR      origin,
DVec3dCR      xVector,
DVec3dCR      yVector,
DVec3dCR      zVector
);

//!
//! @description Returns a transformation with origin at origin, x-axis from
//! origin to xPoint, y-axis from origin to yPoint,
//! and z-axis from origin to zPoint.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define only a line or plane but not a full coordinate system.
//! The axes may be skewed.
//!
//! @param [in] origin The origin of transformed coordinates
//! @param [in] xPoint The 100 point of transformed coordinates
//! @param [in] yPoint The 010 point of transformed coordinates
//! @param [in] zPoint The 001 point of transformed coordinates
//!
static Transform From4Points
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
DPoint3dCR      yPoint,
DPoint3dCR      zPoint
);

//!
//! @description Returns a transformation with origin at origin, x-axis from
//! origin to xPoint, y-axis from origin to yPoint,
//! and z-axis equal to the cross product of x and y axes.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define only a line or plane but not a full coordinate system.
//!
//! @param [in] origin The origin of coordinate system
//! @param [in] xPoint The 100 point of coordinate system
//! @param [in] yPoint The 010 point of coordinate system
//!
static Transform FromPlaneOf3Points
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
DPoint3dCR      yPoint
);

//!
//! @description Returns a transformation with origin at origin, x-axis from
//! origin to xPoint, y-axis from origin to yPoint,
//! and z-axis all zeros.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define only a line or plane but not a full plane.
//!
//! @param [in] origin The origin of coordinate system
//! @param [in] xPoint The 100 point of coordinate system
//! @param [in] yPoint The 010 point of coordinate system
//!
static Transform FromPlaneOf3PointsZeroZ
(
DPoint3dCR origin,
DPoint3dCR xPoint,
DPoint3dCR yPoint
);

//! @description Returns a transformation with x,y,z scales around a fixed point.
//! @param [in] origin The fixed point.
//! @param [in] xScale x direction scale factor.
//! @param [in] yScale y direction scale factor.
//! @param [in] zScale z direction scale factor.
static Transform FromFixedPointAndScaleFactors
(
DPoint3dCR      origin,
double          xScale,
double          yScale,
double          zScale
);

//!
//! @description Returns a (possibly skewed) transformation with origin
//! origin, the axis axisId towards xPoint, and other axes perpendicular.
//! If normalize is false, all axes have length equal to the distance
//! between the two origin and xPoint.
//! The axes may be skewed.
//!
//! @param [in] origin The origin of coordinate system
//! @param [in] xPoint The target point of axis axisId of coordinate system
//! @param [in] axisId The axis that points from origin to xPoint
//! @param [in] normalize true to have coordinate system normalized
//!
static Transform FromPlaneNormalToLine
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
int             axisId,
bool            normalize
);


//!
//! @description Returns a transformation in the xy-plane
//! with origin origin, axis axisId towards xPoint, and the other axis
//! perpendicular.
//! If normalize is false, both axes have length equal to the distance
//! between origin and xPoint.
//!
//! @param [in] origin
//! @param [in] xPoint The target point of axis axisId of coordinate system
//! @param [in] axisId The axis (x=0, y=1) that points from origin to xPoint
//! @param [in] normalize true to have coordinate system normalized
//!
static Transform From2Points
(
DPoint2dCR      origin,
DPoint2dCR      xPoint,
int             axisId,
bool            normalize
);

//flex|| rotation around line  || || outTransform.InitFromLineAndRotationAngle (point0, point1, radians) ||
//!
//! @description Returns a transformation of rotation about a specified line.
//! @param [in] point0 The start point of the line
//! @param [in] point1 The end point of the line
//! @param [in] radians The rotation angle
//!
void InitFromLineAndRotationAngle
(
DPoint3dCR      point0,
DPoint3dCR      point1,
double          radians
);

//flex|| mirror across a plane || || outTransform.InitFromMirrorPlane (point0, planeNormal) ||

//! Initialize a transform that mirrors about a plane.
//! @param [in] origin any point on the mirror plane.
//! @param [in] normal vector perpendicular to mirror plane
bool InitFromMirrorPlane (DPoint3dCR origin, DVec3dCR normal);

//flex|| scale away from a plane || || outTransform.InitFromScalePerpendicularToPlane (point0, planeNormal, scale) ||

//! Initialize a transform that scales around a plane
//! @param [in] origin any point on the target plane
//! @param [in] normal vector perpendicular to target plane.
//! @param [in] scale scale factor.  (e.g. 0.0 to project onto the plane, -1 to mirror)
bool InitFromScalePerpendicularToPlane(DPoint3dCR origin, DVec3dCR normal, double scale);

//flex|| project (flatten) to a plane || || outTransform.InitFromProjectionToPlane (point0, planeNormal) ||
//flex||                              || || outTransform = Transform::FromSweepAlongVectorToPlane (sweepVector, targetPlane) ||

//! return a transform that sweeps points along a vector until they hit a plane.
//!<ul>
//!<li>The return is invalid if the sweep vector is 000.
//!<li>The return is invalid if the plane normal is 000.
//!<li>The return is invalid if the sweep vector is perpendicular to the plane normal.
//!<li>Otherwise the return is valid.
//!<li>However, be aware that because this operation compresses all of space to the plane, the transform matrix is singular.  This is correct.
//!</ul>
//! @returns sweep transform.  This is invalid for if the vectors are zero or perpendicular.
static ValidatedTransform FromSweepToPlane
(
DVec3dCR sweepVector,   //!< [in] sweep direction
DPlane3dCR targetPlane  //!< [in] target plane where sweep stops
);



//! Initialize a transform that projects to a plane.
//! @param [in] origin any point on the target plane.
//! @param [in] normal vector perpendicular to target plane
bool InitFromProjectionToPlane (DPoint3dCR origin, DVec3dCR normal);

//flex|| transform * transform || outTransform = Transform::FromProduct (transformA, transformB) || outTransform.InitProduct (transformA, transformB) ||

//! @description Returns the product of two transforms.
static Transform FromProduct (TransformCR transformA, TransformCR transformB);
//! @description Returns the product of two transformations.
//! Symbolically, given transforms [R t] and [S u], return the product transform
//! [R t][S u] = [R*S t+R*u].
//!
//! @param [in] transform1 The first factor
//! @param [in] transform2 The second factor
//!
void InitProduct  (TransformCR transform1, TransformCR transform2);



//flex|| transformA * transformB * transformC || outTransform = Transform::FromProduct (transformA, transformB, transformC) || outTransform.InitProduct (transformA, transformB. transformC) ||

//! @description Returns the product of thre transforms.
static Transform FromProduct (TransformCR transformA, TransformCR transformB, TransformCR transformC);

//flex|| matrixA * transformB || outTransform = Transform::FromProduct (matrixA, transformB) || outTransform.InitProduct (matrixA, transformB) ||

//! @description Returns the product of a rotmatrix and a transform. (The rotmatrix is treated as a transform with zero translation)
static Transform FromProduct (RotMatrixCR matrixA, TransformCR transformB);
//!
//! @description Returns the product of a matrix and a transformation.
//! The matrix acts like a transformation with a zero point as its translation part.
//!
//! @param [in] matrix The first factor (matrix)
//! @param [in] transform The second factor (transform)
//!
void InitProduct  (RotMatrixCR matrix, TransformCR transform);

//flex|| matrixA * transformB || outTransform = Transform::FromProduct (matrixA, transformB) || outTransform.InitProduct (matrixA, transformB) ||

//! @description Returns the product of a transform and rotmatrix. (The rotmatrix is treated as a transform with zero translation)
static Transform FromProduct (TransformCR transformA, RotMatrixCR matrixB);

//!
//! @description returns the product of a transformation times a matrix, treating
//!   the matrix as a transformation with zero as its translation components.
//! @param [in] transform The first facpatentor (transform)
//! @param [in] matrix The second factor (matrix)
//!
void InitProduct  (TransformCR transform, RotMatrixCR matrix);

//flex!! Content queries and directed updates

//!
//! @description Returns a value from a specified row and column of the matrix part of the transformation.
//! @param [in] row The index of row to read.  Row indices are 0, 1, 2.
//! @param [in] col The index of column to read.  Column indices are 0, 1, 2.
//!
double GetFromMatrixByRowAndColumn
(
int             row,
int             col
) const;

//! @description Sets the value in a specified row and column of the matrix part of the transformation.
//! @param [in] row The index of row to set.  Row indices are 0, 1, 2.
//! @param [in] col The index of column to set.  Column indices are 0, 1, 2.
//! @param [in] value the value to set.
void SetMatrixByRowAndColumn
(
int             row,
int             col,
double          value
);

//!
//! @description Returns a value from a specified component of the point (translation) part of the transformation.
//! @param [in] row The index of point component to read.  Indices are 0, 1, 2 for x, y, z
//!
double GetPointComponent (int row) const;

//!
//! Sets origin to the translation part, and sets vector0, vector1
//! vector2 to the columns of this instance.
//!
//! @param [out] origin origin of transform coordinates
//! @param [out] vector0 100 vector of transform coordinates
//! @param [out] vector1 010 vector of transform coordinates
//! @param [out] vector2 001 vector of transform coordinates
//!
void GetOriginAndVectors  (DPoint3dR origin, DVec3dR vector0, DVec3dR vector1, DVec3dR vector2) const;

//!
//! Sets origin to the translation part, and sets vector0 and vector1
//! to the columns of this 2D instance.
//!
//! @param [out] origin origin of transform coordinates
//! @param [out] vector0 10 vector of transform coordinates
//! @param [out] vector1 01 vector of transform coordinates
//!
void GetOriginAndVectors  (DPoint2dR origin, DVec2dR vector0, DVec2dR vector1) const;

//!
//! Sets point0 to the origin (translation part), and sets point1, point2
//! point3 to the x, y and z points (translations of columns
//! of matrix part by origin) from this instance.
//!
//! @param [out] point0 origin of transform coordinates
//! @param [out] point1 100 point of transform coordinates
//! @param [out] point2 010 point of transform coordinates
//! @param [out] point3 001 point of transform coordinates
//!
void Get4Points  (DPoint3dR point0, DPoint3dR point1, DPoint3dR point2, DPoint3dR point3) const;

//!
//! Sets the four points in array to the origin (translation part), x, y and z
//! points (translations of columns of matrix part by origin) from this
//! instance.
//!
//! @param [out] array origin, 100, 010, 001 points as an array
//!
void Get4Points (DPoint3dP array) const;

//flex|| Matrix part   || transform.GetMatrix (outMatrix)    || transform.SetMatrix (matrix) ||

//! @description Returns the matrix part of a transformation.
//! @param [out] matrix matrix part of transformation
void GetMatrix (RotMatrixR matrix) const;
/** Return the matrix part */
RotMatrix Matrix () const;
/** Return the origin (aka translation) part as a DPoint3d. */
DPoint3d  Origin () const;
/** Return the translation (aka origin) part as a DVec3d. */
DVec3d    Translation () const;
/** return the X column from the matrix part */
DVec3d ColumnX () const;
/** return the Y column from the matrix part */
DVec3d ColumnY () const;
/** return the Z column from the matrix part */
DVec3d ColumnZ () const;


//! @description Overwrites the matrix part of a preexisting transformation.
//!   The translation part is unchanged.
//! @param [in] matrix The matrix to insert
void SetMatrix (RotMatrixCR matrix);

//flex|| Translation   || transform.GetTranslation (outPoint)    || transform.SetTranslation(point) ||
//! @description Returns the translation (point) part of a transformation.
//!
//! @param [out] point vector part of transformation
//!
void GetTranslation (DPoint3dR point) const;


//! Sets the translation part of this instance to point.  The prior
//! translation part is overwritten, and the matrix part is unchanged.
//! Symbolically, if point is u then this instance [R t] becomes the
//! transformation [R u].
//! @param [in] point The vector to insert
void SetTranslation (DPoint3dCR point);
//flex|| set translation to zero (2d) || || transform.ZeroTranslation () ||

//! Sets the translation part of this instance to zero.  The prior
//! translation part is overwritten, and the matrix part is unchanged.
//! Symbolically, this instance [R t] becomes the transformation [R 0].
void ZeroTranslation ();

//flex|| Translation part (2d) || transform.GetTranslation (point2d) || transform.SetTranslation (point2d) ||
//!
//! Sets the given point to the x- and y-coordinates of the translation part
//! of this instance.
//!
//! @param [out] point translation part of transformation
//!
void GetTranslation (DPoint2dR point) const;

//!
//! Sets the translation part of this instance to the given point.
//! The prior translation part is overwritten (and z-coord set to zero) and the
//! matrix part is unchanged.
//! Symbolically, if point is u then this instance [R t] becomes the
//! transformation [R u].
//!
//! @param [in] point The point to insert
//!
void SetTranslation (DPoint2dCR point);


//flex|| Compute translation so given point stays fixed    || || matrix.SetFixedPoint (fixedPoint) ||
//flex|| (2d) || || matrix.SetFixedPoint (fixedPoint2d) ||

//! Sets the translation part of this instance so that it leaves point
//! point unchanged, i.e. so that this instance may be interpreted as
//! applying its matrix part as a rotation or scaling about point.
//! Symbolically, given transform [R t] and column vector p,
//! the returned transform is [R p-R*p].
//! (The prior translation part is destroyed, and does not affect the
//! result in any way.)
//!
//! @param [in] point The point that is to remain fixed when multiplied by the modified transformation
//!
void SetFixedPoint (DPoint3dCR point);

//!
//! Sets the translation part of this instance so that it leaves the given point
//! unchanged, i.e. so that this instance may be interpreted as
//! applying its matrix part as a rotation or scaling (in the xy-plane)
//! about the point.
//! Symbolically, given transform [R t] and column vector p,
//! the returned transform is [R p-R*p].
//! (The prior translation part is destroyed, and does not affect the
//! result in any way.)
//!
//! @param [in] point The point that is to remain fixed when multiplied by the modified transformation
//!
void SetFixedPoint (DPoint2dCR point);


//flex|| access matrix column || matrix.GetMatrixColumn (outVector, columnIndex) ||
//flex||                       || a = matrix.MatrixColumnMagnitude (columnIndex) ||
//flex|| access matrix row || matrix.MatrixRow (outVector, rowIndex) ||
//!
//! @description Returns a column from the matrix part of the transformation.
//!
//! @param [out] column column of matrix part.
//! @param [in] index column index
//!
void GetMatrixColumn  (DVec3dR column, int index) const;
DVec3d GetMatrixColumn 
(
int index       //!< [in] index of matrix column.(cyclic 012012) 
) const;

//!
//! @description Set a column of the matrix part.
//!
//! @param [out] column column data
//! @param [in] index column index
//!
void SetMatrixColumn  (DVec3dCR column, int index);

//!
//! @description Set a column of the matrix part.
//!
//! @param [out] row row data
//! @param [in] index column index
//!
void SetMatrixRow  (DVec3dCR row, int index);

//!
//! @description Returns a row from the matrix part of the transformation.
//!
//! @param [out] row row of matrix part.
//! @param [in] index column index
//!
void GetMatrixRow  (DVec3dR row, int index) const;

//! return the magnitude of a column of the matrix.
//! @param [in] i column index.  Adjusted cyclically if outside 012
double MatrixColumnMagnitude (int i) const;


//flex|| Equality tests    || transform.IsEqual (transformB)  || transform.IsEqual (transformB, matrixTolerance, translationTolerance) ||
//flex|| absolute difference   || a = transform.MaxDiff (transformB) ||
//! @description Returns true if two transforms have exact (bitwise) equality.
//! @param [in] transform2 The second transform
//! @return   true if the transforms are identical
bool IsEqual (TransformCR transform2) const;

//! @description Returns true if two transformations are equal within tolerance, using
//!       separate tolerances for the matrix and point parts.
bool IsEqual  (TransformCR transform2, double matrixTolerance, double pointTolerance) const;

//flex|| Largest difference between corresponding entries || a = transform.MaxDiff (transformB) ||
//! @description Returns the largest absolute value difference between
//! corresponding coefficients
//! @param [in] otherTransform
//! @return largest absolute difference between the two transforms
double MaxDiff (TransformCR otherTransform) const;

//flex|| Determinant of the matrix part || a = transform.Determinant() ||
//! Return the determinant of the matrix part.
double Determinant () const;

//! Return magnitude of X column.  This is commonly considered as the scale factor of the transform.
double ColumnXMagnitude () const;


//flex!! Multiplying other types

//flex|| operation || inplace form || separated input and output || other ||
//flex|| transform * point || transform.Multiply (point) || transform.Multiply (outPoint, inPointA) || transform.Multiply (outPoint, x, y, z) ||

//! @description Multiplies a point by a transform, returning the result in
//!   place of the input point.
//! @param [in,out] point point to be updated
void Multiply (DPoint3dR point) const;

//!
//! @description Returns the product of a transform times a point.
//! @param [out] result returned point.
//! @param [in] point The input point.
//!
void Multiply  (DPoint3dR result, DPoint3dCR point) const;

//! Return the X component of transform*point.
double MultiplyX (DPoint3dCR point) const;
//! Return the  Y component of transform*point.
double MultiplyY (DPoint3dCR point) const;
//! Return the Z component of transform*point.
double MultiplyZ (DPoint3dCR point) const;


//!
//! @description Returns the product of a matrix times a point, with the point
//!       specified as explict x, y, and z values.
//! @param [out] point result of transformation * point operation
//! @param [in] x The x component of the point
//! @param [in] y The y component of the point
//! @param [in] z The z component of the point
//!
void Multiply  (DPoint3dR point, double x, double y, double z) const;
//flex|| point array   || transform.Multiply (points[], n) || transform.Multiply (outPointsp[], inPoints[], n) ||
//flex||               ||         || transform.MultiplyTranspose (points[], n) ||

//!
//! Multiplies this instance times each column vector in pointArray
//! and replaces pointArray with the resulting points.
//! Symbolically, given transform [R t],
//! each returned point is of the form R*p + t, where p is a column vector.
//!
//! @param [in,out] pointArray array of points to be multiplied
//! @param [in] numPoint The number of points
//!
void Multiply  (DPoint3dP pointArray, int numPoint) const;


//! Multiplies this instance times each column vector in inPoint and places the
//! resulting points in outPoint.
//! inPoint and outPoint may be the same.
//! Symbolically, given transform [R t], each returned point has the
//! form R*p + t, where p is a column vector.
//!
//! @param [out] outPoint transformed points
//! @param [in] inPoint The input points
//! @param [in] numPoint The number of points
//!
void Multiply  (DPoint3dP outPoint, DPoint3dCP inPoint, int numPoint) const;


//!
//! Multiplies this instance times each column vector in inPoint,
//! using the transpose of the matrix part of this instance in the multiplications,
//! and places the resulting points in outPoint.
//! Symbolically, given transform [R t], each returned point has the equivalent
//! form p*R + t, where p is a row vector.
//! inPoint and outPoint may be the same.
//!
//! @param [out] outPoint transformed points
//! @param [in] inPoint The input points
//! @param [in] numPoint The number of points
//!
void MultiplyTranspose  (DPoint3dP outPoint, DPoint3dCP inPoint, int numPoint) const;


//!
//! Multiplies this instance times each column vector in pointArray,
//! using the transpose of the matrix part of this instance in the multiplications,
//! and replaces pointArray with the resulting points.
//! Symbolically, given transform [R t], each returned point has the equivalent
//! form p*R + t, where p is a row vector.
//!
//! @param [in,out] pointArray array of points to be multiplied
//! @param [in] numPoint The number of points
//!
void MultiplyTranspose  (DPoint3dP pointArray, int numPoint) const;



//flex|| Multiply with matrix part only || matrix.MultiplyMatrixOnly (inoutPoint) || matrix.MultiplyMatrixOnly (outPoint, inPoint) || matrix.MultiplyMatrixOnly (outPoint, x, y, z) ||

//flex|| Multiply with transposed matrix part only || matrix.MultiplyTransposeMatrixOnly (inoutPoint) || matrix.MultiplyTransposeMatrixOnly (outPoint, inPoint) || matrix.MultiplyTransposeMatrixOnly (outPoint, x, y, z) ||

//!
//! Multiplies the matrix part of this instance times the column vector
//! constructed from components x,y,z.
//! Symbolically, given transform [R t] and column vector p,
//! the returned point is R*p.
//!
//! @param [out] point result of matrix * point operation
//! @param [in] x The x component of the point
//! @param [in] y The y component of the point
//! @param [in] z The z component of the point
//!
void MultiplyMatrixOnly  (DPoint3dR point, double x, double y, double z) const;

//!
//! Multiplies the matrix part of this instance times column vector inPoint.
//! Symbolically, given transform [R t] and column vector p,
//! the returned point is R*p.
//!
//! @param [out] outPoint result of matrix * point operation
//! @param [in] inPoint The point by which matrix is multiplied
//!
void MultiplyMatrixOnly  (DPoint3dR outPoint, DPoint3dCR inPoint) const;

//!
//! Multiplies the matrix part of this instance times column vector point and
//! replaces point with the result.
//! Symbolically, given transform [R t] and column vector p,
//! the returned point is R*p.
//!
//! @param [in,out] point point to be updated
//!
void MultiplyMatrixOnly (DPoint3dR point) const;



//!
//! Multiplies the row vector constructed from components x,y,z times the matrix
//! part of this instance.
//! Symbolically, given transform [R t] and row vector p,
//! the returned point is p*R.
//!
//! @param [out] point result of point * matrix operation
//! @param [in] x The x component of the point
//! @param [in] y The y component of the point
//! @param [in] z The z component of the point
//!
void MultiplyTransposeMatrixOnly  (DPoint3dR point, double x, double y, double z) const;

//!
//! Multiplies the row vector point times the matrix
//! part of this instance, and replaces point with the result.
//! Symbolically, given transform [R t] and row vector p,
//! the returned point is p*R.
//!
//! @param [in,out] point point to be updated
//!
void MultiplyTransposeMatrixOnly (DPoint3dR point) const;


//!
//! Multiplies the row vector inPoint times the matrix
//! part of this instance.
//! Symbolically, given transform [R t] and row vector p,
//! the returned point is p*R.
//!
//! @param [out] outPoint result of point * matrix operation
//! @param [in] inPoint The point which multiplies matrix
//!
void MultiplyTransposeMatrixOnly  (DPoint3dR outPoint, DPoint3dCR inPoint) const;



//flex|| multiply mixed 2d/3d  || || transform.Multiply (outPoint2d, point2d) || transform.Multiply (outPoints2d[], inPoints2d[], n) ||

//! @description Returns the product of a transform times a point, using only the leading 2x2 part of the matrix part
//! @param [out] result returned point.
//! @param [in] point The input point.
void Multiply (DPoint2dR       result, DPoint2dCR      point) const;



//!
//! Multiplies this instance times each column vector in inPoint and places the
//! resulting points in outPoint.
//! inPoint and outPoint may be the same.
//! All z parts (i.e. last row and column) of this instance are ignored.
//!
//! @param [out] outPoint transformed points
//! @param [in] inPoint The input points
//! @param [in] numPoint The number of points
//!
void Multiply  (DPoint2dP outPoint, DPoint2dCP inPoint, int numPoint) const;

//flex|| ||  || transform.Multiply (outPoint3d, point2d) || transform.Multiply (outPoints3d[], inPoints2d[], n) ||

//! @description Returns the product of a transform times a point, using only the leading 2x3 part of the matrix part
//! @param [out] result returned point.
//! @param [in] point The input point.
void Multiply (DPoint3dR       result, DPoint2dCR      point) const;

//!
//! Multiplies this instance times each column vector in inPoint and places the
//! resulting points in outPoint.
//! Each input point is given a z=0.
//!
//! @param [out] outPoint transformed points
//! @param [in] inPoint The input points
//! @param [in] numPoint The number of points
//!
void Multiply  (DPoint3dP outPoint, DPoint2dCP inPoint, int numPoint) const;

//flex|| ||  || transform.Multiply (outPoint2d, point3d) || transform.Multiply (outPoints2d[], inPoints3d[], n) ||


//! @description Returns the product of a transform times a point, using only the leading xy rows (but xyz columns) of the transform
//! @param [out] result returned point.
//! @param [in] point The input point.
void Multiply (DPoint2dR       result, DPoint3dCR      point) const;

//! Multiplies this instance times each column vector in inPoint and places the
//! x and y components of the resulting points in outPoint.
//!
//! @param [out] outPoint transformed points
//! @param [in] inPoint The input points
//! @param [in] numPoint The number of points
//!
void Multiply  (DPoint2dP outPoint, DPoint3dCP inPoint, int numPoint) const;




//flex|| 4d point || transform.Multiply (outPoint4d, point4d) || || transform.Multiply (outPoints4d[], inPoints4d, n) ||

//! @description Returns the product of a transform times a homogeneous point.
//! (Transform is extended by a 0001 row.)
//! @param [out] result returned point.
//! @param [in] point The input point.
void Multiply  (DPoint4dR result, DPoint4dCR point) const;

//!
//! Multiplies this instance times each column vector in inPoint and places the
//! resulting points in outPoint.
//! inPoint and outPoint may be the same.
//! Symbolically, given transform [R t], each returned point has the
//! form R*p + t*w (with weight w), where p is a column vector and w is its
//! weight.
//!
//! @param [out] outPoint transformed points
//! @param [in] inPoint The input points
//! @param [in] numPoint The number of points
//!
void Multiply  (DPoint4dP outPoint, DPoint4dCP inPoint, int numPoint) const;


//flex|| weighted point with separate weight || transform.Multiply (inoutPoint, weight) || || transform.Multiply (outPoint[], inPoints[], weight, n) ||
//! Multiplies a "weighted point" in place.  That is, the point is input and output as
//!   (wx,wy,wz,w) where x,y,z are the cartesian image coordinates.
//! @param [in,out] point point to be updated
//! @param [in] weight The weight
void MultiplyWeighted  (DPoint3dR point, double weight) const;

//! Multiplies an array of "weighted points" in place.  That is, the point is input and output as
//!   (wx,wy,wz,w) where x,y,z are the cartesian image coordinates.
//! @param [in] outPoint The transformed points.
//! @param [in] inPoint The original points
//! @param [in] pWeight The weight array.  If null, unit weight is used.
//! @param [in] numPoint number of points in arrays
void MultiplyWeighted  (DPoint3dP outPoint, DPoint3dCP inPoint, const double *pWeight, int numPoint) const;


//flex|| Solve transform * outPoint = point  || || bool transform.Solve (outPoint, point) ||
//flex|| || || bool transform.Solve (outPoints[], points[], n) ||

//! Solves the linear system Tx=b, where T is this instance, b is the input
//! point and x is the output point.  No simplifying assumptions are made
//! regarding the matrix part of T.  Symbolically, if T = [M t], then
//! x = Q (b - t), where Q is the inverse of M (i.e., the system is equivalent
//! to Mx = b - t).  inPoint and outPoint may have identical addresses.
//! @param [out] outPoint solution to system
//! @param [in] inPoint The constant point of the system
//! @return false if the matrix part of this instance is singular.
bool Solve  (DPoint3dR outPoint, DPoint3dCR inPoint) const;

//! Solves the linear systems TX=B, where T is this instance, B is the matrix
//! of numPoints input points and X is the matrix of numPoints output points.
//! No simplifying assumptions are made regarding the matrix part of T.
//! Symbolically, if T = [M t], then for each input/output point i,
//! X[i] = Q (B[i] - t), where Q is the inverse of M (i.e., the i_th system is
//! equivalent to MX[i] = B[i] - t).  inPoint and outPoint may have identical
//! addresses.
//! @param [out] outPoint column points of solution matrix to system
//! @param [in] inPoint The column points of constant matrix of system
//! @param [in] numPoints The number of input/output points
//! @return false if the matrix part of this instance is singular.
bool SolveArray  (DPoint3dP outPoint, DPoint3dCP inPoint, int numPoints) const;


//flex|| Range after multiplying all 8 corners of input range || || transform.Multiply (outRange, inRange) ||
//!
//! Computes the 8 corner points of the range cube defined by the two points
//! of pRange, multiplies pTransform times each point, and computes the
//! twopoint range limits (min max) of the resulting rotated box.
//! Note that the volume of the rotated box is generally larger than
//! that of the input because the postrotation minmax operation expands
//! the box.
//!
//! @param [out] outRange range of transformed cube
//! @param [in] inRange The any two corners of original cube
//!
void Multiply
(
DRange3dR       outRange,
DRange3dCR      inRange
) const;

//! Trasnform an ellipse
//flex|| Apply transform to an REF(DEllipse3d) || transform.Multiply (inoutEllipse) || transform.Multiply (outEllipse, inEllipse) ||
//! @param inoutEllipse ellipse to transform in place
void Multiply (DEllipse3dR inoutEllipse) const;
//! Trasnform an ellipse
//! @param outEllipse destination ellipse
//! @param inEllipse source ellipse
void Multiply (DEllipse3dR outEllipse, DEllipse3dCR inEllipse) const;

//! Trasnform a plane.
//flex|| Apply transform to an REF(DPlane3d) || transform.Multiply (inoutPlane) || transform.Multiply (outPlane, inPlane) ||
//! @param inoutPlane plane to transform in place
//! @return false if the matrix part is singular.
bool Multiply (DPlane3dR inoutPlane) const;
//! Trasnform an plane
//! @param outPlane destination plane
//! @param inPlane source plane
//! @return false if the matrix part is singular.
bool Multiply (DPlane3dR outPlane, DPlane3dCR inPlane) const;

//! Trasnform a segment
//flex|| Apply transform to an REF(DSegment3d) || transform.Multiply (inoutSegment) || transform.Multiply (outSegment, inSegment) ||
//! @param inoutSegment segment to transform in place
void Multiply (DSegment3dR inoutSegment) const;
//! Trasnform an segment
//! @param outSegment destination segment
//! @param inSegment source segment
void Multiply (DSegment3dR outSegment, DSegment3dCR inSegment) const;

//! Trasnform a ray
//flex|| Apply transform to an REF(DRay3d) || transform.Multiply (inoutRay) || transform.Multiply (outRay, inRay) ||
//! @param inoutRay ray to transform in place
void Multiply (DRay3dR inoutRay) const;
//! Trasnform an ray
//! @param outRay destination ray
//! @param inRay source ray
void Multiply (DRay3dR outRay, DRay3dCR inRay) const;




//flex|| Transform plane ax+by+cz=d    || transform.TransformImplicitPlane (outA, outB, outC, outD, inA, inB, inC, inD) ||
//!
//!
//! Transform the a,b,c,d components for an implicit plane.
//! The plane equation format is ax+by+cz=d.
//!
//! @param [out] aOut x coefficient in transformed plane equation
//! @param [out] bOut y coefficient in transformed plane equation
//! @param [out] cOut z coefficient in transformed plane equation
//! @param [out] dOut constant coefficient for equation.
//! @param [in] a The x coefficient in plane equation
//! @param [in] b The y coefficient in plane equation
//! @param [in] c The z coefficient in plane equation
//! @param [in] d The constant on right hand side of plane equation
//!
bool TransformImplicitPlane  (double &aOut, double &bOut, double &cOut, double &dOut, double a, double b, double c, double d) const;



//! @description Multiply weighted points.
//! @remark This operation changes the <wx,wy,wz> parts but does not affect the weights.
//! @param [out] weightedXYZOut transformed weighted coordinates.
//! @param [in]  weightedXYZIn original weighted coordinates.
//! @param [in] weights weights.
void MultiplyWeighted (bvector<DPoint3d>&weightedXYZOut, bvector<DPoint3d>const &weightedXYZIn, bvector <double> const *weights) const;

//! @description Multiply weighted points.,wy,wz> parts but does not affect the weights.
//! @param [out] xyzwOut transformed weighted coordinates.
//! @param [in]  xyzwIn original weighted coordinates.
void Multiply (bvector<DPoint4d> &xyzwOut, bvector<DPoint4d> const &xyzwIn) const;
//! @description Mutliply 3D points.
//! @param [out] xyzOut transformed points.
//! @param [in] xyzIn original points.
void Multiply (bvector<DPoint3d> &xyzOut, bvector<DPoint3d> const &xyzIn) const;
//! @description Mutliply 3D points in place
//! @param [out] xyzInOut transformed points.
void Multiply (bvector<DPoint3d> &xyzInOut) const;
//! @description Mutliply 2D points.
//! @param [out] xyOut transformed points..   z parts from transform are ignored.
//! @param [in] xyIn original points.  Implicitly z=0 for mutliplication.
void Multiply (bvector<DPoint2d> &xyOut, bvector<DPoint2d> const &xyIn) const;
//! @description Mutliply 3D points.
//! @param [out] xyzOut transformed points.
//! @param [in] xyIn original points.  Implicitly z=0 for mutliplication.
void Multiply (bvector<DPoint3d> &xyzOut, bvector<DPoint2d> const &xyIn) const;
//! @description Mutliply 3D points.
//! @param [out] xyOut transformed points.  z row from transform is ignored.
//! @param [in] xyzIn original points.
void Multiply (bvector<DPoint2d> &xyOut, bvector<DPoint3d> const &xyzIn) const;
//! @description Solve {transform * xyzOut[i] = xyzIn[i]} for each point in the arrays.
//!  (i.e. multiply by the inverse array.)
//! @param [out] xyzOut solution points.
//! @param [in] xyzIn original points.
bool SolveArray (bvector<DPoint3d>&xyzOut, bvector<DPoint3d>const &xyzIn) const;
//flex!! Operational queries

//flex|| Identity? || transform.IsIdentity () ||
//!
//! @description Returns true if the transform is the identity transform.
//! @return true if the transformation is within tolerance of the identity.
//!
bool IsIdentity () const;

//flex|| Rigid (pure rotation) || transform.IsRigid () || transform.IsRigidScale (scale) ||
//! @description Returns true if the matrix part of a transform is a rigid body rotation,
//! i.e. its transpose is its inverse and it has a positive determinant.
//!
//! @return true if the transformation is rigid (no scale or shear in the matrix part)
//!
bool IsRigid () const;

//! @description Returns true if the matrix part of a transform is a rigid body rotation,
//! i.e. its transpose is its inverse and it has a positive determinant.
//! @param [out] scale scale factor.
//! @return true if the transformation is rigid (no scale or shear in the matrix part)
//!
bool IsRigidScale (double &scale) const;

//flex|| close to rigid with scale factor? || transform.IsNearRigidScale (outRigidScale, primaryAxis, secondaryAxis, tolerance) ||
//! Clean up a transform that is close to a pure rotate and scale.
//! If source is NOT near to a rigid scale, return false and copy to the dest.
//! If near an identity return identity.
//! If nearly perpendicular with scales other than 1, clean preserving the length and direction of the primary axis.
//! This is intended to be used with a crude (e.g. 1.0e-6) reltol to identify old DGN file matrices that are "dirty" by modern standards but were 
//!   meant to be identity, rotation, or scaled rotations in the UOR era.
//! @param [in] dest result
//! @param [in] primaryAxis axis whose orientation and direction is preserved.
//! @param [in] tolerance relative tolerance for recognizing near-perpendicular conditions.
bool IsNearRigidScale (TransformR dest, int primaryAxis = 0, double tolerance = 1.0e-6) const;

//flex|| Acts like a 2d transform in some planes? || transform.IsPlanar (outPlaneNormal) ||
//! @description Returns true if transformation effects are entirely within the plane
//!       with given normal.
//!
//! @param [in] normal The plane normal
//! @return true if the transform has no effects perpendicular to planes with the given normal.
//!
bool IsPlanar (DVec3dCR normal) const;

//flex|| simple translation ? || transform.IsTranslate (outTranslation) ||
//!
//! @description Returns true if the transform is a simple translation.
//! @param [out] translation the translation vector. Zero of not a translation transform.
//! @return true if the transformation is a pure translation.
//!
bool IsTranslate (DPoint3dR translation) const;

//flex|| scale about a fixed point || transform.IsUniformScale (outFixedPoint, outScale) ||
//! @description Returns true if the transform is a uniform scale with scale factor other than 1.0.
//! @param [out] fixedPoint (If function result is true) the (one) point which
//!                           remains in place in the transformation.
//! @param [out] scale The scale factor.  If the transform is not a scale, this is returned as 1.0.
//! @return true if the transformation is a uniform scale.
//!
bool IsUniformScale
(
DPoint3dR       fixedPoint,
double          &scale
) const;

//flex|| rotate around a line? || transform.IsRotateAroundLine (outLineOrigin, outLineDirection, outRadians) ||
//!
//! @description Returns true if the transform is a non-zero rotation around a line.
//! @param [out] fixedPoint a point on the line.
//! @param [out] directionVector vector in the line direction.
//! @param [out] radians rotation angle in radians.
//! @return true if the transformation is a non-zero rotation.
//!
bool IsRotateAroundLine
(
DPoint3dR       fixedPoint,
DVec3dR       directionVector,
double          &radians
) const;

//flex|| Mirror across a plane?  || transform.IsMirrorAboutPlane (outPlaneOrigin, outUnitNormal) ||
//!
//! @description Returns true if the transform is a mirror with respect to
//!       a plane.
//! @param [out] planePoint Some point on the plane.
//! @param [out] unitNormal unit vector perpendicular to the plane.
//! @return true if the transformation is a mirror.
//!
bool IsMirrorAboutPlane
(
DPoint3dR       planePoint,
DVec3dR       unitNormal
) const;


//flex|| scale and rotate around a line? || transform.IsUniformScaleAndRotateAroundLine (outLineOrigin, outLineDirection, outRadians, outScale) ||
//!
//! @description Returns true if the transform is a uniform scale combined with
//!       a rotation.  One, but not both, of the two steps may be null (unit scale or no rotation)
//!
//! @param [out] fixedPoint fixed point of scaling.  This is also a point on the
//!               line.
//! @param [out] directionVector vector in the direction of the rotation.
//! @param [out] radians rotation angle
//! @param [out] scale scale factor.
//! @return true if the transformation has at least one of the scale, rotate effects.
//!
bool IsUniformScaleAndRotateAroundLine
(
DPoint3dR       fixedPoint,
DVec3dR       directionVector,
double          &radians,
double          &scale
) const;


//! Return true if the transform a combination of only 2 thing: (1) move origin, (2) rotate around Z
//! @param [out] origin origin of frame.
//! @param [out] rigidAxes unit-length axes.
//! @param [out] scale scale factor on the original axes.
//! @param [out] radians positive rotation around Z
bool IsTranslateScaleRotateAroundZ
(
	DPoint3dR origin,
	RotMatrixR rigidAxes,
	double &scale,
	double &radians
) const;

//flex|| Is there any single point that stays fixed? || bool transform.GetAnyFixedPoint (outPoint) ||
//flex|| Is there a single line that stays fixed? || bool transform.GetFixedLine (outOrigin, outDirectionVector) ||
//flex|| Is there a single plane that stays fixed? || bool transform.GetFixedPlane (outOrigin, outPlaneXVector, outPlaneYVector) ||
//flex|| || bool transform.GetFixedPlane (outResidualTransform, outMirrorTransform, outPlaneOrigin, outPlaneNormal) ||

//! @description Compute any single point that remains unchanged by action of
//!       a transform.   Note that a pure translation has no fixed points,
//!       while any other transformation does.
//! @param [out] fixedPoint Point that is not changed by the transformation.
//! @return true if the transformation has a fixed point.
//!
bool GetAnyFixedPoint (DPoint3dR fixedPoint) const;

//!
//! @description Compute the line (if any) of points that are not affected by
//!       this transformation.  Returns false if the fixed point set for the
//!       transform is empty, a single point, a plane, or all points.
//! @param [out] fixedPoint A point on the line.
//! @param [out] directionVector vector along the line.
//! @return true if the transformation has a fixed point.
//!
bool GetFixedLine (DPoint3dR fixedPoint, DVec3dR directionVector) const;

//!
//! @description Compute the plane (if any) of points that are not affected by
//!       this transformation.  Returns false if the fixed point set for the
//!       transform is empty, a single point, a line, or all points.
//! @param [out] fixedPoint A point on the line.
//! @param [out] planeVectorX a unit vector in the plane.
//! @param [out] planeVectorY another unit vector in the plane,
//!               perpendicular to pDirectionVectorX.
//! @return true if the transformation has a fixed point.
//!
bool GetFixedPlane  (DPoint3dR fixedPoint, DVec3dR planeVectorX, DVec3dR planeVectorY) const;

//!
//! @description Factor a combination of a mirror part and a non-mirror part,
//!           attempting to use a common fixed point for the two parts.
//! Equationally, the old transform T0 becomes
//! <pre>
//!            T0 = T1 * M
//! </pre>
//! where M is the mirror, and T1 is a mirror-free transform.
//! The mirror transform is returned in both matrix form and as plane point with normal.
//! In an order of operations view, a point X is transformed as
//! <pre>
//!            T0 * X = T1 * M * X
//! </pre>
//! That is, X mirrored first, followed by rotation and scaling in the residual transform.
//!
//! @param [out] residualTransform the residual transform.
//! @param [out] mirrorTransform the mirror transform.
//! @param [out] planePoint A fixed point of the mirror transform.
//! @param [out] planeNormal Unit normal for the mirror plane.
//! @return false if the transform has no mirror effects.  In this case the mirror transform is the identity,
//!       the residual transform is the original transform, and the fixed point and normal are both zero.
//!
bool GetFixedPlane  (TransformR residualTransform, TransformR mirrorTransform, DPoint3dR planePoint, DVec3dR planeNormal) const;




//!
//! Sets this instance to a transformation that has the same matrix part
//! as transform in and a translation part that is the SUM of the
//! translation part of in plus the product of the
//! matrix part of in times the given point.
//! If the translation part of in is interpreted as the
//! origin of a coordinate system (whose axis directions and sizes
//! are given by the columns of the matrix part), this instance
//! becomes a coordinate frame with the same axis directions and sizes,
//! but with the origin shifted to point x,y,z of the in
//! system.  That is, x,y,z are the local coordinates of the new
//! origin, and the translation part of this instance becomes the global
//! coordinates of the new origin.
//! in may be identical to this instance.
//! Symbolically, if in is the transform [R t] and the local
//! origin coordinates x,y,z are in column vector p, the result is
//! the transformation [R t+R*p].
//!
//! @param [in] in The input transformation
//! @param [in] x The x-coordinate of the local origin
//! @param [in] y The y-coordinate of the local origin
//! @param [in] z The z-coordinate of the local origin
//!
void TranslateInLocalCoordinates
(
TransformCR     in,
double          x,
double          y,
double          z
);






//!
//! Sets this instance to the transformation obtained by premultiplying
//! inTransform by 3 matrix rotations about principle axes, given by the
//! angles xrot, yrot and zrot.
//! inTransform may be the same as this instance.
//! Symbolically, given transform [M t] and rotation
//! matrices X,Y,Z, the resulting transform is [X*Y*Z*M X*Y*Z*t]
//!
//! @param [in] inTransform The base transformation
//! @param [in] xrot The x axis rotation, in radians
//! @param [in] yrot The y axis rotation, in radians
//! @param [in] zrot The z axis rotation, in radians
//!
void InitFromPrincipleAxisRotations
(
TransformCR     inTransform,
double          xrot,
double          yrot,
double          zrot
);

//! Scale the columns of the matrix part by respective factors.
//! Translation part is unaffected.  (See also scaleMatrixRows, scaleTransformRows)
//! @param [in] transform The input transform.
//! @param [in] xscale The x column scale factor
//! @param [in] yscale The y column scale factor
//! @param [in] zscale The z column scale factor
//!
void ScaleMatrixColumns
(
TransformCR     transform,
double          xscale,
double          yscale,
double          zscale
);

//! Construct transforms between local and world for given origin and axis directions.
//! @param [out] localToWorld transform from local system to world.
//! @param [out] worldToLocal transform from world to local system.
//! @param [in] axes coordinate directions of local system (relative to world)
//! @param [in] origin origin of local system (in world)
//! @return true if axes were independent
static bool InitForwardAndInverseFromAxesAndOrigin
(
TransformR localToWorld,
TransformR worldToLocal,
RotMatrixCR axes,
DPoint3dCR origin
);

//! Scale the columns (in place) of the matrix part by respective factors.
//! Translation part is unaffected.  (See also scaleMatrixRows, scaleTransformRows)
//! @param [in] xscale The x column scale factor
//! @param [in] yscale The y column scale factor
//! @param [in] zscale The z column scale factor
//!
void ScaleMatrixColumns
(
double         xscale,
double         yscale,
double         zscale
);

//!
//! Scale the rows of the matrix part by respective factors.
//! Translation part is unaffected.  (See also scaleMatrixColumns, scaleTransformRows)
//! @param [in] transform The input transform.
//! @param [in] xscale The x column scale factor
//! @param [in] yscale The y column scale factor
//! @param [in] zscale The z column scale factor
//!
void ScaleMatrixRows
(
TransformCR     transform,
double          xscale,
double          yscale,
double          zscale
);

//!
//! Scale the complete rows by respective factors.
//! (See also scaleMatrixColumns, scaleMatrixRows, which only act on the matrix part)
//! @param [in] transform The input transform.
//! @param [in] xscale The x column scale factor
//! @param [in] yscale The y column scale factor
//! @param [in] zscale The z column scale factor
//!
void ScaleCompleteRows
(
TransformCR     transform,
double          xscale,
double          yscale,
double          zscale
);

//! Multiply by a translation "from the left": result = (Identity , scaled translationIn) * transformIn
//! @param [in] translationIn translation vector for left term
//! @param [in] scaleFactor factor for translation
//! @param [in] transformIn full transform for right factor.
void MultiplyTranslationTransform
(
DVec3dCR translationIn,
double scaleFactor,
TransformCR transformIn
);

//! Multiply by a translation "from the left": result = transformIn * (Identity , scaled translationIn)
//! @param [in] transformIn full transform for right factor.
//! @param [in] translationIn translation vector for right term
//! @param [in] scaleFactor scale factor for translation
void MultiplyTransformTranslation
(
TransformCR transformIn,
DVec3dCR translationIn,
double scaleFactor
);

//! Input transforms placed "within" caller's geoemtry so that the geometry range
//! satisfies origin and scaling requirements indicatd by frame type.
//! Expected usage is that a local coordinate frame is placed on some geometry
//! and the range of the geometry is them computed within that local system.
//! The coordinate frame then needs some combination of (a) move origin to lower left of the range,
//!  (b) rescale axes so that one or both of x,y directions scale to 01.
//! @param [in,out] localToWorld local to world transformation to modify.
//! @param [in,out] worldToLocal world to lcoal transformation to modify.
//! @param [in,out] localRange range of subject data.
//! @param [in] frameType identifies target scaling and origin.
//! @param [out] appliedTransformOldLocalToNewLocal (optional) the transfrom applied to modify initial local geometry to final  local geometry
static void CorrectCoordinateFrameXYRange
(
TransformR localToWorld,
TransformR worldToLocal,
DRange3dR localRange,
LocalCoordinateSelect frameType,
TransformP appliedTransformOldLocalToNewLocal = NULL
);


//! scale all values in an array of doubles by the magnitude of a specified column of the matrix.
//! @param [in] data array of doubles.
//! @param [in] n   number of values.
void ScaleDoubleArrayByXColumnMagnitude (double *data, int n) const;


//!
//! Adds column i of the matrix part of this instance to point in and places
//! the result in out.
//!
//! @param [out] out sum of in and column i
//! @param [in] in The base point for sum
//! @param [in] i The column index of matrix
//!
void OffsetPointByColumn  (DPoint3dR out, DPoint3dCR in, int i) const;


/*__PUBLISH_SECTION_END__*/



// MultiplyTranspose is deprecated ....
//!
//! Multiplies this instance times the column vector point and replaces point
//! with the result, using the transpose of the matrix part of this instance in
//! the multiplication.
//! Symbolically, this is equivalent to being given transform [R t] and row
//! vector p, and returning the point p*R + t.
//!
//! @param [in,out] point point to be updated
//!
void MultiplyTranspose (DPoint3dR point) const;
//!
//! Multiplies this instance times the column vector constructed from components
//! x,y,z, using the transpose of the matrix part of this instance in the
//! multiplication.
//! Symbolically, this is equivalent to being given transform [R t] and row
//! vector p, and returning the point p*R + t.
//!
//! @param [out] point result of transformation * point operation
//! @param [in] x The x component of the point
//! @param [in] y The y component of the point
//! @param [in] z The z component of the point
//!
void MultiplyTranspose
(
DPoint3dR       point,
double          x,
double          y,
double          z
) const;
/*__PUBLISH_SECTION_START__*/

#endif // __cplusplus
};
END_BENTLEY_NAMESPACE

#endif // transform_H_
