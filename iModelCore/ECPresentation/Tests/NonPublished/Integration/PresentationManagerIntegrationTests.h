/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationManager.h>
#include <UnitTests/ECPresentation/TestECInstanceChangeEventsSource.h>
#include "../Helpers/TestHelpers.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename THierarchyItem>
struct HierarchyListDef : bvector<THierarchyItem>
{
private:
    Nullable<bool> m_supportsFiltering;
public:
    HierarchyListDef() {}
    HierarchyListDef(bvector<THierarchyItem> nodes) : bvector<THierarchyItem>(nodes) {}
    HierarchyListDef(std::initializer_list<THierarchyItem> nodes) : bvector<THierarchyItem>(nodes) {}
    HierarchyListDef(bool supportsFiltering, bvector<THierarchyItem> nodes) : bvector<THierarchyItem>(nodes), m_supportsFiltering(supportsFiltering) {}
    HierarchyListDef(bool supportsFiltering, std::initializer_list<THierarchyItem> nodes) : bvector<THierarchyItem>(nodes), m_supportsFiltering(supportsFiltering) {}
    Nullable<bool> const& SupportsFiltering() const {return m_supportsFiltering;}
    void SetSupportsFiltering(Nullable<bool> value) {m_supportsFiltering = value;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TNode = NavNodeCPtr>
struct HierarchyDef
    {
    TNode node;
    Nullable<bool> nodeHasChildren;
    HierarchyListDef<HierarchyDef<TNode>> children;
    HierarchyDef(TNode n)
        : node(n), nodeHasChildren(false)
        {}
    HierarchyDef(TNode n, HierarchyListDef<HierarchyDef<TNode>> c)
        : node(n), children(c), nodeHasChildren(!c.empty())
        {}
    HierarchyDef(TNode n, bool hasChildren, HierarchyListDef<HierarchyDef<TNode>> c)
        : node(n), children(c), nodeHasChildren(hasChildren)
        {}
    };

typedef HierarchyDef<std::function<void(NavNodeCR, HierarchyRequestParams const&)>> ExpectedHierarchyDef;
typedef HierarchyListDef<ExpectedHierarchyDef> ExpectedHierarchyListDef;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PresentationManagerIntegrationTests : ECPresentationTest
    {
    DECLARE_SCHEMA_REGISTRY(PresentationManagerIntegrationTests);
    static ECDbTestProject* s_project;

    ECPresentationManager* m_manager;
    std::shared_ptr<IConnectionManager> m_connectionManager;
    DelayLoadingRuleSetLocaterPtr m_locater;
    RuntimeJsonLocalState m_localState;

    static void SetUpTestCase();
    static void TearDownTestCase();

    PresentationManagerIntegrationTests() : m_manager(nullptr) {}
    virtual std::unique_ptr<IConnectionManager> _CreateConnectionManager();
    virtual void _ConfigureManagerParams(ECPresentationManager::Params&);
    virtual ECDbR _GetProject();
    virtual void SetUp() override;
    virtual void TearDown() override;

    ECPresentationManager::Params CreateManagerParams();
    void ReCreatePresentationManager(ECPresentationManager::Params const&);

    void SetUpDefaultLabelRule(PresentationRuleSetR rules);

    ECSchemaCP GetSchema();
    ECClassCP GetClass(Utf8CP name);
    ECClassCP GetClass(Utf8CP schemaName, Utf8CP className);
    ECRelationshipClassCP GetRelationshipClass(Utf8CP name);
    ECRelationshipClassCP GetRelationshipClass(Utf8CP schemaName, Utf8CP className);
    Utf8String GetDisplayLabel(IECInstanceCR instance);

    Utf8String GetDefaultDisplayLabel(IECInstanceCR instance);

    void VerifyNodeInstance(NavNodeCR node, IECInstanceCR instance, InstanceFilterDefinitionCP instanceFilter = nullptr) { VerifyNodeInstances(node, { &instance }, instanceFilter); }
    void VerifyNodeInstances(NavNodeCR node, bvector<RefCountedPtr<IECInstance const>> const& instances, InstanceFilterDefinitionCP instanceFilter = nullptr);
    void VerifyPropertyGroupingNode(NavNodeCR node, InstanceFilterDefinitionCP instanceFilter, bvector<RefCountedPtr<IECInstance const>> const& groupedInstances, bvector<ECValue> const& groupedValues);
    void VerifyPropertyRangeGroupingNode(NavNodeCR node, InstanceFilterDefinitionCP instanceFilter, bvector<RefCountedPtr<IECInstance const>> const& groupedInstances);
    void VerifyClassGroupingNode(NavNodeCR node, InstanceFilterDefinitionCP instanceFilter, bvector<RefCountedPtr<IECInstance const>> const& groupedInstances, ECClassCP groupingClass = nullptr, bool isPolymorphicGrouping = false);
    void VerifyLabelGroupingNode(NavNodeCR node, InstanceFilterDefinitionCP instanceFilter, bvector<RefCountedPtr<IECInstance const>> const& groupedInstances);
    void VerifyCustomNode(NavNodeCR node, Utf8StringCR type, Nullable<Utf8String> const& label);

    HierarchyListDef<HierarchyDef<>> ValidateHierarchy(AsyncHierarchyRequestParams params, std::function<void(AsyncHierarchyRequestParams&)> const& configureParams, ExpectedHierarchyListDef const& expectedHierarchy);
    HierarchyListDef<HierarchyDef<>> ValidateHierarchy(AsyncHierarchyRequestParams const& params, ExpectedHierarchyListDef const& expectedHierarchy);
    std::function<void(NavNodeCR, HierarchyRequestParams const&)> CreateInstanceNodeValidator(bvector<RefCountedPtr<IECInstance const>> const& expectedInstances);
    std::function<void(NavNodeCR, HierarchyRequestParams const&)> CreateClassGroupingNodeValidator(ECClassCR ecClass, bool isPolymorphic, bvector<RefCountedPtr<IECInstance const>> const& expectedInstances);
    std::function<void(NavNodeCR, HierarchyRequestParams const&)> CreateLabelGroupingNodeValidator(Utf8StringCR label, bvector<RefCountedPtr<IECInstance const>> const& expectedInstances);
    std::function<void(NavNodeCR, HierarchyRequestParams const&)> CreatePropertyGroupingNodeValidator(bvector<RefCountedPtr<IECInstance const>> const& expectedInstances, ValueList const& groupedValues);
    std::function<void(NavNodeCR, HierarchyRequestParams const&)> CreateCustomNodeValidator(Utf8String type, Utf8String label);
    std::function<void(NavNodeCR, HierarchyRequestParams const&)> CreateCustomNodeValidator(Utf8String type);
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestUpdateRecordsHandler : IUpdateRecordsHandler
{
private:
    bvector<HierarchyUpdateRecord> m_records;
    bvector<FullUpdateRecord> m_fullUpdateRecords;
protected:
    void _Start() override { m_records.clear(); m_fullUpdateRecords.clear(); }
    void _Accept(HierarchyUpdateRecord const& record) override { m_records.push_back(record); }
    void _Accept(FullUpdateRecord const& record) override { m_fullUpdateRecords.push_back(record); }
    void _Finish() override {}
public:
    bvector<HierarchyUpdateRecord> const& GetRecords() const { return m_records; }
    bvector<FullUpdateRecord> const& GetFullUpdateRecords() const { return m_fullUpdateRecords; }
    void Clear() { m_records.clear(); m_fullUpdateRecords.clear(); }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct UpdateTests : PresentationManagerIntegrationTests
    {
    static BeFileName s_seedProjectPath;

    ECDb m_db;
    std::shared_ptr<TestECInstanceChangeEventsSource> m_eventsSource;
    std::shared_ptr<TestUpdateRecordsHandler> m_updateRecordsHandler;

    ECSchemaCP m_schema;
    ECClassCP m_widgetClass;
    ECClassCP m_gadgetClass;
    ECClassCP m_sprocketClass;

    static void SetUpTestCase();
    virtual void SetUp() override;
    virtual ECDbR _GetProject() override;
    virtual void _ConfigureManagerParams(ECPresentationManager::Params&) override;
    virtual std::unique_ptr<IConnectionManager> _CreateConnectionManager() override;

    void Sync() { m_manager->GetTasksCompletion().wait(); }
    };

#define DEFINE_SCHEMA(name, schema_xml) DEFINE_REGISTRY_SCHEMA(PresentationManagerIntegrationTests, name, schema_xml)

END_ECPRESENTATIONTESTS_NAMESPACE
