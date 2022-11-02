/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "privinc/rg_rangetree.h"
#include <stdlib.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


// Historical note; This range tree originated as the hline range tree, moved to regions,
// then to the bsibasegeom tree.    A more generic form was split off, and this original C-style
// implementation should only be kept in the rg world until an appropriate time for migrating to the
// newer C++.

#define X_AXIS                          0
#define Y_AXIS                          1
#define Z_AXIS                          2




// Top is the top of the range tree
#define NODETYPE_Top            0

// Interior are nodes which have other nodes as children
#define NODETYPE_Interior               1

// Leaf are the lowest nodes (containers) which have the data elements as children
#define NODETYPE_Leaf           2

// UserData are the data items inserted into the tree.
// All the other types are containers required by the tree
#define NODETYPE_UserData       3

typedef struct
    {
    int                 type;
    HideTreeRange       range;
    void                *pParent;
    int                 userData;
    } NodeHeader;

#define MAX_ENTRIES     50

typedef struct _XYRangeTreeNode
    {
    NodeHeader          header;
    void                *entries[MAX_ENTRIES];
    int                 nEntries;
    double              extentSquared;
    } XYRangeTreeNode;

typedef struct
    {
    int             groupNumber[2];
    XYRangeTreeNode         *pNode;
    } NodeTest;

typedef struct
    {
    int             numEntries;
    NodeTest        *pAxisEntries;
    } SplitInfo;

/*----------------------------------------------------------------------+
|                                                                       |
|  Local Function Prototypes                                            |
|                                                                       |
+----------------------------------------------------------------------*/
void    insertEntry
(
XYRangeTreeNode *pNode,
XYRangeTreeNode *pNewNode
);

#define MAXITEMSIZE 100

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static double  extentSquared
(
HideTreeRange    *pRange
)
    {
    if (pRange->xmin == xyMaxVal)
        {
        return  0.0;
        }
    else
        {
        double xdelta = (pRange->xmax - pRange->xmin);
        double ydelta = (pRange->ymax - pRange->ymin);
        return  (xdelta*xdelta + ydelta*ydelta);
        }
    }

/*---------------------------------------------------------------------------------**//**
* extends the coordinates of the range cube points pRange so as to include the range cube range1P.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static  void extendByRange
(
        HideTreeRange        *pRange0,               /* <=> range to be extended */
const   HideTreeRange        *pRange1                /* <=  second range */
)
    {
    if (pRange0 && pRange1)
        {
        pRange0->xmin = MIN (pRange1->xmin, pRange0->xmin);
        pRange0->xmax = MAX (pRange1->xmax, pRange0->xmax);
        pRange0->ymin = MIN (pRange1->ymin, pRange0->ymin);
        pRange0->ymax = MAX (pRange1->ymax, pRange0->ymax);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extendNodeRange
(
XYRangeTreeNode    *pNode,
HideTreeRange     *pRange
)
    {
    extendByRange (&pNode->header.range, pRange);
    pNode->extentSquared = extentSquared (&pNode->header.range);
    }

/*---------------------------------------------------------------------------------**//**
* Initializes a range cube with (inverted) large positive and negative values.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static  void     range_init
(
        HideTreeRange        *pRange        /* <= range to be initialized */
)
    {
    pRange->xmin =  xyMaxVal;
    pRange->xmax =  -xyMaxVal;

    pRange->ymin =  xyMaxVal;
    pRange->ymax =  -xyMaxVal;
    }
#ifdef COMPILE_rangeIsNull
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        rangeIsNull
(
HideTreeRange   *pRange
)
    {
    return pRange->xmin == xyMaxVal;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static  void range_combineRange
(
        HideTreeRange    *pCombRange,
const   HideTreeRange    *pRangeOne,
const   HideTreeRange    *pRangeTwo
)
    {
    if (pCombRange)
        {
        pCombRange->xmin = MIN (pRangeOne->xmin, pRangeTwo->xmin);
        pCombRange->xmax = MAX (pRangeOne->xmax, pRangeTwo->xmax);
        pCombRange->ymin = MIN (pRangeOne->ymin, pRangeTwo->ymin);
        pCombRange->ymax = MAX (pRangeOne->ymax, pRangeTwo->ymax);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static XYRangeTreeNode    *initNode
(
int         nodeType
)
    {
    XYRangeTreeNode         *pNode = (XYRangeTreeNode*)calloc (1, sizeof(XYRangeTreeNode));
    _Analysis_assume_(pNode != nullptr);

    range_init (&pNode->header.range);
    pNode->header.type = nodeType;

    return pNode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static  bool    range_isContained
(
const HideTreeRange  *pInnerRange,
const HideTreeRange  *pOuterRange
)
    {
    return  (pInnerRange->xmin >= pOuterRange->xmin &&
             pInnerRange->xmax <= pOuterRange->xmax &&
             pInnerRange->ymin >= pOuterRange->ymin &&
             pInnerRange->ymax <= pOuterRange->ymax);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static XYRangeTreeNode    *getChild
(
XYRangeTreeNode    *pParent,
int         childIndex
)
    {
    return (childIndex < pParent->nEntries) ? (XYRangeTreeNode *) pParent->entries[childIndex] : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    setChild
(
XYRangeTreeNode    *pParent,
int         childIndex,
XYRangeTreeNode    *pChild
)
    {
    if (childIndex < pParent->nEntries)
        pParent->entries[childIndex] = pChild;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    addChild
(
XYRangeTreeNode *pParent,
XYRangeTreeNode *pChild
)
    {
    pParent->entries[pParent->nEntries++] = pChild;
    if (NODETYPE_Leaf != pParent->header.type)
        pChild->header.pParent = pParent;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    increaseInteriorRange
(
XYRangeTreeNode *pNode,
HideTreeRange   *pRange
)
    {
    if (NULL != pNode && !range_isContained (pRange, &pNode->header.range))
        {
        extendNodeRange (pNode, pRange);
        increaseInteriorRange ((XYRangeTreeNode*)pNode->header.pParent, &pNode->header.range);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int     compareXRange
(
const NodeTest  *pFirst,
const NodeTest  *pSecond
)
    {
    if (pFirst->pNode->header.range.xmin < pSecond->pNode->header.range.xmin)
        return  -1;
    if (pFirst->pNode->header.range.xmin > pSecond->pNode->header.range.xmin)
        return   1;
    if (pFirst->pNode->header.range.xmax < pSecond->pNode->header.range.xmax)
        return  -1;
    if (pFirst->pNode->header.range.xmax > pSecond->pNode->header.range.xmax)
        return   1;
    return  0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int     compareYRange
(
const NodeTest  *pFirst,
const NodeTest  *pSecond
)
    {
    if (pFirst->pNode->header.range.ymin < pSecond->pNode->header.range.ymin)
        return  -1;
    if (pFirst->pNode->header.range.ymin > pSecond->pNode->header.range.ymin)
        return   1;
    if (pFirst->pNode->header.range.ymax < pSecond->pNode->header.range.ymax)
        return  -1;
    if (pFirst->pNode->header.range.ymax > pSecond->pNode->header.range.ymax)
        return   1;
    return  0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static double  checkSeparation
(
NodeTest   *pSplitEntries,
int         count,
int         axis
)
    {
    NodeTest    *pEntryEnd, *pStartEntries, *pCurrEntry, *pNextEntry, *pSepEntry=0, *pMinDistEntry=0;
    NodeTest    *pLastEntry;
    double       maxSeparation = 0.0, separation;
    double       maxMinDist = -1.0e200, minDist;
    int          minSize;


    qsort (pSplitEntries, count, sizeof(NodeTest), (axis==X_AXIS)
                ? (int (*)(const void *,const void *))compareXRange
                : (int (*)(const void *,const void *))compareYRange);

    minSize = count/3;
    pLastEntry = pSplitEntries + count;
    pEntryEnd = pSplitEntries + (count-minSize);
    pStartEntries = pSplitEntries + minSize;

    switch (axis)
        {
        case X_AXIS:
            for (pCurrEntry = pStartEntries; pCurrEntry < pEntryEnd-1; pCurrEntry++)
                {
                pNextEntry = pCurrEntry + 1;

                if (pCurrEntry->pNode->header.range.xmax < pNextEntry->pNode->header.range.xmin)
                    {
                    if ((separation = (pNextEntry->pNode->header.range.xmin - pCurrEntry->pNode->header.range.xmax)) > maxSeparation)
                        {
                        maxSeparation = separation;
                        pSepEntry = pNextEntry;
                        }
                    }
                else
                    {
                    if ((minDist = (pNextEntry->pNode->header.range.xmin - pCurrEntry->pNode->header.range.xmin)) > maxMinDist)
                        {
                        maxMinDist = minDist;
                        pMinDistEntry = pNextEntry;
                        }
                    }
                }
            break;

        case Y_AXIS:
            for (pCurrEntry = pStartEntries; pCurrEntry < pEntryEnd-1; pCurrEntry++)
                {
                pNextEntry = pCurrEntry + 1;

                if (pCurrEntry->pNode->header.range.ymax < pNextEntry->pNode->header.range.ymin)
                    {
                    if ((separation = (pNextEntry->pNode->header.range.ymin - pCurrEntry->pNode->header.range.ymax)) > maxSeparation)
                        {
                        maxSeparation = separation;
                        pSepEntry = pNextEntry;
                        }
                    }
                else
                    {
                    if ((minDist = (pNextEntry->pNode->header.range.ymin - pCurrEntry->pNode->header.range.ymin)) > maxMinDist)
                        {
                        maxMinDist = minDist;
                        pMinDistEntry = pNextEntry;
                        }
                    }
                }
            break;
        }


    if (NULL == pSepEntry)
        {
        pSepEntry = (pMinDistEntry)
                    ? pMinDistEntry
                    : pSplitEntries + count/2;
        }


    for (
         pCurrEntry = pSplitEntries;
         pCurrEntry < pLastEntry;
         pCurrEntry++
        )
        pCurrEntry->groupNumber[axis] = (pCurrEntry >= pSepEntry);

    return  maxSeparation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    splitNode
(
XYRangeTreeNode *pNode
)
    {
    int                 i, optimalSplit;
    double              xSep, ySep;
    XYRangeTreeNode            **ppNodeStart, **ppNodeEnd, **ppCurrNode;
    NodeTest           *pEntryStart, *pEntryEnd, *pCurrEntry;
    SplitInfo           splitInfo;
    XYRangeTreeNode            *pNewNode;

    memset (&splitInfo, 0, sizeof (splitInfo));
    splitInfo.numEntries = pNode->nEntries;
    splitInfo.pAxisEntries = (NodeTest *)calloc (splitInfo.numEntries, sizeof(NodeTest));
    _Analysis_assume_(splitInfo.pAxisEntries != nullptr);

    ppNodeStart = (XYRangeTreeNode **) pNode->entries;
    ppNodeEnd  = ppNodeStart + splitInfo.numEntries;
    pEntryStart = splitInfo.pAxisEntries;
    pEntryEnd = pEntryStart + splitInfo.numEntries;

    for (
         ppCurrNode = ppNodeStart, pCurrEntry = pEntryStart;
         ppCurrNode < ppNodeEnd;
         ppCurrNode++, pCurrEntry++
        )
        pCurrEntry->pNode = *ppCurrNode;

    xSep = checkSeparation (splitInfo.pAxisEntries, splitInfo.numEntries, X_AXIS);
    ySep = checkSeparation (splitInfo.pAxisEntries, splitInfo.numEntries, Y_AXIS);

    optimalSplit = (ySep > xSep) ? Y_AXIS : X_AXIS;

    pNewNode = initNode (pNode->header.type);

    pNode->nEntries = 0;
    range_init (&pNode->header.range);

    for (i=0, pCurrEntry=pEntryStart; pCurrEntry < pEntryEnd; pCurrEntry++, i++)
        {
        if (splitInfo.pAxisEntries[i].groupNumber[optimalSplit] == 0)
            {
            addChild (pNode, pCurrEntry->pNode);
            extendNodeRange (pNode, &pCurrEntry->pNode->header.range);
            }
        else
            {
            addChild (pNewNode, pCurrEntry->pNode);
            extendNodeRange (pNewNode, &pCurrEntry->pNode->header.range);
            }
        }
    insertEntry ((XYRangeTreeNode *)pNode->header.pParent, pNewNode);
    free (splitInfo.pAxisEntries);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    insertEntry
(
XYRangeTreeNode *pNode,
XYRangeTreeNode *pNewNode
)
    {
    if (!range_isContained (&pNewNode->header.range, &pNode->header.range))
        {
        extendNodeRange (pNode, &pNewNode->header.range);
        increaseInteriorRange ((XYRangeTreeNode *)pNode->header.pParent, &pNode->header.range);
        }

    if (pNode->header.type == NODETYPE_Top)
        {
        XYRangeTreeNode    *pNewParent = initNode (NODETYPE_Interior), *pCurrentChild = getChild (pNode, 0);

        pNewParent->header.pParent = pNode;
        setChild (pNode, 0, pNewParent);
        insertEntry (pNewParent, pCurrentChild);
        insertEntry (pNewParent, pNewNode);
        }
    else
        {
        addChild (pNode, pNewNode);
        if (pNode->nEntries >= MAX_ENTRIES)
            splitNode (pNode);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static double  rangeIncrementFit
(
XYRangeTreeNode *pNode,
HideTreeRange        *pAddRange
)
    {
    double      newExtent, increment;
    HideTreeRange    newRange;

    range_combineRange (&newRange, pAddRange, &pNode->header.range);
    newExtent = extentSquared (&newRange);

    increment = newExtent - pNode->extentSquared;

    /* "thisFit" is a somewhat arbitrary measure of how well the range fits into this
       node, taking into account the total size ("new extent") of this node plus this range,
       plus a penalty for increasing it from its existing size. */
    return newExtent + increment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static XYRangeTreeNode *chooseLeaf
(
XYRangeTreeNode *pTree,
HideTreeRange           *pRange
)
    {
    if (NODETYPE_Leaf == pTree->header.type)
        {
        return pTree;
        }
    else
        {
        double          thisFit, bestFit = DBL_MAX;
        XYRangeTreeNode **ppNodeStart, **ppNodeEnd, **ppCurrNode, *pBest = NULL;

        ppNodeStart = (XYRangeTreeNode **) pTree->entries;
        ppNodeEnd  = ppNodeStart + pTree->nEntries;

        for (ppCurrNode = ppNodeStart; ppCurrNode < ppNodeEnd; ppCurrNode++)
            {
            thisFit = rangeIncrementFit (*ppCurrNode, pRange);
            if (ppCurrNode == ppNodeStart || thisFit < bestFit)
                {
                pBest = *ppCurrNode;
                bestFit = thisFit;
                }
            }
        return chooseLeaf (pBest, pRange);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool    rgXYRangeTree_initializeTree
(
void            **ppTree
)
    {
    XYRangeTreeNode         *pLeaf = initNode (NODETYPE_Leaf);
    XYRangeTreeNode         *pTree = initNode (NODETYPE_Top);

    pTree->nEntries = 0;
    pTree->extentSquared = 0.0;
    addChild (pTree, pLeaf);

    *ppTree = (void *)pTree;

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool       rgXYRangeTree_addToTree
(
XYRangeTreeNode *pTree,
double          xMin,
double          yMin,
double          xMax,
double          yMax,
int             userData
)
    {
    NodeHeader *pDI = (NodeHeader *)calloc (1, sizeof(NodeHeader));
    _Analysis_assume_(pDI != nullptr);

    XYRangeTreeNode *pLeaf;

    pDI->range.xmin = (xyDataTyp) xMin;
    pDI->range.ymin = (xyDataTyp) yMin;
    pDI->range.xmax = (xyDataTyp) xMax;
    pDI->range.ymax = (xyDataTyp) yMax;
    pDI->userData = userData;
    pDI->type = NODETYPE_Leaf;

    pLeaf = chooseLeaf (pTree, &pDI->range);

    if (NULL != pLeaf)
        insertEntry (pLeaf, (XYRangeTreeNode *) pDI);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void rgXYRangeTree_destroyRangeTree
(
XYRangeTreeNode **treePP
)
    {
    int i;
    XYRangeTreeNode         *pNode = *treePP, *pChild;

    if (pNode->header.type < NODETYPE_Leaf)
        {
        // NODETYPE_Top and NODETYPE_Interior.  Recurse to children before freeing here.
        for (i = 0; NULL != (pChild = getChild (pNode, i++));)
            rgXYRangeTree_destroyRangeTree (&pChild);
        }
    else if (pNode->header.type == NODETYPE_Leaf)
        {
        // NODETYPE_Leaf.  Bad name, that.  It is indeed a leaf of the structural part,
        // but still has data hanging below.  Free the data directly.
        for (i = 0; NULL != (pChild = getChild (pNode, i++));)
            free (pChild);
        }

    free (pNode);

    *treePP = NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    traverseTree
(
XYRangeTreeNode                 *pTree,
HideTree_NodeFunction            nodeFunction,
void                    *nodeFunctionArgP,
HideTree_ProcessLeafFunction     processLeaf,
void                    *geometryFunctionArgP
)
    {
    XYRangeTreeNode    *pNode = pTree;

    if (pNode->nEntries &&
        (NULL == nodeFunction || (*nodeFunction)(&pNode->header.range, pNode, nodeFunctionArgP)))
        {
        XYRangeTreeNode **ppNodeStart, **ppNodeEnd, **ppCurrNode;

        ppNodeStart = (XYRangeTreeNode **) pNode->entries;
        ppNodeEnd  = ppNodeStart + pNode->nEntries;

        if (NODETYPE_Leaf == pNode->header.type)
            {
            for (ppCurrNode = ppNodeStart; ppCurrNode < ppNodeEnd; ppCurrNode++)
                {
                XYRangeTreeNode *pLeaf = *ppCurrNode;
                if  (NULL == nodeFunction || (*nodeFunction)(&pLeaf->header.range, pNode, nodeFunctionArgP))
                    {
                    if (!(*processLeaf)
                                (
                                &pLeaf->header.range,
                                pLeaf->header.userData,
                                geometryFunctionArgP,
                                pTree,
                                pLeaf
                                ))
                        return false;               }
                }
            }
        else
            {
            for (ppCurrNode = ppNodeStart; ppCurrNode < ppNodeEnd; ppCurrNode++)
                {
                if (!traverseTree (*ppCurrNode, nodeFunction, nodeFunctionArgP, processLeaf, geometryFunctionArgP))
                    return false;
                }
            }
        }

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void rgXYRangeTree_getLeafRange
(
void        *pVoidLeaf,
DRange3d    *pRange
)
    {
    XYRangeTreeNode *pLeaf = (XYRangeTreeNode *)pVoidLeaf;
    pRange->InitFrom(
            pLeaf->header.range.xmin,
            pLeaf->header.range.ymin,
            0.0,
            pLeaf->header.range.xmax,
            pLeaf->header.range.ymax,
            0.0
            );
    }

/*---------------------------------------------------------------------------------**//**
* Traverse a tree, applying functions to subtrees. A NULL node function accepts all subtrees. Returns AND of all leaf-level processLeaf calls.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool             rgXYRangeTree_traverseTree
(
void                    *pTree,
HideTree_NodeFunction            nodeFunction,          /* => If NULL, all subtrees are traversed.
                                                      true from nodeFunction allows subtree
                                                        to be processed.   false skips this
                                                        subtree but continues with remainder of tree */
void                    *nodeFunctionArgP,
HideTree_ProcessLeafFunction     processLeaf,
void                    *geometryFunctionArgP
)
    {
    if  ((pTree==NULL) || (processLeaf==NULL))
            return  true;

    return traverseTree ((XYRangeTreeNode *)pTree, nodeFunction, nodeFunctionArgP, processLeaf, geometryFunctionArgP);

    }

/*---------------------------------------------------------------------------------**//**
* Use this function as the HideTree_NodeFunction on a range search. Returns true if there is any overlap between the test range and the node
* range.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool             rgXYRangeTree_nodeFunction_testForRangeOverlap
(
HideTreeRange           *pNodeRange,
void                    *pNode,
HideTreeRange           *pTestRange
)
    {
    if (   pTestRange->xmin > pNodeRange->xmax
        || pTestRange->xmax < pNodeRange->xmin
        || pTestRange->ymin > pNodeRange->ymax
        || pTestRange->ymax < pNodeRange->ymin
        )
        return  false;
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Use this function as the HideTree_NodeFunction on a range search. Returns true if the node range is completely contained in the test test
* range.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool             rgXYRangeTree_nodeFunction_testForNodeRangeInTestRange
(
HideTreeRange           *pNodeRange,
void                    *pNode,
HideTreeRange           *pTestRange
)
    {
    if (   pTestRange->xmin <= pNodeRange->xmin
        && pTestRange->xmax >= pNodeRange->xmax
        && pTestRange->ymin <= pNodeRange->ymin
        && pTestRange->ymax >= pNodeRange->ymax
        )
        return  true;
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* Use this function as the HideTree_NodeFunction on a range search. Returns true if the node range is completely contained in the test test
* range.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool             rgXYRangeTree_nodeFunction_testForTestRangeAllInNodeRange
(
HideTreeRange           *pNodeRange,
void                    *pNode,
HideTreeRange           *pTestRange
)
    {
    if (   pTestRange->xmin >= pNodeRange->xmin
        && pTestRange->xmax <= pNodeRange->xmax
        && pTestRange->ymin >= pNodeRange->ymin
        && pTestRange->ymax <= pNodeRange->ymax
        )
        return  true;
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void             rgXYRangeTree_initRange
(
HideTreeRange           *pRange,
double                  xmin,
double                  ymin,
double                  xmax,
double                  ymax

)
    {
    pRange->xmin = xmin;
    pRange->ymin = ymin;
    pRange->xmax = xmax;
    pRange->ymax = ymax;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void             rgXYRangeTree_initRangeFromDRange3d
(
HideTreeRange           *pRange,
DRange3d                *pRange3d
)
    {
    pRange->xmin = pRange3d->low.x;
    pRange->ymin = pRange3d->low.y;
    pRange->xmax = pRange3d->high.x;
    pRange->ymax = pRange3d->high.y;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void rgXYRangeTree_drawTree
(
void            *pTree,
int             displayMode,
int             color
)
    {
#if defined(debug)
    int         i;
    XYRangeTreeNode    *pNode = pTree, *pChild;

    debug_displayRange ((DVector3d *) &pNode->header.range, displayMode, color);

    for (i=0; NULL != (pChild = getChild (pNode, i));  i++)
        {
        if (NODETYPE_Leaf == pNodeitem.type)
            debug_displayRange ((DVector3d *) &pChild->header.range, displayMode, color + 1);
        else
            rgXYRangeTree_drawTree (pChild, displayMode, color + 1);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    printLabel
(
char    *labelP,
int     indent
)
    {
    int         i;

    GEOMAPI_PRINTF ("%s", labelP);
    for (i=0; i<indent; i++)
        GEOMAPI_PRINTF ("  ");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void  rgXYRangeTree_dumpTree
(
XYRangeTreeNode *pTree,
int             indent,
char            *labelP
)
    {
    int         i;
    XYRangeTreeNode    *pNode = pTree, *pChild;

    printLabel (labelP, indent);

    switch (pNode->header.type)
        {
        case NODETYPE_Top:
                GEOMAPI_PRINTF ("Top\n");
                break;

        case NODETYPE_Interior:
                GEOMAPI_PRINTF ("Interior, %d Entries (%g,%g) to (%g,%g)\n",
                        pNode->nEntries,
                        pNode->header.range.xmin,
                        pNode->header.range.ymin,
                        pNode->header.range.xmax,
                        pNode->header.range.ymax);
                break;

        case NODETYPE_Leaf:
                GEOMAPI_PRINTF ("Leaf, %d Entries (%g,%g) to (%g,%g)\n",
                        pNode->nEntries,
                        pNode->header.range.xmin,
                        pNode->header.range.ymin,
                        pNode->header.range.xmax,
                        pNode->header.range.ymax);
                break;
        }

    for (i=0; NULL != (pChild = getChild (pNode, i));  i++)
        {
        if (NODETYPE_Leaf == pNode->header.type)
            {
            printLabel (labelP, indent+1);
            }
        else
            {
            rgXYRangeTree_dumpTree (pChild, indent+1, labelP);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void rgXYRangeTree_displayStatistics
(
void
)
    {
#if defined (DEBUG)
    /* debug_printf ("          Max Leaf Nodes: %d\n", sMaxNodeEntries); */
#endif
    }
END_BENTLEY_GEOMETRY_NAMESPACE
