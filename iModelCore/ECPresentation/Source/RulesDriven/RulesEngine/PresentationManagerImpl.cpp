/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/PresentationManagerImpl.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include "PresentationManagerImpl.h"
#include "RulesPreprocessor.h"
#include "QueryExecutor.h"
#include "NavNodeProviders.h"
#include "ContentProviders.h"
#include "LocalizationHelper.h"
#include "NavNodesDataSource.h"
#include "UpdateHandler.h"
#include "LoggingHelper.h"
#include "CustomizationHelper.h"
#include "QueryBuilder.h"
#include "ECExpressionContextsProvider.h"
#include "ECInstanceChangesDirector.h"
#include "ContentClassesLocater.h"
#include "UsedClassesListener.h"

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                04/2018
//=======================================================================================
struct RulesDrivenECPresentationManager::Impl::CompositeUpdateRecordsHandler : IUpdateRecordsHandler
{
private:
    bvector<RefCountedPtr<IUpdateRecordsHandler>> m_handlers;
protected:
    void _Start() override {std::for_each(m_handlers.begin(), m_handlers.end(), [](RefCountedPtr<IUpdateRecordsHandler> h){h->Start();});}
    void _Accept(UpdateRecord const& record) override {std::for_each(m_handlers.begin(), m_handlers.end(), [&record](RefCountedPtr<IUpdateRecordsHandler> h){h->Accept(record);});}
    void _Accept(FullUpdateRecord const& record) override {std::for_each(m_handlers.begin(), m_handlers.end(), [&record](RefCountedPtr<IUpdateRecordsHandler> h){h->Accept(record);});}
    void _Finish() override {std::for_each(m_handlers.begin(), m_handlers.end(), [](RefCountedPtr<IUpdateRecordsHandler> h){h->Finish();});}
public:
    void Register(IUpdateRecordsHandler& handler)
        {
        m_handlers.push_back(&handler);
        }
    bool Unregister(IUpdateRecordsHandler& handler)
        {
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IRulesetLocaterManager* RulesDrivenECPresentationManagerDependenciesFactory::_CreateRulesetLocaterManager(IConnectionManagerCR connections) const {return new RuleSetLocaterManager(connections);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IUserSettingsManager* RulesDrivenECPresentationManagerDependenciesFactory::_CreateUserSettingsManager(BeFileNameCR temporaryDirectory) const {return new UserSettingsManager(temporaryDirectory);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::Impl::Impl(IRulesDrivenECPresentationManagerDependenciesFactory const& dependenciesFactory, Params const& params)
    : m_localState(nullptr), m_ecPropertyFormatter(nullptr), m_categorySupplier(nullptr), m_localizationProvider(nullptr)
    {
    m_locaters = dependenciesFactory._CreateRulesetLocaterManager(params.GetConnections());
    m_userSettings = dependenciesFactory._CreateUserSettingsManager(params.GetPaths().GetTemporaryDirectory());
    m_compositeUpdateRecordsHandler = new CompositeUpdateRecordsHandler();
    m_compositeUpdateRecordsHandler->AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::Impl::~Impl()
    {
    m_compositeUpdateRecordsHandler->Release();
    DELETE_AND_CLEAR(m_userSettings);
    DELETE_AND_CLEAR(m_locaters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::Impl::SetLocalizationProvider(ILocalizationProvider const * provider)
    {
    m_localizationProvider = provider;
    GetUserSettingsManager().SetLocalizationProvider(provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::Impl::SetLocalState(IJsonLocalState* localState)
    {
    m_localState = localState;
    m_userSettings->SetLocalState(localState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IECPropertyFormatter const& RulesDrivenECPresentationManager::Impl::GetECPropertyFormatter() const
    {
    if (nullptr != m_ecPropertyFormatter)
        return *m_ecPropertyFormatter;

    static DefaultPropertyFormatter s_defaultPropertyFormatter;
    return s_defaultPropertyFormatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IPropertyCategorySupplier& RulesDrivenECPresentationManager::Impl::GetCategorySupplier() const
    {
    if (nullptr != m_categorySupplier)
        return *m_categorySupplier;

    static DefaultCategorySupplier s_defaultCategorySupplier;
    return s_defaultCategorySupplier;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::Impl::RegisterECInstanceChangeEventSource(ECInstanceChangeEventSource& source)
    {
    m_ecInstanceChangeEventSources.push_back(&source);
    _OnECInstanceChangeEventSourceRegistered(source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::Impl::UnregisterECInstanceChangeEventSource(ECInstanceChangeEventSource& source)
    {
    _OnECInstanceChangeEventSourceUnregister(source);
    m_ecInstanceChangeEventSources.erase(std::remove(m_ecInstanceChangeEventSources.begin(), m_ecInstanceChangeEventSources.end(), &source));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateRecordsHandler& RulesDrivenECPresentationManager::Impl::GetCompositeUpdateRecordsHandler() const {return *m_compositeUpdateRecordsHandler;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::Impl::RegisterUpdateRecordsHandler(IUpdateRecordsHandler& handler)
    {
    m_compositeUpdateRecordsHandler->Register(handler);
    _OnUpdateRecordsHandlerChanged();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::Impl::UnregisterUpdateRecordsHandler(IUpdateRecordsHandler& handler)
    {
    if (m_compositeUpdateRecordsHandler->Unregister(handler))
        _OnUpdateRecordsHandlerChanged();
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2016
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImpl::UsedClassesListener : IECDbUsedClassesListener
    {
    RulesDrivenECPresentationManagerImpl& m_manager;
    UsedClassesListener(RulesDrivenECPresentationManagerImpl& manager) : m_manager(manager) {}
    void _OnClassUsed(ECDbCR db, ECClassCR ecClass, bool polymorphically) override
        {
        for (ECInstanceChangeEventSourcePtr source : m_manager.GetECInstanceChangeEventSources())
            source->NotifyClassUsed(db, ecClass, polymorphically);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsRulesetSupported(IConnectionCR connection, PresentationRuleSetCR ruleset)
    {
    ECSchemaHelper helper(connection, nullptr, nullptr, nullptr, nullptr);
    return helper.AreSchemasSupported(ruleset.GetSupportedSchemas());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static PresentationRuleSetPtr FindRuleset(IRulesetLocaterManager const& locaters, IConnectionCR connection, Utf8CP rulesetId)
    {
    PresentationRuleSetPtr ruleset = RulesPreprocessor::GetPresentationRuleSet(locaters, connection, rulesetId);
    if (!ruleset.IsValid())
        {
        LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("Invalid ruleset ID: '%s'", rulesetId).c_str(), LOG_ERROR);
        return nullptr;
        }
    if (!IsRulesetSupported(connection, *ruleset))
        {
        LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("Ruleset '%s' not supported by connection", rulesetId).c_str(), LOG_INFO);
        return nullptr;
        }
    return ruleset;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void RegisterDisplayLabelRuleset(IRulesetLocaterManager& locaters)
    {
    RefCountedPtr<SimpleRuleSetLocater> locater = SimpleRuleSetLocater::Create();
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(DISPLAY_LABEL_RULESET_ID, 1, 0, false, "", "", "", false);
    ruleset->AddPresentationRule(*new ContentRule());
    ruleset->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());
    locater->AddRuleSet(*ruleset);
    locaters.RegisterLocater(*locater);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImpl::RulesetECExpressionsCache : IECExpressionsCacheProvider
{
private:
    bmap<Utf8String, ECExpressionsCache*> m_caches;
protected:
    ECExpressionsCache& _Get(Utf8CP rulesetId) override
        {
        auto iter = m_caches.find(rulesetId);
        if (m_caches.end() == iter)
            iter = m_caches.Insert(rulesetId, new ECExpressionsCache()).first;
        return *iter->second;
        }
public:
    ~RulesetECExpressionsCache()
        {
        for (auto pair : m_caches)
            delete pair.second;
        }
    void Clear(Utf8CP rulesetId)
        {
        auto iter = m_caches.find(rulesetId);
        if (m_caches.end() != iter)
            {
            delete iter->second;
            m_caches.erase(iter);
            }
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImpl::ECDbCaches : IConnectionsListener, IECSqlStatementCacheProvider
{
    struct Caches
        {
        RelatedPathsCache* m_relatedPathsCache;
        PolymorphicallyRelatedClassesCache* m_polymorphicallyRelatedClassesCache;
        ECSqlStatementCache* m_statementsCache;
        Caches()
            {
            m_relatedPathsCache = new RelatedPathsCache();
            m_polymorphicallyRelatedClassesCache = new PolymorphicallyRelatedClassesCache();
            m_statementsCache = new ECSqlStatementCache(50);
            }
        ~Caches()
            {
            DELETE_AND_CLEAR(m_relatedPathsCache);
            DELETE_AND_CLEAR(m_polymorphicallyRelatedClassesCache);
            DELETE_AND_CLEAR(m_statementsCache);
            }
        };

private:
    IConnectionManagerCR m_connections;
    mutable bmap<Utf8String, Caches*> m_caches;

private:
    Caches& GetCaches(IConnectionCR connection) const
        {
        auto iter = m_caches.find(connection.GetId());
        if (m_caches.end() == iter)
            iter = m_caches.Insert(connection.GetId(), new Caches()).first;
        return *iter->second;
        }

protected:
    void _OnConnectionEvent(ConnectionEvent const& evt) override
        {
        auto iter = m_caches.find(evt.GetConnection().GetId());
        if (m_caches.end() != iter)
            {
            delete iter->second;
            m_caches.erase(iter);
            }
        }
    ECSqlStatementCache& _GetECSqlStatementCache(IConnectionCR connection) override {return GetStatementsCache(connection);}
public:
    ECDbCaches(IConnectionManagerCR connections)
        : m_connections(connections)
        {
        m_connections.AddListener(*this);
        }
    ~ECDbCaches()
        {
        m_connections.DropListener(*this);
        for (auto iter : m_caches)
            delete iter.second;
        }
    RelatedPathsCache& GetRelatedPathsCache(IConnectionCR connection) const {return *GetCaches(connection).m_relatedPathsCache;}
    PolymorphicallyRelatedClassesCache& GetPolymorphicallyRelatedClassesCache(IConnectionCR connection) const {return *GetCaches(connection).m_polymorphicallyRelatedClassesCache;}
    ECSqlStatementCache& GetStatementsCache(IConnectionCR connection) const {return *GetCaches(connection).m_statementsCache;}
};

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                01/2018
+===============+===============+===============+===============+===============+======*/
struct NavNodeLocater
{
private:
    RulesDrivenECPresentationManagerImpl& m_manager;
    IConnectionCR m_connection;
    RulesDrivenECPresentationManager::NavigationOptions m_options;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas                01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeCPtr LocateNodeInHierarchy(bvector<Utf8String> const& path, int index, NavNodeCP parentNode, ICancelationTokenCR cancelationToken)
        {
        if (path.size() <= index)
            return nullptr;

        INavNodesDataSourcePtr nodes;
        if (!parentNode)
            nodes = m_manager.GetRootNodes(m_connection, PageOptions(), m_options, cancelationToken);
        else
            nodes = m_manager.GetChildren(m_connection, *parentNode, PageOptions(), m_options, cancelationToken);


        size_t nodesCount = nodes->GetSize();
        NavNodeCPtr node;
        bool found = false;
        for (size_t i = 0; i < nodesCount; ++i)
            {
            node = nodes->GetNode(i);
            bvector<Utf8String> const& nodePath = node->GetKey()->GetPathFromRoot();
            if (nodePath.size() <= index)
                {
                BeAssert(false);
                break;
                }

            found = true;
            size_t virtualIndex = index;
            for (; virtualIndex < nodePath.size() && virtualIndex < path.size(); ++virtualIndex)
                {
                if (!nodePath[virtualIndex].Equals(path[virtualIndex]))
                    {
                    found = false;
                    break;
                    }
                }

            if (found)
                {
                index = virtualIndex - 1;
                break;
                }
            }

        if (!found)
            return nullptr;

        if (path.size() == index + 1)
            return node;

        return LocateNodeInHierarchy(path, index + 1, node.get(), cancelationToken);
        }

public:
    NavNodeLocater(RulesDrivenECPresentationManagerImpl& manager, IConnectionCR connection, RulesDrivenECPresentationManager::NavigationOptions options)
        : m_manager(manager), m_connection(connection), m_options(options)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas                01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeCPtr LocateNode(NavNodeKeyCR nodeKey, ICancelationTokenCR cancelationToken)
        {
        NavNodeCPtr node = m_manager.GetNodesCache().LocateNode(m_connection, m_options.GetLocale(), nodeKey);
        if (node.IsNull())
            node = LocateNodeInHierarchy(nodeKey.GetPathFromRoot(), 0, nullptr, cancelationToken);

        return node;
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImpl::NodesProviderFactory : INodesProviderFactory
{
private:
    RulesDrivenECPresentationManagerImpl& m_manager;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr CreateProvider(NavNodesProviderContextR context, JsonNavNodeCP parent) const
        {
        RulesPreprocessor preprocessor(m_manager.m_connections, context.GetConnection(), context.GetRuleset(),
            context.GetLocale(), context.GetUserSettings(), &context.GetUsedSettingsListener(), context.GetECExpressionsCache());
        NavNodesProviderPtr provider;
        if (nullptr == parent)
            {
            RulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
            RootNodeRuleSpecificationsList specs = preprocessor.GetRootNodeSpecifications(params);
            if (!specs.empty())
                provider = MultiSpecificationNodesProvider::Create(context, specs);
            }
        else
            {
            RulesPreprocessor::ChildNodeRuleParameters params(*parent, TargetTree_MainTree);
            ChildNodeRuleSpecificationsList specs = preprocessor.GetChildNodeSpecifications(params);
            if (!specs.empty())
                provider = MultiSpecificationNodesProvider::Create(context, specs, *parent);
            }
        if (provider.IsNull())
            {
            if (nullptr != parent)
                context.SetChildNodeContext(nullptr, *parent);
            provider = EmptyNavNodesProvider::Create(context);
            }
        return provider;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void AdoptProvider(NavNodesProviderR provider, NavNodesProviderContextCR context)
        {
        provider.GetContextR().SetCancelationToken(&context.GetCancelationToken());
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr _Create(NavNodesProviderContextR context, JsonNavNodeCP parent, ProviderCacheType cacheType) const override
        {
        NavNodesProviderPtr provider;
        switch (cacheType)
            {
            case ProviderCacheType::None:
                break;
            case ProviderCacheType::Partial:
                provider = m_manager.m_nodesCache->GetDataSource(context.GetDataSourceInfo());
                break;
            case ProviderCacheType::Full:
                {
                uint64_t parentId = parent ? parent->GetNodeId() : 0;
                HierarchyLevelInfo info = m_manager.m_nodesCache->FindHierarchyLevel(context.GetConnection().GetId().c_str(),
                    context.GetRuleset().GetRuleSetId().c_str(), context.GetLocale().c_str(), parent ? &parentId : nullptr);
                if (info.IsValid())
                    provider = m_manager.m_nodesCache->GetHierarchyLevel(info);
                break;
                }
            case ProviderCacheType::Combined:
                {
                CombinedHierarchyLevelInfo info(context.GetConnection().GetId(), context.GetRuleset().GetRuleSetId(),
                    context.GetLocale(), parent ? parent->GetNodeId() : 0);
                provider = m_manager.m_nodesCache->GetCombinedHierarchyLevel(info);
                break;
                }
            }
        if (provider.IsNull())
            provider = CreateProvider(context, parent);
        else
            AdoptProvider(*provider, context);
        return provider;
        }

public:
    NodesProviderFactory(RulesDrivenECPresentationManagerImpl& mgr) : m_manager(mgr) {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerImpl::NodesProviderContextFactory : INodesProviderContextFactory
{
private:
    RulesDrivenECPresentationManagerImpl& m_manager;

protected:
    NavNodesProviderContextPtr _Create(IConnectionCR connection, Utf8CP rulesetId, Utf8CP locale, uint64_t const* parentNodeId, ICancelationTokenCP cancelationToken, bool disableUpdates) const override
        {
        // get the ruleset
        RefCountedPtr<PerformanceLogger> _l2 = LoggingHelper::CreatePerformanceLogger(Log::Navigation, "[NodesProviderContextFactory::Create] Get ruleset", NativeLogging::LOG_TRACE);
        PresentationRuleSetPtr ruleset = FindRuleset(m_manager.GetLocaters(), connection, rulesetId);
        if (!ruleset.IsValid())
            return nullptr;
        _l2 = nullptr;

        if (nullptr != cancelationToken && cancelationToken->IsCanceled())
            return nullptr;

        // get various caches
        IUserSettings const& settings = m_manager.GetUserSettings(rulesetId);
        ECExpressionsCache& ecexpressionsCache = m_manager.m_rulesetECExpressionsCache->Get(rulesetId);
        RelatedPathsCache& relatedPathsCache = m_manager.m_ecdbCaches->GetRelatedPathsCache(connection);
        PolymorphicallyRelatedClassesCache& polymorphicallyRelatedClassesCache = m_manager.m_ecdbCaches->GetPolymorphicallyRelatedClassesCache(connection);
        ECSqlStatementCache& statementsCache = m_manager.m_ecdbCaches->GetStatementsCache(connection);

        // notify listener with ECClasses used in this ruleset
        ECSchemaHelper schemaHelper(connection, &relatedPathsCache, &polymorphicallyRelatedClassesCache, &statementsCache, &ecexpressionsCache);
        UsedClassesHelper::NotifyListenerWithRulesetClasses(*m_manager.m_usedClassesListener, ecexpressionsCache, connection, *ruleset);

        // set up the nodes provider context
        _l2 = LoggingHelper::CreatePerformanceLogger(Log::Navigation, "[NodesProviderContextFactory::Create] Create context", NativeLogging::LOG_TRACE);
        NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*ruleset, true, TargetTree_MainTree, locale, parentNodeId,
            settings, ecexpressionsCache, relatedPathsCache, polymorphicallyRelatedClassesCache, *m_manager.m_nodesFactory, m_manager.GetNodesCache(),
            *m_manager.m_nodesProviderFactory, m_manager.GetLocalState());
        context->SetQueryContext(m_manager.m_connections, connection, statementsCache, *m_manager.m_customFunctions, m_manager.m_usedClassesListener);
        
        ILocalizationProvider const* localizationProvider = m_manager.GetLocalizationProvider();
        if (nullptr != localizationProvider)
            context->SetLocalizationContext(*localizationProvider);

        context->SetIsUpdatesDisabled(disableUpdates);
        context->SetCancelationToken(cancelationToken);
        _l2 = nullptr;

        return context;
        }
public:
    NodesProviderContextFactory(RulesDrivenECPresentationManagerImpl& mgr) : m_manager(mgr) {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManagerImpl::RulesDrivenECPresentationManagerImpl(IRulesDrivenECPresentationManagerDependenciesFactory const& dependenciesFactory, Params const& params)
    : RulesDrivenECPresentationManager::Impl(dependenciesFactory, params), m_connections(params.GetConnections())
    {
    m_customFunctions = new CustomFunctionsInjector(m_connections);
    m_rulesetECExpressionsCache = new RulesetECExpressionsCache();
    m_ecdbCaches = new ECDbCaches(m_connections);
    m_nodesProviderContextFactory = new NodesProviderContextFactory(*this);
    m_nodesProviderFactory = new NodesProviderFactory(*this);
    m_usedClassesListener = new UsedClassesListener(*this);
    m_nodesFactory = new JsonNavNodesFactory();
    m_nodesCache = new NodesCache(params.GetPaths().GetTemporaryDirectory(), *m_nodesFactory, *m_nodesProviderContextFactory,
        m_connections, GetUserSettingsManager(), *m_ecdbCaches, params.ShouldDisableDiskCache() ? NodesCacheType::Memory : NodesCacheType::Disk);
    m_nodesCache->SetCacheFileSizeLimit(params.GetDiskCacheFileSizeLimit());
    m_contentCache = new ContentCache();
    m_updateHandler = new UpdateHandler(m_nodesCache, m_contentCache, m_connections, *m_nodesProviderContextFactory,
        *m_nodesProviderFactory, *m_rulesetECExpressionsCache);

    GetLocaters().SetRulesetCallbacksHandler(this);
    GetUserSettingsManager().SetChangesListener(this);
    m_connections.AddListener(*this);

    RegisterDisplayLabelRuleset(GetLocaters());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManagerImpl::~RulesDrivenECPresentationManagerImpl()
    {
    // clean up
    DELETE_AND_CLEAR(m_updateHandler);
    DELETE_AND_CLEAR(m_contentCache);
    DELETE_AND_CLEAR(m_nodesCache);
    DELETE_AND_CLEAR(m_nodesFactory);
    DELETE_AND_CLEAR(m_usedClassesListener);
    DELETE_AND_CLEAR(m_nodesProviderFactory);
    DELETE_AND_CLEAR(m_nodesProviderContextFactory);
    DELETE_AND_CLEAR(m_ecdbCaches);
    DELETE_AND_CLEAR(m_rulesetECExpressionsCache);
    DELETE_AND_CLEAR(m_customFunctions);

    m_connections.DropListener(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnRulesetCreated(RuleSetLocaterCR, PresentationRuleSetR ruleset)
    {
    IUserSettings& settings = GetUserSettings(ruleset.GetRuleSetId().c_str());
    settings.InitFrom(ruleset.GetUserSettings());
    m_nodesCache->OnRulesetCreated(ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnRulesetDispose(RuleSetLocaterCR, PresentationRuleSetCR ruleset)
    {
    m_rulesetECExpressionsCache->Clear(ruleset.GetRuleSetId().c_str());
    m_updateHandler->NotifyRulesetDisposed(ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const
    {
    CustomFunctionsManager::GetManager()._OnSettingChanged(rulesetId, settingId);
    m_updateHandler->NotifySettingChanged(rulesetId, settingId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
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
        m_contentCache->ClearCache(evt.GetConnection());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnCategoriesChanged()
    {
    m_updateHandler->NotifyCategoriesChanged();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnECInstancesChanged(ECDbCR db, bvector<ECInstanceChangeEventSource::ChangedECInstance> changes)
    {
    IConnectionPtr connection = m_connections.GetConnection(db);
    if (connection.IsNull())
        {
        BeAssert(false);
        return;
        }

    m_ecdbCaches->GetPolymorphicallyRelatedClassesCache(*connection).Clear();

    if (!changes.empty())
        m_updateHandler->NotifyECInstancesChanged(*connection, changes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodesDataSourcePtr RulesDrivenECPresentationManagerImpl::GetCachedDataSource(IConnectionCR connection, ICancelationTokenCR cancelationToken, NavigationOptions const& options)
    {
    // create the nodes provider context
    NavNodesProviderContextPtr context = m_nodesProviderContextFactory->Create(connection, options.GetRulesetId(), options.GetLocale(),
        nullptr, &cancelationToken, options.GetDisableUpdates());
    if (context.IsNull())
        return nullptr;

    // create the nodes provider
    NavNodesProviderPtr provider = m_nodesProviderFactory->Create(*context, nullptr, ProviderCacheType::Combined);

    // cache the provider in quick cache
    if (provider.IsValid())
        {
        CombinedHierarchyLevelInfo info(connection.GetId(), options.GetRulesetId(), options.GetLocale(), 0);
        m_nodesCache->CacheHierarchyLevel(info, *provider);
        }

    return NavNodesDataSource::Create(*provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodesDataSourcePtr RulesDrivenECPresentationManagerImpl::_GetRootNodes(IConnectionCR connection, PageOptionsCR pageOpts, NavigationOptions const& options, ICancelationTokenCR cancelationToken)
    {
    INavNodesDataSourcePtr source = GetCachedDataSource(connection, cancelationToken, options);
    if (source.IsNull())
        source = EmptyNavNodesDataSource::Create();
    source = PagingDataSource::Create(*source, pageOpts.GetPageStart(), pageOpts.GetPageSize());
    LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[GetRootNodes] Returned [%" PRIu64 ", %" PRIu64 ") nodes",
        (uint64_t)pageOpts.GetPageStart(), (uint64_t)(pageOpts.GetPageStart() + source->GetSize())).c_str());
    return source;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RulesDrivenECPresentationManagerImpl::_GetRootNodesCount(IConnectionCR connection, NavigationOptions const& options, ICancelationTokenCR cancelationToken)
    {
    INavNodesDataSourcePtr source = GetCachedDataSource(connection, cancelationToken, options);
    size_t size = source.IsValid() ? source->GetSize() : 0;
    LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[GetRootNodesCount] Returned %" PRIu64 " root nodes", (uint64_t)size).c_str());
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodesDataSourcePtr RulesDrivenECPresentationManagerImpl::GetCachedDataSource(IConnectionCR connection, ICancelationTokenCR cancelationToken, NavNodeCR parent, NavigationOptions const& options)
    {
    // create the nodes provider context
    uint64_t parentNodeId = parent.GetNodeId();
    NavNodesProviderContextPtr context = m_nodesProviderContextFactory->Create(connection, options.GetRulesetId(), options.GetLocale(),
        &parentNodeId, &cancelationToken, options.GetDisableUpdates());
    if (context.IsNull())
        return nullptr;

    // create the nodes provider
    JsonNavNodeCPtr jsonParent = GetNodesCache().GetNode(parentNodeId);
    NavNodesProviderPtr provider = m_nodesProviderFactory->Create(*context, jsonParent.get(), ProviderCacheType::Combined);

    // cache the provider in quick cache
    if (provider.IsValid())
        {
        CombinedHierarchyLevelInfo info(connection.GetId(), options.GetRulesetId(), options.GetLocale(), parentNodeId);
        m_nodesCache->CacheHierarchyLevel(info, *provider);
        }

    return NavNodesDataSource::Create(*provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodesDataSourcePtr RulesDrivenECPresentationManagerImpl::_GetChildren(IConnectionCR connection, NavNodeCR parent, PageOptionsCR pageOpts, NavigationOptions const& options, ICancelationTokenCR cancelationToken)
    {
    INavNodesDataSourcePtr source = GetCachedDataSource(connection, cancelationToken, parent, options);
    if (source.IsNull())
        source = EmptyNavNodesDataSource::Create();
    source = PagingDataSource::Create(*source, pageOpts.GetPageStart(), pageOpts.GetPageSize());
    LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[GetChildren] Returned [%" PRIu64 ", %" PRIu64 ") nodes for parent: '%s'",
        (uint64_t)pageOpts.GetPageStart(), (uint64_t)(pageOpts.GetPageStart() + source->GetSize()), parent.GetLabel().c_str()).c_str());
    return source;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RulesDrivenECPresentationManagerImpl::_GetChildrenCount(IConnectionCR connection, NavNodeCR parent, NavigationOptions const& options, ICancelationTokenCR cancelationToken)
    {
    INavNodesDataSourcePtr source = GetCachedDataSource(connection, cancelationToken, parent, options);
    size_t size = source.IsValid() ? source->GetSize() : 0;
    LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[GetChildrenCount] Returned %" PRIu64 " for parent: '%s'",
        (uint64_t)size, parent.GetLabel().c_str()).c_str());
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr RulesDrivenECPresentationManagerImpl::_GetParent(IConnectionCR, NavNodeCR node, NavigationOptions const& options, ICancelationTokenCR)
    {
    if (0 == node.GetParentNodeId())
        return nullptr;

    return GetNodesCache().GetNode(node.GetParentNodeId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr RulesDrivenECPresentationManagerImpl::_GetNode(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken)
    {
    NavNodeLocater locater(*this, connection, options);
    return locater.LocateNode(nodeKey, cancelationToken);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void TraverseHierarchy(RulesDrivenECPresentationManagerImpl& mgr, IConnectionCR connection, NavNodeCR root, RulesDrivenECPresentationManager::NavigationOptions const& options, ICancelationTokenCR cancelationToken)
    {
    if (!root.HasChildren())
        return;

    if (cancelationToken.IsCanceled())
        return;

    INavNodesDataSourceCPtr children = mgr.GetChildren(connection, root, PageOptions(), options, cancelationToken);
    size_t count = children->GetSize();
    for (size_t i = 0; i < count; ++i)
        {
        NavNodePtr child = children->GetNode(i);
        TraverseHierarchy(mgr, connection, *child, options, cancelationToken);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavNodeCPtr> RulesDrivenECPresentationManagerImpl::_GetFilteredNodes(IConnectionCR connection, Utf8CP filterText, NavigationOptions const& options, ICancelationTokenCR cancelationToken)
    {
    if (!GetNodesCache().IsHierarchyLevelCached(connection.GetId(), options.GetRulesetId(), options.GetLocale()))
        GetRootNodes(connection, PageOptions(), options, cancelationToken);

    NavNodesProviderPtr provider = GetNodesCache().GetUndeterminedNodesProvider(connection, options.GetRulesetId(),
        options.GetLocale(), options.GetDisableUpdates());
    if (provider.IsNull())
        return bvector<NavNodeCPtr>();

    size_t nodesCount = provider->GetNodesCount();
    for (size_t i = 0; i < nodesCount; i++)
        {
        JsonNavNodePtr node;
        provider->GetNode(node, i);
        TraverseHierarchy(*this, connection, *node, options, cancelationToken);
        }
    return GetNodesCache().GetFilteredNodes(connection, options.GetRulesetId(), options.GetLocale(), filterText);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool RulesDrivenECPresentationManagerImpl::_HasChild(IConnectionCR, NavNodeCR parent, ECInstanceKeyCR childKey, NavigationOptions const&, ICancelationTokenCR)
    {
    NavNodeExtendedData extendedData(parent);
    if (!extendedData.HasGroupingType())
        return false;

    bvector<ECInstanceKey> groupedKeys = extendedData.GetGroupedInstanceKeys();
    auto iter = std::find(groupedKeys.begin(), groupedKeys.end(), childKey);
    return (groupedKeys.end() != iter);
    }

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                01/2018
+===============+===============+===============+===============+===============+======*/
struct ContentRulesSpecificationsInputHandler
{
private:
    NavNodeLocater m_locater;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas                01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<ECInstanceKey> GetECInstanceKeys(NavNodeKeyListCR nodeKeys, ICancelationTokenCR cancelationToken)
        {
        bvector<ECInstanceKey> instanceKeys;
        for (NavNodeKeyCPtr key : nodeKeys)
            {
            if (nullptr != key->AsECInstanceNodeKey())
                {
                instanceKeys.push_back(key->AsECInstanceNodeKey()->GetInstanceKey());
                continue;
                }

            NavNodeCPtr node = m_locater.LocateNode(*key, cancelationToken);
            if (node.IsNull() || NavNodesHelper::IsCustomNode(*node))
                continue;

            NavNodeExtendedData extendedData(*node);
            bvector<ECInstanceKey> groupedInstanceKeys = extendedData.GetGroupedInstanceKeys();
            for (ECInstanceKeyCR key : groupedInstanceKeys)
                instanceKeys.push_back(key);
            }
        return instanceKeys;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas                07/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static RulesDrivenECPresentationManager::NavigationOptions ToNavigationOptions(RulesDrivenECPresentationManager::ContentOptions const& contentOptions)
        {
        return RulesDrivenECPresentationManager::NavigationOptions(contentOptions.GetRulesetId(), TargetTree_Both, false, contentOptions.GetLocale());
        }

public:
    ContentRulesSpecificationsInputHandler(RulesDrivenECPresentationManagerImpl& manager, IConnectionCR connection, RulesDrivenECPresentationManager::ContentOptions const& options)
        : m_locater(manager, connection, ToNavigationOptions(options))
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas                01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentRuleInstanceKeysList HandleSpecifications(ContentRuleInputKeysList& specs, ICancelationTokenCR cancelationToken)
        {
        ContentRuleInstanceKeysList instanceSpecs;
        for (ContentRuleInputKeys& spec : specs)
            {
            bvector<ECInstanceKey> instanceKeys = GetECInstanceKeys(spec.GetMatchingNodeKeys(), cancelationToken);
            instanceSpecs.insert(ContentRuleInstanceKeys(spec.GetRule(), instanceKeys));
            }
        return instanceSpecs;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProviderCPtr RulesDrivenECPresentationManagerImpl::GetContentProvider(IConnectionCR connection, ICancelationTokenCR cancelationToken, ContentProviderKey const& key, INavNodeKeysContainerCR inputKeys, SelectionInfo const* selectionInfo, ContentOptions const& options)
    {
    RefCountedPtr<PerformanceLogger> _l1 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContentProvider]", NativeLogging::LOG_TRACE);
    RefCountedPtr<PerformanceLogger> _l2;

    SpecificationContentProviderPtr provider = m_contentCache->GetProvider(key);
    if (provider.IsValid())
        {
        provider->GetContextR().SetCancelationToken(&cancelationToken);
        return provider;
        }

    // get the ruleset
    PresentationRuleSetPtr ruleset = FindRuleset(GetLocaters(), connection, options.GetRulesetId());
    if (!ruleset.IsValid())
        return nullptr;

    if (cancelationToken.IsCanceled())
        return nullptr;

    // get ruleset-related caches
    IUserSettings const& settings = GetUserSettingsManager().GetSettings(ruleset->GetRuleSetId().c_str());
    ECExpressionsCache& ecexpressionsCache = m_rulesetECExpressionsCache->Get(ruleset->GetRuleSetId().c_str());

    // get connection-related caches
    RelatedPathsCache& relatedPathsCache = m_ecdbCaches->GetRelatedPathsCache(connection);
    PolymorphicallyRelatedClassesCache& polymorphicallyRelatedClassesCache = m_ecdbCaches->GetPolymorphicallyRelatedClassesCache(connection);
    ECSqlStatementCache& statementsCache = m_ecdbCaches->GetStatementsCache(connection);

    // set up the provider context
    ContentProviderContextPtr context = ContentProviderContext::Create(*ruleset, true, options.GetLocale(), key.GetPreferredDisplayType(), inputKeys, *m_nodesCache,
        GetCategorySupplier(), settings, ecexpressionsCache, relatedPathsCache, polymorphicallyRelatedClassesCache, *m_nodesFactory, GetLocalState());
    context->SetQueryContext(m_connections, connection, statementsCache, *m_customFunctions);

    ILocalizationProvider const* localizationProvider = GetLocalizationProvider();
    if (nullptr != localizationProvider)
        context->SetLocalizationContext(*localizationProvider);

    context->SetPropertyFormattingContext(GetECPropertyFormatter());
    context->SetCancelationToken(&cancelationToken);
    if (nullptr != selectionInfo)
        context->SetSelectionInfo(*selectionInfo);

    // get content specifications
    _l2 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContentProvider] Get specifications", NativeLogging::LOG_TRACE);
    RulesPreprocessor preprocessor(m_connections, connection, *ruleset, options.GetLocale(), settings, &context->GetUsedSettingsListener(), ecexpressionsCache);
    RulesPreprocessor::ContentRuleParameters params(inputKeys, key.GetPreferredDisplayType(), selectionInfo, m_nodesCache);
    ContentRuleInputKeysList specs = preprocessor.GetContentSpecifications(params);
    _l2 = nullptr;

    ContentRulesSpecificationsInputHandler inputHandler(*this, connection, ruleset->GetRuleSetId().c_str());
    ContentRuleInstanceKeysList instanceSpecs = inputHandler.HandleSpecifications(specs, cancelationToken);

    _l2 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContentProvider] Create provider", NativeLogging::LOG_TRACE);
    provider = SpecificationContentProvider::Create(*context, instanceSpecs);
    if (!provider.IsValid())
        return nullptr;

    m_contentCache->CacheProvider(key, *provider);
    return provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProviderPtr RulesDrivenECPresentationManagerImpl::GetContentProvider(IConnectionCR connection, ICancelationTokenCR cancelationToken, ContentDescriptorCR descriptor, INavNodeKeysContainerCR inputKeys, SelectionInfo const* selectionInfo, ContentOptions const& options)
    {
    ContentProviderKey key(connection.GetId(), options.GetRulesetId(), descriptor.GetPreferredDisplayType(), options.GetLocale(), inputKeys, selectionInfo);
    SpecificationContentProviderCPtr cachedProvider = GetContentProvider(connection, cancelationToken, key, inputKeys, selectionInfo, options);
    if (cachedProvider.IsNull())
        return nullptr;

    SpecificationContentProviderPtr provider = cachedProvider->Clone();
    provider->SetContentDescriptor(descriptor);
    return provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SelectClassInfo> RulesDrivenECPresentationManagerImpl::_GetContentClasses(IConnectionCR connection, Utf8CP preferredDisplayType, bvector<ECClassCP> const& classes, ContentOptions const& options, ICancelationTokenCR cancelationToken)
    {
    RefCountedPtr<PerformanceLogger> _l = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContentClasses]", NativeLogging::LOG_TRACE);

    if (nullptr == preferredDisplayType || 0 == *preferredDisplayType)
        preferredDisplayType = ContentDisplayType::Undefined;

    // get the ruleset
    PresentationRuleSetPtr ruleset = FindRuleset(GetLocaters(), connection, options.GetRulesetId());
    if (!ruleset.IsValid())
        return bvector<SelectClassInfo>();

    if (cancelationToken.IsCanceled())
        {
        LoggingHelper::LogMessage(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContentClasses] Canceled.");
        return bvector<SelectClassInfo>();
        }

    // get ruleset-related caches
    IUserSettings const& settings = GetUserSettingsManager().GetSettings(ruleset->GetRuleSetId().c_str());
    ECExpressionsCache& ecexpressionsCache = m_rulesetECExpressionsCache->Get(ruleset->GetRuleSetId().c_str());

    // get connection-related caches
    RelatedPathsCache& relatedPathsCache = m_ecdbCaches->GetRelatedPathsCache(connection);
    PolymorphicallyRelatedClassesCache& polymorphicallyRelatedClassesCache = m_ecdbCaches->GetPolymorphicallyRelatedClassesCache(connection);
    ECSqlStatementCache& statementsCache = m_ecdbCaches->GetStatementsCache(connection);

    // locate the classes
    ECSchemaHelper schemaHelper(connection, &relatedPathsCache, &polymorphicallyRelatedClassesCache, &statementsCache, &ecexpressionsCache);
    ContentClassesLocater::Context locaterContext(schemaHelper, m_connections, connection,
        *ruleset, options.GetLocale(), preferredDisplayType, settings, ecexpressionsCache, *m_nodesCache);
    return ContentClassesLocater(locaterContext).Locate(classes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorCPtr RulesDrivenECPresentationManagerImpl::_GetContentDescriptor(IConnectionCR connection, Utf8CP preferredDisplayType,
    KeySetCR inputKeys, SelectionInfo const* selectionInfo, ContentOptions const& options, ICancelationTokenCR cancelationToken)
    {
    RefCountedPtr<PerformanceLogger> _l = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContentDescriptor]", NativeLogging::LOG_TRACE);

    if (nullptr == preferredDisplayType || 0 == *preferredDisplayType)
        preferredDisplayType = ContentDisplayType::Undefined;

    INavNodeKeysContainerCPtr nodeKeys = inputKeys.GetAllNavNodeKeys();
    ContentProviderKey key(connection.GetId(), options.GetRulesetId(), preferredDisplayType, options.GetLocale(), *nodeKeys, selectionInfo);
    ContentProviderCPtr provider = GetContentProvider(connection, cancelationToken, key, *nodeKeys, selectionInfo, options);
    return provider.IsValid() ? provider->GetContentDescriptor() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentCPtr RulesDrivenECPresentationManagerImpl::_GetContent(ContentDescriptorCR descriptor, PageOptionsCR pageOpts, ICancelationTokenCR cancelationToken)
    {
    RefCountedPtr<PerformanceLogger> _l1 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContent]", NativeLogging::LOG_TRACE);
    ContentProviderPtr provider = GetContentProvider(descriptor.GetConnection(), cancelationToken, descriptor, descriptor.GetInputNodeKeys(), descriptor.GetSelectionInfo(),
        descriptor.GetOptions());
    if (provider.IsNull() || nullptr == provider->GetContentDescriptor())
        {
        LoggingHelper::LogMessage(Log::Content, "No content", NativeLogging::LOG_ERROR);
        return nullptr;
        }

    if (cancelationToken.IsCanceled())
        {
        LoggingHelper::LogMessage(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContent] Canceled");
        return nullptr;
        }

    RefCountedPtr<PerformanceLogger> _l2 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContent] Initialize", NativeLogging::LOG_TRACE);
    provider->SetPageOptions(pageOpts);
    provider->Initialize();

    ContentPtr content = Content::Create(*provider->GetContentDescriptor(), *ContentSetDataSource::Create(*provider));

    Utf8PrintfString range("[%" PRIu64 ", %" PRIu64 ")", (uint64_t)pageOpts.GetPageStart(), descriptor.MergeResults() ? 1 : (uint64_t)(pageOpts.GetPageStart() + content->GetContentSet().GetSize()));
    LoggingHelper::LogMessage(Log::Content, Utf8String("[GetContent] Returned ").append(range).append(" of content set").c_str());

    return content;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RulesDrivenECPresentationManagerImpl::_GetContentSetSize(ContentDescriptorCR descriptor, ICancelationTokenCR cancelationToken)
    {
    RefCountedPtr<PerformanceLogger> _l1 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContentSetSize]", NativeLogging::LOG_TRACE);
    ContentProviderPtr provider = GetContentProvider(descriptor.GetConnection(), cancelationToken, descriptor, descriptor.GetInputNodeKeys(), descriptor.GetSelectionInfo(),
        descriptor.GetOptions());
    if (provider.IsNull() || nullptr == provider->GetContentDescriptor())
        {
        LoggingHelper::LogMessage(Log::Content, "No content", NativeLogging::LOG_ERROR);
        return 0;
        }

    if (cancelationToken.IsCanceled())
        {
        LoggingHelper::LogMessage(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContentSetSize] Canceled.");
        return 0;
        }

    RefCountedPtr<PerformanceLogger> _l2 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContentSetSize] Query size", NativeLogging::LOG_TRACE);
    size_t size = provider->GetFullContentSetSize();
    LoggingHelper::LogMessage(Log::Content, Utf8PrintfString("[GetContentSetSize] returned %" PRIu64, (uint64_t)size).c_str());
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RulesDrivenECPresentationManagerImpl::_GetDisplayLabel(IConnectionCR connection, KeySetCR keys, ICancelationTokenCR cancelationToken)
    {
    RefCountedPtr<PerformanceLogger> _l = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetDisplayLabel]", NativeLogging::LOG_TRACE);

    ContentOptions options(DISPLAY_LABEL_RULESET_ID);
    ContentDescriptorCPtr descriptor = GetContentDescriptor(connection, ContentDisplayType::List, keys, nullptr, options, cancelationToken);
    if (descriptor.IsNull())
        return "";

    ContentDescriptorPtr labelDescriptor = ContentDescriptor::Create(*descriptor);
    labelDescriptor->SetContentFlags((int)ContentFlags::NoFields | (int)ContentFlags::ShowLabels | (int)ContentFlags::MergeResults);

    ContentCPtr content = GetContent(*labelDescriptor, PageOptions(), cancelationToken);
    if (content.IsNull())
        return "";

    BeAssert(1 == content->GetContentSet().GetSize());
    ContentSetItemCPtr item = content->GetContentSet().Get(0);
    if (item.IsNull())
        {
        BeAssert(false);
        return "";
        }

    return item->GetDisplayLabel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnECInstanceChangeEventSourceRegistered(ECInstanceChangeEventSource& source)
    {
    source.RegisterEventHandler(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnECInstanceChangeEventSourceUnregister(ECInstanceChangeEventSource& source)
    {
    source.UnregisterEventHandler(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnUpdateRecordsHandlerChanged()
    {
    m_updateHandler->SetRecordsHandler(&GetCompositeUpdateRecordsHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceChangeResult> RulesDrivenECPresentationManagerImpl::_SaveValueChange(IConnectionCR connection, bvector<ChangedECInstanceInfo> const& instances,
    Utf8CP propertyAccessor, ECValueCR value)
    {
    ECInstanceChangesDirector director(GetECInstanceChangeHandlers(), GetLocalizationProvider());
    return director.Handle(connection, instances, propertyAccessor, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnNodeChecked(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken)
    {
    NavNodeLocater locater(*this, connection, options);
    NavNodeCPtr node = locater.LocateNode(nodeKey, cancelationToken);
    JsonNavNodePtr jsonNode;
    if (node.IsValid())
        jsonNode = GetNodesCache().GetNode(node->GetNodeId());
    if (jsonNode.IsValid())
        CustomizationHelper::NotifyCheckedStateChanged(connection, *jsonNode, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnNodeUnchecked(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken)
    {
    NavNodeLocater locater(*this, connection, options);
    NavNodeCPtr node = locater.LocateNode(nodeKey, cancelationToken);
    JsonNavNodePtr jsonNode;
    if (node.IsValid())
        jsonNode = GetNodesCache().GetNode(node->GetNodeId());
    if (jsonNode.IsValid())
        CustomizationHelper::NotifyCheckedStateChanged(connection, *jsonNode, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnNodeExpanded(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken)
    {
    NavNodeLocater locater(*this, connection, options);
    NavNodeCPtr node = locater.LocateNode(nodeKey, cancelationToken);
    JsonNavNodePtr jsonNode;
    if (node.IsValid())
        jsonNode = GetNodesCache().GetNode(node->GetNodeId());
    if (jsonNode.IsValid())
        {
        jsonNode->SetIsExpanded(true);
        GetNodesCache().Update(node->GetNodeId(), *jsonNode);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnNodeCollapsed(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken)
    {
    NavNodeLocater locater(*this, connection, options);
    NavNodeCPtr node = locater.LocateNode(nodeKey, cancelationToken);
    JsonNavNodePtr jsonNode;
    if (node.IsValid())
        jsonNode = GetNodesCache().GetNode(node->GetNodeId());
    if (jsonNode.IsValid())
        {
        jsonNode->SetIsExpanded(false);
        GetNodesCache().Update(node->GetNodeId(), *jsonNode);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnAllNodesCollapsed(IConnectionCR connection, NavigationOptions const& options, ICancelationTokenCR)
    {
    GetNodesCache().ResetExpandedNodes(connection.GetId().c_str(), options.GetRulesetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Lukasonok                11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IRulesPreprocessorPtr RulesDrivenECPresentationManagerImpl::_GetRulesPreprocessor(IConnectionCR connection, Utf8StringCR rulesetId, Utf8StringCR locale, IUsedUserSettingsListener* usedSettingsListener) const
    {
    PresentationRuleSetPtr ruleset = FindRuleset(GetLocaters(), connection, rulesetId.c_str());
    if (!ruleset.IsValid())
        return nullptr;

    IUserSettings const& settings = GetUserSettingsManager().GetSettings(ruleset->GetRuleSetId().c_str());
    ECExpressionsCache& ecexpressionsCache = m_rulesetECExpressionsCache->Get(ruleset->GetRuleSetId().c_str());

    return new RulesPreprocessor(m_connections, connection, *ruleset, locale, settings, usedSettingsListener, ecexpressionsCache);
    }