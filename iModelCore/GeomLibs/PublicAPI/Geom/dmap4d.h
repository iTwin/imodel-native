/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/dmap4d.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/


BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/**
Forward and inverse 4x4 matrices of an invertible perspective transformation.

@ingroup GROUP_Geometry
*/
struct GEOMDLLIMPEXP DMap4d
{

//! Forward transformation.
DMatrix4d         M0;
//! Inverse transformation.
DMatrix4d         M1;
//! Internal state data.
int             mask;

#ifdef __cplusplus
/*__PUBLISH_SECTION_END__*/
private:
void SetTypeBits (int mask);
public:
/*__PUBLISH_SECTION_START__*/



//BEGIN_FROM_METHODS

//!
//!
//! Initialize and identity mapping.
//!
//!
static DMap4d FromIdentity ();

//! Direct initialization from matrices.
//! Caller is responsible for correct inverse relationship.
//! @param [in] forwardMatrix "forward" matrix.
//! @param [in] inverseMatrix "inverse" matrix.
static DMap4d From (DMatrix4dCR forwardMatrix, DMatrix4dCR inverseMatrix);

//!
//! Initialize a rotation about axis vx,vy,vz by angle whose cosine
//! and sine are (proportional to) c and s.
//!
//! @param [in] c cosine of angle
//! @param [in] s sine of angle
//! @param [in] vx x component of rotation axis
//! @param [in] vy y component of rotation axis
//! @param [in] vz z component of rotation axis
//!
static DMap4d FromRotation
(
double          c,
double          s,
double          vx,
double          vy,
double          vz
);

//!
//! Rotate about vx,yv,vz by an integer multiple of 90 degrees.
//! Providing the angle as an integer allows exact table lookup without
//! approximation of pi.
//!
//! @param [in] multiple rotation angle is multiple * 90 degrees
//! @param [in] vx x component of rotation axis
//! @param [in] vy y component of rotation axis
//! @param [in] vz z component of rotation axis
//!
static DMap4d FromQuadrantRotation
(
int             multiple,
double          vx,
double          vy,
double          vz
);

//!
//! Initialize a rotation by angle in radians about axis vx,vy,vz
//!
//! @param [in] radians angle in degrees
//! @param [in] vx x component of rotation axis
//! @param [in] vy y component of rotation axis
//! @param [in] vz z component of rotation axis
//!
static DMap4d FromRotation
(
double          radians,
double          vx,
double          vy,
double          vz
);

//!
//! Initialize a pure scaling transformation.
//! If any scale factor is zero, the corresponding inverse entry is also
//! zero.
//!
//! @param [in] ax x scale factor
//! @param [in] ay y scale factor
//! @param [in] az z scale factor
//!
static DMap4d FromScale
(
double          ax,
double          ay,
double          az
);

//!
//! Initialize a transform that is a (noninvertible) projection to a
//! principle plane.
//!
//! @param [in] height distance of plane from origin
//! @param [in] axis 0,1,2 for x,y,z normal
//!
static DMap4d FromPrincipleProjection
(
double          height,
int             axis
);

/*__PUBLISH_SECTION_END__*/

//!
//! Initialize a transform that incorporates a given origin, x direction, and additional
//! local transformation, all relative to a given transformation.
//! (This is the guts of AgentContext::PushTransformTo())
//! @param [in] origin world origin
//! @param [in] xDir world x direction
//! @param [in] pWorldToTarget map from world to target system
//!
static DMap4d FromTransformedOriginAndDirection
(
DPoint3dCR      origin,
DPoint3dCR      xDir,
DMap4dCR        targetToWorld,
DMap4dCR        localFrame
);
/*__PUBLISH_SECTION_START__*/


//!
//! Initialize a uniform scaling transformation.
//!
//! @param [in] a scale factor
//!
static DMap4d FromScale (double a);

//!
//! Initialize a translation transform.
//!
//! @param [in] tx x component of translation
//! @param [in] ty y component of translation
//! @param [in] tz z component of translation
//!
static DMap4d FromTranslation
(
double          tx,
double          ty,
double          tz
);

//END_FROM_METHODS
/*__PUBLISH_SECTION_END__*/
//BEGIN_REFMETHODS
/*__PUBLISH_SECTION_START__*/

//! Initialize and identity mapping.
void InitIdentity ();

//! Direct initialization from matrices.
//! Caller is responsible for correct inverse relationship.
//! @param [in] forwardMatrix "forward" matrix.
//! @param [in] inverseMatrix "inverse" matrix.
void InitFrom (DMatrix4dCR forwardMatrix, DMatrix4dCR inverseMatrix);

//! Initialize a transform which translates and scales along principle
//! axes so box loAP..hiAP maps to box loBP..hiBP
//!
//! @param [in] loAP corner of box A
//! @param [in] hiAP diagonally opposite corner of box A
//! @param [in] loBP corner of box B
//! @param [in] hiBP diagonally opposite corner of box B
//! @return int
//!
bool InitFromRanges
(
DPoint3dCR      loAP,
DPoint3dCR      hiAP,
DPoint3dCR      loBP,
DPoint3dCR      hiBP
);

//!
//! Initialize a rotation about axis vx,vy,vz by angle whose cosine
//! and sine are (proportional to) c and s.
//!
//! @param [in] c cosine of angle
//! @param [in] s sine of angle
//! @param [in] vx x component of rotation axis
//! @param [in] vy y component of rotation axis
//! @param [in] vz z component of rotation axis
//!
void InitFromRotation
(
double          c,
double          s,
double          vx,
double          vy,
double          vz
);

//!
//! Rotate about vx,yv,vz by an integer multiple of 90 degrees.
//! Providing the angle as an integer allows exact table lookup without
//! approximation of pi.
//!
//! @param [in] multiple rotation angle is multiple * 90 degrees
//! @param [in] vx x component of rotation axis
//! @param [in] vy y component of rotation axis
//! @param [in] vz z component of rotation axis
//!
void InitFromQuadrantRotation
(
int             multiple,
double          vx,
double          vy,
double          vz
);

//!
//! Initialize a rotation by angle in radians about axis vx,vy,vz
//!
//! @param [in] radians angle in degrees
//! @param [in] vx x component of rotation axis
//! @param [in] vy y component of rotation axis
//! @param [in] vz z component of rotation axis
//!
void InitFromRotation
(
double          radians,
double          vx,
double          vy,
double          vz
);

//!
//! Initialize a pure scaling transformation.
//! If any scale factor is zero, the corresponding inverse entry is also
//! zero.
//!
//! @param [in] ax x scale factor
//! @param [in] ay y scale factor
//! @param [in] az z scale factor
//!
void InitFromScale
(
double          ax,
double          ay,
double          az
);

//!
//! Initialize a transform that is a (noninvertible) projection to a
//! principle plane.
//!
//! @param [in] height distance of plane from origin
//! @param [in] axis 0,1,2 for x,y,z normal
//!
void InitFromPrincipleProjection
(
double          height,
int             axis
);

/*__PUBLISH_SECTION_END__*/

//!
//! Initialize a transform that incorporates a given origin, x direction, and additional
//! local transformation, all relative to a given transformation.
//! (This is the guts of AgentContext::PushTransformTo())
//! @param [in] origin world origin
//! @param [in] xDir world x direction
//! @param [in] pWorldToTarget map from world to target system
//!
void InitFromTransformedOriginAndDirection
(
DPoint3dCR      origin,
DPoint3dCR      xDir,
DMap4dCR        targetToWorld,
DMap4dCR        localFrame
);
/*__PUBLISH_SECTION_START__*/


//!
//! Initialize a uniform scaling transformation.
//!
//! @param [in] a scale factor
//!
void InitFromScale (double a);

//!
//!
//!
//! Initialize a translation transform.
//!
//! @param [in] tx x component of translation
//! @param [in] ty y component of translation
//! @param [in] tz z component of translation
//!
void InitFromTranslation
(
double          tx,
double          ty,
double          tz
);

//!
//! Initialize a transform with perspective entries for a (nonzero)
//! taper in the z direction.
//!
//! @param [in] taper taper fraction
//! @return true if an invertible map was constructed.
//!
bool InitFromTaper (double taper);


//!
//! @param [in] transform affine transformation 3x4 matrix
//! @param [in] invert true to treat this matrix as the inverse
//!                                        of the mapping, false if forward.
//! @return true if the Transfrom was invertible.
//!
bool InitFromTransform
(
TransformCR     transform,
bool            invert
);


/*__PUBLISH_SECTION_END__*/
//!
//! Extract view structure from an Map.  The values computed here will
//! probably be useful for constructing the quirky inputs of various
//! rendering systems.
//!
//! @param [out] viewingMatrix Composite view matrix
//! @param [out] viewPlaneFrame orthogonal coordinate frame with
//! <ul>
//! <li>translation = projection of eye point on view plane.
//! <li>x axis in view plane
//! <li>y axis in view plane
//! <li>z axis towards eye
//! </ul>
//! @param [out] w 1.0 / distance to eye.  0 for flat view.
//! @return true if invertible matrix was constructed.
//!
bool GetViewingMatrix
(
DMatrix4dR      viewingMatrix,
DTransform3dR   viewPlaneFrame,
double          &w
) const;
/*__PUBLISH_SECTION_START__*/

//!
//! Fill a mapping between a unit prism and homogeneous skew space.
//! Example use:
//! We want to map a unit rectangle from font space to screen, under
//! a full perspective mapping.   That is, given font space point (x,y)
//! we want A * (x,y,0,1)^T  = the visible pixel.   Also, given pixel
//! (i,j) we want Ainverse * (i,j,0,1) to map back into font space.
//! A long time ago, we were told points P0,P1,P2 which are the
//! preperspective points that correspond to font space (0,0), (1,0), and
//! (0,1).
//! Since then, P00, P10, P01 have been through a homogeneous transformation.
//! (For instance, there may be 'weight' of other than 1 on each one.)
//! The transformed (homogeneous) points are Q00, Q10, Q01
//! In device space, we do a projection in the z direction.  Hence we
//! need a 4th point Qz=(0,0,1,0).
//! Build this matrix by calling
//! jmdlDMap4d_fillHomogeneousSkewFrame (pHMap, Q00, Q10,Q01,Qz)
//!
//! @param point0001 0001 in the unit space maps here
//! @param point1001 1001 in the unit space maps here
//! @param point0101 0100 in the unit space maps here
//! @param point0010 0010 in the unit space maps here
//! @return true if the 4 points are independent
//!
bool InitFromSkewBox
(
DPoint4dCR      point0001,
DPoint4dCR      point1001,
DPoint4dCR      point0101,
DPoint4dCR      point0010
);

//!
//!
//! @param [in] z0 reference z.  Normalized projective coordinate
//!                                is 0 at this z
//! @param [in] zetaHalf controls rate of growth of normalized
//!                                projective z.   Projective z at z0/k
//!                                is k-1*zetahalf, i.e. is zetahalf at
//!                                z0/2
//!
void ZFrustum
(
double          z0,
double          zetaHalf
);

//!
//! Sets pA to the inverse of B.
//!
//! @param [in] B original mapping
//!
void InverseOf (DMap4dCR B);

//!
//! Form the product
//!   C * Binv * A * B * Cinv
//!
//! @param [in] A inside term of sandwich
//! @param [in] B middle term of sandwich
//! @param [in] C outer term of sandwich
//!
void SandwichOf
(
DMap4dCR        A,
DMap4dCR        B,
DMap4dCR        C
);

//!
//! Form the product
//!   B * A * BInv
//! A NULL for B or C skips that part.
//!
//! @param [in] A inside term of sandwich
//! @param [in] B outer term of sandwich
//!
void SandwichOfBABinverse (DMap4dCR A, DMap4dCR B);
//!
//! Form the product
//!   Binv * A * B
//!
//! @param [in] A inside term of sandwich
//! @param [in] B outer term of sandwich
//!
void SandwichOfBinverseAB (DMap4dCR A, DMap4dCR B);

//!
//! Multiply transforms
//!
//! @param [in] A transform A
//! @param [in] B transform B
//!
void InitProduct
(
DMap4dCR        A,
DMap4dCR        B
);

//!
//! Multiply transforms, selecting optional forward or inverse for each
//!
//! @param [in] A transform A
//! @param [in] invertA if true, use invese of A
//! @param [in] B transform B
//! @param [in] invertB if true, use invese of B
//!
void InitProduct
(
DMap4dCR        A,
bool            invertA,
DMap4dCR        B,
bool            invertB
);

//!
//! Checks if the mapping is strictly a scale/translate in specified
//! directions.
//!
//! @param [in] xChange 1 if x may change, 0 if it must stay fixed
//! @param [in] yChange 1 if y may change, 0 if it must stay fixed
//! @param [in] zChange 1 if z may change, 0 if it must stay fixed
//! @return true if independence tests are satisfied.
//!
bool IsIndependent
(
int             xChange,
int             yChange,
int             zChange
);

//!
//! Test if a transform is singular
//!
//! @return true if the mapping is singular
//!
bool IsSingular () const;

//!
//! Test if a transform is affiine.
//!
//! @return true if the mapping is affine.
//!
bool IsAffine () const;

//!
//! Test if a transform is perspective.
//!
//! @return true if the mapping contains perspective.
//!
bool IsPerspective () const;

//!
//! Copy various parts of the 4x4 matrix to separate data structures.
//! @param [out] matrix upper 3x3
//! @param [out] translation last column (above diagonal)
//! @param [out] perspective last row (including diagonal)
//! @param [in] inverse false for forward part, true for inverse
//! @remark The separate pieces do not "mean" much if there is perspective.
void Explode
(
RotMatrixR      matrix,
DPoint3dR       translation,
DPoint4dR       perspective,
bool             inverse
) const;


//!
//!
//! @return true if the mapping is an identity
//!
bool IsIdentity () const;


/*__PUBLISH_SECTION_END__*/
//END_REFMETHODS
#ifdef CompileDMap4dPointerMethods
//!
//!
//! Initialize and identity mapping.
//!
//!
void initIdentity ();

//!
//! Initialize a transform which translates and scales along principle
//! axes so box loAP..hiAP maps to box loBP..hiBP
//!
//! @param [in] loAP corner of box A
//! @param [in] hiAP diagonally opposite corner of box A
//! @param [in] loBP corner of box B
//! @param [in] hiBP diagonally opposite corner of box B
//! @return int
//!
bool    initFromRanges
(
DPoint3dCP      loAP,
DPoint3dCP      hiAP,
DPoint3dCP      loBP,
DPoint3dCP      hiBP
);

//!
//! Initialize a rotation about axis vx,vy,vz by angle whose cosine
//! and sine are (proportional to) c and s.
//!
//! @param [in] c cosine of angle
//! @param [in] s sine of angle
//! @param [in] vx x component of rotation axis
//! @param [in] vy y component of rotation axis
//! @param [in] vz z component of rotation axis
//!
void initFromRotation
(
double          c,
double          s,
double          vx,
double          vy,
double          vz
);

//!
//! Rotate about vx,yv,vz by an integer multiple of 90 degrees.
//! Providing the angle as an integer allows exact table lookup without
//! approximation of pi.
//!
//! @param [in] multiple rotation angle is multiple * 90 degrees
//! @param [in] vx x component of rotation axis
//! @param [in] vy y component of rotation axis
//! @param [in] vz z component of rotation axis
//!
void initFromQuadrantRotation
(
int             multiple,
double          vx,
double          vy,
double          vz
);

//!
//! Initialize a rotation by angle in radians about axis vx,vy,vz
//!
//! @param [in] radians angle in degrees
//! @param [in] vx x component of rotation axis
//! @param [in] vy y component of rotation axis
//! @param [in] vz z component of rotation axis
//!
void initFromRotation
(
double          radians,
double          vx,
double          vy,
double          vz
);

//!
//! Initialize a pure scaling transformation.
//! If any scale factor is zero, the corresponding inverse entry is also
//! zero.
//!
//! @param [in] ax x scale factor
//! @param [in] ay y scale factor
//! @param [in] az z scale factor
//!
void initFromScale
(
double          ax,
double          ay,
double          az
);

//!
//! Initialize a transform that is a (noninvertible) projection to a
//! principle plane.
//!
//! @param [in] height distance of plane from origin
//! @param [in] axis 0,1,2 for x,y,z normal
//!
void initFromPrincipleProjection
(
double          height,
int             axis
);

//!
//! Initialize a transform that incorporates a given origin, x direction, and additional
//! local transformation, all relative to a given transformation.
//! (This is the guts of AgentContext::pushTransformTo())
//! @param [in] pOrigin world origin
//! @param [in] pXDir world x direction
//! @param [in] pWorldToTarget map from world to target system
//!
void initFromTransformedOriginAndDirection
(
DPoint3dCP      pOrigin,
DPoint3dCP      pXDir,
DMap4dCP        pTargetToWorld,
DMap4dCP        pLocalFrame
);

//!
//! Initialize a uniform scaling transformation.
//!
//! @param [in] a scale factor
//!
void initFromScale (double a);

//!
//!
//!
//! Initialize a translation transform.
//!
//! @param [in] tx x component of translation
//! @param [in] ty y component of translation
//! @param [in] tz z component of translation
//!
void initFromTranslation
(
double          tx,
double          ty,
double          tz
);

//!
//! Initialize a transform with perspective entries for a (nonzero)
//! taper in the z direction.
//!
//! @param [in] taper taper fraction
//! @return true if an invertible map was constructed.
//!
bool    initFromTaper (double taper);

//!
//! @param [in] pTransform affine transformation 3x4 matrix
//! @param [in] invert true to treat this matrix as the inverse
//!                                        of the mapping, false if forward.
//! @return true if the Transfrom was invertible.
//!
bool    initFromTransform
(
TransformCP     pTransform,
bool            invert
);

//!
//! Extract view structure from an Map.  The values computed here will
//! probably be useful for constructing the quirky inputs of various
//! rendering systems.
//!
//! @param [out] pViewingMatrix Composite view matrix
//! @param pViewPlaneFrame orthogonal coordinate frame with
//! @param  000 = projection of eye point on view plane.
//! @param  x axis in view plane
//! @param  y axis in view plane
//! @param  z axis towards eye
//! @param [out] pW 1.0 / distance to eye.  0 for flat view.
//! @return true if invertible matrix was constructed.
//!
bool    getViewingMatrix
(
DMatrix4dP      pViewingMatrix,
DTransform3dP   pViewPlaneFrame,
double          *pW
) const;

//!
//! Fill a mapping between a unit prism and homogeneous skew space.
//! Example use:
//! We want to map a unit rectangle from font space to screen, under
//! a full perspective mapping.   That is, given font space point (x,y)
//! we want A * (x,y,0,1)^T  = the visible pixel.   Also, given pixel
//! (i,j) we want Ainverse * (i,j,0,1) to map back into font space.
//! A long time ago, we were told points P0,P1,P2 which are the
//! preperspective points that correspond to font space (0,0), (1,0), and
//! (0,1).
//! Since then, P00, P10, P01 have been through a homogeneous transformation.
//! (For instance, there may be 'weight' of other than 1 on each one.)
//! The transformed (homogeneous) points are Q00, Q10, Q01
//! In device space, we do a projection in the z direction.  Hence we
//! need a 4th point Qz=(0,0,1,0).
//! Build this matrix by calling
//! jmdlDMap4d_fillHomogeneousSkewFrame (pHMap, Q00, Q10,Q01,Qz)
//!
//! @param pPoint0001 0001 in the unit space maps here
//! @param pPoint1001 1001 in the unit space maps here
//! @param pPoint0101 0100 in the unit space maps here
//! @param pPoint0010 0010 in the unit space maps here
//! @return true if the 4 points are independent
//!
bool    initFromSkewBox
(
DPoint4dCP      pPoint0001,
DPoint4dCP      pPoint1001,
DPoint4dCP      pPoint0101,
DPoint4dCP      pPoint0010
);

//!
//!
//! @param [in] z0 reference z.  Normalized projective coordinate
//!                                is 0 at this z
//! @param [in] zetaHalf controls rate of growth of normalized
//!                                projective z.   Projective z at z0/k
//!                                is k-1*zetahalf, i.e. is zetahalf at
//!                                z0/2
//!
void zFrustum
(
double          z0,
double          zetaHalf
);

//!
//! Sets pA to the inverse of pB.
//!
//! @param [in] pB original mapping
//!
void inverseOf (DMap4dCP pB);

//!
//! Form the product
//!   C * Binv * A * B * Cinv
//! A NULL for B or C skips that part.
//!
//! @param [in] pA inside term of sandwich
//! @param [in] pB middle term of sandwich
//! @param [in] pC outer term of sandwich
//!
void sandwichOf
(
DMap4dCP        pA,
DMap4dCP        pB,
DMap4dCP        pC
);

//!
//! Multiply transforms
//!
//! @param [in] pA transform A
//! @param [in] pB transform B
//!
void productOf
(
DMap4dCP        pA,
DMap4dCP        pB
);

//!
//! Multiply transforms, selecting optional forward or inverse for each
//!
//! @param [in] pA transform A
//! @param [in] invertA if true, use invese of A
//! @param [in] pB transform B
//! @param [in] invertB if true, use invese of B
//!
void productOf
(
DMap4dCP        pA,
bool            invertA,
DMap4dCP        pB,
bool            invertB
);

//!
//! Checks if the mapping is strictly a scale/translate in specified
//! directions.
//!
//! @param [in] xChange 1 if x may change, 0 if it must stay fixed
//! @param [in] yChange 1 if y may change, 0 if it must stay fixed
//! @param [in] zChange 1 if z may change, 0 if it must stay fixed
//! @return true if independence tests are satisfied.
//!
bool    isIndependent
(
int             xChange,
int             yChange,
int             zChange
);

//!
//! Test if a transform is singular
//!
//! @return true if the mapping is singular
//!
bool    isSingular () const;

//!
//! Test if a transform is affiine.
//!
//! @return true if the mapping is affine.
//!
bool    isAffine () const;

//!
//! Test if a transform is perspective.
//!
//! @return true if the mapping contains perspective.
//!
bool    isPerspective () const;

//!
//!
//! @return true if the mapping is an identity
//!
bool    isIdentity () const;

#endif
/*__PUBLISH_SECTION_START__*/

#endif
};
END_BENTLEY_GEOMETRY_NAMESPACE
