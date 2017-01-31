/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/jmdl_mtgclip.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
#include "mtgclip.hxx"
#include "../memory/jmdl_iarray.fdf"
#include "../memory/jmdl_dpnt3.fdf"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::OmdlMTG_MergeHandler              |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
OmdlMTG_MergeHandler::OmdlMTG_MergeHandler
(
MTGFacets * pFacetHeader,
MTGMask cutMask
)
    {
    m_pFacetHeader = pFacetHeader;
    m_pGraph       = (&pFacetHeader->graphHdr);
    //vertexLabelOffset = pFacetHeader->vertexLabelOffset;
    m_isShortCircuited = false;
    m_returnMask = m_edgeSplitTransferMask = m_faceSplitTransferMask
            = MTG_NULL_MASK;

    m_returnMask |= (m_opMask    = jmdlMTGGraph_grabMask (m_pGraph));
    m_returnMask |= (m_joinMask  = jmdlMTGGraph_grabMask (m_pGraph));
    m_returnMask |= (m_splitMask = jmdlMTGGraph_grabMask (m_pGraph));
    m_cutMask = cutMask;

    m_pAltitudeHeader = jmdlEmbeddedDPoint3dArray_grab ();
    m_pFaceSeedNodeArray = jmdlEmbeddedIntArray_grab ();
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::~OmdlMTG_MergeHandler             |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
OmdlMTG_MergeHandler::~OmdlMTG_MergeHandler
(
)
    {
    jmdlMTGGraph_dropMask (m_pGraph, m_returnMask);
    jmdlEmbeddedDPoint3dArray_drop (m_pAltitudeHeader);
    jmdlEmbeddedIntArray_drop (m_pFaceSeedNodeArray);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::applyTransform                    |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::applyTransform
(
const Transform *pTransform
)
    {
    jmdlVArrayDPoint3d_applyTransform2 ((&m_pFacetHeader->vertexArrayHdr), m_pAltitudeHeader, pTransform);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::getLocalRange                     |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
bool    OmdlMTG_MergeHandler::boolean_getLocalRange
(
DRange3d *pRange
)
    {
    return SUCCESS == jmdlVArrayDPoint3d_getRange (m_pAltitudeHeader, pRange);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::getLocalFaceRange                 |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
bool    OmdlMTG_MergeHandler::boolean_getLocalFaceRange
(
DRange3d *pRange,
MTGNodeId seedNodeId
)
    {
    DPoint3d currPoint;

    int numPoint = 0;
    bsiDRange3d_init (pRange);
    MTGARRAY_FACE_LOOP (currNodeId, m_pGraph, seedNodeId)
        {
        if (jmdlEmbeddedDPoint3dArray_getDPoint3d (m_pAltitudeHeader, &currPoint, currNodeId))
            {
            bsiDRange3d_extendByDPoint3d (pRange, &currPoint);
            numPoint++;
            }
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, m_pGraph, seedNodeId)
    return numPoint > 0;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::splitFaceByLocalZ                 |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::splitFaceByLocalZ
(
MTGNodeId seedNodeId
)
    {
    }



/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::startConvexSet                    |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::startConvexSet
(
const DPoint4d *pHPlane,
int      numPlane
)
    {
    // Get a mask to mark classified faces:
    switch (m_clipType)
        {
        case MTG_ClipOp_KeepIn:
            // No marking for inside clip -- edges get thrown out as soon
            // as detected.
            m_isShortCircuited = false;
            break;
        case MTG_ClipOp_KeepOut:
            // All ON and OUT edges will be marked for safekeeping.  Clear them first.
            m_faceSplitTransferMask = m_edgeSplitTransferMask = m_opMask;
            jmdlMTGGraph_clearMaskInSet (m_pGraph, m_faceSplitTransferMask);
            m_isShortCircuited = false;
            break;
        case MTG_ClipOp_KeepOn:
            // All ON edges will be marked for safekeeping.  Clear them first.
            m_edgeSplitTransferMask = m_opMask;
            jmdlMTGGraph_clearMaskInSet (m_pGraph, m_faceSplitTransferMask);
            m_isShortCircuited = false;
            break;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::endConvexSet                      |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::endConvexSet
(
const DPoint4d *pHPlane,
int      numPlane
)
    {
    if (m_isShortCircuited)
        {
        }
    else if (m_clipType == MTG_ClipOp_KeepOut)
        {
        // OUT faces are marked.
        // Examine both sides of each edge.
        //      left    right
        //      OUT     OUT             leave it in place
        //      IN      OUT             mark IN side EXTERIOR
        //      OUT     IN
        //      IN      IN              delete it
        MTGNodeId mateId;
        MTGMask nodeMask, mateMask;

        MTGARRAY_SET_LOOP (nodeId, m_pGraph)
            {
            mateId = jmdlMTGGraph_getEdgeMate (m_pGraph, nodeId);
            if (mateId > nodeId)
                {
                nodeMask = jmdlMTGGraph_getMask (m_pGraph, nodeId, m_edgeSplitTransferMask);
                mateMask = jmdlMTGGraph_getMask (m_pGraph, mateId, m_edgeSplitTransferMask);
                if (nodeMask)
                    {
                    if (mateMask)
                        {
                        }
                    else
                        {
                        jmdlMTGGraph_setMask (m_pGraph, mateId, MTG_EXTERIOR_MASK);
                        }
                    }
                else
                    {
                    if (mateMask)
                        {
                        jmdlMTGGraph_setMask (m_pGraph, nodeId, MTG_EXTERIOR_MASK);
                        }
                    else
                        {
                        jmdlMTGGraph_dropEdge (m_pGraph, nodeId);
                        }
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, m_pGraph)
        }
    else if (m_clipType == MTG_ClipOp_KeepOn)
        {
        // ON edges are marked.  Delete any edge for which neither side is marked.
        MTGNodeId mateId;
        MTGMask nodeMask, mateMask;

        MTGARRAY_SET_LOOP (nodeId, m_pGraph)
            {
            mateId = jmdlMTGGraph_getEdgeMate (m_pGraph, nodeId);
            if (mateId > nodeId)
                {
                nodeMask = jmdlMTGGraph_getMask (m_pGraph, nodeId, m_edgeSplitTransferMask);
                mateMask = jmdlMTGGraph_getMask (m_pGraph, mateId, m_edgeSplitTransferMask);
                if (nodeMask || mateMask)
                    {
                    }
                else
                    {
                    jmdlMTGGraph_dropEdge (m_pGraph, nodeId);
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, m_pGraph)
        }


    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::popClipType                       |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
MTGClipOp OmdlMTG_MergeHandler::popClipType
(
)
    {
    return m_clipType;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::pushClipType                      |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::pushClipType
(
MTGClipOp clipType
)
    {
    m_clipType = clipType;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::startPlaneOfConvexSet             |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::startPlaneOfConvexSet
(
const DPoint4d *pHPlane,
int      numPlane,
int      i
)
    {
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::endPlaneOfConvexSet               |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::endPlaneOfConvexSet
(
const DPoint4d *pHPlane,
int      numPlane,
int      i
)
    {
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::isShortCircuited                  |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
bool    OmdlMTG_MergeHandler::isShortCircuited
(
)
    {
    return m_isShortCircuited;
    }



/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::announceAllOut                    |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::announceAllOut
(
)
    {
    switch (m_clipType)
        {
        case MTG_ClipOp_KeepIn:
            m_isShortCircuited = true;
            jmdlMTGGraph_empty (m_pGraph);
            jmdlEmbeddedDPoint3dArray_empty ((&m_pFacetHeader->vertexArrayHdr));
            break;
        case MTG_ClipOp_KeepOut:
            m_isShortCircuited = true;
            break;
        case MTG_ClipOp_KeepOn:
            // nothing to do.   No ON edges to mark.
            break;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::announceAllOn                     |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::announceAllOn
(
)
    {
    switch (m_clipType)
        {
        case MTG_ClipOp_KeepIn:
            // ON is as good as in, at least for this pass, but later passes still
            // have to run.
            break;
        case MTG_ClipOp_KeepOut:
            // ON is as good as OUT.  Keep it all.
            m_isShortCircuited = true;
            break;
        case MTG_ClipOp_KeepOn:
            // Everything is good.
            m_isShortCircuited = true;
            break;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::announceAllIn                     |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::announceAllIn
(
)
    {
    // ALL IN doesn't tell us anything strong enough to either delete edges
    // or mark them for inclusion.  Seems a little strange to have no particular use
    // for this condition.
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::announceEdgeClassification        |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::announceEdgeClassification
(
EmbeddedIntArray  *pEdgeClassification
)
    {
    MTG_SectionEdgeClassification nodeClass;

    if (m_clipType == MTG_ClipOp_KeepOut)
        {
        // Annotate all On, OnAbove, and Above faces
        MTGARRAY_SET_LOOP (nodeId, m_pGraph)
            {
            if (jmdlEmbeddedIntArray_getInt (pEdgeClassification, (int*)&nodeClass, nodeId))
                {
                if (nodeClass & MTG_SECTIONEDGE_BELOWBIT)
                    {
                    // The face is below (but the edge might not be).  Leave it unmarked
                    }
                else
                    {
                    // The face is ON or ABOVE.  Keep it!!
                    jmdlMTGGraph_setMask (m_pGraph, nodeId, m_edgeSplitTransferMask);
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, m_pGraph)
        }
    else if (m_clipType == MTG_ClipOp_KeepIn)
        {
        // Throw away all Above edges, and convert OnAbove to EXTERIOR
        // At the, end there should be complete loops of EXTERIOR edges where faces got sheared
        // off.  This is hard to reason about.
        MTGARRAY_SET_LOOP (nodeId, m_pGraph)
            {
            if (jmdlEmbeddedIntArray_getInt (pEdgeClassification, (int*)&nodeClass, nodeId))
                {
                if (nodeClass == MTG_SectionEdge_OnAbove)
                    {
                    jmdlMTGGraph_setMask (m_pGraph, nodeId, MTG_EXTERIOR_MASK);
                    }
                else if (nodeClass == MTG_SectionEdge_Above)
                    {
                    jmdlMTGGraph_dropEdge (m_pGraph, nodeId);
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, m_pGraph)
        }
    else if (m_clipType == MTG_ClipOp_KeepOn)
        {
        // Annotate all ON edges
        MTGARRAY_SET_LOOP (nodeId, m_pGraph)
            {
            if (jmdlEmbeddedIntArray_getInt (pEdgeClassification, (int*)&nodeClass, nodeId))
                {
                if (nodeClass & MTG_SECTIONEDGE_ONBIT)
                    {
                    // The face is ON.  Keep it!!
                    jmdlMTGGraph_setMask (m_pGraph, nodeId, m_edgeSplitTransferMask);
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, m_pGraph)
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::announceEdgeSplit                 |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
| Copy the mask bit(s) of the m_edgeSplitTransferMask from the parent   |
| nodes to the split nodes.                                             |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::announceEdgeSplit
(
MTGNodeId belowNodeId,     // The node below the section plane
MTGNodeId upNodeId,         // The (new) node pointing up from the section plane
MTGNodeId downNodeId        // The (new) node pointing down from the section plane
)
    {
    if (m_splitMask)
        {
        jmdlMTGGraph_setMask (m_pGraph, upNodeId,   m_splitMask);
        jmdlMTGGraph_setMask (m_pGraph, downNodeId, m_splitMask);
        }

    if (m_joinMask && jmdlMTGGraph_getMask (m_pGraph, belowNodeId, m_joinMask))
        {
        jmdlMTGGraph_setMask (m_pGraph, upNodeId, m_joinMask);
        jmdlMTGGraph_setMask (m_pGraph, downNodeId, m_joinMask);
        }

    if (m_edgeSplitTransferMask)
        {
        // Copy the prior OUT settings along each edge
        MTGNodeId mateId = jmdlMTGGraph_getEdgeMate (m_pGraph, upNodeId);
        MTGMask upMask = jmdlMTGGraph_getMask (m_pGraph, belowNodeId, m_edgeSplitTransferMask );
        MTGMask downMask = jmdlMTGGraph_getMask (m_pGraph, mateId, m_edgeSplitTransferMask );
        if (upMask)
            jmdlMTGGraph_setMask (m_pGraph, upNodeId, upMask);
        if (downMask)
            jmdlMTGGraph_setMask (m_pGraph, downNodeId, downMask);
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::announceJoin                      |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
| Copy the mask bit(s) of the m_faceSplitTransferMask from the parent   |
| nodes to the split nodes.                                             |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::announceJoin
(
MTGNodeId leftNodeId,       // The preexisting start node
MTGNodeId rightNodeId,     // The preexisting end node
MTGNodeId startNodeId,     // The new start node
MTGNodeId endNodeId,        // The new end node
bool       markCut          // true to trigger application of m_cutMask
)
    {
    MTGMask joinMask = m_joinMask;

    if (markCut)
        joinMask |= m_cutMask;

    if (joinMask)
        {
        jmdlMTGGraph_setMask (m_pGraph, startNodeId, joinMask );
        jmdlMTGGraph_setMask (m_pGraph, endNodeId,   joinMask );
        }

    if (m_faceSplitTransferMask)
        {
        // Copy the prior OUT settings along each edge
        MTGMask leftMask = jmdlMTGGraph_getMask (m_pGraph, leftNodeId, m_faceSplitTransferMask );
        MTGMask rightMask = jmdlMTGGraph_getMask (m_pGraph, rightNodeId, m_faceSplitTransferMask );
        if (leftMask)
            jmdlMTGGraph_setMask (m_pGraph, endNodeId, leftMask);
        if (rightMask)
            jmdlMTGGraph_setMask (m_pGraph, startNodeId, rightMask);
        }
    }

static bool    isNotMasked
(
const MTGGraph *     pGraph,     // => The containing graph
MTGNodeId           nodeId,     // => node to test
MTGMask     mask,       // => mask to test
void                *pUserData  // => not used
)
    {
    return !jmdlMTGGraph_getMask (pGraph, nodeId, mask);
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::removeExtraNodes                  |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
| Remove nodes that are marked as JOIN edges but are not needed.        |
+----------------------------------------------------------------------*/
void OmdlMTG_MergeHandler::removeExtraNodes
(
)
    {
    if (m_clipType == MTG_ClipOp_KeepIn || m_clipType == MTG_ClipOp_KeepOn)
        return;

    MTGMask masksToRead  = MTG_EXTERIOR_MASK | m_splitMask | m_joinMask;
    // Remove JOIN marks from exterior edges -- test that everything matches
    MTGNodeId node1Id;
    MTGNodeId node2Id;
    MTGMask node0Mask;
    MTGMask node1Mask;
    MTGMask combinedMask;

    EmbeddedIntArray *pTreeEdge = jmdlEmbeddedIntArray_grab ();

    int errors = 0;
    // Clear the join masks from things that turned out to be exterior.
    MTGARRAY_SET_LOOP (node0Id, m_pGraph)
        {
        node1Id = jmdlMTGGraph_getEdgeMate (m_pGraph, node0Id);
        if (node0Id < node1Id)
            {
            node0Mask = jmdlMTGGraph_getMask (m_pGraph, node0Id, masksToRead);
            node1Mask = jmdlMTGGraph_getMask (m_pGraph, node1Id, masksToRead);
            combinedMask = node0Mask | node1Mask;

            if ((node0Mask & m_joinMask) != (node1Mask & m_joinMask))
                {
                errors ++;
                }

            if (   (combinedMask & m_joinMask)
                && (combinedMask & MTG_EXTERIOR_MASK))
                {
                jmdlMTGGraph_clearMask (m_pGraph, node0Id, m_joinMask);
                jmdlMTGGraph_clearMask (m_pGraph, node1Id, m_joinMask);
                }
            }
        }
    MTGARRAY_END_SET_LOOP (node0Id, m_pGraph)

    // Find the join nodes that are really needed to maintain connectivity:
    jmdlMTGGraph_prioritizedSpanningTree
                (m_pGraph, pTreeEdge, NULL, NULL, isNotMasked, m_joinMask, NULL);

    // Take the join mark off of those also
    int i;
    MTGNodeId node0Id;
    for (i = 0; jmdlEmbeddedIntArray_getInt (pTreeEdge, &node0Id, i); i++)
        {
        node1Id = jmdlMTGGraph_getEdgeMate (m_pGraph, node0Id);
        node0Mask = jmdlMTGGraph_getMask (m_pGraph, node0Id, masksToRead);
        node1Mask = jmdlMTGGraph_getMask (m_pGraph, node1Id, masksToRead);
        combinedMask = node0Mask | node1Mask;

        if ((node0Mask & m_joinMask) != (node1Mask & m_joinMask))
            {
            errors ++;
            }

        if (combinedMask & m_joinMask)
            {
            jmdlMTGGraph_clearMask (m_pGraph, node0Id, m_joinMask);
            jmdlMTGGraph_clearMask (m_pGraph, node1Id, m_joinMask);
            }
        }

    // Drop remaining join edges:
    jmdlMTGGraph_dropByMask (m_pGraph, m_joinMask, 1);

    // Search for edges to heal
    MTGARRAY_SET_LOOP (node0Id, m_pGraph)
        {
        if (jmdlMTGGraph_getMask (m_pGraph, node0Id, m_splitMask))
            {
            node1Id = jmdlMTGGraph_getVSucc (m_pGraph, node0Id);
            node2Id = jmdlMTGGraph_getVSucc (m_pGraph, node1Id);
            if (node2Id == node0Id)
                {
                jmdlMTGGraph_healEdge  (m_pGraph, node0Id);
                }
            }
        }
    MTGARRAY_END_SET_LOOP (node0Id, m_pGraph)
    jmdlEmbeddedIntArray_drop (pTreeEdge);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::isEdgeSplitable                   |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
bool    OmdlMTG_MergeHandler::isEdgeSplitable
(
MTGNodeId  baseNodeId       // base node of edge.
)
    {
    return true;
    }

typedef struct
    {
    MTGNodeId           nodeId;
    int                 type;
    double              sortCoordinate;
    DPoint3d            point;
    } MTG_Crossing;

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGClip_compareCrossings
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     jmdlMTGClip_sign
(
double  z0       // => value whose sign is needed
)
    {
    if (z0 < 0.0)
        return -1;
    if (z0 > 0.0)
        return 1;
    return 0;
    }


#define CROSS_MM (0x00000001)
#define CROSS_M0 (0x00000002)
#define CROSS_MP (0x00000004)
#define CROSS_0M (0x00000008)
#define CROSS_00 (0x00000010)
#define CROSS_0P (0x00000020)
#define CROSS_PM (0x00000040)
#define CROSS_P0 (0x00000080)
#define CROSS_PP (0x00000100)
#define CROSS_TYPES 9

typedef enum
    {
    CrossState_OUT,
    CrossState_INLR, // Currently IN, due to a DOWN crossing, hence UP crossing expected for exit
    CrossState_INRL, // Currently IN, due to an UP crossing, hence DOWN crossing expected for exit
    CrossState_ONNLR, // Currently ON, interior to positive side, going left to right
    CrossState_ONPLR, // Currently IN, interior to negative side, going left to right
    CrossState_ONNRL, // Currently ON, interior to positive side, going right to left
    CrossState_ONPRL, // Currently IN, interior to negative side, going right to left
    CrossState_ERROR,   // Impossible state
    CrossState_DIMENSION
    } CrossState;

typedef enum
    {
    CrossDir_Any,   // no implied direction
    CrossDir_LeftToRight,  // usual left to right
    CrossDir_RightToLeft,  // usual right to left
    CrossDir_Error      // impossible
    } CrossDir;
typedef enum
    {
    CC_Error,           // impossible case
    CC_None,            // no connection
    CC_Start,           // start an edge
    CC_End,             // simple end edge
    CC_EndN,            // end edge; next edge connects to new node
    CC_EndX             // end edge; next edge connects to existing node
    } CrossConnect;

typedef struct
    {
    CrossState nextState;       // The next CrossState
    CrossDir orientation;       // The traverse direction implied by accepting this transition --
                                //    0 = no change, +-1 = positive or negative,
    CrossConnect linkType;      // Type of connection made here
    int leftIsAbove;            // True if left is side of added edge is 'above' the section.
    int counter;                // Number of times this case has been considered.
    } MTG_Transition;


static MTG_Transition s_nextState[CrossState_DIMENSION][CROSS_TYPES] =
    {

    // If we are currently OUT (CrossState_OUT) the next state after transition is:
        {
            { CrossState_OUT,   CrossDir_Any,           CC_None,        0},
            { CrossState_ONNRL, CrossDir_RightToLeft,   CC_None,        0},
            { CrossState_INRL,  CrossDir_RightToLeft,   CC_Start,       0},
            { CrossState_ONNLR, CrossDir_LeftToRight,   CC_None,        0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ONPRL, CrossDir_RightToLeft,   CC_None,        0},
            { CrossState_INLR,  CrossDir_LeftToRight,   CC_Start,       0},
            { CrossState_ONPLR, CrossDir_LeftToRight,   CC_None,        0},
            { CrossState_OUT,   CrossDir_Any,           CC_None,        0}
        },

    // If we are currently IN, moving left to right
        {
            { CrossState_INLR,  CrossDir_LeftToRight,   CC_EndX,        0},
            { CrossState_ONPLR, CrossDir_LeftToRight,   CC_End,         0},
            { CrossState_OUT,   CrossDir_LeftToRight,   CC_End,         0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ONNLR, CrossDir_LeftToRight,   CC_End,         0},
            { CrossState_ERROR, CrossDir_Error,         CC_None,        0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_INLR,  CrossDir_LeftToRight,   CC_EndN,        0}
        },

    // If we are currently IN, moving right to left
        {
            { CrossState_INRL,  CrossDir_RightToLeft,   CC_EndN,        0},
            { CrossState_ERROR, CrossDir_Error,         CC_None,        0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ONPRL, CrossDir_RightToLeft,   CC_End,         0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_OUT,   CrossDir_RightToLeft,   CC_End,         0},
            { CrossState_ONNRL, CrossDir_RightToLeft,   CC_End,         0},
            { CrossState_INRL,  CrossDir_RightToLeft,   CC_EndX,        0}
        },
    // If we are currently ON with interior below and going left to right: (ONNLR)
        {
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_OUT,   CrossDir_Error,         CC_None,        0},
            { CrossState_ONNLR, CrossDir_LeftToRight,   CC_None,        0},
            { CrossState_INLR,  CrossDir_LeftToRight,   CC_None,        0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
        },
    // If we are currently ON with interior above and going left to right: (ONPLR)
        {
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_INLR,  CrossDir_LeftToRight,   CC_None,        0},
            { CrossState_ONPLR, CrossDir_LeftToRight,   CC_None,        0},
            { CrossState_OUT,   CrossDir_Error,         CC_None,        0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
        },
    // If we are currently ON with interior below and going right to left: (ONNRL)
        {
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_OUT,   CrossDir_Error,         CC_None,        0},
            { CrossState_ONNRL, CrossDir_RightToLeft,   CC_None,        0},
            { CrossState_INRL,  CrossDir_RightToLeft,   CC_None,        0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
        },
    // If we are currently ON with interior above and going right to left: (ONPRL)
        {
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_INRL,  CrossDir_RightToLeft,   CC_None,        0},
            { CrossState_ONPRL, CrossDir_RightToLeft,   CC_None,        0},
            { CrossState_OUT,   CrossDir_Error,         CC_None,        0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
        },
    // Error state
        {
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
            { CrossState_ERROR, CrossDir_Error,         CC_Error,       0},
        }

        };

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGFacets_checkCrossingSequence                     |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static bool    jmdlMTGFacets_checkCrossingSequence
(
MTG_Crossing    *pCrossing,
int             numCrossing
)
    {
    CrossState currState = CrossState_OUT;
    int type;
    int i;
    for (i = 0; i < numCrossing && currState != CrossState_ERROR; i++)
        {
        type = pCrossing[i].type;
        if (type < 0  ||  type >= CROSS_TYPES || currState < 0 || currState >= CrossState_DIMENSION)
            {
            // This should positively never happen:
            currState = CrossState_ERROR;
            }
        else
            {
            currState = s_nextState[currState][type].nextState;
            }
        }
    return currState != CrossState_ERROR;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGClip_joinCrossings                               |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void    jmdlMTGClip_joinCrossings
(
EmbeddedIntArray      *pEdgeClassification,   // <=> optional array of edge classifications
MTG_Crossing    *pCrossing,
int             numCrossing,
MTGGraph *      pGraph,
EmbeddedIntArray      *pNodesOnPlane,
OmdlMTG_MergeHandler *pHandler
)
    {
    CrossState currState = CrossState_OUT;
    CrossState nextState;
    MTGNodeId baseNodeId = MTG_NULL_NODEID;
    MTGNodeId currNodeId;
    MTGNodeId tailNodeId, headNodeId;
    int type;

    int i;
    for (i = 0; i < numCrossing && currState != CrossState_ERROR; i++)
        {
        type = pCrossing[i].type;
        currNodeId = pCrossing[i].nodeId;
        if (type < 0  ||  type >= CROSS_TYPES || currState < 0 || currState >= CrossState_DIMENSION)
            {
            // This should positively never happen:
            currState = CrossState_ERROR;
            }
        else
            {
            nextState = s_nextState[currState][type].nextState;
            int linkType = s_nextState[currState][type].linkType;
            CrossDir orientation = s_nextState[currState][type].orientation;
            switch (linkType)
                {
                case CC_End:
                case CC_EndX:
                case CC_EndN:
                    jmdlMTGGraph_join  (pGraph,
                            &tailNodeId, &headNodeId,
                            baseNodeId, currNodeId,
                            MTG_NULL_MASK, MTG_NULL_MASK);

                    if (pNodesOnPlane)
                        {
                        jmdlEmbeddedIntArray_addInt (pNodesOnPlane, tailNodeId);
                        jmdlEmbeddedIntArray_addInt (pNodesOnPlane, headNodeId);
                        }

                    if (pEdgeClassification)
                        {
                        if (orientation == CrossDir_LeftToRight)
                            {
                            jmdlVArrayInt_set (pEdgeClassification, MTG_SectionEdge_OnAbove, tailNodeId);
                            jmdlVArrayInt_set (pEdgeClassification, MTG_SectionEdge_OnBelow, headNodeId);
                            }
                        else if (orientation == CrossDir_RightToLeft)
                            {
                            jmdlVArrayInt_set (pEdgeClassification, MTG_SectionEdge_OnAbove, headNodeId);
                            jmdlVArrayInt_set (pEdgeClassification, MTG_SectionEdge_OnBelow, tailNodeId);
                            }
                        }

                    if (pHandler)
                        pHandler->announceJoin (baseNodeId, currNodeId, tailNodeId, headNodeId, true);
                    if (type == CC_End)
                        {
                        baseNodeId = MTG_NULL_NODEID;
                        }
                    else if (type == CC_EndX)
                        {
                        baseNodeId = currNodeId;
                        }
                    else if (type == CC_EndN)
                        {
                        baseNodeId = headNodeId;
                        }
                    break;
                case CC_Start:
                    baseNodeId = pCrossing[i].nodeId;
                    break;
                case CC_None:
                    break;
                default:
                    nextState = CrossState_ERROR;
                }
            currState = nextState;
            }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGClip_compareCrossings                            |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     jmdlMTGClip_crossingCode
(
double  z0,     // => prededessor altittude
double  z1      // => successor altitude
)
    {
    int i0 = jmdlMTGClip_sign (z0);
    int i1 = jmdlMTGClip_sign (z1);
    return (1 + i0) * 3 + 1 + i1;

    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGClip_compareCrossings
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     jmdlMTGClip_compareCrossings
(
const MTG_Crossing *p0,       // first crossing
const MTG_Crossing *p1  // second crossing
)
    {
    if (p0->sortCoordinate < p1->sortCoordinate)
        return -1;
    if (p0->sortCoordinate > p1->sortCoordinate)
        return 1;
    return 0;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGClip_fillSelectedAltitudes                               |
|                                                                       |
| author        EarlinLutz                              06/97           |
|                                                                       |
| Set the z component in the altitude array.                            |
+----------------------------------------------------------------------*/
static void jmdlMTGClip_fillSelectedAltitudes
(
EmbeddedDPoint3dArray     *pAltitudeArray,          // <= z component is set to altitude
EmbeddedDPoint3dArray     *pVertexArray,            // => vertex coordinates
const DPoint4d      *pHPlane,               // => homogeneous plane
const DRange3d      *pRange,                // => range
RangePlaneMask      mask                    // => select among various altitude definitions.
                                            // ASSUMED TO BE A SINGLE BIT.  FIRST
                                            // NONZERO BIT IN WHATEVER ORDER THIS FUNCTION
                                            // CHOOSES TO SEARCH WINS.
)
    {
    int i;
    int numVertex = jmdlEmbeddedDPoint3dArray_getCount (pVertexArray);
    jmdlEmbeddedDPoint3dArray_empty (pAltitudeArray);

    DPoint3d *pAltitude0 = jmdlVArrayDPoint3d_getBlock (pAltitudeArray, numVertex);
    DPoint3d *pVertex0 = jmdlEmbeddedDPoint3dArray_getPtr (pVertexArray, 0);
    DPoint3d *pCurrPoint;
    double a;

    pCurrPoint = pVertex0;
    if (mask & RangePlane_DPoint4d)
        {
        for (i = 0; i < numVertex; i++, pCurrPoint++)
            {
            pCurrPoint = pVertex0 + i;
            pAltitude0[i].z = pHPlane->w
                    + pHPlane->x * pCurrPoint->x
                    + pHPlane->y * pCurrPoint->y
                    + pHPlane->z * pCurrPoint->z;
            }
        }
    else if (mask & RangePlane_XMax)
            {
            a = pRange->high.x;
            for (i = 0; i < numVertex; i++, pCurrPoint++)
                {
                pAltitude0[i].z = pCurrPoint->x - a;
                }
            }
    else if (mask & RangePlane_XMin)
            {
            a = pRange->low.x;
            for (i = 0; i < numVertex; i++, pCurrPoint++)
                {
                pAltitude0[i].z = a - pCurrPoint->x;
                }
            }
    else if (mask & RangePlane_YMax)
            {
            a = pRange->high.y;
            for (i = 0; i < numVertex; i++, pCurrPoint++)
                {
                pAltitude0[i].z = pCurrPoint->y - a;
                }
            }
    else if (mask & RangePlane_YMin)
            {
            a = pRange->low.y;
            for (i = 0; i < numVertex; i++, pCurrPoint++)
                {
                pAltitude0[i].z = a - pCurrPoint->y;
                }
            }
    else if (mask & RangePlane_ZMax)
            {
            a = pRange->high.z;
            for (i = 0; i < numVertex; i++, pCurrPoint++)
                {
                pAltitude0[i].z = pCurrPoint->z - a;
                }
            }
    else if (mask & RangePlane_ZMin)
            {
            a = pRange->low.z;
            for (i = 0; i < numVertex; i++, pCurrPoint++)
                {
                pAltitude0[i].z = a - pCurrPoint->z;
                }
            }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGClip_checkZeroAltitudes                          |
|                                                                       |
| author        EarlinLutz                              06/97           |
|                                                                       |
| Count positive, negative, and zero altitudes (z components) using     |
| zero tolerance.                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGClip_checkZeroAltitudes
(
EmbeddedDPoint3dArray     *pAltitudeArray,          // <=> z component is altitude.  Changed to zero if toleranced
            int     *pNumZero,              // <= number of zeros (within tolerance)
            int     *pNumPositive,          // <= number of positives
            int     *pNumNegative,          // <= number of negatives
double              tol                     // => tolerance for on-plane tests
)
    {
    DPoint3d *pAltitude0 = jmdlEmbeddedDPoint3dArray_getPtr (pAltitudeArray, 0);
    int numVertex = jmdlEmbeddedDPoint3dArray_getCount (pAltitudeArray);
    double z;
    double zeroPos = fabs(tol);
    double zeroNeg = - zeroPos;

    int i, nPos, nNeg, nZero;

    nPos = nNeg = nZero = 0;
    for (i = 0; i < numVertex; i++)
        {
        z = pAltitude0[i].z;
        if (z < zeroNeg)
            {
            nNeg++;
            }
        else if (z > zeroPos)
            {
            nPos++;
            }
        else
            {
            nZero++;
            pAltitude0[i].z = 0.0;
            }
        }
*pNumZero       = nZero;
    *pNumPositive   = nPos;
    *pNumNegative   = nNeg;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGSection_preLabelNodes                            |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGSection_preLabelNodes
(
EmbeddedIntArray            *pClassification,       // <= array filled with per-node classification as per altitudes
EmbeddedIntArray            *pOnSectionNodes,       // <= array filled with node ids that are ON the section
EmbeddedIntArray            *pOffSectionNodes,      // <= array filled with node ids that are NOT on the section
MTGGraph *          pGraph,                 // => graph to search
MTGMask     onMask,                 // => mask to apply to all nodes that are ON the section
EmbeddedDPoint3dArray     *pAltitudeArray,          // => altitudes
int                 offset,                 // => label offset for indexing from nodes to altitudes
OmdlMTG_MergeHandler    *pHandler           // => message receiver
)
    {
    int currVertexId;
    int numVertex = jmdlEmbeddedDPoint3dArray_getCount (pAltitudeArray);
    DPoint3d *pAltitude0 = jmdlEmbeddedDPoint3dArray_getPtr (pAltitudeArray, 0);
    double z0;
    MTG_SectionEdgeClassification classification = MTG_SectionEdge_Unclassified;

    int numNode = jmdlMTGGraph_getNodeIdCount (pGraph);
    jmdlVArrayInt_setConstant (pClassification, MTG_SectionEdge_Unclassified, numNode);

    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        if (   jmdlMTGGraph_getLabel (pGraph, &currVertexId, currNodeId, offset)
            && 0 <= currVertexId
            && currVertexId < numVertex)
            {

            z0 = pAltitude0[currVertexId].z;
            if (z0 > 0.0)
                {
                classification = MTG_SectionEdge_Above;
                jmdlEmbeddedIntArray_addInt (pOffSectionNodes, currNodeId);
                }
            else if (z0 == 0.0)  // It's ok to test for exact zero -- tol was applied before.
                {
                classification = MTG_SectionEdge_On;
                jmdlMTGGraph_setMask (pGraph, currNodeId, onMask);
                jmdlEmbeddedIntArray_addInt (pOnSectionNodes, currNodeId);
                }
            else if (z0 < 0.0)
                {
                classification = MTG_SectionEdge_Below;
                jmdlEmbeddedIntArray_addInt (pOffSectionNodes, currNodeId);
                }
            jmdlVArrayInt_set (pClassification, classification, currNodeId);
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGSection_labelAndSplitEdges                       |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
| Classify edge neighborhoods.  Must be preceded by call to             |
| jmdlMTGSection_preLabelNodes, i.e. every node is labeled              |
| MTG_SectionEdge_Above, MTG_SectionEdge_Below, or                      |
| MTG_SectionEdge_VertexOn.                                             |
+----------------------------------------------------------------------*/
static void jmdlMTGSection_labelAndSplitEdges
(
EmbeddedIntArray            *pEdgeClassification,   // <= array filled with per-node classification as per altitudes
EmbeddedIntArray            *pOnSectionNodes,       // <=> array with ids of nodes that are on the section.  This function
                                            //     will ADD nodes that it creates to this array.
EmbeddedIntArray            *pCandidateNodes,       // => array with ids of nodes that are candidates to be split
MTGGraph *          pGraph,                 // => graph to search
MTGMask     onMask,                 // => mask to apply to all ON nodes that subsequently need to
                                            //      be connected by section edges
EmbeddedDPoint3dArray     *pVertexArray,            // => vertex xyz
EmbeddedDPoint3dArray     *pAltitudeArray,          // => altitudes
int                 offset,                 // => label offset for indexing from nodes to altitudes
OmdlMTG_MergeHandler    *pHandler           // => message receiver
)
    {
    MTG_SectionEdgeClassification c0, c1, c2, cNew;
    DPoint3d xyz0, xyz1, xyz2;
    DPoint3d xyzH0, xyzH1, xyzH2;
    double   h0,    h2;
    double fraction;
    int vert0Index, vert1Index, vert2Index;
    MTGNodeId node0Id, node1Id, node2Id;
    MTGNodeId leftNodeId, rightNodeId;

    int i;

    for (i = 0; jmdlEmbeddedIntArray_getInt (pCandidateNodes, &node0Id, i); i++)
        {
        if (   jmdlEmbeddedIntArray_getInt (pEdgeClassification, (int*)&c0, node0Id)
            && jmdlEmbeddedIntArray_getInt (pEdgeClassification, (int*)&c1, (node1Id = jmdlMTGGraph_getFSucc (pGraph, node0Id)))
            && c0 != c1)
            {
            // ASSUMPTION: c0 is MTG_SectionEdge_Above or MTG_SectionEdge_Below
            if (c1 == MTG_SectionEdge_On)
                {
                cNew = MTG_SectionEdge_Below == c0
                        ? MTG_SectionEdge_OnBelow : MTG_SectionEdge_OnAbove;

                for (;    c1 == MTG_SectionEdge_On
                        && jmdlEmbeddedIntArray_getInt (pEdgeClassification, (int*)&c2, (node2Id = jmdlMTGGraph_getFSucc (pGraph, node1Id)));
                    c1 = c2, node1Id = node2Id
                    )
                    {
                    if (c2 == MTG_SectionEdge_On)
                        {
                        // The edge is ON the section. c0 tells us which halfspace contains the edge's
                        // neighborhood
                        jmdlVArrayInt_set (pEdgeClassification, cNew, node1Id);
                        }
                    else if (c2 == c0)
                        {
                        // The vertex itself is ON, but the neighborhood is all on the same side as node2.
                        jmdlVArrayInt_set (pEdgeClassification, c2, node1Id);
                        }
                    }
                }
            else if (   c0 == MTG_SectionEdge_Below
                     && c1 == MTG_SectionEdge_Above
                     && pHandler->isEdgeSplitable ( node0Id)
                    )
                {
                // This is a crossing from below to above.  Insert a new vertex at the crossing point.
                //  Label it above (because its EDGE is above everywhere except the new vertex)
                // (Note that we never to the opposite case -- above to below.  This is assumed to
                // get handled at the paired node that is below)

                // Collect node and vertex ids:
                node2Id = jmdlMTGGraph_getFSucc (pGraph, node0Id);
                c2 = c1;
                jmdlMTGGraph_getLabel (pGraph, &vert0Index, node0Id, offset);
                jmdlMTGGraph_getLabel (pGraph, &vert2Index, node2Id, offset);

                // and the associated xyz and altitude:
                jmdlVArrayDPoint3d_getDPoint3d (pVertexArray, &xyz0, vert0Index);
                jmdlVArrayDPoint3d_getDPoint3d (pVertexArray, &xyz2, vert2Index);
                jmdlVArrayDPoint3d_getDPoint3d (pAltitudeArray, &xyzH0, vert0Index);
                jmdlVArrayDPoint3d_getDPoint3d (pAltitudeArray, &xyzH2, vert2Index);
                h0 = xyzH0.z;
                h2 = xyzH2.z;

                // interpolate for the crossing:
                fraction = -h0 / (h2 - h0);
                bsiDPoint3d_interpolate (&xyz1, &xyz0, fraction, &xyz2);
                bsiDPoint3d_zero (&xyzH1);

                // Add the crossing coordinates to the vertex and altitude arrays:
                vert1Index = jmdlEmbeddedDPoint3dArray_getCount (pVertexArray);
                pVertexArray->push_back (xyz1);
                pAltitudeArray->push_back (xyzH1);

                // Split the edge and set the new node's vertex ids and masks:
                jmdlMTGGraph_splitEdge (pGraph, &leftNodeId, &rightNodeId, node0Id);
                jmdlMTGGraph_setLabel (pGraph, leftNodeId, offset, vert1Index);
                jmdlMTGGraph_setLabel (pGraph, rightNodeId, offset, vert1Index);
                jmdlMTGGraph_setMask (pGraph, rightNodeId, onMask);
                jmdlMTGGraph_setMask (pGraph, leftNodeId, onMask);
                pHandler->announceEdgeSplit (node0Id, leftNodeId, rightNodeId);

                // Classify the new edge
                jmdlVArrayInt_set (pEdgeClassification, c2, leftNodeId);
                jmdlVArrayInt_set (pEdgeClassification, c0, rightNodeId);

                // and note the additional ON nodes:
                jmdlEmbeddedIntArray_addInt (pOnSectionNodes, leftNodeId);
                jmdlEmbeddedIntArray_addInt (pOnSectionNodes, rightNodeId);
                }
            }
        }

    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGSection_collectAndClearMaskedNodesAroundFace     |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
| Scan a face for masked nodes.  Add them to an array and clear the     |
| mask.   The array is NOT cleared first.                               |
+----------------------------------------------------------------------*/
static void jmdlMTGSection_collectAndClearMaskedNodesAroundFace
(
EmbeddedIntArray      *pArray,  // <=> array to receive nodes.
int             *pCount,        // <= number of nodes added to array
int             *pFaceCount,    // <= total number of nodes on face
MTGGraph *      pGraph,         // => containing graph
MTGNodeId      seedNodeId,     // => start node of face
MTGMask mask            // => mask to seek
)
    {
    // Accumulate all marked nodes on this face:
    int count = 0;
    int faceCount = 0;
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, seedNodeId)
        {
        faceCount++;
        if (jmdlMTGGraph_getMask (pGraph, currNodeId, mask))
            {
            count++;
            jmdlMTGGraph_clearMask (pGraph, currNodeId, mask);
            jmdlEmbeddedIntArray_addInt (pArray, currNodeId);
            }
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, seedNodeId)
*pCount = count;
    *pFaceCount = faceCount;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGSection_splitFaces                               |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
| Insert edges in all faces that have crossings.                        |
+----------------------------------------------------------------------*/
static void jmdlMTGSection_splitFaces
(
EmbeddedIntArray            *pEdgeClassification,   // <= array filled with per-node classification as per altitudes
EmbeddedIntArray            *pOnSectionNodes,       // => array with ids of nodes that are on the section.  This function
                                            //     will ADD nodes that it creates to this array.
EmbeddedIntArray            *pOffSectionNodes,      // => array with ids of nodes that are NOT on the section.
MTGGraph *          pGraph,                 // => graph to search
MTGMask     onMask,                 // => mask to apply to all ON nodes that subsequently need to
                                            //      be connected by section edges
EmbeddedDPoint3dArray     *pVertexArray,            // => vertex xyz
EmbeddedDPoint3dArray     *pAltitudeArray,          // => altitudes
int                 offset,                 // => label offset for indexing from nodes to altitudes
OmdlMTG_MergeHandler    *pHandler           // => message receiver
)
    {
    int i;
    MTGNodeId seedNodeId, startNodeId, endNodeId;  // Start and end of added edge.
    MTGNodeId newStartNodeId, newEndNodeId;
    int count, faceCount;
    MTG_SectionEdgeClassification c0, c1;
    const DPoint3d *pAltitude0 = jmdlEmbeddedDPoint3dArray_getPtr (pAltitudeArray, 0);

    EmbeddedIntArray *pNodesOnFace = jmdlEmbeddedIntArray_grab ();

    for (i = 0; jmdlEmbeddedIntArray_getInt (pOnSectionNodes, &seedNodeId, i); i++)
        {
        MTGMask masks = jmdlMTGGraph_getMask (pGraph, seedNodeId, onMask | MTG_EXTERIOR_MASK);
        if (masks & MTG_EXTERIOR_MASK)
            {
            }
        else if (masks & onMask)
            {
            // onMask is set .. this node is ON the section, and the face was not
            // visited previously.  Collect up all the other ON nodes.  While doing that, clear
            // their ON mask so we don't revisit them.
            jmdlEmbeddedIntArray_empty (pNodesOnFace);
            jmdlMTGSection_collectAndClearMaskedNodesAroundFace (pNodesOnFace, &count, &faceCount, pGraph, seedNodeId, onMask);

            if (faceCount == count)
                {
                // The face is all ON
                }
            if (count == 2)
                {
                jmdlVArrayInt_getInt (pNodesOnFace, &startNodeId, 0);
                jmdlVArrayInt_getInt (pNodesOnFace, &endNodeId, 1);
                jmdlVArrayInt_getInt (pEdgeClassification, (int*)&c0, startNodeId);
                jmdlVArrayInt_getInt (pEdgeClassification, (int*)&c1, endNodeId);

                // If the two nodes are already at the end of an ON edge,
                // we don't need to connect.
                if (!(c0 & MTG_SECTIONEDGE_ONBIT) && !(c1 & MTG_SECTIONEDGE_ONBIT))
                    {
                    jmdlMTGGraph_join  (pGraph,
                            &newStartNodeId, &newEndNodeId,
                            startNodeId, endNodeId,
                            MTG_NULL_MASK, MTG_NULL_MASK);
                    pHandler->announceJoin (startNodeId, endNodeId, newStartNodeId, newEndNodeId, true);

                    jmdlVArrayInt_set (pEdgeClassification, (c0 | MTG_SECTIONEDGE_ONBIT), newEndNodeId);
                    jmdlVArrayInt_set (pEdgeClassification, (c1 | MTG_SECTIONEDGE_ONBIT), newStartNodeId);
                    }
                }
            else if (count > 2)
                {

#undef CACHE_ARRAY_SIZE
#define CACHE_ARRAY_SIZE 1024
                MTG_Crossing localCrossingArray[CACHE_ARRAY_SIZE];
                MTG_Crossing *pCrossing = count <= CACHE_ARRAY_SIZE ? localCrossingArray :
                                    (MTG_Crossing *)malloc (count * sizeof (MTG_Crossing));

                // Get the node id's and coordinates into an array ...
                int i;
                MTGNodeId nodeId, predId, succId;
                int vertexId, predVertexId, succVertexId;
                DPoint3d currVertexXYZ;
                DRange3d range;
                bool        boolstat = true;
                bsiDRange3d_init (&range);
                for (i = 0; i < count && boolstat; i++)
                    {
                    boolstat = false;
                    if (   jmdlEmbeddedIntArray_getInt (pNodesOnFace, &nodeId, i))
                        {
                        predId = jmdlMTGGraph_getFPred (pGraph, nodeId);
                        succId = jmdlMTGGraph_getFSucc (pGraph, nodeId);
                        if (   jmdlMTGGraph_getLabel (pGraph, &predVertexId, predId, offset)
                            && jmdlMTGGraph_getLabel (pGraph, &succVertexId, succId, offset)
                            && jmdlMTGGraph_getLabel (pGraph, &vertexId, nodeId, offset)
                            && jmdlEmbeddedDPoint3dArray_getDPoint3d (pVertexArray, &currVertexXYZ, vertexId)
                            )
                            {
                            double zPred = pAltitude0[predVertexId].z;
                            double zSucc = pAltitude0[succVertexId].z;
                            pCrossing[i].nodeId = nodeId;
                            pCrossing[i].point  = currVertexXYZ;
                            pCrossing[i].type   = jmdlMTGClip_crossingCode (zPred, zSucc);
                            bsiDRange3d_extendByDPoint3d (&range, &currVertexXYZ);
                            boolstat = true;
                            }
                        }
                    }

                if (boolstat)
                    {
                    // Choose a sort direction for the points
                    int index = 0;
                    DPoint3d diag;
                    bsiDPoint3d_subtractDPoint3dDPoint3d (&diag, &range.high, &range.low);
                    double maxDiag = fabs (diag.x);
                    if (fabs (diag.y) > maxDiag)
                        {
                        index = 1;
                        maxDiag = fabs (diag.y);
                        }
                    if (fabs (diag.z) > maxDiag)
                        {
                        index = 2;
                        maxDiag = diag.z;
                        }

                    for (i = 0; i < count; i++)
                        {
                        pCrossing[i].sortCoordinate =
                                bsiDPoint3d_getComponent (&pCrossing[i].point, index);
                        }
                    qsort (pCrossing, count, sizeof (MTG_Crossing),
                                        (int (*)(const void *, const void *))jmdlMTGClip_compareCrossings);

                    if (jmdlMTGFacets_checkCrossingSequence (pCrossing, count))
                        {
                        jmdlMTGClip_joinCrossings (pEdgeClassification, pCrossing, count,
                                        pGraph, pOnSectionNodes, pHandler);
                        }
                    }


                if (pCrossing != localCrossingArray)
                    free (pCrossing);
                }
            }
        }
    jmdlEmbeddedIntArray_drop (pNodesOnFace);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGClip_section                                     |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void    jmdlMTGClip_section
(
        MTGFacets *     pFacetHeader,   // <=> facets to clip
        bool            *pClipped,      // <= set true if clipped
const   DPoint4d        *pHPlane,       // => plane equation
const   DRange3d        *pRange,        // => range
        RangePlaneMask  planeSelector,  // => selects among plane or range faces
        double          zTol,           // => tolernce for plane hits
OmdlMTG_MergeHandler    *pHandler
)
    {
    EmbeddedDPoint3dArray *pAltitudeArray     = jmdlEmbeddedDPoint3dArray_grab ();

    EmbeddedIntArray      *pEdgeClassification = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray      *pOnSectionNodes = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray      *pOffSectionNodes = jmdlEmbeddedIntArray_grab ();

    EmbeddedDPoint3dArray *pVertexArray   = (&pFacetHeader->vertexArrayHdr);
    int numVertex = jmdlEmbeddedDPoint3dArray_getCount ((&pFacetHeader->vertexArrayHdr));

    int nPos, nNeg, nZero;
    MTGGraph * pGraph = (&pFacetHeader->graphHdr);

    MTGMask    connectMask = jmdlMTGGraph_grabMask (pGraph);

    if (numVertex == 0)
        {
        return;
        }
    else
        {
        jmdlMTGClip_fillSelectedAltitudes (
                        pAltitudeArray,
                        pVertexArray,
                        pHPlane,
                        pRange,
                        planeSelector
                        );

        jmdlMTGClip_checkZeroAltitudes (
                        pAltitudeArray,
                        &nZero,
                        &nPos,
                        &nNeg,
                        zTol);

        if (nNeg == 0 && nZero == 0)
            {
            pHandler->announceAllOut ();
            }
        else if (nPos == 0 && nZero == 0)
            {
            pHandler->announceAllIn ();
            }
        else if (nPos == 0 && nNeg == 0)
            {
            pHandler->announceAllOn ();
            }
        else
            {
            jmdlMTGGraph_clearMaskInSet (pGraph, connectMask);
            jmdlMTGSection_preLabelNodes
                            (
                            pEdgeClassification,
                            pOnSectionNodes,
                            pOffSectionNodes,
                            pGraph,
                            connectMask,
                            pAltitudeArray,
                            pFacetHeader->vertexLabelOffset,
                            pHandler
                            );
            jmdlMTGSection_labelAndSplitEdges
                            (
                            pEdgeClassification,
                            pOnSectionNodes,
                            pOffSectionNodes,
                            pGraph,
                            connectMask,
                            pVertexArray,
                            pAltitudeArray,
                            pFacetHeader->vertexLabelOffset,
                            pHandler
                            );

            jmdlMTGSection_splitFaces
                            (
                            pEdgeClassification,
                            pOnSectionNodes,
                            pOffSectionNodes,
                            pGraph,
                            connectMask,
                            pVertexArray,
                            pAltitudeArray,
                            pFacetHeader->vertexLabelOffset,
                            pHandler
                            );

            pHandler->announceEdgeClassification (pEdgeClassification);

            }
            // Notify the caller of all the categories:
        }

    // Return all that was borrowed ....
    jmdlMTGGraph_dropMask (pGraph, connectMask);
    jmdlEmbeddedDPoint3dArray_drop (pAltitudeArray);
    jmdlEmbeddedIntArray_drop ( pEdgeClassification);
    jmdlEmbeddedIntArray_drop ( pOnSectionNodes);
    jmdlEmbeddedIntArray_drop ( pOffSectionNodes);
    }



/**

* @param pFacetHeader    <=> facets to clip
* @param pClipped <= set true if clipped
* @param pRange => range cube defining planes
* @param planeSelector => bit map of active planes
* @param clipType => one of:
*                   MTG_ClipOp_KeepIn -- usual inside clip.
*                   MTG_ClipOp_KeepOut -- clip away the convex region.
*                   MTG_ClipOp_KeepOn -- union of section cuts on all planes.
* @param cutMask => mask to apply to edges created by the clip
* @see MTGFacets.IClipType
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_clipToRange
(
        MTGFacets      *pFacetHeader,
        bool            *pClipped,      // <= set true if clipped
const   DRange3d        *pRange,
        RangePlaneMask planeSelector,
        MTGClipOp       clipType,
        MTGMask cutMask
)
    {
    double tol = 1.0e-10;   // Should be fraction of actual range...
    int i;

    // Dummm-de-dum-dum. Sort of a bool    here.

    OmdlMTG_MergeHandler *pHandler = new OmdlMTG_MergeHandler (pFacetHeader, cutMask);

    pHandler->pushClipType (clipType);
    pHandler->startConvexSet (NULL, 0);

    for (i = 0; i < RangePlaneIndexedMaskCount && !pHandler->isShortCircuited (); i++)
        {
        RangePlaneMask mask = RangePlaneIndexedMask(i);
        pHandler->startPlaneOfConvexSet (NULL, 0, i);
        jmdlMTGClip_section (
                        pFacetHeader,
                        pClipped,
                        NULL,
                        pRange,
                        mask,
                        tol,
                        pHandler
                        );
        pHandler->endPlaneOfConvexSet (NULL, 0, i);
        }

    pHandler->endConvexSet (NULL, 0);
    pHandler->popClipType ();

    pHandler->removeExtraNodes ();

    delete pHandler;

    }


/**

* @param pFacetHeader    <=> facets to clip
* @param pClipped <= set true if clipped
* @param pHPlane => plane equations
* @param numPlane => number of planes
* @param clipType => one of:
*                   MTG_ClipOp_KeepIn -- usual inside clip.
*                   MTG_ClipOp_KeepOut -- clip away the convex region.
*                   MTG_ClipOp_KeepOn -- union of section cuts on all planes.
* @param cutMask => mask to apply to edges created by the clip
* @see MTGFacets.IClipType
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_clipToPlanes
(
MTGFacets *     pFacetHeader,
        bool            *pClipped,      // <= set true if clipped
const   DPoint4d        *pHPlane,
        int             numPlane,
        MTGClipOp       clipType,
        MTGMask cutMask
)
    {
    double tol = 1.0e-10;   // Should be fraction of actual range...
    int i;
    static int maxPlane = -1;

    if (maxPlane >= 0 && numPlane > maxPlane)
        numPlane = maxPlane;

    // Dummm-de-dum-dum. Sort of a bool    here.

    OmdlMTG_MergeHandler *pHandler = new OmdlMTG_MergeHandler (pFacetHeader, cutMask);

    pHandler->pushClipType (clipType);
    pHandler->startConvexSet (pHPlane, numPlane);

    for (i = 0; i < numPlane && !pHandler->isShortCircuited (); i++)
        {
        pHandler->startPlaneOfConvexSet (&pHPlane[i], numPlane, i);
        jmdlMTGClip_section (
                        pFacetHeader,
                        pClipped,
                        &pHPlane[i],
                        NULL,
                        RangePlane_DPoint4d,
                        tol,
                        pHandler
                        );
        pHandler->endPlaneOfConvexSet (&pHPlane[i], numPlane, i);
        }

    pHandler->endConvexSet (pHPlane, numPlane);
    pHandler->popClipType ();

    pHandler->removeExtraNodes ();

    delete pHandler;

    }

/**
* @param pFacetHeader           <=> facets to clip
* @param pClipped       <= true if graph was changed
* @param pPointArray    => points in polygon
* @param numPoint       => number of planes
* @param pDirection     => sweep direction
* @param clipType       => one of:
*                       MTG_ClipOp_KeepIn -- usual inside clip.
*                       MTG_ClipOp_KeepOut -- clip away the convex region.
*                       MTG_ClipOp_KeepOn -- union of section cuts on all planes.
* @see MTGFacets.IClipType
* @return SUCCESS if hole punched successfully.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlMTGFacets_punchPolygon
(
        MTGFacets     *pFacetHeader,
        bool          *pClipped,
const   DPoint3d      *pPointArray,
        int            numPoint,
const   DPoint3d      *pDirection,
        MTGClipOp      clipType
)
    {
    bool    boolstat = false;
    double volume = bsiPolygon_tentVolume (pPointArray, numPoint, (DVec3dCP) pDirection);
    DPoint3d normal, edgeVector;
#define MAXPLANE 100
    DPoint4d hPlane[MAXPLANE];
    if (numPoint < MAXPLANE)
        {
        int i,j;
        for (i = 0; i < numPoint; i++)
            {
            j = (i + 1) % numPoint;
            bsiDPoint3d_subtractDPoint3dDPoint3d (&edgeVector, pPointArray + j , pPointArray + i);
            if (volume > 0.0)
                {
                bsiDPoint3d_crossProduct (&normal, &edgeVector, pDirection);
                }
            else
                {
                bsiDPoint3d_crossProduct (&normal, pDirection, &edgeVector);
                }
            bsiDPoint4d_planeFromOriginAndNormal(hPlane + i, pPointArray + i, &normal);
            }
        jmdlMTGFacets_clipToPlanes (pFacetHeader, pClipped, hPlane, numPoint, clipType, MTG_NULL_MASK);
        boolstat = true;
        }
    return boolstat;
    }



/**
* @param pFacetHeader       <=> facets to clip
* @param pHPlane    =>  plane equation
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_sectionByPlane
(
        MTGFacets      *pFacetHeader,
const   DPoint4d        *pHPlane
)
    {
    bool    clipped;
    jmdlMTGFacets_clipToPlanes (pFacetHeader, &clipped, pHPlane, 1, MTG_ClipOp_KeepOn, MTG_NULL_MASK);
    }


typedef enum
    {
    MTG_FaceExcluded,
    MTG_FacePositive,
    MTG_FaceNegative,
    MTG_FaceFlush,
    MTG_FaceAmbiguous
    } MTG_FaceVisibilityClassification;



/**
* Collect edges for which the adjacent facet normals have different
* directions relative to a view vector, and at least one of the two
* normals is not perpendicular to the view normal.   Facet normals
* are determined by taking cross products of 3 consecutive points on
* each side of the edge.
* @param pNodeArray <= array with one node per silhouette edge
* @param pFacetHeader    => facets to examine
* @param pViewVector => view direction vector.
* @param ignoreMask => identifies egdges NOT to test
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_findSilhouetteEdges
(
        EmbeddedIntArray        *pNodeArray,
const   MTGFacets *     pFacetHeader,
const   DPoint3d        *pViewVector,
        MTGMask ignoreMask
)
    {
    const MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    const EmbeddedDPoint3dArray *pVertexArray = (&pFacetHeader->vertexArrayHdr);
    MTGNodeId mateNodeId, leftNodeId, rightNodeId, succNodeId;
    int         currVert, mateVert, leftVert, rightVert;
    int vertexLabelOffset = pFacetHeader->vertexLabelOffset;
    DPoint3d leftNormal, rightNormal;
    double leftDot, rightDot;

    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        succNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId);
        mateNodeId = jmdlMTGGraph_getVSucc (pGraph, succNodeId);

        if (currNodeId < mateNodeId)
            {
            if (   !jmdlMTGGraph_getMask (pGraph, currNodeId, ignoreMask)
                && !jmdlMTGGraph_getMask (pGraph, mateNodeId, ignoreMask)
                )
                {
                leftNodeId  = jmdlMTGGraph_getFSucc (pGraph, succNodeId);
                rightNodeId = jmdlMTGGraph_getFPred (pGraph, mateNodeId);
                if (
                      jmdlMTGGraph_getLabel (pGraph, &currVert, currNodeId, vertexLabelOffset)
                   && jmdlMTGGraph_getLabel (pGraph, &mateVert, mateNodeId, vertexLabelOffset)
                   && jmdlMTGGraph_getLabel (pGraph, &leftVert, leftNodeId, vertexLabelOffset)
                   && jmdlMTGGraph_getLabel (pGraph, &rightVert, rightNodeId, vertexLabelOffset)
                   && SUCCESS == jmdlVArrayDPoint3d_crossProduct3Points (pVertexArray, &leftNormal, mateVert, leftVert, currVert)
                   && SUCCESS == jmdlVArrayDPoint3d_crossProduct3Points (pVertexArray, &rightNormal, mateVert, currVert, rightVert)
                    )
                    {
                    leftDot = bsiDPoint3d_dotProduct (&leftNormal, pViewVector);
                    rightDot = bsiDPoint3d_dotProduct (&rightNormal, pViewVector);
                    if (leftDot * rightDot <= 0.0 && (leftDot != 0.0 || rightDot != 0.0))
                        {
                        jmdlEmbeddedIntArray_addInt (pNodeArray, currNodeId);
                        }
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)
    }



/**
* Any of the node arrays may be NULL.   The same pointer may be
* passed for several different arrays, e.g. the zero and ambiguous
* faces.
* @param pPositive          <= array of node ids whose edges
*                               are on the silhouette and adjacent face
*                               point towards the eye.  May be NULL
* @param pNegative          => array of node ids whose edges
*                               are on the silhouette and adjacent face points
*                               away. May be NULL
* @param pFringe            => array of node ids whose edges are on a fringe
*                               this face postive or negative, adjacent face
*                               ambiguous or excluded. May be NULL
* @param pZero              => array with ONE node id for each face that is
*                               perpendicular to the view direction.
* @param pAmbiguous         => array with ONE node id for each face that has
*                               ambiguous visibility
* @param pFacetHeader       => facets to search
* @param excludeFaceMask    => mask for faces to include.  Assumed to be set
*                               entirely around each face
* @param excludeEdgeMask    => mask for edges to exclude.  Tested edge by edge
* @param pEyePoint          => eyepoint for visibility tests.
* @param inTol              => tolerance for flush-face test.
*                               If zero, machine tolerance is used.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_findSilhouette
(
MTGFacets *             pFacetHeader,
EmbeddedIntArray                *pPositive,
EmbeddedIntArray                *pNegative,
EmbeddedIntArray                *pFringe,
EmbeddedIntArray                *pZero,
EmbeddedIntArray                *pAmbiguous,
MTGMask         excludeFaceMask,
MTGMask         excludeEdgeMask,
DPoint4d                *pEyePoint,
double                  inTol
)
    {
    MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    EmbeddedDPoint3dArray *pVertexArray = (&pFacetHeader->vertexArrayHdr);
    int    vertexIdOffset = pFacetHeader->vertexLabelOffset;
    double testTol = inTol;
    double posTol, negTol;
    static double machineRelTol = 1.0e-12;

    jmdlEmbeddedIntArray_empty (pPositive);
    jmdlEmbeddedIntArray_empty (pNegative);
    jmdlEmbeddedIntArray_empty (pZero);
    jmdlEmbeddedIntArray_empty (pFringe);

    if (jmdlMTGFacets_isEmptyFacetSet (pFacetHeader))
        return;

    if (testTol <= 0.0)
        {
        DRange3d range;
        jmdlVArrayDPoint3d_getRange (pVertexArray, &range);
        testTol = machineRelTol * bsiDRange3d_getLargestCoordinate (&range);
        }

    posTol =  testTol;
    negTol = -testTol;
    EmbeddedIntArray *pFaceSeedArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pFaceIdArray   = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pFaceTestArray  = jmdlEmbeddedIntArray_grab ();
    EmbeddedDPoint3dArray *pFaceLoopArray = jmdlEmbeddedDPoint3dArray_grab ();


    MTGNodeId faceSeedId;
    int i;
    DPoint3d origin, normal;
    double planeError;
    int numPos, numNeg;
    numPos = numNeg = 0;

    // Classify each face's visibility
    jmdlMTGGraph_collectAndNumberFaceLoops (pGraph, pFaceSeedArray, pFaceIdArray);

    for (i = 0; jmdlEmbeddedIntArray_getInt (pFaceSeedArray, &faceSeedId, i); i++)
        {
        MTG_FaceVisibilityClassification visClass = MTG_FaceAmbiguous;

        if (jmdlMTGGraph_getMask (pGraph, faceSeedId, excludeFaceMask))
            {
            visClass = MTG_FaceExcluded;
            }
        else
            {

            if (jmdlMTGFacets_getFaceCoordinates (pGraph,
                        pFaceLoopArray, pVertexArray, faceSeedId, vertexIdOffset)
                && SUCCESS == jmdlVArrayDPoint3d_getPolygonPlane (pFaceLoopArray, &normal, &origin, &planeError)
                )
                {
                double visValue = bsiDPoint4d_eyePlaneTest (pEyePoint, &origin, &normal);
                if (visValue > posTol)
                    {
                    visClass = MTG_FacePositive;
                    numPos++;
                    }
                else if (visValue < negTol)
                    {
                    visClass = MTG_FaceNegative;
                    numNeg++;
                    }
                else
                    visClass = MTG_FaceFlush;
                    {
                    }
                }
            }
        jmdlVArrayInt_set (pFaceTestArray, (int)visClass, i);

        if (visClass == MTG_FaceFlush && pZero)
            jmdlEmbeddedIntArray_addInt (pZero, faceSeedId);

        if (visClass == MTG_FaceAmbiguous && pAmbiguous)
            jmdlEmbeddedIntArray_addInt (pAmbiguous, faceSeedId);

        }

    // Classify each mtg node
    MTGNodeId mateEdgeId;
    int     baseFaceId, mateFaceId;
    MTG_FaceVisibilityClassification baseVis, mateVis;
    MTGARRAY_SET_LOOP (baseEdgeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, baseEdgeId, excludeEdgeMask))
            {

            mateEdgeId = jmdlMTGGraph_getEdgeMate (pGraph, baseEdgeId);

            if (    jmdlEmbeddedIntArray_getInt (pFaceIdArray, &baseFaceId, baseEdgeId)
                &&  jmdlEmbeddedIntArray_getInt (pFaceIdArray, &mateFaceId, mateEdgeId)
                &&  jmdlEmbeddedIntArray_getInt (pFaceTestArray, (int*)&baseVis, baseFaceId)
                &&  jmdlEmbeddedIntArray_getInt (pFaceTestArray, (int*)&mateVis, mateFaceId)
               )
                {
                if (baseVis == mateVis)
                    {
                    // ignore it quick.  This is the most common case in a large mesh
                    }
                else if (baseVis == MTG_FacePositive)
                    {
                    if (mateVis == MTG_FaceNegative || mateVis == MTG_FaceFlush)
                        {
                        if (pPositive)
                            jmdlEmbeddedIntArray_addInt (pPositive, baseEdgeId);
                        }
                    else if (pFringe)
                        {
                        jmdlEmbeddedIntArray_addInt (pFringe, baseEdgeId);
                        }
                    }
                else if (baseVis == MTG_FaceNegative)
                    {
                    if (mateVis == MTG_FacePositive || mateVis == MTG_FaceFlush)
                        {
                        if (pNegative)
                            jmdlEmbeddedIntArray_addInt (pNegative, baseEdgeId);
                        }
                    else if (pFringe)
                        {
                        jmdlEmbeddedIntArray_addInt (pFringe, baseEdgeId);
                        }
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP (edgeBaseId, pGraph)

    jmdlEmbeddedDPoint3dArray_drop (pFaceLoopArray);
    jmdlEmbeddedIntArray_drop (pFaceTestArray);
    jmdlEmbeddedIntArray_drop (pFaceSeedArray);
    jmdlEmbeddedIntArray_drop (pFaceIdArray);
    }




/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::OmdlMTG_PolygonPunchHandler       |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
OmdlMTG_PolygonPunchHandler::OmdlMTG_PolygonPunchHandler
(
MTGFacets * pFacetHeader
) :
    OmdlMTG_MergeHandler (pFacetHeader, MTG_NULL_MASK)
    {
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::OmdlMTG_PolygonPunchHandler       |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
OmdlMTG_PolygonPunchHandler::~OmdlMTG_PolygonPunchHandler
(
)
    {
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::startPolygon                      |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
StatusInt   OmdlMTG_PolygonPunchHandler::boolean_startPolygon
(
const DPoint3d  *pPointArray,
      int numPoint,
const DPoint3d *pDirection
)
    {
    DRange3d polygonRange;
    bsiDRange3d_initFromArray (&polygonRange, pPointArray, numPoint);
    m_unitNormal = *pDirection;
    if (numPoint > 0 && 0.0 < bsiDPoint3d_normalizeInPlace (&m_unitNormal))
        {
        return SUCCESS;
        }
    return ERROR;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::endPolygon                        |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
void OmdlMTG_PolygonPunchHandler::endPolygon
(
const DPoint3d  *pPointArray,
      int numPoint,
const DPoint3d *pDirection
)
    {
    }

#ifdef BEIJING_DGNGRAPHICS_WIP_KAB

/*----------------------------------------------------------------------+
|                                                                       |
| name          OmdlMTG_MergeHandler::startPolygonEdge                  |
|                                                                       |
| author        EarlinLutz                              03/97           |
|                                                                       |
+----------------------------------------------------------------------*/
bool    OmdlMTG_PolygonPunchHandler::boolean_applyPolygonEdge
(
      int i0,                   // The edge index
const DPoint3d  *pPointArray,
      int numPoint,
const DPoint3d *pDirection
)
    {
    int i1 = (i0 + 1) % numPoint;
    DPoint3d skewVector, perpVector, normalVector;
    double dot1, dot2, dot3;
    DRange3d graphRange, faceRange;

    m_startPoint = pPointArray[i0];
    bsiDPoint3d_subtractDPoint3dDPoint3d (&skewVector, pPointArray + i1, pPointArray + i0);

    dot1 = bsiDPoint3d_dotProduct (&skewVector, &m_unitNormal);
    dot2 = bsiDPoint3d_dotProduct (&m_unitNormal, &m_unitNormal);    // Well, sure, it's supposed to be 1.

    bsiDPoint3d_addScaledDPoint3d (&perpVector, &skewVector, &m_unitNormal, -dot1/ dot2);

    dot3 = bsiDPoint3d_dotProduct (&perpVector, &perpVector);
    m_edgeVector = perpVector;
    static double fractionalEpsilon = 1.0e-10;
    double scaledEpsilon = dot3 * fractionalEpsilon;
    m_alphaMin = -scaledEpsilon;
    m_alphaMax = dot3 * (1.0 + fractionalEpsilon);

    bsiDPoint3d_crossProduct (&normalVector, &m_unitNormal, &m_edgeVector);

    // Setup sweep coordinate system:
    // origin = point i of polygon
    // x = sweep direction
    // y = edge direction
    // z = plane normal

    m_sweepTransform.InitFromOriginAndVectors (pPointArray[i0], m_unitNormal, perpVector, normalVector);

    this->applyTransform (&m_sweepTransform);

    this->boolean_getLocalRange (&graphRange);

    jmdlMTGGraph_collectAndNumberFaceLoops ((&m_pFacetHeader->graphHdr), m_pFaceSeedNodeArray, NULL);

    int i;
    MTGNodeId seedNodeId;
    for (i = 0; jmdlEmbeddedIntArray_getInt (m_pFaceSeedNodeArray, &seedNodeId, i); i++)
        {
        this->boolean_getLocalFaceRange (&faceRange, seedNodeId);
        this->splitFaceByLocalZ (seedNodeId);
        }

    return SUCCESS;
    }
#endif

END_BENTLEY_GEOMETRY_NAMESPACE
