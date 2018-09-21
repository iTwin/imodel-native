#include "ScalableMeshPCH.h"
#include "ScalableMesh.h"
#include "InternalUtilityFunctions.h"
#include "ScalableMeshProgressiveQuery.h"
#include "ScalableMeshQuery.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshQuadTreeQueries.h"
#include "ScalableMeshProgressiveQueryPlanner.h"



BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
QueryPlan* QueryPlanner::CreatePlanForNextQuery(IScalableMeshDisplayCacheManagerPtr& displayCacheManager, bset<uint64_t>& activeClips) const
{
    return new QueryPlan(displayCacheManager, activeClips);
}

void QueryPlanner::FetchOverviewsAndPlanNextQuery(RequestedQuery& query, QueryPlan& nextQueryPlan, ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP,size_t& currentInd, ProducedNodeContainer<DPoint3d, Extent3dType>& nodesToSearch, ProducedNodeContainer<DPoint3d, Extent3dType>& foundNodes) const
{

    bvector<IScalableMeshCachedDisplayNodePtr>                     lowerResOverviewNodes;
    bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>> searchingNodes;
    bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>> toLoadNodes;

    ComputeOverviewSearchToLoadNodes(query, lowerResOverviewNodes, searchingNodes, toLoadNodes, nodesToSearch, currentInd, foundNodes, nextQueryPlan.m_activeClips, query.m_meshToQuery, nextQueryPlan.m_displayCacheManagerPtr);

    //assert(lowerResOverviewNodes.size() > 0 || (nodesToSearch.GetNodes().size() - currentInd - 1) == 0);

    query.m_overviewMeshNodes.insert(query.m_overviewMeshNodes.end(), lowerResOverviewNodes.begin(), lowerResOverviewNodes.end());

    if (!s_preloadInQueryThread)
    {
        if (toLoadNodes.size() > 0 && (query).m_loadTexture)
        {
            ScalableMesh<DPoint3d>* smP(dynamic_cast<ScalableMesh<DPoint3d>*>(query.m_meshToQuery.get()));

            ScalableMeshProgressiveQueryEngine::PreloadData(smP, toLoadNodes, true);
        }
    }

    if (s_loadNodeNearCamFirst && toLoadNodes.size() > 0)
    {
        vector<size_t> queryNodeOrder;
        bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>> unorderedLoadNodes;

        unorderedLoadNodes.insert(unorderedLoadNodes.end(), toLoadNodes.begin(), toLoadNodes.end());

        queryObjectP->GetQueryNodeOrder(queryNodeOrder, toLoadNodes[0], &toLoadNodes[0], toLoadNodes.size());

        assert(queryNodeOrder.size() == toLoadNodes.size());

        toLoadNodes.clear();

        for (auto& order : queryNodeOrder)
        {
            toLoadNodes.push_back(unorderedLoadNodes[order]);
        }
    }

    nextQueryPlan.m_toLoadNodes = toLoadNodes;
    nextQueryPlan.m_searchingNodes = searchingNodes;
    nextQueryPlan.m_queryId = query.m_queryId;
    nextQueryPlan.m_loadTexture = query.m_loadTexture;
    nextQueryPlan.m_meshToQuery = query.m_meshToQuery;
}

void QueryPlanner::AddQueriesInPlan(QueryPlan& nextQueryPlan, ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP, QueryProcessor& queryProcessor, IScalableMeshViewDependentMeshQueryParamsPtr queryParam, RequestedQuery& query) const
{
    queryProcessor.AddQuery(nextQueryPlan.m_queryId, queryObjectP, nextQueryPlan.m_searchingNodes, nextQueryPlan.m_toLoadNodes, nextQueryPlan.m_loadTexture, nextQueryPlan.m_activeClips, nextQueryPlan.m_meshToQuery, nextQueryPlan.m_displayCacheManagerPtr);
}

END_BENTLEY_SCALABLEMESH_NAMESPACE
