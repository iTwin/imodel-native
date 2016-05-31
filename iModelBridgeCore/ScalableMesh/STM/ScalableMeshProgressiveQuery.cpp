/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshProgressiveQuery.cpp $
|    $RCSfile: ScalableMeshPointQuery.cpp,v $
|   $Revision: 1.41 $
|       $Date: 2012/11/29 17:30:37 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

//NEEDS_WORK_SM : Ugly, just for testing purpose
IScalableMeshDisplayCacheManagerPtr s_displayCacheManagerPtr;
IScalableMeshPtr                    s_scalableMeshPtr;

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

static bool s_computeInParallel = true;

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



BentleyStatus IScalableMeshProgressiveQueryEngine::StartQuery(int                                                                      queryId,
                                                              IScalableMeshViewDependentMeshQueryParamsPtr                             queryParam,
                                                              const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& startingNodes,
                                                              bool                                                                     loadTexture,
                                                              const bvector<bool>&                                                     clipVisibilities,
                                                              const DMatrix4d*                                                         prevLocalToView,
                                                              const DMatrix4d*                                                         newLocalToView)
    {
    return _StartQuery(queryId, queryParam, startingNodes, loadTexture, clipVisibilities, prevLocalToView, newLocalToView);
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
    s_displayCacheManagerPtr = displayCacheManagerPtr;
    //s_scalableMeshPtr = scalableMeshPtr;
    return new ScalableMeshProgressiveQueryEngine(scalableMeshPtr, displayCacheManagerPtr);
    }

static bool s_LoadQVDuringQuery = true;

static bool s_keepSomeInvalidate = true; 
static int s_maxNbLevelToKeep = 4;

                                            
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

    /*
    void GetFoundNodes(bvector<IScalableMeshCachedDisplayNodePtr>& foundNodes)a
        {

    #ifdef DISPLAYLOG
    fprintf(logger.GetFile(), "threadId %i : nb found nodes : %I64d\n", m_threadId, m_foundNodes.GetNodes().size());
    fflush(logger.GetFile());
    #endif

    for (auto& node : m_foundNodes.GetNodes())
    {
    IScalableMeshCachedDisplayNodePtr meshNodePtr;

    if (!s_LoadQVDuringQuery)
    {
    ScalableMeshCachedMeshNode<POINT>* meshNode(new ScalableMeshCachedMeshNode<POINT>(node, m_loadTexture));
    bvector<bool> clipsToShow;
    meshNode->LoadMesh(false, clipsToShow);
    meshNodePtr = meshNode;
    }
    else
    {
    meshNodePtr = CachedDisplayNodeManager::GetManager().FindOrLoadNode<POINT>(node, m_loadTexture);
    }

    foundNodes.push_back(meshNodePtr);
    }
    }

    */

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
        m_isConsumingNode = false;
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
        if (m_producedFoundNodes.WaitConsumption())
            return false;

        if (m_isConsumingNode == true)
            return false;

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

            m_nodeQueryProcessorMutexes[threadInd].lock();
            if (m_nodeQueryProcessors[threadInd] != 0)
                {
                m_nodeQueryProcessorMutexes[threadInd].unlock();
                break;
                }

            m_nodeQueryProcessorMutexes[threadInd].unlock();
            }

        if (threadInd != m_searchingNodes.size())
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

    bvector<NodeQueryProcessor<DPoint3d, YProtPtExtentType>::Ptr>  m_nodeQueryProcessors;
    std::mutex*                                                    m_nodeQueryProcessorMutexes;

    ISMPointIndexQuery<POINT, EXTENT>*                        m_queryObjectP;
    //atomic<int>                                               m_nbSearchingNodes;    
    atomic<bool>                                              m_isCancel;
    bool                                                      m_loadTexture;
    const bset<uint64_t>                                       m_clipVisibilities;
    atomic<bool>                                              m_isConsumingNode;
    IScalableMeshPtr                                          m_scalableMeshPtr;
    IScalableMeshDisplayCacheManagerPtr                       m_displayCacheManagerPtr;
    };

static bool s_delayJoinThread = true;
static bool s_streamingSM = false;

class QueryProcessor
    {
public:

private:

    typedef std::list<ProcessingQuery<DPoint3d, YProtPtExtentType>::Ptr> ProcessingQueryList;

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
        InLoadingNode(HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>> visibleNode, 
                      IScalableMeshCachedDisplayNodePtr                     displayNode)
            {
            m_visibleNode = visibleNode;
            m_displayNode = displayNode; 
            }

        static InLoadingNodePtr Create(HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>> visibleNode, 
                                       IScalableMeshCachedDisplayNodePtr                     displayNode)
            {
            return new InLoadingNode(visibleNode, displayNode);
            }

        HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>> m_visibleNode;
        IScalableMeshCachedDisplayNodePtr                     m_displayNode;         
        };    

    std::mutex m_inLoadingNodeMutex;
    bvector<InLoadingNodePtr> m_inLoadingNodes;


    void LoadNodeDisplayData(IScalableMeshCachedDisplayNodePtr& meshNodePtr, HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>>& visibleNode, bool loadTexture, const bset<uint64_t>& clipVisibilities, IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr)
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
            if (!s_LoadQVDuringQuery)
                {
                ScalableMeshCachedMeshNode<DPoint3d>* meshNode(ScalableMeshCachedMeshNode<DPoint3d>::Create(visibleNode, loadTexture));                
                meshNode->LoadMesh(false, clipVisibilities);
                meshNodePtr = meshNode;
                }
            else
                {
                //meshNodePtr = CachedDisplayNodeManager::GetManager().FindOrLoadNode<DPoint3d>(visibleNode, loadTexture, clipVisibilities);
                ScalableMeshCachedDisplayNode<DPoint3d>* meshNode(ScalableMeshCachedDisplayNode<DPoint3d>::Create(visibleNode));                
                
                if (meshNode->IsLoaded() == false || !meshNode->IsClippingUpToDate() || !meshNode->HasCorrectClipping(clipVisibilities))                    
                    {
                    meshNode->ApplyAllExistingClips();
                    meshNode->RemoveDisplayDataFromCache();                    
                    meshNode->LoadMesh(false, clipVisibilities, s_displayCacheManagerPtr, loadTexture);                               
                    assert(meshNode->HasCorrectClipping(clipVisibilities));                    
                    }

                meshNodePtr = meshNode;                                                
                }

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

        ProcessingQuery<DPoint3d, YProtPtExtentType>::Ptr processingQueryPtr;

        do
            {
            
            m_processingQueriesMutex.lock();
            //assert(m_processingQueries.size() <= 1);

            if (m_processingQueries.size() > 0)
                {
                processingQueryPtr = m_processingQueries.front();
                }
            else
                {
                processingQueryPtr = 0;
                }

            m_processingQueriesMutex.unlock();

            //NEEDS_WORK_MST : Maybe we should prioritize the first processing query found
            if (processingQueryPtr != 0)
                {
                HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>> nodePtr;

                processingQueryPtr->m_searchingNodeMutexes[threadId].lock();

                if (processingQueryPtr->m_searchingNodes[threadId].size() > 0)
                    {                    
                    nodePtr = processingQueryPtr->m_searchingNodes[threadId].back();
                    processingQueryPtr->m_searchingNodes[threadId].pop_back();                    
                    }

                processingQueryPtr->m_searchingNodeMutexes[threadId].unlock();

                if (nodePtr != 0)
                    {
                    processingQueryPtr->m_nodeQueryProcessorMutexes[threadId].lock();
                    processingQueryPtr->m_nodeQueryProcessors[threadId] = NodeQueryProcessor<DPoint3d, YProtPtExtentType>::Create(nodePtr, processingQueryPtr->m_queryObjectP, 0, processingQueryPtr->m_loadTexture, &processingQueryPtr->m_producedFoundNodes, threadId);
                    processingQueryPtr->m_nodeQueryProcessorMutexes[threadId].unlock();

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
                //HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>> nodePtr;                
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
                    HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>> consumedNodePtr;

                    if (!processingQueryPtr->m_producedFoundNodes.WaitConsumption())
                        {
                        m_nbMissed++;
                        continue;
                        }
            
                    //NEEDS_WORK_SM : Should be set only if a no
                    processingQueryPtr->m_isConsumingNode = true;

                    if (processingQueryPtr->m_producedFoundNodes.ConsumeNode(consumedNodePtr))
                        {                        
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
                        }                    

                    processingQueryPtr->m_isConsumingNode = false;
                    }                
                }

            } while (m_run && (processingQueryPtr != 0));

        m_areWorkingThreadRunning[threadId] = false;
        }
         
public:

    QueryProcessor()
        {
        if (!s_streamingSM)
            {
            m_numWorkingThreads = std::thread::hardware_concurrency() - 2;
            //m_numWorkingThreads = 1;
            }
        else
            {
            m_numWorkingThreads = 14;
            }

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
                  ISMPointIndexQuery<DPoint3d, YProtPtExtentType>*                queryObjectP,
                  bvector<HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>>>& searchingNodes,
                  bvector<HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>>>& toLoadNodes,                  
                  bool                                                            loadTexture, 
                  const bset<uint64_t>&                                            clipVisibilities,
                  IScalableMeshPtr&                                                scalableMeshPtr,
                  IScalableMeshDisplayCacheManagerPtr&               displayCacheManagerPtr)
        {

#ifdef DISPLAYLOG
            fprintf(logger.GetFile(), "START NEW QUERY\n");
            fflush(logger.GetFile());                                                    
#endif


#ifndef NDEBUG
        for (auto& query : m_processingQueries)
            {
            if (query->m_queryId == queryId)
                assert(!"Query already processing");
            }
#endif
        ProcessingQuery<DPoint3d, YProtPtExtentType>::Ptr processingQueryPtr(ProcessingQuery<DPoint3d, YProtPtExtentType>::Create(queryId, m_numWorkingThreads, queryObjectP, searchingNodes, toLoadNodes, loadTexture, clipVisibilities, scalableMeshPtr, displayCacheManagerPtr));

        size_t currentNbProcessingQueries;

        m_processingQueriesMutex.lock();
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
        ProcessingQuery<DPoint3d, YProtPtExtentType>::Ptr toCancelQueryPtr;        

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

        ProcessingQuery<DPoint3d, YProtPtExtentType>::Ptr toCancelQueryPtr;

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
            
            ProcessingQuery<DPoint3d, YProtPtExtentType>::Ptr queryPtr;

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
                    if (!s_delayJoinThread && !s_streamingSM)
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

            if (!s_delayJoinThread && !s_streamingSM)
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

            std::list<ProcessingQuery<DPoint3d, YProtPtExtentType>::Ptr>::iterator queryItr(m_processingQueries.begin());
            std::list<ProcessingQuery<DPoint3d, YProtPtExtentType>::Ptr>::iterator queryItrEnd(m_processingQueries.end());

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


ScalableMeshProgressiveQueryEngine::ScalableMeshProgressiveQueryEngine(IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr)
    {
    m_scalableMeshPtr = scalableMeshPtr;
    m_displayCacheManagerPtr = displayCacheManagerPtr;
    }

ScalableMeshProgressiveQueryEngine::~ScalableMeshProgressiveQueryEngine()
    {    
    s_queryProcessor.CancelAllQueries();    
    }

template <class POINT> int BuildQueryObject(//ScalableMeshQuadTreeViewDependentMeshQuery<POINT, YProtPtExtentType>* viewDependentQueryP,
    ISMPointIndexQuery<POINT, YProtPtExtentType>*&                        pQueryObject,
    const DPoint3d*                                                       pQueryExtentPts,
    int                                                                   nbQueryExtentPts,
    IScalableMeshViewDependentMeshQueryParamsPtr                          queryParam)
    {
    //MST More validation is required here.
    assert(queryParam != 0);

    int status = SUCCESS;

    YProtPtExtentType queryExtent;
    /*
    YProtPtExtentType contentExtent(m_scmIndexPtr->GetContentExtent());

    double minZ = ExtentOp<YProtPtExtentType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<YProtPtExtentType>::GetZMax(contentExtent);

    YProtPtExtentType queryExtent(ScalableMeshPointQuery::GetExtentFromClipShape<YProtPtExtentType>(pQueryExtentPts,
    nbQueryExtentPts,
    minZ,
    maxZ));
    */
    //MS Need to be removed
    double viewportRotMatrix[3][3];
    double rootToViewMatrix[4][4];

    memcpy(rootToViewMatrix, queryParam->GetRootToViewMatrix(), sizeof(double) * 4 * 4);

    ScalableMeshQuadTreeViewDependentMeshQuery<POINT, YProtPtExtentType>* viewDependentQueryP = new ScalableMeshQuadTreeViewDependentMeshQuery<POINT, YProtPtExtentType>(queryExtent,
        rootToViewMatrix,
        viewportRotMatrix,
        queryParam->GetViewBox(),
        false,
        queryParam->GetViewClipVector(),
        100000000);

    // viewDependentQueryP->SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 17\\STM\\Bad Resolution Selection\\visitingNodes.xml"));

    viewDependentQueryP->SetMeanScreenPixelsPerPoint(queryParam->GetMinScreenPixelsPerPoint());

    //MS : Might need to be done at the ScalableMeshReprojectionQuery level.    
    if ((queryParam->GetSourceGCS() != 0) && (queryParam->GetTargetGCS() != 0))
        {
        BaseGCSCPtr sourcePtr = queryParam->GetSourceGCS();
        BaseGCSCPtr targetPtr = queryParam->GetTargetGCS();
        viewDependentQueryP->SetReprojectionInfo(sourcePtr, targetPtr);
        }   


    pQueryObject = (ISMPointIndexQuery<POINT, YProtPtExtentType>*)(viewDependentQueryP);

    return status;
    }

DisplayMovementType DetermineDisplayMovementType(const DMatrix4d* prevLocalToView,
                             const DMatrix4d* newLocalToView)
    {
    if (prevLocalToView == nullptr || newLocalToView == nullptr)
        return DisplayMovementType::UNKNOWN;

    /*
    DMatrix4d resultMatrix; 

    resultMatrix.differenceOf(newLocalToView, prevLocalToView);
    */

    return DisplayMovementType::UNKNOWN;
    }

static bool s_searchNodeAround = true; 
static bool s_addStartingNodeToPreview = true;
static bool s_newOverview = true;

#ifndef NDEBUG
static double s_firstNodeSearchingDelay = (double)1 / 15 * CLOCKS_PER_SEC;
#else
static double s_firstNodeSearchingDelay = (double)1 / 30 * CLOCKS_PER_SEC;
#endif
//static int    s_nbIterClock;

void FindOverview(bvector<IScalableMeshCachedDisplayNodePtr>& lowerResOverviewNodes, HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>>& node, bool loadTexture, const bset<uint64_t>& clipVisibilities/*, IScalableMeshPtr& scalableMeshPtr*/)
    {    
    assert(node->IsParentSet() == true);
        
    HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>> parentNodePtr(node->GetParentNodePtr());
    assert(parentNodePtr != node);

    //NEEDS_WORK_MST : Root node could be loaded at load time instead. 
    if (parentNodePtr == nullptr)
        {        
        //assert(!"Should not occurs");
        /*
        IScalableMeshCachedDisplayNodePtr meshNodePtr = CachedDisplayNodeManager::GetManager().FindOrLoadNode<DPoint3d>(node, loadTexture, clipVisibilities);
        lowerResOverviewNodes.push_back(meshNodePtr);        
        */
        return;
        }
         
    ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNodePtr(ScalableMeshCachedDisplayNode<DPoint3d>::Create(parentNodePtr));    
            
    if (!meshNodePtr->IsLoaded() || ((!meshNodePtr->IsClippingUpToDate() || !meshNodePtr->HasCorrectClipping(clipVisibilities)) && !s_keepSomeInvalidate))
        {
        FindOverview(lowerResOverviewNodes, parentNodePtr, loadTexture, clipVisibilities);
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

        if (nodeIter == lowerResOverviewNodes.end())
            {
            lowerResOverviewNodes.push_back(meshNodePtr);
            }        
        }
    }

static bool s_sortOverviewBySize = true; 

class NewQueryStartingNodeProcessor
    {
    private : 

        ProducedNodeContainer<DPoint3d, YProtPtExtentType>* m_nodesToSearch;
        size_t                                              m_nodeToSearchCurrentInd;
        ProducedNodeContainer<DPoint3d, YProtPtExtentType>* m_foundNodes;
        bool                                                m_loadTexture; 
        RequestedQuery*                                     m_newQuery;
        bset<uint64_t>*                                     m_activeClips;

    
        bvector<bvector<IScalableMeshCachedDisplayNodePtr>>                     m_lowerResOverviewNodes;
        bvector<bvector<IScalableMeshCachedDisplayNodePtr>>                     m_requiredMeshNodes;    
        bvector<bvector<HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>>>> m_toLoadNodes;
        
        int          m_numWorkingThreads;
        std::thread* m_workingThreads;    

    public : 

        NewQueryStartingNodeProcessor()
            {
            m_numWorkingThreads = std::thread::hardware_concurrency() - 2;
            //m_numWorkingThreads = 1;

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
            m_lowerResOverviewNodes.clear();
            m_requiredMeshNodes.clear();
            m_toLoadNodes.clear();
            }

        void QueryThread(/*DgnPlatformLib::Host* hostToAdopt,*/ size_t threadId, IScalableMeshPtr& scalableMeshPtr)
            {       
            m_lowerResOverviewNodes[threadId].clear();
            m_toLoadNodes[threadId].clear();
            m_requiredMeshNodes[threadId].clear();

            for (size_t nodeInd = m_nodeToSearchCurrentInd + 1; nodeInd < m_nodesToSearch->GetNodes().size(); nodeInd++)        
                {                              
                if (nodeInd % m_numWorkingThreads != threadId) continue;
                
                ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNodePtr(ScalableMeshCachedDisplayNode<DPoint3d>::Create(m_nodesToSearch->GetNodes()[nodeInd]));
                                
                if (!meshNodePtr->IsLoaded() || ((!meshNodePtr->IsClippingUpToDate() || !meshNodePtr->HasCorrectClipping(*m_activeClips)) && !s_keepSomeInvalidate))
                    {                                
                    FindOverview(m_lowerResOverviewNodes[threadId], m_nodesToSearch->GetNodes()[nodeInd], m_newQuery->m_loadTexture, *m_activeClips);
                    }
                else
                    {
                    m_lowerResOverviewNodes[threadId].push_back(meshNodePtr);
                    }            
                }                    
        
            for (size_t nodeInd = 0; nodeInd < m_foundNodes->GetNodes().size(); nodeInd++)        
                {                      
                if (nodeInd % m_numWorkingThreads != threadId) continue;
                
                ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNodePtr(ScalableMeshCachedDisplayNode<DPoint3d>::Create(m_foundNodes->GetNodes()[nodeInd]));
                
                if (!meshNodePtr->IsLoaded() || ((!meshNodePtr->IsClippingUpToDate() || !meshNodePtr->HasCorrectClipping(*m_activeClips)) && !s_keepSomeInvalidate))
                    {                
                    FindOverview(m_lowerResOverviewNodes[threadId], m_foundNodes->GetNodes()[nodeInd], m_newQuery->m_loadTexture, *m_activeClips/*, scalableMeshPtr*/);
                                        
                    m_toLoadNodes[threadId].push_back(m_foundNodes->GetNodes()[nodeInd]);
                    }
                else
                    {
                    //NEEDS_WORK_SM : Should not be duplicated.
                    m_lowerResOverviewNodes[threadId].push_back(meshNodePtr);
                    if (meshNodePtr->IsClippingUpToDate() && meshNodePtr->HasCorrectClipping(*m_activeClips))
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

        void Execute(RequestedQuery&                                                 newQuery, 
                     bvector<IScalableMeshCachedDisplayNodePtr>&                     lowerResOverviewNodes,
                     bvector<HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>>>& searchingNodes,
                     bvector<HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>>>& toLoadNodes,                 
                     ProducedNodeContainer<DPoint3d, YProtPtExtentType>&             nodesToSearch,
                     size_t                                                          nodeToSearchCurrentInd,
                     ProducedNodeContainer<DPoint3d, YProtPtExtentType>&             foundNodes, 
                     bset<uint64_t>&                                                 activeClips,
                     IScalableMeshPtr& scalableMeshPtr)
            {        
            m_nodesToSearch = &nodesToSearch;
            m_nodeToSearchCurrentInd = nodeToSearchCurrentInd;
            m_foundNodes = &foundNodes;        
            m_newQuery = &newQuery;
            m_activeClips = &activeClips;

            for (size_t threadId = 0; threadId < m_numWorkingThreads; ++threadId) 
                {                                                                                
                m_workingThreads[threadId] = std::thread(&NewQueryStartingNodeProcessor::QueryThread, this, threadId, scalableMeshPtr);
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
                            break;

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


void ComputeOverviewSearchToLoadNodes(RequestedQuery&                                                 newQuery, 
                                      bvector<IScalableMeshCachedDisplayNodePtr>&                     lowerResOverviewNodes,
                                      bvector<HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>>>& searchingNodes,
                                      bvector<HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>>>& toLoadNodes, 
                                      ProducedNodeContainer<DPoint3d, YProtPtExtentType>&             nodesToSearch, 
                                      size_t                                                          currentInd,
                                      ProducedNodeContainer<DPoint3d, YProtPtExtentType>&             foundNodes, 
                                      bset<uint64_t>&                                                 activeClips,
                                      IScalableMeshPtr& scalableMeshPtr)
    {   

    if (s_computeInParallel)   
        {
        for (size_t nodeInd = currentInd + 1; nodeInd < nodesToSearch.GetNodes().size(); nodeInd++)        
            {                                                        
            searchingNodes.push_back(nodesToSearch.GetNodes()[nodeInd]);
            }   

        s_newQueryStartingNodeProcessor->Execute(newQuery, lowerResOverviewNodes, searchingNodes, toLoadNodes, nodesToSearch, currentInd, foundNodes, activeClips, scalableMeshPtr);
        }       
    else
        {
        for (size_t nodeInd = currentInd + 1; nodeInd < nodesToSearch.GetNodes().size(); nodeInd++)        
            {                                                            
            ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNode(ScalableMeshCachedDisplayNode<DPoint3d>::Create(nodesToSearch.GetNodes()[nodeInd]));

            if (!meshNode->IsLoaded() || ((!meshNode->IsClippingUpToDate() || !meshNode->HasCorrectClipping(activeClips)) && !s_keepSomeInvalidate))
                {                                
                FindOverview(lowerResOverviewNodes, nodesToSearch.GetNodes()[nodeInd], newQuery.m_loadTexture, activeClips);
                }
            else
                {
                IScalableMeshCachedDisplayNodePtr meshNodePtr(meshNode.get()) ;
                newQuery.m_overviewMeshNodes.push_back(meshNodePtr);
                }

            searchingNodes.push_back(nodesToSearch.GetNodes()[nodeInd]);
            }                    

        for (auto& node : foundNodes.GetNodes())
            {                                                
            ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNodePtr(ScalableMeshCachedDisplayNode<DPoint3d>::Create(node));

            if (!meshNodePtr->IsLoaded() || ((!meshNodePtr->IsClippingUpToDate() || !meshNodePtr->HasCorrectClipping(activeClips)) && !s_keepSomeInvalidate))
                {                
                FindOverview(lowerResOverviewNodes, node, newQuery.m_loadTexture, activeClips);
                toLoadNodes.push_back(node);
                }
            else
                {
                //NEEDS_WORK_SM : Should not be duplicated.                
                newQuery.m_overviewMeshNodes.push_back(meshNodePtr);

                if (meshNodePtr->IsClippingUpToDate() && meshNodePtr->HasCorrectClipping(activeClips))
                    {                        
                    newQuery.m_requiredMeshNodes.push_back(meshNodePtr);
                    }
                else
                    {
                    toLoadNodes.push_back(node);
                    }
                }
            }
        }    
    
#ifdef PRINT_SMDISPLAY_MSG
    PRINT_MSG("StartNewQuery m_requiredMeshNodes : %I64u toLoadNodes : %I64u \n", newQuery.m_requiredMeshNodes.size(), toLoadNodes.size());
#endif    
    }
    
void ScalableMeshProgressiveQueryEngine::StartNewQuery(RequestedQuery& newQuery, ISMPointIndexQuery<DPoint3d, YProtPtExtentType>* queryObjectP, const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& startingNodes)
    {
    static int s_maxLevel = 2;
       
    HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>> rootNodePtr(((ScalableMesh<DPoint3d>*)m_scalableMeshPtr.get())->GetRootNode());

    assert(rootNodePtr != 0);

    if (s_newOverview)
        {
        ProducedNodeContainer<DPoint3d, YProtPtExtentType> overviewNodes;
        ProducedNodeContainer<DPoint3d, YProtPtExtentType> nodesToSearch;
        ProducedNodeContainer<DPoint3d, YProtPtExtentType> foundNodes;
        
        nodesToSearch.AddNode(rootNodePtr);        

        clock_t startTime = clock(); 
        int nbIterBeforeClock = 10;
        size_t currentInd = 0;
         
        while (currentInd < nodesToSearch.GetNodes().size())
            {
            nodesToSearch.GetNodes()[currentInd]->QueryVisibleNode (queryObjectP, s_maxLevel, overviewNodes, foundNodes, nodesToSearch, nullptr);
            
            if ((clock() - startTime) > s_firstNodeSearchingDelay)
                {
                break;
                }
            
            currentInd++;
            }

        //The other option not yet supported.
        assert(s_LoadQVDuringQuery);
        
        bvector<IScalableMeshCachedDisplayNodePtr>                     lowerResOverviewNodes;
        bvector<HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>>> searchingNodes;
        bvector<HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>>> toLoadNodes;
                  
        ComputeOverviewSearchToLoadNodes(newQuery, lowerResOverviewNodes, searchingNodes, toLoadNodes, nodesToSearch, currentInd, foundNodes, m_activeClips, m_scalableMeshPtr);
        

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

            std::sort(newQuery.m_overviewMeshNodes.begin(), newQuery.m_overviewMeshNodes.end(), ContentExtentGreater);                        
            }
        
        s_queryProcessor.AddQuery(newQuery.m_queryId, queryObjectP, searchingNodes, toLoadNodes, newQuery.m_loadTexture, m_activeClips, m_scalableMeshPtr, m_displayCacheManagerPtr);
        }
    else
        {             
        ProducedNodeContainer<DPoint3d, YProtPtExtentType> overviewNodes;
        ProducedNodeContainer<DPoint3d, YProtPtExtentType> nodesToSearch;
        ProducedNodeContainer<DPoint3d, YProtPtExtentType> foundNodes;

        rootNodePtr->QueryOverview(queryObjectP, s_maxLevel, overviewNodes, foundNodes, nodesToSearch, 0);
        
        //Query not complete  
        if (nodesToSearch.GetNodes().size() > 0)
            {   
            if (foundNodes.GetNodes().size() > 0)
                {            
                for (auto& node : foundNodes.GetNodes())
                    {
                    IScalableMeshCachedDisplayNodePtr meshNodePtr;

                    if (!s_LoadQVDuringQuery)
                        {
                        ScalableMeshCachedMeshNode<DPoint3d>* meshNode(new ScalableMeshCachedMeshNode<DPoint3d>(node, newQuery.m_loadTexture));                        
                            meshNode->LoadMesh(false, m_activeClips);
                        meshNodePtr = meshNode;
                        }
                    else
                        {
                        ScalableMeshCachedDisplayNode<DPoint3d>* meshNode(ScalableMeshCachedDisplayNode<DPoint3d>::Create(node));
                        meshNode->LoadMesh(false, m_activeClips, s_displayCacheManagerPtr, newQuery.m_loadTexture);
                        meshNodePtr = meshNode;
                        }

                    newQuery.m_requiredMeshNodes.push_back(meshNodePtr);
                    }
                }
                      
            bvector<HFCPtr<SMPointIndexNode<DPoint3d, YProtPtExtentType>>> toLoadNodes;

            s_queryProcessor.AddQuery(newQuery.m_queryId, queryObjectP, nodesToSearch.GetNodes(), toLoadNodes, newQuery.m_loadTexture, m_activeClips, m_scalableMeshPtr, m_displayCacheManagerPtr);

            for (auto& node : overviewNodes.GetNodes())
                {
                IScalableMeshCachedDisplayNodePtr meshNodePtr;

                if (!s_LoadQVDuringQuery)
                    {
                    ScalableMeshCachedMeshNode<DPoint3d>* meshNode(new ScalableMeshCachedMeshNode<DPoint3d>(node, newQuery.m_loadTexture));   
                        meshNode->LoadMesh(false, m_activeClips);
                    meshNodePtr = meshNode;
                    }
                else
                    {                    
                    ScalableMeshCachedDisplayNode<DPoint3d>* meshNode(ScalableMeshCachedDisplayNode<DPoint3d>::Create(node));
                    meshNode->LoadMesh(false, m_activeClips, s_displayCacheManagerPtr, newQuery.m_loadTexture);                    
                    meshNodePtr = meshNode;
                    }

                newQuery.m_overviewMeshNodes.push_back(meshNodePtr);
                }

            if (s_addStartingNodeToPreview)
                {
                newQuery.m_overviewMeshNodes.insert(newQuery.m_overviewMeshNodes.end(), startingNodes.begin(), startingNodes.end());            
                }
            }
        else
            {
            for (auto& node : overviewNodes.GetNodes())
                {
                IScalableMeshCachedDisplayNodePtr meshNodePtr;

                if (!s_LoadQVDuringQuery)
                    {
                    ScalableMeshCachedMeshNode<DPoint3d>* meshNode(new ScalableMeshCachedMeshNode<DPoint3d>(node, newQuery.m_loadTexture));
                    meshNode->LoadMesh(false, m_activeClips);
                    meshNodePtr = meshNode;
                    }
                else
                    {
                    ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNode(ScalableMeshCachedDisplayNode<DPoint3d>::Create(node));                    
                    meshNode->LoadMesh(false, m_activeClips, s_displayCacheManagerPtr, newQuery.m_loadTexture);
                    meshNodePtr = meshNode;                    
                    }

                newQuery.m_requiredMeshNodes.push_back(meshNodePtr);
                }

            delete queryObjectP;
            newQuery.m_isQueryCompleted = true;
            newQuery.m_fetchLastCompletedNodes = true;
            }
        }
    }
      
BentleyStatus ScalableMeshProgressiveQueryEngine::_ClearCaching(const bvector<DRange2d>* clearRanges, const IScalableMeshPtr& scalableMeshPtr)
    {
    //CachedDisplayNodeManager::GetManager().ClearCachedNodes(clearRanges, s_scalableMeshPtr);

    return SUCCESS;
    }

BentleyStatus ScalableMeshProgressiveQueryEngine::_ClearCaching(const bvector<uint64_t>& clipIds, const IScalableMeshPtr& scalableMeshPtr)
    {
    //CachedDisplayNodeManager::GetManager().ClearCachedNodes(clipIds, s_scalableMeshPtr);

    return SUCCESS;
    }

void ScalableMeshProgressiveQueryEngine::_SetActiveClips(const bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr)
    {
    m_activeClips = activeClips;
    }

BentleyStatus ScalableMeshProgressiveQueryEngine::_StartQuery(int                                                                      queryId,
                                                              IScalableMeshViewDependentMeshQueryParamsPtr                             queryParam,
                                                              const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& startingNodes,
                                                              bool                                                                     loadTexture,
                                                              const bvector<bool>&                                                     clipVisibilities,
                                                              const DMatrix4d*                                                         prevLocalToView,
                                                              const DMatrix4d*                                                         newLocalToView)
    {
    assert(_IsQueryComplete(queryId) == true);

    RequestedQuery requestedQuery;

    requestedQuery.m_queryId = queryId;
    requestedQuery.m_isQueryCompleted = false;
    requestedQuery.m_fetchLastCompletedNodes = false;
    requestedQuery.m_loadTexture = loadTexture;
    requestedQuery.m_clipVisibilities = clipVisibilities;

    ISMPointIndexQuery<DPoint3d, YProtPtExtentType>* queryObjectP;

    BuildQueryObject<DPoint3d>(queryObjectP, 0/*pQueryExtentPts*/, 0/*nbQueryExtentPts*/, queryParam);

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
