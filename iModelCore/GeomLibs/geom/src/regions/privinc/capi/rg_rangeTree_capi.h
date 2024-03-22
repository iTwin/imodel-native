/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

/* DO NOT EDIT!  THIS FILE IS GENERATED. */

#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool    rgXYRangeTree_initializeTree
(
void            **ppTree
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool       rgXYRangeTree_addToTree
(
XYRangeTreeNode *pTree,
double          xMin,
double          yMin,
double          xMax,
double          yMax,
int             userData
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void rgXYRangeTree_destroyRangeTree
(
XYRangeTreeNode **treePP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void rgXYRangeTree_getLeafRange
(
void        *pVoidLeaf,
DRange3d    *pRange
);

/*---------------------------------------------------------------------------------**//**
* Traverse a tree, applying functions to subtrees. A NULL node function accepts all subtrees. Returns AND of all leaf-level processLeaf calls.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool             rgXYRangeTree_traverseTree
(
void                    *pTree,
HideTree_NodeFunction            nodeFunction,          /* IN      If NULL, all subtrees are traversed.
                                                      true from nodeFunction allows subtree
                                                        to be processed.   false skips this
                                                        subtree but continues with remainder of tree */
void                    *nodeFunctionArgP,
HideTree_ProcessLeafFunction     processLeaf,
void                    *geometryFunctionArgP
);

/*---------------------------------------------------------------------------------**//**
* Use this function as the HideTree_NodeFunction on a range search. Returns true if there is any overlap between the test range and the node
* range.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool             rgXYRangeTree_nodeFunction_testForRangeOverlap
(
HideTreeRange           *pNodeRange,
void                    *pNode,
HideTreeRange           *pTestRange
);

/*---------------------------------------------------------------------------------**//**
* Use this function as the HideTree_NodeFunction on a range search. Returns true if the node range is completely contained in the test test
* range.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool             rgXYRangeTree_nodeFunction_testForNodeRangeInTestRange
(
HideTreeRange           *pNodeRange,
void                    *pNode,
HideTreeRange           *pTestRange
);

/*---------------------------------------------------------------------------------**//**
* Use this function as the HideTree_NodeFunction on a range search. Returns true if the node range is completely contained in the test test
* range.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool             rgXYRangeTree_nodeFunction_testForTestRangeAllInNodeRange
(
HideTreeRange           *pNodeRange,
void                    *pNode,
HideTreeRange           *pTestRange
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void             rgXYRangeTree_initRange
(
HideTreeRange           *pRange,
double                  xmin,
double                  ymin,
double                  xmax,
double                  ymax

);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void             rgXYRangeTree_initRangeFromDRange3d
(
HideTreeRange           *pRange,
DRange3d                *pRange3d
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void rgXYRangeTree_drawTree
(
void            *pTree,
int             displayMode,
int             color
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  rgXYRangeTree_dumpTree
(
XYRangeTreeNode *pTree,
int             indent,
char            *labelP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void rgXYRangeTree_displayStatistics
(
void
);

END_BENTLEY_GEOMETRY_NAMESPACE

