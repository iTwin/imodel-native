/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/TestHelpers.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
#include "ECDbTestProject.h"
#include "TestNavNode.h"

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

    static IECInstancePtr InsertInstance(ECDbTestProject& project, ECClassCR ecClass, std::function<void(IECInstanceR)> const& instancePreparer = nullptr);
    static IECInstancePtr InsertInstance(ECDbTestProject& project, ECInstanceInserter& inserter, ECClassCR ecClass, std::function<void(IECInstanceR)> const& instancePreparer = nullptr);
    static ECInstanceKey InsertRelationship(ECDbR db, ECRelationshipClassCR relationship, IECInstanceCR source, IECInstanceR target, std::function<void(IECInstanceR)> const& instancePreparer = nullptr);
    static ECInstanceKey InsertRelationship(ECDbTestProject& project, ECRelationshipClassCR relationship, IECInstanceCR source, IECInstanceR target, std::function<void(IECInstanceR)> const& instancePreparer = nullptr);
    static void DeleteInstances(ECDbTestProject& project, ECClassCR ecClass, bool polymorphic = false);
    static void DeleteInstance(ECDbR db, ECInstanceKeyCR key);
    static void DeleteInstance(ECDbR db, IECInstanceCR instance);
    static void DeleteInstance(ECDbTestProject& project, ECInstanceKeyCR key);
    static void DeleteInstance(ECDbTestProject& project, IECInstanceCR instance);
    static IECInstancePtr GetInstance(ECDbTestProject& project, ECClassCR ecClass, ECInstanceId id);
    static ECInstanceKey GetInstanceKey(IECInstanceCR);

    static NavigationQueryPtr CreateECInstanceNodesQueryForClasses(ECClassSet const& classes, Utf8CP alias, ComplexQueryHandler handler = nullptr);
    static ComplexNavigationQueryPtr CreateECInstanceNodesQueryForClass(ECEntityClassCR ecClass, bool polymorphic, Utf8CP alias, bvector<RelatedClass> const& = bvector<RelatedClass>());
    static NavigationQueryPtr CreateLabelGroupingNodesQueryForClasses(ECClassSet const& classes, Utf8CP alias, ComplexQueryHandler handler = nullptr);
    static NavigationQueryPtr CreateQuery(NavigationQueryContract const&, bset<ECN::ECClassCP>, bool polymorphic, Utf8CP alias, ComplexQueryHandler handler = nullptr);
    static NavigationQueryPtr CreateQuery(NavigationQueryContract const&, bvector<ECN::ECClassCP>, bool polymorphic, Utf8CP alias, ComplexQueryHandler handler = nullptr);
    
    static void ValidateContentSetItem(ECN::IECInstanceCR instance, ContentSetItemCR item, ContentDescriptorCR descriptor, Utf8CP expectedLabel = nullptr, Utf8CP expectedImageId = nullptr);
    static void ValidateContentSet(bvector<ECN::IECInstanceCP> const& instances, Content const& content, bool validateOrder = false);

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
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
struct TestNodeLocater : INavNodeLocater
{
private:
    bmap<NavNodeKeyCP, NavNodeCPtr, NavNodeKeyPtrComparer> m_nodes;

protected:
    NavNodeCPtr _LocateNode(NavNodeKeyCR key) const override
        {
        auto iter = m_nodes.find(&key);
        if (m_nodes.end() != iter)
            return iter->second;
        return nullptr;
        }

public:
    TestNodeLocater() {}
    TestNodeLocater(NavNodeCR node) {AddNode(node);}
    void AddNode(NavNodeCR node) {m_nodes[&node.GetKey()] = &node;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct TestNodesCache : IHierarchyCache, INavNodeLocater
{
    /*=================================================================================**//**
    * @bsiclass                                     Grigas.Petraitis                02/2017
    +===============+===============+===============+===============+===============+======*/
    struct HierarchyItem
    {
    private:
        JsonNavNodePtr m_node;
        bool m_isVirtual;
        NavNodesProviderPtr m_childrenDataSource;

    public:
        HierarchyItem() : m_node(nullptr), m_isVirtual(false) {}
        HierarchyItem(JsonNavNodeR node, bool isVirtual = false) : m_node(&node), m_isVirtual(isVirtual) {}
        HierarchyItem(JsonNavNodeR node, NavNodesProviderR source) : m_node(&node), m_isVirtual(false), m_childrenDataSource(&source) {}
        ~HierarchyItem() {m_childrenDataSource = nullptr; m_node = nullptr;}
        JsonNavNodeR GetNode() const {return *m_node;}
        NavNodesProviderPtr GetDataSource() const {return m_childrenDataSource;}
        bool IsVirtual() const {return m_isVirtual;}
        void SetIsVirtual(bool value) {m_isVirtual = value;}
        void SetNode(JsonNavNodeR node) {m_node = &node;}
        void SetDataSource(NavNodesProviderR source) {m_childrenDataSource = &source;}
    };
    
    /*=================================================================================**//**
    * @bsiclass                                     Grigas.Petraitis                02/2017
    +===============+===============+===============+===============+===============+======*/
    struct RootKey
        {
        BeGuid m_guid;
        Utf8String m_rulesetId;
        RootKey() {}
        RootKey(ECDbCR db, Utf8String rulesetId) : m_guid(db.GetDbGuid()), m_rulesetId(rulesetId) {}
        RootKey(BeGuid guid, Utf8String rulesetId) : m_guid(guid), m_rulesetId(rulesetId) {}
        bool operator<(RootKey const& other) const
            {
            return m_guid < other.m_guid || m_guid == other.m_guid && m_rulesetId < other.m_rulesetId;
            }
        };

private:
    bmap<uint64_t, HierarchyItem> m_hierarchy;
    bmap<NavNodeKeyCP, uint64_t, NavNodeKeyPtrComparer> m_nodeIdsByKey;
    bmap<RootKey, NavNodesProviderPtr> m_rootDataSources;
    bool m_inCleanup;

private:
    void Clear()
        {
        m_inCleanup = true;
        m_nodeIdsByKey.clear();
        m_hierarchy.clear();
        m_rootDataSources.clear();
        m_inCleanup = false;
        }

protected:
    JsonNavNodePtr _GetNode(uint64_t nodeId, NodeVisibility visibility) const override
        {
        auto iter = m_hierarchy.find(nodeId);
        if (m_hierarchy.end() == iter)
            return nullptr;
        if (iter->second.IsVirtual() && NodeVisibility::Physical == visibility)
            return nullptr;
        if (!iter->second.IsVirtual() && NodeVisibility::Virtual == visibility)
            return nullptr;
        return &iter->second.GetNode();
        }
    NavNodesProviderPtr _GetDataSource(HierarchyLevelInfo const& info) const override
        {
        if (nullptr != info.GetPhysicalParentNodeId())
            {
            auto iter = m_hierarchy.find(*info.GetPhysicalParentNodeId());
            if (m_hierarchy.end() != iter)
                return iter->second.GetDataSource();
            }
        else
            {
            auto iter = m_rootDataSources.find(RootKey(info.GetConnectionId(), info.GetRulesetId()));
            if (m_rootDataSources.end() != iter)
                return iter->second;
            }
        return nullptr;
        }
    NavNodesProviderPtr _GetDataSource(DataSourceInfo const& info) const override
        {
        if (nullptr != info.GetVirtualParentNodeId())
            {
            auto iter = m_hierarchy.find(*info.GetVirtualParentNodeId());
            if (m_hierarchy.end() != iter)
                return iter->second.GetDataSource();
            }
        else
            {
            auto iter = m_rootDataSources.find(RootKey(info.GetConnectionId(), info.GetRulesetId()));
            if (m_rootDataSources.end() != iter)
                return iter->second;
            }
        return nullptr;
        }
    NavNodesProviderPtr _GetDataSource(uint64_t nodeId) const override
        {
        auto nodeIter = m_hierarchy.find(nodeId);
        if (m_hierarchy.end() == nodeIter)
            return nullptr;

        HierarchyItem const& nodeHierarchy = nodeIter->second;
        NavNodesProviderContextCR context = nodeHierarchy.GetDataSource()->GetContext();    
        return GetDataSource(HierarchyLevelInfo(context.GetDb().GetDbGuid(), context.GetRuleset().GetRuleSetId(), nodeHierarchy.GetNode().GetParentNodeId()));
        }
    
    void _Cache(DataSourceInfo& info, DataSourceFilter const&, bvector<ECClassId> const&, bvector<Utf8String> const&, bool) override
        {
        }
    void _Cache(JsonNavNodeR node, bool isVirtual) override
        {
        node.SetNodeId(TestNodesHelper::CreateNodeId());
        m_hierarchy[node.GetNodeId()] = HierarchyItem(node, isVirtual);
        m_nodeIdsByKey[&node.GetKey()] = node.GetNodeId();
        }

    void _MakePhysical(JsonNavNodeCR node) override
        {
        auto iter = m_hierarchy.find(node.GetNodeId());
        if (m_hierarchy.end() != iter)
            iter->second.SetIsVirtual(false);
        }
    void _MakeVirtual(JsonNavNodeCR node) override
        {
        auto iter = m_hierarchy.find(node.GetNodeId());
        if (m_hierarchy.end() != iter)
            iter->second.SetIsVirtual(true);
        }
    void _Update(uint64_t id, JsonNavNodeCR node) override
        {
        JsonNavNodeR tempNode = const_cast<JsonNavNodeR>(node);
        auto iter = m_hierarchy.find(id);
        if (m_hierarchy.end() != iter)
            iter->second.SetNode(tempNode);
        }
    void _Update(DataSourceInfo const&, DataSourceFilter const*, bvector<ECN::ECClassId> const*, bvector<Utf8String> const*) override
        {
        }

    NavNodeCPtr _LocateNode(NavNodeKeyCR key) const override
        {
        auto iter = m_nodeIdsByKey.find(&key);
        if (m_nodeIdsByKey.end() != iter)
            return GetNode(iter->second, NodeVisibility::Physical);
        return nullptr;
        }

public:
    TestNodesCache() : m_inCleanup(false) {}
    ~TestNodesCache() {Clear();}
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
    ContentDescriptor::Category _GetCategory(ECClassCR, RelatedClassPathCR, ECPropertyCR) override {return m_category;}
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
    void _OnClassUsed(ECDbCR connection, ECN::ECClassCR ecClass, bool polymorphically) override
        {
        m_usedClasses[&ecClass] = polymorphically;
        }
    bmap<ECClassCP, bool> const& GetUsedClasses() const {return m_usedClasses;}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct TestUserSettings : IUserSettings
{
private:
    Json::Value m_values;
    IUserSettingsChangeListener* m_changesListener;

    void NotifySettingChanged(Utf8CP settingId)
        {
        if (nullptr != m_changesListener)
            m_changesListener->_OnSettingChanged("", settingId);
        }
    
protected:
    Json::Value _GetPresentationInfo() const override {return Json::Value();}

    bool _HasSetting(Utf8CP id) const override {return m_values.isMember(id);}

    void _SetSettingValue(Utf8CP id, Utf8CP value) override {m_values[id] = value; NotifySettingChanged(id);}
    void _SetSettingBoolValue(Utf8CP id, bool value) override {m_values[id] = value; NotifySettingChanged(id);}
    void _SetSettingIntValue(Utf8CP id, int64_t value) override {m_values[id] = value; NotifySettingChanged(id);}
    void _SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values) override
        {
        for (int64_t v : values)
            m_values[id].append(v);
        NotifySettingChanged(id);
        }    

    Utf8String _GetSettingValue(Utf8CP id) const override {return m_values.isMember(id) ? m_values[id].asCString() : "";}
    bool _GetSettingBoolValue(Utf8CP id) const override {return m_values.isMember(id) ? m_values[id].asBool() : false;}
    int64_t _GetSettingIntValue(Utf8CP id) const override {return m_values.isMember(id) ? m_values[id].asInt64() : 0;}
    bvector<int64_t> _GetSettingIntValues(Utf8CP id) const override
        {
        bvector<int64_t> values;
        if (m_values.isMember(id))
            {
            JsonValueCR jsonArr = m_values[id];
            for (Json::ArrayIndex i = 0; i < jsonArr.size(); ++i)
                values.push_back(jsonArr[i].asInt64());
            }
        return values;
        }    

public:
    TestUserSettings() : m_changesListener(nullptr) {}
    void SetChangesListener(IUserSettingsChangeListener* listener) {m_changesListener = listener;}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct TestNodesProviderFactory : INodesProviderFactory
{
private:

private:
    NavNodesProviderPtr CreateProvider(NavNodesProviderContextR context, NavNodeCP parent) const
        {
        NavNodesProviderPtr provider;
        if (nullptr == parent)
            {
            RulesPreprocessor::RootNodeRuleParameters params(context.GetDb(), context.GetRuleset(), TargetTree_MainTree,
                context.GetUserSettings(), nullptr, context.GetECExpressionsCache());
            RootNodeRuleSpecificationsList specs = RulesPreprocessor::GetRootNodeSpecifications(params);
            provider = MultiSpecificationNodesProvider::Create(context, specs);
            }
        else
            {
            RulesPreprocessor::ChildNodeRuleParameters params(context.GetDb(), *parent, context.GetRuleset(), TargetTree_MainTree, 
                context.GetUserSettings(), nullptr, context.GetECExpressionsCache());
            ChildNodeRuleSpecificationsList specs = RulesPreprocessor::GetChildNodeSpecifications(params);
            provider = MultiSpecificationNodesProvider::Create(context, specs, *parent);
            }
        return provider;
        }

protected:
    NavNodesProviderPtr _CreateForHierarchyLevel(NavNodesProviderContextR context, NavNodeCP parent) const override
        {
        return CreateProvider(context, parent);
        }
    NavNodesProviderPtr _CreateForVirtualParent(NavNodesProviderContextR context, NavNodeCP parent) const override
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
    TestUserSettings m_settings;
    JsonNavNodesFactory m_nodesFactory;
    TestNodesProviderFactory m_providerFactory;
    mutable ECExpressionsCache m_ecexpressionsCache;
    mutable RelatedPathsCache m_relatedPathsCache;
    mutable ECSqlStatementCache m_statementsCache;
    mutable CustomFunctionsInjector m_customFunctions;
    mutable TestNodesCache m_testNodesCache;
    mutable IHierarchyCacheP m_nodesCache;

private:
    IHierarchyCacheR GetNodesCache() const
        {
        return (nullptr != m_nodesCache) ? *m_nodesCache : m_testNodesCache;
        }

protected:
    NavNodesProviderContextPtr _Create(ECDbCR connection, Utf8CP rulesetId, uint64_t const* parentNodeId, bool disableUpdates) const override
        {
        PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(rulesetId, 1, 0, false, "", "", "", false);
        NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*ruleset, true, TargetTree_MainTree, parentNodeId, 
            m_settings, m_ecexpressionsCache, m_relatedPathsCache, m_nodesFactory, GetNodesCache(), m_providerFactory, nullptr);
        context->SetQueryContext(connection, m_statementsCache, m_customFunctions, nullptr);
        context->SetIsUpdatesDisabled(disableUpdates);
        return context;
        }

public:
    TestNodesProviderContextFactory() : m_statementsCache(10) {}
    void SetNodesCache(IHierarchyCacheP cache) {m_nodesCache = cache;}
};

END_ECPRESENTATIONTESTS_NAMESPACE
