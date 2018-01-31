/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/QueryExecutorTests.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/UserSettings.h>
#include "../../../Source/RulesDriven/RulesEngine/QueryExecutor.h"
#include "../../../Source/RulesDriven/RulesEngine/CustomFunctions.h"
#include "ECDbTestProject.h"
#include "TestHelpers.h"
#include "TestNavNode.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct QueryExecutorTests : ::testing::Test
    {
    static ECDbTestProject* s_project;
    
    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    ECSqlStatementCache m_statementCache;
    RelatedPathsCache m_relatedPathsCache;
    ECExpressionsCache m_expressionsCache;
    JsonNavNodesFactory m_nodesFactory;
    TestUserSettings m_userSettings;
    PresentationRuleSetPtr m_ruleset;
    CustomFunctionsInjector* m_customFunctionsInjector;
    DefaultCategorySupplier m_categorySupplier;
    TestPropertyFormatter const* m_propertyFormatter;
    ECSchemaHelper* m_schemaHelper;
    
    ECEntityClassCP m_widgetClass;
    ECEntityClassCP m_gadgetClass;
    ECEntityClassCP m_sprocketClass;
    ECRelationshipClassCP m_widgetHasGadgetsClass;

    static void SetUpTestCase();
    static void TearDownTestCase();
    
    QueryExecutorTests() : m_statementCache(1) {}

    void SetUp() override
        {
        Localization::Init();
        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
        m_customFunctionsInjector = new CustomFunctionsInjector(m_connections, *m_connection);
        m_schemaHelper = new ECSchemaHelper(*m_connection, &m_relatedPathsCache, &m_statementCache);
        m_ruleset = PresentationRuleSet::CreateInstance("QueryExecutorTests", 1, 0, false, "", "", "", false);

        m_widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget")->GetEntityClassCP();
        m_gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget")->GetEntityClassCP();
        m_sprocketClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Sprocket")->GetEntityClassCP();
        m_widgetHasGadgetsClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
        m_propertyFormatter = new TestPropertyFormatter();
        }

    void TearDown() override
        {
        s_project->GetECDb().AbandonChanges();
        delete m_customFunctionsInjector;
        delete m_propertyFormatter;
        delete m_schemaHelper;
        Localization::Terminate();
        }

    ContentDescriptor::Field& AddField(ContentDescriptorR descriptor, ECClassCR primaryClass, ContentDescriptor::Property prop)
        {
        return RulesEngineTestHelpers::AddField(descriptor, primaryClass, prop, m_categorySupplier);
        }
    };
