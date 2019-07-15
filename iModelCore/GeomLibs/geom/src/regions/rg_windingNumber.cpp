/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "Regions/rg_intern.h"
#include <Mtg/mtgprint.fdf>
#include <stdio.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*-----------------------------------------------------------------*//**
* Combine a local noisy flag with jmdlRG_getNoisy ().
+--------------------------------------------------------------------*/
static     int getNoisy
(
int suggestedNoisy
)
    {
    return suggestedNoisy | jmdlRG_getNoisy ();
    }

struct AreaMarkupContext
    {
    RG_Header *m_pRG;
    AreaSelect m_groupOp;
    BoolSelect m_boolOp;
    MTG_MarkSet *pMarkSet;
    bvector<int> groupStateArray;
    int m_compositeNumberIn;
    int m_compositeCount;
    bool reverseSense;
    MTGMask       activeEdgeMask;

    enum FloodEvent
        {
        FloodEvent_EnterComponent,
        FloodEvent_EnterFace,
        FloodEvent_ExitFace,
        FloodEvent_ExitComponent
        };


    AreaMarkupContext
        (
        RG_Header *_pRG,
        AreaSelect _groupOp,
        BoolSelect _boolOp,
        MTG_MarkSet *_pMarkSet,
        bool _reverseSense,
        MTGMask _activeEdgeMask
        )
        : m_pRG (_pRG),
          m_groupOp (_groupOp),
          m_boolOp (_boolOp),
          pMarkSet (_pMarkSet),
          reverseSense (_reverseSense),
          activeEdgeMask (_activeEdgeMask)
        {
        }
    ~AreaMarkupContext ()
        {
        }
        
    bool IsValidGroupId (size_t g)
        {
        return g < groupStateArray.size ();
        }

    bool GroupInOut (size_t groupId)
        {
        if (IsValidGroupId (groupId))
            {
            int z = groupStateArray[groupId];
            switch (m_groupOp)
                {
                case AreaSelect_Parity:
                    return (z & 0x01) != 0;
                case AreaSelect_CCWPositiveWindingNumber:
                    return z > 0;
                case AreaSelect_CCWNonzeroWindingNumber:
                    return z != 0;
                case AreaSelect_CCWNegativeWindingNumber:
                    return z < 0;
                }
            }
        assert (0);
        return false;
        }

    int GroupInOut01 (size_t groupId)
        {
        return GroupInOut (groupId) ? 1 : 0;
        }

    bool CompositeInOut ()
        {
        switch (m_boolOp)
            {
            case BoolSelect_Union:
                return m_compositeNumberIn > 0;
            case BoolSelect_Parity:
                return (m_compositeNumberIn & 0x01) != 0;
            case BoolSelect_Summed_Parity:
                return (m_compositeCount & 0x01) != 0;
            case BoolSelect_Summed_Positive:
                return m_compositeCount > 0;
            case BoolSelect_Summed_NonZero:
                return m_compositeCount != 0;
            case BoolSelect_Summed_Negative:
                return m_compositeCount < 0;
            }
        // REMARK: BoolSelect_ByStructure should not get this far !!!!
        assert (0);
        return false;
        }

    // return +-1 according as the single group toggled in or out.  0 if no group.
    void UpdateState (size_t groupId, int increment)
        {
        if (IsValidGroupId (groupId))
            {
            int b0 = GroupInOut01 (groupId);
            groupStateArray[groupId] += increment;
            int b1 = GroupInOut01 (groupId);
            m_compositeNumberIn += b1 - b0;
            m_compositeCount += increment;
            }
        }

bool EvaluateState ()
    {
    bool state = CompositeInOut ();
    if (reverseSense)
        state = !state;
    return state;
    }

void ClearCounts ()
    {
    m_compositeNumberIn = 0;
    m_compositeCount = 0;
    }

/*------------------------------------------------------------------*//**
* Callback to handle face entry and exit.
*
* @param groupId => the group
* @param direction => positive for push, negative for pop.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
void RecordGroupStateChange
(
size_t groupId,
int direction
)
    {
    UpdateState (groupId, direction);
    if (jmdlRG_getNoisy () > 10)
        GEOMAPI_PRINTF ("           (g %d groupDelta %d) => (globalInOut %d) (globalCount %d)\n", (int)groupId, direction, m_compositeNumberIn, m_compositeCount);
    }

bool IsActiveEdge (MTGNodeId nodeId)
    {
    if (0 != activeEdgeMask)
        {
        MTGGraph *graph = jmdlRG_getGraph (m_pRG);
        if (0 != graph->GetMaskAt (nodeId, activeEdgeMask))
            return true;
        if (0 != graph->GetMaskAt (graph->EdgeMate (nodeId), activeEdgeMask))
            return true;
        return false;
        }
    return true;
    }    
/*------------------------------------------------------------------*//**
* Find the maximum group id in the graph.
* Initialize parity array to zero for each group id from 0 to max.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
bool  InitStateArrays (size_t &numOrphan)
    {
    MTGGraph *pGraph = jmdlRG_getGraph (m_pRG);
    MTGMask edgeVisited = jmdlMTGGraph_grabMask (pGraph);
    bool    boolstat = true;
    int groupId;
    ptrdiff_t maxGroupId = -1;
    numOrphan = 0;
    ClearCounts ();
    jmdlMTGGraph_clearMaskInSet (pGraph, edgeVisited);
    MTGARRAY_SET_LOOP (seedNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, edgeVisited))
            {
            jmdlMTGGraph_setMaskAroundEdge (pGraph, seedNodeId, edgeVisited);
            if (jmdlRG_getGroupId (m_pRG, &groupId, seedNodeId)
                && groupId >= 0)
                {
                ptrdiff_t g = (ptrdiff_t) groupId;
                if (g > maxGroupId)
                    maxGroupId = g;
                }
            else
                {
                numOrphan++; 
                }
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, pGraph)
    jmdlMTGGraph_dropMask (pGraph, edgeVisited);

    ClearCounts ();
    groupStateArray.clear ();
    for (ptrdiff_t i = 0; i <= maxGroupId; i++)
        groupStateArray.push_back (0);

    return boolstat;
    }


/*------------------------------------------------------------------*//**
* Callback to handle face entry and exit.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
void RecordFaceEvent
(
FloodEvent edgeEvent,
bvector<MTGNodeId> &stack,
int noisy
)
    {
    MTGNodeId currNodeId;
    int groupId;
    jmdlEmbeddedIntArray_getInt(&stack, &currNodeId, -1);
    //markup_announce (m_pRG, edgeEvent, stack, pMarkup, noisy);
    /* Synch the groupid stack */
    switch (edgeEvent)
        {
        case FloodEvent_EnterComponent:
            /* No action -- this node is outside. */
            break;
        case FloodEvent_EnterFace:
            if (jmdlRG_getGroupId (m_pRG, &groupId, currNodeId))
                {
                if (IsActiveEdge (currNodeId))
                    RecordGroupStateChange (groupId, 1);
                }
            break;
        case FloodEvent_ExitFace:
            if (jmdlRG_getGroupId (m_pRG, &groupId, currNodeId))
                {
                if (IsActiveEdge (currNodeId))
                    RecordGroupStateChange (groupId, -1);
                }
            break;
        case FloodEvent_ExitComponent:
            // Check if group parity array is all zeros?
            break;
        }

    if (   currNodeId != MTG_NULL_NODEID
        && edgeEvent == FloodEvent_EnterFace && EvaluateState ()
        )
        {
        jmdlMTGMarkSet_addNode (pMarkSet, currNodeId);
        }
    }

bool       FloodFromOneSeed
(
bvector<MTGNodeId> &stack,
MTGNodeId           seedNodeId,
MTGMask             faceVisited,
MTGMask             edgeVisited,
int                 noisy
)
    {
    MTGNodeId currNodeId, mateNodeId, topNodeId;
    MTGGraph *pGraph = jmdlRG_getGraph (m_pRG);    
    bool    boolstat = true;
    static int s_noisyTrigger = 0;

    stack.clear ();
    stack.push_back (seedNodeId);
    jmdlMTGGraph_setMaskAroundFace (pGraph, seedNodeId, faceVisited);

    RecordFaceEvent (FloodEvent_EnterComponent, stack, noisy);
    currNodeId = seedNodeId;

    for (;;)
        {
        mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
        jmdlMTGGraph_setMask (pGraph, currNodeId, edgeVisited);
        jmdlMTGGraph_setMask (pGraph, mateNodeId, edgeVisited);
        if (noisy > s_noisyTrigger)
                GEOMAPI_PRINTF ("    probe edge %d mate %d\n", currNodeId, mateNodeId);

        if (!jmdlMTGGraph_getMask (pGraph, mateNodeId, faceVisited))
            {
            /* Jump into the new face. */
            if (noisy > s_noisyTrigger)
                GEOMAPI_PRINTF ("    enter and mark face %d\n",  mateNodeId);
            jmdlMTGGraph_setMaskAroundFace (pGraph, mateNodeId, faceVisited);
            stack.push_back (mateNodeId);
            RecordFaceEvent (FloodEvent_EnterFace, stack, noisy);
            currNodeId = mateNodeId;
            }
        else
            {
            currNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId);
            if (currNodeId == seedNodeId)
                {
                // hmm.. Unexpected, but ok if the seed started at an outside dangler.
                break;
                }
            if (noisy > s_noisyTrigger)
                GEOMAPI_PRINTF ("     face step to currNode.fSucc = %d\n",  currNodeId);
            if (!jmdlEmbeddedIntArray_getInt (&stack, &topNodeId, -1))
                {
                /* Stack should never be empty ... entry to current face
                   is on top */
                boolstat = false;
                break;
                }
            else
                {
                if (noisy > s_noisyTrigger)
                    GEOMAPI_PRINTF ("            top of stack = %d\n",  topNodeId);
                if (topNodeId == currNodeId)
                    {
                    if (noisy > s_noisyTrigger)
                        GEOMAPI_PRINTF ("    exit face %d\n", currNodeId);
                    /* We are done with the current face.
                        Exit back to previous face */
                    RecordFaceEvent (FloodEvent_ExitFace, stack, noisy);
                    /* Don't even need to check if pop succeeded -- previous getInt
                        verified that there is something there. */
                    jmdlEmbeddedIntArray_popInt (&stack, &topNodeId);
                    /* We should only run into a visted edge on return
                       to the face entry, as recorded on the stack. */
                    currNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
                    /* These two conditions should occur at the same time!!! */
                    if (currNodeId == seedNodeId)
                        {
                        break;
                        }
                    currNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId);
                    }
                }
            }
        }
    if (jmdlEmbeddedIntArray_getCount (&stack) != 1)
        {
        /* This should not happen ... */
        boolstat = false;
        }
    RecordFaceEvent (FloodEvent_ExitComponent, stack, noisy);
    return boolstat;
    }


/*------------------------------------------------------------------*//**
* Face-by-face DFS through the fully merged graph structure.
*<ul>
*<li>Only begin at true exterior edges.</li>
*<li>Announce each face entry and exit through a callback.</li>
*</ul>
* @param transitionFunction => Callback function of form
*<pre>
*           F(m_pRG,
*</pre>
*<ul>
*<li>Begin connected component</li>
*<li>Enter face across edge</li>
*<li>Exit face across edge</li>
*<li>End connected component</li>
*<li>
*</ul>
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
bool        FloodFromAllSeeds ()
    {
    MTGGraph *pGraph = jmdlRG_getGraph (m_pRG);
    MTGMask faceVisited = jmdlMTGGraph_grabMask (pGraph);
    MTGMask edgeVisited = jmdlMTGGraph_grabMask (pGraph);
    bvector<MTGNodeId> stack;
    bool    boolstat = true;
    static int s_noisy = 0;
    int noisy = getNoisy (s_noisy);


    jmdlMTGGraph_clearMaskInSet (pGraph, faceVisited | edgeVisited);
    MTGARRAY_SET_LOOP (seedNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, faceVisited)
            && jmdlRG_faceIsTrueExterior (m_pRG, seedNodeId))
            {
            if (noisy)
                    GEOMAPI_PRINTF (" flood loop seed %d\n", seedNodeId);
            MTGARRAY_FACE_LOOP (entryNodeId, pGraph, seedNodeId)
                {
                if (!jmdlMTGGraph_getMask (pGraph, jmdlMTGGraph_getEdgeMate (pGraph, entryNodeId), faceVisited))
                    {
                    boolstat = FloodFromOneSeed
                            (
                            stack,
                            entryNodeId,
                            faceVisited,
                            edgeVisited,
                            noisy
                            );
                    if (!boolstat)
                        break;
                    }
                }
            MTGARRAY_END_FACE_LOOP (entryNodeId, pGraph, seedNodeId)
            }
        if (!boolstat)
            break;
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, pGraph)

    jmdlMTGGraph_dropMask (pGraph, faceVisited);
    jmdlMTGGraph_dropMask (pGraph, edgeVisited);
    return boolstat;
    }
    
};


extern void regularizeMarkSet (RG_Header *pRG, MTG_MarkSet *pMarkSet);

//static int s_noisy = 0;

void printNodeXY
(
MTGGraph *pGraph,
MTGNodeId nodeId,
RG_Header   *pRG
);

/*------------------------------------------------------------------*//**
* @bsihdr                                       EarlinLutz      04/13
+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRG_collectAnalysisFaces
(
RG_Header    *pRG,
AreaSelect   groupSelect,
BoolSelect   boolSelect,
MTG_MarkSet  *pMarkSet,
bool         reverseSense,
MTGMask      activeEdgeMask
)
    {
    AreaMarkupContext context (pRG, groupSelect, boolSelect, pMarkSet, reverseSense, activeEdgeMask);
    static bool    s_noisy = false;
    int oldNoisy = jmdlRG_getNoisy ();
    if (s_noisy)
        jmdlRG_setNoisy (1000);  
    bool    result = false;

    jmdlMTGMarkSet_empty (pMarkSet);
    if (s_noisy)
        jmdlMTGGraph_printFaceLoopsExt (pRG->pGraph, (MTGNodeFunc)printNodeXY, pRG);
    size_t numOrphan = 0;   // Do we care if the graph has nodes with no group?
    if (context.InitStateArrays (numOrphan))
        {
        jmdlRG_buildBridgeEdges (pRG);

        if (s_noisy)
            jmdlMTGGraph_printFaceLoopsExt (pRG->pGraph, (MTGNodeFunc)printNodeXY, pRG);


        result = context.FloodFromAllSeeds ();
        jmdlRG_dropBrideEdges (pRG);
        regularizeMarkSet (pRG, pMarkSet);
        }

    if (s_noisy)
        jmdlMTGGraph_printFaceLoopsExt (pRG->pGraph, (MTGNodeFunc)printNodeXY, pRG);

    jmdlRG_setNoisy (oldNoisy);
    return result;
    }

void ArrayToMap (bvector<int> &data, bmap<int,bool> &map)
    {
    map.clear ();
    for (size_t i = 0; i < data.size (); i++)
        map[data[i]] = true;
    }

Public void jmdlRG_setMaskByGroupArray
(
RG_Header    *pRG,
bvector<int>  &groups,
MTGMask      mask,
bool         maskValue,
bool         initializeSetToOppositeValue
)
    {
    MTGGraph *pGraph = jmdlRG_getGraph (pRG);
    bmap<int,bool> map;
    ArrayToMap (groups, map);
    int groupId;
    if (initializeSetToOppositeValue)
        {
        if (maskValue)
            pGraph->ClearMask (mask);
        else
            pGraph->SetMask (mask);

        }

    MTGARRAY_SET_LOOP (nodeId, pGraph)
        {
        if (jmdlRG_getGroupId (pRG, &groupId, nodeId))
            {
            if (map.count (groupId) == 1)
                {
                if (maskValue)
                    pGraph->SetMaskAroundEdge (nodeId, mask);
                else
                    pGraph->ClearMaskAroundEdge (nodeId, mask);
                }
            }
         }
    MTGARRAY_END_SET_LOOP (nodeId, pGraph)
    }
END_BENTLEY_GEOMETRY_NAMESPACE
