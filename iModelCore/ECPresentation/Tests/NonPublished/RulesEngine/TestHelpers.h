/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "../../../Source/RulesDriven/RulesEngine/QueryExecutor.h"
#include "../../../Source/RulesDriven/RulesEngine/ECSchemaHelper.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryContracts.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryBuilder.h"
#include "../../../Source/RulesDriven/RulesEngine/NavNodeProviders.h"
#include "../../BackDoor/PublicAPI/BackDoor/ECPresentation/Localization.h"
#include <UnitTests/BackDoor/ECPresentation/TestConnectionCache.h>
#include <UnitTests/BackDoor/ECPresentation/TestUserSettings.h>
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>
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
    RegisterSchemaHelper(Utf8String name, Utf8String schemaXml) {TRegistry::RegisterSchemaXml(name, schemaXml);}
    };
// add this to class declaration
#define DECLARE_SCHEMA_REGISTRY(registry) \
    static bmap<Utf8String, Utf8String>& GetRegisteredSchemaXmls(); \
    static void RegisterSchemaXml(Utf8String name, Utf8String schemaXml);
// add this to source file (registry = test class name)
#define DEFINE_SCHEMA_REGISTRY(registry) \
    bmap<Utf8String, Utf8String>& registry::GetRegisteredSchemaXmls() \
        { \
        static bmap<Utf8String, Utf8String> s_registeredSchemaXmls; \
        return s_registeredSchemaXmls; \
        } \
    void registry::RegisterSchemaXml(Utf8String name, Utf8String schemaXml) {GetRegisteredSchemaXmls()[name] = schemaXml;}
// add this to test setup
#define INIT_SCHEMA_REGISTRY(ecdb) \
    RulesEngineTestHelpers::InitSchemaRegistry(ecdb, GetRegisteredSchemaXmls());
// use this to create and register a schema
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

    static void InitSchemaRegistry(ECDbR ecdb, bmap<Utf8String, Utf8String> const& schemas);

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
    static DataContainer<NavNodeCPtr> GetValidatedNodes(std::function<DataContainer<NavNodeCPtr>()> getter);

    static ContentDescriptor::Field& AddField(ContentDescriptorR, ECN::ECClassCR, ContentDescriptor::Property, IPropertyCategorySupplierR);

    static void CacheNode(IHierarchyCacheR cache, JsonNavNodeR node);


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
    // TODO: this is bad implementation ("null" strings), refer to RuntimeJsonLocalState
    void _SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) override
        {
        Json::Value jsonValue;
        Json::Reader().parse(value, jsonValue, false);
        if (nullptr != m_saveHandler)
            m_saveHandler(nameSpace, key, jsonValue);
        }
    // TODO: this is bad implementation ("null" strings), refer to RuntimeJsonLocalState
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
    typedef std::function<JsonNavNodePtr(uint64_t)> GetNodeHandler;
    typedef std::function<NavNodesProviderPtr(HierarchyLevelInfo const&)> GetHierarchyDataSourceHandler;
    typedef std::function<NavNodesProviderPtr(DataSourceInfo const&)> GetVirtualDataSourceHandler;
    typedef std::function<NavNodesProviderPtr(uint64_t)> GetParentNodeDataSourceHandler;
    typedef std::function<void(HierarchyLevelInfo&)> CacheHierarchyLevelHandler;
    typedef std::function<void(DataSourceInfo&, DataSourceFilter const&, bmap<ECClassId, bool> const&, bvector<UserSettingEntry> const&)> CacheDataSourceHandler;
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
        bmap<CombinedHierarchyLevelInfo, bvector<DataSourceInfo>> m_physicalHierarchy;
        bmap<HierarchyLevelInfo, bvector<DataSourceInfo>> m_virtualHierarchy;
        bmap<DataSourceInfo, bvector<JsonNavNode*>> m_partialHierarchies;
        bset<uint64_t> m_finalizedDataSources;
        Savepoint(TestNodesCache& cache)
            : m_cache(cache)
            {
            BeMutexHolder lock(m_cache.GetMutex());
            m_nodes = cache.m_nodes;
            m_partialHierarchies = cache.m_partialHierarchies;
            m_physicalHierarchy = cache.m_physicalHierarchy;
            m_virtualHierarchy = cache.m_virtualHierarchy;
            m_finalizedDataSources = cache.m_finalizedDataSources;
            }
        void _Cancel() override
            {
            BeMutexHolder lock(m_cache.GetMutex());
            m_cache.m_nodes = m_nodes;
            m_cache.m_partialHierarchies = m_partialHierarchies;
            m_cache.m_physicalHierarchy = m_physicalHierarchy;
            m_cache.m_virtualHierarchy = m_virtualHierarchy;
            m_cache.m_finalizedDataSources = m_finalizedDataSources;
            }
        };

private:
    mutable BeMutex m_mutex;

    uint64_t m_nodeIds;
    uint64_t m_datasourceIds;
    uint64_t m_hierarchyLevelIds;

    bmap<uint64_t, JsonNavNodePtr> m_nodes;    
    bmap<CombinedHierarchyLevelInfo, bvector<DataSourceInfo>> m_physicalHierarchy;
    bmap<HierarchyLevelInfo, bvector<DataSourceInfo>> m_virtualHierarchy;
    bmap<DataSourceInfo, bvector<JsonNavNode*>> m_partialHierarchies;
    bset<uint64_t> m_finalizedDataSources;
    bset<uint64_t> m_physicalNodeIds;

    INodesProviderContextFactory* m_nodesProviderContextFactory;
    IConnectionCacheCR m_connections;

    GetNodeHandler m_getNodeHandler;
    GetHierarchyDataSourceHandler m_getHierarchyDataSourceHandler;
    GetVirtualDataSourceHandler m_getVirtualDataSourceHandler;
    GetParentNodeDataSourceHandler m_getParentNodeDataSourceHandler;
    CacheHierarchyLevelHandler m_cacheHierarchyLevelHandler;
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
        return HierarchyLevelInfo(ex.GetConnectionId(), ex.GetRulesetId(), ex.GetLocale(), node.GetParentNodeId(), node.GetParentNodeId());
        }
    HierarchyLevelInfo GetHierarchyLevelInfo(DataSourceInfo const& dsInfo) const
        {
        for (auto entry : m_virtualHierarchy)
            {
            if (entry.first.GetId() == dsInfo.GetHierarchyLevelId())
                return entry.first;
            }
        ADD_FAILURE();
        return HierarchyLevelInfo();
        }
    bvector<JsonNavNode*> GetFullHierarchyLevel(bvector<DataSourceInfo> const& partialInfos) const
        {
        bvector<JsonNavNode*> vec;
        for (auto partialInfo : partialInfos)
            {
            auto iter = m_partialHierarchies.find(partialInfo);
            if (iter == m_partialHierarchies.end())
                continue;
            std::copy(iter->second.begin(), iter->second.end(), std::back_inserter(vec));
            }
        return vec;
        }

protected:
    BeMutex& _GetMutex() override { return m_mutex; }
    JsonNavNodePtr _GetNode(uint64_t nodeId) const override
        {
        BeMutexHolder lock(m_mutex);
        if (m_getNodeHandler)
            return m_getNodeHandler(nodeId);

        auto iter = m_nodes.find(nodeId);
        return (m_nodes.end() != iter) ? iter->second : nullptr;
        }
    NodeVisibility _GetNodeVisibility(uint64_t nodeId) const override
        {
        BeMutexHolder lock(m_mutex);
        return (m_physicalNodeIds.end() != m_physicalNodeIds.find(nodeId)) ? NodeVisibility::Physical : NodeVisibility::Virtual;
        }
    
    HierarchyLevelInfo _FindHierarchyLevel(Utf8CP connectionId, Utf8CP rulesetId, Utf8CP locale, uint64_t const* virtualParentNodeId) const override
        {
        BeMutexHolder lock(m_mutex);
        for (auto entry : m_virtualHierarchy)
            {
            if ((!connectionId || entry.first.GetConnectionId().Equals(connectionId))
                && (!rulesetId || entry.first.GetRulesetId().Equals(rulesetId))
                && (!locale || entry.first.GetLocale().Equals(locale))
                && (!entry.first.GetVirtualParentNodeId() && !virtualParentNodeId 
                    || entry.first.GetVirtualParentNodeId() && virtualParentNodeId && *entry.first.GetVirtualParentNodeId() == *virtualParentNodeId))
                return entry.first;
            }
        return HierarchyLevelInfo();
        }
    DataSourceInfo _FindDataSource(uint64_t hierarchyLevelId, bvector<uint64_t> const& index) const override
        {
        BeMutexHolder lock(m_mutex);
        for (auto entry : m_partialHierarchies)
            {
            if (entry.first.GetHierarchyLevelId() == hierarchyLevelId && entry.first.GetIndex() == index)
                return entry.first;
            }
        return DataSourceInfo();
        }
    DataSourceInfo _FindDataSource(uint64_t nodeId) const override
        {
        BeMutexHolder lock(m_mutex);
        for (auto entry : m_partialHierarchies)
            {
            for (JsonNavNodeCPtr node : entry.second)
                {
                if (node->GetNodeId() == nodeId)
                    return entry.first;
                }
            }
        return DataSourceInfo();
        }
    NavNodesProviderPtr _GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo const& info, bool removeIfInvalid, bool) const override
        {
        BeMutexHolder lock(m_mutex);
        if (nullptr == m_nodesProviderContextFactory)
            return nullptr;

        if (!IsInitialized(info))
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
        return (m_physicalHierarchy.end() != iter) ? BVectorNodesProvider::Create(*context, GetFullHierarchyLevel(iter->second)) : nullptr;
        }
    NavNodesProviderPtr _GetHierarchyLevel(HierarchyLevelInfo const& info, bool removeIfInvalid, bool) const override
        {
        BeMutexHolder lock(m_mutex);
        if (m_getHierarchyDataSourceHandler)
            return m_getHierarchyDataSourceHandler(info);

        if (nullptr == m_nodesProviderContextFactory)
            return nullptr;

        if (!IsInitialized(info))
            return nullptr;

        IConnectionPtr connection = m_connections.GetConnection(info.GetConnectionId().c_str());
        if (connection.IsNull())
            {
            BeAssert(false);
            return nullptr;
            }

        NavNodesProviderContextPtr context = m_nodesProviderContextFactory->Create(*connection, info.GetRulesetId().c_str(),
            info.GetLocale().c_str(), info.GetPhysicalParentNodeId());
        auto iter = m_virtualHierarchy.find(info);
        return (m_virtualHierarchy.end() != iter) ? BVectorNodesProvider::Create(*context, GetFullHierarchyLevel(iter->second)) : nullptr;
        }
    NavNodesProviderPtr _GetDataSource(DataSourceInfo const& dsInfo, bool removeIfInvalid, bool) const override
        {
        BeMutexHolder lock(m_mutex);
        if (m_getVirtualDataSourceHandler)
            return m_getVirtualDataSourceHandler(dsInfo);

        if (nullptr == m_nodesProviderContextFactory)
            return nullptr;
        
        HierarchyLevelInfo hlInfo = GetHierarchyLevelInfo(dsInfo);
        IConnectionPtr connection = m_connections.GetConnection(hlInfo.GetConnectionId().c_str());
        if (connection.IsNull())
            {
            BeAssert(false);
            return nullptr;
            }

        NavNodesProviderContextPtr context = m_nodesProviderContextFactory->Create(*connection, hlInfo.GetRulesetId().c_str(),
            hlInfo.GetLocale().c_str(), hlInfo.GetVirtualParentNodeId());
        auto iter = m_partialHierarchies.find(dsInfo);
        return (m_partialHierarchies.end() != iter) ? BVectorNodesProvider::Create(*context, iter->second) : nullptr;
        }
    NavNodesProviderPtr _GetDataSource(uint64_t nodeId, bool removeIfInvalid, bool) const override
        {
        BeMutexHolder lock(m_mutex);
        if (m_getParentNodeDataSourceHandler)
            return m_getParentNodeDataSourceHandler(nodeId);

        JsonNavNodePtr node = GetNode(nodeId);
        for (auto entry : m_partialHierarchies)
            {
            for (JsonNavNodePtr const& entryNode : entry.second)
                {
                if (entryNode == node)
                    return GetDataSource(entry.first, removeIfInvalid);
                }
            }
        return nullptr;
        }
    
    void _Cache(HierarchyLevelInfo& info) override
        {
        BeMutexHolder lock(m_mutex);
        info.SetId(++m_hierarchyLevelIds);
        m_physicalHierarchy[info] = bvector<DataSourceInfo>();
        m_virtualHierarchy[info] = bvector<DataSourceInfo>();

        if (m_cacheHierarchyLevelHandler)
            m_cacheHierarchyLevelHandler(info);
        }
    void _Cache(DataSourceInfo& info, DataSourceFilter const& filter, bmap<ECClassId, bool> const& relatedClassIds, bvector<UserSettingEntry> const& relatedSettings) override
        {
        BeMutexHolder lock(m_mutex);
        info.SetId(++m_datasourceIds);
        m_partialHierarchies[info] = bvector<JsonNavNode*>();
        bvector<DataSourceInfo>& physicalHierarchy = m_physicalHierarchy[GetHierarchyLevelInfo(info)];
        physicalHierarchy.insert(physicalHierarchy.begin() + info.GetIndex().back(), info);
        bvector<DataSourceInfo>& virtualHierarchy = m_virtualHierarchy[GetHierarchyLevelInfo(info)];
        virtualHierarchy.insert(virtualHierarchy.begin() + info.GetIndex().back(), info);

        if (m_cacheDataSourceHandler)
            m_cacheDataSourceHandler(info, filter, relatedClassIds, relatedSettings);
        }
    void _Cache(JsonNavNodeR node, DataSourceInfo const& dsInfo, uint64_t index, bool isVirtual) override
        {
        BeMutexHolder lock(m_mutex);
        uint64_t nodeId = ++m_nodeIds;
        node.SetNodeId(nodeId);
        
        HierarchyLevelInfo hlInfo = GetHierarchyLevelInfo(dsInfo);
        uint64_t const* parentId = hlInfo.GetVirtualParentNodeId();
        bvector<Utf8String> pathFromRoot = (nullptr == parentId || 0 == *parentId) ? bvector<Utf8String>() : m_nodes[*parentId]->GetKey()->GetPathFromRoot();
        pathFromRoot.push_back(std::to_string(nodeId).c_str());
        IConnectionPtr connection = m_connections.GetConnection(hlInfo.GetConnectionId().c_str());
        node.SetNodeKey(*NavNodesHelper::CreateNodeKey(*connection, node, pathFromRoot));

        m_nodes[nodeId] = &node;
        m_partialHierarchies[dsInfo].push_back(&node);

        if (!isVirtual)
            m_physicalNodeIds.insert(node.GetNodeId());

        if (m_cacheNodeHandler)
            m_cacheNodeHandler(node, isVirtual);
        }

    void _MakePhysical(JsonNavNodeCR node) override
        {
        BeMutexHolder lock(m_mutex);
        m_physicalNodeIds.insert(node.GetNodeId());
        if (m_makePhysicalHandler)
            m_makePhysicalHandler(node);
        }
    void _MakeVirtual(JsonNavNodeCR node) override
        {
        BeMutexHolder lock(m_mutex);
        m_physicalNodeIds.erase(node.GetNodeId());
        if (m_makeVirtualHandler)
            return m_makeVirtualHandler(node);
        }
    
    bool _IsInitialized(CombinedHierarchyLevelInfo const& info) const override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_physicalHierarchy.find(info);
        if (m_physicalHierarchy.end() == iter)
            return false;
        for (auto dsInfo : iter->second)
            {
            if (!IsInitialized(dsInfo))
                return false;
            }
        return true;
        }
    bool _IsInitialized(HierarchyLevelInfo const& info) const override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_virtualHierarchy.find(info);
        if (m_virtualHierarchy.end() == iter)
            return false;
        for (auto dsInfo : iter->second)
            {
            if (!IsInitialized(dsInfo))
                return false;
            }
        return true;
        }
    bool _IsInitialized(DataSourceInfo const& info) const override 
        {
        BeMutexHolder lock(m_mutex); 
        return m_finalizedDataSources.end() != m_finalizedDataSources.find(info.GetId());
        }
    void _FinalizeInitialization(DataSourceInfo const& info) override 
        {
        BeMutexHolder lock(m_mutex); 
        m_finalizedDataSources.insert(info.GetId());
        }

    void _Update(uint64_t id, JsonNavNodeCR node) override
        {
        BeMutexHolder lock(m_mutex);
        if (m_updateNodeHandler)
            return m_updateNodeHandler(id, node);
        }
    void _Update(DataSourceInfo const& info, DataSourceFilter const* filter, bmap<ECClassId, bool> const* relatedClassIds, bvector<UserSettingEntry> const* relatedSettings) override
        {
        BeMutexHolder lock(m_mutex);
        if (m_updateDataSourceHandler)
            return m_updateDataSourceHandler(info, filter, relatedClassIds, relatedSettings);
        }

    JsonNavNodeCPtr _LocateNode(IConnectionCR connection, Utf8StringCR locale, NavNodeKeyCR key) const override
        {
        BeMutexHolder lock(m_mutex);
        if (m_locateNodeHandler)
            return m_locateNodeHandler(connection, locale, key);
        return nullptr;
        }
    IHierarchyCache::SavepointPtr _CreateSavepoint() override {return new Savepoint(*this);}

public:
    TestNodesCache(IConnectionCacheCR connections, INodesProviderContextFactory* nodesProviderContextFactory = nullptr) 
        : m_nodesProviderContextFactory(nodesProviderContextFactory), m_connections(connections), m_nodeIds(1), m_datasourceIds(1), m_hierarchyLevelIds(1)
        {}
    void SetGetNodeHandler(GetNodeHandler handler) {m_getNodeHandler = handler;}
    void SetGetHierarchyDataSourceHandler(GetHierarchyDataSourceHandler handler) {m_getHierarchyDataSourceHandler = handler;}
    void SetGetVirtualDataSourceHandler(GetVirtualDataSourceHandler handler) {m_getVirtualDataSourceHandler = handler;}
    void SetGetParentNodeDataSourceHandler(GetParentNodeDataSourceHandler handler) {m_getParentNodeDataSourceHandler = handler;}
    void SetCacheHierarchyLevelHandler(CacheHierarchyLevelHandler handler) {m_cacheHierarchyLevelHandler = handler;}
    void SetCacheDataSourceHandler(CacheDataSourceHandler handler) {m_cacheDataSourceHandler = handler;}
    void SetCacheNodeHandler(CacheNodeHandler handler) {m_cacheNodeHandler = handler;}
    void SetMakePhysicalHandler(MakePhysicalHandler handler) {m_makePhysicalHandler = handler;}
    void SetMakeVirtualHandler(MakeVirtualHandler handler) {m_makeVirtualHandler = handler;}
    void SetUpdateNodeHandler(UpdateNodeHandler handler) {m_updateNodeHandler = handler;}
    void SetUpdateDataSourceHandler(UpdateDataSourceHandler handler) {m_updateDataSourceHandler = handler;}
    void SetLocateNodeHandler(LocateNodeHandler handler) {m_locateNodeHandler = handler;}
    size_t GetCachedChildrenCount(uint64_t parentId) 
        {
        BeMutexHolder lock(m_mutex);
        return std::count_if(m_nodes.begin(), m_nodes.end(), 
            [&](bpair<uint64_t, JsonNavNodePtr> nodePair) 
            {
            return parentId == nodePair.second->GetParentNodeId(); 
            }); 
        }
};

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis                01/2017
//=======================================================================================
struct TestCategorySupplier : IPropertyCategorySupplier
    {
    ContentDescriptor::Category m_category;
    TestCategorySupplier()
        : m_category(ContentDescriptor::Category::GetDefaultCategory())
        {
        }
    TestCategorySupplier(ContentDescriptor::Category category)
        : m_category(category)
        {
        }
    virtual ContentDescriptor::Category _GetECClassCategory(ECClassCR) const override { return m_category; }
    virtual ContentDescriptor::Category _GetRelatedECClassCategory(ECClassCR, RelationshipMeaning) const override { return m_category; }
    virtual ContentDescriptor::Category _GetPropertyCategory(ECPropertyCR) const override { return m_category; }
    virtual ContentDescriptor::Category _CreateCategory(ECClassCR, ECPropertyCR, RelationshipMeaning) const override { return m_category; }
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
    TestPropertyFormatter() {}
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
        RulesPreprocessor preprocessor(context.GetConnections(), context.GetConnection(), context.GetRuleset(),
            context.GetLocale(), context.GetUserSettings(), nullptr, context.GetECExpressionsCache());
        if (nullptr == parent)
            {
            RulesPreprocessor::RootNodeRuleParameters params(TargetTree_MainTree);
            RootNodeRuleSpecificationsList specs = preprocessor.GetRootNodeSpecifications(params);
            provider = MultiSpecificationNodesProvider::Create(context, specs);
            }
        else
            {
            RulesPreprocessor::ChildNodeRuleParameters params(*parent, TargetTree_MainTree);
            ChildNodeRuleSpecificationsList specs = preprocessor.GetChildNodeSpecifications(params);
            provider = MultiSpecificationNodesProvider::Create(context, specs, *parent);
            }
        return provider;
        }

protected:
    NavNodesProviderPtr _Create(NavNodesProviderContextR context, JsonNavNodeCP parent, ProviderCacheType) const override
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
    mutable CustomFunctionsInjector m_customFunctions;
    mutable TestNodesCache m_testNodesCache;
    mutable IHierarchyCacheP m_nodesCache;

private:
    IHierarchyCacheR GetNodesCache() const {return (nullptr != m_nodesCache) ? *m_nodesCache : m_testNodesCache;}

protected:
    NavNodesProviderContextPtr _Create(IConnectionCR connection, Utf8CP rulesetId, Utf8CP locale, 
        uint64_t const* parentNodeId, ICancelationTokenCP cancelationToken, size_t pageSize) const override
        {
        PresentationRuleSetCPtr ruleset = m_ruleset;
        if (ruleset.IsNull())
            ruleset = PresentationRuleSet::CreateInstance(rulesetId, 1, 0, false, "", "", "", false);
        NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*ruleset, TargetTree_MainTree, locale, parentNodeId, 
            m_settings, m_ecexpressionsCache, m_relatedPathsCache, m_polymorphicallyRelatedClassesCache, m_nodesFactory, 
            GetNodesCache(), m_providerFactory, nullptr);
        context->SetQueryContext(m_connections, connection, m_usedClassesListener);
        context->SetCancelationToken(cancelationToken);
        if (-1 != pageSize)
            context->SetPageSize(pageSize);
        return context;
        }

public:
    TestNodesProviderContextFactory(IConnectionManagerCR connections) 
        : m_connections(connections), m_testNodesCache(connections), m_nodesCache(nullptr), m_customFunctions(connections)
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
    virtual void _OnRulesetDispose(RuleSetLocaterCR, PresentationRuleSetR ruleset) override
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
