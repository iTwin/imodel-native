/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "Regions/rg_intern.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static int s_counter;

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void     jmdlRGI_set
(
RG_Intersection     *pIntersection, /* <= structure to be filled */
      int           gType,          /* => geometry type bits */
      int           pType,          /* => parse type bits */
      MTGNodeId    nodeId,    /* => node at start of edge */
      double        param,
      MTGNodeId    mergeNodeId,
      int           clusterIndex,
      MTGNodeId    seedNodeId
)
    {
    memset (pIntersection, 0, sizeof (RG_Intersection));
    pIntersection->type     = RGI_ASSEMBLE_MASK (pType, gType);
    pIntersection->nodeId   = nodeId;
    pIntersection->mergedNodeId = mergeNodeId;
    pIntersection->param    = param;
    pIntersection->label    = s_counter++;
    pIntersection->clusterIndex = clusterIndex;
    pIntersection->seedNodeId  = seedNodeId;
    }

/*---------------------------------------------------------------------------------**//**
* @return true if an intersection record was returned.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRGI_get
(
RG_IntersectionList *pIL,
RG_Intersection     *pIntersection,
int                 index
)
    {
    static int s_flagLabel = -1;
    static int s_dummy;
    bool    myStat = pIL->GetIntersection ((size_t)index, *pIntersection);
    if (myStat && s_flagLabel == pIntersection->label)
        {
        s_dummy++;
        }
    return myStat;
    }

/*---------------------------------------------------------------------------------**//**
* @return the number of intersection descriptors in the list.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public int             jmdlRGIL_getCount
(
RG_IntersectionList             *pRGIL
)
    {
    return (int)pRGIL->GetIntersectionSize ();
    }

/*---------------------------------------------------------------------------------**//**
* Add a pair of linked intersection records.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void     jmdlRGIL_addPair
(
RG_IntersectionList *pIL,
int                 gType0,
int                 pType0,
MTGNodeId           nodeId0,
double              param0,
MTGNodeId           mergeNodeId0,
int                 gType1,
int                 pType1,
MTGNodeId           nodeId1,
double              param1,
MTGNodeId           mergeNodeId1
)
    {
    RG_Intersection descr[2];

    int clusterIndex = pIL->NewClusterIndex ();

    jmdlRGI_set (&descr[0], gType0, pType0, nodeId0, param0, mergeNodeId0, clusterIndex, MTG_NULL_NODEID);
    jmdlRGI_set (&descr[1], gType1, pType1, nodeId1, param1, mergeNodeId1, clusterIndex, MTG_NULL_NODEID);

    pIL->AddIntersection (descr[0]);
    pIL->AddIntersection (descr[1]);
    }

/*---------------------------------------------------------------------------------**//**
* Add a pair of linked intersection records.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void     jmdlRGIL_addSingleton
(
RG_IntersectionList *pIL,
int                 gType0,
int                 pType0,
MTGNodeId           nodeId0,
double              param0,
MTGNodeId           mergeNodeId0
)
    {
    RG_Intersection descr[2];

    int clusterIndex = pIL->NewClusterIndex ();

    jmdlRGI_set (&descr[0], gType0, pType0, nodeId0, param0, mergeNodeId0, clusterIndex, MTG_NULL_NODEID);
    pIL->AddIntersection (descr[0]);
    }

/*---------------------------------------------------------------------------------**//**
* Merge the clusters for the intersection descriptors with given indices.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_mergeClustersOf
(
RG_IntersectionList *pIL,
int                 index0,
int                 index1
)
    {
    pIL->MergeClusters ((size_t)index0, (size_t)index1);
    }

/*---------------------------------------------------------------------------------**//**
* Set the seed nodes on intersection descriptors with indices index0 <= i < index1
* @param index0 => first index to set
* @param index1 => one past last index to set
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_setSeeds
(
RG_IntersectionList *pIL,
int                 index0,
int                 index1,
MTGNodeId          seedNodeId
)
    {
    for (int i = index0; i < index1; i++)
        {
        pIL->SetSeedNodeIdAt ((size_t)i, seedNodeId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @return the resolved cluster index of the given descriptor. The resolved cluster
*   index is saved in the descriptor.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public int     jmdlRGIL_resolveClusterOf
(
RG_IntersectionList *pIL,
int                 intersectionIndex
)
    {
    return pIL->ResolveClusterIndexAt ((size_t)intersectionIndex);
    }

/*---------------------------------------------------------------------------------**//**
*
* Initialize an intersectionlist
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_init
(
RG_IntersectionList *pIL
)
    {
    pIL->ClearAll ();   // ASSUME -- caller goes through normal constructor sequence !!!
    s_counter = 0;
    }

/*---------------------------------------------------------------------------------**//**
*
* Empty an already-initialized intersection list.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_empty
(
RG_IntersectionList *pIL
)
    {
    pIL->ClearAll ();
    }

/*---------------------------------------------------------------------------------**//**
*
* Free the secondary memory allocated to an intersection list.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_releaseMem
(
RG_IntersectionList *pIL
)
    {
    // destructor does it...
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_declareIsolatedBreak
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
      MTGNodeId    sNodeId,
      double        s               /* => parameter on curve */
)
    {
    jmdlRGIL_addSingleton (pIL,
                RGI_MASK_SIMPLE, RGI_MASK_SINGLE_PARAM,
                sNodeId, s, MTG_NULL_NODEID);
    }


/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_declareSimpleIntersection
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
      MTGNodeId    sNodeId,
      double        s,              /* => parameter on first curve */
      MTGNodeId    tNodeId,
      double        t               /* => parameter on second curve */
)
    {
    jmdlRGIL_addPair (pIL,
                RGI_MASK_SIMPLE, RGI_MASK_SINGLE_PARAM,
                sNodeId, s, MTG_NULL_NODEID,
                RGI_MASK_SIMPLE, RGI_MASK_SINGLE_PARAM,
                tNodeId, t, MTG_NULL_NODEID);
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_declareVertexOnSegment
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
      MTGNodeId    edgeBaseNodeId0,
      int           where0,
      MTGNodeId    edgeBaseNodeId1,
      double        t               /* => parameter on second curve */
)
    {
    MTGNodeId mergedNodeId0;
    int gType0;
    double s;

    /* EDL Sept 2000 -- this function is called from transverse
        intersectors.   We hope the proximity tester will
        be better --- use this static to run away. */
    static bool    s_ignoreThisCall = true;
    if (s_ignoreThisCall)
            return;


    if (where0 == 0)
        {
        mergedNodeId0 = edgeBaseNodeId0;
        gType0  = RGI_MASK_START_POINT;
        s       = 0.0;
        }
    else
        {
        mergedNodeId0 = jmdlMTGGraph_getEdgeMate (pRG->pGraph, edgeBaseNodeId0);
        gType0 = RGI_MASK_END_POINT;
        s       = 1.0;
        }

    jmdlRGIL_addPair (pIL,
                gType0, RGI_MASK_SINGLE_PARAM,
                edgeBaseNodeId0, s, mergedNodeId0,
                RGI_MASK_SIMPLE, RGI_MASK_SINGLE_PARAM,
                edgeBaseNodeId1, t, MTG_NULL_NODEID);
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_declareVertexOnSegmentByProximityTest
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
      MTGNodeId    edgeBaseNodeId0,
      int           where0,
      MTGNodeId    edgeBaseNodeId1,
      double        t               /* => parameter on second curve */
)
    {
    MTGNodeId mergedNodeId0;
    int gType0;
    double s;
    if (where0 == 0)
        {
        mergedNodeId0 = edgeBaseNodeId0;
        gType0  = RGI_MASK_START_POINT;
        s       = 0.0;
        }
    else
        {
        mergedNodeId0 = jmdlMTGGraph_getEdgeMate (pRG->pGraph, edgeBaseNodeId0);
        gType0 = RGI_MASK_END_POINT;
        s       = 1.0;
        }

    jmdlRGIL_addPair (pIL,
                gType0, RGI_MASK_SINGLE_PARAM,
                edgeBaseNodeId0, s, mergedNodeId0,
                RGI_MASK_SIMPLE, RGI_MASK_SINGLE_PARAM,
                edgeBaseNodeId1, t, MTG_NULL_NODEID);
    }



/*---------------------------------------------------------------------------------**//**
*
* Add intersection records for coincident vertices.  HOWEVER ... test for common
* vertex id, and do NOT add if already identical.
* @param where0 => 0,1 end id on edge0
* @param where1 => 0,1 end id on edge1
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_declareVertexOnVertex
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
MTGNodeId           edgeBaseNodeId0,
int                 where0,
MTGNodeId           edgeBaseNodeId1,
int                 where1
)
    {
    double      s;
    double      t;
    MTGNodeId  nodeId0, nodeId1;
    int         gType0, gType1;
    int         vertex0Index;
    int         vertex1Index;
    /* EDL: I don't know why addPair was not called when same vertex.
        Maybe I thought this case could safely be ignored. */
    static      int s_acceptAll = 1;

    if (where0 == 0)
        {
        nodeId0 = edgeBaseNodeId0;
        gType0  = RGI_MASK_START_POINT;
        s       = 0.0;
        }
    else
        {
        nodeId0 = jmdlMTGGraph_getEdgeMate (pRG->pGraph, edgeBaseNodeId0);
        gType0 = RGI_MASK_END_POINT;
        s       = 1.0;
        }


    if (where1 == 0)
        {
        nodeId1 = edgeBaseNodeId1;
        gType1 = RGI_MASK_START_POINT;
        t       = 0.0;
        }
    else
        {
        nodeId1 = jmdlMTGGraph_getEdgeMate (pRG->pGraph, edgeBaseNodeId1);
        gType1 = RGI_MASK_END_POINT;
        t       = 1.0;
        }


    jmdlRG_getVertexData (pRG, NULL, 0, &vertex0Index, nodeId0, 0.0);
    jmdlRG_getVertexData (pRG, NULL, 0, &vertex1Index, nodeId1, 0.0);

    if (vertex0Index != vertex1Index || s_acceptAll)
        {
        jmdlRGIL_addPair (pIL,
                gType0, RGI_MASK_SINGLE_PARAM,
                edgeBaseNodeId0, s, nodeId0,
                gType1, RGI_MASK_SINGLE_PARAM,
                edgeBaseNodeId1, t, nodeId1);
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_addCloseApproachPoint
(
RG_IntersectionList *pIL,
      MTGNodeId    edgeBaseNodeId0,
      double        s,              /* => parameter on first curve */
const DPoint3d      *pSPoint,       /* => point on first curve */
      MTGNodeId    edgeBaseNodeId1,
      double        t,              /* => parameter on second curve */
const DPoint3d      *pTPoint        /* => point on second curve */
)
    {
    }


/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRGI_declareShortEdge
(
RG_Header                       *pRG,
RG_IntersectionList             *pIL,
MTGNodeId                       nodeId
)
    {
    }

static int jmdlRGIL_compareByNodeIdAndParameter
(
const RG_Intersection     *pA,
const RG_Intersection     *pB
)
    {
    if  (pA->nodeId < pB->nodeId)
        {
        return  -1;
        }
    else if  (pA->nodeId > pB->nodeId)
        {
        return   1;
        }
    else if  (pA->param < pB->param)
        {
        return  -1;
        }
    else if (pA->param > pB->param)
        {
        return   1;
        }

    return 0;
    }

static int jmdlRGIL_compareByClusterId
(
const RG_Intersection     *pA,
const RG_Intersection     *pB
)
    {
    if  (pA->clusterIndex       <   pB->clusterIndex)
        {
        return  -1;
        }
    else if  (pA->clusterIndex  >   pB->clusterIndex)
        {
        return   1;
        }

    return 0;
    }

static int jmdlRGIL_compareBySeedNodeId
(
const RG_Intersection     *pA,
const RG_Intersection     *pB
)
    {
    if  (pA->seedNodeId       <   pB->seedNodeId)
        {
        return  -1;
        }
    else if  (pA->seedNodeId  >   pB->seedNodeId)
        {
        return   1;
        }

    return 0;
    }
/*---------------------------------------------------------------------------------**//**
* Lexical sort by (nodeId, parameter)
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_sortByNodeIdAndParameter
(
RG_IntersectionList *pIL
)
    {
    pIL->SortIntersections ((VBArray_SortFunction)jmdlRGIL_compareByNodeIdAndParameter);
    }

/*---------------------------------------------------------------------------------**//**
* Lexical sort by (clusterid)
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_sortByClusterId
(
RG_IntersectionList *pIL
)
    {
    pIL->SortIntersections ((VBArray_SortFunction)jmdlRGIL_compareByClusterId);
    }


/*---------------------------------------------------------------------------------**//**
* Lexical sort by (seedNodeId)
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGIL_sortBySeedNodeId
(
RG_IntersectionList *pIL
)
    {
    pIL->SortIntersections ((VBArray_SortFunction)jmdlRGIL_compareBySeedNodeId);
    }


/*---------------------------------------------------------------------------------**//**
* Fill pArray with the unique node ids from the intersection array.
*
* @return true if intersections computed successfully.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRGIL_getUniqueSeedNodeIdArray
(
RG_Header                       *pRG,
bvector<int>                *pArray,
RG_IntersectionList             *pRGIL
)
    {
    MTGNodeId currNodeId;
    pArray->clear ();
    jmdlRGIL_sortBySeedNodeId (pRGIL);

    currNodeId = MTG_NULL_NODEID;
    MTGNodeId newSeedNodeId;
    for (size_t i = 0; pRGIL->GetSeedNodeIdAt (i, newSeedNodeId); i++)
        {
        if (newSeedNodeId != currNodeId && newSeedNodeId != MTG_NULL_NODEID)
            {
            currNodeId = newSeedNodeId;
            jmdlEmbeddedIntArray_addInt(pArray, currNodeId);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Fill pArray with the unique node ids from the intersection array.
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRGIL_getUniqueNodeIdArray
(
RG_Header                       *pRG,
bvector<int>                *pArray,
RG_IntersectionList             *pRGIL
)
    {
    MTGNodeId currNodeId, newNodeId;

    jmdlRGIL_sortByNodeIdAndParameter (pRGIL);
    pArray->clear ();

    currNodeId = MTG_NULL_NODEID;
    for (size_t i = 0; pRGIL->GetNodeIdAt (i, newNodeId); i++)
        {
        if (newNodeId != currNodeId && newNodeId != MTG_NULL_NODEID)
            {
            currNodeId = newNodeId;
            jmdlEmbeddedIntArray_addInt(pArray, currNodeId);
            }
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE
