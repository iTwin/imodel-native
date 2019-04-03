#pragma once
#include <GeoCoord/BaseGeoCoord.h>
#include "SMPointIndex.h"
#include "InternalUtilityFunctions.h"
#include <ScalableMesh/IScalableMeshClipContainer.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <DgnPlatform/DgnPlatformLib.h>

#include <ScalableMesh/IScalableMeshQuery.h>

#include "ScalableMesh.h"

#if !defined(VANCOUVER_API) && !defined(DGNDB06_API)
USING_NAMESPACE_BENTLEY_DGN
#endif

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
typedef DRange3d Extent3dType;

class QueryProcessor;

template <class POINT, class EXTENT> class NodeQueryProcessor : public RefCountedBase, public IStopQuery
{
private:

    HFCPtr<SMPointIndexNode<POINT, EXTENT>>          m_queryNode;
    ISMPointIndexQuery<POINT, EXTENT>*               m_pQueryObject;
    StopQueryCallbackFP                              m_stopQueryCallbackFP;
    ProducedNodeContainer<POINT, EXTENT>*            m_foundNodesP;
    atomic<bool>                                     m_stopQuery;
    bool                                             m_loadTexture;
    bool m_invertClip;
    int                                              m_threadId;
    const bset<uint64_t>& m_activeClips;
    size_t                                           m_maxLevel;
    bool                                             m_useMaxLevel;



    NodeQueryProcessor(HFCPtr<SMPointIndexNode<POINT, EXTENT>> queryNode,
        ISMPointIndexQuery<POINT, EXTENT>*      queryObject,
        StopQueryCallbackFP                     stopQueryCallbackFP,
        bool                                    loadTexture,
        bool invertClip,
        ProducedNodeContainer<POINT, EXTENT>*   foundNodesP,
        int                                     threadId,
        const bset<uint64_t>& activeClips)
        :m_activeClips(activeClips)
    {
        m_queryNode = queryNode;
        m_pQueryObject = queryObject;
        m_stopQueryCallbackFP = stopQueryCallbackFP;
        m_stopQuery = false;
        m_loadTexture = loadTexture;
        m_invertClip = invertClip;
        m_threadId = threadId;
        m_foundNodesP = foundNodesP;
        m_useMaxLevel = false;
    }

    NodeQueryProcessor(HFCPtr<SMPointIndexNode<POINT, EXTENT>> queryNode,
        ISMPointIndexQuery<POINT, EXTENT>*      queryObject,
        StopQueryCallbackFP                     stopQueryCallbackFP,
        bool                                    loadTexture,
        bool invertClip,
        ProducedNodeContainer<POINT, EXTENT>*   foundNodesP,
        int                                     threadId,
        const bset<uint64_t>& activeClips,
        size_t maxLevel)
        :m_activeClips(activeClips)
    {
        m_queryNode = queryNode;
        m_pQueryObject = queryObject;
        m_stopQueryCallbackFP = stopQueryCallbackFP;
        m_stopQuery = false;
        m_loadTexture = loadTexture;
        m_invertClip = invertClip;
        m_threadId = threadId;
        m_foundNodesP = foundNodesP;
        m_maxLevel = maxLevel;
        m_useMaxLevel = true;
    }

public:

    typedef RefCountedPtr<NodeQueryProcessor> Ptr;

    virtual bool DoStop() const override
    {
        return m_stopQuery;
    }

    void DoQuery()
    {
        if (m_useMaxLevel)
            m_pQueryObject->ToggleLimitToLevel(m_useMaxLevel, m_maxLevel);
        if (!m_stopQuery)
            m_queryNode->Query(m_pQueryObject, *m_foundNodesP, this);
        m_pQueryObject->ToggleLimitToLevel(false, m_maxLevel);
    }

    void SetStopQuery(bool stopQuery)
    {
        m_stopQuery = stopQuery;
    }

    static Ptr Create(HFCPtr<SMPointIndexNode<POINT, EXTENT>> queryNode,
        ISMPointIndexQuery<POINT, EXTENT>*      queryObject,
        StopQueryCallbackFP                     stopQueryCallbackFP,
        bool                                    loadTexture,
        bool invertClip,
        ProducedNodeContainer<POINT, EXTENT>*   foundNodesP,
        int                                     threadId,
        const bset<uint64_t>& activeClips)
    {
        return new NodeQueryProcessor(queryNode, queryObject, stopQueryCallbackFP, loadTexture, invertClip, foundNodesP, threadId, activeClips);
    }

    static Ptr Create(HFCPtr<SMPointIndexNode<POINT, EXTENT>> queryNode,
        ISMPointIndexQuery<POINT, EXTENT>*      queryObject,
        StopQueryCallbackFP                     stopQueryCallbackFP,
        bool                                    loadTexture,
        bool invertClip,
        ProducedNodeContainer<POINT, EXTENT>*   foundNodesP,
        int                                     threadId,
        const bset<uint64_t>& activeClips,
        size_t maxLevel)
    {
        return new NodeQueryProcessor(queryNode, queryObject, stopQueryCallbackFP, loadTexture, invertClip, foundNodesP, threadId, activeClips, maxLevel);
    }
};

template <class POINT, class EXTENT> struct ProcessingQuery : public RefCountedBase
{
public:
    typedef RefCountedPtr<ProcessingQuery<POINT, EXTENT>> Ptr;

protected:
    ProcessingQuery(int                                               queryId,
        int                                               nbWorkingThreads,
        ISMPointIndexQuery<POINT, EXTENT>*                queryObjectP,
        bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>& searchingNodes,
        bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>& toLoadNodes,
        bool                                              loadTexture,
        const bset<uint64_t>&                              clipVisibilities,
        IScalableMeshPtr&                              scalableMeshPtr,
        IScalableMeshDisplayCacheManagerPtr&               displayCacheManagerPtr)
:m_clipVisibilities(clipVisibilities)
    {
        m_queryId = queryId;
        m_searchingNodes.resize(nbWorkingThreads);
        m_scalableMeshPtr = scalableMeshPtr;
        m_displayCacheManagerPtr = displayCacheManagerPtr;
        m_collectCallback = nullptr;
        m_alwaysReloadMeshNodes = false;

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
        m_updatedOverviewNodes.resize(nbWorkingThreads);
        m_updatedOverviewNodeMutexes = new std::mutex[nbWorkingThreads];
        m_nodeQueryProcessors.resize(nbWorkingThreads);
        m_nodeQueryProcessorMutexes = new std::mutex[nbWorkingThreads];
        m_dirtyOverviews.resize(nbWorkingThreads);
        m_newOverviews.resize(nbWorkingThreads);
        m_dirtyOverviewNodeMutexes = new std::mutex[nbWorkingThreads];

        m_queryObjectP = queryObjectP;
        m_isCancel = false;
        m_loadTexture = loadTexture;
        m_producedFoundNodes.SetThreadSafe(true);
    }

public:

    enum class Type
    {
        LoadMesh = 0,
        Contour
    };

    typedef std::function<void(IScalableMeshCachedDisplayNodePtr&, size_t)> CollectLoadedNodesCallback;

    ~ProcessingQuery()
    {
        delete[] m_searchingNodeMutexes;
        delete[] m_toLoadNodeMutexes;
        delete[] m_foundMeshNodeMutexes;
        delete[] m_updatedOverviewNodeMutexes;
        delete[] m_nodeQueryProcessorMutexes;
        delete[] m_dirtyOverviewNodeMutexes;
        delete m_queryObjectP;
        if (nullptr != m_collectCallback)
            delete m_collectCallback;
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
        IScalableMeshDisplayCacheManagerPtr&               displayCacheManagerPtr,
        typename ProcessingQuery<POINT, EXTENT>::Type                              queryType
    );

    void AddOverviewsToRecompute(bvector<RefCountedPtr<ScalableMeshCachedDisplayNode<DPoint3d>>>& dirtyOverviews) 
    {
        for (size_t nodeId = 0; nodeId < dirtyOverviews.size(); nodeId++)
        {
            m_dirtyOverviews[nodeId % m_dirtyOverviews.size()].push_back(RefCountedPtr<IScalableMeshCachedDisplayNode>(&*dirtyOverviews[nodeId]));
        }
    }

    virtual void Run(size_t threadInd, QueryProcessor& processor);

    void OnLoadedMeshNode(IScalableMeshCachedDisplayNodePtr& meshNodePtr, size_t threadInd)
    {
        if (nullptr != m_collectCallback)
        {
            (*m_collectCallback)(meshNodePtr, threadInd);
        }
        m_foundMeshNodeMutexes[threadInd].lock();
        m_foundMeshNodes[threadInd].push_back(meshNodePtr);
        m_foundMeshNodeMutexes[threadInd].unlock();

        m_toLoadNodeMutexes[threadInd].lock();
        m_toLoadNodes[threadInd].pop_back();
        m_toLoadNodeMutexes[threadInd].unlock();
    }

    void OnLoadedOverviewNode(IScalableMeshCachedDisplayNodePtr& overviewNodePtr, size_t threadInd)
    {
        m_updatedOverviewNodeMutexes[threadInd].lock();
        m_updatedOverviewNodes[threadInd].push_back(overviewNodePtr);
        m_updatedOverviewNodeMutexes[threadInd].unlock();
    }

    void SetCollectCallback(CollectLoadedNodesCallback* func)
    {
        if (nullptr != m_collectCallback)
            delete m_collectCallback;
        m_collectCallback = func;
    }

    int                                                       m_queryId;
    bvector<bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>> m_searchingNodes;
    std::mutex*                                               m_searchingNodeMutexes;
    bvector<bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>> m_toLoadNodes;
    std::mutex*                                               m_toLoadNodeMutexes;
    ProducedNodeContainer<POINT, EXTENT>                      m_producedFoundNodes;
    bvector<bvector<IScalableMeshCachedDisplayNodePtr>>       m_foundMeshNodes;
    std::mutex*                                               m_foundMeshNodeMutexes;

    bvector<bvector<IScalableMeshCachedDisplayNodePtr>>       m_updatedOverviewNodes;
    bvector<bvector<IScalableMeshCachedDisplayNodePtr>>       m_dirtyOverviews;
    bvector<bvector<IScalableMeshCachedDisplayNodePtr>>       m_newOverviews;
    std::mutex*                                               m_updatedOverviewNodeMutexes;
    std::mutex*                                               m_dirtyOverviewNodeMutexes;

    bvector<NodeQueryProcessor<DPoint3d, Extent3dType>::Ptr>  m_nodeQueryProcessors;
    std::mutex*                                                    m_nodeQueryProcessorMutexes;

    ISMPointIndexQuery<POINT, EXTENT>*  m_queryObjectP;
    atomic<bool>                        m_isCancel;
    bool                                m_loadTexture;
    const bset<uint64_t>&                                       m_clipVisibilities;
    IScalableMeshPtr                    m_scalableMeshPtr;
    IScalableMeshDisplayCacheManagerPtr m_displayCacheManagerPtr;
    CollectLoadedNodesCallback* m_collectCallback;
    bool m_alwaysReloadMeshNodes;
};

class QueryProcessor
{
    friend struct ProcessingQuery<DPoint3d, Extent3dType>;
public:

private:

    typedef std::list<ProcessingQuery<DPoint3d, Extent3dType>::Ptr> ProcessingQueryList;
    typedef ProcessingQuery<DPoint3d, Extent3dType>* QueryHandle;

    bvector<int>                  m_processingQueryIndexes;
    ProcessingQueryList           m_processingQueries;
    std::mutex                    m_processingQueriesMutex;
    std::condition_variable       m_processingQueriesCondition;
    std::atomic<bool>                  m_run;

    int                           m_numWorkingThreads;
    std::thread*                  m_workingThreads;

#ifndef LINUX_SCALABLEMESH_BUILD        
    DgnPlatformLib::Host*         m_host;
#endif

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


    void LoadNodeDisplayData(IScalableMeshCachedDisplayNodePtr& meshNodePtr, HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>& visibleNode, bool loadTexture, const bset<uint64_t>& clipVisibilities, IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr);

    void QueryThread(
#ifndef LINUX_SCALABLEMESH_BUILD
        DgnPlatformLib::Host* hostToAdopt,
#endif
        int threadId);
 


public:

    QueryProcessor();

    virtual ~QueryProcessor();

    QueryHandle AddQuery(int                                                             queryId,
        ISMPointIndexQuery<DPoint3d, Extent3dType>*                queryObjectP,
        bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& searchingNodes,
        bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& toLoadNodes,
        bool                                                            loadTexture,
        const bset<uint64_t>&                                            clipVisibilities,
        IScalableMeshPtr&                                                scalableMeshPtr,
        IScalableMeshDisplayCacheManagerPtr&               displayCacheManagerPtr,
        bvector<RefCountedPtr<ScalableMeshCachedDisplayNode<DPoint3d>>>&              dirtyOverviews,
        ProcessingQuery<DPoint3d, Extent3dType>::Type                             type = ProcessingQuery<DPoint3d, Extent3dType>::Type::LoadMesh);

    QueryHandle AddDependentQuery(QueryHandle sourceQuery,
        bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& searchingNodes,
        bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& toLoadNodes,
        bool                                                            loadTexture,
        typename ProcessingQuery<DPoint3d, Extent3dType>::Type  type = ProcessingQuery<DPoint3d, Extent3dType>::Type::LoadMesh);

    StatusInt CancelAllQueries();

    StatusInt CancelQuery(int queryId);

    bool IsQueryComplete(int queryId);

    void Start();

    void Stop();

    StatusInt GetFoundNodes(bvector<IScalableMeshCachedDisplayNodePtr>& foundNodes, int queryId);

    StatusInt GetUpdatedOverviewNodes(bvector<IScalableMeshCachedDisplayNodePtr>& overviewNodes, int queryId);
    StatusInt UpdateDirtyOverviews(bvector<RefCountedPtr<ScalableMeshCachedDisplayNode<DPoint3d>>>& staleOverviews, int queryId);
};


END_BENTLEY_SCALABLEMESH_NAMESPACE

