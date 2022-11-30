/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationManager.h>
#include "../../../Source/Shared/Queries/QueryExecutor.h"
#include "../../../Source/Shared/Queries/QueryContracts.h"
#include "../../../Source/Shared/Queries/QueryBuilder.h"
#include "../../../Source/Shared/Queries/QueryBuilderHelpers.h"
#include "../../../Source/Shared/ECSchemaHelper.h"
#include "../../../Source/Hierarchies/NavNodeProviders.h"
#include "../../../Source/Hierarchies/NavNodesCache.h"
#include <UnitTests/ECPresentation/ECPresentationTest.h>
#include <UnitTests/ECPresentation/TestConnectionCache.h>
#include <UnitTests/ECPresentation/TestUserSettings.h>
#include <UnitTests/ECPresentation/TestRuleSetLocater.h>
#include "ECDbTestProject.h"
#include "TestNavNode.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

//#define USE_HYBRID_CACHE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TRegistry> struct RegisterSchemaHelper
    {
    RegisterSchemaHelper(Utf8String name, Utf8String schemaXml) {TRegistry::RegisterSchemaXml(name, schemaXml);}
    };
// add this to class declaration
#define DECLARE_SCHEMA_REGISTRY(registry) \
    static bvector<bpair<Utf8String, Utf8String>>& GetRegisteredSchemaXmls(); \
    static void RegisterSchemaXml(Utf8String name, Utf8String schemaXml);
// add this to source file (registry = test class name)
#define DEFINE_SCHEMA_REGISTRY(registry) \
    bvector<bpair<Utf8String, Utf8String>>& registry::GetRegisteredSchemaXmls() \
        { \
        static bvector<bpair<Utf8String, Utf8String>> s_registeredSchemaXmls; \
        return s_registeredSchemaXmls; \
        } \
    void registry::RegisterSchemaXml(Utf8String name, Utf8String schemaXml) {GetRegisteredSchemaXmls().push_back(bpair<Utf8String, Utf8String>(name, schemaXml));}
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct InstanceInputAndResult
{
    bvector<IECInstanceCP> m_inputs;
    IECInstanceCP m_result;
    InstanceInputAndResult() : m_result(nullptr) {}
    InstanceInputAndResult(IECInstanceCP input, IECInstanceCP result) : m_result(result) {if (input) { m_inputs.push_back(input); }}
    InstanceInputAndResult(IECInstanceCR input, IECInstanceCR result) : m_result(&result) {m_inputs.push_back(&input);}
    InstanceInputAndResult(bvector<IECInstanceCP> inputs, IECInstanceCR result) : m_result(&result), m_inputs(inputs) {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesEngineTestHelpers
    {
    typedef std::function<void(ComplexNavigationQueryR)> ComplexQueryHandler;

    static void InitSchemaRegistry(ECDbR ecdb, bvector<bpair<Utf8String, Utf8String>> const& schemas);

    static ECPresentationManager::Paths GetPaths(BeTest::Host&);

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
    static IECInstancePtr GetInstance(ECDbR project, ECClassCR ecClass, ECInstanceId id);
    static ECClassInstanceKey GetInstanceKey(IECInstanceCR);

    static PresentationQueryContractFieldPtr CreateDisplayLabelField(ECSchemaHelper const&, SelectClass<ECClass> const&,
        bvector<RelatedClassPath> const& = {}, bvector<InstanceLabelOverrideValueSpecification const*> const& = {});
    static PresentationQueryContractFieldPtr CreateNullDisplayLabelField();
    static ComplexNavigationQueryPtr CreateMultiECInstanceNodesQuery(ECClassCR ecClass, NavigationQueryR instanceNodesQuery);
    static NavigationQueryPtr CreateECInstanceNodesQueryForClasses(ECSchemaHelper const&, ECClassSet const& classes, Utf8CP alias, ComplexQueryHandler handler = nullptr);
    static ComplexNavigationQueryPtr CreateECInstanceNodesQueryForClass(ECSchemaHelper const&, SelectClass<ECClass> const& selectClass, bvector<RelatedClassPath> const& = bvector<RelatedClassPath>());
    static NavigationQueryPtr CreateQuery(NavigationQueryContract const&, bset<ECN::ECClassCP>, bool polymorphic, Utf8CP alias, ComplexQueryHandler handler = nullptr);
    static NavigationQueryPtr CreateQuery(NavigationQueryContract const&, bvector<ECN::ECClassCP>, bool polymorphic, Utf8CP alias, ComplexQueryHandler handler = nullptr);

    static void ValidateContentSetItem(ECN::IECInstanceCR instance, ContentSetItemCR item, ContentDescriptorCR descriptor, Utf8CP expectedLabel = nullptr, Utf8CP expectedImageId = nullptr);
    static void ValidateContentSet(bvector<ECN::IECInstanceCP> instances, Content const& content, bool validateOrder = false);
    static void ValidateContentSet(bvector<InstanceInputAndResult> instances, Content const& content, bool validateOrder = false);
    static void ValidateNodesPagination(std::function<NodesResponse(PageOptionsCR)> getter, bvector<NavNodeCPtr> const& expectedNodes);
    static void ValidateNodeInstances(ECDbCR, NavNodeCR node, bvector<RefCountedPtr<IECInstance const>> const& instances);
    static void ValidateNodeInstances(INodeInstanceKeysProvider const&, NavNodeCR node, bvector<RefCountedPtr<IECInstance const>> const& instances);
    static void ValidateNodeGroupedValues(NavNodeCR node, bvector<ECValue> const& groupedValues);
    static NavNodesContainer GetValidatedNodes(std::function<NodesResponse()> getter);
    static NavNodesContainer GetValidatedNodes(std::function<NodesResponse(PageOptionsCR)> nodesGetter, std::function<NodesCountResponse()> countGetter);

    static ContentDescriptor::Field& AddField(ContentDescriptorR, ContentDescriptor::Field&);
    static ContentDescriptor::Field& AddField(ContentDescriptorR, ECN::ECClassCR, ContentDescriptor::Property, IPropertyCategorySupplierR);

    static void CacheNode(IHierarchyCacheR cache, NavNodeR node);

    static void ImportSchema(ECDbR, std::function<void(ECSchemaR)> const& schemaBuilder);
    static bvector<ECEntityClassP> CreateNDerivedClasses(ECSchemaR schema, ECEntityClassCR baseClass, int numberOfChildClasses);

    static Utf8String SerializeClassNames(bvector<ECClassCP> const& ecClasses)
        {
        Utf8String str;
        bool first = true;
        for (ECClassCP ecClass : ecClasses)
            {
            if (!first)
                str.append("_");
            str.append(Utf8String(ecClass->GetSchema().GetAlias()).append("_").append(ecClass->GetName()));
            first = false;
            }
        return str;
        }
    static Utf8String CreateFieldName(bvector<ECClassCP> const& ecClasses, Utf8CP propertyName, uint64_t counter = 0)
        {
        Utf8String name("pc_");
        name.append(SerializeClassNames(ecClasses));
        name.append("_").append(propertyName);
        if (counter != 0)
            name.append("_").append(std::to_string(counter));
        return name;
        }
    static Utf8String CreateFieldName(ECClassCP ecClass, Utf8CP propertyName, uint64_t counter = 0)
        {
        return CreateFieldName(bvector<ECClassCP>{ecClass}, propertyName, counter);
        }
    static Utf8String CreateRelatedFieldName(bvector<ECClassCP> relatedClasses, bvector<ECClassCP> propertyClasses, Utf8CP propertyName)
        {
        Utf8String name;
        name.append("rc_").append(SerializeClassNames(relatedClasses));
        name.append("_pc_").append(SerializeClassNames(propertyClasses));
        name.append("_").append(propertyName);
        return name;
        }
    static Utf8String CreateRelatedFieldName(bvector<ECClassCP> relatedClasses, ECClassCP propertyClass, Utf8CP propertyName)
        {
        return CreateRelatedFieldName(relatedClasses, bvector<ECClassCP>{propertyClass}, propertyName);
        }
    static Utf8String CreateRelatedFieldName(ECClassCP relatedClass, ECClassCP propertyClass, Utf8CP propertyName)
        {
        return CreateRelatedFieldName(bvector<ECClassCP>{relatedClass}, bvector<ECClassCP>{propertyClass}, propertyName);
        }
    static Utf8String CreateNestedContentFieldName(bvector<ECClassCP> relationshipClasses, ECClassCP nestedContentClass, uint64_t counter = 0)
        {
        Utf8String name;
        name.append("rc_").append(SerializeClassNames(relationshipClasses));
        name.append("_ncc_").append(Utf8String(nestedContentClass->GetSchema().GetAlias()).append("_").append(nestedContentClass->GetName()));
        if (counter != 0)
            name.append("_").append(std::to_string(counter));
        return name;
        }
    static Utf8String CreateNestedContentFieldName(ECClassCP relationshipClass, ECClassCP nestedContentClass, uint64_t counter = 0)
        {
        return CreateNestedContentFieldName(bvector<ECClassCP>{relationshipClass}, nestedContentClass, counter);
        }
    static Utf8String CreateDisplayLabelValueClause(Utf8CP fieldName)
        {
        return Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetLabelDefinitionDisplayValue, fieldName);
        }

    static Utf8String CreateClassNamesList(bvector<ECClassCP> const& classes);
    };
#define FIELD_NAME(ecClass, propertyName) RulesEngineTestHelpers::CreateFieldName(ecClass, propertyName).c_str()
#define FIELD_NAME_C(ecClass, propertyName, counter) RulesEngineTestHelpers::CreateFieldName(ecClass, propertyName, counter).c_str()
#define RELATED_FIELD_NAME(relatedClass, propertyClass, propertyName) RulesEngineTestHelpers::CreateRelatedFieldName(relatedClass, propertyClass, propertyName).c_str()
#define NESTED_CONTENT_FIELD_NAME(selectClass, nestedContentClass) RulesEngineTestHelpers::CreateNestedContentFieldName(selectClass, nestedContentClass).c_str()
#define NESTED_CONTENT_FIELD_NAME_C(selectClass, nestedContentClass, counter) RulesEngineTestHelpers::CreateNestedContentFieldName(selectClass, nestedContentClass, counter).c_str()

/*=================================================================================**//**
* @bsiclass
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestNodeLocater : INavNodeLocater
{
private:
    bmap<NavNodeKeyCP, NavNodeCPtr, NavNodeKeyPtrComparer> m_nodes;

protected:
    NavNodeCPtr _LocateNode(IConnectionCR, Utf8StringCR, NavNodeKeyCR key, RulesetVariables const&) const override
        {
        auto iter = m_nodes.find(&key);
        if (m_nodes.end() != iter)
            return iter->second;
        return nullptr;
        }

public:
    TestNodeLocater() {}
    TestNodeLocater(NavNodeCR node) {AddNode(node);}
    void AddNode(NavNodeCR node) {m_nodes[node.GetKey().get()] = &node;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TestCategorySupplier : IPropertyCategorySupplier
    {
    std::unique_ptr<ContentDescriptor::Category> m_category;
    TestCategorySupplier()
        : m_category(std::make_unique<ContentDescriptor::Category>())
        {
        }
    TestCategorySupplier(std::unique_ptr<ContentDescriptor::Category>&& category)
        : m_category(std::move(category))
        {}
    TestCategorySupplier(ContentDescriptor::Category const& category)
        : m_category(std::make_unique<ContentDescriptor::Category>(category))
        {}
    virtual std::unique_ptr<ContentDescriptor::Category> _CreateDefaultCategory() const override { return std::make_unique<ContentDescriptor::Category>(*m_category); }
    virtual std::unique_ptr<ContentDescriptor::Category> _CreateECClassCategory(ECClassCR) const override { return std::make_unique<ContentDescriptor::Category>(*m_category); }
    virtual std::unique_ptr<ContentDescriptor::Category> _CreatePropertyCategory(ECPropertyCR) const override { return std::make_unique<ContentDescriptor::Category>(*m_category); }
    ContentDescriptor::Category const& GetUsedCategory() const {return *m_category;}
    void SetUsedCategory(std::unique_ptr<ContentDescriptor::Category>&& category)
        {
        m_category = std::move(category);
        }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct StubPropertyFormatter : IECPropertyFormatter
{
private:
    bool m_addUnitSystemSuffix;
protected:
    BentleyStatus _GetFormattedPropertyValue(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue, ECPresentation::UnitSystem) const override;
    BentleyStatus _GetFormattedPropertyLabel(Utf8StringR formattedLabel, ECPropertyCR ecProperty, ECClassCR propertyClass, RelatedClassPathCR relatedClassPath, RelationshipMeaning relationshipMeaning) const override;
    Formatting::Format const* _GetActiveFormat(KindOfQuantityCR koq, ECPresentation::UnitSystem unitSystem) const override
        {
        return nullptr;
        }

public:
    StubPropertyFormatter(bool addUnitSystemSuffix = false)
        : m_addUnitSystemSuffix(addUnitSystemSuffix)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestPropertyFormatter : IECPropertyFormatter
{
private:
    std::function<BentleyStatus(Utf8StringR, ECPropertyCR, ECValueCR, ECPresentation::UnitSystem)> m_valueFormatter;
    std::function<BentleyStatus(Utf8StringR, ECPropertyCR, ECClassCR, RelatedClassPathCR, RelationshipMeaning)> m_labelFormatter;
    std::function<Formatting::Format const* (KindOfQuantityCR, ECPresentation::UnitSystem)> m_activeFormatFormatter;
protected:
    BentleyStatus _GetFormattedPropertyValue(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue, ECPresentation::UnitSystem unitSystem) const override
        {
        return m_valueFormatter ? m_valueFormatter(formattedValue, ecProperty, ecValue, unitSystem) : ERROR;
        }
    BentleyStatus _GetFormattedPropertyLabel(Utf8StringR formattedLabel, ECPropertyCR ecProperty, ECClassCR propertyClass, RelatedClassPathCR relatedClassPath, RelationshipMeaning relationshipMeaning) const override
        {
        return m_labelFormatter ? m_labelFormatter(formattedLabel, ecProperty, propertyClass, relatedClassPath, relationshipMeaning) : ERROR;
        }
    Formatting::Format const* _GetActiveFormat(KindOfQuantityCR koq, ECPresentation::UnitSystem unitSystem) const override
        {
        return m_activeFormatFormatter ? m_activeFormatFormatter(koq, unitSystem) : nullptr;
        }
public:
    TestPropertyFormatter() {}
    void SetValueFormatter(std::function<BentleyStatus(Utf8StringR, ECPropertyCR, ECValueCR, ECPresentation::UnitSystem)> formatter) {m_valueFormatter = formatter;}
    void SetLabelFormatter(std::function<BentleyStatus(Utf8StringR, ECPropertyCR, ECClassCR, RelatedClassPathCR, RelationshipMeaning)> formatter) {m_labelFormatter = formatter;}
    void SetActiveFormatFormatter(std::function<Formatting::Format const* (KindOfQuantityCR, ECPresentation::UnitSystem)> formatter) {m_activeFormatFormatter = formatter;}
};

/*=================================================================================**//**
* @bsiclass
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
    TestParsedInput(bvector<bpair<ECClassCP, ECInstanceId>> const& pairs)
        {
        bset<ECClassCP> used;
        for (auto const& pair : pairs)
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
* @bsiclass
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
* @bsiclass
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestRulesetCallbacksHandler : IRulesetCallbacksHandler
    {
    typedef std::function<void(PresentationRuleSetCR)> CallbackHandler;
    CallbackHandler m_onCreatedHandler;
    CallbackHandler m_onDisposedHandler;

    void SetCreatedHandler(CallbackHandler handler) {m_onCreatedHandler = handler;}
    void SetDisposedHandler(CallbackHandler handler) {m_onDisposedHandler = handler;}

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
* @bsiclass
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
* @bsiclass
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
* @bsiclass
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

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestNodeLabelCalculator : INodeLabelCalculator
{
private:
    Utf8StringCR m_label;
protected:
    LabelDefinitionPtr _GetNodeLabel(ECClassInstanceKeyCR, bvector<ECInstanceKey> const&) const override
        {
        return LabelDefinition::Create(m_label.c_str());
        }
public:
    TestNodeLabelCalculator(Utf8StringCR label = Utf8String()) : m_label(label)
        {}
};

END_ECPRESENTATIONTESTS_NAMESPACE
