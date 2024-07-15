/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//!
//! Initialize and identity mapping.
//!
//! @param pHMap OUT     identity transform
//!
Public GEOMDLLIMPEXP void bsiDMap4d_initIdentity (DMap4dP pHMap);

//!
//! Initialize a transform which translates and scales along principle
//! axes so box loAP..hiAP maps to box loBP..hiBP
//!
//! @param pHMap OUT     transform
//! @param loAP IN      corner of box A
//! @param hiAP IN      diagonally opposite corner of box A
//! @param loBP IN      corner of box B
//! @param hiBP IN      diagonally opposite corner of box B
//! @return int
//!
Public GEOMDLLIMPEXP bool    bsiDMap4d_initBoxMap
(
DMap4dP  pHMap,
DPoint3dCP loAP,
DPoint3dCP hiAP,
DPoint3dCP loBP,
DPoint3dCP hiBP
);

//!
//! Initialize a rotation about axis vx,vy,vz by angle whose cosine
//! and sine are (proportional to) c and s.
//!
//! @param pHMap OUT     returned DMap4d
//! @param c IN      cosine of angle
//! @param s IN      sine of angle
//! @param vx IN      x component of rotation axis
//! @param vy IN      y component of rotation axis
//! @param vz IN      z component of rotation axis
//!
Public GEOMDLLIMPEXP void bsiDMap4d_initRotateTrig
(
DMap4dP  pHMap,
double c,
double s,
double vx,
double vy,
double vz
);

//!
//! Rotate about vx,yv,vz by an integer multiple of 90 degrees.
//! Providing the angle as an integer allows exact table lookup without
//! approximation of pi.
//!
//! @param pHMap OUT     returned DMap4d
//! @param multiple IN      rotation angle is multiple * 90 degrees
//! @param vx IN      x component of rotation axis
//! @param vy IN      y component of rotation axis
//! @param vz IN      z component of rotation axis
//!
Public GEOMDLLIMPEXP void bsiDMap4d_initRotateQuadrant
(
DMap4dP  pHMap,
int     multiple,
double vx,
double vy,
double vz
);

//!
//! Initialize a rotation by angle in radians about axis vx,vy,vz
//!
//! @param pHMap OUT     rotational transform
//! @param radians IN      angle in degrees
//! @param vx IN      x component of rotation axis
//! @param vy IN      y component of rotation axis
//! @param vz IN      z component of rotation axis
//!
Public GEOMDLLIMPEXP void bsiDMap4d_initRotate
(
DMap4dP  pHMap,
double radians,
double vx,
double vy,
double vz
);

//!
//! Initialize a pure scaling transformation.
//! If any scale factor is zero, the corresponding inverse entry is also
//! zero.
//!
//! @param pHMap OUT     scaling transformation
//! @param ax IN      x scale factor
//! @param ay IN      y scale factor
//! @param az IN      z scale factor
//!
Public GEOMDLLIMPEXP void bsiDMap4d_initScale
(
DMap4dP  pHMap,
double ax,
double ay,
double az
);

//!
//! Initialize a transform that is a (noninvertible) projection to a
//! principle plane.
//!
//! @param pHMap OUT     projection map
//! @param height IN      distance of plane from origin
//! @param axis IN      0,1,2 for x,y,z normal
//!
Public GEOMDLLIMPEXP void bsiDMap4d_initPrincipleProjection
(
DMap4dP  pHMap,
double height,
int    axis
);

//!
//! Initialize a transform that incorporates a given origin, x direction, and additional
//! local transformation, all relative to a given transformation.
//! (This is the guts of AgentContext::pushTransformTo())
//! @param pHMap OUT     projection map
//! @param pOrigin IN      world origin
//! @param pXDir   IN      world x direction
//! @param pWorldToTarget IN      map from world to target system
//!
Public GEOMDLLIMPEXP void bsiDMap4d_initFromTransformedOriginAndDirection
(
DMap4dP pCompositeMap,
DPoint3dCP pOrigin,
DPoint3dCP pXDir,
DMap4dCP pTargetToWorld,
DMap4dCP pLocalFrame
);

//!
//! Initialize a uniform scaling transformation.
//!
//! @param pHMap OUT     scaling transformation
//! @param a IN      scale factor
//!
Public GEOMDLLIMPEXP void bsiDMap4d_initUniformScale
(
DMap4dP  pHMap,
double a
);

//!
//!
//!
//! Initialize a translation transform.
//!
//! @param pHMap OUT     translation transform
//! @param tx IN      x component of translation
//! @param ty IN      y component of translation
//! @param tz IN      z component of translation
//!
Public GEOMDLLIMPEXP void bsiDMap4d_initTranslate
(
DMap4dP  pHMap,
double tx,
double ty,
double tz
);

//!
//! Initialize a transform with perspective entries for a (nonzero)
//! taper in the z direction.
//!
//! @param pHMap OUT     transform
//! @param taper IN      taper fraction
//! @return true if an invertible map was constructed.
//!
Public GEOMDLLIMPEXP bool    bsiDMap4d_initTaper
(
DMap4dP     pHMap,
double      taper
);

//!
//! @param pHMap OUT     mapping constructed from the matrix
//! @param pMatrix IN      3x3 matrix
//! @param invert IN      true to treat this matrix as the inverse
//!                                        of the mapping, false if forward.
//! @return true if the DMatrix3d was invertible.
//!
Public GEOMDLLIMPEXP bool    bsiDMap4d_initFromDMatrix3d
(
DMap4dP pHMap,
DMatrix3dCP pMatrix,
bool          invert
);

//!
//! @param pHMap OUT     mapping constructed from the matrix
//! @param pTransform IN      affine transformation 3x3 matrix
//! @param invert IN      true to treat this matrix as the inverse
//!                                        of the mapping, false if forward.
//! @return true if the DTransfrom3d was invertible.
//!
Public GEOMDLLIMPEXP bool    bsiDMap4d_initFromDTransform3d
(
DMap4dP pHMap,
DTransform3dCP pTransform,
bool            invert
);

//!
//! @param pHMap OUT     mapping constructed from the matrix
//! @param pTransform IN      affine transformation 3x4 matrix
//! @param invert IN      true to treat this matrix as the inverse
//!                                        of the mapping, false if forward.
//! @return true if the Transfrom was invertible.
//!
Public GEOMDLLIMPEXP bool    bsiDMap4d_initFromTransform
(
DMap4dP pHMap,
TransformCP pTransform,
bool        invert
);

//!
//! Construct a projective transformations corresponding to
//! the opengl functions glOrtho and glFrustum.
//! The usual description is:
//! <UL>
//! <LI>Eyepoint at origin, looking down the negative z axis.
//! <LI> far and near clipping planes at
//!   z=farCoord and z=nearCoord, respectively.
//! <LI>Side clipping planes pass through (leftCoord,backCoord,nearCoord) and (rightCoord,topCoord,nearCoord)
//! i.e. leftCoord,rightCoord,backCoord,topCoord give window coordinates on a plane at distance nearCoord.
//! The view frustum is mapped to a unit cube with +1 values
//! at the corner points.
//! <LI>In x: (leftCoord,rightCoord) maps to (1,+1)
//! <LI>In y: (backCoord,topCoord) maps to (1,+1)
//! <LI>In z: (farCoord,nearCoord) maps to (1,+1)
//! <UL>
//! In strict use of the opengl documentation, the
//! quantities should satisfy leftCoord<rightCoord, backCoord<topCoord, and nearCoord<farCoord,
//! i.e. correspond to the names left, right, bottom, top, near,
//!  far.
//! However, it is perfectly fine to reverse any or all in order
//! to control the orientation and scaling.  In particular,
//! you can get a right handed coordinate system in the projected
//! box by using farCoord<nearCoord. The interpretation that leftCoord,rightCoord,backCoord,topCoord are a window
//! at depth nearCoord still holds, but it is just at the back of the
//! frustum instead of the front.
//!
//! @param pHMap OUT     projection transform
//! @param leftCoord IN      left x coordinate at depth z=-nearCoord near plane
//! @param rightCoord IN      right x coordiante at depth z=-nearCoord near plane
//! @param backCoord IN      bottom y coordiante at depth z=-nearCoord near plane
//! @param topCoord IN      top y coordiante at depth z=-nearCoord near plane
//! @param nearCoord IN      distance to near plane
//! @param farCoord IN      distance to far plane
//!
Public GEOMDLLIMPEXP void bsiDMap4d_glOrtho
(
DMap4dP  pHMap,
double leftCoord,
double rightCoord,
double backCoord,
double topCoord,
double nearCoord,
double farCoord
);

//!
//! Initialize a flat projection frustum.
//!
//! @param pHMap OUT     projection transform
//! @param leftCoord IN      left x coordinate at depth z=-nearCoord near plane
//! @param rightCoord IN      right x coordiante at depth z=-nearCoord near plane
//! @param backCoord IN      bottom y coordiante at depth z=-nearCoord near plane
//! @param topCoord IN      top y coordiante at depth z=-nearCoord near plane
//! @param nearCoord IN      distance to near plane
//! @param farCoord IN      distance to far plane
//!
Public GEOMDLLIMPEXP void bsiDMap4d_glFrustum
(
DMap4dP  pHMap,
double leftCoord,
double rightCoord,
double backCoord,
double topCoord,
double nearCoord,
double farCoord
);

//!
//! There is a simple, direct transformation from world to a perspective
//! in a slab.   Unfortunately, camerabased abstraction presented to
//! the outside world wants to go there by way of a (rigid) camera.
//! So we have to take the long way around the barn to achieve
//! <UL>
//! <LI> world > camera > slab > perspective slab
//! </UL>
//! by combining the two easily computed chains:
//! <UL>
//! <LI> world > camera
//! <LI> world > slab > perspective slab
//! </UL>
//!
//! @param pWorldToCameraMap OUT     world to camera part of transform. May be NULL pointer.
//! @param pCameraToNpcMap OUT     camera to npc part of transform. May be NULL pointer.
//! @param pWorldToNpcMap OUT     world to npc.  May be NULL pointer
//! @param pOrigin IN      lower left rear of frustum
//! @param pUPoint IN      lower right rear of frustum
//! @param pVPoint IN      upper right rear of frustum
//! @param pWPoint IN      lower left front of frustum
//! @param fraction IN      front size divided by back size
//! @return true if frustum is well defined
//!
Public GEOMDLLIMPEXP bool    bsiDMap4d_init3PointCameraAndFrustum
(
DMap4dP        pWorldToCameraMap,
DMap4dP        pCameraToNpcMap,
DMap4dP        pWorldToNpcMap,
DPoint3dCP pOrigin,
DPoint3dCP pUPoint,
DPoint3dCP pVPoint,
DPoint3dCP pWPoint,
double      fraction
);

//!
//! There is a simple, direct transformation from world to a perspective
//! in a slab.   Unfortunately, camerabased abstraction presented to
//! the outside world wants to go there by way of a (rigid) camera.
//! So we have to take the long way around the barn to achieve
//! <UL>
//! <LI> world > camera > slab > perspective slab
//! </UL>
//! by combining the two easily computed chains:
//! <UL>
//! <LI> world > camera
//! <LI> world > slab > perspective slab
//! </UL>
//!
//! @param pWorldToCameraMap OUT     world to camera part of transform. May be NULL pointer.
//! @param pCameraToNpcMap OUT     camera to npc part of transform. May be NULL pointer.
//! @param pWorldToNpcMap OUT     world to npc.  May be NULL pointer
//! @param pOrigin IN      lower left rear of frustum
//! @param pUVector IN      lower left to lower right backface extent
//! @param pVVector IN      lower left to upper left backface extent vector
//! @param pWPoint IN      lower left rear to lower left front extent vector
//! @param fraction IN      front size divided by back size
//! @return true if frustum is well defined
//!
Public GEOMDLLIMPEXP bool    bsiDMap4d_initFromVectorFrustum
(
DMap4dP        pWorldToNpcMap,
DPoint3dCP pOrigin,
DPoint3dCP pUVector,
DPoint3dCP pVVector,
DPoint3dCP pWVector,
double      fraction
);

//!
//! Extract view structure from an Map.  The values computed here will
//! probably be useful for constructing the quirky inputs of various
//! rendering systems.
//!
//! @param pWorldToNpcMap IN      exisiting world-to-npc transformation.
//! @param pViewingMatrix OUT     Composite view matrix
//! @param pViewPlaneFrame orthogonal coordinate frame with
//! @param  000 = projection of eye point on view plane.
//! @param  x axis in view plane
//! @param  y axis in view plane
//! @param  z axis towards eye
//! @param pW OUT     1.0 / distance to eye.  0 for flat view.
//! @return true if invertible matrix was constructed.
//!
Public GEOMDLLIMPEXP bool    bsiDMap4d_getViewingMatrix
(
DMap4dCP pWorldToNpcMap,
DMatrix4dP pViewingMatrix,
DTransform3dP pViewPlaneFrame,
double          *pW
);

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
//! @param pHMap OUT     projective transform
//! @param pPoint0001 0001 in the unit space maps here
//! @param pPoint1001 1001 in the unit space maps here
//! @param pPoint0101 0100 in the unit space maps here
//! @param pPoint0010 0010 in the unit space maps here
//! @return true if the 4 points are independent
//!
Public GEOMDLLIMPEXP bool    bsiDMap4d_initHomgeneousSkewFrame
(
DMap4dP pHMap,
DPoint4dCP pPoint0001,
DPoint4dCP pPoint1001,
DPoint4dCP pPoint0101,
DPoint4dCP pPoint0010
);

//!
//!
//! @param pHMap OUT     projective transform
//! @param z0 IN      reference z.  Normalized projective coordinate
//!                                is 0 at this z
//! @param zetaHalf IN      controls rate of growth of normalized
//!                                projective z.   Projective z at z0/k
//!                                is k-1*zetahalf, i.e. is zetahalf at
//!                                z0/2
//!
Public GEOMDLLIMPEXP void bsiDMap4d_zFrustum
(
DMap4dP  pHMap,
double z0,
double zetaHalf
);

//!
//! Build a transform that maps a perspective frustum to a rectilinear
//! box.  If either point of the box is NULL, the box defaults to
//! 1..+1 in all dimensions.
//!
//! @param pHMap OUT     DMap4d structure to be filled
//! @param pCamera IN      camera point origin of viewing system
//! @param pTarget IN      target point
//!                                where viewplane intersects -z axis
//! @param pUpVector IN      camera up vector
//! @param pViewplaneWindow IN      viewplane clip window.  Only xy parts are used.
//! @param dFront IN      unsigned distance to front clip plane
//! @param dBack IN      unsigned distance to back clip plane
//! @param pLoPoint IN      the xyz coordinates that are to be the
//!                                lower left in transformed space
//! @param pHiPoint IN      the xyz coordinates that are to be the
//!                                upper right in transformed space
//! @return int
//!
Public GEOMDLLIMPEXP int bsiDMap4d_initFromCameraParameters
(
DMap4dP  pHMap,
DPoint3dCP pCamera,
DPoint3dCP pTarget,
DPoint3dCP pUpVector,
DRange3dCP pViewplaneWindow,
double dFront,
double dBack,
DPoint3dCP pLoPoint,
DPoint3dCP pHiPoint
);

//!
//! Sets pA to the inverse of pB.
//!
//! @param pA OUT     inverted mapping
//! @param pB IN      original mapping
//!
Public GEOMDLLIMPEXP void bsiDMap4d_invert
(
DMap4dP pA,
DMap4dCP pB
);

//!
//! Form the product
//!   C * Binv * A * B * Cinv
//! A NULL for B or C skips that part.
//!
//! @param pResult OUT     product DMap4d Binv*A*B
//! @param pA IN      inside term of sandwich
//! @param pB IN      middle term of sandwich
//! @param pC IN      outer term of sandwich
//!
Public GEOMDLLIMPEXP void bsiDMap4d_sandwich
(
DMap4dP  pResult,
DMap4dCP  pA,
DMap4dCP  pB,
DMap4dCP  pC
);

//!
//! Multiply transforms
//!
//! @param pC OUT     product A * B
//! @param pA IN      transform A
//! @param pB IN      transform B
//!
Public GEOMDLLIMPEXP void bsiDMap4d_multiply
(
DMap4dP pC,
DMap4dCP pA,
DMap4dCP pB
);

//!
//! Multiply transforms, selecting optional forward or inverse for each
//!
//! @param pC OUT     product
//! @param pA IN      transform A
//! @param invertA IN      if true, use invese of A
//! @param pB IN      transform B
//! @param invertB IN      if true, use invese of B
//!
Public GEOMDLLIMPEXP void bsiDMap4d_multiplyInverted
(
DMap4dP pC,
DMap4dCP pA,
bool        invertA,
DMap4dCP pB,
bool        invertB
);

//!
//! Checks if the mapping is strictly a scale/translate in specified
//! directions.
//!
//! @param pA IN      Transform to test
//! @param xChange IN      1 if x may change, 0 if it must stay fixed
//! @param yChange IN      1 if y may change, 0 if it must stay fixed
//! @param zChange IN      1 if z may change, 0 if it must stay fixed
//! @return true if independence tests are satisfied.
//!
Public GEOMDLLIMPEXP bool     bsiDMap4d_isIndependent
(
DMap4dP    pA,
int     xChange,
int     yChange,
int     zChange
);

//!
//! Test if a transform is singular
//!
//! @param pHMap IN      transform to test
//! @return true if the mapping is singular
//!
Public GEOMDLLIMPEXP bool    bsiDMap4d_isSingular (DMap4dCP pHMap);

//!
//! Test if a transform is affiine.
//!
//! @param pHMap IN      transform to test
//! @return true if the mapping is affine.
//!
Public GEOMDLLIMPEXP bool    bsiDMap4d_isAffine (DMap4dCP pHMap);

//!
//! Test if a transform is perspective.
//!
//! @param pHMap IN      transform to test
//! @return true if the mapping contains perspective.
//!
Public GEOMDLLIMPEXP bool    bsiDMap4d_isPerspective (DMap4dCP pHMap);

//!
//!
//! @return size required to allocate a DMap4d
//!
Public GEOMDLLIMPEXP int bsiDMap4d_size ();

//!
//! Copy various parts of the 4x4 matrix to separate data structures.
//! @param hMapP IN      map to query
//! @param rotP OUT     rotation part
//! @param orgP OUT     origin part
//! @param perspetiveP OUT     perspective part
//! @param inverse IN      false for forward part, true for inverse
//!
Public GEOMDLLIMPEXP void       bsiDMap4d_explode
(
DMap4dP hMapP,
DMatrix3dP rotP,
DPoint3dP orgP,
DPoint4dP perspectiveP,
int             inverse
);

//!
//!
//! @param IN      mapping to test
//! @return true if the mapping is an identity
//!
Public GEOMDLLIMPEXP bool    bsiDMap4d_isIdentity (DMap4dCP pInstance);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlHMap_fillFromMatrices                               |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_initFromMatrices
(
DMap4dP pHMap,     /* OUT     transform */
DMatrix4dCP pM0,       /* IN      forward matrix */
DMatrix4dCP pM1     /* IN      inverse matrix.  Caller is responsible for accuracy!!! */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlHMap_fillFromIndexedMatrices                        |
|                                                                       |
|                                                                       |
| Initialize a transform with perspective entries for a (nonzero)       |
| taper in the z direction.                                             |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsiDMap4d_initFromIndexedMatrices
(
DMap4dP pHMap,     /* OUT     transform */
DMatrix4dCP pM0,       /* IN      forward matrix */
DMatrix4dCP pM1,            /* IN      inverse matrix.  Caller is responsible for accuracy!!! */
const int       *pColIndex  /* IN      indices of columns in M0, rows in M1. */
);

END_BENTLEY_GEOMETRY_NAMESPACE

