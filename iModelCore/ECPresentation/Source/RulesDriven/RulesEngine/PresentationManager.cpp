/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/PresentationManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include "RulesPreprocessor.h"
#include "NavNodesCache.h"
#include "QueryExecutor.h"
#include "NavNodeProviders.h"
#include "ContentProviders.h"
#include "LocalizationHelper.h"
#include "NavNodesDataSource.h"
#include "UpdateHandler.h"
#include "LoggingHelper.h"
#include "CustomizationHelper.h"
#include "ContentCache.h"
#include "QueryBuilder.h"
#include "ECExpressionContextsProvider.h"
#include "ECInstanceChangesDirector.h"
#include "ContentClassesLocater.h"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2016
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManager::UsedClassesListener : IECDbUsedClassesListener
    {
    RulesDrivenECPresentationManager& m_manager;
    UsedClassesListener(RulesDrivenECPresentationManager& manager) : m_manager(manager) {}
    void _OnClassUsed(ECDbCR db, ECClassCR ecClass, bool polymorphically) override
        {
        for (ECInstanceChangeEventSourcePtr& source : m_manager.m_ecInstanceChangeEventSources)
            source->NotifyClassUsed(db, ecClass, polymorphically);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsRulesetSupported(ECDbCR connection, PresentationRuleSetCR ruleset)
    {
    RelatedPathsCache relatedPathsCache;
    ECSchemaHelper helper(connection, relatedPathsCache, nullptr);
    return helper.AreSchemasSupported(ruleset.GetSupportedSchemas());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static PresentationRuleSetPtr FindRuleset(RuleSetLocaterManager const& locaters, ECDbCR connection, Utf8CP rulesetId)
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

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManager::RulesetECExpressionsCache : IECExpressionsCacheProvider
{
private:
    bmap<Utf8CP, ECExpressionsCache*> m_caches;
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
struct SimpleECDbBaseCache : ECDbBasedCache
{
private:
    bmap<ECDb const*, T*> m_caches;

protected:
    void _ClearECDbCache(ECDbCR db) override
        {
        auto iter = m_caches.find(&db);
        if (m_caches.end() != iter)
            {
            delete iter->second;
            m_caches.erase(iter);
            }
        }
public:
    SimpleECDbBaseCache() : ECDbBasedCache(true) {}
    T& GetCache(ECDbCR db)
        {
        auto iter = m_caches.find(&db);
        if (m_caches.end() == iter)
            {
            OnConnection(db);
            iter = m_caches.Insert(&db, CreateFunc()).first;
            }
        return *iter->second;
        }
    ~SimpleECDbBaseCache()
        {
        for (auto iter : m_caches)
            delete iter.second;
        }
};

ECSqlStatementCache* CreateECSqlStatementCache() {return new ECSqlStatementCache(50);}
struct RulesDrivenECPresentationManager::ECDbStatementsCache : SimpleECDbBaseCache<ECSqlStatementCache, CreateECSqlStatementCache> {};

RelatedPathsCache* CreateRelatedPathsCache() {return new RelatedPathsCache();}
struct RulesDrivenECPresentationManager::ECDbRelatedPathsCache : SimpleECDbBaseCache<RelatedPathsCache, CreateRelatedPathsCache> {};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManager::NodesProviderFactory : INodesProviderFactory
{
private:
    RulesDrivenECPresentationManager& m_manager;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr CreateProvider(NavNodesProviderContextR context, NavNodeCP parent) const
        {
        NavNodesProviderPtr provider;
        if (nullptr == parent)
            {
            RulesPreprocessor::RootNodeRuleParameters params(context.GetDb(), context.GetRuleset(), TargetTree_MainTree,
                context.GetUserSettings(), &context.GetUsedSettingsListener(), context.GetECExpressionsCache());
            RootNodeRuleSpecificationsList specs = RulesPreprocessor::GetRootNodeSpecifications(params);
            provider = MultiSpecificationNodesProvider::Create(context, specs);
            }
        else
            {
            RulesPreprocessor::ChildNodeRuleParameters params(context.GetDb(), *parent, context.GetRuleset(), TargetTree_MainTree, 
                context.GetUserSettings(), &context.GetUsedSettingsListener(), context.GetECExpressionsCache());
            ChildNodeRuleSpecificationsList specs = RulesPreprocessor::GetChildNodeSpecifications(params);
            provider = MultiSpecificationNodesProvider::Create(context, specs, *parent);
            }
        return provider;
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr _CreateForHierarchyLevel(NavNodesProviderContextR context, NavNodeCP parent) const override
        {
        HierarchyLevelInfo info(context.GetDb().GetDbGuid(), context.GetRuleset().GetRuleSetId(), nullptr != parent ? parent->GetNodeId() : 0);
        NavNodesProviderPtr provider = m_manager.m_nodesCache->GetDataSource(info);
        if (provider.IsNull())
            provider = CreateProvider(context, parent);
        if (provider.IsNull())
            return nullptr;
        BeAssert(provider->GetContext().IsUpdatesDisabled() == context.IsUpdatesDisabled());
        
        m_manager.m_nodesCache->CacheHierarchyLevel(info, *provider);
        return provider;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr _CreateForVirtualParent(NavNodesProviderContextR context, NavNodeCP parent) const override
        {
        uint64_t parentId = (nullptr != parent) ? parent->GetNodeId() : 0;
        uint64_t const* parentIdP = (nullptr != parent) ? &parentId : nullptr;
        DataSourceInfo info(context.GetDb().GetDbGuid(), context.GetRuleset().GetRuleSetId(), parentIdP, parentIdP);
        NavNodesProviderPtr provider = m_manager.m_nodesCache->GetDataSource(info);
        if (provider.IsNull())
            provider = CreateProvider(context, parent);
        return provider;
        }

public:
    NodesProviderFactory(RulesDrivenECPresentationManager& mgr) : m_manager(mgr) {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManager::NodesProviderContextFactory : INodesProviderContextFactory
{
private:
    RulesDrivenECPresentationManager& m_manager;

protected:
    NavNodesProviderContextPtr _Create(ECDbCR connection, Utf8CP rulesetId, uint64_t const* parentNodeId, bool disableUpdates) const override
        {
        // get the ruleset
        RefCountedPtr<PerformanceLogger> _l2 = LoggingHelper::CreatePerformanceLogger(Log::Navigation, "[NodesProviderContextFactory::Create] Get ruleset", NativeLogging::LOG_TRACE);
        PresentationRuleSetPtr ruleset = FindRuleset(m_manager.GetLocaters(), connection, rulesetId);
        if (!ruleset.IsValid())
            return nullptr;
        _l2 = nullptr;

        // get various caches
        IUserSettings const& settings = m_manager.GetUserSettings(rulesetId);
        ECExpressionsCache& ecexpressionsCache = m_manager.m_rulesetECExpressionsCache->Get(rulesetId);
        RelatedPathsCache& relatedPathsCache = m_manager.m_relatedPathsCache->GetCache(connection);
        ECSqlStatementCache& statementsCache = m_manager.m_statementCache->GetCache(connection);

        // notify listener with ECClasses used in this ruleset
        ECSchemaHelper schemaHelper(connection, relatedPathsCache, &statementsCache);
        UsedClassesHelper::NotifyListenerWithRulesetClasses(*m_manager.m_usedClassesListener, schemaHelper, ecexpressionsCache, *ruleset);

        // set up the nodes provider context
        _l2 = LoggingHelper::CreatePerformanceLogger(Log::Navigation, "[NodesProviderContextFactory::Create] Create context", NativeLogging::LOG_TRACE);
        NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*ruleset, true, TargetTree_MainTree, parentNodeId, 
            settings, ecexpressionsCache, relatedPathsCache, *m_manager.m_nodesFactory, m_manager.GetNodesCacheR(), 
            *m_manager.m_nodesProviderFactory, m_manager.m_localState);
        context->SetQueryContext(connection, statementsCache, *m_manager.m_customFunctions, m_manager.m_usedClassesListener);
        context->SetLocalizationContext(m_manager.GetLocalizationProvider());
        context->SetIsUpdatesDisabled(disableUpdates);
        _l2 = nullptr;

        return context;
        }
public:
    NodesProviderContextFactory(RulesDrivenECPresentationManager& mgr) : m_manager(mgr) {}
};

const Utf8CP RulesDrivenECPresentationManager::ContentOptions::OPTION_NAME_RulesetId = "RulesetId";
const Utf8CP RulesDrivenECPresentationManager::NavigationOptions::OPTION_NAME_RulesetId = "RulesetId";
const Utf8CP RulesDrivenECPresentationManager::NavigationOptions::OPTION_NAME_RuleTargetTree = "RuleTargetTree";
const Utf8CP RulesDrivenECPresentationManager::NavigationOptions::OPTION_NAME_DisableUpdates = "DisableUpdates";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::RulesDrivenECPresentationManager(Paths const& paths)
    : m_localState(nullptr), m_userSettings(paths.GetTemporaryDirectory(), this), m_selectionManager(nullptr), 
    m_categorySupplier(nullptr), m_ecPropertyFormatter(nullptr), m_localizationProvider(nullptr)
    {
    GetLocaters().SetRulesetCallbacksHandler(this);
    m_nodesFactory = new JsonNavNodesFactory();
    m_customFunctions = new CustomFunctionsInjector();
    m_rulesetECExpressionsCache = new RulesetECExpressionsCache();
    m_nodesProviderContextFactory = new NodesProviderContextFactory(*this);
    m_nodesProviderFactory = new NodesProviderFactory(*this);
    m_nodesCache = new NodesCache(paths.GetTemporaryDirectory(), *m_nodesFactory, *m_nodesProviderContextFactory, GetConnections(), NodesCacheType::Disk);
    m_contentCache = new ContentCache();
    m_statementCache = new ECDbStatementsCache();
    m_relatedPathsCache = new ECDbRelatedPathsCache();
    m_userSettings.SetLocalizationProvider(&GetLocalizationProvider());
    m_updateHandler = new UpdateHandler(m_nodesCache, m_contentCache, GetConnections(), *m_nodesProviderContextFactory, *m_nodesProviderFactory, *m_rulesetECExpressionsCache);
    m_usedClassesListener = new UsedClassesListener(*this);

    BeFileName supplementalRulesetsDirectory = paths.GetAssetsDirectory();
    supplementalRulesetsDirectory.append(L"UI\\");
    supplementalRulesetsDirectory.append(L"PresentationRules\\");
    GetLocaters().RegisterLocater(*SupplementalRuleSetLocater::Create(supplementalRulesetsDirectory));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::~RulesDrivenECPresentationManager()
    {
    GetLocaters().SetRulesetCallbacksHandler(nullptr);
    DELETE_AND_CLEAR(m_updateHandler);
    DELETE_AND_CLEAR(m_statementCache);
    DELETE_AND_CLEAR(m_relatedPathsCache);
    DELETE_AND_CLEAR(m_nodesFactory);
    DELETE_AND_CLEAR(m_nodesCache);
    DELETE_AND_CLEAR(m_nodesProviderContextFactory);
    DELETE_AND_CLEAR(m_nodesProviderFactory);
    DELETE_AND_CLEAR(m_contentCache);
    DELETE_AND_CLEAR(m_rulesetECExpressionsCache);
    DELETE_AND_CLEAR(m_usedClassesListener);
    DELETE_AND_CLEAR(m_customFunctions);

    if (nullptr != m_selectionManager)
        {
        m_selectionManager->RemoveListener(*this);
        m_selectionManager = nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::OnConnection(ECDbCR connection) const
    {
    ECDbClosedNotifier::Register(*m_updateHandler, connection, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::_OnRulesetCreated(PresentationRuleSetCR ruleset)
    {
    UserSettings& settings = m_userSettings.GetSettings(ruleset.GetRuleSetId().c_str());
    settings.InitFrom(ruleset.GetUserSettings());
    m_nodesCache->OnRulesetCreated(ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::_OnRulesetDispose(PresentationRuleSetCR ruleset)
    {
    m_rulesetECExpressionsCache->Clear(ruleset.GetRuleSetId().c_str());
    m_updateHandler->NotifyRulesetDisposed(ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::_OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const
    {
    CustomFunctionsManager::GetManager()._OnSettingChanged(rulesetId, settingId);
    m_updateHandler->NotifySettingChanged(rulesetId, settingId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::NotifyCategoriesChanged()
    {
    m_updateHandler->NotifyCategoriesChanged();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::_OnSelectionChanged(SelectionChangedEventCR evt)
    {
    if (!evt.IsValid())
        {
        BeAssert(false);
        return;
        }
    m_contentCache->ClearCache(evt.GetConnection());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodesDataSourcePtr RulesDrivenECPresentationManager::GetCachedDataSource(ECDbR connection, JsonValueCR jsonOptions)
    {
    OnConnection(connection);
    NavigationOptions options(jsonOptions);

    // create the nodes provider context
    NavNodesProviderContextPtr context = m_nodesProviderContextFactory->Create(connection, options.GetRulesetId(), nullptr, options.GetDisableUpdates());
    if (context.IsNull())
        return nullptr;

    // create the nodes provider
    NavNodesProviderPtr provider = m_nodesProviderFactory->CreateForHierarchyLevel(*context, nullptr);
    return NavNodesDataSource::Create(*provider);
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DataContainer<NavNodeCPtr> RulesDrivenECPresentationManager::_GetRootNodes(ECDbR connection, PageOptionsCR pageOpts, JsonValueCR jsonOptions)
    {
    INavNodesDataSourcePtr source = GetCachedDataSource(connection, jsonOptions);
    if (source.IsNull())
        source = EmptyNavNodesDataSource::Create();
    source = PagingDataSource::Create(*source, pageOpts.GetPageStart(), pageOpts.GetPageSize());

    LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[GetRootNodes] Returned [%" PRIu64 ", %" PRIu64 ") nodes", 
        (uint64_t)pageOpts.GetPageStart(), (uint64_t)(pageOpts.GetPageStart() + source->GetSize())).c_str());

    return DataContainer<NavNodeCPtr>(*source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RulesDrivenECPresentationManager::_GetRootNodesCount(ECDbR connection, JsonValueCR jsonOptions)
    {
    INavNodesDataSourcePtr source = GetCachedDataSource(connection, jsonOptions);
    size_t size = source.IsValid() ? source->GetSize() : 0;

    LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[GetRootNodesCount] Returned %" PRIu64, (uint64_t)size).c_str());
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodesDataSourcePtr RulesDrivenECPresentationManager::GetCachedDataSource(ECDbR connection, NavNodeCR parent, JsonValueCR jsonOptions)
    {
    OnConnection(connection);
    NavigationOptions options(jsonOptions);

    // create the nodes provider context
    uint64_t parentNodeId = parent.GetNodeId();
    NavNodesProviderContextPtr context = m_nodesProviderContextFactory->Create(connection, options.GetRulesetId(), &parentNodeId, options.GetDisableUpdates());
    if (context.IsNull())
        return nullptr;

    // create the nodes provider
    NavNodesProviderPtr provider = m_nodesProviderFactory->CreateForHierarchyLevel(*context, &parent);
    return NavNodesDataSource::Create(*provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DataContainer<NavNodeCPtr> RulesDrivenECPresentationManager::_GetChildren(ECDbR connection, NavNodeCR parent, PageOptionsCR pageOpts, JsonValueCR jsonOptions)
    {
    INavNodesDataSourcePtr source = GetCachedDataSource(connection, parent, jsonOptions);
    if (source.IsNull())
        source = EmptyNavNodesDataSource::Create();
    source = PagingDataSource::Create(*source, pageOpts.GetPageStart(), pageOpts.GetPageSize());

    LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[GetChildren] Returned [%" PRIu64 ", %" PRIu64 ") nodes for parent: '%s'", 
        (uint64_t)pageOpts.GetPageStart(), (uint64_t)(pageOpts.GetPageStart() + source->GetSize()), parent.GetLabel().c_str()).c_str());

    return DataContainer<NavNodeCPtr>(*source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RulesDrivenECPresentationManager::_GetChildrenCount(ECDbR connection, NavNodeCR parent, JsonValueCR jsonOptions)
    {
    INavNodesDataSourcePtr source = GetCachedDataSource(connection, parent, jsonOptions);
    size_t size = source.IsValid() ? source->GetSize() : 0;

    LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[GetChildrenCount] Returned %" PRIu64 " for parent: '%s'", (uint64_t)size, parent.GetLabel().c_str()).c_str());
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr RulesDrivenECPresentationManager::_GetParent(ECDbR connection, NavNodeCR node, JsonValueCR)
    {
    if (0 == node.GetParentNodeId())
        return nullptr;

    return GetNodesCache().GetNode(node.GetParentNodeId(), NodeVisibility::Physical);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr RulesDrivenECPresentationManager::_GetNode(ECDbR connection, uint64_t nodeId)
    {
    return GetNodesCache().GetNode(nodeId, NodeVisibility::Physical);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool RulesDrivenECPresentationManager::_HasChild(ECDbR, NavNodeCR parent, NavNodeKeyCR childKey, JsonValueCR)
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
SpecificationContentProviderCPtr RulesDrivenECPresentationManager::GetContentProvider(BeSQLite::EC::ECDbR connection, ContentProviderKey const& key, SelectionInfo const& selectionInfo, ContentOptions const& options)
    {
    RefCountedPtr<PerformanceLogger> _l1 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManager] GetContentProvider", NativeLogging::LOG_TRACE);

    OnConnection(connection);
    SpecificationContentProviderPtr provider = m_contentCache->GetProvider(key);
    if (provider.IsValid())
        return provider;

    // get the ruleset
    RefCountedPtr<PerformanceLogger> _l2 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManager::GetContentProvider] Get ruleset", NativeLogging::LOG_TRACE);
    PresentationRuleSetPtr ruleset = FindRuleset(GetLocaters(), connection, options.GetRulesetId());
    if (!ruleset.IsValid())
        return nullptr;
    _l2 = nullptr;

    // get ruleset-related caches
    IUserSettings const& settings = m_userSettings.GetSettings(ruleset->GetRuleSetId().c_str());
    ECExpressionsCache& ecexpressionsCache = m_rulesetECExpressionsCache->Get(ruleset->GetRuleSetId().c_str());

    // get connection-related caches
    RelatedPathsCache& relatedPathsCache = m_relatedPathsCache->GetCache(connection);
    ECSqlStatementCache& statementCache = m_statementCache->GetCache(connection);

    // set up the provider context
    _l2 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManager::GetContentProvider] Prepare context", NativeLogging::LOG_TRACE);
    ContentProviderContextPtr context = ContentProviderContext::Create(*ruleset, true, key.GetPreferredDisplayType(), *m_nodesCache, 
        GetCategorySupplier(), settings, ecexpressionsCache, relatedPathsCache, *m_nodesFactory, m_localState);
    context->SetQueryContext(connection, statementCache, *m_customFunctions);
    context->SetLocalizationContext(GetLocalizationProvider());
    context->SetSelectionContext(selectionInfo);
    context->SetPropertyFormattingContext(GetECPropertyFormatter());
    _l2 = nullptr;
    
    // get content specifications
    _l2 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPreseintationManager::GetContentProvider] Get specifications", NativeLogging::LOG_TRACE);
    RulesPreprocessor::ContentRuleParameters params(connection, selectionInfo.GetSelectedNodeKeys(), key.GetPreferredDisplayType(), selectionInfo.GetSelectionProviderName(), 
        selectionInfo.IsSubSelection(), *ruleset, settings, &context->GetUsedSettingsListener(), ecexpressionsCache, *m_nodesCache);
    ContentRuleSpecificationsList specs = RulesPreprocessor::GetContentSpecifications(params);
    _l2 = nullptr;

    _l2 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManager::GetContentProvider] Create provider", NativeLogging::LOG_TRACE);
    provider = SpecificationContentProvider::Create(*context, specs);
    if (!provider.IsValid())
        return nullptr;

    m_contentCache->CacheProvider(key, *provider);
    return provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProviderPtr RulesDrivenECPresentationManager::GetContentProvider(ECDbR connection, ContentDescriptorCR descriptor, SelectionInfo const& selectionInfo, ContentOptions const& options)
    {
    ContentProviderKey key(connection, options.GetRulesetId(), descriptor.GetPreferredDisplayType(), selectionInfo);
    SpecificationContentProviderCPtr cachedProvider = GetContentProvider(connection, key, selectionInfo, options);
    if (cachedProvider.IsNull())
        return nullptr;

    SpecificationContentProviderPtr provider = cachedProvider->Clone();
    provider->SetContentDescriptor(descriptor);
    return provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SelectClassInfo> RulesDrivenECPresentationManager::_GetContentClasses(ECDbR connection, Utf8CP preferredDisplayType, bvector<ECClassCP> const& classes, JsonValueCR jsonOptions)
    {
    RefCountedPtr<PerformanceLogger> _l = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManager] GetContentClasses", NativeLogging::LOG_TRACE);

    if (nullptr == preferredDisplayType || 0 == *preferredDisplayType) 
        preferredDisplayType = ContentDisplayType::Undefined;

    ContentOptions options(jsonOptions);

    // get the ruleset
    RefCountedPtr<PerformanceLogger> _l2 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManager::GetContentProvider] Get ruleset", NativeLogging::LOG_TRACE);
    PresentationRuleSetPtr ruleset = FindRuleset(GetLocaters(), connection, options.GetRulesetId());
    if (!ruleset.IsValid())
        return bvector<SelectClassInfo>();
    _l2 = nullptr;
    
    // get ruleset-related caches
    IUserSettings const& settings = m_userSettings.GetSettings(ruleset->GetRuleSetId().c_str());
    ECExpressionsCache& ecexpressionsCache = m_rulesetECExpressionsCache->Get(ruleset->GetRuleSetId().c_str());

    // get connection-related caches
    RelatedPathsCache& relatedPathsCache = m_relatedPathsCache->GetCache(connection);
    ECSqlStatementCache& statementCache = m_statementCache->GetCache(connection);

    // locate the classes
    ECSchemaHelper schemaHelper(connection, relatedPathsCache, &statementCache);
    ContentClassesLocater::Context locaterContext(schemaHelper, *ruleset, preferredDisplayType, settings, ecexpressionsCache, *m_nodesCache);
    return ContentClassesLocater(locaterContext).Locate(classes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorCPtr RulesDrivenECPresentationManager::_GetContentDescriptor(ECDbR connection, Utf8CP preferredDisplayType, SelectionInfo const& selectionInfo, JsonValueCR jsonOptions)
    {
    RefCountedPtr<PerformanceLogger> _l = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManager] GetContentDescriptor", NativeLogging::LOG_TRACE);

    if (nullptr == preferredDisplayType || 0 == *preferredDisplayType) 
        preferredDisplayType = ContentDisplayType::Undefined;

    ContentOptions options(jsonOptions);
    ContentProviderKey key(connection, options.GetRulesetId(), preferredDisplayType, selectionInfo);
    ContentProviderCPtr provider = GetContentProvider(connection, key, selectionInfo, options);
    if (provider.IsValid())
        return provider->GetContentDescriptor();
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentCPtr RulesDrivenECPresentationManager::_GetContent(ECDbR connection, ContentDescriptorCR descriptor, SelectionInfo const& selectionInfo, PageOptionsCR pageOpts, JsonValueCR jsonOptions)
    {
    RefCountedPtr<PerformanceLogger> _l1 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManager] GetContent", NativeLogging::LOG_TRACE);
    
    ContentOptions options(jsonOptions);
    ContentProviderPtr provider = GetContentProvider(connection, descriptor, selectionInfo, options);
    if (provider.IsNull() || nullptr == provider->GetContentDescriptor())
        {
        LoggingHelper::LogMessage(Log::Content, "No content", NativeLogging::LOG_ERROR);
        return nullptr;
        }

    provider->SetPageOptions(pageOpts);
    
    ContentPtr content = Content::Create(*provider->GetContentDescriptor(), *ContentSetDataSource::Create(*provider));

    Utf8PrintfString range("[%" PRIu64 ", %" PRIu64 ")", (uint64_t)pageOpts.GetPageStart(), descriptor.MergeResults() ? 1 : (uint64_t)(pageOpts.GetPageStart() + content->GetContentSet().GetSize()));
    LoggingHelper::LogMessage(Log::Content, Utf8String("[GetContent] Returned ").append(range).append(" of content set").c_str());

    return content;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RulesDrivenECPresentationManager::_GetContentSetSize(ECDbR connection, ContentDescriptorCR descriptor, SelectionInfo const& selectionInfo, JsonValueCR jsonOptions)
    {
    RefCountedPtr<PerformanceLogger> _l1 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManager] GetContentSetSize", NativeLogging::LOG_TRACE);
    
    ContentOptions options(jsonOptions);
    ContentProviderPtr provider = GetContentProvider(connection, descriptor, selectionInfo, options);
    if (provider.IsNull() || nullptr == provider->GetContentDescriptor())
        {
        LoggingHelper::LogMessage(Log::Content, "No content", NativeLogging::LOG_ERROR);
        return 0;
        }

    RefCountedPtr<PerformanceLogger> _l2 = LoggingHelper::CreatePerformanceLogger(Log::Content, "[RulesDrivenECPresentationManager::GetContentSetSize] Query size", NativeLogging::LOG_TRACE);
    size_t size = provider->GetFullContentSetSize();
    LoggingHelper::LogMessage(Log::Content, Utf8PrintfString("[GetContentSetSize] returned %" PRIu64, (uint64_t)size).c_str(), NativeLogging::LOG_DEBUG);
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NodesCache& RulesDrivenECPresentationManager::GetNodesCacheR() const {return *m_nodesCache;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RuleSetLocaterManager& RulesDrivenECPresentationManager::GetLocaters() {return m_locaters;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IUserSettings& RulesDrivenECPresentationManager::GetUserSettings(Utf8CP rulesetId) const
    {
    return m_userSettings.GetSettings(rulesetId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::SetLocalState(IJsonLocalState& localState)
    {
    m_localState = &localState;
#ifdef USER_SETTINGS_LOCALSTATE
    m_userSettings.SetLocalState(&localState);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::RegisterECInstanceChangeEventSource(ECInstanceChangeEventSource& source)
    {
    m_ecInstanceChangeEventSources.push_back(&source);
    source.RegisterEventHandler(*m_updateHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::UnregisterECInstanceChangeEventSource(ECInstanceChangeEventSource& source)
    {
    source.UnregisterEventHandler(*m_updateHandler);
    m_ecInstanceChangeEventSources.erase(std::find(m_ecInstanceChangeEventSources.begin(), m_ecInstanceChangeEventSources.end(), &source));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::RegisterUpdateRecordsHandler(IUpdateRecordsHandler* handler)
    {
    m_updateHandler->SetRecordsHandler(handler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::SetSelectionManager(SelectionManagerP manager)
    {
    if (nullptr != m_selectionManager)
        m_selectionManager->RemoveListener(*this);
    
    if (nullptr != manager)
        manager->AddListener(*this);
    
    m_updateHandler->SetSelectionManager(manager);
    m_selectionManager = manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IECPropertyFormatter const& RulesDrivenECPresentationManager::GetECPropertyFormatter() const
    {
    if (nullptr != m_ecPropertyFormatter)
        return *m_ecPropertyFormatter;

    static DefaultPropertyFormatter s_defaultPropertyFormatter;
    return s_defaultPropertyFormatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IPropertyCategorySupplier& RulesDrivenECPresentationManager::GetCategorySupplier() const
    {
    if (nullptr != m_categorySupplier)
        return *m_categorySupplier;

    static DefaultCategorySupplier s_defaultCategorySupplier;
    return s_defaultCategorySupplier;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ILocalizationProvider const& RulesDrivenECPresentationManager::GetLocalizationProvider() const
    {
    if (nullptr != m_localizationProvider)
        return *m_localizationProvider;

    static SQLangLocalizationProvider s_localizationProvider;
    return s_localizationProvider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::SetLocalizationProvider(ILocalizationProvider const* provider)
    {
    m_localizationProvider = provider;
    m_userSettings.SetLocalizationProvider(&GetLocalizationProvider());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::RegisterECInstanceChangeHandler(IECInstanceChangeHandler& handler)
    {
    m_ecInstanceChangeHandlers.insert(&handler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::UnregisterECInstanceChangeHandler(IECInstanceChangeHandler& handler)
    {
    m_ecInstanceChangeHandlers.erase(&handler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceChangeResult> RulesDrivenECPresentationManager::_SaveValueChange(ECDbR connection, bvector<ChangedECInstanceInfo> const& instances, 
    Utf8CP propertyAccessor, ECValueCR value, JsonValueCR)
    {
    ECInstanceChangesDirector director(m_ecInstanceChangeHandlers);
    return director.Handle(connection, instances, propertyAccessor, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::_OnNodeChecked(ECDbR connection, uint64_t nodeId)
    {
    NavNodeCPtr node = GetNode(connection, nodeId);
    if (node.IsValid())
        CustomizationHelper::NotifyCheckedStateChanged(connection, *node, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::_OnNodeUnchecked(ECDbR connection, uint64_t nodeId)
    {
    NavNodeCPtr node = GetNode(connection, nodeId);
    if (node.IsValid())
        CustomizationHelper::NotifyCheckedStateChanged(connection, *node, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::_OnNodeExpanded(ECDbR connection, uint64_t nodeId)
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
void RulesDrivenECPresentationManager::_OnNodeCollapsed(ECDbR connection, uint64_t nodeId)
    {
    JsonNavNodePtr node = GetNodesCache().GetNode(nodeId);
    if (node.IsValid())
        {
        node->SetIsExpanded(false);
        GetNodesCache().Update(nodeId, *node);
        }
    }
