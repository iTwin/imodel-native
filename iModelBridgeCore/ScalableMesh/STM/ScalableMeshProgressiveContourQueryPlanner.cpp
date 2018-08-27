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


void ContourQueryPlanner::AddQueriesInPlan(QueryPlan& nextQueryPlan, ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP, QueryProcessor& queryProcessor) const
{
    auto handle = queryProcessor.AddQuery(nextQueryPlan.m_queryId, queryObjectP, nextQueryPlan.m_searchingNodes, nextQueryPlan.m_toLoadNodes, nextQueryPlan.m_loadTexture, nextQueryPlan.m_activeClips, nextQueryPlan.m_meshToQuery, nextQueryPlan.m_displayCacheManagerPtr);
    bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>> emptyNodeSet;
    handle = queryProcessor.AddDependentQuery(handle, emptyNodeSet, emptyNodeSet, nextQueryPlan.m_loadTexture, ProcessingQuery<DPoint3d, Extent3dType>::Type::Contour);
    handle->m_alwaysReloadMeshNodes = true;
}


template <class POINT, class EXTENT> void ContourProcessingQuery<POINT,EXTENT>::Run(size_t threadInd, QueryProcessor& processor)
{
    HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> nodePtr;
    size_t idx = 0;
    while (m_toLoadNodes[threadInd].size() > 0)
    {
        IScalableMeshCachedDisplayNodePtr meshNodePtr;
        if (m_toLoadNodes[threadInd].size() > 0)
        {
            nodePtr = m_toLoadNodes[threadInd].back();
            idx = m_toLoadNodes[threadInd].size()-1;
            m_toLoadNodes[threadInd].pop_back();
        }
        if (nodePtr != 0)
        {
            m_foundMeshNodeMutexes[threadInd].lock();
            IScalableMeshCachedDisplayNodePtr displayNodePtr = m_foundMeshNodes[threadInd][idx];
            meshNodePtr = ScalableMeshContourCachedDisplayNode<POINT>::Create(displayNodePtr.get());
            m_foundMeshNodes[threadInd][idx] = meshNodePtr;
            m_foundMeshNodeMutexes[threadInd].unlock();

            ContoursParameters p;
            p.majorContourSpacing = 50.0;
            p.minorContourSpacing = 10.0;
            dynamic_cast<ScalableMeshContourCachedDisplayNode<POINT>*>(meshNodePtr.get())->ComputeContours(p);
        }
    }
}

template class ContourProcessingQuery<DPoint3d, Extent3dType>;

END_BENTLEY_SCALABLEMESH_NAMESPACE

