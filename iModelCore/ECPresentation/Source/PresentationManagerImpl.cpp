/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>
#include "Shared/ECExpressions/ECExpressionContextsProvider.h"
#include "Shared/Queries/QueryExecutor.h"
#include "Shared/Queries/QueryBuilder.h"
#include "Shared/CustomizationHelper.h"
#include "Shared/NodeLabelCalculator.h"
#include "Shared/RulesPreprocessor.h"
#include "Shared/UsedClassesListener.h"
#include "Content/ContentProviders.h"
#include "Content/ContentClassesLocater.h"
#include "Hierarchies/NavNodeProviders.h"
#include "Hierarchies/NavNodesDataSource.h"
#include "Hierarchies/HierarchiesComparer.h"
#include "Hierarchies/HierarchiesFiltering.h"
#include "Hierarchies/NavNodesCacheWrapper.h"
#include "PresentationManagerImpl.h"
#include "UpdateHandler.h"

#define VALID_HIERARCHY_CACHE_PRECONDITION(cache, resultOnFailure) \
    if (nullptr == cache) \
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Hierarchies cache not found for current task");

//=======================================================================================
// @bsiclass
//=======================================================================================
struct CompositeUpdateRecordsHandler : IUpdateRecordsHandler
{
private:
    bvector<std::shared_ptr<IUpdateRecordsHandler>> m_handlers;
    mutable BeMutex m_mutex;
protected:
    void _Start() override {BeMutexHolder lock(m_mutex); std::for_each(m_handlers.begin(), m_handlers.end(), [](auto const& h){h->Start();});}
    void _Accept(HierarchyUpdateRecord const& record) override {BeMutexHolder lock(m_mutex); std::for_each(m_handlers.begin(), m_handlers.end(), [&record](auto const& h){h->Accept(record);});}
    void _Accept(FullUpdateRecord const& record) override {BeMutexHolder lock(m_mutex); std::for_each(m_handlers.begin(), m_handlers.end(), [&record](auto const& h){h->Accept(record);});}
    void _Finish() override {BeMutexHolder lock(m_mutex); std::for_each(m_handlers.begin(), m_handlers.end(), [](auto const& h){h->Finish();});}
public:
    CompositeUpdateRecordsHandler(bvector<std::shared_ptr<IUpdateRecordsHandler>> handlers)
        : m_handlers(handlers)
        {}
    void Register(std::shared_ptr<IUpdateRecordsHandler> handler)
        {
        BeMutexHolder lock(m_mutex);
        m_handlers.push_back(handler);
        }
    bool Unregister(IUpdateRecordsHandler& handler)
        {
        BeMutexHolder lock(m_mutex);
        for (auto iter = m_handlers.begin(); iter != m_handlers.end(); ++iter)
            {
            if (iter->get() == &handler)
                {
                m_handlers.erase(iter);
                return true;
                }
            }
        return false;
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImpl::UsedClassesListener : IECDbUsedClassesListener
    {
    RulesDrivenECPresentationManagerImpl& m_manager;
    UsedClassesListener(RulesDrivenECPresentationManagerImpl& manager) : m_manager(manager) {}
    void _OnClassUsed(ECDbCR db, ECClassCR ecClass, bool polymorphically) override
        {
        for (std::shared_ptr<ECInstanceChangeEventSource> const& source : m_manager.GetECInstanceChangeEventSources())
            source->NotifyClassUsed(db, ecClass, polymorphically);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static PresentationRuleSetPtr FindRuleset(IRulesetLocaterManager const& locaters, IConnectionCR connection, Utf8CP rulesetId)
    {
    auto scope = Diagnostics::Scope::Create("Find ruleset");
    PresentationRuleSetPtr ruleset = RulesPreprocessor::GetPresentationRuleSet(locaters, connection, rulesetId);
    if (!ruleset.IsValid())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_INFO, LOG_ERROR, Utf8PrintfString("Ruleset with ID '%s' not found", rulesetId));
        return nullptr;
        }
    return ruleset;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void RegisterDisplayLabelRuleset(IRulesetLocaterManager& locaters)
    {
    RefCountedPtr<SimpleRuleSetLocater> locater = SimpleRuleSetLocater::Create();
    locaters.RegisterLocater(*locater);

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(DISPLAY_LABEL_RULESET_ID);
    locater->AddRuleSet(*ruleset);

    auto rule = new ContentRule();
    rule->AddSpecification(*new SelectedNodeInstancesSpecification());
    ruleset->AddPresentationRule(*rule);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImpl::RulesetECExpressionsCache : IECExpressionsCacheProvider
{
private:
    bmap<Utf8String, ECExpressionsCache*> m_caches;
    mutable BeMutex m_mutex;
protected:
    ECExpressionsCache& _Get(Utf8CP rulesetId) override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_caches.find(rulesetId);
        if (m_caches.end() == iter)
            iter = m_caches.Insert(rulesetId, new ECExpressionsCache()).first;
        return *iter->second;
        }
public:
    ~RulesetECExpressionsCache()
        {
        BeMutexHolder lock(m_mutex);
        for (auto pair : m_caches)
            delete pair.second;
        }
    void Clear(Utf8CP rulesetId)
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_caches.find(rulesetId);
        if (m_caches.end() != iter)
            {
            delete iter->second;
            m_caches.erase(iter);
            }
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesetUsedClassesNotificationFlags
{
private:
    mutable BeMutex m_mutex;
    bset<Utf8String> m_rulesetIds;
public:
    bool Add(Utf8String id)
        {
        BeMutexHolder lock(m_mutex);
        if (m_rulesetIds.end() != m_rulesetIds.find(id))
            return false;
        m_rulesetIds.insert(id);
        return true;
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesCacheManager : INodesCacheManager, IConnectionsListener
{
    typedef RulesDrivenECPresentationManagerImpl::Params::CachingParams::Mode CacheMode;

private:
    BeFileName m_cacheDirectory;
    uint64_t m_cacheSizeLimit;
    Nullable<uint64_t> m_diskCacheMemoryCacheSize;

    NavNodesFactoryCR m_nodeFactory;
    INodesProviderContextFactoryCR m_contextFactory;
    INodesProviderFactoryCR m_providersFactory;
    IConnectionManagerCR m_connections;

    bmap<Utf8String, std::shared_ptr<NodesCache::DbFactory>> m_initializedCaches;
    mutable BeMutex m_mutex;

private:
    std::shared_ptr<NodesCache> CreateCache(std::shared_ptr<NodesCache::DbFactory> initializedCache, bool ensureThreadSafety) const
        {
        return NodesCache::Create(initializedCache, m_nodeFactory, m_contextFactory, m_providersFactory, ensureThreadSafety);
        }

    void IterateCaches(std::function<void(std::shared_ptr<NodesCache>)> callback) const
        {
        BeMutexHolder lock(m_mutex);
        for (auto entry : m_initializedCaches)
            {
            auto cache = _FindCache(entry.first);
            if (nullptr == cache)
                continue;
            callback(cache);
            }
        }

protected:
    virtual NodesCacheType _GetCacheType() const = 0;
    virtual std::shared_ptr<NodesCache> _FindCache(Utf8StringCR connectionId) const = 0;
    virtual void _OnConnectionOpened(IConnectionCR connection) {}
    virtual void _OnConnectionClosed(IConnectionCR connection) {}

protected:
    NodesCacheManager(BeFileNameCR tempDirectory, NavNodesFactoryCR nodeFactory, INodesProviderContextFactoryCR nodeProviderContextFactory, INodesProviderFactoryCR nodeProvidersFactory,
        IConnectionManagerCR connectionManager, uint64_t cacheSizeLimit, Nullable<uint64_t> diskCacheMemoryCacheSize)
        : m_cacheDirectory(tempDirectory), m_nodeFactory(nodeFactory), m_contextFactory(nodeProviderContextFactory), m_connections(connectionManager),
        m_cacheSizeLimit(cacheSizeLimit), m_providersFactory(nodeProvidersFactory), m_diskCacheMemoryCacheSize(diskCacheMemoryCacheSize)
        {
        m_connections.AddListener(*this);
        }
    ~NodesCacheManager()
        {
        m_connections.DropListener(*this);
        }
    BeMutex& GetMutex() const {return m_mutex;}

    std::shared_ptr<NodesCache> CreateCache(Utf8CP connectionId, bool ensureThreadSafety) const
        {
        BeMutexHolder lock(m_mutex);
        auto initializedCache = m_initializedCaches.find(connectionId);
        if (initializedCache == m_initializedCaches.end())
            return nullptr;
        return CreateCache(initializedCache->second, ensureThreadSafety);
        }

    void _OnConnectionEvent(ConnectionEvent const& event) override
        {
        BeMutexHolder lock(m_mutex);
        if (event.GetEventType() == ConnectionEventType::Opened)
            {
            auto scope = Diagnostics::Scope::Create("NodesCacheManager: Connection opened");
            auto dbFactory = NodesCache::DbFactory::Create(event.GetConnection(), m_cacheDirectory, _GetCacheType(), m_cacheSizeLimit, m_diskCacheMemoryCacheSize);
            m_initializedCaches.Insert(event.GetConnection().GetId(), dbFactory);
            _OnConnectionOpened(event.GetConnection());
            }
        else if (event.GetEventType() == ConnectionEventType::Closed)
            {
            auto scope = Diagnostics::Scope::Create("NodesCacheManager: Connection closed");
            m_initializedCaches.erase(event.GetConnection().GetId());
            _OnConnectionClosed(event.GetConnection());
            }
        }

    std::shared_ptr<NodesCache> _GetPersistentCache(Utf8StringCR connectionId) const override
        {
        auto scope = Diagnostics::Scope::Create("NodesCacheManager: Get persistent cache");
        return _FindCache(connectionId);
        }

    virtual std::shared_ptr<INavNodesCache> _GetCache(Utf8StringCR connectionId, BeGuidCR rootNodeId) const override
        {
        auto scope = Diagnostics::Scope::Create("NodesCacheManager: Get cache");
        return _FindCache(connectionId);
        }

    void _ClearCaches(Utf8CP rulesetId) const override
        {
        IterateCaches([&](std::shared_ptr<NodesCache> cache) {cache->Clear(rulesetId);});
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MemoryNodesCacheManager : NodesCacheManager
{
private:
    bmap<Utf8String, std::shared_ptr<NodesCache>> m_caches;

protected:
    NodesCacheType _GetCacheType() const override {return NodesCacheType::Memory;}

    void _OnConnectionOpened(IConnectionCR connection) override
        {
        auto cache = CreateCache(connection.GetId().c_str(), true);
        if (nullptr == cache)
            return;

        m_caches.Insert(connection.GetId(), cache);
        }

    void _OnConnectionClosed(IConnectionCR connection) override
        {
        m_caches.erase(connection.GetId());
        }

    std::shared_ptr<NodesCache> _FindCache(Utf8StringCR connectionId) const override
        {
        BeMutexHolder lock(GetMutex());
        auto iter = m_caches.find(connectionId);
        if (m_caches.end() == iter)
            return nullptr;
        return iter->second;
        }

public:
    MemoryNodesCacheManager(BeFileNameCR tempDirectory, NavNodesFactoryCR nodeFactory, INodesProviderContextFactoryCR nodeProviderContextFactory, INodesProviderFactoryCR nodeProvidersFactory,
        IConnectionManagerCR connectionManager, uint64_t cacheSizeLimit)
        : NodesCacheManager(tempDirectory, nodeFactory, nodeProviderContextFactory, nodeProvidersFactory, connectionManager, cacheSizeLimit, nullptr)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DiskNodesCacheManager : NodesCacheManager
{
private:
    mutable BeThreadLocalStorage m_cachesStorage;
    mutable bvector<bmap<Utf8String, std::shared_ptr<NodesCache>>*> m_allCaches;

private:
    bmap<Utf8String, std::shared_ptr<NodesCache>>& GetCachesForCurrentThread() const
        {
        void* ptr = m_cachesStorage.GetValueAsPointer();
        bmap<Utf8String, std::shared_ptr<NodesCache>>* threadCaches = nullptr;
        if (nullptr != ptr)
            threadCaches = static_cast<bmap<Utf8String, std::shared_ptr<NodesCache>>*>(ptr);

        if (nullptr == threadCaches)
            {
            threadCaches = new bmap<Utf8String, std::shared_ptr<NodesCache>>();
            m_cachesStorage.SetValueAsPointer(threadCaches);
            BeMutexHolder lock(GetMutex());
            m_allCaches.push_back(threadCaches);
            }
        return *threadCaches;
        }

protected:
    virtual NodesCacheType _GetCacheType() const override {return NodesCacheType::Disk;}

    void _OnConnectionClosed(IConnectionCR connection) override
        {
        BeMutexHolder lock(GetMutex());
        for (auto caches : m_allCaches)
            caches->erase(connection.GetId());
        }

    std::shared_ptr<NodesCache> _FindCache(Utf8StringCR connectionId) const override
        {
        bmap<Utf8String, std::shared_ptr<NodesCache>>& threadCaches = GetCachesForCurrentThread();
        auto iter = threadCaches.find(connectionId);
        if (threadCaches.end() == iter)
            iter = threadCaches.Insert(connectionId, CreateCache(connectionId.c_str(), false)).first;
        return iter->second;
        }

public:
    DiskNodesCacheManager(BeFileNameCR tempDirectory, NavNodesFactoryCR nodeFactory, INodesProviderContextFactoryCR nodeProviderContextFactory, INodesProviderFactoryCR nodeProvidersFactory,
        IConnectionManagerCR connectionManager, uint64_t cacheSizeLimit, Nullable<uint64_t> memoryCacheSize)
        : NodesCacheManager(tempDirectory, nodeFactory, nodeProviderContextFactory, nodeProvidersFactory, connectionManager, cacheSizeLimit, memoryCacheSize)
        {}

    ~DiskNodesCacheManager()
        {
        for (auto caches : m_allCaches)
            delete caches;
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HybridNodesCacheManager : DiskNodesCacheManager
{
protected:
    NodesCacheType _GetCacheType() const override {return NodesCacheType::HybridDisk;}

    std::shared_ptr<INavNodesCache> _GetCache(Utf8StringCR connectionId, BeGuidCR rootNodeId) const override
        {
        auto cache = DiskNodesCacheManager::_FindCache(connectionId);
        if (nullptr == cache)
            return nullptr;
        return std::make_shared<NodesCacheWrapper>(*cache, rootNodeId);
        }

public:
    HybridNodesCacheManager(BeFileNameCR tempDirectory, NavNodesFactoryCR nodeFactory, INodesProviderContextFactoryCR nodeProviderContextFactory, INodesProviderFactoryCR nodeProvidersFactory,
        IConnectionManagerCR connectionManager, uint64_t cacheSizeLimit, Nullable<uint64_t> diskCacheMemoryCacheSize)
        : DiskNodesCacheManager(tempDirectory, nodeFactory, nodeProviderContextFactory, nodeProvidersFactory, connectionManager, cacheSizeLimit, diskCacheMemoryCacheSize)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::unique_ptr<INodesCacheManager> CreateCacheManager(BeFileNameCR tempDirectory, NavNodesFactoryCR nodeFactory, INodesProviderContextFactoryCR nodeProviderContextFactory, INodesProviderFactoryCR nodeProvidersFactory,
    IConnectionManagerCR connectionManager, RulesDrivenECPresentationManagerImpl::Params::CachingParams::Mode mode, uint64_t cacheSizeLimit, Nullable<uint64_t> diskCacheMemoryCacheSize)
    {
    switch (mode)
        {
        case ECPresentationManager::Params::CachingParams::Mode::Memory:
            return std::make_unique<MemoryNodesCacheManager>(tempDirectory, nodeFactory, nodeProviderContextFactory, nodeProvidersFactory, connectionManager, cacheSizeLimit);
        case ECPresentationManager::Params::CachingParams::Mode::Hybrid:
            return std::make_unique<HybridNodesCacheManager>(tempDirectory, nodeFactory, nodeProviderContextFactory, nodeProvidersFactory, connectionManager, cacheSizeLimit, diskCacheMemoryCacheSize);
        case ECPresentationManager::Params::CachingParams::Mode::Disk:
        default:
            return std::make_unique<DiskNodesCacheManager>(tempDirectory, nodeFactory, nodeProviderContextFactory, nodeProvidersFactory, connectionManager, cacheSizeLimit, diskCacheMemoryCacheSize);
        }
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImpl::ECDbCaches
{
    struct Caches
        {
        RelatedPathsCache* m_relatedPathsCache;
        RulesetUsedClassesNotificationFlags m_rulesetUsedClassesNotificationFlags;
        Caches()
            {
            m_relatedPathsCache = new RelatedPathsCache();
            }
        ~Caches()
            {
            DELETE_AND_CLEAR(m_relatedPathsCache);
            }
        };

private:
    mutable bmap<Utf8String, Caches*> m_caches;
    mutable BeMutex m_mutex;

private:
    Caches& GetCaches(IConnectionCR connection) const
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_caches.find(connection.GetId());
        if (m_caches.end() == iter)
            iter = m_caches.Insert(connection.GetId(), new Caches()).first;
        return *iter->second;
        }

public:
    ECDbCaches() {}
    ~ECDbCaches()
        {
        BeMutexHolder lock(m_mutex);
        for (auto iter : m_caches)
            delete iter.second;
        }
    void Clear(IConnectionCR connection)
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_caches.find(connection.GetId());
        if (m_caches.end() != iter)
            {
            delete iter->second;
            m_caches.erase(iter);
            }
        }
    RelatedPathsCache& GetRelatedPathsCache(IConnectionCR connection) const {return *GetCaches(connection).m_relatedPathsCache;}
    RulesetUsedClassesNotificationFlags& GetRulesetUsedClassesNotificationFlags(IConnectionCR connection) const {return GetCaches(connection).m_rulesetUsedClassesNotificationFlags;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TDerivedBase>
static HierarchyRequestImplParams CreateHierarchyRequestParams(ImplTaskParams<TDerivedBase> const& source, NavNodeCP parentNode = nullptr)
    {
    return HierarchyRequestImplParams::Create(HierarchyRequestParams(source, parentNode), source);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImpl::NavNodeLocater
{
private:
    RulesDrivenECPresentationManagerImpl const& m_manager;
    RequestWithRulesetImplParams m_params;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeCPtr LocateNodeInHierarchyLevel(bvector<Utf8String> const& path, int depth, NavNodeCP parentNode)
        {
        if (path.size() <= depth)
            return nullptr;

        auto nodes = m_manager.GetCachedDataSource(CreateHierarchyRequestParams(m_params, parentNode));
        if (nodes.IsNull())
            return nullptr;

        DisabledFullNodesLoadContext disableFullLoad(*nodes->GetProvider());

        NavNodePtr curr;
        bool found = false;
        for (NavNodePtr node : *nodes)
            {
            ThrowIfCancelled(m_params.GetCancellationToken());

            bvector<Utf8String> const& nodePath = node->GetKey()->GetHashPath();
            if (nodePath.size() <= depth)
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Node hash path shorter than requested depth");

            curr = node;
            found = true;
            for (size_t virtualIndex = depth; virtualIndex < nodePath.size() && virtualIndex < path.size(); ++virtualIndex)
                {
                if (!nodePath[virtualIndex].Equals(path[virtualIndex]))
                    {
                    found = false;
                    break;
                    }
                }

            if (found)
                return curr;
            }

        return nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeCPtr LocateNodeInHierarchy(bvector<Utf8String> const& path, int depth, NavNodeCP parentNode)
        {
        ThrowIfCancelled(m_params.GetCancellationToken());

        NavNodeCPtr curr = LocateNodeInHierarchyLevel(path, depth, parentNode);
        if (curr.IsNull())
            return nullptr;

        depth = std::min(path.size(), curr->GetKey()->GetHashPath().size()) - 1;

        if (path.size() == depth + 1)
            return curr;

        return LocateNodeInHierarchy(path, depth + 1, curr.get());
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeLocater(RulesDrivenECPresentationManagerImpl const& manager, RequestWithRulesetImplParams params)
        : m_manager(manager), m_params(params)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeCPtr LocateNode(NavNodeKeyCR nodeKey)
        {
        auto scope = Diagnostics::Scope::Create("Locate node");

        std::shared_ptr<INavNodesCache> nodesCache = m_manager.GetHierarchyCache(m_params.GetConnection().GetId());
        VALID_HIERARCHY_CACHE_PRECONDITION(nodesCache, nullptr);

        NavNodeCPtr node = nodesCache->LocateNode(m_params.GetConnection(), m_params.GetRulesetId(), nodeKey);
        if (node.IsValid())
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Node taken from cache");
            return node;
            }

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Node not found in cache.");
        auto locateScope = Diagnostics::Scope::Create("Load & Locate");
        return LocateNodeInHierarchy(nodeKey.GetHashPath(), 0, nullptr);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImpl::NodesProviderFactory : INodesProviderFactory
{
private:
    bvector<std::unique_ptr<IProvidedNodesPostProcessor>> m_postProcessors;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static NavNodesProviderPtr WithDeprecatedPostProcessing(NavNodesProviderR provider, bvector<ChildNodeSpecificationCP> const& specs)
        {
        RefCountedPtr<PostProcessingNodesProviderDeprecated> postProcessingProvider = PostProcessingNodesProviderDeprecated::Create(provider);
        postProcessingProvider->RegisterPostProcessor(std::make_unique<SameLabelGroupingNodesPostProcessorDeprecated>(provider.GetContext().GetRulesPreprocessor(),
            provider.GetContext().GetVirtualParentNode().get(), specs, provider.GetContext().GetSchemaHelper(), provider.GetContext().GetLocalState()));
        return postProcessingProvider;
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TRuleSpecification> static bvector<ChildNodeSpecificationCP> MapSpecs(bvector<TRuleSpecification> const& ruleSpecs)
        {
        return ContainerHelpers::TransformContainer<bvector<ChildNodeSpecificationCP>>(ruleSpecs, [](auto const& ruleSpec){return &ruleSpec.GetSpecification();});
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<IProvidedNodesPostProcessor const*> _GetPostProcessors() const override
        {
        return ContainerHelpers::TransformContainer<bvector<IProvidedNodesPostProcessor const*>>(m_postProcessors, [](auto const& ptr){return ptr.get();});
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr _Create(NavNodesProviderContextR context) const override
        {
        auto scope = Diagnostics::Scope::Create("Create nodes provider");
        NavNodeCPtr parent = context.GetVirtualParentNode();
        NavNodesProviderPtr provider;
        if (parent.IsNull())
            {
            IRulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
            RootNodeRuleSpecificationsList specs = context.GetRulesPreprocessor().GetRootNodeSpecifications(params);
            if (!specs.empty())
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, Utf8PrintfString("Creating root nodes provider using %" PRIu64 " specifications.", (uint64_t)specs.size()));
                provider = WithDeprecatedPostProcessing(*MultiSpecificationNodesProvider::Create(context, specs), MapSpecs(specs));
                }
            else
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Creating empty root nodes provider (found 0 specifications).");
                }
            }
        else
            {
            IRulesPreprocessor::ChildNodeRuleParameters params(*parent, TargetTree_MainTree);
            ChildNodeRuleSpecificationsList specs = context.GetRulesPreprocessor().GetChildNodeSpecifications(params);
            if (!specs.empty())
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, Utf8PrintfString("Creating child nodes provider for parent %s using %" PRIu64 " specifications.",
                    DiagnosticsHelpers::CreateNodeIdentifier(*parent).c_str(), (uint64_t)specs.size()));
                provider = WithDeprecatedPostProcessing(*MultiSpecificationNodesProvider::Create(context, specs, *parent), MapSpecs(specs));
                }
            else
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, Utf8PrintfString("Creating empty child nodes provider for parent %s (found 0 specifications).",
                    DiagnosticsHelpers::CreateNodeIdentifier(*parent).c_str()));
                }
            }
        if (provider.IsNull())
            provider = EmptyNavNodesProvider::Create(context);
        return provider;
        }

public:
    NodesProviderFactory()
        {
        m_postProcessors.push_back(std::make_unique<DisplayLabelGroupingNodesPostProcessor>());
#ifdef wip_enable_display_label_postprocessor
        m_postProcessors.push_back(std::make_unique<DisplayLabelSortingPostProcessor>());
#endif
        m_postProcessors.push_back(std::make_unique<NodesFinalizingPostProcessor>());
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImpl::NodesProviderContextFactory : INodesProviderContextFactory
{
private:
    RulesDrivenECPresentationManagerImpl& m_manager;

protected:
    NavNodesProviderContextPtr _Create(IConnectionCR connection, Utf8CP rulesetId, NavNodeCP parentNode, std::shared_ptr<INavNodesCache> cache, ICancelationTokenCP cancelationToken, RulesetVariables const& variables) const override
        {
        auto scope = Diagnostics::Scope::Create("Create nodes provider context");

        // get the ruleset
        PresentationRuleSetPtr ruleset = FindRuleset(m_manager.GetLocaters(), connection, rulesetId);
        if (!ruleset.IsValid())
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Ruleset '%s' not found. Returning NULL.", rulesetId));
            return nullptr;
            }

        ThrowIfCancelled(cancelationToken);

        // get various caches
        IUserSettings const& settings = m_manager.GetUserSettings(rulesetId);
        ECExpressionsCache& ecexpressionsCache = m_manager.m_rulesetECExpressionsCache->Get(rulesetId);
        RelatedPathsCache& relatedPathsCache = m_manager.m_ecdbCaches->GetRelatedPathsCache(connection);
        RulesetUsedClassesNotificationFlags& rulesetUsedClassesNotificationFlags = m_manager.m_ecdbCaches->GetRulesetUsedClassesNotificationFlags(connection);

        // make sure latest ruleset version is used in cache
        cache->OnRulesetUsed(*ruleset);
        std::unique_ptr<RulesetVariables> rulesetVariables = std::make_unique<RulesetVariables>(variables);
        rulesetVariables->Merge(settings);

        // set up the nodes provider context
        NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*ruleset, TargetTree_MainTree, parentNode,
            std::move(rulesetVariables), ecexpressionsCache, relatedPathsCache, *m_manager.m_nodesFactory, cache,
            *m_manager.m_nodesProviderFactory, m_manager.GetLocalState());
        context->SetQueryContext(*m_manager.m_connections, connection, m_manager.m_usedClassesListener);

        if (parentNode)
            context->SetChildNodeContext(nullptr, *parentNode);
        else
            context->SetRootNodeContext(nullptr);

        context->SetPropertyFormattingContext(m_manager.GetECPropertyFormatter(), UnitSystem::Undefined);
        context->SetCancelationToken(cancelationToken);

        // notify listener with ECClasses used in this ruleset
        if (rulesetUsedClassesNotificationFlags.Add(ruleset->GetRuleSetId()))
            UsedClassesHelper::NotifyListenerWithRulesetClasses(*m_manager.m_usedClassesListener, ecexpressionsCache, connection, context->GetRulesPreprocessor());

        return context;
        }
public:
    NodesProviderContextFactory(RulesDrivenECPresentationManagerImpl& mgr) : m_manager(mgr) {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManagerImpl::RulesDrivenECPresentationManagerImpl(Params const& params)
    : m_connections(params.GetConnections() ? params.GetConnections() : std::make_shared<ConnectionManager>())
    {
    m_localState = params.GetLocalState();
    m_ecPropertyFormatter = params.GetECPropertyFormatter();
    m_categorySupplier = params.GetCategorySupplier();

    m_locaters = params.GetRulesetLocaters() ? params.GetRulesetLocaters() : std::make_shared<RuleSetLocaterManager>(*m_connections);
    m_locaters->SetRulesetCallbacksHandler(this);

    m_userSettings = params.GetUserSettings() ? params.GetUserSettings() : std::make_shared<UserSettingsManager>(params.GetPaths().GetTemporaryDirectory());
    GetUserSettingsManager().SetChangesListener(this);
    GetUserSettingsManager().SetLocalState(m_localState);

    m_ecInstanceChangeEventSources = params.GetECInstanceChangeEventSources(); // need to copy this list to keep the ref counts
    for (auto const& ecInstanceChangeEventSource : m_ecInstanceChangeEventSources)
        ecInstanceChangeEventSource->RegisterEventHandler(*this);

    m_customFunctions = new CustomFunctionsInjector(*m_connections);
    m_rulesetECExpressionsCache = new RulesetECExpressionsCache();
    m_ecdbCaches = new ECDbCaches();
    m_nodesProviderContextFactory = new NodesProviderContextFactory(*this);
    m_nodesProviderFactory = new NodesProviderFactory();
    m_usedClassesListener = new UsedClassesListener(*this);
    m_nodesFactory = new NavNodesFactory();

    m_nodesCachesManager = CreateCacheManager(params.GetCachingParams().GetCacheDirectoryPath(), *m_nodesFactory, *m_nodesProviderContextFactory, *m_nodesProviderFactory,
        *m_connections, params.GetCachingParams().GetCacheMode(), params.GetCachingParams().GetDiskCacheFileSizeLimit(), params.GetCachingParams().GetDiskCacheMemoryCacheSize());
    m_contentCache = new ContentCache(params.GetContentCachingParams().GetPrivateCacheSize());

    m_updateHandler = new UpdateHandler(*m_nodesCachesManager, m_contentCache, *m_connections, *m_nodesProviderContextFactory,
        *m_nodesProviderFactory, *m_rulesetECExpressionsCache, params.GetUiStateProvider());
    m_updateHandler->SetRecordsHandler(std::make_unique<CompositeUpdateRecordsHandler>(params.GetUpdateRecordsHandlers()));

    m_connections->AddListener(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::Initialize()
    {
    RegisterDisplayLabelRuleset(GetLocaters());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<NodesCache> RulesDrivenECPresentationManagerImpl::GetNodesCache(IConnectionCR connection)
    {
    return m_nodesCachesManager->GetPersistentCache(connection.GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<INavNodesCache> RulesDrivenECPresentationManagerImpl::_GetHierarchyCache(Utf8StringCR connectionId) const
    {
    return m_nodesCachesManager->GetPersistentCache(connectionId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManagerImpl::~RulesDrivenECPresentationManagerImpl()
    {
    auto scope = Diagnostics::Scope::Create("Destroy manager impl");
    m_connections->DropListener(*this);
    m_connections->CloseConnections();
    DELETE_AND_CLEAR(m_updateHandler);
    DELETE_AND_CLEAR(m_contentCache);
    DELETE_AND_CLEAR(m_nodesFactory);
    DELETE_AND_CLEAR(m_usedClassesListener);
    DELETE_AND_CLEAR(m_nodesProviderFactory);
    DELETE_AND_CLEAR(m_nodesProviderContextFactory);
    DELETE_AND_CLEAR(m_ecdbCaches);
    DELETE_AND_CLEAR(m_rulesetECExpressionsCache);
    DELETE_AND_CLEAR(m_customFunctions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECPropertyFormatter const& RulesDrivenECPresentationManagerImpl::_GetECPropertyFormatter() const
    {
    BeMutexHolder lock(m_mutex);
    if (nullptr != m_ecPropertyFormatter)
        return *m_ecPropertyFormatter;

    static const DefaultPropertyFormatter s_defaultPropertyFormatter;
    return s_defaultPropertyFormatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IPropertyCategorySupplier const& RulesDrivenECPresentationManagerImpl::_GetCategorySupplier() const
    {
    BeMutexHolder lock(m_mutex);
    if (nullptr != m_categorySupplier)
        return *m_categorySupplier;

    static const DefaultCategorySupplier s_defaultCategorySupplier;
    return s_defaultCategorySupplier;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnRulesetCreated(RuleSetLocaterCR, PresentationRuleSetR ruleset)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Ruleset created: '%s'", ruleset.GetRuleSetId().c_str()));
    IUserSettings& settings = GetUserSettings(ruleset.GetRuleSetId().c_str());
    settings.InitFrom(ruleset.GetUserSettings());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnRulesetDispose(RuleSetLocaterCR, PresentationRuleSetR ruleset)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Ruleset disposed: '%s'", ruleset.GetRuleSetId().c_str()));
    m_rulesetECExpressionsCache->Clear(ruleset.GetRuleSetId().c_str());
    m_updateHandler->NotifyRulesetDisposed(ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const
    {
    // TODO: is this necessary anymore?
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Setting changed: '%s'", settingId));
    CustomFunctionsManager::GetManager()._OnSettingChanged(rulesetId, settingId);
    m_updateHandler->NotifySettingChanged(rulesetId, settingId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnConnectionEvent(ConnectionEvent const& evt)
    {
    if (evt.GetEventType() == ConnectionEventType::Opened)
        {
        RefCountedPtr<EmbeddedRuleSetLocater> embeddedLocater = EmbeddedRuleSetLocater::Create(evt.GetConnection());
        RuleSetLocaterPtr supplementalLocater = SupplementalRuleSetLocater::Create(*embeddedLocater);
        RuleSetLocaterPtr nonsupplementalLocater = NonSupplementalRuleSetLocater::Create(*embeddedLocater);
        m_embeddedRuleSetLocaters[evt.GetConnection().GetId()] = { supplementalLocater, nonsupplementalLocater };
        GetLocaters().RegisterLocater(*supplementalLocater);
        GetLocaters().RegisterLocater(*nonsupplementalLocater);
        }
    else if (evt.GetEventType() == ConnectionEventType::Closed)
        {
        auto iter = m_embeddedRuleSetLocaters.find(evt.GetConnection().GetId());
        if (m_embeddedRuleSetLocaters.end() != iter)
            {
            for (RuleSetLocaterPtr locater : iter->second )
                GetLocaters().UnregisterLocater(*locater);
            m_embeddedRuleSetLocaters.erase(iter);
            }
        }
    if (evt.GetEventType() == ConnectionEventType::Closed || evt.GetEventType() == ConnectionEventType::Suspended)
        {
        m_contentCache->ClearCache(evt.GetConnection());
        m_ecdbCaches->Clear(evt.GetConnection());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnECInstancesChanged(ECDbCR db, bvector<ECInstanceChangeEventSource::ChangedECInstance> changes)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("ECInstances changed with %" PRIu64 " changes", (uint64_t)changes.size()));
    IConnectionPtr connection = m_connections->GetConnection(db);
    if (connection.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Connections, "Failed to get a connection for current task");

    if (!changes.empty())
        m_updateHandler->NotifyECInstancesChanged(*connection, changes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::shared_ptr<NavNodesProviderContext::PageOptions> CreateProviderPageOptions(PageOptionsCP pageOptions)
    {
    if (!pageOptions || pageOptions->Empty())
        return nullptr;

    if (pageOptions->GetPageSize() == 0)
        return std::make_shared<NavNodesProviderContext::PageOptions>(pageOptions->GetPageStart());

    return std::make_shared<NavNodesProviderContext::PageOptions>(pageOptions->GetPageStart(), pageOptions->GetPageSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderContextPtr RulesDrivenECPresentationManagerImpl::CreateNodesProviderContext(HierarchyRequestImplParams const& params, std::shared_ptr<INavNodesCache> cache) const
    {
    auto scope = Diagnostics::Scope::Create("Create nodes provider context");

    // locate the parent node if it's passed by key
    NavNodeCPtr parentNode = params.GetParentNode();
    if (parentNode.IsNull() && params.GetParentNodeKey())
        {
        parentNode = NavNodeLocater(*this, RequestWithRulesetImplParams::Create(params)).LocateNode(*params.GetParentNodeKey());
        if (parentNode.IsNull())
            throw InvalidArgumentException("Node for given parent node key does not exist");
        }

    // get the nodes cache
    if (nullptr == cache)
        cache = m_nodesCachesManager->GetCache(params.GetConnection().GetId(), parentNode.IsValid() ? parentNode->GetNodeId() : BeGuid());
    if (nullptr == cache)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Failed to find the hierarchy cache for given connection: '%s'.", params.GetConnection().GetId().c_str()));

    // create the nodes provider context
    NavNodesProviderContextPtr context = m_nodesProviderContextFactory->Create(params.GetConnection(), params.GetRulesetId().c_str(),
        parentNode.get(), cache, params.GetCancellationToken(), params.GetRulesetVariables());
    if (context.IsValid())
        context->SetInstanceFilter(params.GetInstanceFilter());
    return context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::FinalizeNode(RequestWithRulesetImplParams const& params, NavNodeR node) const
    {
    auto scope = Diagnostics::Scope::Create("Finalize node");

    auto hierarchyCache = m_nodesCachesManager->GetCache(params.GetConnection().GetId());
    auto parentNodeId = hierarchyCache->GetVirtualParentNodeId(node.GetNodeId());
    auto parentNode = parentNodeId.IsValid() ? hierarchyCache->GetNode(parentNodeId) : nullptr;
    auto contextParams = CreateHierarchyRequestParams(params, parentNode.get());
    auto context = CreateNodesProviderContext(contextParams);
    if (context.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to create context for finalizing node.");

    NodesFinalizer(*context).Finalize(node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr RulesDrivenECPresentationManagerImpl::FinalizeNode(RequestWithRulesetImplParams const& params, NavNodeCR node) const
    {
    NavNodePtr clone = node.Clone();
    FinalizeNode(params, *clone);
    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<INodeInstanceKeysProvider> RulesDrivenECPresentationManagerImpl::_CreateNodeInstanceKeysProvider(NodeInstanceKeysRequestImplParams const& params) const
    {
    auto scope = Diagnostics::Scope::Create("Create nodes instance keys provider");

    auto hierarchyParams = CreateHierarchyRequestParams(params);
    hierarchyParams.SetInstanceFilter(params.GetInstanceFilter());

    auto context = CreateNodesProviderContext(hierarchyParams);
    if (context.IsNull())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_WARNING, "Failed to create context. Returning NULL.");
        return nullptr;
        }

    return context->CreateNodeInstanceKeysProvider();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ProviderBasedNodesDataSource> RulesDrivenECPresentationManagerImpl::GetCachedDataSource(NavNodesProviderContextR context, PageOptionsCP pageOptions) const
    {
    auto scope = Diagnostics::Scope::Create("Create data source");

    if (context.GetVirtualParentNode().IsValid() && NodesFinalizer(context).HasSimilarNodeInHierarchy(*context.GetVirtualParentNode()))
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Parent node has similar ancestor - returning empty data source");
        return nullptr;
        }

    NavNodesProviderPtr provider;
    if (!pageOptions || pageOptions->GetPageStart() == 0)
        {
        // look for provider in persistent cache
        // note: combined hierarchy level provider is only efficient to get nodes without offset, in all other cases
        // it's more efficient to create a new provider
        provider = context.GetNodesCache().GetCombinedHierarchyLevel(context, context.GetHierarchyLevelIdentifier());
        if (provider.IsValid())
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Found provider in persistent cache");
        }

    // create the provider
    if (provider.IsNull())
        {
        provider = m_nodesProviderFactory->Create(context);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Created a new provider");
        }

    // post-process
    provider = provider->PostProcess(m_nodesProviderFactory->GetPostProcessors());
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Provider post-processed");

    provider->SetPageOptions(CreateProviderPageOptions(pageOptions));

    auto source = ProviderBasedNodesDataSource::Create(*provider);
    source->SetSupportsFiltering(HierarchiesFilteringHelper::SupportsFiltering(
        context.GetVirtualParentNode().get(),
        TraverseHierarchyRulesProps(context.GetNodesFactory(), context.GetRulesPreprocessor(), context.GetSchemaHelper()),
        nullptr
        ));

    return source;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ProviderBasedNodesDataSource> RulesDrivenECPresentationManagerImpl::GetCachedDataSource(WithPageOptions<HierarchyRequestImplParams> const& params) const
    {
    auto scope = Diagnostics::Scope::Create("Create data source");

    NavNodesProviderContextPtr context = CreateNodesProviderContext(params);
    if (context.IsNull())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_WARNING, "Failed to create context. Returning NULL.");
        return nullptr;
        }

    // create hierarchy level locker for this hierarchy level. If lock was acquired it will be released when context is destroyed.
    context->SetHierarchyLevelLocker(context->GetNodesCache().CreateHierarchyLevelLocker(context->GetHierarchyLevelIdentifier()));

    return GetCachedDataSource(*context, &params.GetPageOptions());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Diagnostics::Scope::Holder CreateScopeForHierarchyRequest(HierarchyRequestParams const& params, Utf8CP requestIdentifier)
    {
    if (params.GetParentNode())
        return Diagnostics::Scope::Create(Utf8PrintfString("Get child %s for parent %s", requestIdentifier, DiagnosticsHelpers::CreateNodeIdentifier(*params.GetParentNode()).c_str()));

    if (params.GetParentNodeKey())
        return Diagnostics::Scope::Create(Utf8PrintfString("Get child %s for parent %s", requestIdentifier, DiagnosticsHelpers::CreateNodeKeyIdentifier(*params.GetParentNodeKey()).c_str()));

    return Diagnostics::Scope::Create(Utf8PrintfString("Get root %s", requestIdentifier));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ReportNodesResponse(WithPageOptions<HierarchyRequestParams> const& params, NavNodesDataSource const* ds)
    {
    auto pageStart = (uint64_t)params.GetPageOptions().GetPageStart();
    auto dsSizeWithOffset = (uint64_t)(params.GetPageOptions().GetPageStart() + (ds ? ds->GetSize() : 0));
    if (params.GetParentNode())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_INFO, Utf8PrintfString("Returning [%" PRIu64 ", %" PRIu64 ") child nodes for parent %s",
            pageStart, dsSizeWithOffset, DiagnosticsHelpers::CreateNodeIdentifier(*params.GetParentNode()).c_str()));
        }
    else if (params.GetParentNodeKey())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_INFO, Utf8PrintfString("Returning [%" PRIu64 ", %" PRIu64 ") child nodes for parent %s",
            pageStart, dsSizeWithOffset, DiagnosticsHelpers::CreateNodeKeyIdentifier(*params.GetParentNodeKey()).c_str()));
        }
    else
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_INFO, Utf8PrintfString("Returning [%" PRIu64 ", %" PRIu64 ") root nodes",
            pageStart, dsSizeWithOffset));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesDataSourcePtr RulesDrivenECPresentationManagerImpl::_GetNodes(WithPageOptions<HierarchyRequestImplParams> const& params)
    {
    auto scope = CreateScopeForHierarchyRequest(params, "nodes");
    NavNodesDataSourcePtr source = GetCachedDataSource(params);
    ReportNodesResponse(params, source.get());
    return source;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ReportNodesCountResponse(WithPageOptions<HierarchyRequestParams> const& params, size_t nodesCount)
    {
    if (params.GetParentNode())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_INFO, Utf8PrintfString("Returning %" PRIu64 " for parent %s",
            (uint64_t)nodesCount, DiagnosticsHelpers::CreateNodeIdentifier(*params.GetParentNode()).c_str()));
        }
    else if (params.GetParentNodeKey())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_INFO, Utf8PrintfString("Returning %" PRIu64 " for parent %s",
            (uint64_t)nodesCount, DiagnosticsHelpers::CreateNodeKeyIdentifier(*params.GetParentNodeKey()).c_str()));
        }
    else
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_INFO, Utf8PrintfString("Returning %" PRIu64, (uint64_t)nodesCount));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RulesDrivenECPresentationManagerImpl::_GetNodesCount(HierarchyRequestImplParams const& params)
    {
    auto scope = CreateScopeForHierarchyRequest(params, "nodes count");

    NavNodesDataSourcePtr source = GetCachedDataSource(params);
    size_t size = source.IsValid() ? source->GetSize() : 0;
    ReportNodesCountResponse(params, size);
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Diagnostics::Scope::Holder CreateScopeForHierarchyRequest(HierarchyLevelDescriptorRequestParams const& params, Utf8CP requestIdentifier)
    {
    if (params.GetParentNodeKey())
        return Diagnostics::Scope::Create(Utf8PrintfString("Get child %s for parent %s", requestIdentifier, DiagnosticsHelpers::CreateNodeKeyIdentifier(*params.GetParentNodeKey()).c_str()));

    return Diagnostics::Scope::Create(Utf8PrintfString("Get root %s", requestIdentifier));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorCPtr RulesDrivenECPresentationManagerImpl::_GetNodesDescriptor(HierarchyLevelDescriptorRequestImplParams const& params)
    {
    auto scope = CreateScopeForHierarchyRequest(params, "nodes descriptor");

    auto context = CreateNodesProviderContext(HierarchyRequestImplParams::Create(HierarchyRequestParams(params, params.GetParentNodeKey()), params));
    auto ruleset = HierarchiesFilteringHelper::CreateHierarchyLevelDescriptorRuleset(
        context->GetVirtualParentNode().get(),
        TraverseHierarchyRulesProps(context->GetNodesFactory(), context->GetRulesPreprocessor(), context->GetSchemaHelper())
        );
    TempRulesetRegistration registerRuleset(*m_locaters, *ruleset);

    auto descriptorParams = ContentDescriptorRequestImplParams::Create(ContentDescriptorRequestParams(
        ContentMetadataRequestParams(
            ruleset->GetRuleSetId(),
            RulesetVariables(),
            "HierarchyFiltering",
            (int)ContentFlags::DescriptorOnly
            ),
        *KeySet::Create(params.GetParentNodeKey() ? NavNodeKeyList{ params.GetParentNodeKey() } : NavNodeKeyList{})
        ), params);
    return GetContentDescriptor(descriptorParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr RulesDrivenECPresentationManagerImpl::_GetParent(NodeParentRequestImplParams const& params)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Get parent for node %s", DiagnosticsHelpers::CreateNodeIdentifier(params.GetNode()).c_str()));

    std::shared_ptr<NodesCache> cache = m_nodesCachesManager->GetPersistentCache(params.GetConnection().GetId());
    VALID_HIERARCHY_CACHE_PRECONDITION(cache, nullptr);

    auto node = cache->GetPhysicalParentNode(params.GetNode().GetNodeId(), params.GetRulesetVariables(), params.GetInstanceFilter().get());
    if (node.IsValid())
        FinalizeNode(RequestWithRulesetImplParams::Create(params), *node);

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::TraverseHierarchy(HierarchyRequestImplParams const& params, std::shared_ptr<INavNodesCache> cache) const
    {
    ThrowIfCancelled(params.GetCancellationToken());

    NavNodesProviderContextPtr context = CreateNodesProviderContext(params, cache);
    if (context.IsNull())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_WARNING, "Failed to create context - stopping hierarchy traversal.");
        return;
        }

    NavNodesDataSourceCPtr nodes = GetCachedDataSource(*context, nullptr);
    if (nodes.IsValid())
        {
        for (NavNodePtr node : *nodes)
            TraverseHierarchy(CreateHierarchyRequestParams(params, node.get()), cache);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavNodeCPtr> RulesDrivenECPresentationManagerImpl::_GetFilteredNodes(NodePathsFromFilterTextRequestImplParams const& params)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Get filtered nodes: '%s'", params.GetFilterText().c_str()));
    bvector<NavNodeCPtr> result;

    std::shared_ptr<NodesCache> nodesCache = m_nodesCachesManager->GetPersistentCache(params.GetConnection().GetId());
    VALID_HIERARCHY_CACHE_PRECONDITION(nodesCache, result);

    // create a savepoint to avoid committing any changes while we're creating the hierarchy
    auto cacheSavepoint = nodesCache->CreateSavepoint(true);

    if (!nodesCache->IsCombinedHierarchyLevelInitialized(CombinedHierarchyLevelIdentifier(params.GetConnection().GetId(), params.GetRulesetId().c_str(), BeGuid()), params.GetRulesetVariables(), nullptr))
        {
        NavNodesProviderContextPtr rootNodesContext = CreateNodesProviderContext(CreateHierarchyRequestParams(params), nodesCache);
        if (rootNodesContext.IsNull())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_WARNING, "Failed to create root nodes context. Returning empty list.");
            return result;
            }
        NavNodesDataSourceCPtr rootNodes = GetCachedDataSource(*rootNodesContext, nullptr);
        if (rootNodes.IsValid())
            {
            for (NavNodeCPtr node : *rootNodes)
                ;
            }
        }

    // first we need to make sure the hierarchy is fully traversed so we can search in cache
    NavNodesProviderContextPtr undeterminedChildNodesContext = CreateNodesProviderContext(CreateHierarchyRequestParams(params), nodesCache);
    if (undeterminedChildNodesContext.IsNull())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_WARNING, "Failed to create undetermined child nodes context. Returning empty list.");
        return result;
        }
    NavNodesProviderCPtr provider = nodesCache->GetUndeterminedNodesProvider(*undeterminedChildNodesContext);
    if (provider.IsNull())
        return result;

    for (NavNodeCPtr node : *provider)
        {
        NOT_NULL_PRECONDITION(node, "RulesDrivenECPresentationManagerImpl::_GetFilteredNodes");
        TraverseHierarchy(CreateHierarchyRequestParams(params, node.get()), nodesCache);
        }

    // now we can filter nodes in cache
    NavNodesProviderContextPtr filteredNodesContext = CreateNodesProviderContext(CreateHierarchyRequestParams(params), nodesCache);
    if (filteredNodesContext.IsNull())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_WARNING, "Failed to create filtered nodes context. Returning empty list.");
        return result;
        }
    NavNodesProviderPtr filteredProvider = nodesCache->GetFilteredNodesProvider(*filteredNodesContext, params.GetFilterText().c_str());
    for (NavNodeCPtr node : *filteredProvider)
        {
        NOT_NULL_PRECONDITION(node, "RulesDrivenECPresentationManagerImpl::_GetFilteredNodes");
        result.push_back(node);
        }
    return result;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentRulesSpecificationsInputHandler
{
private:
    ContentProviderContextCR m_context;
    std::unique_ptr<RulesDrivenECPresentationManagerImpl::NavNodeLocater> m_nodesLocater;
    std::unique_ptr<INodeInstanceKeysProvider> m_nodeInstanceKeysProvider;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<ECInstanceKey> GetECInstanceKeys(NavNodeKeyListCR nodeKeys, ICancelationTokenCP cancelationToken)
        {
        bvector<ECInstanceKey> instanceKeys;
        for (NavNodeKeyCPtr key : nodeKeys)
            {
            if (nullptr != key->AsECInstanceNodeKey())
                {
                ContainerHelpers::TransformContainer(instanceKeys, key->AsECInstanceNodeKey()->GetInstanceKeys(), [](auto const& k)
                    {
                    return ECInstanceKey(k.GetClass()->GetId(), k.GetId());
                    });
                continue;
                }

            NavNodeCPtr nodePtr = m_nodesLocater->LocateNode(*key);
            if (nodePtr.IsNull() || NavNodesHelper::IsCustomNode(*nodePtr))
                continue;

            m_nodeInstanceKeysProvider->IterateInstanceKeys(*nodePtr, [&](ECInstanceKey k)
                {
                instanceKeys.push_back(k);
                return true;
                });
            }
        return instanceKeys;
        }

public:
    ContentRulesSpecificationsInputHandler(RulesDrivenECPresentationManagerImpl const& manager, ContentProviderContextCR context)
        : m_context(context)
        {
        auto locaterParams = RequestWithRulesetImplParams::Create(context.GetConnection(), &context.GetCancelationToken(),
            context.GetRuleset().GetRuleSetId(), context.GetRulesetVariables());
        locaterParams.SetUnitSystem(context.GetUnitSystem());
        m_nodesLocater = std::make_unique<RulesDrivenECPresentationManagerImpl::NavNodeLocater>(manager, locaterParams);
        m_nodeInstanceKeysProvider = m_context.CreateNodeInstanceKeysProvider();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentRuleInstanceKeysContainer HandleSpecifications(ContentRuleInputKeysContainer& specs, ICancelationTokenCP cancelationToken)
        {
        auto scope = Diagnostics::Scope::Create("Convert content rule node keys to instance keys");
        ContentRuleInstanceKeysContainer instanceSpecs;
        if (!m_nodeInstanceKeysProvider)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_TRACE, LOG_WARNING, "Failed to create node instance keys provider. Returning empty list.");
            return instanceSpecs;
            }
        for (ContentRuleInputKeys& spec : specs)
            {
            bvector<ECInstanceKey> instanceKeys = GetECInstanceKeys(spec.GetMatchingNodeKeys(), cancelationToken);
            instanceSpecs.push_back(ContentRuleInstanceKeys(spec.GetRule(), instanceKeys));
            }
        return instanceSpecs;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderContextPtr RulesDrivenECPresentationManagerImpl::CreateContentProviderContext(IConnectionCR connection, ContentProviderKey const& key, std::unique_ptr<RulesetVariables> variables, ICancelationTokenCP cancelationToken) const
    {
    auto scope = Diagnostics::Scope::Create("Create content provider context");

    // get the ruleset
    PresentationRuleSetPtr ruleset = FindRuleset(GetLocaters(), connection, key.GetRulesetId().c_str());
    if (!ruleset.IsValid())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Ruleset '%s' not found. Returning NULL.", key.GetRulesetId().c_str()));
        return nullptr;
        }

    // get nodes cache
    std::shared_ptr<INavNodesCache> nodesCache = m_nodesCachesManager->GetCache(connection.GetId());
    if (nullptr == nodesCache)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Failed to find the hierarchy cache for given connection: '%s'. Returning NULL.", connection.GetId().c_str()));
    nodesCache->OnRulesetUsed(*ruleset); // make sure latest ruleset version is used in cache

    // get caches
    ECExpressionsCache& ecexpressionsCache = m_rulesetECExpressionsCache->Get(ruleset->GetRuleSetId().c_str());
    RelatedPathsCache& relatedPathsCache = m_ecdbCaches->GetRelatedPathsCache(connection);

    // set up the provider context
    ContentProviderContextPtr context = ContentProviderContext::Create(*ruleset, key.GetPreferredDisplayType(), key.GetContentFlags(), key.GetInputNodeKeys(), nodesCache,
        GetCategorySupplier(), std::move(variables), ecexpressionsCache, relatedPathsCache, *m_nodesFactory, GetLocalState());
    context->SetQueryContext(*m_connections, connection);
    context->SetPropertyFormattingContext(GetECPropertyFormatter(), key.GetUnitSystem());
    context->SetCancelationToken(cancelationToken);
    if (nullptr != key.GetSelectionInfo())
        context->SetSelectionInfo(*key.GetSelectionInfo());

    return context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProviderPtr RulesDrivenECPresentationManagerImpl::GetContentProvider(IConnectionCR connection, ICancelationTokenCP cancelationToken, ContentProviderKey const& key, RulesetVariables const& variables) const
    {
    auto scope = Diagnostics::Scope::Create("Get content provider");

    // init ruleset variables from input & user settings
    std::unique_ptr<RulesetVariables> rulesetVariables = std::make_unique<RulesetVariables>(variables);
    rulesetVariables->Merge(GetUserSettingsManager().GetSettings(key.GetRulesetId()));

    // attempt to find cached provider
    BeMutexHolder contentCacheLock(m_contentCache->GetMutex());
    SpecificationContentProviderPtr provider = m_contentCache->GetProvider(key, *rulesetVariables);
    if (provider.IsValid())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Found cached provider. Adopt and return.");
        provider->Adopt(connection, cancelationToken);
        return provider;
        }

    // create the context
    auto context = CreateContentProviderContext(connection, key, std::move(rulesetVariables), cancelationToken);
    if (context.IsNull())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_WARNING, "Failed to create context. Returning NULL.");
        return nullptr;
        }

    // get content specifications
    NodeLabelCalculator nodeLabelCalculator(context->GetSchemaHelper(), *m_connections, connection, key.GetRulesetId(), context->GetRulesPreprocessor(), variables, context->GetECExpressionsCache(), *m_nodesFactory);
    IRulesPreprocessor::ContentRuleParameters params(key.GetInputNodeKeys(), key.GetPreferredDisplayType(), key.GetSelectionInfo(), nodeLabelCalculator, &context->GetNodesLocater());
    ContentRuleInputKeysContainer specs = context->GetRulesPreprocessor().GetContentSpecifications(params);
    DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_TRACE, LOG_INFO, Utf8PrintfString("Creating content provider using %" PRIu64 " specifications.", (uint64_t)specs.size()));

    // create the provider
    ContentRulesSpecificationsInputHandler inputHandler(*this, *context);
    ContentRuleInstanceKeysContainer instanceSpecs = inputHandler.HandleSpecifications(specs, cancelationToken);
    provider = SpecificationContentProvider::Create(*context, instanceSpecs);
    m_contentCache->CacheProvider(key, *provider);
    return provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SelectClassInfo> RulesDrivenECPresentationManagerImpl::_GetContentClasses(ContentClassesRequestImplParams const& params)
    {
    auto scope = Diagnostics::Scope::Create("Get content classes");

    // get the ruleset
    PresentationRuleSetPtr ruleset = FindRuleset(GetLocaters(), params.GetConnection(), params.GetRulesetId().c_str());
    if (!ruleset.IsValid())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Ruleset '%s' not found. Returning empty list.", params.GetRulesetId().c_str()));
        return bvector<SelectClassInfo>();
        }

    std::shared_ptr<INavNodesCache> nodesCache = m_nodesCachesManager->GetCache(params.GetConnection().GetId());
    if (nullptr == nodesCache)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Failed to find the hierarchy cache for given connection: '%s'. Returning empty list.", params.GetConnection().GetId().c_str()));
    // make sure latest ruleset version is used in cache
    nodesCache->OnRulesetUsed(*ruleset);

    // get ruleset-related caches
    IUserSettings const& settings = GetUserSettingsManager().GetSettings(ruleset->GetRuleSetId().c_str());
    ECExpressionsCache& ecexpressionsCache = m_rulesetECExpressionsCache->Get(ruleset->GetRuleSetId().c_str());

    RulesetVariables rulesetVariables(params.GetRulesetVariables());
    rulesetVariables.Merge(settings);

    // get connection-related caches
    RelatedPathsCache& relatedPathsCache = m_ecdbCaches->GetRelatedPathsCache(params.GetConnection());

    // locate the classes
    Utf8CP preferredDisplayType = params.GetPreferredDisplayType().size() ? params.GetPreferredDisplayType().c_str() : ContentDisplayType::Undefined;
    ECSchemaHelper schemaHelper(params.GetConnection(), &relatedPathsCache, &ecexpressionsCache);
    RulesPreprocessor rulesPreprocessor(*m_connections, params.GetConnection(), *ruleset, rulesetVariables, nullptr, ecexpressionsCache);
    ContentClassesLocater::Context locaterContext(schemaHelper, *m_connections, params.GetConnection(), params.GetCancellationToken(), rulesPreprocessor,
        *ruleset, preferredDisplayType, rulesetVariables, *nodesCache, *m_nodesFactory);
    locaterContext.SetContentFlagsCalculator([contentFlags = params.GetContentFlags()](int){return contentFlags | (int)ContentFlags::DescriptorOnly;});
    auto result = ContentClassesLocater(locaterContext).Locate(params.GetInputClasses());
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_INFO, Utf8PrintfString("Returning %" PRIu64 " content classes.", (uint64_t)result.size()));
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorCPtr RulesDrivenECPresentationManagerImpl::_GetContentDescriptor(ContentDescriptorRequestImplParams const& params)
    {
    auto scope = Diagnostics::Scope::Create("Get content descriptor");

    Utf8CP preferredDisplayType = params.GetPreferredDisplayType().size() ? params.GetPreferredDisplayType().c_str() : ContentDisplayType::Undefined;
    INavNodeKeysContainerCPtr nodeKeys = params.GetInputKeys().GetAllNavNodeKeys();
    ContentProviderKey key(params.GetConnection().GetId(), params.GetRulesetId(), preferredDisplayType, params.GetContentFlags(), params.GetUnitSystem(), *nodeKeys, params.GetSelectionInfo());
    ContentProviderCPtr provider = GetContentProvider(params.GetConnection(), params.GetCancellationToken(), key, params.GetRulesetVariables());
    if (provider.IsNull())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Failed to get content provider. Returning NULL.");
        return nullptr;
        }

    auto descriptor = provider->GetContentDescriptor();
    if (!descriptor)
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_INFO, LOG_INFO, "Returning NULL content descriptor (given specifications didn't result in any content).");
        return nullptr;
        }
    DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_INFO, LOG_INFO, Utf8PrintfString("Returning descriptor with %" PRIu64 " content classes and %" PRIu64 " fields.",
        (uint64_t)descriptor->GetSelectClasses().size(), (uint64_t)descriptor->GetVisibleFields().size()));
    return descriptor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProviderPtr RulesDrivenECPresentationManagerImpl::GetContentProvider(ContentRequestImplParams const& params) const
    {
    ContentDescriptorCR descriptor = params.GetContentDescriptor();
    ContentProviderKey key(params.GetConnection().GetId(), descriptor.GetRuleset().GetRuleSetId(), descriptor.GetPreferredDisplayType(), descriptor.GetContentFlags(),
        descriptor.GetUnitSystem(), descriptor.GetInputNodeKeys(), descriptor.GetSelectionInfo());
    return GetContentProvider(params.GetConnection(), params.GetCancellationToken(), key, descriptor.GetRulesetVariables());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentCPtr RulesDrivenECPresentationManagerImpl::_GetContent(WithPageOptions<ContentRequestImplParams> const& params)
    {
    auto scope = Diagnostics::Scope::Create("Get content");

    SpecificationContentProviderPtr provider = GetContentProvider(params);
    if (provider.IsNull())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Failed to get content provider. Returning NULL.");
        return nullptr;
        }

    auto providerDescriptor = provider->GetContentDescriptor();
    if (nullptr == providerDescriptor)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Failed to get content descriptor. Returning NULL.");
        return nullptr;
        }
    if (providerDescriptor != &params.GetContentDescriptor())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Received a modified descriptor, cloning provider.");
        provider = provider->Clone();
        provider->SetContentDescriptor(params.GetContentDescriptor());
        }

    auto initializationScope = Diagnostics::Scope::Create("Initialize content");
    provider->SetPageOptions(params.GetPageOptions());
    provider->Initialize();
    ContentPtr content = Content::Create(params.GetContentDescriptor(), *ContentSetDataSource::Create(*provider));
    initializationScope = nullptr;

    DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_INFO, LOG_INFO, Utf8PrintfString("Returning content with [%" PRIu64 ", %" PRIu64 ") records.",
        (uint64_t)params.GetPageOptions().GetPageStart(), params.GetContentDescriptor().MergeResults() ? 1 : (uint64_t)(params.GetPageOptions().GetPageStart() + content->GetContentSet().GetSize())));
    return content;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RulesDrivenECPresentationManagerImpl::_GetContentSetSize(ContentRequestImplParams const& params)
    {
    auto scope = Diagnostics::Scope::Create("Get content set size");

    SpecificationContentProviderPtr provider = GetContentProvider(params);
    if (provider.IsNull())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Failed to get content provider. Returning 0.");
        return 0;
        }

    auto providerDescriptor = provider->GetContentDescriptor();
    if (nullptr == providerDescriptor)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Failed to get content descriptor. Returning NULL.");
        return 0;
        }
    if (providerDescriptor != &params.GetContentDescriptor())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Received a modified descriptor, cloning provider.");
        provider = provider->Clone();
        provider->SetContentDescriptor(params.GetContentDescriptor());
        }

    auto queryCountScope = Diagnostics::Scope::Create("Query count");
    size_t size = provider->GetFullContentSetSize();
    queryCountScope = nullptr;

    DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_INFO, LOG_INFO, Utf8PrintfString("Returning %" PRIu64, (uint64_t)size));
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LabelDefinitionCPtr RulesDrivenECPresentationManagerImpl::_GetDisplayLabel(KeySetDisplayLabelRequestImplParams const& params)
    {
    auto scope = Diagnostics::Scope::Create("Get display label");

    Utf8String rulesetId(params.GetRulesetId());
    RulesetVariables rulesetVariables(params.GetRulesetVariables());
    if (rulesetId.empty())
        {
        rulesetId = DISPLAY_LABEL_RULESET_ID;
        rulesetVariables = RulesetVariables();
        }

    int contentFlags = ((int)ContentFlags::NoFields | (int)ContentFlags::ShowLabels | (int)ContentFlags::MergeResults);
    auto descriptorParams = ContentDescriptorRequestImplParams::Create(ContentDescriptorRequestParams(ContentMetadataRequestParams(rulesetId, rulesetVariables, ContentDisplayType::List, contentFlags), params.GetKeys()), params);

    ContentDescriptorCPtr descriptor = GetContentDescriptor(descriptorParams);
    if (descriptor.IsNull())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Failed to get content descriptor");
        return nullptr;
        }

    auto contentParams = ContentRequestImplParams::Create(params.GetConnection(), params.GetCancellationToken(), *descriptor);
    ContentCPtr content = GetContent(contentParams);
    if (content.IsNull())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Failed to get content");
        return nullptr;
        }

    ContentSetItemCPtr item = content->GetContentSet().Get(0);
    if (item.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Content contains invalid record. Returning invalid label.");

    DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_INFO, LOG_INFO, Utf8PrintfString("Returning '%s'", item->GetDisplayLabelDefinition().GetDisplayValue().c_str()));
    return &item->GetDisplayLabelDefinition();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PagingDataSourcePtr<DisplayValueGroupCPtr> RulesDrivenECPresentationManagerImpl::_GetDistinctValues(WithPageOptions<DistinctValuesRequestImplParams> const& params)
    {
    auto scope = Diagnostics::Scope::Create("Get distinct values");

    ContentDescriptor::Field const* field = params.GetContentDescriptor().FindField(params.GetDistinctFieldMatcher());
    if (field == nullptr)
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_INFO, LOG_ERROR, "Descriptor doesn't contain requested field");
        return nullptr;
        }

    SpecificationContentProviderCPtr contentProvider = GetContentProvider(ContentRequestImplParams::Create(params));
    if (contentProvider.IsNull())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Failed to get content provider");
        return nullptr;
        }

    auto providerDescriptor = contentProvider->GetContentDescriptor();
    if (nullptr == providerDescriptor)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Failed to get content descriptor");
        return nullptr;
        }

    auto queryScope = Diagnostics::Scope::Create("Query distinct values");
    IDataSourceCPtr<DisplayValueGroupCPtr> values = contentProvider->GetDistinctValues(*field);
    if (values.IsNull())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Got invalid data source");
        return nullptr;
        }

    auto pagedValues = PagingDataSource<DisplayValueGroupCPtr>::Create(*values, params.GetPageOptions().GetPageStart(), params.GetPageOptions().GetPageSize());
    DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_INFO, LOG_INFO, Utf8PrintfString("Returning a distinct values range of [%" PRIu64 ", %" PRIu64 ").",
        (uint64_t)params.GetPageOptions().GetPageStart(), (uint64_t)(params.GetPageOptions().GetPageStart() + pagedValues->GetSize())));
    return pagedValues;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct CompareReporter : IHierarchyChangesReporter
{
private:
    IHierarchyChangeRecordsHandler& m_recordsHandler;
    IConnectionCR m_connection;
    int m_recordsCount;
    int m_recordsThreshold;
protected:
    bool _OnStartCompare(NavNodesProviderCR, NavNodesProviderCR) override {return true;}
    void _Added(CombinedHierarchyLevelIdentifier const& hli, NavNodeCR node, NavNodeCPtr parentNode, size_t index) override
        {
        m_recordsHandler.Accept(HierarchyChangeRecord(ChangeType::Insert, hli.GetRulesetId(), m_connection.GetECDb().GetDbFileName(), node, parentNode, index));
        m_recordsCount++;
        }
    void _Removed(CombinedHierarchyLevelIdentifier const& hli, NavNodeCR node, NavNodeCPtr parent, uint64_t position) override
        {
        m_recordsHandler.Accept(HierarchyChangeRecord(ChangeType::Delete, hli.GetRulesetId(), m_connection.GetECDb().GetDbFileName(), node, parent, position));
        m_recordsCount++;
        }
    void _Changed(CombinedHierarchyLevelIdentifier const& hli, NodeChanges const& changes) override
        {
        if (changes.GetNumChangedFields() > 0)
            {
            m_recordsHandler.Accept(HierarchyChangeRecord(hli.GetRulesetId(), m_connection.GetECDb().GetDbFileName(), changes));
            m_recordsCount++;
            }
        }
    bool _ShouldContinue() override
        {
        return m_recordsThreshold < 0 || m_recordsCount < m_recordsThreshold;
        }
public:
    CompareReporter(IHierarchyChangeRecordsHandler& handler, IConnectionCR connection) : m_recordsHandler(handler), m_connection(connection), m_recordsCount(0), m_recordsThreshold(-1) {}
    void SetRecordsThreshold(int count) {m_recordsThreshold = count;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchyComparePositionPtr RulesDrivenECPresentationManagerImpl::_CompareHierarchies(HierarchyCompareRequestImplParams const& params)
    {
    auto scope = Diagnostics::Scope::Create("Compare hierarchies");

    std::shared_ptr<INavNodesCache> nodesCache = m_nodesCachesManager->GetPersistentCache(params.GetConnection().GetId());
    if (nullptr == nodesCache)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Failed to find the hierarchy cache for given connection: '%s'. Returning.", params.GetConnection().GetId().c_str()));

    // get the rulesets
    PresentationRuleSetPtr lhsRuleset = FindRuleset(GetLocaters(), params.GetConnection(), params.GetLhsRulesetId().c_str());
    if (!lhsRuleset.IsValid())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("LHS ruleset '%s' not found. Returning.", params.GetLhsRulesetId().c_str()));
        return nullptr;
        }

    PresentationRuleSetPtr rhsRuleset = FindRuleset(GetLocaters(), params.GetConnection(), params.GetRhsRulesetId().c_str());
    if (!rhsRuleset.IsValid())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("RHS ruleset '%s' not found. Returning.", params.GetRhsRulesetId().c_str()));
        return nullptr;
        }

    if (lhsRuleset->GetHash().Equals(rhsRuleset->GetHash()) && params.GetLhsVariables() == params.GetRhsVariables())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_INFO, "Hashes and ruleset variables of LHS and RHS rulesets match, meaning that hierarchies are equal. Returning.");
        return nullptr;
        }

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_INFO, Utf8PrintfString("Comparing '%s' and '%s'", lhsRuleset->GetRuleSetId().c_str(), rhsRuleset->GetRuleSetId().c_str()));

    BeMutexHolder lock(nodesCache->GetMutex());
    IHierarchyCache::SavepointPtr cacheSavepoint = nodesCache->CreateSavepoint();

    nodesCache->OnRulesetUsed(*lhsRuleset);
    nodesCache->OnRulesetUsed(*rhsRuleset);

    CompareReporter reporter(*params.GetRecordsHandler(), params.GetConnection());
    if (params.GetResultSize() > 0)
        reporter.SetRecordsThreshold(params.GetResultSize());

    CombinedHierarchyLevelIdentifier lhsInfo(params.GetConnection().GetId(), params.GetLhsRulesetId(), BeGuid());
    CombinedHierarchyLevelIdentifier rhsInfo(params.GetConnection().GetId(), params.GetRhsRulesetId(), BeGuid());
    HierarchiesComparer comparer(HierarchiesComparer::ComparerParams(*m_connections, nodesCache, *m_nodesProviderContextFactory, *m_nodesProviderFactory));
    auto result = comparer.Compare(HierarchiesComparer::CompareWithConnectionParams(reporter, params.GetConnection(), lhsInfo, params.GetLhsVariables(), rhsInfo, params.GetRhsVariables(),
        params.GetExpandedNodeKeys(), params.GetContinuationToken(), true, true, params.GetCancellationToken()));
    if (result.GetStatus() == HierarchyCompareStatus::Complete)
        return nullptr;

    return std::make_shared<HierarchyComparePosition>(result.GetContinuationToken());
    }
