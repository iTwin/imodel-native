/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
@description Delete all nodes on stacked graphs.
@remarks Nodes in the current graph are unaffected.
@param graphP IN OUT graph header
@group "VU Graph Stack"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_stackClearAll
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Push all nodes from the current graph onto the graph stack.
@remarks The current graph appears to be empty after the push.
@remarks If there are no current nodes, an empty stack frame is pushed.
@param graphP IN OUT graph header
@return the number of stacked node sets after this push
@group "VU Graph Stack"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackPush
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Push all nodes from the current graph onto the graph stack, and immediately
    assemble a copy of the just-stacked nodes in the graph.
@remarks If there are no current nodes, an empty stack frame is pushed, and the post-copy graph has no nodes.
@param graphP IN OUT graph header
@return the number of stacked node sets after this push
@group "VU Graph Stack"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackPushCopy
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Pop all nodes from the top node set of the graph stack into the current graph.
@remarks All popped nodes become accessible by normal traversals such as visiting all nodes in the graph.
@param graphP IN OUT graph header
@return the number of stacked node sets after this pop
@group "VU Graph Stack"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackPop
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Pop all nodes from the top node set of the graph stack into the current graph, and
    immediately execute the callback operation on the graph.
@param graphP IN OUT graph header
@param operationFunc IN callback operation
@param userDataP IN unused
@return the number of stacked node sets after this pop
@group "VU Graph Stack"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackPopWithOperation
(
VuSetP          graphP,
VuStackOpFuncP  operationFunc,
void            *userDataP
);

/*---------------------------------------------------------------------------------**//**
@description Pop all nodes of a specified number of stacked node sets back into the current graph.
@remarks All popped nodes become accessible by normal traversals such as visiting all nodes in the graph.
@param graphP IN OUT graph header
@param numberOfEntries IN number of pops to execute
@return the number of stacked node sets after this pop
@group "VU Graph Stack"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackPopNEntries
(
VuSetP          graphP,
int             numberOfEntries
);

/*---------------------------------------------------------------------------------**//**
@description Pop all nodes of all stacked node sets back into the current graph.
@remarks All nodes become accessible by normal traversals such as visiting all nodes in the graph.
@param graphP IN OUT graph header
@return the number of stacked node sets after this pop: zero
@group "VU Graph Stack"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackPopAll
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Swap node sets between the "current" graph and the top node set in the graph stack.
@param graphP IN OUT graph header
@group "VU Graph Stack"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_stackExchange
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Query the number of stacked node sets.
@param graphP IN graph header
@return number of stacked node sets
@group "VU Graph Stack"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_stackSize
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description return (a pointer to ) the list of all active nodes.  Clear the active set.
    Caller becomes responsible for returning the nodes at application specific time.
@param graphP IN OUT graph header
@return pointer to nodes in (prior) active set.
@group "VU Graph Stack"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP vu_extractNodeList (VuSetP graphP);

/*---------------------------------------------------------------------------------**//**
@description Reactivate nodes previously obtained from vu_extractNodeList.
@param graphP IN OUT graph header
@param nodeListP IN node pointer previously returned from vu_extractNodeList
@group "VU Graph Stack"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_restoreNodeList (VuSetP graphP, VuP nodeListP);

END_BENTLEY_GEOMETRY_NAMESPACE

