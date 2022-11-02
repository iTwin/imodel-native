/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* The "successor" of an edge in an extended face is reached by (1) moving to its true
* face successor and (2) continuing around that vertex for 0 or more steps until reaching
* an edge whose mask is NOT set on the edge mate.
*
* @param pGraph      IN      graph to search
* @param nodeId         IN      node whose successor is needed.
* @param mask           IN      mask which must be cleared on the edge mate of the successor.
* @returns successor node
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlRG_extendedFaceSuccessor
(
const MTGGraph     *pGraph,
MTGNodeId           nodeId,
MTGMask     mask
);

/**
* Search the graph for face loops, where faces have an extended definition based
* on a mask.   The definition assumes that the mask is atomic on each face -- either
* set throughout or clear throughout.
*
* The "successor" of an edge in the extended face is reached by (1) moving to its true
* face successor and (2) continuing around that vertex for 0 or more steps until reaching
* an edge whose mask is NOT set on the edge mate.
*
* @param pGraph      IN      graph to search
* @param pStartArray    IN OUT  Array giving an arbitraryily chosen start node on each face.
*                               Faces for a single connected component are grouped together
*                               and are bracketed by MTG_MARKER_START_COMPONENT and MTG_MARKER_END_COMPONENT.
*                           Hence the sequence is:
*                           SC f f f f EC SC f f f f EC ....
*                           where each f is the start node of a face and each SC and EC are the
*                           marker values.
*
* @param pSequenceArray IN OUT  Array giving the complete sequence of nodes around each face.  THe
*                           faces for a connected component are clustered together;  the entire cluster
*                           bracketed by MTG_MARKER_START_COMPONENT and MTG_MARKER_END_COMPONENT
*                           Each face's nodes are bracketed by MTG_MAREKR_START_FACE and MTG_MARKER_END_FACE
*                           Hence the overall sequence is:
*                               SC SF f f f EF SF f f f EF EC    SC ...SF f f f EF .... EC
* @see
* @return the number of components (contiguous blocks of face indices).
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTG_collectAndNumberExtendedFaceLoops
(
MTGGraph            *pGraph,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pSequenceArray,
MTG_MarkSet         *pMarkSet
);

/*------------------------------------------------------------------*//**
* Search the graph for face loops.   For each loop, report a seed node
* and an integer parity indicator.
*
* @param pEvenParityNodeArray IN OUT  array to accumulate one node per face (outer
*                   and holes) on even parity faces.
*                   May be NULL.
* @param pOddParityNodeArray IN OUT  array to accumulate one node per face (outer
*                   and holes) on odd parity faces.
*                   May be NULL.
*
* @param seedNodeId IN      start node for search.
*               Search type depends on face status:
*                   Any negative area face -- Cross each edge of the face.  The face on the
*                       other side is even in a recursive search.
*                   Any positive area face -- This face is even parity.  Cross each edge
*                               of each hole in the face;  at each such face start a recursive search
*                               with odd parity.
*                   MTG_NULL_NODEID -- From each true exterior face, cross each edge.
*                               The first face reached is an even parity seed for recursive
*                               search.
* @param    parityWithinComponent IN      if true, face-to-face steps within component
*               are considered parity switches.  If false, only step through hole boundary changes
*               parity.
* @param    vertexContactSufficient IN      if false, when stepping into an island only
*               faces with edge contact are enabled.  If true vertex contact is sufficient.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRG_collectAllNodesOnInwardParitySearchExt
(
RG_Header           *pRG,
EmbeddedIntArray    *pEvenParityNodeArray,
EmbeddedIntArray    *pOddParityNodeArray,
MTGNodeId           seedNodeId,
bool                parityWithinComponent,
bool                vertexContactSufficient
);

Public GEOMDLLIMPEXP void     jmdlRG_collectAllNodesOnInwardParitySearch
(
RG_Header           *pRG,
EmbeddedIntArray    *pEvenParityNodeArray,
EmbeddedIntArray    *pOddParityNodeArray,
MTGNodeId           seedNodeId
);

/**
* Search the graph for face loops, where faces have an extended definition based
* on a mask.   The definition assumes that the mask is atomic on each face -- either
* set throughout or clear throughout.
*
* The "successor" of an edge in the extended face is reached by (1) moving to its true
* face successor and (2) continuing around that vertex for 0 or more steps until reaching
* an edge whose mask is NOT set on the edge mate.
*
* @param pGraph      IN      graph to search
* @param pStartArray    IN OUT  Array giving an arbitraryily chosen start node on each face.
*                               Faces for a single connected component are grouped together
*                               and are bracketed by MTG_MARKER_START_COMPONENT and MTG_MARKER_END_COMPONENT.
*                           Hence the sequence is:
*                           SC f f f f EC SC f f f f EC ....
*                           where each f is the start node of a face and each SC and EC are the
*                           marker values.
*
* @param pSequenceArray IN OUT  Array giving the complete sequence of nodes around each face.  THe
*                           faces for a connected component are clustered together;  the entire cluster
*                           bracketed by MTG_MARKER_START_COMPONENT and MTG_MARKER_END_COMPONENT
*                           Each face's nodes are bracketed by MTG_MAREKR_START_FACE and MTG_MARKER_END_FACE
*                           Hence the overall sequence is:
*                               SC SF f f f EF SF f f f EF EC    SC ...SF f f f EF .... EC
* @see
* @return the number of components (contiguous blocks of face indices).
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlRG_collectAndNumberExtendedFaceLoops
(
RG_Header           *pRG,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pSequenceArray,
MTG_MarkSet         *pMarkSet
);

/*------------------------------------------------------------------*//**
* Collect a markset with faces that are "in" according to
* one of several bool    operations, with boundaries distinguished
* by groupId value attached to the curve of each edge.
* @param highestLeftOperandGroupId IN      highest group id which is considered
*           part of the "left" operand.
* @param [in] reverseSense if true, reverse the sense of the test.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRG_collectBooleanFaces
(
RG_Header           *pRG,
RGBoolSelect        select,
int                 highestLeftOperandGroupId,
MTG_MarkSet         *pMarkSet,
bool                reverseSense = false
);


/*------------------------------------------------------------------*//**
* Collect a markset with faces that are "in" according to
* one of several bool    operations, with boundaries distinguished
* by groupId value attached to the curve of each edge.
* Groups with ids up to (inclusive) highestOperandA are A
* Groups with ids up to (inclusive) highestOperandB are B
* Higher groups are C.
* The results is ((A boolOpAB B) boolOpC C)
* @param [in] reverseSense if true, reverse the sense of the test.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRG_collectBooleanFaces
(
RG_Header           *pRG,
RGBoolSelect        boolOpAB,
RGBoolSelect        boolOpC,
int                 highestOperandA,
int                 highestOperandB,
MTG_MarkSet         *pMarkSet,
bool                reverseSense = false
);



/*------------------------------------------------------------------*//**
* Collect a markset with faces that are "in" according to
* one of several analysis rules.
* @param [in] reverseSense if true, reverse the sense of the test.
* @param [in] activeEdgeMask if zero, no effect.   If nonzero, only edges with this mask
*      are considered for transitions.  (Mask on either side works.  Both sides are tested)
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRG_collectAnalysisFaces
(
RG_Header           *pRG,
AreaSelect        leafRule,    // Rule for in/out based on crossing counts at single leaf
BoolSelect        compositeRule,    // rule for composite in/out 
MTG_MarkSet         *pMarkSet,
bool                reverseSense = false,
MTGMask             activeEdgeMask = 0
);

/*------------------------------------------------------------------*//**
* Set a mask on (both sides of) each edge that whose group appears in the groups[] array.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlRG_setMaskByGroupArray
(
RG_Header    *pRG,      //!< containing region.
bvector<int> &groups,    //!< array of group values
MTGMask      mask,       //!< mask to set
bool         maskValue,   //!< true to set the mask, false to clear
bool         initializeSetToOppositeValue //! true to preset the entire graph to the opposite value.  false to leave the rest of the graph unchanged.
);

//! "flood" from a seed node to "reachable" faces.
//! @param [in] barrierMask mask for uncrossable edges.  This is tested on both sides.
//! @param [inout] pMarkSet face set already accepted.   Flooding will not reenter these faces.  Flood faces are added to this set.
//! @param [in] seedNode
Public GEOMDLLIMPEXP  void jmdlMTG_floodToBoundary (MTGGraph *graph, 
MTG_MarkSet &markset,
MTGMask barrierMask,
MTGNodeId seedNodeId
);

/*------------------------------------------------------------------*//**
* Walk around the given face looking across edges for a mate
* marked as a hole edge.  Return first found, or MTG_NULL_NODEID if no such found.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId  jmdlRG_stepOutOfHole
(
RG_Header *pRG,
MTGNodeId seedNodeId
);

/*------------------------------------------------------------------*//**
* Return the entire face-hole node array in flattened form with MTG_NULL_NODEID separators:
*   outerA,innerA1,innerA2,MTG_NULL_NODEID,outerB,innerB1,.....
* @param pRG IN region header
* @param pLoopArray OUT array to receive node sequences.
* @param pMarkSet IN optional mark set.
* @param bTargetMarkState IN true/false to collect only if the outer face is in/notIn the mark set.
* @param bIncludeMarkedSimpleLoops IN true to add singleton entries (nodeID+MTG_NULL_NODEID) for all
*    faces that do not have holes.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlRG_collectOuterAndInnerFaces
(
RG_Header *pRG,
EmbeddedIntArray *pLoopArray,
MTG_MarkSet *pMarkSet,
bool    bTargetMarkState,
bool    bIncludeMarkedSimpleLoops
);

struct RIMSBS_Context;

/*------------------------------------------------------------------*//**
* Return the entire face-hole node array as CurveVector content.
* @param pRG IN region header
* @param pCurves IN curve data
* @param pMarkSet IN mark set with seeds.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP CurveVectorPtr jmdlRG_collectExtendedFaces
(
RG_Header           *pRG,
RIMSBS_Context*     pCurves,
MTG_MarkSet         *pMarkSet
);

/*------------------------------------------------------------------*//**
* Return a single face CurveVector.
* The loop is marked BOUNDARY_TYPE_Outer.
* @param pRG IN region header
* @param pCurves IN curve data
* @param faceNodeId any node on the face.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP CurveVectorPtr jmdlRG_collectSimpleFace
(
RG_Header           *pRG,
RIMSBS_Context*     pCurves,
MTGNodeId           faceNodeId
);
END_BENTLEY_GEOMETRY_NAMESPACE

