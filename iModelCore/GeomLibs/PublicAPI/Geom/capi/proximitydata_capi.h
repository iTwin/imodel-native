/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Initialize proximity test data for xy distance testing.
//!
Public GEOMDLLIMPEXP void               bsiProximityData_init
(
ProximityDataP pProx,
DPoint3dCP pTestPoint,
int         index,
double      param
);

//!
//! Update proximity test data with a new, possibly closer, test point.
//!
Public GEOMDLLIMPEXP void               bsiProximityData_testXY
(
ProximityDataP pProx,
DPoint4dCP pNewPoint,
double    newParam,
int       newIndex
);

//!
//! Update proximity test data with a new, possibly closer, test point.
//!
Public GEOMDLLIMPEXP void               bsiProximityData_test
(
ProximityDataP pProx,
DPoint4dCP pNewPoint,
double    newParam,
int       newIndex
);

//!
//! Update proximity test data with a new, possibly closer, test point.  Do the
//! distance test on a different point from the one stored.
//!
Public GEOMDLLIMPEXP void               bsiProximityData_testProxy
(
ProximityDataP pProx,
DPoint4dCP pProxyPoint,
DPoint4dCP pNewPoint,
double    newParam,
int       newIndex
);

//!
//! Copy from proximity data structure to output args.
//! (Assumes proximity data is initialized to copyable values even if dataValid
//!   flag is false)
//! @return dataValid flag from proximtity data.
//!
Public GEOMDLLIMPEXP bool               bsiProximityData_unload
(
ProximityDataP pProx,
int       *pCloseIndex,
double    *pCloseParam,
DPoint4dP pClosePoint,
DPoint3dP pClosePoint3d,
double    *pMinDistSquared
);

END_BENTLEY_GEOMETRY_NAMESPACE

