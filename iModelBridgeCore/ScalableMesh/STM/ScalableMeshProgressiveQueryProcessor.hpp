#pragma once
#include "ScalableMeshProgressiveContourQueryPlanner.h"
#include "ScalableMeshProgressiveQueryProcessor.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


extern bool s_preloadInQueryThread;
extern bool s_useIntermediateOverviews;

template <class POINT, class EXTENT>  RefCountedPtr<ProcessingQuery<POINT, EXTENT>> ProcessingQuery<POINT, EXTENT>::Create(int                                               queryId,
    int                                               nbWorkingThreads,
    ISMPointIndexQuery<POINT, EXTENT>*                queryObjectP,
    bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>& searchingNodes,
    bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>& toLoadNodes,
    bool                                              loadTexture,
    const bset<uint64_t>&                              clipVisibilities,
    IScalableMeshPtr&                                  scalableMeshPtr,
    IScalableMeshDisplayCacheManagerPtr&               displayCacheManagerPtr,
    typename ProcessingQuery<POINT, EXTENT>::Type                              type)
    
        {
        if (type == ProcessingQuery<POINT, EXTENT>::Type::Contour)
            return new ContourProcessingQuery<POINT, EXTENT>(queryId, nbWorkingThreads, queryObjectP, searchingNodes, toLoadNodes, loadTexture, clipVisibilities, scalableMeshPtr, displayCacheManagerPtr);
        else
            return new ProcessingQuery<POINT, EXTENT>(queryId, nbWorkingThreads, queryObjectP, searchingNodes, toLoadNodes, loadTexture, clipVisibilities, scalableMeshPtr, displayCacheManagerPtr);
        }

template <class POINT, class EXTENT>  void ProcessingQuery<POINT,EXTENT>::Run(size_t threadInd, QueryProcessor& processor)
{
    if (s_preloadInQueryThread)
    {
        bool doPreLoad = (((ScalableMesh<DPoint3d>*)m_scalableMeshPtr.get())->GetMainIndexP()->IsTextured() == SMTextureType::Streaming) && m_loadTexture;

        if (m_toLoadNodes[threadInd].size() > 0 && doPreLoad)
        {
            ScalableMeshProgressiveQueryEngine::PreloadData((ScalableMesh<DPoint3d>*)m_scalableMeshPtr.get(), m_toLoadNodes[threadInd], false);
        }
    }

    HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> nodePtr;

    //processingQueryPtr->m_searchingNodeMutexes[threadId].lock();

    if (m_searchingNodes[threadInd].size() > 0)
    {
        nodePtr = m_searchingNodes[threadInd].back();
        //processingQueryPtr->m_searchingNodes[threadId].pop_back();                    
    }

    //processingQueryPtr->m_searchingNodeMutexes[threadId].unlock();

    if (nodePtr != 0)
    {

        if (s_useIntermediateOverviews)
        {
            /* First query a better overview*/
            ProducedNodeContainer<DPoint3d, Extent3dType> producedOverviewNodes;
            m_nodeQueryProcessorMutexes[threadInd].lock();
            m_nodeQueryProcessors[threadInd] = NodeQueryProcessor<DPoint3d, Extent3dType>::Create(nodePtr, m_queryObjectP, 0, m_loadTexture, m_scalableMeshPtr->ShouldInvertClips(), &producedOverviewNodes, threadInd, m_clipVisibilities, nodePtr->GetLevel() + 2);
            m_nodeQueryProcessorMutexes[threadInd].unlock();
            if (!m_isCancel)
            {
                m_nodeQueryProcessors[threadInd]->DoQuery();
                HFCPtr<SMPointIndexNode<POINT, EXTENT>> node = 0;
                while (producedOverviewNodes.ConsumeNode(node))
                {

                    IScalableMeshCachedDisplayNodePtr meshNodePtr;

                    processor.LoadNodeDisplayData(meshNodePtr, node, m_loadTexture, m_clipVisibilities, m_scalableMeshPtr, m_displayCacheManagerPtr);
                    OnLoadedOverviewNode(meshNodePtr, threadInd);
                    node = 0;
                }
                m_nodeQueryProcessorMutexes[threadInd].lock();
                m_nodeQueryProcessors[threadInd] = 0;
                m_nodeQueryProcessorMutexes[threadInd].unlock();
            }
        }

        ProducedNodeContainer<DPoint3d, Extent3dType> producedFoundNodes;
        m_nodeQueryProcessorMutexes[threadInd].lock();

        bool doPreLoad = (((ScalableMesh<DPoint3d>*)m_scalableMeshPtr.get())->GetMainIndexP()->IsTextured() == SMTextureType::Streaming) && m_loadTexture;

        if (doPreLoad)
        {
            m_nodeQueryProcessors[threadInd] = NodeQueryProcessor<DPoint3d, Extent3dType>::Create(nodePtr, m_queryObjectP, 0, m_loadTexture, m_scalableMeshPtr->ShouldInvertClips(), &producedFoundNodes, threadInd, m_clipVisibilities);
        }
        else
        {
            m_nodeQueryProcessors[threadInd] = NodeQueryProcessor<DPoint3d, Extent3dType>::Create(nodePtr, m_queryObjectP, 0, m_loadTexture, m_scalableMeshPtr->ShouldInvertClips(), &m_producedFoundNodes, threadInd, m_clipVisibilities);
        }
        m_nodeQueryProcessorMutexes[threadInd].unlock();

        m_searchingNodeMutexes[threadInd].lock();
        m_searchingNodes[threadInd].pop_back();
        m_searchingNodeMutexes[threadInd].unlock();

        if (!m_isCancel)
        {
            m_nodeQueryProcessors[threadInd]->DoQuery();

            if (producedFoundNodes.GetNodes().size() > 0 && doPreLoad)
            {
                ScalableMeshProgressiveQueryEngine::PreloadData((ScalableMesh<DPoint3d>*)m_scalableMeshPtr.get(), producedFoundNodes.GetNodes(), false);

                for (auto& node : producedFoundNodes.GetNodes())
                {
                    m_producedFoundNodes.AddNode(node);
                }
            }

            m_nodeQueryProcessorMutexes[threadInd].lock();
            m_nodeQueryProcessors[threadInd] = 0;
            m_nodeQueryProcessorMutexes[threadInd].unlock();
        }

        return;
    }

    //Load unloaded node
    //HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> nodePtr;                

    clock_t startT = clock();
    size_t nNodes = m_toLoadNodes[threadInd].size();
    while (m_toLoadNodes[threadInd].size() > 0)
    {
        if (m_toLoadNodes[threadInd].size() > 0)
        {
            nodePtr = m_toLoadNodes[threadInd].back();
        }

        if (nodePtr != 0)
        {
            IScalableMeshCachedDisplayNodePtr meshNodePtr;

            processor.LoadNodeDisplayData(meshNodePtr, nodePtr, m_loadTexture, m_clipVisibilities, m_scalableMeshPtr, m_displayCacheManagerPtr);
            OnLoadedMeshNode(meshNodePtr, threadInd);
        }
    }
    double elapsed = ((double)clock() - startT) / CLOCKS_PER_SEC * 1000.0;
    TRACEPOINT(THREAD_ID(), EventType::QUERY_LOADNODELIST, threadInd, (uint64_t)-1, -1, -1, elapsed, (uint32_t)nNodes)

    size_t m_nbMissed = 0;
    static size_t MAX_MISSED = 5;

    //NEED_WORK_SM : Part of node query processor;
    while (m_nbMissed < MAX_MISSED && !m_isCancel)
    {
        HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> consumedNodePtr;

        if (!m_producedFoundNodes.WaitConsumption())
        {
            m_nbMissed++;
            continue;
        }

        m_toLoadNodeMutexes[threadInd].lock();

        bool result = m_producedFoundNodes.ConsumeNode(consumedNodePtr);

        if (result)
        {
            m_toLoadNodes[threadInd].push_back(consumedNodePtr);
            m_toLoadNodeMutexes[threadInd].unlock();

#ifdef DISPLAYLOG
            fprintf(logger.GetFile(), "Consumed node : %I64d\n", consumedNodePtr->GetBlockID().m_integerID);
            fflush(logger.GetFile());
#endif
            //NEED_WORK_SM : Maybe will lead to too much wait for other query when multiples queries
            m_nbMissed = 0;

            IScalableMeshCachedDisplayNodePtr meshNodePtr;

            processor.LoadNodeDisplayData(meshNodePtr, consumedNodePtr, m_loadTexture, m_clipVisibilities, m_scalableMeshPtr, m_displayCacheManagerPtr);

            OnLoadedMeshNode(meshNodePtr, threadInd);
        }
        else
        {
            m_toLoadNodeMutexes[threadInd].unlock();
        }
    }

}
END_BENTLEY_SCALABLEMESH_NAMESPACE
