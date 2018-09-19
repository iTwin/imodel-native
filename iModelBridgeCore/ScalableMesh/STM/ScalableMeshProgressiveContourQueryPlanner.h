/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshProgressiveContourQueryPlanner.h $
|    $RCSfile: ScalableMeshProgressiveContourQueryPlanner.h,v $
|   $Revision: 1.20 $
|       $Date: 2018/08/15 14:07:12 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include "ScalableMeshProgressiveQuery.h"
#include "ScalableMeshQuery.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshQuadTreeQueries.h"
#include "ScalableMeshProgressiveQueryPlanner.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
struct ContourQueryPlanner : public QueryPlanner
{
public:
    //virtual void FetchOverviewsAndPlanNextQuery(RequestedQuery& query, QueryPlan& nextQueryPlan, int& currentInd, ProducedNodeContainer<DPoint3d, Extent3dType>& nodesToSearch, ProducedNodeContainer<DPoint3d, Extent3dType>& foundNodes) const override;
    virtual void AddQueriesInPlan(QueryPlan& nextQueryPlan, ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP, QueryProcessor& queryProcessor, IScalableMeshViewDependentMeshQueryParamsPtr viewParams, RequestedQuery& query) const override;
};

template <class POINT, class EXTENT> struct ContourProcessingQuery : public ProcessingQuery<POINT, EXTENT>
{
    friend struct ProcessingQuery<POINT, EXTENT>;
private:
    ContoursParameters m_params;
    std::vector<int> m_nbOfFoundNodes;

    ContourProcessingQuery(int                                               queryId,
        int                                               nbWorkingThreads,
        ISMPointIndexQuery<POINT, EXTENT>*                queryObjectP,
        bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>& searchingNodes,
        bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>& toLoadNodes,
        bool                                              loadTexture,
        const bset<uint64_t>&                              clipVisibilities,
        IScalableMeshPtr&                              scalableMeshPtr,
        IScalableMeshDisplayCacheManagerPtr&               displayCacheManagerPtr) :
        ProcessingQuery<POINT, EXTENT>(queryId, nbWorkingThreads, queryObjectP, searchingNodes, toLoadNodes, loadTexture, clipVisibilities, scalableMeshPtr, displayCacheManagerPtr) {
        m_nbOfFoundNodes.resize(nbWorkingThreads, 0);
    }
public:
    virtual void Run(size_t threadInd, QueryProcessor& processor) override;

    void SetContourParam(double majorInterval, double minorInterval)
    {
        m_params.majorContourSpacing = majorInterval;
        m_params.minorContourSpacing = minorInterval;
    }
};

END_BENTLEY_SCALABLEMESH_NAMESPACE
