/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/internal/dtransform3d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
/*__PUBLISH_SECTION_START__*/


BEGIN_BENTLEY_GEOMETRY_NAMESPACE
//!
//! 3d-to-3d transformation with matrix and translation parts directly addressible as public members.
//!
struct GEOMDLLIMPEXP _dTransform3d
{
DMatrix3d    matrix;
DPoint3d     translation;

#ifdef __cplusplus
//!
//! Copy all entries from mdl-style matrix to DTransform3d
//!
void initFromTransform (TransformCP mdlTransformP);


//!
//! Scale the columns of the matrix part by respective factors.
//! Translation part is unaffected.  (See also scaleMatrixRows, scaleTransformRows)
//! @param [in] pTransform The input transform.  If NULL, an identity transform is implied.
//! @param [in] xscale The x column scale factor
//! @param [in] yscale The y column scale factor
//! @param [in] zscale The z column scale factor
//!
void scaleMatrixColumns
(
DTransform3dCP pTransform,
double         xscale,
double         yscale,
double         zscale

);


//!
//! @description Returns a value from a specified row and column of the matrix part of the transformation.
//! @param [in] row The index of row to read.  Row indices are 0, 1, 2.
//! @param [in] col The index of column to read.  Column indices are 0, 1, 2.
//!
double getFromMatrixByRowAndColumn
(
int            row,
int          col
) const;

//!
//! @description Returns a value from a specified component of the point (translation) part of the transformation.
//! @param [in] row The index of point component to read.  Indices are 0, 1, 2 for x, y, z
//!
double getPointComponent (int row) const;

//!
//! @description Multiplies a point by a transform, returning the result in
//!   place of the input point.
//! @param [in,out] pPoint point to be updated
//!
void multiply (DPoint3dP pPoint) const;

//!
//! @description Returns the product of a transform times a point.
//! @param [out] pResult returned point.
//! @param [in] pPoint The input point.
//!
void multiply
(
DPoint3dP pResult,
DPoint3dCP pPoint
) const;

//!
//! Multiplies a "weighted point" in place.  That is, the point is input and output as
//!   (wx,wy,wz,w) where x,y,z are the cartesian image coordinates.
//!
//! @param [in,out] pPoint point to be updated
//! @param [in] weight The weight
//!
void multiplyWeighted
(
DPoint3dP pPoint,
double          weight
) const;

//!
//! Multiplies an array of "weighted points" in place.  That is, the point is input and output as
//!   (wx,wy,wz,w) where x,y,z are the cartesian image coordinates.
//!
//! @param [in] pOutPoint The transformed points.
//! @param [in] pInPoint The original points
//! @param [in] pWeight The weight array.  If null, unit weight is used.
//!
void multiplyWeighted
(
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
const double    *pWeight,
int             numPoint
) const;

//!
//! Multiplies this instance times the column vector pPoint and replaces pPoint
//! with the result, using the transpose of the matrix part of this instance in
//! the multiplication.
//! Symbolically, this is equivalent to being given transform [R t] and row
//! vector p, and returning the point p*R + t.
//!
//! @param [in,out] pPoint point to be updated
//!
void multiplyTranspose (DPoint3dP pPoint) const;

//!
//! @description Returns the product of a matrix times a point, with the point
//!       specified as explict x, y, and z values.
//! @param [out] pPoint result of transformation * point operation
//! @param [in] x The x component of the point
//! @param [in] y The y component of the point
//! @param [in] z The z component of the point
//!
void multiply
(
DPoint3dP pPoint,
double         x,
double         y,
double         z
) const;

//!
//! Multiplies this instance times the column vector constructed from components
//! x,y,z, using the transpose of the matrix part of this instance in the
//! multiplication.
//! Symbolically, this is equivalent to being given transform [R t] and row
//! vector p, and returning the point p*R + t.
//!
//! @param [out] pPoint result of transformation * point operation
//! @param [in] x The x component of the point
//! @param [in] y The y component of the point
//! @param [in] z The z component of the point
//!
void multiplyTranspose
(
DPoint3dP pPoint,
double         x,
double         y,
double         z
) const;

//!
//!
//! Transform the a,b,c,d components for an implicit plane.
//! The plane equation format is ax+by+cz=d.
//!
//! @param [out] paOut x coefficient in transformed plane equation
//! @param [out] pbOut y coefficient in transformed plane equation
//! @param [out] pcOut z coefficient in transformed plane equation
//! @param [out] pcOut transformed right hand side constant
//! @param [in] a The x coefficient in plane equation
//! @param [in] b The y coefficient in plane equation
//! @param [in] c The z coefficient in plane equation
//! @param [in] d The constant on right hand side of plane equation
//!
bool    transformImplicitPlane
(
double      *paOut,
double      *pbOut,
double      *pcOut,
double      *pdOut,
double      a,
double      b,
double      c,
double      d
) const;

//!
//! Multiplies the matrix part of this instance times the column vector
//! constructed from components x,y,z.
//! Symbolically, given transform [R t] and column vector p,
//! the returned point is R*p.
//!
//! @param [out] pPoint result of matrix * point operation
//! @param [in] x The x component of the point
//! @param [in] y The y component of the point
//! @param [in] z The z component of the point
//!
void multiplyMatrixOnly
(
DPoint3dP pPoint,
double         x,
double         y,
double         z
) const;

//!
//! Multiplies the matrix part of this instance times column vector pInPoint.
//! Symbolically, given transform [R t] and column vector p,
//! the returned point is R*p.
//!
//! @param [out] pOutPoint result of matrix * point operation
//! @param [in] pInPoint The point by which matrix is multiplied
//!
void multiplyMatrixOnly
(
DPoint3dP pOutPoint,
DPoint3dCP pInPoint
) const;

//!
//! Multiplies the matrix part of this instance times column vector pPoint and
//! replaces pPoint with the result.
//! Symbolically, given transform [R t] and column vector p,
//! the returned point is R*p.
//!
//! @param [in,out] pPoint point to be updated
//!
void multiplyMatrixOnly (DPoint3dP pPoint) const;

//!
//! Multiplies the row vector constructed from components x,y,z times the matrix
//! part of this instance.
//! Symbolically, given transform [R t] and row vector p,
//! the returned point is p*R.
//!
//! @param [out] pPoint result of point * matrix operation
//! @param [in] x The x component of the point
//! @param [in] y The y component of the point
//! @param [in] z The z component of the point
//!
void multiplyTransposeMatrixOnly
(
DPoint3dP pPoint,
double         x,
double         y,
double         z
) const;

//!
//! Multiplies the row vector pInPoint times the matrix
//! part of this instance.
//! Symbolically, given transform [R t] and row vector p,
//! the returned point is p*R.
//!
//! @param [out] pOutPoint result of point * matrix operation
//! @param [in] pInPoint The point which multiplies matrix
//!
void multiplyTransposeMatrixOnly
(
DPoint3dP pOutPoint,
DPoint3dCP pInPoint
) const;

//!
//! Multiplies the row vector pPoint times the matrix
//! part of this instance, and replaces pPoint with the result.
//! Symbolically, given transform [R t] and row vector p,
//! the returned point is p*R.
//!
//! @param [in,out] pPoint point to be updated
//!
void multiplyTransposeMatrixOnly (DPoint3dP pPoint) const;

//!
//! Multiplies this instance times each column vector in pPointArray
//! and replaces pPointArray with the resulting points.
//! Symbolically, given transform [R t],
//! each returned point is of the form R*p + t, where p is a column vector.
//!
//! @param [in,out] pPointArray array of points to be multiplied
//! @param [in] numPoint The number of points
//!
void multiply
(
DPoint3dP pPointArray,
int            numPoint
) const;

//!
//! Multiplies this instance times each column vector in pPointArray,
//! using the transpose of the matrix part of this instance in the multiplications,
//! and replaces pPointArray with the resulting points.
//! Symbolically, given transform [R t], each returned point has the equivalent
//! form p*R + t, where p is a row vector.
//!
//! @param [in,out] pPointArray array of points to be multiplied
//! @param [in] numPoint The number of points
//!
void multiplyTranspose
(
DPoint3dP pPointArray,
int            numPoint
) const;

//!
//! Multiplies this instance times each column vector in pInPoint and places the
//! resulting points in pOutPoint.
//! pInPoint and pOutPoint may be the same.
//! Symbolically, given transform [R t], each returned point has the
//! form R*p + t*w (with weight w), where p is a column vector and w is its
//! weight.
//!
//! @param [out] pOutPoint transformed points
//! @param [in] pInPoint The input points
//! @param [in] numPoint The number of points
//!
void multiply
(
DPoint4dP pOutPoint,
DPoint4dCP pInPoint,
int            numPoint
) const;

//!
//! Multiplies this instance times each column vector in pInPoint and places the
//! resulting points in pOutPoint.
//! pInPoint and pOutPoint may be the same.
//! Symbolically, given transform [R t], each returned point has the
//! form R*p + t, where p is a column vector.
//!
//! @param [out] pOutPoint transformed points
//! @param [in] pInPoint The input points
//! @param [in] numPoint The number of points
//!
void multiply
(
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
int            numPoint
) const;

//!
//! Multiplies this instance times each column vector in pInPoint and places the
//! resulting points in pOutPoint.
//! pInPoint and pOutPoint may be the same.
//! Symbolically, given transform [R t], each returned point has the
//! form R*p + t, where p is a column vector.
//!
//! @param [out] pOutPoint transformed points
//! @param [in] pInPoint The input points
//! @param [in] numPoint The number of points
//!
void multiply
(
FPoint3dP pOutPoint,
FPoint3dCP pInPoint,
int            numPoint
) const;

//!
//! Multiplies this instance times each column vector in pInPoint,
//! using the transpose of the matrix part of this instance in the multiplications,
//! and places the resulting points in pOutPoint.
//! Symbolically, given transform [R t], each returned point has the equivalent
//! form p*R + t, where p is a row vector.
//! pInPoint and pOutPoint may be the same.
//!
//! @param [out] pOutPoint transformed points
//! @param [in] pInPoint The input points
//! @param [in] numPoint The number of points
//!
void multiplyTranspose
(
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
int            numPoint
) const;

//!
//! Multiplies this instance times each column vector in pInPoint and places the
//! resulting points in pOutPoint.
//! pInPoint and pOutPoint may be the same.
//! All z parts (i.e. last row and column) of this instance are ignored.
//!
//! @param [out] pOutPoint transformed points
//! @param [in] pInPoint The input points
//! @param [in] numPoint The number of points
//!
void multiply
(
DPoint2dP pOutPoint,
DPoint2dCP pInPoint,
int            numPoint
) const;

//!
//! Multiplies this instance times each column vector in pInPoint and places the
//! resulting points in pOutPoint.
//! pInPoint and pOutPoint may be the same.
//! All z parts (i.e. last row and column) of this instance are ignored.
//!
//! @param [out] pOutPoint transformed points
//! @param [in] pInPoint The input points
//! @param [in] numPoint The number of points
//!
void multiply
(
FPoint2dP pOutPoint,
FPoint2dCP pInPoint,
int            numPoint
) const;

//!
//! Multiplies this instance times each column vector in pInPoint and places the
//! resulting points in pOutPoint.
//! Each input point is given a z=0.
//!
//! @param [in] pTransform The transformation to apply
//! @param [out] pOutPoint transformed points
//! @param [in] pInPoint The input points
//! @param [in] numPoint The number of points
//!
void multiply
(
DPoint3dP pOutPoint,
DPoint2dCP pInPoint,
int            numPoint
) const;

//!
//! Multiplies this instance times each column vector in pInPoint and places the
//! x and y components of the resulting points in pOutPoint.
//!
//! @param [out] pOutPoint transformed points
//! @param [in] pInPoint The input points
//! @param [in] numPoint The number of points
//!
void multiply
(
DPoint2dP pOutPoint,
DPoint3dCP pInPoint,
int            numPoint
) const;

//!
//! @description Returns the product of two transformations.
//! Symbolically, given transforms [R t] and [S u], return the product transform
//! [R t][S u] = [R*S t+R*u].
//!
//! @param [in] pTransform1 The first factor
//! @param [in] pTransform2 The second factor
//!
void productOf
(
DTransform3dCP pTransform1,
DTransform3dCP pTransform2
);


//!
//! @description Returns the product of a matrix and a transformation.
//! The matrix acts like a transformation with a zero point as its translation part.
//!
//! @param [in] pMatrix The first factor (matrix)
//! @param [in] pTransform The second factor (transform)
//!
void productOf
(
DMatrix3dCP pMatrix,
DTransform3dCP pTransform
);


//!
//! @description returns the product of a transformation times a matrix, treating
//!   the matrix as a transformation with zero as its translation components.
//! @param [in] pTransform The first facpatentor (transform)
//! @param [in] pMatrix The second factor (matrix)
//!
void productOf
(
DTransform3dCP pTransform,
DMatrix3dCP pMatrix
);


//!
//! Computes the 8 corner points of the range cube defined by the two points
//! of pRange, multiplies pTransform times each point, and computes the
//! twopoint range limits (min max) of the resulting rotated box.
//! Note that the volume of the rotated box is generally larger than
//! that of the input because the postrotation minmax operation expands
//! the box.
//!
//! @param [out] pOutRange range of transformed cube
//! @param [in] pInRange The any two corners of original cube
//!
void multiply
(
DRange3dP pOutRange,
DRange3dCP pInRange
) const;

//!
//! @description returns an identity transformation, i.e. zero
//! translation part and identity matrix part.
//!
//!
void initIdentity ();


//!
//! @description Returns a transformation with the given matrix part and a zero
//!           translation part.
//! @param [in] pMatrix The matrix part
//!
void initFrom (DMatrix3dCP pMatrix);


//!
//! @description Returns a transformation with the given matrix and translation parts.
//! @param [in] pMatrix The matrix part
//! @param [in] pTranslation The translation part
//!
void initFrom
(
DMatrix3dCP pMatrix,
DPoint3dCP pTranslation
);


//!
//! @description Returns a transformation with identity matrix part and
//!           given translation part.
//! @param [in] pTranslation The translation part
//!
void initFrom (DPoint3dCP pTranslation);


//!
//! @description Returns a transformation with identity matrix part and
//!       translation part given as x, y, and z components.
//! @param [in] x The x-coordinate of translation part
//! @param [in] y The y-coordinate of translation part
//! @param [in] z The z-coordinate of translation part
//!
void initFrom
(
double      x,
double      y,
double      z
);


//!
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
void initFromRowValues
(
double      x00,
double      x01,
double      x02,
double      tx,
double      x10,
double      x11,
double      x12,
double      ty,
double      x20,
double      x21,
double      x22,
double      tz
);


//!
//! @description Returns a transformation of rotation about a specified line.
//! @param [in] pPoint0 The start point of the line
//! @param [in] pPoint1 The end point of the line
//! @param [in] radians The rotation angle
//!
void initFromLineAndRotationAngle
(
DPoint3dCP pPoint0,
DPoint3dCP pPoint1,
double         radians
);


//!
//! Sets this instance to a transformation that has the same matrix part
//! as transform pIn and a translation part that is the SUM of the
//! translation part of pIn plus the product of the
//! matrix part of pIn times the given point.
//! If the translation part of pIn is interpreted as the
//! origin of a coordinate system (whose axis directions and sizes
//! are given by the columns of the matrix part), this instance
//! becomes a coordinate frame with the same axis directions and sizes,
//! but with the origin shifted to point x,y,z of the pIn
//! system.  That is, x,y,z are the local coordinates of the new
//! origin, and the translation part of this instance becomes the global
//! coordinates of the new origin.
//! pIn may be identical to this instance.
//! Symbolically, if pIn is the transform [R t] and the local
//! origin coordinates x,y,z are in column vector p, the result is
//! the transformation [R t+R*p].
//!
//! @param [in] pIn The input transformation
//! @param [in] x The x-coordinate of the local origin
//! @param [in] y The y-coordinate of the local origin
//! @param [in] z The z-coordinate of the local origin
//!
void translateInLocalCoordinates
(
DTransform3dCP pIn,
double         x,
double         y,
double         z
);


//!
//! @description Overwrites the matrix part of a preexisting transformation.
//!   The translation part is unchanged.
//! @param [in] pMatrix The matrix to insert
//!
void setMatrix (DMatrix3dCP pMatrix);


//!
//! Sets the translation part of this instance to pPoint.  The prior
//! translation part is overwritten, and the matrix part is unchanged.
//! Symbolically, if pPoint is u then this instance [R t] becomes the
//! transformation [R u].
//!
//! @param [in] pPoint The vector to insert
//!
void setTranslation (DPoint3dCP pPoint);

//!
//! Sets the translation part of this instance to pPoint.  The prior
//! translation part is overwritten, and the matrix part is unchanged.
//! Symbolically, if pPoint is u then this instance [R t] becomes the
//! transformation [R u].
//!
//! @param [in] point The vector to insert
//!
void SetTranslation (DPoint3dCR point)
    {
    translation = point;
    }
//!
//! @description Set a column of the matrix part.
//!
//! @param [out] column column data
//! @param [in] index column index
//!
void SetMatrixColumn  (DVec3dCR column, int index)
    {
    index = Angle::Cyclic3dAxis (index);
    matrix.column[index] = column;
    }

//!
//! Sets the translation part of this instance to zero.  The prior
//! translation part is overwritten, and the matrix part is unchanged.
//! Symbolically, this instance [R t] becomes the transformation [R 0].
//!
//!
void zeroTranslation ();


//!
//! Sets the translation part of this instance so that it leaves point
//! pPoint unchanged, i.e. so that this instance may be interpreted as
//! applying its matrix part as a rotation or scaling about pPoint.
//! Symbolically, given transform [R t] and column vector p,
//! the returned transform is [R p-R*p].
//! (The prior translation part is destroyed, and does not affect the
//! result in any way.)
//!
//! @param [in] pPoint The point that is to remain fixed when multiplied by the modified transformation
//!
void setFixedPoint (DPoint3dCP pPoint);


//!
//! @description Returns a transformation with given matrix part, and translation
//!   part computed from the matrix and a given fixed point.  This translation part
//!   is generally different from the fixed point itself.   The resulting transformation
//!   will leave the fixed point unchanged and apply whatever effects are contained in the
//!   matrix as if the fixed point is the origin.
//! @param [in] pMatrix The matrix part
//! @param [in] pOrigin The point that is to remain fixed when multiplied by the transformation.
//!
void initFromMatrixAndFixedPoint
(
DMatrix3dCP pMatrix,
DPoint3dCP pOrigin
);


//!
//!
//! Sets this instance to a matrix which is the inverse of pIn
//! IN THE SPECIAL CASE WHERE pIn HAS ONLY PURE ROTATION OR
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
//! pIn may be the same as this instance.
//! The specific computations in this special-case inversion are (1) the
//! output transform's translation is the input transform's
//! matrix times the negative of the input transform's translation, and (2) the
//! output transform's matrix part is the tranpose of the input transform's
//! matrix part.
//! Symbolically, given transform [R t] return transform [R^ (R^)*(-t)]
//! where ^ indicates transposition.
//!
//! @param [in] pIn The input transformation
//! @see #inverseOf
//!
void invertRigidBodyTransformation (DTransform3dCP pIn);


//!
//! Sets this instance to the inverse transform of pIn.
//! pIn may be the same as this instance.
//! This is a modestly expensive floating point computation (33
//! multiplies, 14 adds).
//! Symbolically, given transform [R t] return transform [Q Q*(-t)]
//! where Q is the inverse of matrix R.
//!
//! @param [in] pIn The input transformation
//! @return true if transform is invertible
//!
bool    inverseOf (DTransform3dCP pIn);


//!
//! Solves the linear system Tx=b, where T is this instance, b is the input
//! point and x is the output point.  No simplifying assumptions are made
//! regarding the matrix part of T.  Symbolically, if T = [M t], then
//! x = Q (b - t), where Q is the inverse of M (i.e., the system is equivalent
//! to Mx = b - t).  pInPoint and pOutPoint may have identical addresses.
//!
//! @param [out] pOutPoint solution to system
//! @param [in] pInPoint The constant point of the system
//! @return false if the matrix part of this instance is singular.
//!
bool    solve
(
DPoint3dP pOutPoint,
DPoint3dCP pInPoint
) const;

//!
//! Solves the linear systems TX=B, where T is this instance, B is the matrix
//! of numPoints input points and X is the matrix of numPoints output points.
//! No simplifying assumptions are made regarding the matrix part of T.
//! Symbolically, if T = [M t], then for each input/output point i,
//! X[i] = Q (B[i] - t), where Q is the inverse of M (i.e., the i_th system is
//! equivalent to MX[i] = B[i] - t).  pInPoint and pOutPoint may have identical
//! addresses.
//!
//! @param [out] pOutPoint column points of solution matrix to system
//! @param [in] pInPoint The column points of constant matrix of system
//! @param [in] numPoints The number of input/output points
//! @return false if the matrix part of this instance is singular.
//!
bool    solveArray
(
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
int         numPoints
) const;

//!
//! Sets this instance to the transformation obtained by premultiplying
//! pInTransform by 3 matrix rotations about principle axes, given by the
//! angles xrot, yrot and zrot.
//! pInTransform may be the same as this instance.
//! Symbolically, given transform [M t] and rotation
//! matrices X,Y,Z, the resulting transform is [X*Y*Z*M X*Y*Z*t]
//!
//! @param [in] pInTransform The base transformation
//! @param [in] xrot The x axis rotation, in radians
//! @param [in] yrot The y axis rotation, in radians
//! @param [in] zrot The z axis rotation, in radians
//!
void initFromPrincipleAxisRotations
(
DTransform3dCP pInTransform,
double         xrot,
double         yrot,
double         zrot
);


//!
//! Scale the columns of the matrix part by respective factors.
//! Translation part is unaffected.  (See also scaleMatrixRows, scaleTransformRows)
//! @param [in] pTransform The input transform.  If NULL, an identity transform is implied.
//! @param [in] xscale The x column scale factor
//! @param [in] yscale The y column scale factor
//! @param [in] zscale The z column scale factor
//!
void scaleMatrixColums
(
DTransform3dCP pTransform,
double         xscale,
double         yscale,
double         zscale
);


//!
//! Scale the rows of the matrix part by respective factors.
//! Translation part is unaffected.  (See also scaleMatrixColumns, scaleTransformRows)
//! @param [in] pTransform The input transform.  If NULL, an identity transform is implied.
//! @param [in] xscale The x column scale factor
//! @param [in] yscale The y column scale factor
//! @param [in] zscale The z column scale factor
//!
void scaleMatrixRows
(
DTransform3dCP pTransform,
double         xscale,
double         yscale,
double         zscale
);


//!
//! Scale the complete rows by respective factors.
//! (See also scaleMatrixColumns, scaleMatrixRows, which only act on the matrix part)
//! @param [in] pTransform The input transform.  If NULL, an identity transform is implied.
//! @param [in] xscale The x column scale factor
//! @param [in] yscale The y column scale factor
//! @param [in] zscale The z column scale factor
//!
void scaleCompleteRows
(
DTransform3dCP pTransform,
double         xscale,
double         yscale,
double         zscale
);


//!
//! @description Returns the translation (point) part of a transformation.
//!
//! @param [out] pPoint vector part of transformation
//!
void getTranslation (DPoint3dP pPoint) const;

//!
//! @description Returns a column from the matrix part of the transformation.
//!
//! @param [out] pPoint column of matrix part.
//! @param [in] index
//!
void getMatrixColumn
(
DPoint3dP pColumn,
int         index
) const;

//!
//! @descriptioin Returns the matrix part of a transformation.
//!
//! @param [out] pMatrix matrix part of transformation
//!
void getMatrix (DMatrix3dP pMatrix) const;

//!
//! Adds column i of the matrix part of this instance to point pIn and places
//! the result in pOut.
//!
//! @param [out] pOut sum of pIn and column i
//! @param [in] pIn The base point for sum
//! @param [in] i The column index of matrix
//!
void offsetPointByColumn
(
DPoint3dP pOut,
DPoint3dCP pIn,
int             i
) const;

//!
//! Sets pPoint0 to the origin (translation part), and sets pPoint1, pPoint2
//! pPoint3 to the x, y and z points (translations of columns
//! of matrix part by origin) from this instance.
//!
//! @param [out] pPoint0 origin of transform coordinates
//! @param [out] pPoint1 100 point of transform coordinates
//! @param [out] pPoint2 010 point of transform coordinates
//! @param [out] pPoint3 001 point of transform coordinates
//!
void get4Points
(
DPoint3dP pPoint0,
DPoint3dP pPoint1,
DPoint3dP pPoint2,
DPoint3dP pPoint3
) const;

//!
//! Sets the four points in pArray to the origin (translation part), x, y and z
//! points (translations of columns of matrix part by origin) from this
//! instance.
//!
//! @param [in] pTransform The input transformation
//! @param [out] pArray origin, 100, 010, 001 points as an array
//!
void get4Points (DPoint3dP pArray) const;

//!
//! Sets pOrigin to the translation part, and sets pVector0, pVector1
//! pVector2 to the columns of this instance.
//!
//! @param [out] pOrigin origin of transform coordinates
//! @param [out] pVector0 100 vector of transform coordinates
//! @param [out] pVector1 010 vector of transform coordinates
//! @param [out] pVector2 001 vector of transform coordinates
//!
void getOriginAndVectors
(
DPoint3dP pOrigin,
DPoint3dP pVector0,
DPoint3dP pVector1,
DPoint3dP pVector2
) const;

//!
//! Sets the four points in pArray to the translation part and the columns
//! of this instance.
//!
//! @param [out] pArray origin, 100, 010, 001 vectors as an array
//!
void getOriginAndVectors (DPoint3dP pArray) const;

//!
//! @description Returns true if the transform is the identity transform.
//! @return true if the transformation is within tolerance of the identity.
//!
bool    isIdentity () const;

//!
//! @description Returns true if the matrix part of a transform is a rigid body rotation,
//! i.e. its transpose is its inverse and it has a positive determinant.
//!
//! @return true if the transformation is rigid (no scale or shear in the matrix part)
//!
bool    isRigid () const;

//!
//! @description Returns true if transformation effects are entirely within the plane
//!       with given normal.
//!
//! @param [in] pNormal The plane normal
//! @return true if the transform has no effects perpendicular to planes with the given normal.
//!
bool    isPlanar (DVec3dCP pNormal) const;

//!
//! @description Returns true if two transforms have exact (bitwise) equality.
//!
//! @param [in] pTransform2 The second transform
//! @return   true if the transforms are identical
//!
bool    isEqual (DTransform3dCP pTransform2) const;

//!
//! @description Returns true if two transformations are equal within tolerance, using
//!       separate tolerances for the matrix and point parts.
//!
bool    isEqual
(
DTransform3dCP pTransform2,
double                  matrixTolerance,
double                  pointTolerance
) const;

//!
//! @description Returns a transformation with origin at pOrigin, x-axis
//! pXVector, y-axis pYVector, and z-axis pZVector.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define only a line or plane but not a full coordinate system.
//! The axes may be skewed.
//!
//! @param [in] pOrigin The origin of transformed coordinates
//! @param [in] pXVector The 100 point of transformed coordinates
//! @param [in] pYVector The 010 point of transformed coordinates
//! @param [in] pZVector The 001 point of transformed coordinates
//!
void initFromOriginAndVectors
(
DPoint3dCP pOrigin,
DPoint3dCP pXVector,
DPoint3dCP pYVector,
DPoint3dCP pZVector
);


//!
//! @description Returns a transformation with origin at pOrigin, x-axis from
//! pOrigin to pXPoint, y-axis from pOrigin to pYPoint,
//! and z-axis from pOrigin to pZPoint.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define only a line or plane but not a full coordinate system.
//! The axes may be skewed.
//!
//! @param [in] pOrigin The origin of transformed coordinates
//! @param [in] pXPoint The 100 point of transformed coordinates
//! @param [in] pYPoint The 010 point of transformed coordinates
//! @param [in] pZPoint The 001 point of transformed coordinates
//!
void initFrom4Points
(
DPoint3dCP pOrigin,
DPoint3dCP pXPoint,
DPoint3dCP pYPoint,
DPoint3dCP pZPoint
);


//!
//! @description Returns a transformation with origin at pOrigin, x-axis from
//! pOrigin to pXPoint, y-axis from pOrigin to pYPoint,
//! and z-axis equal to the cross product of x and y axes.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define only a line or plane but not a full coordinate system.
//!
//! @param [in] pOrigin The origin of coordinate system
//! @param [in] pXPoint The 100 point of coordinate system
//! @param [in] pYPoint The 010 point of coordinate system
//!
void initFromPlaneOf3Points
(
DPoint3dCP pOrigin,
DPoint3dCP pXPoint,
DPoint3dCP pYPoint
);


//!
//! @description Returns a (possibly skewed) transformation with origin
//! pOrigin, the axis axisId towards pXPoint, and other axes perpendicular.
//! If normalize is false, all axes have length equal to the distance
//! between the two pOrigin and pXPoint.
//! The axes may be skewed.
//!
//! @param [in] pOrigin The origin of coordinate system
//! @param [in] pXPoint The target point of axis axisId of coordinate system
//! @param [in] axisId The axis that points from pOrigin to pXPoint
//! @param [in] normalize true to have coordinate system normalized
//!
void initFromPlaneNormalToLine
(
DPoint3dCP pOrigin,
DPoint3dCP pXPoint,
int            axisId,
bool           normalize
);


//!
//! Initializes a DTransform3d from the affine part of one of the
//! matrices in an DMap4d.
//! If the scale factor is 0, the transform is returned as an identity.
//! If the scale factor is other than 1, the all coefficients are divided
//! by the scale factor.
//! Return code is false if the 4'th row is significantly different from
//! (0,0,0,A).
//!
//! @param [in] pHMap The source mapping
//! @param [in] inverse true to extract the inverse, false to extract the forward
//! @return true if the DMatrix4d can be collapsed to affine form.
//!
bool    initFrom
(
DMap4dCP            pHMap,
int             inverse
);


//!
//! @description Returns a copy of a transformation.
//!
//! @param [in] pSource The source transform
//!
void copy (DTransform3dCP pSource);


//!
//! @description Returns a transformation in the xy-plane with origin pOrigin
//! and x,y-axes of given lengths.
//! The z-coordinate of the origin is zero and the z-axis is unscaled.
//!
//! @param [in] pOrigin origin of coordinate system (or null)
//! @param [in] xAxisLength The length of x-axis
//! @param [in] yAxisLength The length of y-axis
//!
void initFromOriginAndLengths
(
DPoint2dCP pOrigin,
double          xAxisLength,
double          yAxisLength
);


//!
//! @description Returns a transformation in the xy-plane with    origin pOrigin and
//! x,y-axes of the given lengths rotated counter-clockwise from standard position
//! by the given angle.
//! The z-coordinate of the origin is zero and the z-axis is unscaled.
//!
//! @param [in] pOrigin origin of coordinate system (or null)
//! @param [in] xAxisAngleRadians The ccw angle     separating x-axis from its standard position
//! @param [in] xAxisLength The length of x-axis
//! @param [in] yAxisLength The length of y-axis
//!
void initFromOriginAngleAndLengths
(
DPoint2dCP pOrigin,
double          xAxisAngleRadians,
double          xAxisLength,
double          yAxisLength
);


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
//! @param [in] pPoint The point that is to remain fixed when multiplied by the modified transformation
//!
void setFixedPoint (DPoint2dCP pPoint);


//!
//! Sets the translation part of this instance to the given point.
//! The prior translation part is overwritten (and z-coord set to zero) and the
//! matrix part is unchanged.
//! Symbolically, if pPoint is u then this instance [R t] becomes the
//! transformation [R u].
//!
//! @param [in] pPoint The point to insert
//!
void setTranslation (DPoint2dCP pPoint);


//!
//! Sets the given point to the x- and y-coordinates of the translation part
//! of this instance.
//!
//! @param [out] pPoint translation part of transformation
//!
void getTranslation (DPoint2dP pPoint) const;

//!
//! Sets this instance to a transformation in the xy-plane with origin at
//! pOrigin, x-axis pXVector and y-axis pYVector.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define a line but not a full coordinate system.
//! The axes may be skewed.
//!
//! @param [in] pOrigin The origin of transformed coordinates (or null)
//! @param [in] pXVector The 10 point of transformed coordinates
//! @param pYVector=> 01 point of transformed coordinates
//!
void initFromOriginAndVectors
(
DPoint2dCP pOrigin,
DPoint2dCP pXVector,
DPoint2dCP pYVector
);


//!
//! Sets this instance to a transformation in the xy-plane with origin at pOrigin,
//! x-axis from pOrigin to pXPoint and y-axis from pOrigin to pYPoint.
//! All axes are unnormalized.
//! There is no effort to detect zero length axes or degenerate points that
//! define a line but not a full coordinate system.
//! The axes may be skewed.
//!
//! @param [in] pOrigin The origin of transformed coordinates (or null)
//! @param [in] pXPoint The 10 point of transformed coordinates
//! @param [in] pYPoint The 01 point of transformed coordinates
//!
void initFrom3Points
(
DPoint2dCP pOrigin,
DPoint2dCP pXPoint,
DPoint2dCP pYPoint
);


//!
//! Initialize from first three rows of DMatrix4d, divided by weight entry.
//! ALWAYS copy the data, even if final row has invalid data.
//! @return false if 4th row is other than 0001
//!
bool    initFrom (DMatrix4dCP pMatrix);


//!
//! @description Returns a transformation in the xy-plane
//! with origin pOrigin, axis axisId towards pXPoint, and the other axis
//! perpendicular.
//! If normalize is false, both axes have length equal to the distance
//! between pOrigin and pXPoint.
//!
//! @param [in] pOrigin The origin of coordinate system (or null)
//! @param [in] pXPoint The target point of axis axisId of coordinate system
//! @param [in] axisId The axis (x=0, y=1) that points from pOrigin to pXPoint
//! @param [in] normalize true to have coordinate system normalized
//!
void initFrom2Points
(
DPoint2dCP pOrigin,
DPoint2dCP pXPoint,
int            axisId,
bool           normalize
);


//!
//! Sets pOrigin to the translation part, and sets pVector0 and pVector1
//! to the columns of this 2D instance.
//!
//! @param [out] pOrigin origin of transform coordinates
//! @param [out] pVector0 10 vector of transform coordinates
//! @param [out] pVector1 01 vector of transform coordinates
//!
void getOriginAndVectors
(
DPoint2dP pOrigin,
DPoint2dP pVector0,
DPoint2dP pVector1
) const;

//!
//! Sets the three points in pArray to the translation part and the columns
//! of this 2D instance.
//!
//! @param [out] pArray origin, 10, 01 vectors as an array
//!
void getOriginAndVectors (DPoint2dP pArray) const;

//!
//! @description Construct a transform which preserves both a primary column directon
//! and a secondary column plane.   Scale all columns to length of
//! primary axis.
//! @param [in] pTransform original matrix.
//! @param [in] primaryAxis axis to be retained.
//! @param [in] secondaryAxis axis defining plane to be maintained.
//!
bool    initUniformScaleApproximation
(
DTransform3dCP pTransform,
int             primaryAxis,
int             secondaryAxis
);


//!
//! @description Returns true if the transform is a simple translation.
//! @param [in] pTransform The transformation to test
//! @param [out] pTranslation the translation vector. Zero of not a translation transform.
//! @return true if the transformation is a pure translation.
//!
bool    isTranslate (DPoint3dP pTranslation) const;

//!
//! @description Returns true if the transform is a uniform scale with scale factor other than 1.0.
//! @param [in] pTransform The transformation to test
//! @param [out] pFixedPoint (If function result is true) the (one) point which
//!                           remains in place in the transformation.
//! @param [out] pScale The scale factor.  If the transform is not a scale, this is returned as 1.0.
//! @return true if the transformation is a uniform scale.
//!
bool    isUniformScale
(
DPoint3dP pFixedPoint,
double          *pScale
) const;

//!
//! @description Returns true if the transform is a non-zero rotation around a line.
//! @param [in] pTransform The transformation to test
//! @param [out] pLinePoint a point on the line.
//! @param [out] pDirectionVector vector in the line direction.
//! @param [out] pRadians rotation angle in radians.
//! @return true if the transformation is a non-zero rotation.
//!
bool    isRotateAroundLine
(
DPoint3dP pFixedPoint,
DPoint3dP pDirectionVector,
double          *pRadians
) const;

//!
//! @description Returns true if the transform is a mirror with respect to
//!       a plane.
//! @param [in] pTransform The transformation to test
//! @param [out] pPlanePoint Some point on the plane.
//! @param [out] pPlaneNormal unit vector perpendicular to the plane.
//! @return true if the transformation is a mirror.
//!
bool    isMirrorAboutPlane
(
DPoint3dP pPlanePoint,
DPoint3dP pUnitNormal
) const;

//!
//! @description Returns true if the transform is a uniform scale combined with
//!       a rotation.  One, but not both, of the two steps may be null (unit scale or no rotation)
//!
//! @param [in] pTransform The transformation to test
//! @param [out] pFixedPoint fixed point of scaling.  This is also a point on the
//!               line.
//! @param [out] pDirectionVector vector in the direction of the rotation.
//! @param [out] pRadians rotation angle
//! @param [out] pScale scale factor.
//! @return true if the transformation has at least one of the scale, rotate effects.
//!
bool    isUniformScaleAndRotateAroundLine
(
DPoint3dP pFixedPoint,
DPoint3dP pDirectionVector,
double          *pRadians,
double          *pScale
) const;

//!
//! @description Compute any single point that remains unchanged by action of
//!       a transform.   Note that a pure translation has no fixed points,
//!       while any other transformation does.
//! @param [in] pTransform The transformation to test
//! @param [out] pFixedPoint Point that is not changed by the transformation.
//! @return true if the transformation has a fixed point.
//!
bool    getAnyFixedPoint (DPoint3dP pFixedPoint) const;

//!
//! @description Compute the line (if any) of points that are not affected by
//!       this transformation.  Returns false if the fixed point set for the
//!       transform is empty, a single point, a plane, or all points.
//! @param [in] pTransform The transformation to test
//! @param [out] pFixedPoint A point on the line.
//! @param [out] pDirectionVector vector along the line.
//! @return true if the transformation has a fixed point.
//!
bool    getFixedLine
(
DPoint3dP pFixedPoint,
DPoint3dP pDirectionVector
) const;

//!
//! @description Compute the plane (if any) of points that are not affected by
//!       this transformation.  Returns false if the fixed point set for the
//!       transform is empty, a single point, a line, or all points.
//! @param [in] pTransform The transformation to test
//! @param [out] pFixedPoint A point on the line.
//! @param [out] pDirectionVectorX a unit vector in the plane.
//! @param [out] pDirectionVectorY another unit vector in the plane,
//!               perpendicular to pDirectionVectorX.
//! @return true if the transformation has a fixed point.
//!
bool    getFixedPlane
(
DPoint3dP pFixedPoint,
DPoint3dP pPlaneVectorX,
DPoint3dP pPlaneVectorY
) const;

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
//! @param [in] pTransform The transformation to test
//! @param [out] pResidualTransform the residual transform.
//! @param [out] pMirrorTransform the mirror transform.
//! @param [out] pFixedPoint A fixed point of the mirror transform.
//! @param [out] pPlaneNormal Unit normal for the mirror plane.
//! @return false if the transform has no mirror effects.  In this case the mirror transform is the identity,
//!       the residual transform is the original transform, and the fixed point and normal are both zero.
//!
bool    getFixedPlane
(
DTransform3dP pResidualTransform,
DTransform3dP pMirrorTransform,
DPoint3dP pPlanePoint,
DPoint3dP pPlaneNormal
) const;
#endif
};
END_BENTLEY_GEOMETRY_NAMESPACE
