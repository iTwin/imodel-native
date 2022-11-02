/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/UserSettings.h>
#include "../../../../Source/Shared/Queries/QueryExecutor.h"
#include "../../../../Source/Shared/Queries/CustomFunctions.h"
#include "../../../../Source/Shared/RulesPreprocessor.h"
#include <UnitTests/ECPresentation/ECPresentationTest.h>
#include "../../Helpers/ECDbTestProject.h"
#include "../../Helpers/TestHelpers.h"
#include "../../Helpers/TestNavNode.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryExecutorTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;

    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    NavNodesFactory m_nodesFactory;
    RulesetVariables m_rulesetVariables;
    std::unique_ptr<IRulesPreprocessor> m_rulesPreprocessor;
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

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
        m_customFunctionsInjector = new CustomFunctionsInjector(m_connections, *m_connection);
        m_schemaHelper = new ECSchemaHelper(*m_connection, nullptr, nullptr);
        m_ruleset = PresentationRuleSet::CreateInstance("QueryExecutorTests");
        m_rulesPreprocessor = std::make_unique<RulesPreprocessor>(m_connections, *m_connection, *m_ruleset,
            m_rulesetVariables, nullptr, m_schemaHelper->GetECExpressionsCache());

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
        }

    ContentDescriptor::Field& AddField(ContentDescriptorR descriptor, ECClassCR primaryClass, ContentDescriptor::Property prop)
        {
        return RulesEngineTestHelpers::AddField(descriptor, primaryClass, prop, m_categorySupplier);
        }

    PresentationQueryContractFieldPtr CreateDisplayLabelField(SelectClass<ECClass> const& selectClass, bvector<RelatedClassPath> const& relatedInstancePaths = {},
        bvector<InstanceLabelOverrideValueSpecification const*> const& instanceLabelOverrides = {}) const
        {
        return RulesEngineTestHelpers::CreateDisplayLabelField(*m_schemaHelper, selectClass, relatedInstancePaths, instanceLabelOverrides);
        }
    };
