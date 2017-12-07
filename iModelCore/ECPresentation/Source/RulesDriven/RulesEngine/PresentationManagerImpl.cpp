/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/PresentationManagerImpl.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
RulesDrivenECPresentationManager::Impl::Impl(IRulesDrivenECPresentationManagerDependenciesFactory const& dependenciesFactory, IConnectionManagerCR connections, Paths const& paths)
    : m_localState(nullptr), m_selectionManager(nullptr), m_localizationProvider(nullptr), m_ecPropertyFormatter(nullptr), m_categorySupplier(nullptr)
    {
    m_locaters = dependenciesFactory._CreateRulesetLocaterManager(connections);
    m_userSettings = dependenciesFactory._CreateUserSettingsManager(paths.GetTemporaryDirectory());
    m_userSettings->SetLocalizationProvider(&GetLocalizationProvider());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::Impl::~Impl()
    {
    DELETE_AND_CLEAR(m_userSettings);
    DELETE_AND_CLEAR(m_locaters);
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
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::Impl::SetSelectionManager(ISelectionManager* manager)
    {
    ISelectionManager* before = m_selectionManager;
    m_selectionManager = manager;
    _OnSelectionManagerChanged(before, m_selectionManager);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::Impl::SetLocalizationProvider(ILocalizationProvider const* provider)
    {
    m_localizationProvider = provider;
    m_userSettings->SetLocalizationProvider(&GetLocalizationProvider());
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
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ILocalizationProvider const& RulesDrivenECPresentationManager::Impl::GetLocalizationProvider() const
    {
    if (nullptr != m_localizationProvider)
        return *m_localizationProvider;

    static SQLangLocalizationProvider s_localizationProvider;
    return s_localizationProvider;
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
static bool IsRulesetSupported(ECDbCR connection, PresentationRuleSetCR ruleset)
    {
    ECSchemaHelper helper(connection, nullptr, nullptr);
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
    if (!IsRulesetSupported(connection.GetDb(), *ruleset))
        {
        LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("Ruleset '%s' not supported by connection", rulesetId).c_str(), LOG_INFO);
        return nullptr;
        }
    return ruleset;
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
* @bsiclass                                     Grigas.Petraitis                01/2017
+===============+===============+===============+===============+===============+======*/
template<typename T, T*(*CreateFunc)()>
struct ConnectionBasedCache : IConnectionsListener
{
private:
    IConnectionManagerCR m_connections;
    bmap<Utf8String, T*> m_caches;

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
public:
    ConnectionBasedCache(IConnectionManagerCR connections) 
        : m_connections(connections)
        {
        m_connections.AddListener(*this);
        }
    ~ConnectionBasedCache()
        {
        m_connections.DropListener(*this);
        for (auto iter : m_caches)
            delete iter.second;
        }
    T& GetCache(ECDbCR db)
        {
        IConnectionPtr connection = m_connections.GetConnection(db);
        auto iter = m_caches.find(connection->GetId());
        if (m_caches.end() == iter)
            iter = m_caches.Insert(connection->GetId(), CreateFunc()).first;
        return *iter->second;
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2017
+===============+===============+===============+===============+===============+======*/
ECSqlStatementCache* CreateECSqlStatementCache() {return new ECSqlStatementCache(50);}
struct RulesDrivenECPresentationManagerImpl::ECDbStatementsCache : ConnectionBasedCache<ECSqlStatementCache, CreateECSqlStatementCache>, IECSqlStatementCacheProvider
    {
    ECSqlStatementCache& _GetECSqlStatementCache(ECDbCR db) override {return ConnectionBasedCache::GetCache(db);}
    ECDbStatementsCache(IConnectionManagerCR connections) : ConnectionBasedCache(connections) {}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2017
+===============+===============+===============+===============+===============+======*/
RelatedPathsCache* CreateRelatedPathsCache() {return new RelatedPathsCache();}
struct RulesDrivenECPresentationManagerImpl::ECDbRelatedPathsCache : ConnectionBasedCache<RelatedPathsCache, CreateRelatedPathsCache>
    {
    ECDbRelatedPathsCache(IConnectionManagerCR connections) : ConnectionBasedCache(connections) {}
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
        NavNodesProviderPtr provider;
        if (nullptr == parent)
            {
            RulesPreprocessor::RootNodeRuleParameters params(m_manager.m_connections, context.GetConnection(), context.GetRuleset(), TargetTree_MainTree,
                context.GetUserSettings(), &context.GetUsedSettingsListener(), context.GetECExpressionsCache());
            RootNodeRuleSpecificationsList specs = RulesPreprocessor::GetRootNodeSpecifications(params);
            if (!specs.empty())
                provider = MultiSpecificationNodesProvider::Create(context, specs);
            }
        else
            {
            RulesPreprocessor::ChildNodeRuleParameters params(m_manager.m_connections, context.GetConnection(), *parent, context.GetRuleset(), TargetTree_MainTree,
                context.GetUserSettings(), &context.GetUsedSettingsListener(), context.GetECExpressionsCache());
            ChildNodeRuleSpecificationsList specs = RulesPreprocessor::GetChildNodeSpecifications(params);
            if (!specs.empty())
                provider = MultiSpecificationNodesProvider::Create(context, specs, *parent);
            }
        if (provider.IsNull())
            provider = EmptyNavNodesProvider::Create(context);
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
    NavNodesProviderPtr _CreateForHierarchyLevel(NavNodesProviderContextR context, JsonNavNodeCP parent) const override
        {
        HierarchyLevelInfo info(context.GetConnection().GetId(), context.GetRuleset().GetRuleSetId(), nullptr != parent ? parent->GetNodeId() : 0);
        NavNodesProviderPtr provider = m_manager.m_nodesCache->GetDataSource(info);
        if (provider.IsNull())
            provider = CreateProvider(context, parent);
        else
            AdoptProvider(*provider, context);

        if (provider.IsValid())
            {
            BeAssert(provider->GetContext().IsUpdatesDisabled() == context.IsUpdatesDisabled());
            if (!context.GetCancelationToken().IsCanceled())
                m_manager.m_nodesCache->CacheHierarchyLevel(info, *provider);
            }
        return provider;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr _CreateForVirtualParent(NavNodesProviderContextR context, JsonNavNodeCP parent) const override
        {
        uint64_t parentId = (nullptr != parent) ? parent->GetNodeId() : 0;
        uint64_t const* parentIdP = (nullptr != parent) ? &parentId : nullptr;
        DataSourceInfo info(context.GetConnection().GetId(), context.GetRuleset().GetRuleSetId(), parentIdP, parentIdP);
        NavNodesProviderPtr provider = m_manager.m_nodesCache->GetDataSource(info);
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
    NavNodesProviderContextPtr _Create(IConnectionCR connection, Utf8CP rulesetId, uint64_t const* parentNodeId, ICancelationTokenCP cancelationToken, bool disableUpdates) const override
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
        RelatedPathsCache& relatedPathsCache = m_manager.m_relatedPathsCache->GetCache(connection.GetDb());
        ECSqlStatementCache& statementsCache = m_manager.m_statementCache->GetCache(connection.GetDb());

        // notify listener with ECClasses used in this ruleset
        ECSchemaHelper schemaHelper(connection.GetDb(), &relatedPathsCache, &statementsCache);
        UsedClassesHelper::NotifyListenerWithRulesetClasses(*m_manager.m_usedClassesListener, ecexpressionsCache, connection, *ruleset);

        // set up the nodes provider context
        _l2 = LoggingHelper::CreatePerformanceLogger(Log::Navigation, "[NodesProviderContextFactory::Create] Create context", NativeLogging::LOG_TRACE);
        NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*ruleset, true, TargetTree_MainTree, parentNodeId,
            settings, ecexpressionsCache, relatedPathsCache, *m_manager.m_nodesFactory, m_manager.GetNodesCache(),
            *m_manager.m_nodesProviderFactory, m_manager.GetLocalState());
        context->SetQueryContext(m_manager.m_connections, connection, statementsCache, *m_manager.m_customFunctions, m_manager.m_usedClassesListener);
        context->SetLocalizationContext(m_manager.GetLocalizationProvider());
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
RulesDrivenECPresentationManagerImpl::RulesDrivenECPresentationManagerImpl(IRulesDrivenECPresentationManagerDependenciesFactory const& dependenciesFactory,
    IConnectionManagerCR connections, Paths const& paths, bool disableDiskCache)
    : RulesDrivenECPresentationManager::Impl(dependenciesFactory, connections, paths), m_connections(connections)
    {
    m_customFunctions = new CustomFunctionsInjector(connections);
    m_rulesetECExpressionsCache = new RulesetECExpressionsCache();
    m_statementCache = new ECDbStatementsCache(connections);
    m_relatedPathsCache = new ECDbRelatedPathsCache(connections);
    m_nodesProviderContextFactory = new NodesProviderContextFactory(*this);
    m_nodesProviderFactory = new NodesProviderFactory(*this);
    m_usedClassesListener = new UsedClassesListener(*this);
    m_nodesFactory = new JsonNavNodesFactory();
    m_nodesCache = new NodesCache(paths.GetTemporaryDirectory(), *m_nodesFactory, *m_nodesProviderContextFactory,
        connections, *m_statementCache, disableDiskCache ? NodesCacheType::Memory : NodesCacheType::Disk);
    m_contentCache = new ContentCache();
    m_updateHandler = new UpdateHandler(m_nodesCache, m_contentCache, connections, *m_nodesProviderContextFactory,
        *m_nodesProviderFactory, *m_rulesetECExpressionsCache);

    GetLocaters().SetRulesetCallbacksHandler(this);
    GetUserSettingsManager().SetChangesListener(this);
    connections.AddListener(*this);

    BeFileName supplementalRulesetsDirectory = paths.GetAssetsDirectory();
    supplementalRulesetsDirectory.append(L"UI\\");
    supplementalRulesetsDirectory.append(L"PresentationRules\\");
    GetLocaters().RegisterLocater(*SupplementalRuleSetLocater::Create(*DirectoryRuleSetLocater::Create(supplementalRulesetsDirectory.GetNameUtf8().c_str())));
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
    DELETE_AND_CLEAR(m_relatedPathsCache);
    DELETE_AND_CLEAR(m_statementCache);
    DELETE_AND_CLEAR(m_rulesetECExpressionsCache);
    DELETE_AND_CLEAR(m_customFunctions);

    m_connections.DropListener(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnRulesetCreated(PresentationRuleSetCR ruleset)
    {
    IUserSettings& settings = GetUserSettings(ruleset.GetRuleSetId().c_str());
    settings.InitFrom(ruleset.GetUserSettings());
    m_nodesCache->OnRulesetCreated(ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnRulesetDispose(PresentationRuleSetCR ruleset)
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
        RuleSetLocaterPtr locater = m_embeddedRuleSetLocaters[evt.GetConnection().GetId()] = SupplementalRuleSetLocater::Create(*EmbeddedRuleSetLocater::Create(evt.GetConnection()));
        GetLocaters().RegisterLocater(*locater);
        }
    else if (evt.GetEventType() == ConnectionEventType::Closed)
        {
        auto iter = m_embeddedRuleSetLocaters.find(evt.GetConnection().GetId());
        if (m_embeddedRuleSetLocaters.end() == iter)
            {
            BeAssert(false);
            return;
            }
        GetLocaters().UnregisterLocater(*iter->second);
        m_embeddedRuleSetLocaters.erase(iter);
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
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnSelectionChanged(SelectionChangedEventCR evt)
    {
    if (!evt.IsValid())
        {
        BeAssert(false);
        return;
        }

    m_contentCache->ClearCache(evt.GetConnection());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnECInstancesChanged(ECDbCR db, bvector<ECInstanceChangeEventSource::ChangedECInstance> changes)
    {
    IConnectionPtr connection = m_connections.GetConnection(db);
    m_updateHandler->NotifyECInstancesChanged(*connection, changes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodesDataSourcePtr RulesDrivenECPresentationManagerImpl::GetCachedDataSource(IConnectionCR connection, ICancelationTokenCR cancelationToken, NavigationOptions const& options)
    {
    // create the nodes provider context
    NavNodesProviderContextPtr context = m_nodesProviderContextFactory->Create(connection, options.GetRulesetId(), nullptr, &cancelationToken, options.GetDisableUpdates());
    if (context.IsNull())
        return nullptr;

    // create the nodes provider
    NavNodesProviderPtr provider = m_nodesProviderFactory->CreateForHierarchyLevel(*context, nullptr);
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
    NavNodesProviderContextPtr context = m_nodesProviderContextFactory->Create(connection, options.GetRulesetId(), &parentNodeId, &cancelationToken, options.GetDisableUpdates());
    if (context.IsNull())
        return nullptr;

    // create the nodes provider
    JsonNavNodeCPtr jsonParent = GetNodesCache().GetNode(parentNodeId);
    NavNodesProviderPtr provider = m_nodesProviderFactory->CreateForHierarchyLevel(*context, jsonParent.get());
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

    return GetNodesCache().GetNode(node.GetParentNodeId(), NodeVisibility::Physical);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr RulesDrivenECPresentationManagerImpl::_GetNode(IConnectionCR, uint64_t nodeId, ICancelationTokenCR)
    {
    return GetNodesCache().GetNode(nodeId, NodeVisibility::Physical);
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
    if (!GetNodesCache().IsDataSourceCached(connection.GetId(), options.GetRulesetId()))
        GetRootNodes(connection, PageOptions(), options, cancelationToken);
    NavNodesProviderPtr provider = GetNodesCache().GetUndeterminedNodesProvider(connection, options.GetRulesetId(), options.GetDisableUpdates());
    size_t nodesCount = provider->GetNodesCount();
    for (size_t i = 0; i < nodesCount; i++)
        {
        JsonNavNodePtr node;
        provider->GetNode(node, i);
        TraverseHierarchy(*this, connection, *node, options, cancelationToken);
        }
    return GetNodesCache().GetFilteredNodes(connection, options.GetRulesetId(), filterText);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool RulesDrivenECPresentationManagerImpl::_HasChild(IConnectionCR, NavNodeCR parent, NavNodeKeyCR childKey, NavigationOptions const&, ICancelationTokenCR)
    {
    ECInstanceNodeKey const* key = childKey.AsECInstanceNodeKey();
    if (nullptr == key)
        return false;

    ECInstanceKey instanceKey(key->GetECClassId(), key->GetInstanceId());
    NavNodeExtendedData extendedData(parent);
    if (!extendedData.HasGroupingType())
        return false;

    bvector<ECInstanceKey> groupedKeys = extendedData.GetGroupedInstanceKeys();
    auto iter = std::find(groupedKeys.begin(), groupedKeys.end(), instanceKey);
    return (groupedKeys.end() != iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProviderCPtr RulesDrivenECPresentationManagerImpl::GetContentProvider(IConnectionCR connection, ICancelationTokenCR cancelationToken, ContentProviderKey const& key, SelectionInfo const& selectionInfo, ContentOptions const& options)
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
    RelatedPathsCache& relatedPathsCache = m_relatedPathsCache->GetCache(connection.GetDb());
    ECSqlStatementCache& statementCache = m_statementCache->GetCache(connection.GetDb());

    // set up the provider context
    ContentProviderContextPtr context = ContentProviderContext::Create(*ruleset, true, key.GetPreferredDisplayType(), *m_nodesCache,
        GetCategorySupplier(), settings, ecexpressionsCache, relatedPathsCache, *m_nodesFactory, GetLocalState());
    context->SetQueryContext(m_connections, connection, statementCache, *m_customFunctions);
    context->SetLocalizationContext(GetLocalizationProvider());
    context->SetSelectionContext(selectionInfo);
    context->SetPropertyFormattingContext(GetECPropertyFormatter());
    context->SetCancelationToken(&cancelationToken);

    // get content specifications
    _l2 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContentProvider] Get specifications", NativeLogging::LOG_TRACE);
    RulesPreprocessor::ContentRuleParameters params(m_connections, connection, selectionInfo.GetSelectedNodeKeys(), key.GetPreferredDisplayType(), selectionInfo.GetSelectionProviderName(),
        selectionInfo.IsSubSelection(), *ruleset, settings, &context->GetUsedSettingsListener(), ecexpressionsCache, *m_nodesCache);
    ContentRuleSpecificationsList specs = RulesPreprocessor::GetContentSpecifications(params);
    _l2 = nullptr;

    _l2 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContentProvider] Create provider", NativeLogging::LOG_TRACE);
    provider = SpecificationContentProvider::Create(*context, specs);
    if (!provider.IsValid())
        return nullptr;

    m_contentCache->CacheProvider(key, *provider);
    return provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProviderPtr RulesDrivenECPresentationManagerImpl::GetContentProvider(IConnectionCR connection, ICancelationTokenCR cancelationToken, ContentDescriptorCR descriptor, SelectionInfo const& selectionInfo, ContentOptions const& options)
    {
    ContentProviderKey key(connection.GetId(), options.GetRulesetId(), descriptor.GetPreferredDisplayType(), selectionInfo);
    SpecificationContentProviderCPtr cachedProvider = GetContentProvider(connection, cancelationToken, key, selectionInfo, options);
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
    RelatedPathsCache& relatedPathsCache = m_relatedPathsCache->GetCache(connection.GetDb());
    ECSqlStatementCache& statementCache = m_statementCache->GetCache(connection.GetDb());

    // locate the classes
    ECSchemaHelper schemaHelper(connection.GetDb(), &relatedPathsCache, &statementCache);
    ContentClassesLocater::Context locaterContext(schemaHelper, m_connections, connection, *ruleset, preferredDisplayType, settings, ecexpressionsCache, *m_nodesCache);
    return ContentClassesLocater(locaterContext).Locate(classes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorCPtr RulesDrivenECPresentationManagerImpl::_GetContentDescriptor(IConnectionCR connection, Utf8CP preferredDisplayType, SelectionInfo const& selectionInfo, ContentOptions const& options, ICancelationTokenCR cancelationToken)
    {
    RefCountedPtr<PerformanceLogger> _l = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContentDescriptor]", NativeLogging::LOG_TRACE);

    if (nullptr == preferredDisplayType || 0 == *preferredDisplayType)
        preferredDisplayType = ContentDisplayType::Undefined;

    ContentProviderKey key(connection.GetId(), options.GetRulesetId(), preferredDisplayType, selectionInfo);
    ContentProviderCPtr provider = GetContentProvider(connection, cancelationToken, key, selectionInfo, options);
    return provider.IsValid() ? provider->GetContentDescriptor() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentCPtr RulesDrivenECPresentationManagerImpl::_GetContent(IConnectionCR connection, ContentDescriptorCR descriptor, SelectionInfo const& selectionInfo, PageOptionsCR pageOpts, ContentOptions const& options, ICancelationTokenCR cancelationToken)
    {
    RefCountedPtr<PerformanceLogger> _l1 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContent]", NativeLogging::LOG_TRACE);
    ContentProviderPtr provider = GetContentProvider(connection, cancelationToken, descriptor, selectionInfo, options);
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
size_t RulesDrivenECPresentationManagerImpl::_GetContentSetSize(IConnectionCR connection, ContentDescriptorCR descriptor, SelectionInfo const& selectionInfo, ContentOptions const& options, ICancelationTokenCR cancelationToken)
    {
    RefCountedPtr<PerformanceLogger> _l1 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManagerImpl::GetContentSetSize]", NativeLogging::LOG_TRACE);
    ContentProviderPtr provider = GetContentProvider(connection, cancelationToken, descriptor, selectionInfo, options);
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
    m_updateHandler->SetRecordsHandler(GetUpdateRecordsHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnSelectionManagerChanged(ISelectionManager* before, ISelectionManager* after)
    {
    if (nullptr != before)
        before->RemoveListener(*this);

    if (nullptr != after)
        after->AddListener(*this);

    m_updateHandler->SetSelectionManager(after);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceChangeResult> RulesDrivenECPresentationManagerImpl::_SaveValueChange(IConnectionCR connection, bvector<ChangedECInstanceInfo> const& instances,
    Utf8CP propertyAccessor, ECValueCR value)
    {
    ECInstanceChangesDirector director(GetECInstanceChangeHandlers());
    return director.Handle(connection, instances, propertyAccessor, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnNodeChecked(IConnectionCR connection, uint64_t nodeId, ICancelationTokenCR)
    {
    JsonNavNodePtr node = GetNodesCache().GetNode(nodeId);
    if (node.IsValid())
        CustomizationHelper::NotifyCheckedStateChanged(connection.GetDb(), *node, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnNodeUnchecked(IConnectionCR connection, uint64_t nodeId, ICancelationTokenCR)
    {
    JsonNavNodePtr node = GetNodesCache().GetNode(nodeId);
    if (node.IsValid())
        CustomizationHelper::NotifyCheckedStateChanged(connection.GetDb(), *node, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnNodeExpanded(IConnectionCR connection, uint64_t nodeId, ICancelationTokenCR)
    {
    JsonNavNodePtr node = GetNodesCache().GetNode(nodeId);
    if (node.IsValid())
        {
        node->SetIsExpanded(true);
        GetNodesCache().Update(nodeId, *node);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnNodeCollapsed(IConnectionCR connection, uint64_t nodeId, ICancelationTokenCR)
    {
    JsonNavNodePtr node = GetNodesCache().GetNode(nodeId);
    if (node.IsValid())
        {
        node->SetIsExpanded(false);
        GetNodesCache().Update(nodeId, *node);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerImpl::_OnAllNodesCollapsed(IConnectionCR connection, NavigationOptions const& options, ICancelationTokenCR)
    {
    GetNodesCache().ResetExpandedNodes(connection.GetId().c_str(), options.GetRulesetId());
    }
