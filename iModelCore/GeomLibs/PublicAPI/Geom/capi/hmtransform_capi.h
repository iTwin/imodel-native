/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Copy various parts of the 4x4 matrix to separate data structures.
//! @instance hMapP => map to query
//! @param rotP <= rotation part
//! @param orgP <= origin part
//! @param perspetiveP <= perspective part
//! @param inverse => false for forward part, true for inverse
//!
Public GEOMDLLIMPEXP void    bsiDMap4d_explode
(
DMap4dP hMapP,
DMatrix3dP rotP,
DPoint3dP orgP,
DPoint4dP perspectiveP,
int             inverse
);

END_BENTLEY_GEOMETRY_NAMESPACE

