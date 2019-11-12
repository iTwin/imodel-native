/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
#include <stdlib.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


GEOMDLLIMPEXP MTG_MarkSet::MTG_MarkSet (MTGGraphP _pGraph, MTGMarkScope   _scope)
    {
    pGraph = _pGraph;
    scope = _scope;
    mask = jmdlMTGGraph_grabMask (pGraph);
    jmdlMTGGraph_clearMaskInSet (pGraph, mask);
    }

GEOMDLLIMPEXP MTG_MarkSet::MTG_MarkSet ()
    {
    pGraph = NULL;
    }

GEOMDLLIMPEXP void MTG_MarkSet::Attach (MTGGraphP _pGraph, MTGMarkScope   _scope)
    {
    if (NULL != pGraph)
        {
        jmdlMTGGraph_dropMask (pGraph, mask);
        }
    pGraph = _pGraph;
    scope = _scope;
    if (NULL != pGraph)
        {
        mask = jmdlMTGGraph_grabMask (pGraph);
        jmdlMTGGraph_clearMaskInSet (pGraph, mask);
        }
    else
        mask = MTG_NULL_MASK;
    nodes.clear ();    
    }



GEOMDLLIMPEXP MTG_MarkSet::~MTG_MarkSet ()
    {
    if (NULL != pGraph)
        jmdlMTGGraph_dropMask (pGraph, mask);
    }

bool MTG_MarkSet::TryPopCandidate (MTGNodeId &nodeId)
    {
    if (nodes.size () == 0)
        return false;
    nodeId = nodes.back ();
    nodes.pop_back ();
    return true;
    }

bool MTG_MarkSet::TryGetCandidateAt (size_t index, MTGNodeId &nodeId) const
    {
    if (index >= nodes.size ())
        return false;
    nodeId = nodes[index];
    return true;
    }

GEOMDLLIMPEXP MTGMask MTG_MarkSet::GetMask () const {return mask;}

/*---------------------------------------------------------------------------------**//**
* @param    pInstance   <=> affected set.
* @param nodeId          => first affected node.
* @return
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGMarkSet_addNode
(
MTG_MarkSet     *pInstance,
MTGNodeId      nodeId
)
    {
    pInstance->AddNode (nodeId);
    }

GEOMDLLIMPEXP void MTG_MarkSet::AddNode (MTGNodeId nodeId)
    {
    if (!jmdlMTGGraph_getMask (pGraph, nodeId, mask))
        {
        switch (scope)
            {
            case MTG_ScopeNode:
                jmdlMTGGraph_setMask (pGraph, nodeId, mask);
                break;

            case MTG_ScopeVertex:
                jmdlMTGGraph_setMaskAroundVertex (pGraph, nodeId, mask);
                break;

            case MTG_ScopeFace:
                jmdlMTGGraph_setMaskAroundFace (pGraph, nodeId, mask);
                break;

            case MTG_ScopeEdge:
                jmdlMTGGraph_setMaskAroundEdge (pGraph, nodeId, mask);
                break;
            }
        nodes.push_back (nodeId);
        }
    }

GEOMDLLIMPEXP void MTG_MarkSet::AddNodesInScope (MTGNodeId seedNodeId, MTGMarkScope markScope)
    {
    if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, mask))
        {
        switch (markScope)
            {
            case MTG_ScopeNode:
                AddNode (seedNodeId);
                break;

            case MTG_ScopeVertex:
                MTGARRAY_VERTEX_LOOP (nodeId, pGraph, seedNodeId)
                    {
                    AddNode (nodeId);
                    }
                MTGARRAY_END_VERTEX_LOOP (nodeId, pGraph, seedNodeId)
                break;

            case MTG_ScopeFace:
                MTGARRAY_FACE_LOOP (nodeId, pGraph, seedNodeId)
                    {
                    AddNode (nodeId);
                    }
                MTGARRAY_END_FACE_LOOP (nodeId, pGraph, seedNodeId)
                break;

            case MTG_ScopeEdge:
                AddNode (seedNodeId);
                AddNode (pGraph->EdgeMate (seedNodeId));
                break;
            }
        }
    }



/*---------------------------------------------------------------------------------**//**
* @param    pInstance   <=> affected set.
* @param nodeId          => node to test
* @return
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGMarkSet_isNodeInSet
(
MTG_MarkSet     *pInstance,
MTGNodeId      nodeId
)
    {
    return pInstance->IsNodeInSet (nodeId);
    }

GEOMDLLIMPEXP bool MTG_MarkSet::IsNodeInSet (MTGNodeId nodeId) const
    {
    return  jmdlMTGGraph_getMask (pGraph, nodeId, mask) ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* Unmark a node.   This is done by removing the mask, but the array is not searched.
* @param    pInstance   <=> affected set.
* @param nodeId          => node to remove
* @return
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlMTGMarkSet_removeNode
(
MTG_MarkSet     *pInstance,
MTGNodeId      nodeId
)
    {
    return pInstance->RemoveNode (nodeId);
    }

GEOMDLLIMPEXP void MTG_MarkSet::RemoveNode (MTGNodeId nodeId)
    {
    switch (scope)
        {
        case MTG_ScopeNode:
            jmdlMTGGraph_clearMask (pGraph, nodeId, mask);
            break;

        case MTG_ScopeVertex:
            jmdlMTGGraph_clearMaskAroundVertex (pGraph, nodeId, mask);
            break;

        case MTG_ScopeFace:
            jmdlMTGGraph_clearMaskAroundFace (pGraph, nodeId, mask);
            break;

        case MTG_ScopeEdge:
            jmdlMTGGraph_clearMaskAroundEdge (pGraph, nodeId, mask);
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Choose any node in the set.  Remove it (and all other nodes in its scope)
* from the set.
* @param    pInstance   <=> affected set.
* @return
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId   jmdlMTGMarkSet_chooseAndRemoveNode
(
MTG_MarkSet     *pInstance
)
    {
    return pInstance->ChooseAndRemoveNode ();
    }

GEOMDLLIMPEXP MTGNodeId   MTG_MarkSet::ChooseAndRemoveNode ()
    {
    MTGNodeId candidateId;
    /* The array gives candidates, which may have been unmarked asynchronously.
       As they are pulled out, ignore the ones that are not marked. */
    while   (TryPopCandidate (candidateId))
        {
        if (jmdlMTGGraph_getMask (pGraph, candidateId, mask))
            {
            /* removeNode will just unmark the scope */
            RemoveNode (candidateId);
            return  candidateId;
            }
        }
    return  MTG_NULL_NODEID;
    }


/*---------------------------------------------------------------------------------**//**
* @param    pInstance   <=> header whose memory and mask are to be released.
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGMarkSet_empty
(
MTG_MarkSet     *pInstance
)
    {
    pInstance->Clear ();
    }

GEOMDLLIMPEXP void MTG_MarkSet::Clear ()
    {
    while (MTG_NULL_NODEID != ChooseAndRemoveNode ())
        {
        }
    }

/*---------------------------------------------------------------------------------**//**
* Prepare a traversal index prior to iterating through the mark set.
* @param    pInstance   <=> affected set.
* @param pIteratorIndex <=> traversal index
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlMTGMarkSet_initIteratorIndex
(
MTG_MarkSet     *pInstance,
int             *pIteratorIndex
)
    {
    *pIteratorIndex = (int)pInstance->InitIteratorIndex ();
    }

GEOMDLLIMPEXP int MTG_MarkSet::InitIteratorIndex () const
    {
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* Return the next node in a traversal of the set.
* @param    pInstance   <=> affected set.
* @param pIteratorIndex <=> traversal index
* @param pNodeId        <=  the retrived node
* @return true if node found.
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGMarkSet_getNextNode
(
MTG_MarkSet     *pInstance,
int             *pIteratorIndex,
int             *pNodeId
)
    {
    return pInstance->TryGetNextNode (*pIteratorIndex, *pNodeId);
    }

GEOMDLLIMPEXP bool MTG_MarkSet::TryGetNextNode (int &movingIndex, MTGNodeId &nodeId) const
    {
    MTGNodeId candidateId;
    nodeId = MTG_NULL_NODEID;
    /* The array gives candidates, which may have been unmarked asynchronously.
       As they are pulled out, ignore the ones that are not marked. */
    while   (TryGetCandidateAt ((size_t)movingIndex, candidateId))
        {
        movingIndex += 1;
        if (jmdlMTGGraph_getMask (pGraph, candidateId, mask))
            {
            nodeId = candidateId;
            return  true;
            }
        }
    return  false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
