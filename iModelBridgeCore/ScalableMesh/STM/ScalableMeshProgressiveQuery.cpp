/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshProgressiveQuery.cpp $
|    $RCSfile: ScalableMeshPointQuery.cpp,v $
|   $Revision: 1.41 $
|       $Date: 2012/11/29 17:30:37 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#undef static_assert
#include <DgnPlatform/DgnPlatformLib.h>
#include "ImagePPHeaders.h"
//#define GPU
USING_NAMESPACE_IMAGEPP;

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include <GeoCoord/BaseGeoCoord.h>
#include "SMPointIndex.h"
#include "InternalUtilityFunctions.h"
#include <ScalableMesh/IScalableMeshClipContainer.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>

#include "ScalableMesh.h"
#include "ScalableMeshProgressiveQuery.h"
#include "ScalableMeshQuery.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshQuadTreeQueries.h"


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


#define PRINT_SMDISPLAY_MSG

#ifdef PRINT_SMDISPLAY_MSG
#define PRINT_MSG_IF(condition, ...) if(##condition##) printf(__VA_ARGS__);
#define PRINT_MSG(...) printf(__VA_ARGS__);
#else
#define PRINT_MSG_IF(condition, ...)
#define PRINT_MSG(...) 
#endif


#ifdef DISPLAYLOG

class Logger
    {
public:

    Logger()
    {
        m_file = fopen("D:\\MyDoc\\RM - SM - Sprint 8\\Display\\log\\progressifLog.txt", "w+");
    }

    ~Logger()
    {
        fclose(m_file);
    }

    FILE* GetFile()
    {
        return m_file;
    }

    FILE* m_file;
    };


static Logger logger;

#endif

//#define DEACTIVATE_THREADING

/*==================================================================*/
/*                   IScalableMeshProgressiveQueryEngine            */
/*==================================================================*/
BentleyStatus IScalableMeshProgressiveQueryEngine::GetOverviewNodes(bvector<IScalableMeshCachedDisplayNodePtr>& meshNodes,
    int                                         queryId)
    {
    return _GetOverviewNodes(meshNodes, queryId);
    }

BentleyStatus IScalableMeshProgressiveQueryEngine::GetRequiredNodes(bvector<IScalableMeshCachedDisplayNodePtr>& meshNodes,
                                                                    int                                         queryId)
    {
    return _GetRequiredNodes(meshNodes, queryId);
    }

BentleyStatus IScalableMeshProgressiveQueryEngine::ClearCaching(const bvector<DRange2d>* clearRanges, const IScalableMeshPtr& scalableMeshPtr)
    {
    return _ClearCaching(clearRanges, scalableMeshPtr);
    }

BentleyStatus IScalableMeshProgressiveQueryEngine::ClearCaching(const bvector<uint64_t>& clipIds, const IScalableMeshPtr& scalableMeshPtr)
    {
    return _ClearCaching(clipIds, scalableMeshPtr);
    }

void IScalableMeshProgressiveQueryEngine::SetActiveClips(const bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr)
    {
    return _SetActiveClips(activeClips, scalableMeshPtr);
    }

void IScalableMeshProgressiveQueryEngine::InitScalableMesh(IScalableMeshPtr& scalableMeshPtr)
    {
    return _InitScalableMesh(scalableMeshPtr);
    }


void IScalableMeshProgressiveQueryEngine::ClearOverviews(IScalableMesh* scalableMeshP)
    {
    return _ClearOverviews(scalableMeshP);
    }



BentleyStatus IScalableMeshProgressiveQueryEngine::StartQuery(int                                                                      queryId,
                                                              IScalableMeshViewDependentMeshQueryParamsPtr                             queryParam,
                                                              const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& startingNodes,
                                                              bool                                                                     loadTexture,
                                                              const bvector<bool>&                                                     clipVisibilities,
                                                              IScalableMeshPtr&                                                         smPtr)
    {
    return _StartQuery(queryId, queryParam, startingNodes, loadTexture, clipVisibilities, smPtr);
    }

BentleyStatus IScalableMeshProgressiveQueryEngine::StopQuery(int queryId)
    {
    return _StopQuery(queryId);
    }


bool IScalableMeshProgressiveQueryEngine::IsQueryComplete(int queryId)
    {
    return _IsQueryComplete(queryId);
    }

IScalableMeshProgressiveQueryEnginePtr IScalableMeshProgressiveQueryEngine::Create(IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr)
    {    
    return new ScalableMeshProgressiveQueryEngine(scalableMeshPtr, displayCacheManagerPtr);
    }

static bool s_keepSomeInvalidate = true; 
                                            
template <class POINT, class EXTENT> class NodeQueryProcessor : public RefCountedBase, public IStopQuery
    {
private:

    HFCPtr<SMPointIndexNode<POINT, EXTENT>>          m_queryNode;
    ISMPointIndexQuery<POINT, EXTENT>*               m_pQueryObject;
    StopQueryCallbackFP                              m_stopQueryCallbackFP;
    ProducedNodeContainer<POINT, EXTENT>*            m_foundNodesP;
    atomic<bool>                                     m_stopQuery;
    bool                                             m_loadTexture;
    int                                              m_threadId;



    NodeQueryProcessor(HFCPtr<SMPointIndexNode<POINT, EXTENT>> queryNode,
        ISMPointIndexQuery<POINT, EXTENT>*      queryObject,
        StopQueryCallbackFP                     stopQueryCallbackFP,
        bool                                    loadTexture,
        ProducedNodeContainer<POINT, EXTENT>*   foundNodesP,
        int                                     threadId)
        {
        m_queryNode = queryNode;
        m_pQueryObject = queryObject;
        m_stopQueryCallbackFP = stopQueryCallbackFP;
        m_stopQuery = false;
        m_loadTexture = loadTexture;
        m_threadId = threadId;
        m_foundNodesP = foundNodesP;
        }

public:

    typedef RefCountedPtr<NodeQueryProcessor> Ptr;

    virtual bool DoStop() const override
        {
        return m_stopQuery;
        }

    void DoQuery()
        {
        if (!m_stopQuery)
            m_queryNode->Query(m_pQueryObject, *m_foundNodesP, this);
        }

    void SetStopQuery(bool stopQuery)
        {
        m_stopQuery = stopQuery;
        }
    
    static Ptr Create(HFCPtr<SMPointIndexNode<POINT, EXTENT>> queryNode,
        ISMPointIndexQuery<POINT, EXTENT>*      queryObject,
        StopQueryCallbackFP                     stopQueryCallbackFP,
        bool                                    loadTexture,
        ProducedNodeContainer<POINT, EXTENT>*   foundNodesP,
        int                                     threadId)
        {
        return new NodeQueryProcessor(queryNode, queryObject, stopQueryCallbackFP, loadTexture, foundNodesP, threadId);
        }
    };
   
  
template <class POINT, class EXTENT> struct ProcessingQuery : public RefCountedBase
    {
    typedef RefCountedPtr<ProcessingQuery<POINT, EXTENT>> Ptr;
      
    ProcessingQuery(int                                               queryId,
                    int                                               nbWorkingThreads,
                    ISMPointIndexQuery<POINT, EXTENT>*                queryObjectP,
                    bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>& searchingNodes,
                    bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>& toLoadNodes,
                    bool                                              loadTexture, 
                    const bset<uint64_t>&                              clipVisibilities,
                    IScalableMeshPtr&                              scalableMeshPtr,
                    IScalableMeshDisplayCacheManagerPtr&               displayCacheManagerPtr)
        : m_producedFoundNodes(true),
          m_clipVisibilities(clipVisibilities)
        {
        m_queryId = queryId;
        m_searchingNodes.resize(nbWorkingThreads);
        m_scalableMeshPtr = scalableMeshPtr;
        m_displayCacheManagerPtr = displayCacheManagerPtr;

        for (size_t nodeId = 0; nodeId < searchingNodes.size(); nodeId++)
            {
            m_searchingNodes[nodeId % nbWorkingThreads].push_back(searchingNodes[nodeId]);
            }

        m_toLoadNodes.resize(nbWorkingThreads);

        for (size_t nodeId = 0; nodeId < toLoadNodes.size(); nodeId++)
            {
            m_toLoadNodes[nodeId % nbWorkingThreads].push_back(toLoadNodes[nodeId]);
            }

        m_searchingNodeMutexes = new std::mutex[nbWorkingThreads];        
        m_toLoadNodeMutexes = new std::mutex[nbWorkingThreads];
        m_foundMeshNodes.resize(nbWorkingThreads);
        m_foundMeshNodeMutexes = new std::mutex[nbWorkingThreads];
        m_nodeQueryProcessors.resize(nbWorkingThreads);
        m_nodeQueryProcessorMutexes = new std::mutex[nbWorkingThreads];
                
        m_queryObjectP = queryObjectP;
        m_isCancel = false;        
        m_loadTexture = loadTexture;        
        }

    ~ProcessingQuery()
        {
        delete[] m_searchingNodeMutexes;
        delete[] m_toLoadNodeMutexes;
        delete[] m_foundMeshNodeMutexes;
        delete[] m_nodeQueryProcessorMutexes;
        delete m_queryObjectP;
        }

    bool IsComplete()
        {        
        size_t threadInd;

        for (threadInd = 0; threadInd < m_searchingNodes.size(); threadInd++)
            {
            m_searchingNodeMutexes[threadInd].lock();
            if (m_searchingNodes[threadInd].size() > 0)
                {
                m_searchingNodeMutexes[threadInd].unlock();
                break;
                }

            m_searchingNodeMutexes[threadInd].unlock();
            }

        if (threadInd != m_searchingNodes.size())
            return false;

        for (threadInd = 0; threadInd < m_nodeQueryProcessors.size(); threadInd++)
            {            
            m_nodeQueryProcessorMutexes[threadInd].lock();
            if (m_nodeQueryProcessors[threadInd] != 0)
                {
                m_nodeQueryProcessorMutexes[threadInd].unlock();
                break;
                }

            m_nodeQueryProcessorMutexes[threadInd].unlock();
            }
        
        if (threadInd != m_nodeQueryProcessors.size())
            return false;

        if (m_producedFoundNodes.WaitConsumption())
            return false;        
        
        for (threadInd = 0; threadInd < m_toLoadNodes.size(); threadInd++)
            {
            m_toLoadNodeMutexes[threadInd].lock();
            if (m_toLoadNodes[threadInd].size() > 0)
                {
                m_toLoadNodeMutexes[threadInd].unlock();
                break;
                }

            m_toLoadNodeMutexes[threadInd].unlock();
            }

        if (threadInd != m_toLoadNodes.size())
            return false;

        return true;
        }

    bool IsComplete(size_t threadInd)
        {                        
        m_searchingNodeMutexes[threadInd].lock();
        if (m_searchingNodes[threadInd].size() > 0)
            {
            m_searchingNodeMutexes[threadInd].unlock();
            return false;            
            }

        m_searchingNodeMutexes[threadInd].unlock();        

        m_nodeQueryProcessorMutexes[threadInd].lock();
        if (m_nodeQueryProcessors[threadInd] != 0)
            {
            m_nodeQueryProcessorMutexes[threadInd].unlock();
            return false;            
            }

        m_nodeQueryProcessorMutexes[threadInd].unlock();
                        
        if (m_producedFoundNodes.WaitConsumption())
            return false;        
        
        
        m_toLoadNodeMutexes[threadInd].lock();
        if (m_toLoadNodes[threadInd].size() > 0)
            {
            m_toLoadNodeMutexes[threadInd].unlock();
            return false;
            }

        m_toLoadNodeMutexes[threadInd].unlock();
        
        if (threadInd != m_toLoadNodes.size())
            return false;

        return true;
        }

    static Ptr Create(int                                               queryId,
                      int                                               nbWorkingThreads,
                      ISMPointIndexQuery<POINT, EXTENT>*                queryObjectP,
                      bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>& searchingNodes,
                      bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>& toLoadNodes,
                      bool                                              loadTexture, 
                      const bset<uint64_t>&                              clipVisibilities,
                      IScalableMeshPtr&                                  scalableMeshPtr,
                      IScalableMeshDisplayCacheManagerPtr&               displayCacheManagerPtr
                      )
        {
        return new ProcessingQuery(queryId, nbWorkingThreads, queryObjectP, searchingNodes, toLoadNodes, loadTexture, clipVisibilities, scalableMeshPtr, displayCacheManagerPtr);
        }

    int                                                       m_queryId;
    bvector<bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>> m_searchingNodes;
    std::mutex*                                               m_searchingNodeMutexes;
    bvector<bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>> m_toLoadNodes;
    std::mutex*                                               m_toLoadNodeMutexes;    
    ProducedNodeContainer<POINT, EXTENT>                      m_producedFoundNodes;
    bvector<bvector<IScalableMeshCachedDisplayNodePtr>>       m_foundMeshNodes;
    std::mutex*                                               m_foundMeshNodeMutexes;

    bvector<NodeQueryProcessor<DPoint3d, Extent3dType>::Ptr>  m_nodeQueryProcessors;
    std::mutex*                                                    m_nodeQueryProcessorMutexes;

    ISMPointIndexQuery<POINT, EXTENT>*  m_queryObjectP;    
    atomic<bool>                        m_isCancel;
    bool                                m_loadTexture;
    const bset<uint64_t>                                       m_clipVisibilities;
    IScalableMeshPtr                    m_scalableMeshPtr;
    IScalableMeshDisplayCacheManagerPtr m_displayCacheManagerPtr;
    };

//NEEDS_WORK_SM : Set to true it can lead to race condition, should be removed (and maybe m_areWorkingThreadRunning[threadId] too).
static bool s_delayJoinThread = true;

class QueryProcessor
    {
public:

private:

    typedef std::list<ProcessingQuery<DPoint3d, Extent3dType>::Ptr> ProcessingQueryList;

    bvector<int>                  m_processingQueryIndexes;
    ProcessingQueryList           m_processingQueries;
    std::mutex                    m_processingQueriesMutex;
    atomic<bool>                  m_run;

    int                           m_numWorkingThreads;
    std::thread*                  m_workingThreads;    
    atomic<bool>*                 m_areWorkingThreadRunning;    
       
    struct InLoadingNode;

    typedef RefCountedPtr<InLoadingNode> InLoadingNodePtr;

    struct InLoadingNode : public RefCountedBase
        {
        InLoadingNode(HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> visibleNode, 
                      IScalableMeshCachedDisplayNodePtr                     displayNode)
            {
            m_visibleNode = visibleNode;
            m_displayNode = displayNode; 
            }

        static InLoadingNodePtr Create(HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> visibleNode, 
                                       IScalableMeshCachedDisplayNodePtr                     displayNode)
            {
            return new InLoadingNode(visibleNode, displayNode);
            }

        HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> m_visibleNode;
        IScalableMeshCachedDisplayNodePtr                     m_displayNode;         
        };    

    std::mutex m_inLoadingNodeMutex;
    bvector<InLoadingNodePtr> m_inLoadingNodes;


    void LoadNodeDisplayData(IScalableMeshCachedDisplayNodePtr& meshNodePtr, HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>& visibleNode, bool loadTexture, const bset<uint64_t>& clipVisibilities, IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr)
        {
        InLoadingNodePtr inloadingNodePtr; 
        bool inLoading = false;

        while (!m_inLoadingNodeMutex.try_lock());                     
        for (auto& inLoadingNode : m_inLoadingNodes)
            {
            if (inLoadingNode->m_visibleNode.GetPtr() == visibleNode.GetPtr())
                {
                inloadingNodePtr = inLoadingNode;
                inLoading = true;
                }                        
            }

        if (inloadingNodePtr == 0)
            {
            inloadingNodePtr = InLoadingNode::Create(visibleNode, meshNodePtr);
            m_inLoadingNodes.push_back(inloadingNodePtr);
            }

        m_inLoadingNodeMutex.unlock();

        if (!inLoading)
            {                         
            ScalableMeshCachedDisplayNode<DPoint3d>* meshNode(ScalableMeshCachedDisplayNode<DPoint3d>::Create(visibleNode, scalableMeshPtr.get()));
                         

            TRACEPOINT(THREAD_ID(), EventType::EVT_CREATE_DISPLAY_LOAD, visibleNode->GetBlockID().m_integerID, (uint64_t)-1, dynamic_cast<SMMeshIndexNode<DPoint3d, Extent3dType>*>(visibleNode.GetPtr())->GetSingleTextureID(), -1, (uint64_t)meshNode, -1)


            if ((meshNode->IsLoaded(displayCacheManagerPtr.get()) == false && meshNode->IsLoadedInVRAM(displayCacheManagerPtr.get()) == false) || !meshNode->IsClippingUpToDate() || !meshNode->HasCorrectClipping(clipVisibilities))
                {
                if (!meshNode->IsDataUpToDate()) meshNode->UpdateData();
                meshNode->ApplyAllExistingClips();
                meshNode->RemoveDisplayDataFromCache();                    
                meshNode->LoadMesh(false, clipVisibilities, displayCacheManagerPtr, loadTexture, scalableMeshPtr->ShouldInvertClips());
                assert(meshNode->HasCorrectClipping(clipVisibilities));                 
                }

            meshNodePtr = meshNode;                                                

            inloadingNodePtr->m_displayNode = meshNodePtr;
            
            while (!m_inLoadingNodeMutex.try_lock());                     
            
            auto nodeIter = m_inLoadingNodes.begin();

            while (nodeIter != m_inLoadingNodes.end())
                {
                if (nodeIter->get() == inloadingNodePtr.get())                    
                    break;

                nodeIter++;
                }

            assert(nodeIter != m_inLoadingNodes.end());

            m_inLoadingNodes.erase(nodeIter);

            m_inLoadingNodeMutex.unlock();
            }
        else
            {
            while (inloadingNodePtr->m_displayNode == 0)
                {
                BeThreadUtilities::BeSleep(50);
                }

            meshNodePtr = inloadingNodePtr->m_displayNode;
            }
        }

    void QueryThread(DgnPlatformLib::Host* hostToAdopt, int threadId)
        {
        DgnPlatformLib::AdoptHost(*hostToAdopt);

        ProcessingQuery<DPoint3d, Extent3dType>::Ptr processingQueryPtr;

        do
            {            
            processingQueryPtr = nullptr;

            m_processingQueriesMutex.lock();
            //assert(m_processingQueries.size() <= 1);

            for (auto& processingQuery : m_processingQueries)
                {
                if (!processingQuery->IsComplete(threadId))
                    {
                    processingQueryPtr = processingQuery;
                    break;
                    }
                }            

            m_processingQueriesMutex.unlock();

            //NEEDS_WORK_MST : Maybe we should prioritize the first processing query found
            if (processingQueryPtr != 0)
                {
                HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> nodePtr;

                //processingQueryPtr->m_searchingNodeMutexes[threadId].lock();

                if (processingQueryPtr->m_searchingNodes[threadId].size() > 0)
                    {                    
                    nodePtr = processingQueryPtr->m_searchingNodes[threadId].back();
                    //processingQueryPtr->m_searchingNodes[threadId].pop_back();                    
                    }

                //processingQueryPtr->m_searchingNodeMutexes[threadId].unlock();

                if (nodePtr != 0)
                    {
                    processingQueryPtr->m_nodeQueryProcessorMutexes[threadId].lock();
                    processingQueryPtr->m_nodeQueryProcessors[threadId] = NodeQueryProcessor<DPoint3d, Extent3dType>::Create(nodePtr, processingQueryPtr->m_queryObjectP, 0, processingQueryPtr->m_loadTexture, &processingQueryPtr->m_producedFoundNodes, threadId);
                    processingQueryPtr->m_nodeQueryProcessorMutexes[threadId].unlock();

                    processingQueryPtr->m_searchingNodeMutexes[threadId].lock();                                    
                    processingQueryPtr->m_searchingNodes[threadId].pop_back();                                        
                    processingQueryPtr->m_searchingNodeMutexes[threadId].unlock();

                    if (!processingQueryPtr->m_isCancel)
                        {
                        processingQueryPtr->m_nodeQueryProcessors[threadId]->DoQuery();

                        processingQueryPtr->m_nodeQueryProcessorMutexes[threadId].lock();
                        processingQueryPtr->m_nodeQueryProcessors[threadId] = 0;
                        processingQueryPtr->m_nodeQueryProcessorMutexes[threadId].unlock();
                        }

                    continue;
                    }
                
                //Load unloaded node
                //HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> nodePtr;                
                if (processingQueryPtr->m_toLoadNodes[threadId].size() > 0)
                    {                    
                    nodePtr = processingQueryPtr->m_toLoadNodes[threadId].back();                    
                    }

                if (nodePtr != 0)
                    {       
                    IScalableMeshCachedDisplayNodePtr meshNodePtr;

                    LoadNodeDisplayData(meshNodePtr, nodePtr, processingQueryPtr->m_loadTexture, processingQueryPtr->m_clipVisibilities, processingQueryPtr->m_scalableMeshPtr, processingQueryPtr->m_displayCacheManagerPtr);                            
                          
                    processingQueryPtr->m_foundMeshNodeMutexes[threadId].lock();
                    processingQueryPtr->m_foundMeshNodes[threadId].push_back(meshNodePtr);
                    processingQueryPtr->m_foundMeshNodeMutexes[threadId].unlock();  

                    processingQueryPtr->m_toLoadNodeMutexes[threadId].lock();
                    processingQueryPtr->m_toLoadNodes[threadId].pop_back();                                        
                    processingQueryPtr->m_toLoadNodeMutexes[threadId].unlock();
                    }                   

                size_t m_nbMissed = 0;
                static size_t MAX_MISSED = 5;

                //NEED_WORK_SM : Part of node query processor;
                while (m_nbMissed < MAX_MISSED && !processingQueryPtr->m_isCancel)                            
                    {
                    HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> consumedNodePtr;

                    if (!processingQueryPtr->m_producedFoundNodes.WaitConsumption())
                        {
                        m_nbMissed++;
                        continue;
                        }
                                
                    processingQueryPtr->m_toLoadNodeMutexes[threadId].lock();

                    bool result = processingQueryPtr->m_producedFoundNodes.ConsumeNode(consumedNodePtr);

                    if (result)
                        {
                        processingQueryPtr->m_toLoadNodes[threadId].push_back(consumedNodePtr);                                        
                        processingQueryPtr->m_toLoadNodeMutexes[threadId].unlock();

#ifdef DISPLAYLOG
                        fprintf(logger.GetFile(), "Consumed node : %I64d\n", consumedNodePtr->GetBlockID().m_integerID);
                        fflush(logger.GetFile());                                        
#endif
                            //NEED_WORK_SM : Maybe will lead to too much wait for other query when multiples queries
                        m_nbMissed = 0;

                        IScalableMeshCachedDisplayNodePtr meshNodePtr;

                        LoadNodeDisplayData(meshNodePtr, consumedNodePtr, processingQueryPtr->m_loadTexture, processingQueryPtr->m_clipVisibilities, processingQueryPtr->m_scalableMeshPtr, processingQueryPtr->m_displayCacheManagerPtr);
                        
                        processingQueryPtr->m_foundMeshNodeMutexes[threadId].lock();
                        processingQueryPtr->m_foundMeshNodes[threadId].push_back(meshNodePtr);
                        processingQueryPtr->m_foundMeshNodeMutexes[threadId].unlock();

                        processingQueryPtr->m_toLoadNodeMutexes[threadId].lock();
                        processingQueryPtr->m_toLoadNodes[threadId].pop_back();                                        
                        processingQueryPtr->m_toLoadNodeMutexes[threadId].unlock();
                        }
                    else
                        {
                        processingQueryPtr->m_toLoadNodeMutexes[threadId].unlock();                    
                        }
                    }                
                }

            } while (m_run && (processingQueryPtr != 0));

        m_areWorkingThreadRunning[threadId] = false;
        }
         
public:

    QueryProcessor()
        {        
#ifndef DEACTIVATE_THREADING
        m_numWorkingThreads = std::thread::hardware_concurrency() - 2;
        m_numWorkingThreads = max(1, m_numWorkingThreads);
#else
        m_numWorkingThreads = 1;        
#endif
        m_workingThreads = new std::thread[m_numWorkingThreads];
        m_areWorkingThreadRunning = new std::atomic<bool>[m_numWorkingThreads];

        for (size_t ind = 0; ind < m_numWorkingThreads; ind++)
            m_areWorkingThreadRunning[ind] = false;
                
        m_run = false;
        m_processingQueryIndexes.resize(m_numWorkingThreads);
        }

    virtual ~QueryProcessor()
        {
        for (size_t threadInd = 0; threadInd < m_numWorkingThreads; threadInd++)
            {
            if (m_workingThreads[threadInd].joinable())
                m_workingThreads[threadInd].join();
            }

        delete[] m_workingThreads;
        delete[] m_areWorkingThreadRunning;
        }

    void AddQuery(int                                                             queryId,
                  ISMPointIndexQuery<DPoint3d, Extent3dType>*                queryObjectP,
                  bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& searchingNodes,
                  bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& toLoadNodes,                  
                  bool                                                            loadTexture, 
                  const bset<uint64_t>&                                            clipVisibilities,
                  IScalableMeshPtr&                                                scalableMeshPtr,
                  IScalableMeshDisplayCacheManagerPtr&               displayCacheManagerPtr)
        {

#ifdef DISPLAYLOG
            fprintf(logger.GetFile(), "START NEW QUERY\n");
            fflush(logger.GetFile());                                                    
#endif

        ProcessingQuery<DPoint3d, Extent3dType>::Ptr processingQueryPtr(ProcessingQuery<DPoint3d, Extent3dType>::Create(queryId, m_numWorkingThreads, queryObjectP, searchingNodes, toLoadNodes, loadTexture, clipVisibilities, scalableMeshPtr, displayCacheManagerPtr));

        size_t currentNbProcessingQueries;

        m_processingQueriesMutex.lock();

#ifndef NDEBUG
        for (auto& query : m_processingQueries)
            {
            if (query->m_queryId == queryId)
                assert(!"Query already processing");
            }
#endif

        currentNbProcessingQueries = m_processingQueries.size();
        m_processingQueries.push_back(processingQueryPtr);
        m_processingQueriesMutex.unlock();
        
        if (currentNbProcessingQueries == 0)
            {
            Stop();
            Start();
            }        
        }


    StatusInt CancelAllQueries()
        {
        ProcessingQuery<DPoint3d, Extent3dType>::Ptr toCancelQueryPtr;        

        m_processingQueriesMutex.lock();

        ProcessingQueryList::iterator queryIter(m_processingQueries.begin());
        ProcessingQueryList::iterator queryIterEnd(m_processingQueries.end());

        while (queryIter != queryIterEnd)
            {
            toCancelQueryPtr = *queryIter;
            queryIter = m_processingQueries.erase(queryIter);
            toCancelQueryPtr->m_isCancel = true;   

            for (size_t processorInd = 0; processorInd < toCancelQueryPtr->m_nodeQueryProcessors.size(); processorInd++)
                {
                toCancelQueryPtr->m_nodeQueryProcessorMutexes[processorInd].lock();
                if (toCancelQueryPtr->m_nodeQueryProcessors[processorInd] != 0)
                    {
                    toCancelQueryPtr->m_nodeQueryProcessors[processorInd]->SetStopQuery(true);
                    }
                toCancelQueryPtr->m_nodeQueryProcessorMutexes[processorInd].unlock();
                }            
            }

        m_processingQueriesMutex.unlock();

        for (size_t threadInd = 0; threadInd < m_numWorkingThreads; threadInd++)
            {
            if (m_workingThreads[threadInd].joinable())
                m_workingThreads[threadInd].join();
            }

        return SUCCESS;
        }
     
    StatusInt CancelQuery(int queryId)
        {              
#ifdef DISPLAYLOG 
        fprintf(logger.GetFile(), "STOP NEW QUERY\n");
        fflush(logger.GetFile());                                        
#endif

        ProcessingQuery<DPoint3d, Extent3dType>::Ptr toCancelQueryPtr;

        m_processingQueriesMutex.lock();

        ProcessingQueryList::iterator queryIter(m_processingQueries.begin());
        ProcessingQueryList::iterator queryIterEnd(m_processingQueries.end());

        while (queryIter != queryIterEnd)
            {
            if ((*queryIter)->m_queryId == queryId)
                {
                toCancelQueryPtr = *queryIter;
                m_processingQueries.erase(queryIter);
                toCancelQueryPtr->m_isCancel = true;
                break;
                }

            queryIter++;
            }

        m_processingQueriesMutex.unlock();

        if (toCancelQueryPtr != 0)
            {
            for (size_t processorInd = 0; processorInd < toCancelQueryPtr->m_nodeQueryProcessors.size(); processorInd++)
                {
                toCancelQueryPtr->m_nodeQueryProcessorMutexes[processorInd].lock();
                if (toCancelQueryPtr->m_nodeQueryProcessors[processorInd] != 0)
                    {
                    toCancelQueryPtr->m_nodeQueryProcessors[processorInd]->SetStopQuery(true);
                    }
                toCancelQueryPtr->m_nodeQueryProcessorMutexes[processorInd].unlock();
                }
            }

        return SUCCESS;
        }


        bool IsQueryComplete(int queryId) 
            {
            bool isQueryComplete = true;
            
            ProcessingQuery<DPoint3d, Extent3dType>::Ptr queryPtr;

            //m_processingQueriesMutex.lock();                    

            auto& queryIter(m_processingQueries.begin());     
            auto& queryIterEnd(m_processingQueries.end());     

            while (queryIter != queryIterEnd)
                {
                if ((*queryIter)->m_queryId == queryId)
                    {
                    queryPtr = *queryIter;                                        
                    break;
                    }

                queryIter++;
                }
            
            if (queryPtr != 0)
                {                
                isQueryComplete = queryPtr->IsComplete();
                }

            return isQueryComplete;
            }
            
        void Start()
            { 
            if (m_run == false)
                {
                m_run = true;

                //Launch a group of threads
                for (int threadId = 0; threadId < m_numWorkingThreads; ++threadId) 
                    {                                                        
                    if (!s_delayJoinThread)
                        {                
                        m_workingThreads[threadId] = std::thread(&QueryProcessor::QueryThread, this, DgnPlatformLib::QueryHost(), threadId);
                        }
                    else
                        {
                        if (m_areWorkingThreadRunning[threadId] == false)
                            {
                            if (m_workingThreads[threadId].joinable())                            
                                m_workingThreads[threadId].join();

                            m_workingThreads[threadId] = std::thread(&QueryProcessor::QueryThread, this, DgnPlatformLib::QueryHost(), threadId);
                            m_areWorkingThreadRunning[threadId] = true;
                            }
                        }
                    }
                }
            }
        
        void Stop()
            {                        
            m_run = false;

            if (!s_delayJoinThread)
                {                
                for (int threadId = 0; threadId < m_numWorkingThreads; ++threadId) 
                    {
                    if (m_workingThreads[threadId].joinable())
                        m_workingThreads[threadId].join();                    
                    }         
                }
            }
       
        StatusInt GetFoundNodes(bvector<IScalableMeshCachedDisplayNodePtr>& foundNodes, int queryId)
            {             
            StatusInt status;

            std::list<ProcessingQuery<DPoint3d, Extent3dType>::Ptr>::iterator queryItr(m_processingQueries.begin());
            std::list<ProcessingQuery<DPoint3d, Extent3dType>::Ptr>::iterator queryItrEnd(m_processingQueries.end());

            while (queryItr != queryItrEnd)
                {
                if ((*queryItr)->m_queryId == queryId)
                    {
                    break;
                    }
                ++queryItr;
                }

            if (queryItr != queryItrEnd)
                {                
                for (size_t threadIter = 0; threadIter < (*queryItr)->m_foundMeshNodes.size(); threadIter++)
                    {
                    (*queryItr)->m_foundMeshNodeMutexes[threadIter].lock();
#ifdef DISPLAYLOG                 
                    /*
                    fprintf(logger.GetFile(), "threadId %i : nb found nodes : %I64d\n", threadIter, (*queryItr)->m_foundMeshNodes[threadIter].size());
                    fflush(logger.GetFile());                    
                    */
#endif
                    foundNodes.insert(foundNodes.end(), (*queryItr)->m_foundMeshNodes[threadIter].begin(),(*queryItr)->m_foundMeshNodes[threadIter].end());
                    (*queryItr)->m_foundMeshNodes[threadIter].clear();
                    (*queryItr)->m_foundMeshNodeMutexes[threadIter].unlock();
                    }   

                status = SUCCESS;
                }
            else
                {
                status = ERROR;
                }             

            return status;
            }
    };

static QueryProcessor s_queryProcessor;

#define MAX_PRELOAD_OVERVIEW_LEVEL 1

void ScalableMeshProgressiveQueryEngine::UpdatePreloadOverview()
    {    
    for (auto& node : m_overviewNodes)
        {
        //Empty node are never loaded
        if (node->IsLoaded() == false) continue;
        
        if (!node->IsClippingUpToDate() || !node->HasCorrectClipping(m_activeClips))
            {
            node->ApplyAllExistingClips();
            node->RemoveDisplayDataFromCache();                    
            node->LoadMesh(false, m_activeClips, m_displayCacheManagerPtr, true, m_smOverviews[&node - &m_overviewNodes[0]]->ShouldInvertClips());
            assert(node->HasCorrectClipping(m_activeClips));                 
            }
        }        
    }

void ScalableMeshProgressiveQueryEngine::PreloadOverview(HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>& node, IScalableMesh* sMesh)
    {     
    if (std::find(m_smOverviews.begin(), m_smOverviews.end(), sMesh) != m_smOverviews.end()) return;
    ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNode(ScalableMeshCachedDisplayNode<DPoint3d>::Create(node, sMesh));
    assert(meshNode->IsLoaded(m_displayCacheManagerPtr.get()) == false);
    

    TRACEPOINT(THREAD_ID(), EventType::EVT_CREATE_DISPLAY_OVR_PRELOAD, node->GetBlockID().m_integerID, (uint64_t)-1, dynamic_cast<SMMeshIndexNode<DPoint3d, Extent3dType>*>(node.GetPtr())->GetSingleTextureID(), -1, (uint64_t)meshNode.get(), -1)


    meshNode->ApplyAllExistingClips();
    meshNode->RemoveDisplayDataFromCache();                    
    meshNode->LoadMesh(false, m_activeClips, m_displayCacheManagerPtr, true, sMesh->ShouldInvertClips());                               
    assert(meshNode->IsLoaded(m_displayCacheManagerPtr.get()) == false || meshNode->HasCorrectClipping(m_activeClips));

    m_overviewNodes.push_back(meshNode);
        
    if (meshNode->GetLevel() < MAX_PRELOAD_OVERVIEW_LEVEL)
        {                
        bvector<IScalableMeshNodePtr> childrenNodes(meshNode->GetChildrenNodes());

        for (auto& childNode : childrenNodes)
            {            
            HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> node;
            node = (dynamic_cast<ScalableMeshNode<DPoint3d>*>(childNode.get()))->GetNodePtr();            
            PreloadOverview(node, sMesh);
            }
        }     
        m_smOverviews.push_back(sMesh);
    }             

#define VALID_EXTENT_RATIO 0.5
#define MAX_LEVEL 4
static double s_validExtentRatio = 0;

void EstimateMeanNbPointsPerNode(int64_t& nbObjects, int64_t& nbNodes, HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>& node)
    {            						
	if (!node->IsEmpty())
		{
		double nodeExtentArea = (ExtentOp<Extent3dType>::GetXMax(node->GetNodeExtent()) - ExtentOp<Extent3dType>::GetXMin(node->GetNodeExtent())) *
							(ExtentOp<Extent3dType>::GetYMax(node->GetNodeExtent()) - ExtentOp<Extent3dType>::GetYMin(node->GetNodeExtent()));

		double contentExtentArea = (ExtentOp<Extent3dType>::GetXMax(node->GetContentExtent()) - ExtentOp<Extent3dType>::GetXMin(node->GetContentExtent())) *
							   (ExtentOp<Extent3dType>::GetYMax(node->GetContentExtent()) - ExtentOp<Extent3dType>::GetYMin(node->GetContentExtent()));
	

		if (contentExtentArea  / nodeExtentArea > s_validExtentRatio) 
			{
			nbObjects += node->GetNbObjects();
			nbNodes++;
			}
		}

	ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNode(ScalableMeshCachedDisplayNode<DPoint3d>::Create(node));
		                
    if (node->GetLevel() < MAX_LEVEL)
        {                
        bvector<IScalableMeshNodePtr> childrenNodes(meshNode->GetChildrenNodes());

        for (auto& childNode : childrenNodes)
            {            
			HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> node;
            node = (dynamic_cast<ScalableMeshNode<DPoint3d>*>(childNode.get()))->GetNodePtr();            
			EstimateMeanNbPointsPerNode(nbObjects, nbNodes, node);            
            }
        }            
    }      

static double s_minScreenPixelCorrectionFactor = 1.0;
 
ScalableMeshProgressiveQueryEngine::ScalableMeshProgressiveQueryEngine(IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr)
    {
    m_displayCacheManagerPtr = displayCacheManagerPtr;            
        
    HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> rootNodePtr(((ScalableMesh<DPoint3d>*)scalableMeshPtr.get())->GetRootNode());

    bvector<uint64_t> allShownIds;
    scalableMeshPtr->GetAllClipIds(allShownIds);

    bset<uint64_t> activeClips;
    for (auto&id : allShownIds) activeClips.insert(id);
    _SetActiveClips(activeClips, scalableMeshPtr);

    if (rootNodePtr == nullptr) return;
    PreloadOverview(rootNodePtr, scalableMeshPtr.get());       

	int64_t nbObjects = 0;
	int64_t nbNodes = 0;
	EstimateMeanNbPointsPerNode(nbObjects, nbNodes, rootNodePtr);

	if (nbNodes > 0)
		{
		s_minScreenPixelCorrectionFactor = ((double)nbObjects / nbNodes) / rootNodePtr->GetSplitTreshold();
		}
	else
		{
		s_minScreenPixelCorrectionFactor = 1.0;
		}
    }

ScalableMeshProgressiveQueryEngine::~ScalableMeshProgressiveQueryEngine()
    {    
    //NEEDS_WORK_SM : Need to cancel only for particular ScalableMesh
    s_queryProcessor.CancelAllQueries();    
    m_overviewNodes.clear();
    m_smOverviews.clear();
    }


template <class POINT> int BuildQueryObject(//ScalableMeshQuadTreeViewDependentMeshQuery<POINT, Extent3dType>* viewDependentQueryP,
    ISMPointIndexQuery<POINT, Extent3dType>*&                        pQueryObject,
    const DPoint3d*                                                       pQueryExtentPts,
    int                                                                   nbQueryExtentPts,
    IScalableMeshViewDependentMeshQueryParamsPtr                          queryParam,
    IScalableMesh*                                                        smP)
    {
    //MST More validation is required here.
    assert(queryParam != 0);

    int status = SUCCESS;

    Extent3dType queryExtent;
    /*
    Extent3dType contentExtent(m_scmIndexPtr->GetContentExtent());

    double minZ = ExtentOp<Extent3dType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<Extent3dType>::GetZMax(contentExtent);

    Extent3dType queryExtent(ScalableMeshPointQuery::GetExtentFromClipShape<Extent3dType>(pQueryExtentPts,
    nbQueryExtentPts,
    minZ,
    maxZ));
    */
    //MS Need to be removed
    double viewportRotMatrix[3][3];
    double rootToViewMatrix[4][4];

    memcpy(rootToViewMatrix, queryParam->GetRootToViewMatrix(), sizeof(double) * 4 * 4);
    
    ScalableMeshQuadTreeViewDependentMeshQuery<POINT, Extent3dType>* viewDependentQueryP = new ScalableMeshQuadTreeViewDependentMeshQuery<POINT, Extent3dType>(queryExtent,
        rootToViewMatrix,
        viewportRotMatrix,
        queryParam->GetViewBox(),
        false,
        queryParam->GetViewClipVector(),
        smP->ShouldInvertClips(),
        100000000);

    // viewDependentQueryP->SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 17\\STM\\Bad Resolution Selection\\visitingNodes.xml"));

    viewDependentQueryP->SetMeanScreenPixelsPerPoint(queryParam->GetMinScreenPixelsPerPoint() * s_minScreenPixelCorrectionFactor);

    viewDependentQueryP->SetMaxPixelError(queryParam->GetMaxPixelError());

    //MS : Might need to be done at the ScalableMeshReprojectionQuery level.    
    if ((queryParam->GetSourceGCS() != 0) && (queryParam->GetTargetGCS() != 0))
        {
        BaseGCSCPtr sourcePtr = queryParam->GetSourceGCS();
        BaseGCSCPtr targetPtr = queryParam->GetTargetGCS();
        viewDependentQueryP->SetReprojectionInfo(sourcePtr, targetPtr);
        }   


    pQueryObject = (ISMPointIndexQuery<POINT, Extent3dType>*)(viewDependentQueryP);

    return status;
    }

#ifndef NDEBUG
static double s_firstNodeSearchingDelay = (double)1 / 15 * CLOCKS_PER_SEC;
#else
static double s_firstNodeSearchingDelay = (double)1 / 10 * CLOCKS_PER_SEC;
#endif
//static int    s_nbIterClock;

void FindOverview(bvector<IScalableMeshCachedDisplayNodePtr>& lowerResOverviewNodes, DRange3d& extentToCover, HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>& node, bool loadTexture, const bset<uint64_t>& clipVisibilities, IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr)
    {    
    assert(node->IsParentSet() == true);
        
    HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> parentNodePtr(node->GetParentNodePtr());
    assert(parentNodePtr != node);
    
    if (parentNodePtr == nullptr)
        {        
        //assert(!"Should not occurs");               
        return;
        }
    

    ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNodePtr(ScalableMeshCachedDisplayNode<DPoint3d>::Create(parentNodePtr, scalableMeshPtr.get()));


    TRACEPOINT(THREAD_ID(), EventType::EVT_CREATE_DISPLAY_OVR_1, parentNodePtr->GetBlockID().m_integerID,(uint64_t)-1, dynamic_cast<SMMeshIndexNode<DPoint3d, Extent3dType>*>(parentNodePtr.GetPtr())->GetSingleTextureID(), -1, (uint64_t)meshNodePtr.get(), -1)

    
    if (!meshNodePtr->IsLoadedInVRAM(displayCacheManagerPtr.get()) || ((!meshNodePtr->IsClippingUpToDate() || !meshNodePtr->HasCorrectClipping(clipVisibilities)) && !s_keepSomeInvalidate))
        {        
        FindOverview(lowerResOverviewNodes, extentToCover/*meshNodePtr->GetContentExtent()*/, parentNodePtr, loadTexture, clipVisibilities, scalableMeshPtr, displayCacheManagerPtr);
        }
    else   
        {
        auto nodeIter = lowerResOverviewNodes.begin();

        while (nodeIter != lowerResOverviewNodes.end())
            {            
            if ((*nodeIter)->GetNodeId() == meshNodePtr->GetNodeId())
                break;            

            nodeIter++;
            }

        ClipVectorPtr clipVector;

        if (!extentToCover.IsEmpty())
            {
            CurveVectorPtr curvePtr = CurveVector::CreateRectangle(extentToCover.low.x, extentToCover.low.y, extentToCover.high.x, extentToCover.high.y, 0);
            ClipPrimitivePtr clipPrimitive = ClipPrimitive::CreateFromBoundaryCurveVector(*curvePtr, DBL_MAX, 0, 0, 0, 0, true);
            clipPrimitive->SetIsMask(false);
            clipVector = ClipVector::CreateFromPrimitive(clipPrimitive);
            }
        
        if (nodeIter == lowerResOverviewNodes.end())
            {                    
            if (clipVector.IsValid())
                meshNodePtr->AddClipVector(clipVector);    

            lowerResOverviewNodes.push_back(meshNodePtr);             
            }        
        else
            {          
            ScalableMeshCachedDisplayNode<DPoint3d>* displayNode(dynamic_cast<ScalableMeshCachedDisplayNode<DPoint3d>*>((*nodeIter).get()));

            if (clipVector.IsValid())
                displayNode->AddClipVector(clipVector);            
            }
        }
    }

static bool s_sortOverviewBySize = true; 

class NewQueryStartingNodeProcessor
    {
    private : 

        ProducedNodeContainer<DPoint3d, Extent3dType>* m_nodesToSearch;
        size_t                                              m_nodeToSearchCurrentInd;
        ProducedNodeContainer<DPoint3d, Extent3dType>* m_foundNodes;
        bool                                                m_loadTexture; 
        RequestedQuery*                                     m_newQuery;
        bset<uint64_t>*                                     m_activeClips;

    
        bvector<bvector<IScalableMeshCachedDisplayNodePtr>>                     m_lowerResOverviewNodes;
        bvector<bvector<IScalableMeshCachedDisplayNodePtr>>                     m_requiredMeshNodes;    
        bvector<bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>> m_toLoadNodes;
        
        int          m_numWorkingThreads;
        std::thread* m_workingThreads;    

    public : 

        NewQueryStartingNodeProcessor()
            {
#ifndef DEACTIVATE_THREADING
            m_numWorkingThreads = std::thread::hardware_concurrency() - 2;       
            m_numWorkingThreads = max(1, m_numWorkingThreads);
#else
            m_numWorkingThreads = 1;            
#endif
            m_lowerResOverviewNodes.resize(m_numWorkingThreads);
            m_requiredMeshNodes.resize(m_numWorkingThreads);        
            m_toLoadNodes.resize(m_numWorkingThreads);
            m_workingThreads = new std::thread[m_numWorkingThreads];
            }

        virtual ~NewQueryStartingNodeProcessor()
            {
            delete [] m_workingThreads;
            }

        void ClearNodeInfo()
            {
            for (size_t i = 0; i < m_numWorkingThreads; ++i)
                {
                m_lowerResOverviewNodes[i].clear();
                m_requiredMeshNodes[i].clear();
                m_toLoadNodes[i].clear();
                }
            }

        void QueryThread(/*DgnPlatformLib::Host* hostToAdopt,*/ size_t threadId, IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr)
            {       
            m_lowerResOverviewNodes[threadId].clear();
            m_toLoadNodes[threadId].clear();
            m_requiredMeshNodes[threadId].clear();
            for (size_t nodeInd = m_nodeToSearchCurrentInd + 1; nodeInd < m_nodesToSearch->GetNodes().size(); nodeInd++)        
                {                              
                if (nodeInd % m_numWorkingThreads != threadId) continue;

                if (!m_nodesToSearch->GetNodes()[nodeInd]->IsLoaded())
                    {
                    DRange3d range3d(DRange3d::NullRange());

                    FindOverview(m_lowerResOverviewNodes[threadId], range3d, m_nodesToSearch->GetNodes()[nodeInd], m_newQuery->m_loadTexture, *m_activeClips, scalableMeshPtr, displayCacheManagerPtr);
                    }
                else
                    {                
                    ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNodePtr(ScalableMeshCachedDisplayNode<DPoint3d>::Create(m_nodesToSearch->GetNodes()[nodeInd], scalableMeshPtr.get()));
              
                    if (!meshNodePtr->IsLoadedInVRAM(displayCacheManagerPtr.get()) || ((!meshNodePtr->IsClippingUpToDate() || !meshNodePtr->HasCorrectClipping(*m_activeClips)) && !s_keepSomeInvalidate))
                        {            
                        FindOverview(m_lowerResOverviewNodes[threadId], meshNodePtr->GetNodeExtent(), m_nodesToSearch->GetNodes()[nodeInd], m_newQuery->m_loadTexture, *m_activeClips, scalableMeshPtr, displayCacheManagerPtr);
                        }
                    else
                        {
                        m_lowerResOverviewNodes[threadId].push_back(meshNodePtr);
                        }
                    }
                }                    
        
            for (size_t nodeInd = 0; nodeInd < m_foundNodes->GetNodes().size(); nodeInd++)        
                {                      
                if (nodeInd % m_numWorkingThreads != threadId) continue;
                
                ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNodePtr(ScalableMeshCachedDisplayNode<DPoint3d>::Create(m_foundNodes->GetNodes()[nodeInd], scalableMeshPtr.get()));
                              
                if (!meshNodePtr->IsLoadedInVRAM(displayCacheManagerPtr.get()) || ((!meshNodePtr->IsClippingUpToDate() || !meshNodePtr->HasCorrectClipping(*m_activeClips)) && !s_keepSomeInvalidate))
                    {                
                    FindOverview(m_lowerResOverviewNodes[threadId], meshNodePtr->GetNodeExtent(), m_foundNodes->GetNodes()[nodeInd], m_newQuery->m_loadTexture, *m_activeClips, scalableMeshPtr, displayCacheManagerPtr);
                                        
                    m_toLoadNodes[threadId].push_back(m_foundNodes->GetNodes()[nodeInd]);
                    }
                else
                    {
                    //NEEDS_WORK_SM : Should not be duplicated.
                    m_lowerResOverviewNodes[threadId].push_back(meshNodePtr);
                    if (meshNodePtr->IsDataUpToDate() && meshNodePtr->IsClippingUpToDate() && meshNodePtr->HasCorrectClipping(*m_activeClips))
                        {                        
                        m_requiredMeshNodes[threadId].push_back(meshNodePtr);
                        }
                    else
                        {
                        m_toLoadNodes[threadId].push_back(m_foundNodes->GetNodes()[nodeInd]);
                        }
                    }
                }
            }
            
        void Execute(RequestedQuery&                                            newQuery, 
                     bvector<IScalableMeshCachedDisplayNodePtr>&                lowerResOverviewNodes,
                     bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& searchingNodes,
                     bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& toLoadNodes,                 
                     ProducedNodeContainer<DPoint3d, Extent3dType>&             nodesToSearch,
                     size_t                                                     nodeToSearchCurrentInd,
                     ProducedNodeContainer<DPoint3d, Extent3dType>&             foundNodes, 
                     bset<uint64_t>&                                            activeClips,
                     IScalableMeshPtr&                                          scalableMeshPtr, 
                     IScalableMeshDisplayCacheManagerPtr&                       displayCacheManagerPtr)
            {        
            m_nodesToSearch = &nodesToSearch;
            m_nodeToSearchCurrentInd = nodeToSearchCurrentInd;
            m_foundNodes = &foundNodes;        
            m_newQuery = &newQuery;
            m_activeClips = &activeClips;

            for (size_t threadId = 0; threadId < m_numWorkingThreads; ++threadId) 
                {                                                                                
                m_workingThreads[threadId] = std::thread(&NewQueryStartingNodeProcessor::QueryThread, this, threadId, scalableMeshPtr, displayCacheManagerPtr);
                }

            for (size_t threadInd = 0; threadInd < m_numWorkingThreads; threadInd++)
                {
                if (m_workingThreads[threadInd].joinable())
                    m_workingThreads[threadInd].join();

                m_newQuery->m_requiredMeshNodes.insert(m_newQuery->m_requiredMeshNodes.end(), m_requiredMeshNodes[threadInd].begin(), m_requiredMeshNodes[threadInd].end());                
                toLoadNodes.insert(toLoadNodes.end(), m_toLoadNodes[threadInd].begin(), m_toLoadNodes[threadInd].end());
                
                //Avoid duplicate
                for (auto& node : m_lowerResOverviewNodes[threadInd])
                    {
                    auto nodeIter = lowerResOverviewNodes.begin();

                    while (nodeIter != lowerResOverviewNodes.end())
                        {
                        if ((*nodeIter)->GetNodeId() == node->GetNodeId())
                            {
                            bvector<ClipVectorPtr> clipVectors;

                            node->GetDisplayClipVectors(clipVectors);
                            ScalableMeshCachedDisplayNode<DPoint3d>* displayNode(dynamic_cast<ScalableMeshCachedDisplayNode<DPoint3d>*>((*nodeIter).get()));
                            for (auto& clip : clipVectors)
                                {
                                displayNode->AddClipVector(clip);
                                }
                            break;
                            }

                        nodeIter++;
                        }

                    if (nodeIter == lowerResOverviewNodes.end())
                        {
                        lowerResOverviewNodes.push_back(node);
                        }
                    }                
                }                                                               
            }    
    };


//NEEDS_WORK_SM: needed to remove the global constructor, but this isn't that much cleaner
NewQueryStartingNodeProcessor* s_newQueryStartingNodeProcessor = nullptr;

void InitializeProgressiveQueries()
    {
    if (nullptr == s_newQueryStartingNodeProcessor)
        s_newQueryStartingNodeProcessor = new NewQueryStartingNodeProcessor();
    }

void ClearProgressiveQueriesInfo()
    {
    s_newQueryStartingNodeProcessor->ClearNodeInfo();
    }

void TerminateProgressiveQueries()
    {
    if (nullptr != s_newQueryStartingNodeProcessor)
        delete s_newQueryStartingNodeProcessor;
    s_newQueryStartingNodeProcessor = nullptr;
    }

void ComputeOverviewSearchToLoadNodes(RequestedQuery&                                            newQuery, 
                                      bvector<IScalableMeshCachedDisplayNodePtr>&                lowerResOverviewNodes,
                                      bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& searchingNodes,
                                      bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& toLoadNodes, 
                                      ProducedNodeContainer<DPoint3d, Extent3dType>&             nodesToSearch, 
                                      size_t                                                     currentInd,
                                      ProducedNodeContainer<DPoint3d, Extent3dType>&             foundNodes, 
                                      bset<uint64_t>&                                            activeClips,
                                      IScalableMeshPtr&                                          scalableMeshPtr, 
                                      IScalableMeshDisplayCacheManagerPtr&                       displayCacheManagerPtr)
    {       
    for (size_t nodeInd = currentInd + 1; nodeInd < nodesToSearch.GetNodes().size(); nodeInd++)        
        {                                                        
        searchingNodes.push_back(nodesToSearch.GetNodes()[nodeInd]);
        }   

    s_newQueryStartingNodeProcessor->Execute(newQuery, lowerResOverviewNodes, searchingNodes, toLoadNodes, nodesToSearch, currentInd, foundNodes, activeClips, scalableMeshPtr, displayCacheManagerPtr);
        
#ifdef PRINT_SMDISPLAY_MSG
    static uint64_t totalToLoadNodes = 0;
    totalToLoadNodes += toLoadNodes.size();
    PRINT_MSG("StartNewQuery m_requiredMeshNodes : %I64u toLoadNodes : %I64u totalToLoadNodes : %I64u \n", newQuery.m_requiredMeshNodes.size(), toLoadNodes.size(), totalToLoadNodes);
#endif    
    }

static bool s_loadNodeNearCamFirst = true;
    
void ScalableMeshProgressiveQueryEngine::StartNewQuery(RequestedQuery& newQuery, ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP, const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& startingNodes)
    {
    static int s_maxLevel = 2;
       
    HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> rootNodePtr(((ScalableMesh<DPoint3d>*)newQuery.m_meshToQuery.get())->GetRootNode());

    assert(rootNodePtr != 0);
    
    ProducedNodeContainer<DPoint3d, Extent3dType> overviewNodes;
    ProducedNodeContainer<DPoint3d, Extent3dType> nodesToSearch;
    ProducedNodeContainer<DPoint3d, Extent3dType> foundNodes;
    
    nodesToSearch.AddNode(rootNodePtr);        

    clock_t startTime = clock(); 
    int nbIterBeforeClock = 10;
    size_t currentInd = 0;
     
    while (currentInd < nodesToSearch.GetNodes().size())
        {
        //Increase display responsiveness, especially for streaming.
        if (!nodesToSearch.GetNodes()[currentInd]->IsLoaded())
            break;

        nodesToSearch.GetNodes()[currentInd]->QueryVisibleNode (queryObjectP, s_maxLevel, overviewNodes, foundNodes, nodesToSearch, nullptr);
        
        if ((clock() - startTime) > s_firstNodeSearchingDelay)
            {
            break;
            }
        
        currentInd++;
        }        
    
    bvector<IScalableMeshCachedDisplayNodePtr>                     lowerResOverviewNodes;
    bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>> searchingNodes;
    bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>> toLoadNodes;
              
    ComputeOverviewSearchToLoadNodes(newQuery, lowerResOverviewNodes, searchingNodes, toLoadNodes, nodesToSearch, currentInd, foundNodes, m_activeClips, newQuery.m_meshToQuery, m_displayCacheManagerPtr);

    //assert(lowerResOverviewNodes.size() > 0 || (nodesToSearch.GetNodes().size() - currentInd - 1) == 0);

    newQuery.m_overviewMeshNodes.insert(newQuery.m_overviewMeshNodes.end(), lowerResOverviewNodes.begin(), lowerResOverviewNodes.end());                    



    if (s_sortOverviewBySize == true)
        {
        struct {
            bool operator()(IScalableMeshCachedDisplayNodePtr& nodeA, IScalableMeshCachedDisplayNodePtr& nodeB)
            {   
            double maxLengthA = max(max(nodeA->GetContentExtent().XLength(), nodeA->GetContentExtent().YLength()), nodeA->GetContentExtent().ZLength());
            double maxLengthB = max(max(nodeB->GetContentExtent().XLength(), nodeB->GetContentExtent().YLength()), nodeB->GetContentExtent().ZLength());
            
            return maxLengthA > maxLengthB;
            }   
        } ContentExtentGreater;

        vector<size_t> queryNodeOrder;
        bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>> nodes;
        bmap<IScalableMeshCachedDisplayNode*, size_t> mapNodes;

        if (newQuery.m_overviewMeshNodes.size() > 0)
            {
            for (auto& node : newQuery.m_overviewMeshNodes) nodes.push_back(dynamic_cast<ScalableMeshNode<DPoint3d>*>(node.get())->GetNodePtr());
            queryObjectP->GetQueryNodeOrder(queryNodeOrder, nodes[0], &nodes[0], nodes.size());
        
            for (auto& node : newQuery.m_overviewMeshNodes) mapNodes[node.get()] = &node - &newQuery.m_overviewMeshNodes[0];
            }

        struct OverviewScoringMethod
            {
            vector<size_t>& m_scores;
            bmap<IScalableMeshCachedDisplayNode*, size_t>& m_map;
            bool operator()(IScalableMeshCachedDisplayNodePtr& nodeA, IScalableMeshCachedDisplayNodePtr& nodeB)
                {
                volatile double maxLengthA = max(max(nodeA->GetContentExtent().XLength(), nodeA->GetContentExtent().YLength()), nodeA->GetContentExtent().ZLength());
                volatile double maxLengthB = max(max(nodeB->GetContentExtent().XLength(), nodeB->GetContentExtent().YLength()), nodeB->GetContentExtent().ZLength());

				volatile size_t scoreA = m_scores[m_map[nodeA.get()]];
				volatile size_t scoreB = m_scores[m_map[nodeB.get()]];

                return (maxLengthA - maxLengthB) + ((int)scoreA -(int)scoreB)*(maxLengthA - maxLengthB) / 6 < 0;
                }

            OverviewScoringMethod(vector<size_t>& scores, bmap<IScalableMeshCachedDisplayNode*, size_t>& map) : m_scores(scores), m_map(map)
                {
                
                }
            } OverviewScore(queryNodeOrder, mapNodes);
        std::sort(newQuery.m_overviewMeshNodes.begin(), newQuery.m_overviewMeshNodes.end(), OverviewScore);
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

    
    s_queryProcessor.AddQuery(newQuery.m_queryId, queryObjectP, searchingNodes, toLoadNodes, newQuery.m_loadTexture, m_activeClips, newQuery.m_meshToQuery, m_displayCacheManagerPtr);
    }
      
BentleyStatus ScalableMeshProgressiveQueryEngine::_ClearCaching(const bvector<DRange2d>* clearRanges, const IScalableMeshPtr& scalableMeshPtr)
    {
    //NEEDS_WORK_SM : What to do 
    //CachedDisplayNodeManager::GetManager().ClearCachedNodes(clearRanges, scalableMeshPtr);

    return SUCCESS;
    }

BentleyStatus ScalableMeshProgressiveQueryEngine::_ClearCaching(const bvector<uint64_t>& clipIds, const IScalableMeshPtr& scalableMeshPtr)
    {
    //NEEDS_WORK_SM : What to do 
    //CachedDisplayNodeManager::GetManager().ClearCachedNodes(clipIds, scalableMeshPtr);

    return SUCCESS;
    }

void ScalableMeshProgressiveQueryEngine::_SetActiveClips(const bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr)
    {
    m_activeClips = activeClips;
    UpdatePreloadOverview();
    }

void ScalableMeshProgressiveQueryEngine::_InitScalableMesh(IScalableMeshPtr& scalableMeshPtr)
    {
    HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> rootNodePtr(((ScalableMesh<DPoint3d>*)scalableMeshPtr.get())->GetRootNode());

    bvector<uint64_t> allShownIds;
    scalableMeshPtr->GetAllClipIds(allShownIds);

    bset<uint64_t> activeClips = m_activeClips;
    for (auto&id : allShownIds) activeClips.insert(id);
    _SetActiveClips(activeClips, scalableMeshPtr);

    PreloadOverview(rootNodePtr, scalableMeshPtr.get());
    }

void ScalableMeshProgressiveQueryEngine::_ClearOverviews(IScalableMesh* scalableMeshP)
    {
    bvector<ScalableMeshCachedDisplayNode<DPoint3d>::Ptr> m_remainingOverviews;
    bvector<IScalableMesh*> m_meshForOverviews;
    for (auto& node : m_overviewNodes)
        {
        if (m_smOverviews[&node - &m_overviewNodes.front()] != scalableMeshP)
            {
            m_remainingOverviews.push_back(node);
            m_meshForOverviews.push_back(m_smOverviews[&node - &m_overviewNodes.front()]);
            }
        }
    m_overviewNodes = m_remainingOverviews;
    m_smOverviews = m_meshForOverviews;
    }

BentleyStatus ScalableMeshProgressiveQueryEngine::_StartQuery(int                                                                      queryId,
                                                              IScalableMeshViewDependentMeshQueryParamsPtr                             queryParam,
                                                              const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& startingNodes,
                                                              bool                                                                     loadTexture,
                                                              const bvector<bool>&                                                     clipVisibilities,
                                                              IScalableMeshPtr&                                                        smPtr)
    {
    assert(_IsQueryComplete(queryId) == true);

    RequestedQuery requestedQuery;

    requestedQuery.m_queryId = queryId;
    requestedQuery.m_isQueryCompleted = false;
    requestedQuery.m_fetchLastCompletedNodes = false;
    requestedQuery.m_loadTexture = loadTexture;
    requestedQuery.m_clipVisibilities = clipVisibilities;
    requestedQuery.m_meshToQuery = smPtr; 

    ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP;

    BuildQueryObject<DPoint3d>(queryObjectP, 0/*pQueryExtentPts*/, 0/*nbQueryExtentPts*/, queryParam, smPtr.get());

    assert(queryObjectP != 0);
      
    StartNewQuery(requestedQuery, queryObjectP, startingNodes);

    m_requestedQueries.push_back(requestedQuery);

    return SUCCESS;
    }

BentleyStatus ScalableMeshProgressiveQueryEngine::_GetOverviewNodes(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& meshNodes,
                                                                    int                                                                               queryId) const
    {
    const RequestedQuery* requestedQueryP = 0;

    for (auto& query : m_requestedQueries)
        {
        if (query.m_queryId == queryId)
            {
            requestedQueryP = &query;
            break;
            }
        }

    BentleyStatus status;

    if (requestedQueryP != 0)
        {
        if (s_keepSomeInvalidate)
            {
            for (auto& overviewNode : requestedQueryP->m_overviewMeshNodes)
                {
                auto requiredNodeIter(requestedQueryP->m_requiredMeshNodes.begin());
                auto requiredNodeIterEnd(requestedQueryP->m_requiredMeshNodes.end());

                while (requiredNodeIter != requiredNodeIterEnd)
                    {                           
                    if (overviewNode->GetNodeId() == (*requiredNodeIter)->GetNodeId())
                        break;

                    requiredNodeIter++;
                    }

                if (requiredNodeIter == requiredNodeIterEnd)
                    {
                    meshNodes.push_back(overviewNode);
                    }
                }                                                
            }
        else
            {
            meshNodes.insert(meshNodes.end(), requestedQueryP->m_overviewMeshNodes.begin(), requestedQueryP->m_overviewMeshNodes.end());
            }

        status = SUCCESS;
        }
    else
        {
        status = ERROR;
        }

    return status;
    }

BentleyStatus ScalableMeshProgressiveQueryEngine::_GetRequiredNodes(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& meshNodes,
                                                                    int                                                   queryId) const
    {
    //if (m_requestedQueries.empty()) return SUCCESS;
    RequestedQuery* requestedQueryP = 0;

    for (auto& query : m_requestedQueries)
        {
        if (query.m_queryId == queryId)
            {
            requestedQueryP = &query;
            break;
            }
        }

    BentleyStatus status;

    if (requestedQueryP != 0)
        {
        if (requestedQueryP->m_isQueryCompleted == false || requestedQueryP->m_fetchLastCompletedNodes == false)
            {            
            StatusInt status = s_queryProcessor.GetFoundNodes(requestedQueryP->m_requiredMeshNodes, queryId);
            assert(status == SUCCESS);

            if (requestedQueryP->m_isQueryCompleted == true)
                {
                requestedQueryP->m_fetchLastCompletedNodes = true;
                //assert(requestedQueryP->m_requiredMeshNodes.size() > 0);
                s_queryProcessor.CancelQuery(queryId);
                }
            }

        meshNodes.insert(meshNodes.end(), requestedQueryP->m_requiredMeshNodes.begin(), requestedQueryP->m_requiredMeshNodes.end());
        status = SUCCESS;
        }
    else
        {
        status = ERROR;
        }

    return status;
    }

BentleyStatus ScalableMeshProgressiveQueryEngine::_StopQuery(int queryId)
    {
    RequestedQuery* requestedQueryP = 0;

    auto queryIter(m_requestedQueries.begin());
    auto queryIterEnd(m_requestedQueries.end());

    while (queryIter != queryIterEnd)
    {
        if (queryIter->m_queryId == queryId)
        {
            requestedQueryP = &(*queryIter);
            break;
        }
        queryIter++;
    }

    if (requestedQueryP != 0)
    {
        StatusInt status = s_queryProcessor.CancelQuery(requestedQueryP->m_queryId);
        assert(status == SUCCESS);
        m_requestedQueries.erase(queryIter);
    }

    return SUCCESS;
    }


bool ScalableMeshProgressiveQueryEngine::_IsQueryComplete(int queryId)
    {
    bool isQueryComplete;

    RequestedQuery* requestedQueryP = 0;

    for (auto& query : m_requestedQueries)
    {
        if (query.m_queryId == queryId)
        {
            requestedQueryP = &query;
            break;
        }
    }

    if (requestedQueryP == 0)
    {
        isQueryComplete = true;
    }
    else
    {
        if (!requestedQueryP->m_isQueryCompleted)
        {
            //NEEDS_WORK_SM_PROGRESSIVE : Should have auto notification
            requestedQueryP->m_isQueryCompleted = s_queryProcessor.IsQueryComplete(queryId);
        }

        isQueryComplete = requestedQueryP->m_isQueryCompleted;
    }

    return isQueryComplete;
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
