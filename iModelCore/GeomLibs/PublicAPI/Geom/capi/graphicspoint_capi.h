/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#ifdef __cplusplus

//!
//! Initialize a graphics point from complete data
//!
//! @param pInstance OUT     graphics point to initialize
//! @param pPoint IN      point to add
//! @param a      IN      floating point label value
//! @param mask   IN      mask value
//! @param userData IN      user data labal
//!
//!
Public GEOMDLLIMPEXP void    bsiGraphicsPoint_initFromDPoint4d
(
GraphicsPointP pInstance,
DPoint4dCP pPoint,
double          a,
int             mask,
int             userData
);

//!
//! Initialize a graphics point from complete data
//!
//! @param pInstance OUT     graphics point to initialize
//! @param pPoint IN      point to add
//! @param a      IN      floating point label value
//! @param mask   IN      mask value
//! @param userData IN      user data labal
//!
//!
Public GEOMDLLIMPEXP void    bsiGraphicsPoint_initFromDPoint3d
(
GraphicsPointP pInstance,
DPoint3dCP pPoint,
double          weight,
double          a,
int             mask,
int             userData
);

//!
//! Initialize a graphics point from complete data
//!
//! @param pInstance OUT     graphics point to initialize
//! @param pPoint IN      point to add
//! @param a      IN      floating point label value
//! @param mask   IN      mask value
//! @param userData IN      user data labal
//!
//!
Public GEOMDLLIMPEXP void    bsiGraphicsPoint_init
(
GraphicsPointP pInstance,
double          x,
double          y,
double          z,
double          w,
double          a,
int             mask,
int             userData
);

//!
//! Normalize the point weight in place.
//! @param pInstance OUT     normalized graphics point
//! @param pPoint IN      source point
//! @return true if the weight is nonzero
//!
//!
Public GEOMDLLIMPEXP bool        bsiGraphicsPoint_normalizeWeight (GraphicsPointP pInstance);

//!
//! Copies an array of graphics point
//!
//! @param pOutPoint OUT     destination array
//! @param m IN      maximum number of points to copy
//! @param pInPoint IN      source array
//! @param n IN      number of points
//! @return number of copied
//!
//!
Public GEOMDLLIMPEXP int bsiGraphicsPoint_copyArray
(
GraphicsPointP pOutPoint,
int           m,
GraphicsPointCP pInPoint,
int           n
);

//!
//! copies n DPoint4d structures from the pSource array to the pDest
//! using an index array to rearrange (not necessarily 1to1) the order.
//! The indexing assigns pDest[i] = pSource[indexP[i]].
//!
//!
//! @param pDest OUT     destination array
//! @param pSource IN      source array
//! @param pIndex IN      array of indices into source array
//! @param nIndex IN      number of points
//!
//!
Public GEOMDLLIMPEXP void bsiGraphicsPoint_copyIndexedArray
(
GraphicsPointP pDest,
GraphicsPointCP pSource,
int             *pIndex,
int             nIndex
);

//!
//! Sets the in and out bits of the mask.
//! @param bIn  IN      in bit setting.  Any nonzero sets it.
//! @param bOut IN      out bit setting. Any nonzero sets it.
//! @param returns the exact bits set in the point.  Calling with NULL point
//!               is a way to convert generic bool    classifitions to
//!               exact bits for the mask.
//!
Public GEOMDLLIMPEXP int bsiGraphicsPoint_setInOut
(
GraphicsPointP pGP,
bool            bIn,
bool            bOut
);

//!
//! Sets the in and out bits of the mask.
//! @param bIn  IN      in bit setting.  Any nonzero sets it.
//! @param bOut IN      out bit setting. Any nonzero sets it.
//! @param returns the exact bits set in the point.  Calling with NULL point
//!               is a way to convert generic bool    classifitions to
//!               exact bits for the mask.
//!
Public GEOMDLLIMPEXP int bsiGraphicsPoint_setInOutArray
(
GraphicsPointP pGP,
int             numGP,
bool            bIn,
bool            bOut
);

//!
//! Gets the inout bits.
//! @param bbIn  OUT     in part of bit setting.
//! @param bbOut OUT     out part of bit setting.
//! @param returns the bits from the the point.
//!
Public GEOMDLLIMPEXP int bsiGraphicsPoint_getInOut
(
GraphicsPointP  pGP,
int             *pbIn,
int             *pbOut
);

#endif
END_BENTLEY_GEOMETRY_NAMESPACE

