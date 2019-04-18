/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//! An AlternatingConvexClipTreeNode is a node in a tree structure in which
//! <ul>
//! <li>Each node contains a ConvexClipPlaneSet
//! <li>Each node contains an array of children which are also AlternativingConvexClipTreeNode.
//! <li>The rule for an in/out decision is that a point is IN the subtree under a node if
//! <ul>
//! <li>It is IN the node's ConvexClipPlaneSet.
//! <li>It is NOT IN any of the children.
//! </ul>
//! <li>Applying "NOT IN any of the children" locally to children at each level means that the ConvexClipPlaneSet
//!     at adjacent levels flip between being positive areas and holes.
//! <li>Use an AlternatingConvexClipTreeNodeBuilder to construct the tree from a polygon.
//! <li>It is possible for the root clip plane set to be empty.  An empty clip plane set returns "true"
//!       for all point tests, so the meaning is just that holes are to be subtracted from the rest 
//!       of space.
//! <li>Althogh the interpretation of in/out alternates with tree levels, the ConvexClipPlaneSets
//      at each level are all "enclosing" planes in the usual way.
//! </ul>
struct AlternatingConvexClipTreeNode
{
bvector<DPoint3d> m_points;
ConvexClipPlaneSet m_planes;
bvector<AlternatingConvexClipTreeNode> m_children;
size_t m_startIndex;  // first index in master array (not the local m_points)
size_t m_numPoints;   // number of points used from master array, possibly wrapping)

//! Initialize this node with index data referencing the parent polygon.
GEOMDLLIMPEXP void InitialzeWithIndices (size_t index0, size_t numPoint);

//! Add a new child that has an empty plane set and given indices.
GEOMDLLIMPEXP void AddEmptyChild (size_t index0, size_t numPoint);

//! Add a plane to the ConvexClipPlaneSet
GEOMDLLIMPEXP void AddPlane (ClipPlaneCR plane);
//! Add an AlternatingConvexClipTreeNode as a child of this one -- i.e. a hole.
//! <ul>
//! <li>The contents is bit-swapped into place, so it is captured without copy.
//! <li>The positive flag is altered as needed to be the opposite of its new parent.
//! </ul>
GEOMDLLIMPEXP void CaptureConvexClipPlaneSetAsVoid (AlternatingConvexClipTreeNode &child);
//! This this node as a "positive area" and recurse to children as the opposite.
GEOMDLLIMPEXP void SetAlternatingAreaFlag (bool value);

//! Recursive search with alternating in and out semantics.
GEOMDLLIMPEXP bool IsPointOnOrInside (DPoint3d xyz) const;
//! <ul>
//! <li>Build the tree for a polygon.
//! <li>Caller creates the root node with empty constructor AlternatingConvexClipTreeNode.
//! </ul>
GEOMDLLIMPEXP static bool CreateTreeForPolygon (bvector<DPoint3d> const &points, AlternatingConvexClipTreeNode &rootNode);

//! Append start-end positions for curve intervals classified as inside or outside.
GEOMDLLIMPEXP void AppendCurvePrimitiveClipIntervals
(
ICurvePrimitiveCR curve,
bvector<CurveLocationDetailPair> *insideIntervals,
bvector<CurveLocationDetailPair> *outsideIntervals
);

//! Append start-end positions for curve intervals classified as inside or outside.
GEOMDLLIMPEXP void AppendCurveVectorClipIntervals
(
CurveVectorCR curves,
bvector<CurveLocationDetailPair> *insideIntervals,
bvector<CurveLocationDetailPair> *outsideIntervals
);


};


END_BENTLEY_GEOMETRY_NAMESPACE
