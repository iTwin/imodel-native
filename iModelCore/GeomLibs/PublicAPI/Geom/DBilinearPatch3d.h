/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/DBilinearPatch3d.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/**
4-sided patch defined by its 4 vertices.
The patch is parameterized by u,v as
   {xyz = (1-u)*(1-v)*xyz00 + u*(1-v)*xyz10 + (1-u)*v*xyz10 + u*v*xyz11}
*/
struct GEOMDLLIMPEXP DBilinearPatch3d
{
//! corner points addressable as point[i][j]
DPoint3d point[2][2];
//! Initialize as zeros.
DBilinearPatch3d ();

//! Constructor from points in 00,10,01,11 index order.
DBilinearPatch3d (DPoint3dCR xyz0, DPoint3dCR xyz10, DPoint3dCR xyz01, DPoint3dCR xyz11);
//! Constructor from points in 00,10,01,11 index order.
DBilinearPatch3d (DPoint2dCR xyz0, DPoint2dCR xyz10, DPoint2dCR xyz01, DPoint2dCR xyz11);

//! Constructor from points on lower, upper edges
DBilinearPatch3d (DSegment3dCR lowerEdge, DSegment3dCR upperEdge);

//! Return surface coordinates at u,v parameters
DPoint3d Evaluate (double u, double v) const;
//! Return surface coordinates and first derivatives at u,v parameters
void Evaluate (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) const;
//! Return surface coordinates and unit normal at u,v parameters
void EvaluateNormal (double u, double v, DPoint3dR xyz, DVec3dR unitNormal) const;

// fill a grid with (at least 2 points) on each edge.
void EvaluateGrid (int numUPoint, int numVPoint, bvector<DPoint3d> &gridPoints) const;

//! return uv coordinates of projections of xyz onto the (bounded) patch.
//! This returns only true perpendicular projections (i.e. does not give "close but nonperpendicular point on edge")
bool PerpendicularsOnBoundedPatch (DPoint3dCR spacePoint, bvector<DPoint2d> &uv) const;
//! Return specified edge as a line segment.
//! Edge order is: bottom, right, top, left with CCW loop direction.
DSegment3d GetCCWEdge (int i) const;

//! return vector along u edge (lower or upper)
//! @param [in] i 0,1 for lower, upper edge.
DVec3d GetUEdgeVector (int i) const;

//! return diagonal vector starting at 00
DVec3d GetDiagonalFrom00 () const;
//! return diagonal vector starting at 01
DVec3d GetDiagonalFrom01 () const;


//! return vector along v edge (left, right)
//! @param [in] i 0,1 for left, right edge.
DVec3d GetVEdgeVector (int i) const;

//! Test if the patch is just a parallelogram
bool IsParallelogram () const;

//! Test if the patch is planar, with usual system angle tolerances (tight)
bool IsPlanar () const;

//! Test if the patch is planar, with specified angle tolerance.
//! Typically used with angleTol of 1e-5 or smaller -- implementation assumes sin(tol) almost equal to tol.
bool IsPlanar (double angleTol) const;
};

END_BENTLEY_GEOMETRY_NAMESPACE
