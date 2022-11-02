/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* EdgeStar services.
*
* In a double-sided graph representation of a branched sheet body, the normal
* face and vertex loops define a 2-manifold which is shrink-wrapped around both
* sides of the sheet.  On each face of the two sided sheet, there are two face loops.
* Each node has access to its partner via the "partnerLabelOffset".
*
* At each edge of the sheet body, the possible configurations are:
*   1) 2 incident faces.  This is the usual two manifold case.
*   2) 1 incident face.  This is the ragged edge of the sheet.
*   3) 3 or more incident faces.  This is a junction among several sheets.
*
* Services available per node:
*   getEdgeStarSuccessor() -- take both "partner" and vertex steps to go around
*       the edge star to the next node going the same direction along the same edge.
*       This step is counter-clockwise whenv viewed with the edge pointing to the eye.
*   getEdgeStarPredecessor () -- ditto, but in opposite direction.
*   countNodesAroundEdgeStar () -- walk all the way around the edge star, counting node (hence
*           faces).
*
* setBarrierMasks () -- set masks identifying edges with 1, 2, and more incident faces,
*       and in each set of faces bounded by the non-2-manifold edges mark one side
*       with a backface bit.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/


/*---------------------------------------------------------------------------------**//**
* Depth first search to flood from a given start node to all nodes reachable without
* crossing a barrier mask.
* @param pGraph => containing graph.
* @param pStack => rubber array to hold search stack.
* @param baseNodeId => start node for search.
* @param visitMask => mask to apply as nodes are visited (both near and far sides.)
* @param barrierMask => never cross an edge which has this mask on the side being
*       left behind. (i.e. Look at this mask on the near side of an edge being crossed,
*       but not the far side.)
* @param backSideMask => mask to apply to the back side of visited faces, reached by
*       stepping through the visited face.
* @param partnerLabelOffset => offset to access the label data leading "through"
*       a face to its backside neighbor.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void floodAndMarkBackFaces
(
MTGGraph    *pGraph,
EmbeddedIntArray  *pStack,
MTGNodeId   baseNodeId,
MTGMask     visitMask,
MTGMask     barrierMask,
MTGMask     backSideMask,
int         partnerLabelOffset
)
    {
    int currNodeId;
    MTGNodeId partnerNodeId;
    /* Initialize the stack with just the seed node .. */
    jmdlEmbeddedIntArray_empty (pStack);
    jmdlEmbeddedIntArray_addInt (pStack, baseNodeId);
    /* As long as the stack has candidates ... */
    while (jmdlEmbeddedIntArray_popInt (pStack, &currNodeId))
        {
        /* skip candidates that were previously visited. */
        if (!jmdlMTGGraph_getMask (pGraph, currNodeId, visitMask))
            {
            /* Mark the face on this side ... */
            jmdlMTGGraph_setMaskAroundFace
                        (
                        pGraph, currNodeId, visitMask
                        );
            /* And also the one on the other side ... */
            if (  jmdlMTGGraph_getLabel (pGraph, &partnerNodeId, currNodeId, partnerLabelOffset)
               && partnerNodeId != MTG_NULL_NODEID
               )
                {
                jmdlMTGGraph_setMaskAroundFace
                            (
                            pGraph,
                            partnerNodeId,
                            visitMask | backSideMask
                            );
                }
            /* For each edge of the face .... */
            MTGARRAY_FACE_LOOP (faceNodeId, pGraph, currNodeId)
                {
                /* Cautiously step across and push opposite side on candidate stack. */
                if (!jmdlMTGGraph_getMask (pGraph, faceNodeId, barrierMask))
                    {
                    jmdlEmbeddedIntArray_addInt (pStack,
                        jmdlMTGGraph_getEdgeMate (pGraph, faceNodeId));
                    }
                }
            MTGARRAY_END_FACE_LOOP (faceNodeId, pGraph, currNodeId)
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Get the successor of a node around its edge star, i.e. counterclockwise around the edge
* when viewed with the edge pointing at the eye.
bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGGraph_getEdgeStarSucc
(
const MTGGraph    *pGraph,
MTGNodeId       nodeId,
int             partnerLabelOffset
)
    {
    MTGNodeId vpredNodeId = jmdlMTGGraph_getVPred (pGraph, nodeId);
    MTGNodeId partnerNodeId;
    if (jmdlMTGGraph_getLabel (pGraph, &partnerNodeId, vpredNodeId, partnerLabelOffset))
        {
        return partnerNodeId;
        }
    return MTG_NULL_NODEID;
    }

/*---------------------------------------------------------------------------------**//**
* Get the predecessor of a node around its edge star, i.e. clockwise around the edge
* when viewed with the edge pointing at the eye.
bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGGraph_getEdgeStarPred
(
const MTGGraph    *pGraph,
MTGNodeId       nodeId,
int             partnerLabelOffset
)
    {
    MTGNodeId partnerNodeId;
    if (jmdlMTGGraph_getLabel (pGraph, &partnerNodeId, nodeId, partnerLabelOffset)
        && partnerNodeId != MTG_NULL_NODEID)
        {
        return jmdlMTGGraph_getVSucc (pGraph, partnerNodeId);
        }
    return MTG_NULL_NODEID;
    }

/*---------------------------------------------------------------------------------**//**
* Get a node's partner "through" its face.  This is the node at the same vertex
* and on the other side of the face.
bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGGraph_getFacePartner
(
const MTGGraph    *pGraph,
MTGNodeId       nodeId,
int             partnerLabelOffset
)
    {
    MTGNodeId partnerNodeId;
    return jmdlMTGGraph_getLabel (pGraph, &partnerNodeId, nodeId, partnerLabelOffset)
            ? partnerNodeId : MTG_NULL_NODEID;
    }

static int s_maxStarLoopCount = 10000;
/*************************************************************************************
** Macros for walking edge star loop.s
** Basic logic is a repeat loop for a cyclic loop.
** Because edge stars are a less formal data structure than face or vertex loops,
** the macros are a little different from vertex and face loop macros.
** 1) loop var must be declared by the surrounding code.
** 2) on exit from a successful loop, the loop var is equal to teh seedNodeId.
** 3) on exit due to count error the loop var is the node that would be visited
**      on the next pass.
** 4) on exit due to failure to reach the edge star successor, the loop var is MTG_NULL_NODEID
**
** A normal use program should look for the normal successful case currNodeId == seedNodeId,
** but not bother distinguishing among the errors.
**************************************************************************************/

#define MTGARRAY_EDGE_STAR_LOOP(_currNodeId_,_pGraph_,_seedNodeId_,_partnerLabelOffset_)  \
        {                                           \
        int  starLoopCount = 0;                     \
        _currNodeId_ = _seedNodeId_;                \
        for (;_currNodeId_ != MTG_NULL_NODEID;)     \
            {

#define MTGARRAY_END_EDGE_STAR_LOOP(_currNodeId_,_pGraph_,_seedNodeId_,_partnerLabelOffset_)  \
            _currNodeId_ = jmdlMTGGraph_getEdgeStarSucc             \
                    (_pGraph_, _currNodeId_, _partnerLabelOffset_); \
            if (_currNodeId_ == _seedNodeId_)                       \
                break;                                              \
            if (++starLoopCount >= s_maxStarLoopCount)              \
                break;                                              \
            }\
        }


/*---------------------------------------------------------------------------------**//**
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_countNodesAroundEdgeStar
(
const MTGGraph    *pGraph,
MTGNodeId       seedNodeId,
int             partnerLabelOffset
)
    {
    int count = 0;
    MTGNodeId currNodeId = seedNodeId;
    MTGARRAY_EDGE_STAR_LOOP (currNodeId,pGraph,seedNodeId,partnerLabelOffset)
        {
        count++;
        }
    MTGARRAY_END_EDGE_STAR_LOOP (currNodeId,pGraph,seedNodeId,partnerLabelOffset)
    if (currNodeId == seedNodeId)
        return count;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @return the number edges set in the star loop.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_setOrClearMaskAroundEdgeStar
(
MTGGraph        *pGraph,
MTGNodeId       seedNodeId,
int             partnerLabelOffset,
MTGMask         mask,
bool            newValue,
bool            markEdgeMates
)
    {
    int count = 0;
    MTGNodeId currNodeId;
    MTGARRAY_EDGE_STAR_LOOP (currNodeId,pGraph,seedNodeId,partnerLabelOffset)
        {
        count++;
        if (newValue)
            {
            jmdlMTGGraph_setMask (pGraph, currNodeId, mask);
            if (markEdgeMates)
                jmdlMTGGraph_setMask (pGraph, jmdlMTGGraph_getEdgeMate (pGraph, currNodeId), mask);
            }
        else
            {
            jmdlMTGGraph_clearMask (pGraph, currNodeId, mask);
            if (markEdgeMates)
                jmdlMTGGraph_clearMask (pGraph, jmdlMTGGraph_getEdgeMate (pGraph, currNodeId), mask);
            }
        }
    MTGARRAY_END_EDGE_STAR_LOOP (currNodeId,pGraph,seedNodeId,partnerLabelOffset)
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* Search a double-sided face structure to (a) mark special edges at the fringe
* and multiple-face internal junctions, and (b) assign front and back faces.
* Some typical mask combinations are:
*<LIST>
*<LI>(m0,null,null) -- Mark just the fringe. Leave internal T junctions unmarked.</LI>
*<LI>(m0,null,m0) -- Mark the fringe and T junctions with the same mask.</LI>
*<LI>(m0,null,m1) -- Mark the fringe and T junctions with distinct masks.
*</LIST>
*
* The backsideMask is typically an exterior mask.
*
* The front/back mark is skipped if (a) backsideMask is null or (b) mask1 and mask3 are both
*   null.
*
* @param partnerLabelOffset => at each node, this label lead through the face to the
*           node's partner on the opposite side of the face.
* @param mask1 => mask to apply on edges with only a single incident face. (These
*       are exposed fringe edges.)
* @param mask2 => mask to apply to edges with exactly two incident faces (these are
*       conventional interior edges).
* @param mask3 => mask to apply to edges with 3 or more incident faces (these
*       are T junctions.)
* @param backsideMask => mask to apply to an arbitrarily assigned backside of
*       faces.  This mask is applied during a search finding connected components not
*       broken by a barrier of mask1 | mask3 edges.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setEdgeStarClassificationMasks
(
MTGGraph    *pGraph,
int        partnerLabelOffset,
MTGMask     mask1,
MTGMask     mask2,
MTGMask     mask3,
MTGMask     backsideMask
)
    {
    int numFace;
    MTGMask visitMask = jmdlMTGGraph_grabMask (pGraph);
    EmbeddedIntArray *pStack = jmdlEmbeddedIntArray_grab ();
    /* Combine the caller's masks with a visit mask. */
    MTGMask extendedMask1 = visitMask | mask1;
    MTGMask extendedMask2 = visitMask | mask2;
    MTGMask extendedMask3 = visitMask | mask3;
    MTGMask floodBarrierMask = mask1 | mask3;

    jmdlMTGGraph_clearMaskInSet (pGraph, mask1 | mask2 | mask3 | visitMask);

    if (mask1 || mask2 || mask3)
        {
        MTGARRAY_SET_LOOP (baseNodeId, pGraph)
            {
            if (!jmdlMTGGraph_getMask (pGraph, baseNodeId, visitMask))
                {
                numFace = jmdlMTGGraph_countNodesAroundEdgeStar (pGraph, baseNodeId, partnerLabelOffset);
                if (numFace == 2)
                    {
                    jmdlMTGGraph_setOrClearMaskAroundEdgeStar (pGraph, baseNodeId, partnerLabelOffset,
                                        extendedMask2, true, true);
                    }
                else if (numFace == 1)
                    {
                    jmdlMTGGraph_setOrClearMaskAroundEdgeStar (pGraph, baseNodeId, partnerLabelOffset,
                                        extendedMask1, true, true);
                    }
                else
                    {
                    jmdlMTGGraph_setOrClearMaskAroundEdgeStar (pGraph, baseNodeId, partnerLabelOffset,
                                        extendedMask3, true, true);
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (baseNodeId, pGraph)
        }

    if (backsideMask && floodBarrierMask)
        {
        jmdlMTGGraph_clearMaskInSet (pGraph, backsideMask | visitMask);
        /*
            When nodes are added using jmdlMTGFacets_addDoubleFace (as is
            usually the case before this function is called), the even MTG nodes
            represent the side of the face that preserves the orientation of the
            vertices passed into that function.  For subsequent calculations
            (e.g., normals), preserving this orientation is helpful.

            Thus, for each flood component (delimited by the floodBarrierMask),
            we start with an even node, and declare it to be on the "interior"
            or "front" side of the MTG.  This ensures that the "interior" faces
            of each flood component will share the original orientation of the
            face containing the (even) node that started the flood of that
            component.  NOTE: different flood components may still have opposite
            orientations; it is best to have one flood component per MTG.

            Faces in a flood component that have original orientation opposite
            the flood-starting node's original face are thus precisely those
            faces whose nodes are "interior" and odd.

            DA4 3/01

            EDL 11/01 -- I don't like "only start at even faces".
                Compromise -- start at even faces first passs, anything second pass..

        */

        /* First pass -- even faces. */
        MTGARRAY_SET_LOOP (baseNodeId, pGraph)
            {

            if (   !(baseNodeId % 2)
                && !jmdlMTGGraph_getMask (pGraph, baseNodeId, visitMask))
                {
                floodAndMarkBackFaces (pGraph, pStack, baseNodeId, visitMask,
                    floodBarrierMask, backsideMask, partnerLabelOffset);
                }
            }
        MTGARRAY_END_SET_LOOP (baseNodeId, pGraph)

        /* Second pass -- any unvisited face. */
        MTGARRAY_SET_LOOP (baseNodeId, pGraph)
            {
            if (!jmdlMTGGraph_getMask (pGraph, baseNodeId, visitMask))
                {
                floodAndMarkBackFaces (pGraph, pStack, baseNodeId, visitMask,
                    floodBarrierMask, backsideMask, partnerLabelOffset);
                }
            }
        MTGARRAY_END_SET_LOOP (baseNodeId, pGraph)

        }

    jmdlEmbeddedIntArray_drop (pStack);
    jmdlMTGGraph_dropMask (pGraph, visitMask);
    }

// double sided facets ctor .. immediately set up through-the-face partner tag
MTGDoubleFaceFacets::MTGDoubleFaceFacets(double newFaceVertexTolerance) : MTGFacets(MTG_Facets_VertexOnly) {
    m_newFaceVertexTolerance = std::max (1.0e-10, newFaceVertexTolerance);
    m_partnerLabel = graphHdr.DefineLabel(
        MTG_NODE_PARTNER_TAG,
        MTG_LabelMask_SectorProperty,
        -1);
    m_modalFaceValue = -1;
    m_modalFaceLabel = -1;
    }

void MTGDoubleFaceFacets::DefineModalFaceLabel(int initialModalValue, int defaultValue)
    {
    m_modalFaceLabel = graphHdr.DefineLabel(
        MTG_NODE_FACE_TAG,
        MTG_LabelMask_FaceProperty,
        defaultValue);
    m_modalFaceValue = initialModalValue;
    m_defaultModalFaceValue = defaultValue;
    }

int MTGDoubleFaceFacets::GetFaceValueInNode(MTGNodeId node) const
    {
    int value;
    if (graphHdr.TryGetLabel(node, m_modalFaceLabel, value))
        return value;
    return m_defaultModalFaceValue;
    }

int MTGDoubleFaceFacets::SetModalFaceValue(int value)
    {
    int oldValue = m_modalFaceValue;
    m_modalFaceValue = value;
    return oldValue;
    }

MTGNodeId MTGDoubleFaceFacets::GetPartnerNode(MTGNodeId node) const
    {
    int value;
    if (graphHdr.TryGetLabel(node, m_modalFaceLabel, value))
        return value;
    return MTG_NULL_NODEID;
    }

MTGNodeId MTGDoubleFaceFacets::AddDoubleSidedFace(bvector<DPoint3d> const &xyzIn)
    {
    bvector<DPoint3d> xyzCompressed = xyzIn;
    DPoint3dOps::CompressCyclic(xyzCompressed, m_newFaceVertexTolerance);
    if (xyzCompressed.size () < 3)
        return MTG_NULL_NODEID;
    MTGNodeId baseNodeId = MTG_NULL_NODEID;

    for (auto &xyz : xyzCompressed)
        {
        MTGNodeId leftNodeId, rightNodeId;
        graphHdr.SplitEdge (leftNodeId, rightNodeId, baseNodeId);
        size_t pointIndex = AddPoint (xyz);
        graphHdr.TrySetLabel(leftNodeId, vertexLabelOffset, (int)pointIndex);
        graphHdr.TrySetLabel(rightNodeId, vertexLabelOffset, (int)pointIndex);
        graphHdr.TrySetLabel(leftNodeId, m_partnerLabel, rightNodeId);
        graphHdr.TrySetLabel(rightNodeId, m_partnerLabel, leftNodeId);
        if (m_modalFaceLabel >= 0)
            {
            graphHdr.TrySetLabel(leftNodeId, m_modalFaceLabel, m_modalFaceValue);
            graphHdr.TrySetLabel(rightNodeId, m_modalFaceLabel, m_modalFaceValue);
            }
        baseNodeId = leftNodeId;
        }
    return baseNodeId;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
