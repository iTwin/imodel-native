/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#if defined (jmdlXYRangeTree_INT)
#define xyDataTyp   int
#define xyMaxVal    INT_MAX
#define xyMinVal    INT_MIN
#else
#define xyDataTyp   double
#define xyMaxVal    DBL_MAX
#define xyMinVal    DBL_MIN
#endif
typedef struct
    {
    xyDataTyp  xmin;
    xyDataTyp  xmax;
    xyDataTyp  ymin;
    xyDataTyp  ymax;
    } HideTreeRange;

typedef bool        (*HideTree_ProcessLeafFunction)
    (
    HideTreeRange   *pLeafRange,
    int             userDataFromLeaf,
    void            *pAppContext,
    void            *pParent,
    void            *pLeaf
    );

typedef bool        (*HideTree_NodeFunction)
                        (HideTreeRange *pNodeRange,
                        void *pNode,
                        void *pAppData);

typedef bool        (*HideTree_SortFunction)(HideTreeRange *, HideTreeRange *, void *);

typedef struct _XYRangeTreeNode XYRangeTreeNode;
END_BENTLEY_GEOMETRY_NAMESPACE

#include "rg_rangeTree.fdf"



