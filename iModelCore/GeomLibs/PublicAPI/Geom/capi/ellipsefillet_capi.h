/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Compute intersections of the (signed, possibly different) offsets of an ellipse and a plane.
//! @param pXYZ OUT array (ALLOCATED BY CALLER, OPTIONAL) of intersection points.
//! @param pTrigPoint OUT array (ALLOCATED BY CALLER, OPTIONAL) with x,y,z carrying
//!            cosine, sine, and angle of the point on the ellipse.
//! @param pNumXYZ OUT number of intersection points.
//! @param maxXYZ IN max number of points to place in pXYZ.  Note that returned count may exceed this number --
//!    all intersections are computed and counted, but extras are not returned.
//! @param pEllipse IN ellipse to offset.
//! @param ellipseOffset IN signed offset of the ellipse.  The positive offset direction is defined in the
//!       in the right handed system of the ellipse axes.
//! @param pPlane IN base plane
//! @param planeOffset IN signed offset from plane.
//!
Public GEOMDLLIMPEXP void bsiGeom_intersectOffsetEllipseOffsetPlane
(
DPoint3dP pXYZ,
DPoint3dP pTrigPoint,
int      *pNumXYZ,
int      maxXYZ,
DEllipse3dCP pEllipse,
double     ellipseOffset,
DPlane3dCP  pPlane,
double     planeOffset
);

END_BENTLEY_GEOMETRY_NAMESPACE

