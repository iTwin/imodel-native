/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshProgressiveQuery.cpp $
|    $RCSfile: ScalableMeshPointQuery.cpp,v $
|   $Revision: 1.41 $
|       $Date: 2012/11/29 17:30:37 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#undef static_assert
#ifndef LINUX_SCALABLEMESH_BUILD
#include <DgnPlatform/DgnPlatformLib.h>
#else
#include <DgnPlatform/ClipVector.h>
#endif
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
#include "ScalableMeshProgressiveQueryPlanner.h"
#include "ScalableMeshProgressiveContourQueryPlanner.h"
#include "ScalableMeshProgressiveQueryProcessor.h"
#include "ScalableMeshProgressiveQueryProcessor.hpp"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

typedef DRange3d Extent3dType;

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

BentleyStatus IScalableMeshProgressiveQueryEngine::GetRequiredTextureTiles(bvector<SMRasterTile>& rasterTiles,
                                                                           int                    queryId)
    {
    return _GetRequiredTextureTiles(rasterTiles, queryId);
    }

void IScalableMeshProgressiveQueryEngine::SetActiveClips(const bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr)
    {
    return _SetActiveClips(activeClips, scalableMeshPtr);
    }

void IScalableMeshProgressiveQueryEngine::GetActiveClips(bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr)
{
    return _GetActiveClips(activeClips, scalableMeshPtr);
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

IScalableMeshProgressiveQueryEnginePtr IScalableMeshProgressiveQueryEngine::Create(IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr, bool loadTexture)
    {    
    return new ScalableMeshProgressiveQueryEngine(scalableMeshPtr, displayCacheManagerPtr, loadTexture);
    }

static bool s_keepSomeInvalidate = true; 
bool s_preloadInQueryThread = true;
                                            

   
      

//NEEDS_WORK_SM : Set to true it can lead to race condition, should be removed (and maybe m_areWorkingThreadRunning[threadId] too).
static bool s_delayJoinThread = true;

void ScalableMeshProgressiveQueryEngine::CancelPreload(ScalableMesh<DPoint3d>* smP)
    {
    ISMDataStoreTypePtr<Extent3dType> dataStore(smP->m_scmIndexPtr->GetDataStore());
    
    dataStore->CancelPreloadData();
    }

static bool s_doPreload = true;

#ifdef VANCOUVER_API
//Imagepp on Topaz is different then Imagepp (the redesigned Imagepp) on DgnDb06/Bim02 platform, and thus less thread safe.
extern std::mutex s_imageppCopyFromLock;
#endif

void ScalableMeshProgressiveQueryEngine::PreloadData(ScalableMesh<DPoint3d>* smP, bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& toLoadNodes, bool cancelLastPreload)
    {
    //Currently the preload is just use for streaming texture source.
    if (SMTextureType::Streaming != smP->GetMainIndexP()->IsTextured() || !s_doPreload)
        return;    
    
    bvector<DRange3d> tileRanges;

    for (auto& loadNode : toLoadNodes)
        {
        //tileRanges.push_back(loadNode->GetContentExtent());
        tileRanges.push_back(loadNode->GetNodeExtent());
        }
    
    ISMDataStoreTypePtr<Extent3dType> dataStore(smP->m_scmIndexPtr->GetDataStore());    

#ifdef VANCOUVER_API
    s_imageppCopyFromLock.lock();    
#endif

    if (cancelLastPreload)        
        dataStore->CancelPreloadData();

    dataStore->PreloadData(tileRanges);

#ifdef VANCOUVER_API
    s_imageppCopyFromLock.unlock();    
#endif
    }

//static bool s_doPreLoad = true;

QueryPlanner* ScalableMeshProgressiveQueryEngine::GetQueryPlanner(const RequestedQuery& queryObject)
{
    if (!queryObject.m_loadContours)
        return m_planners[0];
    else return m_planners[1];
}


    void QueryProcessor::LoadNodeDisplayData(IScalableMeshCachedDisplayNodePtr& meshNodePtr, HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>& visibleNode, bool loadTexture, const bset<uint64_t>& clipVisibilities, IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr)
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
                         
            SMMeshIndexNode<DPoint3d, Extent3dType>* smNode = dynamic_cast<SMMeshIndexNode<DPoint3d, Extent3dType>*>(visibleNode.GetPtr());
            TRACEPOINT(THREAD_ID(), EventType::EVT_CREATE_DISPLAY_LOAD, visibleNode->GetBlockID().m_integerID, (uint64_t)-1, smNode->GetSingleTextureID(), -1, (uint64_t)meshNode, -1)


            if ((meshNode->IsLoaded(displayCacheManagerPtr.get(), loadTexture) == false && meshNode->IsLoadedInVRAM(displayCacheManagerPtr.get(), loadTexture) == false) || !meshNode->IsClippingUpToDate() || !meshNode->HasCorrectClipping(clipVisibilities)
                 || (scalableMeshPtr->ShouldInvertClips() && !meshNode->HasInvertedClips()))
                {
                if (!meshNode->IsDataUpToDate()) meshNode->UpdateData();
                meshNode->RefreshMergedClip(scalableMeshPtr->GetReprojectionTransform());
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

    void QueryProcessor::QueryThread(
#ifndef LINUX_SCALABLEMESH_BUILD
DgnPlatformLib::Host* hostToAdopt, 
#endif
int threadId)
        {        
#ifdef VANCOUVER_API
        DgnPlatformLib::AdoptHost(*hostToAdopt);
#else
        //assert(!"No AdoptHost on BIM0200 - Untested behavior");
#endif

        ProcessingQuery<DPoint3d, Extent3dType>::Ptr processingQueryPtr;

        do
            {            
            processingQueryPtr = nullptr;
            
            //assert(m_processingQueries.size() <= 1);            

            {
            std::unique_lock<std::mutex> lck(m_processingQueriesMutex);

            for (auto& processingQuery : m_processingQueries)
                {
                if (!processingQuery->IsComplete(threadId))
                    {
                    processingQueryPtr = processingQuery;
                    break;
                    }
                }           

            if (processingQueryPtr == nullptr && m_run)
                m_processingQueriesCondition.wait(lck);
            }

            //NEEDS_WORK_MST : Maybe we should prioritize the first processing query found
            if (processingQueryPtr != 0 && m_run)
            {
                processingQueryPtr->Run(threadId, *this);
#if 0
                if (s_preloadInQueryThread)
                {
                    bool doPreLoad = (((ScalableMesh<DPoint3d>*)processingQueryPtr->m_scalableMeshPtr.get())->GetMainIndexP()->IsTextured() == SMTextureType::Streaming) && processingQueryPtr->m_loadTexture;

                    if (processingQueryPtr->m_toLoadNodes[threadId].size() > 0 && doPreLoad)
                    {
                        ScalableMeshProgressiveQueryEngine::PreloadData((ScalableMesh<DPoint3d>*)processingQueryPtr->m_scalableMeshPtr.get(), processingQueryPtr->m_toLoadNodes[threadId], false);
                    }
                }

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
                    ProducedNodeContainer<DPoint3d, Extent3dType> producedFoundNodes;
                    processingQueryPtr->m_nodeQueryProcessorMutexes[threadId].lock();

                    bool doPreLoad = (((ScalableMesh<DPoint3d>*)processingQueryPtr->m_scalableMeshPtr.get())->GetMainIndexP()->IsTextured() == SMTextureType::Streaming) && processingQueryPtr->m_loadTexture;

                    if (doPreLoad)
                    {
                        processingQueryPtr->m_nodeQueryProcessors[threadId] = NodeQueryProcessor<DPoint3d, Extent3dType>::Create(nodePtr, processingQueryPtr->m_queryObjectP, 0, processingQueryPtr->m_loadTexture, &producedFoundNodes, threadId);
                    }
                    else
                    {
                        processingQueryPtr->m_nodeQueryProcessors[threadId] = NodeQueryProcessor<DPoint3d, Extent3dType>::Create(nodePtr, processingQueryPtr->m_queryObjectP, 0, processingQueryPtr->m_loadTexture, &processingQueryPtr->m_producedFoundNodes, threadId);
                    }
                    processingQueryPtr->m_nodeQueryProcessorMutexes[threadId].unlock();

                    processingQueryPtr->m_searchingNodeMutexes[threadId].lock();
                    processingQueryPtr->m_searchingNodes[threadId].pop_back();
                    processingQueryPtr->m_searchingNodeMutexes[threadId].unlock();

                    if (!processingQueryPtr->m_isCancel)
                    {
                        processingQueryPtr->m_nodeQueryProcessors[threadId]->DoQuery();

                        if (producedFoundNodes.GetNodes().size() > 0 && doPreLoad)
                        {
                            ScalableMeshProgressiveQueryEngine::PreloadData((ScalableMesh<DPoint3d>*)processingQueryPtr->m_scalableMeshPtr.get(), producedFoundNodes.GetNodes(), false);

                            for (auto& node : producedFoundNodes.GetNodes())
                            {
                                processingQueryPtr->m_producedFoundNodes.AddNode(node);
                            }
                        }

                        processingQueryPtr->m_nodeQueryProcessorMutexes[threadId].lock();
                        processingQueryPtr->m_nodeQueryProcessors[threadId] = 0;
                        processingQueryPtr->m_nodeQueryProcessorMutexes[threadId].unlock();
                    }

                    continue;
                }

                //Load unloaded node
                //HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> nodePtr;                

                while (processingQueryPtr->m_toLoadNodes[threadId].size() > 0)
                {
                    if (processingQueryPtr->m_toLoadNodes[threadId].size() > 0)
                    {
                        nodePtr = processingQueryPtr->m_toLoadNodes[threadId].back();
                    }

                    if (nodePtr != 0)
                    {
                        IScalableMeshCachedDisplayNodePtr meshNodePtr;

                        LoadNodeDisplayData(meshNodePtr, nodePtr, processingQueryPtr->m_loadTexture, processingQueryPtr->m_clipVisibilities, processingQueryPtr->m_scalableMeshPtr, processingQueryPtr->m_displayCacheManagerPtr);
                        processingQueryPtr->OnLoadedMeshNode(meshNodePtr, threadId);
#if 0                        
                        processingQueryPtr->m_foundMeshNodeMutexes[threadId].lock();
                        processingQueryPtr->m_foundMeshNodes[threadId].push_back(meshNodePtr);
                        processingQueryPtr->m_foundMeshNodeMutexes[threadId].unlock();

                        processingQueryPtr->m_toLoadNodeMutexes[threadId].lock();
                        processingQueryPtr->m_toLoadNodes[threadId].pop_back();
                        processingQueryPtr->m_toLoadNodeMutexes[threadId].unlock();
#endif
                    }
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

                        processor.LoadNodeDisplayData(meshNodePtr, consumedNodePtr, processingQueryPtr->m_loadTexture, processingQueryPtr->m_clipVisibilities, processingQueryPtr->m_scalableMeshPtr, processingQueryPtr->m_displayCacheManagerPtr);

                        processingQueryPtr->OnLoadedMeshNode(meshNodePtr, threadId);
#if 0
                        processingQueryPtr->m_foundMeshNodeMutexes[threadId].lock();
                        processingQueryPtr->m_foundMeshNodes[threadId].push_back(meshNodePtr);
                        processingQueryPtr->m_foundMeshNodeMutexes[threadId].unlock();

                        processingQueryPtr->m_toLoadNodeMutexes[threadId].lock();
                        processingQueryPtr->m_toLoadNodes[threadId].pop_back();
                        processingQueryPtr->m_toLoadNodeMutexes[threadId].unlock();
#endif
                    }
                    else
                    {
                        processingQueryPtr->m_toLoadNodeMutexes[threadId].unlock();
                    }
                }
            
#endif
            }

            } while (m_run);        
        }


    QueryProcessor::QueryProcessor()
        {        
#ifndef DEACTIVATE_THREADING
        m_numWorkingThreads = std::min((int)16, (int)(std::thread::hardware_concurrency() - 2));
        m_numWorkingThreads = max(1, m_numWorkingThreads);
#else
        m_numWorkingThreads = 1;        
#endif
        m_workingThreads = new std::thread[m_numWorkingThreads];
                
        m_run = false;
        m_processingQueryIndexes.resize(m_numWorkingThreads);

#ifndef LINUX_SCALABLEMESH_BUILD
        m_host = nullptr; 
#endif       
        }

    QueryProcessor::~QueryProcessor()
        {
        Stop();
        
        delete[] m_workingThreads;        
        }

    QueryProcessor::QueryHandle QueryProcessor::AddQuery(int                                                             queryId,
                  ISMPointIndexQuery<DPoint3d, Extent3dType>*                queryObjectP,
                  bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& searchingNodes,
                  bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& toLoadNodes,                  
                  bool                                                            loadTexture, 
                  const bset<uint64_t>&                                            clipVisibilities,
                  IScalableMeshPtr&                                                scalableMeshPtr,
                  IScalableMeshDisplayCacheManagerPtr&               displayCacheManagerPtr,
        ProcessingQuery<DPoint3d, Extent3dType>::Type                  type)
        {

#ifdef DISPLAYLOG
            fprintf(logger.GetFile(), "START NEW QUERY\n");
            fflush(logger.GetFile());                                                    
#endif

        ProcessingQuery<DPoint3d, Extent3dType>::Ptr processingQueryPtr(ProcessingQuery<DPoint3d, Extent3dType>::Create(queryId, m_numWorkingThreads, queryObjectP, searchingNodes, toLoadNodes, loadTexture, clipVisibilities, scalableMeshPtr, displayCacheManagerPtr, type));

        size_t currentNbProcessingQueries;

        std::unique_lock<std::mutex> lck(m_processingQueriesMutex);        

#ifndef NDEBUG
        for (auto& query : m_processingQueries)
            {
            if (query->m_queryId == queryId)
                assert(!"Query already processing");
            }
#endif   

        currentNbProcessingQueries = m_processingQueries.size();
        m_processingQueries.push_back(processingQueryPtr);        
        
        if (currentNbProcessingQueries == 0)
            {            
            Start();
            }        
        
        m_processingQueriesCondition.notify_all();
        return processingQueryPtr.get();
        }


    QueryProcessor::QueryHandle QueryProcessor::AddDependentQuery(QueryHandle sourceQuery,
        bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& searchingNodes,
        bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& toLoadNodes,
        bool                                                            loadTexture,
        ProcessingQuery<DPoint3d, Extent3dType>::Type                             type)
    {
        ProcessingQuery<DPoint3d, Extent3dType>* targetedQuery = (ProcessingQuery<DPoint3d, Extent3dType>*)sourceQuery;
        ProcessingQuery<DPoint3d, Extent3dType>::Ptr processingQueryPtr(ProcessingQuery<DPoint3d, Extent3dType>::Create(targetedQuery->m_queryId, m_numWorkingThreads, targetedQuery->m_queryObjectP->Clone(), searchingNodes, toLoadNodes, loadTexture, targetedQuery->m_clipVisibilities, targetedQuery->m_scalableMeshPtr, targetedQuery->m_displayCacheManagerPtr, type));


        targetedQuery->SetCollectCallback(new std::function<void(IScalableMeshCachedDisplayNodePtr&,size_t)>(std::bind(
            [](ProcessingQuery<DPoint3d, Extent3dType>* progressiveQueryP, IScalableMeshCachedDisplayNodePtr& meshNodePtr, size_t threadInd)
        {
            if (!progressiveQueryP->m_isCancel && !progressiveQueryP->m_foundMeshNodes.empty())
            {
                progressiveQueryP->m_foundMeshNodeMutexes[threadInd].lock();
                progressiveQueryP->m_foundMeshNodes[threadInd].push_back(meshNodePtr);
                progressiveQueryP->m_foundMeshNodeMutexes[threadInd].unlock();
                progressiveQueryP->m_toLoadNodeMutexes[threadInd].lock();
                progressiveQueryP->m_toLoadNodes[threadInd].push_back(dynamic_cast<ScalableMeshNode<DPoint3d>*>(meshNodePtr.get())->GetNodePtr());
                progressiveQueryP->m_toLoadNodeMutexes[threadInd].unlock();
            }
        }, processingQueryPtr.get(), std::placeholders::_1, std::placeholders::_2
            )));

        size_t currentNbProcessingQueries;

        std::unique_lock<std::mutex> lck(m_processingQueriesMutex);

#ifndef NDEBUG
        for (auto& query : m_processingQueries)
        {
            if (query->m_queryId == targetedQuery->m_queryId && query != targetedQuery)
                assert(!"Query already processing");
        }
#endif   

        currentNbProcessingQueries = m_processingQueries.size();
        m_processingQueries.push_back(processingQueryPtr);

        if (currentNbProcessingQueries == 0)
        {
            Start();
        }

        m_processingQueriesCondition.notify_all();
        return processingQueryPtr.get();
    }

    StatusInt QueryProcessor::CancelAllQueries()
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

        Stop();
    
        return SUCCESS;
        }
     
    StatusInt QueryProcessor::CancelQuery(int queryId)
        {              
#ifdef DISPLAYLOG 
        fprintf(logger.GetFile(), "STOP NEW QUERY\n");
        fflush(logger.GetFile());                                        
#endif

        ProcessingQuery<DPoint3d, Extent3dType>::Ptr toCancelQueryPtr;

        do
        {
            m_processingQueriesMutex.lock();
            toCancelQueryPtr = nullptr;

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
        } while (toCancelQueryPtr != 0);

        return SUCCESS;
        }


        bool QueryProcessor::IsQueryComplete(int queryId)
            {
            bool isQueryComplete = true;
            
            ProcessingQuery<DPoint3d, Extent3dType>::Ptr queryPtr;

            //m_processingQueriesMutex.lock();                    

            auto queryIter(m_processingQueries.begin());     
            const auto& queryIterEnd(m_processingQueries.end());     

            while (queryIter != queryIterEnd)
                {
                if ((*queryIter)->m_queryId == queryId)
                    {
                    queryPtr = *queryIter;                                        
                    //break;
                    isQueryComplete = isQueryComplete && queryPtr->IsComplete();
                    }

                queryIter++;
                }
            
            /*if (queryPtr != 0)
                {                
                isQueryComplete = queryPtr->IsComplete();
                }*/

            return isQueryComplete;
            }
            
        void QueryProcessor::Start()
            { 
#ifndef LINUX_SCALABLEMESH_BUILD
            if (m_host == nullptr)
                m_host = DgnPlatformLib::QueryHost();
#endif

            if (m_run == false)
                {
                m_run = true;

                //Launch a group of threads
                for (int threadId = 0; threadId < m_numWorkingThreads; ++threadId) 
                    {                                                        
                    if (!s_delayJoinThread)
                        {                
                        m_workingThreads[threadId] = std::thread(&QueryProcessor::QueryThread, this,
#ifndef LINUX_SCALABLEMESH_BUILD
 m_host,
#endif
 threadId);
                        }
                    else
                        {                                              
                        if (m_workingThreads[threadId].joinable())                            
                            m_workingThreads[threadId].join();

                        m_workingThreads[threadId] = std::thread(&QueryProcessor::QueryThread, this,
#ifndef LINUX_SCALABLEMESH_BUILD
 m_host, 
#endif
threadId);
                        }
                    }
                }
            }
        
        void QueryProcessor::Stop()
            {        
            
            {
            std::unique_lock<std::mutex> lck(m_processingQueriesMutex);            
            m_run = false;
            m_processingQueriesCondition.notify_all();
            }
            
            for (int threadId = 0; threadId < m_numWorkingThreads; ++threadId) 
                {
                try
                    {
                    if (m_workingThreads[threadId].joinable())
                        m_workingThreads[threadId].join();
                    }
                catch (...)
                    {//TFS #773063 - For some reason sometime the join is throwing an exception. For now just catch it and do nothing. 
                    }
                }                                
            }
       
        StatusInt QueryProcessor::GetFoundNodes(bvector<IScalableMeshCachedDisplayNodePtr>& foundNodes, int queryId)
        {
            StatusInt status = ERROR;

            std::list<ProcessingQuery<DPoint3d, Extent3dType>::Ptr>::iterator queryItr(m_processingQueries.begin());
            std::list<ProcessingQuery<DPoint3d, Extent3dType>::Ptr>::iterator queryItrEnd(m_processingQueries.end());

            while (queryItr != queryItrEnd)
            {
                if ((*queryItr)->m_queryId == queryId)
                {
                    if ((*queryItr)->m_alwaysReloadMeshNodes)
                        foundNodes.clear();
                    for (size_t threadIter = 0; threadIter < (*queryItr)->m_foundMeshNodes.size(); threadIter++)
                    {
                        (*queryItr)->m_foundMeshNodeMutexes[threadIter].lock();
#ifdef DISPLAYLOG                 
                        /*
                        fprintf(logger.GetFile(), "threadId %i : nb found nodes : %I64d\n", threadIter, (*queryItr)->m_foundMeshNodes[threadIter].size());
                        fflush(logger.GetFile());
                        */
#endif
                        foundNodes.insert(foundNodes.end(), (*queryItr)->m_foundMeshNodes[threadIter].begin(), (*queryItr)->m_foundMeshNodes[threadIter].end());
                        if(!(*queryItr)->m_alwaysReloadMeshNodes)
                            (*queryItr)->m_foundMeshNodes[threadIter].clear();
                        (*queryItr)->m_foundMeshNodeMutexes[threadIter].unlock();
                    }

                    status = SUCCESS;
                }
                ++queryItr;
            }
#if 0
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
                    foundNodes.insert(foundNodes.end(), (*queryItr)->m_foundMeshNodes[threadIter].begin(), (*queryItr)->m_foundMeshNodes[threadIter].end());
                    (*queryItr)->m_foundMeshNodes[threadIter].clear();
                    (*queryItr)->m_foundMeshNodeMutexes[threadIter].unlock();
                }

                status = SUCCESS;
            }
            else
            {
                status = ERROR;
            }
#endif
            return status;
        }

static QueryProcessor s_queryProcessor;

void IScalableMeshProgressiveQueryEngine::CancelAllQueries()
    {
    s_queryProcessor.CancelAllQueries();
    }

#define MAX_PRELOAD_OVERVIEW_LEVEL 1

void ScalableMeshProgressiveQueryEngine::UpdatePreloadOverview()
    {    
    for (auto& node : m_overviewNodes)
        {
        //Empty node are never loaded
        if (node->IsLoaded() == false) continue;
        
        if (!node->IsClippingUpToDate() || !node->HasCorrectClipping(m_activeClips) || node->HasInvertedClips() != m_smOverviews[&node - &m_overviewNodes[0]]->ShouldInvertClips())
            {
            node->RefreshMergedClip(m_smOverviews[&node - &m_overviewNodes[0]]->GetReprojectionTransform());
            node->RemoveDisplayDataFromCache();                    
            node->LoadMesh(false, m_activeClips, m_displayCacheManagerPtr, m_loadTexture, m_smOverviews[&node - &m_overviewNodes[0]]->ShouldInvertClips());
            assert(node->HasCorrectClipping(m_activeClips));                 
            }
        }   
    }

void ScalableMeshProgressiveQueryEngine::PreloadOverview(HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>& node, IScalableMesh* sMesh)
    {     
    ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNode(ScalableMeshCachedDisplayNode<DPoint3d>::Create(node, sMesh));
    assert(meshNode->IsLoaded(m_displayCacheManagerPtr.get(), m_loadTexture) == false || node->GetNbPoints() == 0);
    
    SMMeshIndexNode<DPoint3d, Extent3dType>* smNode = dynamic_cast<SMMeshIndexNode<DPoint3d, Extent3dType>*>(node.GetPtr());
    TRACEPOINT(THREAD_ID(), EventType::EVT_CREATE_DISPLAY_OVR_PRELOAD, node->GetBlockID().m_integerID, (uint64_t)-1, smNode->GetSingleTextureID(), -1, (uint64_t)meshNode.get(), -1)


    meshNode->RefreshMergedClip(sMesh->GetReprojectionTransform());
    meshNode->RemoveDisplayDataFromCache();                    
    meshNode->LoadMesh(false, m_activeClips, m_displayCacheManagerPtr, m_loadTexture, sMesh->ShouldInvertClips());
    assert(meshNode->IsLoaded(m_displayCacheManagerPtr.get(), m_loadTexture) == false || meshNode->HasCorrectClipping(m_activeClips));

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
 
ScalableMeshProgressiveQueryEngine::ScalableMeshProgressiveQueryEngine(IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr, bool loadTexture)
    {
    m_loadTexture = loadTexture;
    m_displayCacheManagerPtr = displayCacheManagerPtr;            
        
    HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>> rootNodePtr(((ScalableMesh<DPoint3d>*)scalableMeshPtr.get())->GetRootNode());

    bvector<uint64_t> allShownIds;
    scalableMeshPtr->GetAllClipIds(allShownIds);

    bset<uint64_t> activeClips;
    for (auto&id : allShownIds) activeClips.insert(id);
    _SetActiveClips(activeClips, scalableMeshPtr);

    if (rootNodePtr == nullptr) return;
	if (std::find(m_smOverviews.begin(), m_smOverviews.end(), scalableMeshPtr.get()) == m_smOverviews.end())
		PreloadOverview(rootNodePtr, scalableMeshPtr.get());

    if (rootNodePtr->GetMinResolution() == 0)
        {
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

    m_planners.push_back(new QueryPlanner());
    m_planners.push_back(new ContourQueryPlanner());
    }

ScalableMeshProgressiveQueryEngine::~ScalableMeshProgressiveQueryEngine()
    {    
    //NEEDS_WORK_SM : Need to cancel only for particular ScalableMesh
    s_queryProcessor.CancelAllQueries();    
    m_overviewNodes.clear();
    m_smOverviews.clear();
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

    SMMeshIndexNode<DPoint3d, Extent3dType>* smNode = dynamic_cast<SMMeshIndexNode<DPoint3d, Extent3dType>*>(parentNodePtr.GetPtr());
    TRACEPOINT(THREAD_ID(), EventType::EVT_CREATE_DISPLAY_OVR_1, parentNodePtr->GetBlockID().m_integerID,(uint64_t)-1, smNode->GetSingleTextureID(), -1, (uint64_t)meshNodePtr.get(), -1)

            if (!meshNodePtr->IsLoadedInVRAM(displayCacheManagerPtr.get(), loadTexture) || ((!meshNodePtr->IsClippingUpToDate() || !meshNodePtr->HasCorrectClipping(clipVisibilities)) && !s_keepSomeInvalidate)
                || (scalableMeshPtr->ShouldInvertClips() != meshNodePtr->HasInvertedClips()))
            {
                if (!(meshNodePtr->IsLoaded(displayCacheManagerPtr.get(), loadTexture) && (meshNodePtr->GetLevel() == 0 || smNode->GetParentNodePtr()->GetNbPoints() < 4)))
                {
                    return FindOverview(lowerResOverviewNodes, extentToCover/*meshNodePtr->GetContentExtent()*/, parentNodePtr, loadTexture, clipVisibilities, scalableMeshPtr, displayCacheManagerPtr);
                }
            }
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
            clipVector = ClipVector::CreateFromPrimitive(clipPrimitive.get());
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

    
        bvector<bvector<IScalableMeshCachedDisplayNodePtr>>                m_lowerResOverviewNodes;
        bvector<bvector<IScalableMeshCachedDisplayNodePtr>>                m_requiredMeshNodes;    
        bvector<bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>> m_toLoadNodes;
#ifndef LINUX_SCALABLEMESH_BUILD
        DgnPlatformLib::Host* m_host;
#endif
                        
        int          m_numWorkingThreads;
        std::thread* m_workingThreads;    

    public : 

        NewQueryStartingNodeProcessor()
            {
            
#ifndef DEACTIVATE_THREADING
            m_numWorkingThreads = std::min((int)16, (int)(std::thread::hardware_concurrency() - 2));
            m_numWorkingThreads = max(1, m_numWorkingThreads);
#else
            m_numWorkingThreads = 1;            
#endif
            m_lowerResOverviewNodes.resize(m_numWorkingThreads);
            m_requiredMeshNodes.resize(m_numWorkingThreads);        
            m_toLoadNodes.resize(m_numWorkingThreads);
            m_workingThreads = new std::thread[m_numWorkingThreads];
#ifndef LINUX_SCALABLEMESH_BUILD
            m_host = nullptr;
#endif
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

        void QueryThread(
#ifndef LINUX_SCALABLEMESH_BUILD
DgnPlatformLib::Host* hostToAdopt, 
#endif
size_t threadId, IScalableMeshPtr* scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr* displayCacheManagerPtr)
            {  
#ifdef VANCOUVER_API
            DgnPlatformLib::AdoptHost(*hostToAdopt);
#else
            //assert(!"No AdoptHost on BIM0200 - Untested behavior");
#endif     
                  
            m_lowerResOverviewNodes[threadId].clear();
            m_toLoadNodes[threadId].clear();
            m_requiredMeshNodes[threadId].clear();
            for (size_t nodeInd = m_nodeToSearchCurrentInd + 1; nodeInd < m_nodesToSearch->GetNodes().size(); nodeInd++)        
                {                              
                if (nodeInd % m_numWorkingThreads != threadId) continue;

                if (!m_nodesToSearch->GetNodes()[nodeInd]->IsLoaded())
                    {
                    DRange3d range3d(DRange3d::NullRange());

                    FindOverview(m_lowerResOverviewNodes[threadId], range3d, m_nodesToSearch->GetNodes()[nodeInd], m_newQuery->m_loadTexture, *m_activeClips, *scalableMeshPtr, *displayCacheManagerPtr);
                    }
                else
                    {                
                    ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNodePtr(ScalableMeshCachedDisplayNode<DPoint3d>::Create(m_nodesToSearch->GetNodes()[nodeInd], scalableMeshPtr->get()));
              
                    if (!meshNodePtr->IsLoadedInVRAM(displayCacheManagerPtr->get(), m_newQuery->m_loadTexture) || ((!meshNodePtr->IsClippingUpToDate() || !meshNodePtr->HasCorrectClipping(*m_activeClips)) && !s_keepSomeInvalidate)
                        || ((*scalableMeshPtr)->ShouldInvertClips() !=  meshNodePtr->HasInvertedClips()))
                        {  
                        DRange3d range3d = meshNodePtr->GetNodeExtent();          
                        FindOverview(m_lowerResOverviewNodes[threadId], range3d, m_nodesToSearch->GetNodes()[nodeInd], m_newQuery->m_loadTexture, *m_activeClips, *scalableMeshPtr, *displayCacheManagerPtr);
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
                
                ScalableMeshCachedDisplayNode<DPoint3d>::Ptr meshNodePtr(ScalableMeshCachedDisplayNode<DPoint3d>::Create(m_foundNodes->GetNodes()[nodeInd], scalableMeshPtr->get()));
                              
                if (!meshNodePtr->IsLoadedInVRAM(displayCacheManagerPtr->get(), m_newQuery->m_loadTexture) || ((!meshNodePtr->IsClippingUpToDate() || !meshNodePtr->HasCorrectClipping(*m_activeClips)) && !s_keepSomeInvalidate)
                    || ((*scalableMeshPtr)->ShouldInvertClips() != meshNodePtr->HasInvertedClips()))
                    {        
                    DRange3d range3d = meshNodePtr->GetNodeExtent();          
                    FindOverview(m_lowerResOverviewNodes[threadId], range3d, m_foundNodes->GetNodes()[nodeInd], m_newQuery->m_loadTexture, *m_activeClips, *scalableMeshPtr, *displayCacheManagerPtr);
                                        
                    m_toLoadNodes[threadId].push_back(m_foundNodes->GetNodes()[nodeInd]);
                    }
                else
                    {
                    //NEEDS_WORK_SM : Should not be duplicated.
                    assert(meshNodePtr->IsLoaded());

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
#ifndef LINUX_SCALABLEMESH_BUILD
            if (m_host == nullptr)
                {
                m_host = DgnPlatformLib::QueryHost();
                }
#endif
            
            m_nodesToSearch = &nodesToSearch;
            m_nodeToSearchCurrentInd = nodeToSearchCurrentInd;
            m_foundNodes = &foundNodes;        
            m_newQuery = &newQuery;
            m_activeClips = &activeClips;

            for (size_t threadId = 0; threadId < m_numWorkingThreads; ++threadId) 
                {                                                                               
                m_workingThreads[threadId] = std::thread(&NewQueryStartingNodeProcessor::QueryThread, this,
#ifndef LINUX_SCALABLEMESH_BUILD
 m_host, 
#endif
threadId, &scalableMeshPtr, &displayCacheManagerPtr);  
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

bool s_loadNodeNearCamFirst = true;

void ScalableMeshProgressiveQueryEngine::SortOverviews(bvector<IScalableMeshCachedDisplayNodePtr>& overviews, ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP)
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

    if (overviews.size() > 0)
    {
        for (auto& node : overviews) nodes.push_back(dynamic_cast<ScalableMeshNode<DPoint3d>*>(node.get())->GetNodePtr());
        queryObjectP->GetQueryNodeOrder(queryNodeOrder, nodes[0], &nodes[0], nodes.size());

        for (auto& node : overviews) mapNodes[node.get()] = &node - &overviews[0];
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

            return (maxLengthA - maxLengthB) + ((int)scoreA - (int)scoreB)*(maxLengthA - maxLengthB) / 6 < 0;
        }

        OverviewScoringMethod(vector<size_t>& scores, bmap<IScalableMeshCachedDisplayNode*, size_t>& map) : m_scores(scores), m_map(map)
        {

        }
    } OverviewScore(queryNodeOrder, mapNodes);
    std::sort(overviews.begin(), overviews.end(), OverviewScore);
}
    
void ScalableMeshProgressiveQueryEngine::StartNewQuery(RequestedQuery& newQuery, ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP, const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& startingNodes, IScalableMeshViewDependentMeshQueryParamsPtr queryParam)
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
        if (!nodesToSearch.GetNodes()[currentInd]->IsLoaded() /*&& !s_doPreLoad*/)
        {
            currentInd--; //TFS# 669028 -- otherwise the current node never gets loaded
            break;
        }

        nodesToSearch.GetNodes()[currentInd]->QueryVisibleNode (queryObjectP, s_maxLevel, overviewNodes, foundNodes, nodesToSearch, nullptr);
        
        if ((clock() - startTime) > s_firstNodeSearchingDelay /*&& !s_doPreLoad*/)
            {                    
            break;
            }
        
        currentInd++;
        }        
 
#if 0
    bvector<IScalableMeshCachedDisplayNodePtr>                     lowerResOverviewNodes;
    bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>> searchingNodes;
    bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>> toLoadNodes;
              
    ComputeOverviewSearchToLoadNodes(newQuery, lowerResOverviewNodes, searchingNodes, toLoadNodes, nodesToSearch, currentInd, foundNodes, m_activeClips, newQuery.m_meshToQuery, m_displayCacheManagerPtr);

    //assert(lowerResOverviewNodes.size() > 0 || (nodesToSearch.GetNodes().size() - currentInd - 1) == 0);

    newQuery.m_overviewMeshNodes.insert(newQuery.m_overviewMeshNodes.end(), lowerResOverviewNodes.begin(), lowerResOverviewNodes.end());                    
#endif
    QueryPlan* nextQueryOrQueries = GetQueryPlanner(newQuery)->CreatePlanForNextQuery(m_displayCacheManagerPtr, m_activeClips);
    GetQueryPlanner(newQuery)->FetchOverviewsAndPlanNextQuery(newQuery, *nextQueryOrQueries, queryObjectP,currentInd, nodesToSearch, foundNodes);

    //PRE 
#if 0
    if (!s_preloadInQueryThread)
        { 
        if (toLoadNodes.size() > 0 && (newQuery).m_loadTexture)
            {         
            ScalableMesh<DPoint3d>* smP(dynamic_cast<ScalableMesh<DPoint3d>*>(newQuery.m_meshToQuery.get()));

            ScalableMeshProgressiveQueryEngine::PreloadData(smP, toLoadNodes, true);
            }
        }
    else    
#endif
        {
        ScalableMesh<DPoint3d>* smP(dynamic_cast<ScalableMesh<DPoint3d>*>(newQuery.m_meshToQuery.get()));
        ScalableMeshProgressiveQueryEngine::CancelPreload(smP);
        }
    //PRE


    if (s_sortOverviewBySize == true)
        {
        SortOverviews(newQuery.m_overviewMeshNodes, queryObjectP);
        }

#if 0  
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
#endif

    GetQueryPlanner(newQuery)->AddQueriesInPlan(*nextQueryOrQueries, queryObjectP, s_queryProcessor, queryParam, newQuery);
    delete nextQueryOrQueries;
    }


void ScalableMeshProgressiveQueryEngine::_SetActiveClips(const bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr)
    {
    m_activeClips = activeClips;
    UpdatePreloadOverview();
    }


void ScalableMeshProgressiveQueryEngine::_GetActiveClips(bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr)
{
    activeClips = m_activeClips;
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
    requestedQuery.m_loadContours = queryParam->ShouldLoadContours();

    ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP;

    BuildQueryObject<DPoint3d>(queryObjectP, 0/*pQueryExtentPts*/, 0/*nbQueryExtentPts*/, queryParam, smPtr.get());

    assert(queryObjectP != 0);
      
    StartNewQuery(requestedQuery, queryObjectP, startingNodes, queryParam);

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
    BentleyStatus status;
    for (auto& query : m_requestedQueries)
        {
        if (query.m_queryId == queryId)
            {
            requestedQueryP = &query;
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
        }


    /*if (requestedQueryP != 0)
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
    else*/
    if(requestedQueryP == 0)
        {
        status = ERROR;
        }

    return status;
    }

BentleyStatus ScalableMeshProgressiveQueryEngine::_GetRequiredTextureTiles(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::SMRasterTile>& rasterTiles,
                                                                           int                                                   queryId) const
    {    
    bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr> meshNodes;

    BentleyStatus status = _GetRequiredNodes(meshNodes, queryId);
                        
    if (status != SUCCESS)
        return status;

    bvector<DRange3d> tileRanges;

    for (auto& nodes : meshNodes)
        {
        //tileRanges.push_back(loadNode->GetContentExtent());
        tileRanges.push_back(nodes->GetNodeExtent());
        }


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
        return ERROR;
        }


    ScalableMesh<DPoint3d>* smP((ScalableMesh<DPoint3d>*)requestedQueryP->m_meshToQuery.get());

    ISMDataStoreTypePtr<Extent3dType> dataStore(smP->m_scmIndexPtr->GetDataStore());

#ifdef VANCOUVER_API
    //s_imageppCopyFromLock.lock();
#endif
       
    dataStore->ComputeRasterTiles(rasterTiles, tileRanges);
    
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
            {
                if (!requestedQueryP->m_isQueryCompleted)
                {
                    //NEEDS_WORK_SM_PROGRESSIVE : Should have auto notification
                    requestedQueryP->m_isQueryCompleted = s_queryProcessor.IsQueryComplete(queryId);
                }

                isQueryComplete = requestedQueryP->m_isQueryCompleted;
            }
            //break;
        }
    }

    if (requestedQueryP == 0)
    {
        isQueryComplete = true;
    }
 /*   else
    {
        if (!requestedQueryP->m_isQueryCompleted)
        {
            //NEEDS_WORK_SM_PROGRESSIVE : Should have auto notification
            requestedQueryP->m_isQueryCompleted = s_queryProcessor.IsQueryComplete(queryId);
        }

        isQueryComplete = requestedQueryP->m_isQueryCompleted;
    }*/

    return isQueryComplete;
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
