/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "../RulesEngine/TestHelpers.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct PresentationManagerIntegrationTests : ECPresentationTest
    {
    DECLARE_SCHEMA_REGISTRY(PresentationManagerIntegrationTests);
    static ECDbTestProject* s_project;

    RulesDrivenECPresentationManager* m_manager;
    IConnectionManager* m_connections;
    DelayLoadingRuleSetLocaterPtr m_locater;
    SQLangLocalizationProvider m_localizationProvider;
    RuntimeJsonLocalState m_localState;

    static void SetUpTestCase();
    static void TearDownTestCase();
    static void InitTestL10N();
    static void ShutDownTestL10N();

    PresentationManagerIntegrationTests() : m_manager(nullptr) {}
    virtual IConnectionManager* _CreateConnectionManager();
    virtual void _ConfigureManagerParams(RulesDrivenECPresentationManager::Params&);
    virtual ECDbR _GetProject();
    virtual void SetUp() override;
    virtual void TearDown() override;

    void SetUpDefaultLabelRule(PresentationRuleSetR rules);

    ECSchemaCP GetSchema();
    ECClassCP GetClass(Utf8CP name);
    ECClassCP GetClass(Utf8CP schemaName, Utf8CP className);
    ECRelationshipClassCP GetRelationshipClass(Utf8CP name);
    ECRelationshipClassCP GetRelationshipClass(Utf8CP schemaName, Utf8CP className);
    Utf8String GetClassNamesList(bvector<ECClassCP> const& classes);
    Utf8String GetDisplayLabel(IECInstanceCR instance);

    Utf8String GetDefaultDisplayLabel(IECInstanceCR instance);
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct TestUpdateRecordsHandler : IUpdateRecordsHandler
    {
    private:
        bvector<UpdateRecord> m_records;
        bvector<FullUpdateRecord> m_fullUpdateRecords;
    protected:
        void _Start() override { m_records.clear(); m_fullUpdateRecords.clear(); }
        void _Accept(UpdateRecord const& record) override { m_records.push_back(record); }
        void _Accept(FullUpdateRecord const& record) override { m_fullUpdateRecords.push_back(record); }
        void _Finish() override {}
    public:
        static RefCountedPtr<TestUpdateRecordsHandler> Create() { return new TestUpdateRecordsHandler(); }
        bvector<UpdateRecord> const& GetRecords() const { return m_records; }
        bvector<FullUpdateRecord> const& GetFullUpdateRecords() const { return m_fullUpdateRecords; }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct UpdateTests : PresentationManagerIntegrationTests
    {
    static BeFileName s_seedProjectPath;

    ECDb m_db;
    RefCountedPtr<TestECInstanceChangeEventsSource> m_eventsSource;
    RefCountedPtr<TestUpdateRecordsHandler> m_updateRecordsHandler;

    ECSchemaCP m_schema;
    ECClassCP m_widgetClass;
    ECClassCP m_gadgetClass;
    ECClassCP m_sprocketClass;

    static void SetUpTestCase();
    virtual void SetUp() override;
    virtual ECDbR _GetProject() override;
    virtual void _ConfigureManagerParams(RulesDrivenECPresentationManager::Params&) override;
    virtual IConnectionManager* _CreateConnectionManager() override;

    void Sync() { m_manager->GetTasksCompletion().wait(); }
    };

#define DEFINE_SCHEMA(name, schema_xml) DEFINE_REGISTRY_SCHEMA(PresentationManagerIntegrationTests, name, schema_xml)

END_ECPRESENTATIONTESTS_NAMESPACE
