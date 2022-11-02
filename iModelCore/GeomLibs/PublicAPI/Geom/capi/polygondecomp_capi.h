/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct ClipPlaneTree;
typedef struct ClipPlaneTree & ClipPlaneTreeR;
struct ClipPlaneTree
{
enum NodeType
    {
    Difference,
    Union
    };
NodeType m_nodeType;
bvector<ClipPlane>     m_planes;
bvector<struct ClipPlaneTree> m_children;
//! Create a PlaneTree where
//!<ul>
//!<li>The top node has the convex hull of the polygon.
//!<li>Each successive level is a convex hull of a "notch" into the polygon.
//!<li>The polygon at each level is the convex hull at that level minus the notches below.
//!</ul>
//!      
static void CreateNotchDifferenceTree (bvector<DPoint3d> &points, struct ClipPlaneTree &tree);

ClipPlaneTree (NodeType nodeType) : m_nodeType (nodeType) {}
ClipPlaneTree () : m_nodeType (NodeType::Difference) {}

GEOMDLLIMPEXP void Clear ()
    {
    m_planes.clear ();
    m_children.clear ();
    }

GEOMDLLIMPEXP bool AddXYPlane (DPoint3dCR point0, DPoint3dCR point1, double sign);
GEOMDLLIMPEXP bool IsPointInOrOn (DPoint3dCR point) const;
};



//!
//! Decompose an (arbitrary) polygon into an XOR of simpler polygons.
//! Each output polygon will have a consistent turning direction, either CW or CCW as
//! indicated by the flag value.
//! If you have any reason to feel that the input polygon does not criss-cross itself,
//! "consistent turning direction" means it is convex.  If the input is not trusted, the
//! output may need to be inspected to check for cases where the polygon spirals "inward" then
//! outward so that it manages to cross over itself without having any changes in concavity.
//!
//! The output is an array of indices into the original points.
//! Each output polygon is described by a sequence of point indices, followed by
//! a negative index.   The negative index is  -1 for a CCW polygon, -2 for CW.
//!
//! @param pIndexArray OUT     indices and flags
//! @param pNumIndex   OUT     number of ints in the index array.
//! @param maxIndex    IN      max number of indices (dimension of pIndexArray).  The recommended
//!                           value for this is twice the number of points.
//! @param pPointArray IN      point coordinates.  Only the xy coordiantes are used -- transform
//!                           to planar condition as needed.
//! @param numPoint   IN      number of points in the array.  The initial/final point should NOT
//!                       be duplicated.
//!
Public GEOMDLLIMPEXP size_t bsiPolygon_decomposeXYToXOR
(
bvector<ptrdiff_t> &indexArray,
bvector<DPoint3d> const &points,
ClipPlaneTreeR tree
);



END_BENTLEY_GEOMETRY_NAMESPACE

