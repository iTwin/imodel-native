/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/TestHelpers.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "../../../Source/RulesDriven/RulesEngine/QueryExecutor.h"
#include "../../../Source/RulesDriven/RulesEngine/ECSchemaHelper.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryContracts.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryBuilder.h"
#include "../../../Source/RulesDriven/RulesEngine/NavNodeProviders.h"
#include "../../BackDoor/PublicAPI/BackDoor/ECPresentation/Localization.h"
#include "../../BackDoor/PublicAPI/BackDoor/ECPresentation/StubLocalState.h"
#include <UnitTests/BackDoor/ECPresentation/TestConnectionCache.h>
#include <UnitTests/BackDoor/ECPresentation/TestUserSettings.h>
#include "ECDbTestProject.h"
#include "TestNavNode.h"
#include "TestNodesProvider.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2017
+===============+===============+===============+===============+===============+======*/
template<typename TRegistry> struct RegisterSchemaHelper
    {
    RegisterSchemaHelper(Utf8String name, Utf8String schemaXml)
        {
        TRegistry::RegisterSchemaXml(name, schemaXml);
        }
    };
#define DEFINE_REGISTRY_SCHEMA(registry, name, schema_xml) \
    static RegisterSchemaHelper<registry> _register_schema_##name(#name, \
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
        "<ECSchema schemaName=\"" #name "\" alias=\"alias_" #name "\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">" \
            "<ECSchemaReference name=\"CoreCustomAttributes\" version=\"1.0\" alias=\"CoreCA\"/>" \
            "<ECSchemaReference name=\"ECDbMap\" version=\"2.0\" alias=\"ecdbmap\"/>" \
            schema_xml \
        "</ECSchema>")

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct RulesEngineTestHelpers
    {
    typedef std::function<void(ComplexNavigationQueryR)> ComplexQueryHandler;

    static RulesDrivenECPresentationManager::Paths GetPaths(BeTest::Host&);

    static Utf8String GetDisplayLabel(IECInstanceCR instance);

    static IECInstancePtr InsertInstance(ECDbR, ECClassCR ecClass, std::function<void(IECInstanceR)> const& instancePreparer = nullptr, bool commit = false);
    static IECInstancePtr InsertInstance(ECDbR, ECInstanceInserter& inserter, ECClassCR ecClass, std::function<void(IECInstanceR)> const& instancePreparer = nullptr, bool commit = false);
    static ECInstanceKey InsertRelationship(ECDbR db, ECRelationshipClassCR relationship, IECInstanceCR source, IECInstanceR target, std::function<void(IECInstanceR)> const& instancePreparer = nullptr, bool commit = false);
    static ECInstanceKey InsertRelationship(ECDbTestProject& project, ECRelationshipClassCR relationship, IECInstanceCR source, IECInstanceR target, std::function<void(IECInstanceR)> const& instancePreparer = nullptr, bool commit = false);
    static void DeleteInstances(ECDbR db, ECClassCR ecClass, bool polymorphic = false, bool commit = false);
    static void DeleteInstance(ECDbR db, ECInstanceKeyCR key, bool commit = false);
    static void DeleteInstance(ECDbR db, IECInstanceCR instance, bool commit = false);
    static void DeleteInstance(ECDbTestProject& project, ECInstanceKeyCR key, bool commit = false);
    static void DeleteInstance(ECDbTestProject& project, IECInstanceCR instance, bool commit = false);
    static IECInstancePtr GetInstance(ECDbR& project, ECClassCR ecClass, ECInstanceId id);
    static ECClassInstanceKey GetInstanceKey(IECInstanceCR);

    static NavigationQueryPtr CreateECInstanceNodesQueryForClasses(ECClassSet const& classes, Utf8CP alias, ComplexQueryHandler handler = nullptr);
    static ComplexNavigationQueryPtr CreateECInstanceNodesQueryForClass(ECEntityClassCR ecClass, bool polymorphic, Utf8CP alias, bvector<RelatedClass> const& = bvector<RelatedClass>());
    static NavigationQueryPtr CreateLabelGroupingNodesQueryForClasses(ECClassSet const& classes, Utf8CP alias, ComplexQueryHandler handler = nullptr);
    static NavigationQueryPtr CreateQuery(NavigationQueryContract const&, bset<ECN::ECClassCP>, bool polymorphic, Utf8CP alias, ComplexQueryHandler handler = nullptr);
    static NavigationQueryPtr CreateQuery(NavigationQueryContract const&, bvector<ECN::ECClassCP>, bool polymorphic, Utf8CP alias, ComplexQueryHandler handler = nullptr);
    
    static void ValidateContentSetItem(ECN::IECInstanceCR instance, ContentSetItemCR item, ContentDescriptorCR descriptor, Utf8CP expectedLabel = nullptr, Utf8CP expectedImageId = nullptr);
    static void ValidateContentSet(bvector<ECN::IECInstanceCP> instances, Content const& content, bool validateOrder = false);

    static ContentDescriptor::Field& AddField(ContentDescriptorR, ECN::ECClassCR, ContentDescriptor::Property, IPropertyCategorySupplierR);
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct TestLocalState : IJsonLocalState
{
private:
    std::function<void(Utf8CP, Utf8CP, JsonValueCR)> m_saveHandler;
    std::function<Json::Value(Utf8CP, Utf8CP)> m_getHandler;

protected:
    void _SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) override
        {
        Json::Value jsonValue;
        Json::Reader().parse(value, jsonValue, false);
        if (nullptr != m_saveHandler)
            m_saveHandler(nameSpace, key, jsonValue);
        }
    Utf8String _GetValue(Utf8CP nameSpace, Utf8CP key) const override
        {
        return nullptr != m_getHandler ? Json::FastWriter().write(m_getHandler(nameSpace, key)) : "null"; 
        }

public:
    TestLocalState(){}
    void SetSaveHandler(std::function<void(Utf8CP, Utf8CP, JsonValueCR)> const& handler) {m_saveHandler = handler;}
    void SetGetHandler(std::function<Json::Value(Utf8CP, Utf8CP)> const& handler) {m_getHandler = handler;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct TestECInstanceChangeEventsSource : ECInstanceChangeEventSource
{
private:
    void NotifyECInstanceChanged(ECDbCR db, IECInstanceCR instance, ChangeType change) const
        {
        ECInstanceId id;
        ECInstanceId::FromString(id, instance.GetInstanceId().c_str());
        ECInstanceChangeEventSource::NotifyECInstanceChanged(db, ECInstanceChangeEventSource::ChangedECInstance(instance.GetClass(), id, change));
        }
    
public:
    static RefCountedPtr<TestECInstanceChangeEventsSource> Create() {return new TestECInstanceChangeEventsSource();}
    void NotifyECInstancesChanged(ECDbCR db, bvector<IECInstanceCP> instances, ChangeType change) const
        {
        bvector<ECInstanceChangeEventSource::ChangedECInstance> changes;
        for (IECInstanceCP instance : instances)
            {
            ECInstanceId id;
            ECInstanceId::FromString(id, instance->GetInstanceId().c_str());
            changes.push_back(ECInstanceChangeEventSource::ChangedECInstance(instance->GetClass(), id, change));
            }
        ECInstanceChangeEventSource::NotifyECInstancesChanged(db, changes);
        }
    void NotifyECInstanceInserted(ECDbCR db, IECInstanceCR instance) const {NotifyECInstanceChanged(db, instance, ChangeType::Insert);}
    void NotifyECInstanceDeleted(ECDbCR db, IECInstanceCR instance) const {NotifyECInstanceChanged(db, instance, ChangeType::Delete);}
    void NotifyECInstanceUpdated(ECDbCR db, IECInstanceCR instance) const {NotifyECInstanceChanged(db, instance, ChangeType::Update);}
    void NotifyECInstancesInserted(ECDbCR db, bvector<IECInstanceCP> instances) const {NotifyECInstancesChanged(db, instances, ChangeType::Insert);}
    void NotifyECInstancesDeleted(ECDbCR db, bvector<IECInstanceCP> instances) const {NotifyECInstancesChanged(db, instances, ChangeType::Delete);}
    void NotifyECInstancesUpdated(ECDbCR db, bvector<IECInstanceCP> instances) const {NotifyECInstancesChanged(db, instances, ChangeType::Update);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TestECInstanceChangeEventsHandler : ECInstanceChangeEventSource::IEventHandler
{
private:
    std::function<void(ECDbCR, bvector<ECInstanceChangeEventSource::ChangedECInstance>)> m_callback;
protected:
    void _OnECInstancesChanged(ECDbCR db, bvector<ECInstanceChangeEventSource::ChangedECInstance> changes) override
        {
        if (m_callback)
            m_callback(db, changes);
        }
public:
    void SetCallback(std::function<void(ECDbCR, bvector<ECInstanceChangeEventSource::ChangedECInstance>)> callback) {m_callback = callback;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
struct TestNodeLocater : INavNodeLocater
{
private:
    bmap<NavNodeKeyCP, JsonNavNodeCPtr, NavNodeKeyPtrComparer> m_nodes;

protected:
    JsonNavNodeCPtr _LocateNode(IConnectionCR, Utf8StringCR, NavNodeKeyCR key) const override
        {
        auto iter = m_nodes.find(&key);
        if (m_nodes.end() != iter)
            return iter->second;
        return nullptr;
        }

public:
    TestNodeLocater() {}
    TestNodeLocater(JsonNavNodeCR node) {AddNode(node);}
    void AddNode(JsonNavNodeCR node) {m_nodes[node.GetKey().get()] = &node;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct TestNodesCache : IHierarchyCache, INavNodeLocater
{
    typedef std::function<JsonNavNodePtr(uint64_t, NodeVisibility)> GetNodeHandler;
    typedef std::function<NavNodesProviderPtr(HierarchyLevelInfo const&)> GetHierarchyDataSourceHandler;
    typedef std::function<NavNodesProviderPtr(DataSourceInfo const&)> GetVirtualDataSourceHandler;
    typedef std::function<NavNodesProviderPtr(uint64_t)> GetParentNodeDataSourceHandler;
    typedef std::function<void(DataSourceInfo&, DataSourceFilter const&, bmap<ECClassId, bool> const&, bvector<UserSettingEntry> const&, bool)> CacheDataSourceHandler;
    typedef std::function<void(JsonNavNodeR, bool)> CacheNodeHandler;
    typedef std::function<void(JsonNavNodeCR)> MakePhysicalHandler;
    typedef std::function<void(JsonNavNodeCR)> MakeVirtualHandler;
    typedef std::function<void(uint64_t, JsonNavNodeCR)> UpdateNodeHandler;
    typedef std::function<void(DataSourceInfo const&, DataSourceFilter const*, bmap<ECClassId, bool> const*, bvector<UserSettingEntry> const*)> UpdateDataSourceHandler;
    typedef std::function<JsonNavNodeCPtr(IConnectionCR, Utf8StringCR, NavNodeKeyCR)> LocateNodeHandler;

    /*=================================================================================**//**
    * @bsiclass                                     Grigas.Petraitis                11/2017
    +===============+===============+===============+===============+===============+======*/
    struct Savepoint : IHierarchyCache::Savepoint
        {
        TestNodesCache& m_cache;
        bmap<uint64_t, JsonNavNodePtr> m_nodes;
        bmap<HierarchyLevelInfo, bvector<JsonNavNode*>> m_physicalHierarchy;
        bmap<DataSourceInfo, bvector<JsonNavNode*>> m_virtualHierarchy;
        Savepoint(TestNodesCache& cache)
            : m_cache(cache)
            {
            m_nodes = cache.m_nodes;
            m_physicalHierarchy = cache.m_physicalHierarchy;
            m_virtualHierarchy = cache.m_virtualHierarchy;
            }
        void _Cancel() override
            {
            m_cache.m_nodes = m_nodes;
            m_cache.m_physicalHierarchy = m_physicalHierarchy;
            m_cache.m_virtualHierarchy = m_virtualHierarchy;
            }
        };

private:
    uint64_t m_nodeIds;
    uint64_t m_datasourceIds;
    bmap<uint64_t, JsonNavNodePtr> m_nodes;
    bmap<HierarchyLevelInfo, bvector<JsonNavNode*>> m_physicalHierarchy;
    bmap<DataSourceInfo, bvector<JsonNavNode*>> m_virtualHierarchy;

    INodesProviderContextFactory* m_nodesProviderContextFactory;
    IConnectionCacheCR m_connections;

    GetNodeHandler m_getNodeHandler;
    GetHierarchyDataSourceHandler m_getHierarchyDataSourceHandler;
    GetVirtualDataSourceHandler m_getVirtualDataSourceHandler;
    GetParentNodeDataSourceHandler m_getParentNodeDataSourceHandler;
    CacheDataSourceHandler m_cacheDataSourceHandler;
    CacheNodeHandler m_cacheNodeHandler;
    MakePhysicalHandler m_makePhysicalHandler;
    MakeVirtualHandler m_makeVirtualHandler;
    UpdateNodeHandler m_updateNodeHandler;
    UpdateDataSourceHandler m_updateDataSourceHandler;
    LocateNodeHandler m_locateNodeHandler;

private:
    HierarchyLevelInfo GetHierarchyLevelInfo(JsonNavNodeCR node) const
        {
        NavNodeExtendedData ex(node);
        return HierarchyLevelInfo(ex.GetConnectionId(), ex.GetRulesetId(), ex.GetLocale(), node.GetParentNodeId());
        }
    DataSourceInfo GetDataSourceInfo(JsonNavNodeCR node) const
        {
        NavNodeExtendedData ex(node);
        uint64_t physicalParentId = node.GetParentNodeId();
        uint64_t virtualParentId = ex.GetVirtualParentId();
        return DataSourceInfo(ex.GetConnectionId(), ex.GetRulesetId(), ex.GetLocale(), &physicalParentId, &virtualParentId);
        }

protected:
    JsonNavNodePtr _GetNode(uint64_t nodeId, NodeVisibility visibility) const override
        {
        if (m_getNodeHandler)
            return m_getNodeHandler(nodeId, visibility);

        auto iter = m_nodes.find(nodeId);
        return (m_nodes.end() != iter) ? iter->second : nullptr;
        }
    NavNodesProviderPtr _GetDataSource(HierarchyLevelInfo const& info, bool removeIfInvalid) const override
        {
        if (m_getHierarchyDataSourceHandler)
            return m_getHierarchyDataSourceHandler(info);

        if (nullptr == m_nodesProviderContextFactory)
            return nullptr;

        IConnectionPtr connection = m_connections.GetConnection(info.GetConnectionId().c_str());
        if (connection.IsNull())
            {
            BeAssert(false);
            return nullptr;
            }

        NavNodesProviderContextPtr context = m_nodesProviderContextFactory->Create(*connection, info.GetRulesetId().c_str(),
            info.GetLocale().c_str(), info.GetPhysicalParentNodeId());
        auto iter = m_physicalHierarchy.find(info);
        return (m_physicalHierarchy.end() != iter) ? BVectorNodesProvider::Create(*context, iter->second) : nullptr;
        }
    NavNodesProviderPtr _GetDataSource(DataSourceInfo const& info, bool removeIfInvalid) const override
        {
        if (m_getVirtualDataSourceHandler)
            return m_getVirtualDataSourceHandler(info);

        if (nullptr == m_nodesProviderContextFactory)
            return nullptr;
        
        IConnectionPtr connection = m_connections.GetConnection(info.GetConnectionId().c_str());
        if (connection.IsNull())
            {
            BeAssert(false);
            return nullptr;
            }

        NavNodesProviderContextPtr context = m_nodesProviderContextFactory->Create(*connection, info.GetRulesetId().c_str(),
            info.GetLocale().c_str(), info.GetVirtualParentNodeId());
        auto iter = m_virtualHierarchy.find(info);
        return (m_virtualHierarchy.end() != iter) ? BVectorNodesProvider::Create(*context, iter->second) : nullptr;
        }
    NavNodesProviderPtr _GetDataSource(uint64_t nodeId, bool removeIfInvalid) const override
        {
        if (m_getParentNodeDataSourceHandler)
            return m_getParentNodeDataSourceHandler(nodeId);

        JsonNavNodePtr node = GetNode(nodeId);
        DataSourceInfo info = GetDataSourceInfo(*node);
        return GetDataSource(info, removeIfInvalid);
        }
    
    void _Cache(DataSourceInfo& info, DataSourceFilter const& filter, bmap<ECClassId, bool> const& relatedClassIds, bvector<UserSettingEntry> const& relatedSettings, bool updatesDisabled) override
        {
        info.SetDataSourceId(++m_datasourceIds);
        m_physicalHierarchy[info] = bvector<JsonNavNode*>();
        m_virtualHierarchy[info] = bvector<JsonNavNode*>();

        if (m_cacheDataSourceHandler)
            m_cacheDataSourceHandler(info, filter, relatedClassIds, relatedSettings, updatesDisabled);
        }
    void _Cache(JsonNavNodeR node, bool isVirtual) override
        {
        uint64_t nodeId = ++m_nodeIds;
        node.SetNodeId(nodeId);
        m_nodes[nodeId] = &node;

        DataSourceInfo dsInfo = GetDataSourceInfo(node);
        uint64_t const* parentId = dsInfo.GetVirtualParentNodeId();
        bvector<Utf8String> pathFromRoot = (nullptr == parentId || 0 == *parentId) ? bvector<Utf8String>() : m_nodes[*parentId]->GetKey()->GetPathFromRoot();
        pathFromRoot.push_back(std::to_string(nodeId).c_str());
        IConnectionPtr connection = m_connections.GetConnection(dsInfo.GetConnectionId().c_str());
        node.SetNodeKey(*NavNodesHelper::CreateNodeKey(*connection, node, pathFromRoot));
        m_physicalHierarchy[GetHierarchyLevelInfo(node)].push_back(&node);
        m_virtualHierarchy[dsInfo].push_back(&node);

        if (m_cacheNodeHandler)
            m_cacheNodeHandler(node, isVirtual);
        }

    void _MakePhysical(JsonNavNodeCR node) override
        {
        if (m_makePhysicalHandler)
            m_makePhysicalHandler(node);
        }
    void _MakeVirtual(JsonNavNodeCR node) override
        {
        if (m_makeVirtualHandler)
            return m_makeVirtualHandler(node);
        }
    void _Update(uint64_t id, JsonNavNodeCR node) override
        {
        if (m_updateNodeHandler)
            return m_updateNodeHandler(id, node);
        }
    void _Update(DataSourceInfo const& info, DataSourceFilter const* filter, bmap<ECClassId, bool> const* relatedClassIds, bvector<UserSettingEntry> const* relatedSettings) override
        {
        if (m_updateDataSourceHandler)
            return m_updateDataSourceHandler(info, filter, relatedClassIds, relatedSettings);
        }

    JsonNavNodeCPtr _LocateNode(IConnectionCR connection, Utf8StringCR locale, NavNodeKeyCR key) const override
        {
        if (m_locateNodeHandler)
            return m_locateNodeHandler(connection, locale, key);
        return nullptr;
        }
    IHierarchyCache::SavepointPtr _CreateSavepoint() override {return new Savepoint(*this);}

public:
    TestNodesCache(IConnectionCacheCR connections, INodesProviderContextFactory* nodesProviderContextFactory = nullptr) 
        : m_nodesProviderContextFactory(nodesProviderContextFactory), m_connections(connections), m_nodeIds(1), m_datasourceIds(1)
        {}
    void SetGetNodeHandler(GetNodeHandler handler) {m_getNodeHandler = handler;}
    void SetGetHierarchyDataSourceHandler(GetHierarchyDataSourceHandler handler) {m_getHierarchyDataSourceHandler = handler;}
    void SetGetVirtualDataSourceHandler(GetVirtualDataSourceHandler handler) {m_getVirtualDataSourceHandler = handler;}
    void SetGetParentNodeDataSourceHandler(GetParentNodeDataSourceHandler handler) {m_getParentNodeDataSourceHandler = handler;}
    void SetCacheDataSourceHandler(CacheDataSourceHandler handler) {m_cacheDataSourceHandler = handler;}
    void SetCacheNodeHandler(CacheNodeHandler handler) {m_cacheNodeHandler = handler;}
    void SetMakePhysicalHandler(MakePhysicalHandler handler) {m_makePhysicalHandler = handler;}
    void SetMakeVirtualHandler(MakeVirtualHandler handler) {m_makeVirtualHandler = handler;}
    void SetUpdateNodeHandler(UpdateNodeHandler handler) {m_updateNodeHandler = handler;}
    void SetUpdateDataSourceHandler(UpdateDataSourceHandler handler) {m_updateDataSourceHandler = handler;}
    void SetLocateNodeHandler(LocateNodeHandler handler) {m_locateNodeHandler = handler;}
};

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis                01/2017
//=======================================================================================
struct TestCategorySupplier : IPropertyCategorySupplier
    {
    ContentDescriptor::Category m_category;
    TestCategorySupplier( )
        : m_category(ContentDescriptor::Category::GetDefaultCategory())
        {
        }
    TestCategorySupplier(ContentDescriptor::Category category)
        : m_category(category)
        {
        }
    ContentDescriptor::Category _GetCategory(ECClassCR, RelatedClassPathCR, ECPropertyCR, RelationshipMeaning) override {return m_category;}
    ContentDescriptor::Category _GetCategory(ECClassCR, RelatedClassPathCR, ECClassCR) override {return m_category;}
    ContentDescriptor::Category GetUsedCategory() const {return m_category;}
    void SetUsedCategory(ContentDescriptor::Category category)
        {
        m_category = category;
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2016
+===============+===============+===============+===============+===============+======*/
struct TestPropertyFormatter : IECPropertyFormatter
    {
    BentleyStatus _GetFormattedPropertyValue(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue) const override;
    BentleyStatus _GetFormattedPropertyLabel(Utf8StringR formattedLabel, ECPropertyCR ecProperty, ECClassCR propertyClass, RelatedClassPath const& relatedClassPath, RelationshipMeaning relationshipMeaning) const override;
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct TestParsedInput : IParsedInput
{
private:
    bvector<ECClassCP> m_classes;
    bmap<ECClassCP, bvector<ECInstanceId>> m_instanceIds;
protected:
    bvector<ECClassCP> const& _GetClasses() const override {return m_classes;}
    bvector<ECInstanceId> const& _GetInstanceIds(ECClassCR ecClass) const override
        {
        auto iter = m_instanceIds.find(&ecClass);
        if (m_instanceIds.end() != iter)
            return iter->second;
        static bvector<ECInstanceId> s_empty;
        return s_empty;
        }
public:
    TestParsedInput() {}
    TestParsedInput(IECInstanceCR instance)
        {
        m_classes.push_back(&instance.GetClass());
        m_instanceIds[&instance.GetClass()].push_back((ECInstanceId)ECInstanceId::FromString(instance.GetInstanceId().c_str()));
        }
    TestParsedInput(ECClassCR ecClass, ECInstanceId instanceId)
        {
        m_classes.push_back(&ecClass);
        m_instanceIds[&ecClass].push_back(instanceId);
        }
    TestParsedInput(ECClassCR ecClass, bvector<ECInstanceId> instanceIds)
        {
        m_classes.push_back(&ecClass);
        m_instanceIds[&ecClass] = instanceIds;
        }
    TestParsedInput(bvector<bpair<ECClassCP, ECInstanceId>> pairs)
        {
        bset<ECClassCP> used;
        for (auto pair : pairs)
            {
            if (used.end() == used.find(pair.first))
                {
                m_classes.push_back(pair.first);
                used.insert(pair.first);
                }
            m_instanceIds[pair.first].push_back(pair.second);
            }
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2016
+===============+===============+===============+===============+===============+======*/
struct TestUsedClassesListener : IUsedClassesListener
    {
    bmap<ECClassCP, bool> m_usedClasses;
    void _OnClassUsed(ECClassCR ecClass, bool polymorphically)
        {
        m_usedClasses[&ecClass] = polymorphically;
        }
    bmap<ECClassCP, bool> const& GetUsedClasses() const {return m_usedClasses;}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+===============+===============+===============+===============+===============+======*/
struct TestECDbUsedClassesListener : IECDbUsedClassesListener
    {
    bmap<ECClassCP, bool> m_usedClasses;
    void _OnClassUsed(ECDbCR, ECN::ECClassCR ecClass, bool polymorphically) override
        {
        m_usedClasses[&ecClass] = polymorphically;
        }
    bmap<ECClassCP, bool> const& GetUsedClasses() const {return m_usedClasses;}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct TestNodesProviderFactory : INodesProviderFactory
{
private:

private:
    NavNodesProviderPtr CreateProvider(NavNodesProviderContextR context, JsonNavNodeCP parent) const
        {
        NavNodesProviderPtr provider;
        if (nullptr == parent)
            {
            RulesPreprocessor::RootNodeRuleParameters params(context.GetConnections(), context.GetConnection(), context.GetRuleset(), TargetTree_MainTree,
                context.GetLocale(), context.GetUserSettings(), nullptr, context.GetECExpressionsCache());
            RootNodeRuleSpecificationsList specs = RulesPreprocessor::GetRootNodeSpecifications(params);
            provider = MultiSpecificationNodesProvider::Create(context, specs);
            }
        else
            {
            RulesPreprocessor::ChildNodeRuleParameters params(context.GetConnections(), context.GetConnection(), *parent, context.GetRuleset(), TargetTree_MainTree, 
                context.GetLocale(), context.GetUserSettings(), nullptr, context.GetECExpressionsCache());
            ChildNodeRuleSpecificationsList specs = RulesPreprocessor::GetChildNodeSpecifications(params);
            provider = MultiSpecificationNodesProvider::Create(context, specs, *parent);
            }
        return provider;
        }

protected:
    NavNodesProviderPtr _CreateForHierarchyLevel(NavNodesProviderContextR context, JsonNavNodeCP parent) const override
        {
        return CreateProvider(context, parent);
        }
    NavNodesProviderPtr _CreateForVirtualParent(NavNodesProviderContextR context, JsonNavNodeCP parent) const override
        {
        return CreateProvider(context, parent);

        }
public:
    TestNodesProviderFactory() {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct TestNodesProviderContextFactory : INodesProviderContextFactory
{
private:
    IConnectionManagerCR m_connections;
    TestUserSettings m_settings;
    JsonNavNodesFactory m_nodesFactory;
    TestNodesProviderFactory m_providerFactory;
    PresentationRuleSetCPtr m_ruleset;
    IECDbUsedClassesListener* m_usedClassesListener;
    mutable ECExpressionsCache m_ecexpressionsCache;
    mutable RelatedPathsCache m_relatedPathsCache;
    mutable PolymorphicallyRelatedClassesCache m_polymorphicallyRelatedClassesCache;
    mutable ECSqlStatementCache m_statementsCache;
    mutable CustomFunctionsInjector m_customFunctions;
    mutable TestNodesCache m_testNodesCache;
    mutable IHierarchyCacheP m_nodesCache;

private:
    IHierarchyCacheR GetNodesCache() const {return (nullptr != m_nodesCache) ? *m_nodesCache : m_testNodesCache;}

protected:
    NavNodesProviderContextPtr _Create(IConnectionCR connection, Utf8CP rulesetId, Utf8CP locale, 
        uint64_t const* parentNodeId, ICancelationTokenCP cancelationToken, bool disableUpdates) const override
        {
        m_customFunctions.OnConnection(connection);

        PresentationRuleSetCPtr ruleset = m_ruleset;
        if (ruleset.IsNull())
            ruleset = PresentationRuleSet::CreateInstance(rulesetId, 1, 0, false, "", "", "", false);
        NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*ruleset, true, TargetTree_MainTree, locale, parentNodeId, 
            m_settings, m_ecexpressionsCache, m_relatedPathsCache, m_polymorphicallyRelatedClassesCache, m_nodesFactory, 
            GetNodesCache(), m_providerFactory, nullptr);
        context->SetQueryContext(m_connections, connection, m_statementsCache, m_customFunctions, m_usedClassesListener);
        context->SetIsUpdatesDisabled(disableUpdates);
        context->SetCancelationToken(cancelationToken);
        return context;
        }

public:
    TestNodesProviderContextFactory(IConnectionManagerCR connections) 
        : m_connections(connections), m_statementsCache(10), m_testNodesCache(connections), m_nodesCache(nullptr), m_customFunctions(connections)
        {}
    void SetNodesCache(IHierarchyCacheP cache) {m_nodesCache = cache;}
    void SetRuleset(PresentationRuleSetCP ruleset) {m_ruleset = ruleset;}
    void SetUsedClassesListener(IECDbUsedClassesListener* listener) {m_usedClassesListener = listener;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+===============+===============+===============+===============+===============+======*/
struct TestRulesetCallbacksHandler : IRulesetCallbacksHandler
    {
    typedef std::function<void(PresentationRuleSetCR)> CallbackHandler;
    CallbackHandler m_onCreatedHandler;
    CallbackHandler m_onDisposedHandler;

    void SetCreatedHandler(CallbackHandler const& handler) {m_onCreatedHandler = handler;}
    void SetDisposedHandler(CallbackHandler const& handler) {m_onDisposedHandler = handler;}

    virtual void _OnRulesetCreated(RuleSetLocaterCR,PresentationRuleSetR ruleset) override
        {
        if (nullptr != m_onCreatedHandler)
            m_onCreatedHandler(ruleset);
        }
    virtual void _OnRulesetDispose(RuleSetLocaterCR, PresentationRuleSetCR ruleset) override
        {
        if (nullptr != m_onDisposedHandler)
            m_onDisposedHandler(ruleset);
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TestUserSettingsChangeListener : IUserSettingsChangeListener
    {
    std::function<void(Utf8CP, Utf8CP)> m_callback;

    virtual void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override
        {
        if (m_callback)
            m_callback(rulesetId, settingId);
        }

    void SetCallback(std::function<void(Utf8CP, Utf8CP)> callback) {m_callback = callback;}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TestCancelationToken : ICancelationToken
{
private:
    std::function<bool()> m_isCanceledCallback;
protected:
    bool _IsCanceled() const override {return m_isCanceledCallback();}
public:
    TestCancelationToken(std::function<bool()> callback) : m_isCanceledCallback(callback) {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TestCallbackRulesetLocater : RefCounted<RuleSetLocater>
{
private:
    std::function<void()> m_callback;
    void Callback() const {if (m_callback) {m_callback();}}
protected:
    bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const override {Callback(); return bvector<PresentationRuleSetPtr>();}
    bvector<Utf8String> _GetRuleSetIds() const override {Callback(); return bvector<Utf8String>();}
    int _GetPriority() const override {Callback(); return 1;}
    void _InvalidateCache(Utf8CP rulesetId) override {Callback();}
public:
    static RefCountedPtr<TestCallbackRulesetLocater> Create() {return new TestCallbackRulesetLocater();}
    void SetCallback(std::function<void()> callback) {m_callback = callback;}
};


END_ECPRESENTATIONTESTS_NAMESPACE
