/*--------------------------------------------------------------------------------------+
|    $RCSfile: ScalableMeshProgressiveQueryPlanner.h,v $
|   $Revision: 1.20 $
|       $Date: 2018/08/15 14:07:12 $
|     $Author: Elenie.Godzaridis $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ScalableMesh/IScalableMeshProgressiveQuery.h>
#include <ScalableMesh/IScalableMeshClipContainer.h>
#include <ScalableMesh/IScalableMesh.h>
#include "ScalableMeshProgressiveQueryProcessor.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
typedef DRange3d Extent3dType;

extern bool s_loadNodeNearCamFirst;
extern bool s_preloadInQueryThread;

struct RequestedQuery
{
    RequestedQuery()
    {
        //m_queryObjectP = 0;
    }

    ~RequestedQuery()
    {
        /*
        if (m_queryObjectP != 0)
        {
        delete m_queryObjectP;
        m_queryObjectP = 0;
        }
        */
    }

    int                                                          m_queryId;
    bvector<IScalableMeshCachedDisplayNodePtr>                                m_overviewMeshNodes;
    bvector<IScalableMeshCachedDisplayNodePtr>                                m_requiredMeshNodes;
    IScalableMeshPtr m_meshToQuery;
    //ISMPointIndexQuery<ISMStore::Point3d64f, Extent3dType>* m_queryObjectP;    
    bool                                                         m_isQueryCompleted;
    bool                                                         m_fetchLastCompletedNodes;
    bool                                                         m_loadTexture;
    bvector<bool>                                                m_clipVisibilities;
    bool m_loadContours;
};

struct QueryPlan
{
    bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>> m_searchingNodes;
    bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>> m_toLoadNodes;
    int                                                          m_queryId;
    bool                                                         m_loadTexture;
    bool                                                         m_loadContours;
    IScalableMeshDisplayCacheManagerPtr&                   m_displayCacheManagerPtr;
    bset<uint64_t>&                                        m_activeClips;
    IScalableMeshPtr m_meshToQuery;
    bvector<RefCountedPtr<ScalableMeshCachedDisplayNode<DPoint3d>>>              m_overviewsToUpdate;

    QueryPlan(IScalableMeshDisplayCacheManagerPtr& displayCacheManager, bset<uint64_t>& activeClips)
        :m_displayCacheManagerPtr(displayCacheManager), m_activeClips(activeClips)
    {

    }
};

struct QueryPlanner
{
public:
    virtual QueryPlan* CreatePlanForNextQuery(IScalableMeshDisplayCacheManagerPtr& displayCacheManager, bset<uint64_t>& activeClips) const;
    virtual void FetchOverviewsAndPlanNextQuery(RequestedQuery& query, QueryPlan& nextQueryPlan, ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP, size_t& currentInd, ProducedNodeContainer<DPoint3d, Extent3dType>& nodesToSearch, ProducedNodeContainer<DPoint3d, Extent3dType>& foundNodes) const;
    virtual void AddQueriesInPlan(QueryPlan& nextQueryPlan, ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP, QueryProcessor& queryProcessor, IScalableMeshViewDependentMeshQueryParamsPtr viewParams, RequestedQuery& query) const;
    virtual void AddDirtyOverviews(QueryPlan& nextQueryPlan, bvector<RefCountedPtr<ScalableMeshCachedDisplayNode<DPoint3d>>>& dirtyOverviews);
};

void ComputeOverviewSearchToLoadNodes(RequestedQuery&                                            newQuery,
    bvector<IScalableMeshCachedDisplayNodePtr>&                lowerResOverviewNodes,
    bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& searchingNodes,
    bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& toLoadNodes,
    ProducedNodeContainer<DPoint3d, Extent3dType>&             nodesToSearch,
    size_t                                                     currentInd,
    ProducedNodeContainer<DPoint3d, Extent3dType>&             foundNodes,
    bset<uint64_t>&                                            activeClips,
    IScalableMeshPtr&                                          scalableMeshPtr,
    IScalableMeshDisplayCacheManagerPtr&                       displayCacheManagerPtr);

END_BENTLEY_SCALABLEMESH_NAMESPACE
