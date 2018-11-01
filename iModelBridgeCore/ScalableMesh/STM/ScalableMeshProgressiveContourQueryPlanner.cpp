#include "ScalableMeshPCH.h"
#include "ScalableMesh.h"
#include "InternalUtilityFunctions.h"
#include "ScalableMeshProgressiveQuery.h"
#include "ScalableMeshQuery.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshQuadTreeQueries.h"
#include "ScalableMeshProgressiveQueryPlanner.h"
#include "ScalableMeshProgressiveQueryProcessor.hpp"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


void ContourQueryPlanner::AddQueriesInPlan(QueryPlan& nextQueryPlan, ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP, QueryProcessor& queryProcessor, IScalableMeshViewDependentMeshQueryParamsPtr queryParams, RequestedQuery& query) const
{
    auto handle = queryProcessor.AddQuery(nextQueryPlan.m_queryId, queryObjectP, nextQueryPlan.m_searchingNodes, nextQueryPlan.m_toLoadNodes, nextQueryPlan.m_loadTexture, nextQueryPlan.m_activeClips, nextQueryPlan.m_meshToQuery, nextQueryPlan.m_displayCacheManagerPtr);
    bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>> emptyNodeSet;
    handle = queryProcessor.AddDependentQuery(handle, emptyNodeSet, emptyNodeSet, nextQueryPlan.m_loadTexture, ProcessingQuery<DPoint3d, Extent3dType>::Type::Contour);
    handle->m_alwaysReloadMeshNodes = true;
    dynamic_cast<ContourProcessingQuery<DPoint3d, Extent3dType>*>(handle)->SetContourParam(queryParams->GetMajorContourInterval(), queryParams->GetMinorContourInterval());
    for (auto& node : query.m_requiredMeshNodes)
    {
        size_t threadInd = (&node - query.m_requiredMeshNodes.data()) % handle->m_toLoadNodes.size();
        handle->m_foundMeshNodeMutexes[threadInd].lock();
        handle->m_toLoadNodeMutexes[threadInd].lock();
        assert(handle->m_foundMeshNodes[threadInd].size() == handle->m_toLoadNodes[threadInd].size());
        handle->m_foundMeshNodes[threadInd].push_back(node);
        handle->m_toLoadNodes[threadInd].push_back(dynamic_cast<ScalableMeshNode<DPoint3d>*>(node.get())->GetNodePtr());
        handle->m_toLoadNodeMutexes[threadInd].unlock();
        handle->m_foundMeshNodeMutexes[threadInd].unlock();
    }
}


template <class POINT, class EXTENT> void ContourProcessingQuery<POINT,EXTENT>::Run(size_t threadInd, QueryProcessor& processor)
{
    HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> nodePtr;
    size_t idx = 0;
    m_toLoadNodeMutexes[threadInd].lock();
    while (m_toLoadNodes[threadInd].size() > 0)
    {
        IScalableMeshCachedDisplayNodePtr meshNodePtr;
            nodePtr = m_toLoadNodes[threadInd].front();
            m_toLoadNodeMutexes[threadInd].unlock();
            idx = m_nbOfFoundNodes[threadInd]++;
         m_foundMeshNodeMutexes[threadInd].lock();
            IScalableMeshCachedDisplayNodePtr displayNodePtr = m_foundMeshNodes[threadInd][idx];
            meshNodePtr = ScalableMeshContourCachedDisplayNode<POINT>::Create(displayNodePtr.get());
            m_foundMeshNodes[threadInd][idx] = meshNodePtr;
            //assert(dynamic_cast<ScalableMeshContourCachedDisplayNode<POINT>*>(m_foundMeshNodes[threadInd].front().get()) != 0);
            m_foundMeshNodeMutexes[threadInd].unlock();

            dynamic_cast<ScalableMeshContourCachedDisplayNode<POINT>*>(meshNodePtr.get())->ComputeContours(m_params);
        
            m_toLoadNodeMutexes[threadInd].lock();
            HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> tmpNode = m_toLoadNodes[threadInd].back();
            m_toLoadNodes[threadInd][m_toLoadNodes[threadInd].size() - 1] = m_toLoadNodes[threadInd][0];
            m_toLoadNodes[threadInd][0] = tmpNode;
            m_toLoadNodes[threadInd].pop_back();
        //assert(dynamic_cast<ScalableMeshContourCachedDisplayNode<POINT>*>(m_foundMeshNodes[threadInd].front().get()) != 0);
    }
    m_toLoadNodeMutexes[threadInd].unlock();
}

template class ContourProcessingQuery<DPoint3d, Extent3dType>;

END_BENTLEY_SCALABLEMESH_NAMESPACE

