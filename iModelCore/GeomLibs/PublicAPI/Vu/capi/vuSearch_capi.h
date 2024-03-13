/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//! LINEAR SEARCH for any face that contains xyz by parity rules.
//! @param [in] pGraph graph to search
//! @param [in] xyz test point.
//! @param [in] skipMask mask for faces to skip.
GEOMDLLIMPEXP VuP vu_findContainingFace_linearSearch (VuSetP pGraph, DPoint3dCR xyz, VuMask skipMask);
//! Test if face contains point.
//! @param [in] graph containing graph
//! @param [in] nodeA any node on face
//! @param [in] xyz space point
//! @param [in] edgeHit base if edge if exactly on.
GEOMDLLIMPEXP bool vu_faceContainsPoint (VuSetP pGraph, VuP nodeA, DPoint3dCR xyz, VuP &edgeHitNode, double &edgeFraction);

//! Return a face, edge, or vertex position detail that contains xyz.
//! A successful search is indicated by {GetITag()==1} in the returned detail.
GEOMDLLIMPEXP VuPositionDetail vu_moveTo
(
VuSetP pGraph, VuPositionDetailR startPosition,
DPoint3dCR xyz,
VuPositonDetailAnnouncer * announcer = nullptr  //!< [in] optional test object.  If this returns false the search is terminated.
);

//! Insert points into interior faces and update triangulations.
//! @param [in] pGraph graph to update
//! @param [in] points array of points.
//! @param [in] n number of points
//! @param [in] newZWins Controls selection of z values if a new point is an exact dupliate (in x,y) of
//!     an existing vertex.   If newZWins is true, the newer z is used; else the old.
GEOMDLLIMPEXP Public void vu_insertAndRetriangulate (VuSetP pGraph, DPoint3dCP points, size_t n, bool newZWins);


//! Insert points into interior faces and update triangulations.
//! @param [in] pGraph graph to update
//! @param [in] allPoints array of arrays of points.
//! @param [in] newZWins Controls selection of z values if a new point is an exact dupliate (in x,y) of
//!     an existing vertex.   If newZWins is true, the newer z is used; else the old.
GEOMDLLIMPEXP Public void vu_insertAndRetriangulate (VuSetP pGraph, TaggedPolygonVectorCR allPoints, bool newZWins);

//! Return coordianates around all interior faces of the graph.
//! @param [in] graph
//! @param [out] polygons array of polygons.
GEOMDLLIMPEXP void vu_collectInteriorFaces (VuSetP graph, TaggedPolygonVectorR polygons);


END_BENTLEY_GEOMETRY_NAMESPACE