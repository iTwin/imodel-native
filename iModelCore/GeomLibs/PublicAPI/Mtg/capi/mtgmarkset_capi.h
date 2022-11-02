/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @param    pInstance   IN OUT  header to initialize.
* @param pGraph IN      graph being marked.  The pointer to the graph is saved.
* @param scope IN      indicates scope of node marking.
* @return
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlMTGMarkSet_init
(
MTG_MarkSet     *pInstance,
MTGGraph       *pGraph,
MTGMarkScope   scope
);

/*---------------------------------------------------------------------------------**//**
* @param    pInstance   IN OUT  header whose memory and mask are to be released.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGMarkSet_decommission
(
MTG_MarkSet     *pInstance
);

/*---------------------------------------------------------------------------------**//**
* @param    pInstance   IN OUT  affected set.
* @param nodeId          IN      first affected node.
* @return
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGMarkSet_addNode
(
MTG_MarkSet     *pInstance,
MTGNodeId      nodeId
);

/*---------------------------------------------------------------------------------**//**
* @param    pInstance   IN OUT  affected set.
* @param nodeId          IN      node to test
* @return
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGMarkSet_isNodeInSet
(
MTG_MarkSet     *pInstance,
MTGNodeId      nodeId
);

/*---------------------------------------------------------------------------------**//**
* Unmark a node.   This is done by removing the mask, but the array is not searched.
* @param    pInstance   IN OUT  affected set.
* @param nodeId          IN      node to remove
* @return
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlMTGMarkSet_removeNode
(
MTG_MarkSet     *pInstance,
MTGNodeId      nodeId
);

/*---------------------------------------------------------------------------------**//**
* Choose any node in the set.  Remove it (and all other nodes in its scope)
* from the set.
* @param    pInstance   IN OUT  affected set.
* @return
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId   jmdlMTGMarkSet_chooseAndRemoveNode
(
MTG_MarkSet     *pInstance
);

/*---------------------------------------------------------------------------------**//**
* @param    pInstance   IN OUT  header whose memory and mask are to be released.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGMarkSet_empty
(
MTG_MarkSet     *pInstance
);

/*---------------------------------------------------------------------------------**//**
* Prepare a traversal index prior to iterating through the mark set.
* @param    pInstance   IN OUT  affected set.
* @param pIteratorIndex IN OUT  traversal index
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlMTGMarkSet_initIteratorIndex
(
MTG_MarkSet     *pInstance,
int             *pIteratorIndex
);

/*---------------------------------------------------------------------------------**//**
* Return the next node in a traversal of the set.
* @param    pInstance   IN OUT  affected set.
* @param pIteratorIndex IN OUT  traversal index
* @param pNodeId        OUT     the retrived node
* @return true if node found.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGMarkSet_getNextNode
(
MTG_MarkSet     *pInstance,
int             *pIteratorIndex,
int             *pNodeId
);

END_BENTLEY_GEOMETRY_NAMESPACE

