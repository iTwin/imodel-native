/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Vu/VuOps.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
// To be included from VuApi.h !!!
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#ifdef __cplusplus

/**
@description class wrapper for static utiliity functions on Vu graphs.
*/
struct VuOps
{
private:
    VuOps (); // No instances.
public:
/**
@description Consruct a (single) loop from a contiguous subset of an
   array of points.  Omit duplicates within an xy tolerance.
  No disconnect logic -- next level up must pick index intervals.
  Each node is labeled with the index of its corresponding xyz coordinates.
@param graphP IN pointer to receiving graph.
@param pXYZIn IN input points.
@param iFirst IN index of first point of loop.
@param iLast IN index of last point of loop.
@param leftMask IN mask for left side of edges.
@param rightmask IN mask for right side of edges.
@param xyTolerance IN tolerance for declaring adjacent points identical.
@return pointer to some Vu node in the loop.
*/
static GEOMDLLIMPEXP VuP MakeIndexedLoopFromPartialArray
(
VuSetP          graphP,
bvector<DPoint3d> const*pXYZIn,
size_t          iFirst,
size_t          iLast,
VuMask          leftMask,
VuMask          rightMask,
double          xyTolerance
);


/**
@description Consruct loops from an array of points with disconnect points separating
    the loops.   Omit duplicates within an xy tolerance.
  Each node is labeled with the index of its corresponding xyz coordinates.
@param graphP IN pointer to receiving graph.
@param pXYZIn IN input points.
@param leftMask IN mask for left side of edges.
@param rightmask IN mask for right side of edges.
@param xyTolerance IN tolerance for declaring adjacent points identical.
@return pointer to some Vu node in the loop.
*/
static GEOMDLLIMPEXP VuP MakeIndexedLoopsFromArrayWithDisconnects
(
VuSetP          graphP,
bvector<DPoint3d> const *pXYZIn,
VuMask          leftMask,
VuMask          rightMask,
double          xyTolerance
);

/**
@description Consruct an edge with coordinates and masks.lyz coordinates.
@param graphP IN pointer to receiving graph.
@param xyzA IN start coordinates
@param xyzB IN end coordinates
@param leftMask IN mask for left side of edge
@param rightmask IN mask for right side of edge
@return pointer to VuNode at start
*/
static GEOMDLLIMPEXP VuP MakeEdge 
(
VuSetP          graphP,
DPoint3dCR      xyzA,
DPoint3dCR      xyzB,
VuMask          leftMask,
VuMask          rightMask
);
//! unconditional construction with coordinates and masks.
static GEOMDLLIMPEXP void MakeEdge 
(
VuSetP          graphP,
VuP             &nodeA,
VuP             &nodeB,
DPoint3dCR      xyzA,
DPoint3dCR      xyzB,
VuMask          leftMask,
VuMask          rightMask
);


static GEOMDLLIMPEXP void MakeEdges
(
VuSetP          graph,
VuP             &chainTail,
VuP             &chainHead,
bvector<DPoint3d> const &xyz,
VuMask          leftMask,
VuMask          rightMask,
double          abstol = 0.0
);
//! Make a loop with coordinates in an array.
static GEOMDLLIMPEXP void MakeLoop
(
VuSetP          graph,
VuP             &leftNode,
VuP             &rightNode,
bvector<DPoint3d> const &xyz,
VuMask          leftMask,
VuMask          rightMask,
double          abstol = 0.0
);

//! Add a rectangle (with subdivided edges) "outside" of the given range, using long edge count to control exterior subdivision.
static GEOMDLLIMPEXP VuP AddRangeBase
(
VuSetP          pGraph,         //!< [inout] graph
DRange3dR       outRange,       //!< [out] range of constructed edges
DRange3dCR      inRange,        //!< [in] input range.
double          absDelta,       //!< [in] minimum allowed expansion
double          relDelta,       //!< [in] relative expansion (fraction of diagonal)
size_t          numOnLongEdge,  //!< [in] target number of edges to place on the longer side
VuMask          interiorMask,   //!< [in] mask for inside of rectangle.
VuMask          exteriorMask    //!< [in] mask for outside of rectangle.
);

//! Add a rectangle (with subdivided edges) "outside" of the given range, using specified (numPoint) count of "interior" points to control the exterior subdivision.
static GEOMDLLIMPEXP VuP AddExpandedRange
(
VuSetP pGraph,          //!< [inout] graph
DRange3dCR localRange,  //!< [in] base range, to be expanded
size_t numPoint,        //!< [in] approximate number of points in the graph. More points causes more subdivisions so outer edge lengths are similar to interior ege lengths.
double expansionFracton,//!< [in] added edges are on a rectangle expanded by this fraction of the diagonal.
VuMask insideMask,      //!< [in] mask for inside of rectangle.
VuMask outsideMask      //!< [in] mask for outside of rectangle.
);

};
#endif

END_BENTLEY_GEOMETRY_NAMESPACE
