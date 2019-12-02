/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryContracts.h"
#include "../../../Source/RulesDriven/RulesEngine/LocalizationHelper.h"
#include "../../../Localization/Xliffs/ECPresentation.xliff.h"
#include "QueryExecutorTests.h"
#include "ExpectedQueries.h"
#include "TestHelpers.h"

ECDbTestProject* QueryExecutorTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutorTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("QueryExecutorTests", "RulesEngineTest.01.00.ecschema.xml");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutorTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TestExecutor : QueryExecutor
    {
    std::function<void()> m_resetCallback;
    std::function<void(ECSqlStatement&)> m_readRecordCallback;

    void _Reset() override {if (m_resetCallback) m_resetCallback();}
    void _ReadRecord(ECSqlStatement& stmt) override {if (m_readRecordCallback) m_readRecordCallback(stmt);}

    TestExecutor(IConnectionCR connection, PresentationQueryBase const& query)
        : QueryExecutor(connection, query)
        {}
    void SetResetCallback(std::function<void()> callback) {m_resetCallback = callback;}
    void SetReadRecordCallback(std::function<void(ECSqlStatement&)> callback) {m_readRecordCallback = callback;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void VerifyNodeInstance(NavNodeCR node, Utf8StringCR id)
    {
    bvector<ECInstanceKey> nodeInstanceKeys = NavNodeExtendedData(node).GetInstanceKeys();
    ASSERT_EQ(1, nodeInstanceKeys.size());
    EXPECT_STREQ(id.c_str(), nodeInstanceKeys[0].GetInstanceId().ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorTests, StopsReadingWhenCanceled)
    {
    // insert a couple of widgets
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    // create a query to select all widgets
    GenericQueryPtr query = StringGenericQuery::Create("SELECT * FROM [RET].[Widget]", BoundQueryValuesList());

    bool wasReset = false;
    uint32_t recordsRead = 0;

    // create the executor
    TestExecutor executor(*m_connection, *query);
    executor.SetResetCallback([&]()
        {
        wasReset = true;
        });
    executor.SetReadRecordCallback([&](ECSqlStatement&)
        {
        recordsRead++;
        });
    TestCancelationToken cancelationToken([&]()
        {
        // cancel after the first record
        return (recordsRead > 0);
        });
    executor.ReadRecords(&cancelationToken);

    // verify the read was canceled succesfully
    EXPECT_EQ(1, recordsRead);
    EXPECT_TRUE(wasReset);

    // attempt to read again without a cancelation token
    wasReset = false;
    recordsRead = 0;
    executor.ReadRecords(nullptr);

    // verify the read succeeded
    EXPECT_EQ(2, recordsRead);
    EXPECT_FALSE(wasReset);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryExecutorTests : QueryExecutorTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, GetInstanceNodes)
    {
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    ECClassSet classes;
    classes[m_widgetClass] = true;
    classes[m_gadgetClass] = true;

    NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(classes, "this");
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::MultiECInstanceNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();

    JsonNavNodePtr node;
    size_t nodesCount = executor.GetNodesCount();
    ASSERT_EQ(2, nodesCount);

    node = executor.GetNode(0);
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
    VerifyNodeInstance(*node, gadget->GetInstanceId());

    node = executor.GetNode(1);
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
    VerifyNodeInstance(*node, widget->GetInstanceId());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#711486
* @bsitest                                      Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, GetInstanceNodes_GroupsByInstanceKey)
    {
    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCP widgetsHaveGadgetsRelationship = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetsHaveGadgets")->GetRelationshipClassCP();

    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetsHaveGadgetsRelationship, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetsHaveGadgetsRelationship, *widget1, *gadget2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget2, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget2, *gadget2);

    RelatedClassPath relationshipPath = {
        RelatedClass(*m_widgetClass, *m_gadgetClass, *widgetHasGadgetsRelationship, true, "gadget", "r_WidgetHasGadgets"),
        RelatedClass(*m_gadgetClass, *m_widgetClass, *widgetsHaveGadgetsRelationship, false, "widget1", "r_WidgetsHaveGadgets")
        };
    RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    contract->SetRelationshipPath(relationshipPath);
    ComplexNavigationQueryPtr inner = ComplexNavigationQuery::Create();
    inner->SelectContract(*contract, "widget2");
    inner->From(*m_widgetClass, false, "widget2");
    inner->Join(relationshipPath, false);
    inner->Where("widget1.ECInstanceId = ?", {new BoundQueryId(widget1->GetInstanceId())});
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectContract(*contract);
    query->From(*inner);
    query->GroupByContract(*contract);
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();

    size_t nodesCount = executor.GetNodesCount();
    ASSERT_EQ(1, nodesCount);

    JsonNavNodePtr node = executor.GetNode(0);
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
    VerifyNodeInstance(*node, widget2->GetInstanceId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, GetClassGroupingNodes)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    ECInstanceId widgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [&widgetId](IECInstanceR gadget){gadget.SetValue("Widget", ECValue((int64_t)widgetId.GetValue()));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [&widgetId](IECInstanceR gadget){gadget.SetValue("Widget", ECValue((int64_t)widgetId.GetValue()));});

    bvector<ECClassCP> classes;
    classes.push_back(m_gadgetClass);
    classes.push_back(m_widgetClass);

    NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create();
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectAll();
    nested->From(*RulesEngineTestHelpers::CreateQuery(*contract, classes, false, "this"));
    nested->GroupByContract(*contract);

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll();
    query->From(*nested);
    query->OrderBy(Utf8String("[").append(ECClassGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ClassGroupingNodes);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();
    ASSERT_EQ(2, executor.GetNodesCount());

    NavNodeCPtr gadgetNode = executor.GetNode(0);
    NavNodeCPtr widgetNode = executor.GetNode(1);

    ASSERT_STREQ(Utf8String(m_widgetClass->GetDisplayLabel().c_str()).c_str(), widgetNode->GetLabel().c_str());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, widgetNode->GetType().c_str());
    ASSERT_EQ(1, NavNodeExtendedData(*widgetNode).GetInstanceKeys().size());

    ASSERT_STREQ(Utf8String(m_gadgetClass->GetDisplayLabel().c_str()).c_str(), gadgetNode->GetLabel().c_str());
    ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, gadgetNode->GetType().c_str());
    ASSERT_EQ(2, NavNodeExtendedData(*gadgetNode).GetInstanceKeys().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, GetDisplayLabelGroupingNodes)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR widget){widget.SetValue("MyID", ECValue("WidgetID"));});

    ECInstanceId widgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [&widgetId](IECInstanceR gadget)
        {
        gadget.SetValue("MyID", ECValue("GadgetID"));
        gadget.SetValue("Widget", ECValue((int64_t)widgetId.GetValue()));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [&widgetId](IECInstanceR gadget)
        {
        gadget.SetValue("MyID", ECValue("GadgetID"));
        gadget.SetValue("Widget", ECValue((int64_t)widgetId.GetValue()));
        });

    ECClassSet classes;
    classes[m_widgetClass] = true;
    classes[m_gadgetClass] = true;

    RefCountedPtr<DisplayLabelGroupingNodesQueryContract> gadgetContract = DisplayLabelGroupingNodesQueryContract::Create(m_gadgetClass, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("MyID")});
    ComplexNavigationQueryPtr gadgetInstancesQuery = ComplexNavigationQuery::Create();
    gadgetInstancesQuery->SelectContract(*gadgetContract, "this");
    gadgetInstancesQuery->From(*m_gadgetClass, false, "this");

    RefCountedPtr<DisplayLabelGroupingNodesQueryContract> widgetContract = DisplayLabelGroupingNodesQueryContract::Create(m_widgetClass, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("MyID")});
    ComplexNavigationQueryPtr widgetInstancesQuery = ComplexNavigationQuery::Create();
    widgetInstancesQuery->SelectContract(*widgetContract, "this");
    widgetInstancesQuery->From(*m_widgetClass, false, "this");

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll();
    query->From(*UnionNavigationQuery::Create(*gadgetInstancesQuery, *widgetInstancesQuery));
    query->GroupByContract(*DisplayLabelGroupingNodesQueryContract::Create(nullptr));
    query->OrderBy(Utf8String("[").append(ECClassGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::DisplayLabelGroupingNodes);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();
    ASSERT_EQ(2, executor.GetNodesCount());

    NavNodeCPtr gadgetNode = executor.GetNode(0);
    NavNodeCPtr widgetNode = executor.GetNode(1);

    EXPECT_STREQ("WidgetID", widgetNode->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, widgetNode->GetType().c_str());

    EXPECT_STREQ("GadgetID", gadgetNode->GetLabel().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, gadgetNode->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, GetChildNodesOfDisplayLabelGroupingNode)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("AAA"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("AAA"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("BBB"));});

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll();
    query->From(ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false));
    query->Where(Utf8PrintfString("[%s] = ?", DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName).c_str(), {new BoundQueryECValue(ECValue("AAA"))});
    query->OrderBy(Utf8String("[").append(ECClassGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();

    size_t nodesCount = executor.GetNodesCount();
    ASSERT_EQ(2, nodesCount);

    for (size_t i = 0; i < nodesCount; i++)
        {
        JsonNavNodePtr node = executor.GetNode(i);
        ASSERT_TRUE(node.IsValid());
        EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        EXPECT_STREQ("AAA", node->GetLabel().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, GetChildNodesOfDisplayLabelGroupingNode_InstanceLabelOverride)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("AAA"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("AAA"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("BBB"));});

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("MyID")});
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll();
    query->From(ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false));
    query->Where(Utf8PrintfString("[%s] = ?", DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName).c_str(), {new BoundQueryECValue(ECValue("AAA"))});
    query->OrderBy(Utf8String("[").append(ECClassGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();

    size_t nodesCount = executor.GetNodesCount();
    ASSERT_EQ(2, nodesCount);

    for (size_t i = 0; i < nodesCount; i++)
        {
        JsonNavNodePtr node = executor.GetNode(i);
        ASSERT_TRUE(node.IsValid());
        EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        EXPECT_STREQ("AAA", node->GetLabel().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, GetChildNodesOfClassGroupingNode_NotGroupedByLabel)
    {
    IECInstancePtr widgets[2];
    widgets[0] = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    widgets[1] = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll();
    query->From(ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false));
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale",
        m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();
    ASSERT_EQ(2, executor.GetNodesCount());

    for (size_t i = 0; i < 2; i++)
        {
        JsonNavNodePtr node = executor.GetNode(i);
        ASSERT_TRUE(node.IsValid());
        EXPECT_STREQ(NAVNODE_TYPE_ECInstancesNode, node->GetType().c_str());
        VerifyNodeInstance(*node, widgets[i]->GetInstanceId());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, GetChildNodesOfClassGroupingNode_GroupedByLabel)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR widget){widget.SetValue("MyID", ECValue("WidgetID"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR widget){widget.SetValue("MyID", ECValue("WidgetID"));});

    RefCountedPtr<DisplayLabelGroupingNodesQueryContract> contract = DisplayLabelGroupingNodesQueryContract::Create(m_widgetClass, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("MyID")});
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll();
    query->From(ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false));
    query->GroupByContract(*contract);
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::DisplayLabelGroupingNodes);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();
    ASSERT_EQ(1, executor.GetNodesCount());

    JsonNavNodePtr node = executor.GetNode(0);
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, node->GetType().c_str());
    EXPECT_STREQ("WidgetID", node->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, OverridesLabel)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.IsInstanceNode AND ThisNode.ClassName=\"Widget\"", 1, "\"NavigationQueryExecutorTests.OverridesLabel\"", ""));

    ECClassSet classes;
    classes[m_widgetClass] = true;
    classes[m_gadgetClass] = true;

    NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(classes, "this");
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::MultiECInstanceNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();
    ASSERT_EQ(2, executor.GetNodesCount());

    JsonNavNodePtr node = executor.GetNode(0);
    ASSERT_TRUE(node.IsValid());
    EXPECT_EQ(m_gadgetClass->GetId(), NavNodeExtendedData(*node).GetECClassId());
    EXPECT_STRNE("NavigationQueryExecutorTests.OverridesLabel", node->GetLabel().c_str());

    node = executor.GetNode(1);
    ASSERT_TRUE(node.IsValid());
    EXPECT_EQ(m_widgetClass->GetId(), NavNodeExtendedData(*node).GetECClassId());
    EXPECT_STREQ("NavigationQueryExecutorTests.OverridesLabel", node->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, InstanceNodesSortedAlphanumerically)
    {
    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectContract(*contract);
    query->From(*m_widgetClass, false);
    query->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, "MyID").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget10"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget9A10"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget9A9"));});

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();
    ASSERT_EQ(3, executor.GetNodesCount());

    JsonNavNodePtr node = executor.GetNode(0);
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("Widget9A9", node->GetLabel().c_str());

    node = executor.GetNode(1);
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("Widget9A10", node->GetLabel().c_str());

    node = executor.GetNode(2);
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("Widget10", node->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, InstanceNodesSortedAlphanumerically_InstanceLabelOverride)
    {
    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("MyID")});
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectContract(*contract);
    query->From(*m_widgetClass, false);
    query->OrderBy(Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetSortingValue, "MyID").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget10"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget9A10"));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget9A9"));});

    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();
    ASSERT_EQ(3, executor.GetNodesCount());

    JsonNavNodePtr node = executor.GetNode(0);
    EXPECT_STREQ("Widget9A9", node->GetLabel().c_str());

    node = executor.GetNode(1);
    EXPECT_STREQ("Widget9A10", node->GetLabel().c_str());

    node = executor.GetNode(2);
    EXPECT_STREQ("Widget10", node->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, ClassGroupingNodesSortedByLabel)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    ECInstanceId widgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [&widgetId](IECInstanceR gadget){gadget.SetValue("Widget", ECValue((int64_t)widgetId.GetValue()));});
    ECInstanceId gadgetId;
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass, [&gadgetId](IECInstanceR sprocket){sprocket.SetValue("Gadget", ECValue((int64_t)gadgetId.GetValue()));});

    bvector<ECClassCP> classes;
    classes.push_back(m_gadgetClass);
    classes.push_back(m_widgetClass);
    classes.push_back(m_sprocketClass);

    // taken from ECClassSortingQueryContext in QueryBuilder
    static Utf8PrintfString sortedDisplayLabel("%s([%s])", FUNCTION_NAME_GetSortingValue, ECClassGroupingNodesQueryContract::DisplayLabelFieldName);

    NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create();
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll();
    grouped->From(*RulesEngineTestHelpers::CreateQuery(*contract, classes, false, "this"));
    grouped->GroupByContract(*contract);
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll();
    query->From(*grouped);
    query->OrderBy(sortedDisplayLabel.c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ClassGroupingNodes);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();
    ASSERT_EQ(3, executor.GetNodesCount());

    JsonNavNodePtr node = executor.GetNode(0);
    ASSERT_EQ(m_gadgetClass->GetId(), NavNodeExtendedData(*node).GetECClassId());

    node = executor.GetNode(1);
    ASSERT_EQ(m_sprocketClass->GetId(), NavNodeExtendedData(*node).GetECClassId());

    node = executor.GetNode(2);
    ASSERT_EQ(m_widgetClass->GetId(), NavNodeExtendedData(*node).GetECClassId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryExecutorTests, PropertyGroupingByBooleanProperty)
    {
    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("BoolProperty", ECValue(true)); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("BoolProperty", ECValue(false)); });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("BoolProperty", ECValue(false)); });

    PropertyGroup propertyGroup("", "TestImageId", false, "BoolProperty", "");

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*m_widgetClass, *m_widgetClass->GetPropertyP("BoolProperty"), nullptr, propertyGroup, nullptr);
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract);
    nested->From(*m_widgetClass, false);
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *grouped);
    executor.ReadRecords();
    ASSERT_EQ(2, executor.GetNodesCount());

    JsonNavNodePtr node0 = executor.GetNode(0);
    EXPECT_STREQ("False", node0->GetLabel().c_str());
    ASSERT_TRUE(NavNodeExtendedData(*node0).HasPropertyValue());
    EXPECT_EQ(false, NavNodeExtendedData(*node0).GetPropertyValue()->GetBool());

    JsonNavNodePtr node1 = executor.GetNode(1);
    EXPECT_STREQ("True", node1->GetLabel().c_str());
    ASSERT_TRUE(NavNodeExtendedData(*node1).HasPropertyValue());
    EXPECT_EQ(true, NavNodeExtendedData(*node1).GetPropertyValue()->GetBool());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, PropertyGroupingByIntProperty)
    {
    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(10));});

    PropertyGroup propertyGroup("", "TestImageId", false, "IntProperty", "");

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*m_widgetClass, *m_widgetClass->GetPropertyP("IntProperty"), nullptr, propertyGroup, nullptr);
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract);
    nested->From(*m_widgetClass, false);
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *grouped);
    executor.ReadRecords();
    ASSERT_EQ(3, executor.GetNodesCount());
    for (size_t i = 0; i < 3; i++)
        {
        JsonNavNodePtr node = executor.GetNode(i);
        ASSERT_TRUE(node.IsValid());
        switch (i + 1)
            {
            case 1: EXPECT_STREQ("-1", node->GetLabel().c_str()); break;
            case 2: EXPECT_STREQ("10", node->GetLabel().c_str()); break;
            case 3: EXPECT_STREQ("5", node->GetLabel().c_str()); break;
            }
        EXPECT_STREQ("TestImageId", node->GetCollapsedImageId().c_str());
        EXPECT_STREQ("TestImageId", node->GetExpandedImageId().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, PropertyRangeGrouping)
    {
    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(6));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(10));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(21));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(100));});

    PropertyGroup propertyGroup("", "", false, "IntProperty", "");
    propertyGroup.AddRange(*new PropertyRangeGroupSpecification("", "", "0", "5"));
    propertyGroup.AddRange(*new PropertyRangeGroupSpecification("", "", "6", "10"));
    propertyGroup.AddRange(*new PropertyRangeGroupSpecification("", "", "11", "20"));

    ECPropertyCR groupingProperty = *m_widgetClass->GetPropertyP("IntProperty");

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*m_widgetClass, groupingProperty, nullptr, propertyGroup, nullptr);
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract);
    nested->From(*m_widgetClass, false);
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    NavigationQueryExtendedData(*grouped).AddRangesData(groupingProperty, propertyGroup);
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *grouped);
    executor.ReadRecords();
    ASSERT_EQ(3, executor.GetNodesCount());
    for (size_t i = 0; i < 3; i++)
        {
        JsonNavNodePtr node = executor.GetNode(i);
        ASSERT_TRUE(node.IsValid());
        switch (i + 1)
            {
            case 1: EXPECT_STREQ("0 - 5", node->GetLabel().c_str()); break;
            case 2: EXPECT_STREQ("6 - 10", node->GetLabel().c_str()); break;
            case 3: EXPECT_STREQ(RULESENGINE_LOCALIZEDSTRING_Other.c_str(), node->GetLabel().c_str()); break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, PropertyRangeGroupingWithCustomLabelsAndImageIds)
    {
    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(6));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(10));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(21));});
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(100));});

    PropertyGroup propertyGroup("", "", false, "IntProperty", "");
    propertyGroup.AddRange(*new PropertyRangeGroupSpecification("CustomLabel2", "", "0", "5"));
    propertyGroup.AddRange(*new PropertyRangeGroupSpecification("CustomLabel1", "CustomImage1", "6", "10"));
    propertyGroup.AddRange(*new PropertyRangeGroupSpecification("Custom3", "CustomImage3", "11", "20"));

    ECPropertyCR groupingProperty = *m_widgetClass->GetPropertyP("IntProperty");

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*m_widgetClass, groupingProperty, nullptr, propertyGroup, nullptr);
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract);
    nested->From(*m_widgetClass, false);
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    NavigationQueryExtendedData(*grouped).AddRangesData(groupingProperty, propertyGroup);
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *grouped);
    executor.ReadRecords();
    ASSERT_EQ(3, executor.GetNodesCount());
    for (size_t i = 0; i < executor.GetNodesCount(); i++)
        {
        JsonNavNodePtr node = executor.GetNode(i);
        ASSERT_TRUE(node.IsValid());
        switch (i + 1)
            {
            case 1:
                EXPECT_STREQ(RULESENGINE_LOCALIZEDSTRING_Other.c_str(), node->GetLabel().c_str());
                EXPECT_STREQ("", node->GetCollapsedImageId().c_str());
                EXPECT_STREQ("", node->GetExpandedImageId().c_str());
                break;
            case 2:
                EXPECT_STREQ("CustomLabel1", node->GetLabel().c_str());
                EXPECT_STREQ("CustomImage1", node->GetCollapsedImageId().c_str());
                EXPECT_STREQ("CustomImage1", node->GetExpandedImageId().c_str());
                break;
            case 3:
                EXPECT_STREQ("CustomLabel2", node->GetLabel().c_str());
                EXPECT_STREQ("", node->GetCollapsedImageId().c_str());
                EXPECT_STREQ("", node->GetExpandedImageId().c_str());
                break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, PropertyGroupingByForeignKeyWithoutDisplayLabel)
    {
    ECRelationshipClassCP gadgetHasSprockets = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();

    // create our own instances
    ECInstanceInserter gadgetInserter(s_project->GetECDb(), *m_gadgetClass, nullptr);
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("GadgetID"));});
    ECInstanceInserter sprocketInserter(s_project->GetECDb(), *m_sprocketClass, nullptr);
    IECInstancePtr sprocket1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), sprocketInserter, *m_sprocketClass);
    IECInstancePtr sprocket2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), sprocketInserter, *m_sprocketClass);
    IECInstancePtr sprocket3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), sprocketInserter, *m_sprocketClass);
    IECInstancePtr sprocket4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), sprocketInserter, *m_sprocketClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *gadgetHasSprockets, *gadget1, *sprocket1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *gadgetHasSprockets, *gadget1, *sprocket2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *gadgetHasSprockets, *gadget2, *sprocket3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *gadgetHasSprockets, *gadget2, *sprocket4);

    PropertyGroup propertyGroup("", "", false, "Gadget", "");

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*m_sprocketClass, *m_sprocketClass->GetPropertyP("Gadget"), nullptr, propertyGroup, m_gadgetClass);
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract, "alias");
    nested->From(*m_sprocketClass, false, "alias");
    nested->From(*m_gadgetClass, true, "parentInstance");
    nested->Where("([alias].[Gadget].[Id] = [parentInstance].[ECInstanceId] OR [alias].[Gadget].[Id] IS NULL)", BoundQueryValuesList());
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *grouped);
    executor.ReadRecords();
    ASSERT_EQ(1, executor.GetNodesCount());

    JsonNavNodePtr node = executor.GetNode(0);
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("GadgetID", node->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, PropertyGroupingByForeignKeyWithDisplayLabelOverridenByLabelOverride)
    {
    ECRelationshipClassCP widgetHasGadgets = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();

    // create our own instances
    ECInstanceInserter widgetInserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), widgetInserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), widgetInserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    ECInstanceInserter gadgetInserter(s_project->GetECDb(), *m_gadgetClass, nullptr);
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget1, *gadget2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget2, *gadget3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget2, *gadget4);

    PropertyGroup propertyGroup("", "", false, "Widget", "");

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*m_gadgetClass, *m_gadgetClass->GetPropertyP("Widget"), nullptr, propertyGroup, m_widgetClass);
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract, "alias");
    nested->From(*m_gadgetClass, false, "alias");
    nested->From(*m_widgetClass, true, "parentInstance");
    nested->Where("([alias].[Widget].[Id] = [parentInstance].[ECInstanceId] OR [alias].[Widget].[Id] IS NULL)", BoundQueryValuesList());
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *grouped);
    executor.ReadRecords();
    ASSERT_EQ(2, executor.GetNodesCount());
    for (size_t i = 0; i < executor.GetNodesCount(); i++)
        {
        JsonNavNodePtr node = executor.GetNode(i);
        ASSERT_TRUE(node.IsValid());
        switch (i + 1)
            {
            case 1: EXPECT_STREQ("Widget1", node->GetLabel().c_str()); break;
            case 2: EXPECT_STREQ("Widget2", node->GetLabel().c_str()); break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, PropertyGroupingByForeignKeyWithDisplayLabelOveridenByInstanceLabelOverride)
    {
    ECRelationshipClassCP widgetHasGadgets = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();

    // create our own instances
    ECInstanceInserter widgetInserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), widgetInserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), widgetInserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget2"));});
    ECInstanceInserter gadgetInserter(s_project->GetECDb(), *m_gadgetClass, nullptr);
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget1, *gadget2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget2, *gadget3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget2, *gadget4);

    PropertyGroup propertyGroup("", "", false, "Widget", "");

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*m_gadgetClass, *m_gadgetClass->GetPropertyP("Widget"), nullptr, propertyGroup, m_widgetClass);
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract, "alias");
    nested->From(*m_gadgetClass, false, "alias");
    nested->From(*m_widgetClass, true, "parentInstance");
    nested->Where("([alias].[Widget].[Id] = [parentInstance].[ECInstanceId] OR [alias].[Widget].[Id] IS NULL)", BoundQueryValuesList());
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *grouped);
    executor.ReadRecords();
    ASSERT_EQ(2, executor.GetNodesCount());
    for (size_t i = 0; i < executor.GetNodesCount(); i++)
        {
        JsonNavNodePtr node = executor.GetNode(i);
        ASSERT_TRUE(node.IsValid());
        switch (i + 1)
            {
            case 1: EXPECT_STREQ("Widget1", node->GetLabel().c_str()); break;
            case 2: EXPECT_STREQ("Widget2", node->GetLabel().c_str()); break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, PropertyGroupingByForeignKeyWithDisplayLabel_UsingNavigationProperty)
    {
    ECClassCP classD = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassD");
    ECClassCP classE = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassE");
    ECRelationshipClassCP classDHasClassE = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassCP();

    // create our own instances
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD, [](IECInstanceR instance){instance.SetValue("StringProperty", ECValue("ClassD_Label"));});
    IECInstancePtr instanceE1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr instanceE2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classDHasClassE, *instanceD, *instanceE1);

    PropertyGroup propertyGroup("", "", false, "ClassD", "");

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*classE, *classE->GetPropertyP("ClassD"), nullptr, propertyGroup, classD->GetEntityClassCP());
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract, "alias");
    nested->From(*classE, false, "alias");
    nested->From(*classD, true, "parentInstance");
    nested->Where("([alias].[ClassD].[Id] = [parentInstance].[ECInstanceId] OR [alias].[ClassD].[Id] IS NULL)", BoundQueryValuesList());
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:ClassD", "StringProperty"));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *grouped);
    executor.ReadRecords();
    ASSERT_EQ(1, executor.GetNodesCount());

    JsonNavNodePtr node = executor.GetNode(0);
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("ClassD_Label", node->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, PropertyGroupingByForeignKeyWithLabelOverride)
    {
    ECClassCP classD = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassD");
    ECClassCP classE = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassE");
    ECRelationshipClassCP classDHasClassE = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassCP();

    // create our own instances
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr instanceE1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr instanceE2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classDHasClassE, *instanceD, *instanceE1);

    PropertyGroup propertyGroup("", "", false, "ClassD", "");
    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.IsInstanceNode ANDALSO ThisNode.ClassName=\"ClassD\"", 1, "\"MyLabel\"", ""));

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*classE, *classE->GetPropertyP("ClassD"), nullptr, propertyGroup, classD->GetEntityClassCP());
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract, "alias");
    nested->From(*classE, false, "alias");
    nested->From(*classD, true, "parentInstance");
    nested->Where("([alias].[ClassD].[Id] = [parentInstance].[ECInstanceId] OR [alias].[ClassD].[Id] IS NULL)", BoundQueryValuesList());
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *grouped);
    executor.ReadRecords();
    ASSERT_EQ(1, executor.GetNodesCount());

    JsonNavNodePtr node = executor.GetNode(0);
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("MyLabel", node->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, PropertyGroupingByForeignKeyWithInstanceLabelOverride)
    {
    ECClassCP classD = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassD");
    ECClassCP classE = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassE");
    ECRelationshipClassCP classDHasClassE = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassCP();

    // create our own instances
    IECInstancePtr instanceD = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD, [](IECInstanceR instance){instance.SetValue("StringProperty", ECValue("ClassD_StringProperty"));});
    IECInstancePtr instanceE1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    IECInstancePtr instanceE2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *classDHasClassE, *instanceD, *instanceE1);

    PropertyGroup propertyGroup("", "", false, "ClassD", "");
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:ClassD", "StringProperty"));

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*classE, *classE->GetPropertyP("ClassD"), nullptr, propertyGroup, classD->GetEntityClassCP());
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract, "alias");
    nested->From(*classE, false, "alias");
    nested->From(*classD, true, "parentInstance");
    nested->Where("([alias].[ClassD].[Id] = [parentInstance].[ECInstanceId] OR [alias].[ClassD].[Id] IS NULL)", BoundQueryValuesList());
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *grouped);
    executor.ReadRecords();
    ASSERT_EQ(1, executor.GetNodesCount());

    JsonNavNodePtr node = executor.GetNode(0);
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("ClassD_StringProperty", node->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, PropertyGroupingByForeignKeyWithDisplayLabel_DifferentInstancesWithSameLabelGetMerged)
    {
    ECRelationshipClassCP widgetHasGadgets = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();

    // create our own instances
    ECInstanceInserter widgetInserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), widgetInserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetName"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), widgetInserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetName"));});
    ECInstanceInserter gadgetInserter(s_project->GetECDb(), *m_gadgetClass, nullptr);
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget1, *gadget2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget2, *gadget3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget2, *gadget4);

    PropertyGroup propertyGroup("", "", false, "Widget", "");

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*m_gadgetClass, *m_gadgetClass->GetPropertyP("Widget"), nullptr, propertyGroup, m_widgetClass);
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract, "alias");
    nested->From(*m_gadgetClass, false, "alias");
    nested->From(*m_widgetClass, true, "parentInstance");
    nested->Where("([alias].[Widget].[Id] = [parentInstance].[ECInstanceId] OR [alias].[Widget].[Id] IS NULL)", BoundQueryValuesList());
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *grouped);
    executor.ReadRecords();
    ASSERT_EQ(1, executor.GetNodesCount());

    JsonNavNodePtr node = executor.GetNode(0);
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("WidgetName", node->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryExecutorTests, PropertyGroupingByForeignKeyWithDisplayLabel_DifferentInstancesWithSameInstanceLabelGetMerged)
    {
    ECRelationshipClassCP widgetHasGadgets = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();

    // create our own instances
    ECInstanceInserter widgetInserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), widgetInserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetName"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), widgetInserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetName"));});
    ECInstanceInserter gadgetInserter(s_project->GetECDb(), *m_gadgetClass, nullptr);
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget1, *gadget1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget1, *gadget2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget2, *gadget3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgets, *widget2, *gadget4);

    PropertyGroup propertyGroup("", "", false, "Widget", "");

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*m_gadgetClass, *m_gadgetClass->GetPropertyP("Widget"), nullptr, propertyGroup, m_widgetClass);
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract, "alias");
    nested->From(*m_gadgetClass, false, "alias");
    nested->From(*m_widgetClass, true, "parentInstance");
    nested->Where("([alias].[Widget].[Id] = [parentInstance].[ECInstanceId] OR [alias].[Widget].[Id] IS NULL)", BoundQueryValuesList());
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *grouped);
    executor.ReadRecords();
    ASSERT_EQ(1, executor.GetNodesCount());

    JsonNavNodePtr node = executor.GetNode(0);
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("WidgetName", node->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static ECInstanceKey CreateECInstanceKey(ECClassCR ecClass, ECInstanceId instanceId)
    {
    return ECInstanceKey(ecClass.GetId(), instanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static ECInstanceKey CreateECInstanceKey(ECClassCR ecClass, Utf8StringCR instanceIdStr)
    {
    ECInstanceId instanceId;
    EXPECT_EQ(SUCCESS, ECInstanceId::FromString(instanceId, instanceIdStr.c_str()));
    return CreateECInstanceKey(ecClass, instanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void AssertNodeGroupsECInstance(NavNodeCR node, ECInstanceKeyCR key)
    {
    NavNodeExtendedData extendedData(node);
    bvector<ECInstanceKey> groupedKeys = extendedData.GetInstanceKeys();
    EXPECT_EQ(1, groupedKeys.size());
    EXPECT_EQ(key, groupedKeys[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void AssertNodeGroupsECInstances(NavNodeCR node, bvector<ECInstanceKey> const& keys)
    {
    NavNodeExtendedData extendedData(node);
    bvector<ECInstanceKey> groupedKeys = extendedData.GetInstanceKeys();
    EXPECT_EQ(keys.size(), groupedKeys.size());
    for (ECInstanceKeyCR key : keys)
        {
        auto iter = std::find(groupedKeys.begin(), groupedKeys.end(), key);
        EXPECT_NE(groupedKeys.end(), iter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryExecutorTests, SetsGroupedInstanceKeysForECInstanceNodes)
    {
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClass(*m_widgetClass, true, "this");
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();
    ASSERT_EQ(2, executor.GetNodesCount());

    JsonNavNodePtr node1 = executor.GetNode(0);
    VerifyNodeInstance(*node1, widget1->GetInstanceId());

    JsonNavNodePtr node2 = executor.GetNode(1);
    VerifyNodeInstance(*node2, widget2->GetInstanceId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryExecutorTests, SetsGroupedInstanceKeysForECClassGroupingNodes)
    {
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    bvector<ECClassCP> classes;
    classes.push_back(m_gadgetClass);
    classes.push_back(m_widgetClass);

    NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create();
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll();
    grouped->From(*RulesEngineTestHelpers::CreateQuery(*contract, classes, false, "this"));
    grouped->GroupByContract(*contract);
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll();
    query->From(*grouped);
    query->OrderBy(Utf8String("[").append(ECClassGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ClassGroupingNodes);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Class);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();
    ASSERT_EQ(2, executor.GetNodesCount());

    JsonNavNodePtr gadgetNode = executor.GetNode(0);
    AssertNodeGroupsECInstance(*gadgetNode, CreateECInstanceKey(*m_gadgetClass, gadget->GetInstanceId()));

    JsonNavNodePtr widgetNode = executor.GetNode(1);
    bvector<ECInstanceKey> widgetIds;
    widgetIds.push_back(CreateECInstanceKey(*m_widgetClass, widget1->GetInstanceId()));
    widgetIds.push_back(CreateECInstanceKey(*m_widgetClass, widget2->GetInstanceId()));
    AssertNodeGroupsECInstances(*widgetNode, widgetIds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryExecutorTests, SetsGroupedInstanceKeysForBaseClassGroupingNodes)
    {
    ECClassCP classE = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassE");
    ECClassCP classF = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassF");
    ECClassCP classG = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassG");

    IECInstancePtr itemF1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);
    IECInstancePtr itemF2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classF);
    IECInstancePtr itemG = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classG);

    NavigationQueryContractPtr contract = BaseECClassGroupingNodesQueryContract::Create(classE->GetId());

    ComplexNavigationQueryPtr nested1 = ComplexNavigationQuery::Create();
    nested1->SelectContract(*contract);
    nested1->From(*classF, false, "this");

    ComplexNavigationQueryPtr nested2 = ComplexNavigationQuery::Create();
    nested2->SelectContract(*contract);
    nested2->From(*classG, false, "this");

    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll();
    grouped->From(*UnionNavigationQuery::Create(*nested1, *nested2));
    grouped->GroupByContract(*contract);

    ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
    sorted->SelectAll();
    sorted->From(*grouped);
    sorted->OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &sorted->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *sorted);
    executor.ReadRecords();
    ASSERT_EQ(1, executor.GetNodesCount());

    JsonNavNodePtr node = executor.GetNode(0);
    bvector<ECInstanceKey> keys;
    keys.push_back(CreateECInstanceKey(*classF, itemF1->GetInstanceId()));
    keys.push_back(CreateECInstanceKey(*classF, itemF2->GetInstanceId()));
    keys.push_back(CreateECInstanceKey(*classG, itemG->GetInstanceId()));
    AssertNodeGroupsECInstances(*node, keys);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryExecutorTests, SetsGroupedInstanceKeysForDisplayLabelGroupingNodes)
    {
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("WidgetID"));});

    NavigationQueryContractPtr contract = DisplayLabelGroupingNodesQueryContract::Create(m_widgetClass, bvector<RelatedClass>(), {new InstanceLabelOverridePropertyValueSpecification("MyID")});
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll();
    query->From(ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, true));
    query->GroupByContract(*contract);
    query->OrderBy(Utf8String("[").append(ECClassGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::DisplayLabelGroupingNodes);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);

    //m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *query);
    executor.ReadRecords();
    ASSERT_EQ(1, executor.GetNodesCount());

    JsonNavNodePtr widgetNode = executor.GetNode(0);
    bvector<ECInstanceKey> widgetIds;
    widgetIds.push_back(CreateECInstanceKey(*m_widgetClass, widget1->GetInstanceId()));
    widgetIds.push_back(CreateECInstanceKey(*m_widgetClass, widget2->GetInstanceId()));
    AssertNodeGroupsECInstances(*widgetNode, widgetIds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryExecutorTests, SetsGroupedInstanceKeysForPropertyGroupingNodes)
    {
    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    IECInstancePtr widget4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    IECInstancePtr widget5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(10));});

    PropertyGroup propertyGroup("", "", false, "IntProperty", "");

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*m_widgetClass, *m_widgetClass->GetPropertyP("IntProperty"), nullptr, propertyGroup, nullptr);
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract);
    nested->From(*m_widgetClass, false);
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("GetSortingValue([").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("])").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *grouped);
    executor.ReadRecords();
    ASSERT_EQ(3, executor.GetNodesCount());

    JsonNavNodePtr node1 = executor.GetNode(0);
    bvector<ECInstanceKey> node1Ids;
    node1Ids.push_back(CreateECInstanceKey(*m_widgetClass, widget1->GetInstanceId()));
    node1Ids.push_back(CreateECInstanceKey(*m_widgetClass, widget4->GetInstanceId()));
    AssertNodeGroupsECInstances(*node1, node1Ids);

    JsonNavNodePtr node2 = executor.GetNode(1);
    bvector<ECInstanceKey> node2Ids;
    node2Ids.push_back(CreateECInstanceKey(*m_widgetClass, widget2->GetInstanceId()));
    node2Ids.push_back(CreateECInstanceKey(*m_widgetClass, widget3->GetInstanceId()));
    AssertNodeGroupsECInstances(*node2, node2Ids);

    JsonNavNodePtr node3 = executor.GetNode(2);
    AssertNodeGroupsECInstance(*node3, CreateECInstanceKey(*m_widgetClass, widget5->GetInstanceId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryExecutorTests, SetsRelatedInstanceKeysForECInstanceNodes)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr sprocket = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_sprocketClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *m_widgetHasGadgetsClass, *widget, *gadget);

    RelatedClass widgetRelatedToGadgetInfo(*m_gadgetClass, *m_widgetClass, *m_widgetHasGadgetsClass, false, "w");
    bvector<RelatedClass> gadgetRelatedClasses;
    gadgetRelatedClasses.push_back(widgetRelatedToGadgetInfo);

    ComplexNavigationQueryPtr gadgetsQuery = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClass(*m_gadgetClass, true, "this", gadgetRelatedClasses);
    gadgetsQuery->Join(RelatedClass(*m_gadgetClass, *m_widgetClass, *m_widgetHasGadgetsClass, false, "w", nullptr, true, false));
    ComplexNavigationQueryPtr sprocketsQuery = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClass(*m_sprocketClass, true, "this");
    UnionNavigationQueryPtr unionQuery = UnionNavigationQuery::Create(*gadgetsQuery, *sprocketsQuery);
    unionQuery->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &unionQuery->GetExtendedData());
    NavigationQueryExecutor executor(m_nodesFactory, *m_connection, "locale", *unionQuery);
    executor.ReadRecords();
    ASSERT_EQ(2, executor.GetNodesCount());

    JsonNavNodePtr node = executor.GetNode(0);
    VerifyNodeInstance(*node, gadget->GetInstanceId());
    bvector<NavNodeExtendedData::RelatedInstanceKey> relatedInstanceKeys = NavNodeExtendedData(*node).GetRelatedInstanceKeys();
    ASSERT_EQ(1, relatedInstanceKeys.size());
    EXPECT_STREQ("w", relatedInstanceKeys[0].GetAlias());
    EXPECT_EQ(m_widgetClass->GetId(), relatedInstanceKeys[0].GetInstanceKey().GetClassId());
    EXPECT_STREQ(widget->GetInstanceId().c_str(), relatedInstanceKeys[0].GetInstanceKey().GetInstanceId().ToString().c_str());

    node = executor.GetNode(1);
    VerifyNodeInstance(*node, sprocket->GetInstanceId());
    ASSERT_TRUE(NavNodeExtendedData(*node).GetRelatedInstanceKeys().empty());
    }


/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct ContentQueryExecutorTests : QueryExecutorTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryExecutorTests, HandlesUnionSelectionFromClassWithPointProperty)
    {
    ECEntityClassCP classH = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassH")->GetEntityClassCP();
    ECEntityClassCP classE = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, *classH, ContentDescriptor::Property("h", *classH, *classH->GetPropertyP("PointProperty")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *classE, ContentDescriptor::Property("e", *classE, *classE->GetPropertyP("IntProperty")->GetAsPrimitiveProperty()));

    ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
    q1->SelectContract(*ContentQueryContract::Create(1, *descriptor, classH, *q1), "h");
    q1->From(*classH, false, "h");

    ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
    q2->SelectContract(*ContentQueryContract::Create(2, *descriptor, classE, *q2), "e");
    q2->From(*classE, false, "e");

    UnionContentQueryPtr query = UnionContentQuery::Create(*q1, *q2);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryExecutor executor(*m_connection, *query);
    executor.ReadRecords();

    ASSERT_EQ(2, executor.GetRecordsCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryExecutorTests, HandlesResultsMergingFromOneClass)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("GadgetId"));
        instance.SetValue("Description", ECValue("Gadget 1"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("GadgetId"));
        instance.SetValue("Description", ECValue("Gadget 2"));
        });

    Utf8String localizationId = PRESENTATION_LOCALIZEDSTRING(RulesEngineL10N::GetNameSpace().m_namespace, RulesEngineL10N::LABEL_General_Varies().m_str);
    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, localizationId.c_str());

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, *m_gadgetClass, ContentDescriptor::Property("gadget", *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_gadgetClass, ContentDescriptor::Property("gadget", *m_gadgetClass, *m_gadgetClass->GetPropertyP("Description")->GetAsPrimitiveProperty()));
    descriptor->AddContentFlag(ContentFlags::MergeResults);

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_gadgetClass, *query), "gadget");
    query->From(*m_gadgetClass, false, "gadget");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryExecutor executor(*m_connection, *query);
    executor.ReadRecords();

    ASSERT_EQ(1, executor.GetRecordsCount());
    ContentSetItemPtr record = executor.GetRecord(0);
    ASSERT_TRUE(record.IsValid());

    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "Gadget_MyID": "GadgetId",
        "Gadget_Description": null
        })");
    expectedValues["Gadget_Description"].SetString(rapidjson::StringRef(formattedVariesStr.c_str()));

    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryExecutorTests, HandlesResultsMergingFromMultipleClasses)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Gadget"));
        instance.SetValue("Description", ECValue("Gadget description"));
        });
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Widget"));
        instance.SetValue("Description", ECValue("Widget description"));
        });

    Utf8String localizationId = PRESENTATION_LOCALIZEDSTRING(RulesEngineL10N::GetNameSpace().m_namespace, RulesEngineL10N::LABEL_General_Varies().m_str);
    Utf8PrintfString formattedVariesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, localizationId.c_str());

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr innerDescriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*innerDescriptor, *m_gadgetClass, ContentDescriptor::Property("gadget", *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    AddField(*innerDescriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    ContentDescriptor::Field* field = new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category("Misc.", "Misc.", "", 0), "Description", "Description");
    field->AsPropertiesField()->AddProperty(ContentDescriptor::Property("gadget", *m_gadgetClass, *m_gadgetClass->GetPropertyP("Description")->GetAsPrimitiveProperty()));
    field->AsPropertiesField()->AddProperty(ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("Description")->GetAsPrimitiveProperty()));
    field->SetName("Description");
    innerDescriptor->AddField(field);

    ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
    q1->SelectContract(*ContentQueryContract::Create(1, *innerDescriptor, m_gadgetClass, *q1), "gadget");
    q1->From(*m_gadgetClass, false, "gadget");

    ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
    q2->SelectContract(*ContentQueryContract::Create(2, *innerDescriptor, m_widgetClass, *q2), "widget");
    q2->From(*m_widgetClass, false, "widget");

    ContentDescriptorPtr outerDescriptor = ContentDescriptor::Create(*innerDescriptor);
    for (ContentDescriptor::Field* field : outerDescriptor->GetAllFields())
        {
        for (ContentDescriptor::Property const& fieldProperty : field->AsPropertiesField()->GetProperties())
            const_cast<ContentDescriptor::Property&>(fieldProperty).SetPrefix("");
        }
    outerDescriptor->AddContentFlag(ContentFlags::MergeResults);

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(0, *outerDescriptor, nullptr, *query));
    query->From(*UnionContentQuery::Create(*q1, *q2));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryExecutor executor(*m_connection, *query);
    executor.ReadRecords();

    ASSERT_EQ(1, executor.GetRecordsCount());
    ContentSetItemPtr record = executor.GetRecord(0);
    ASSERT_TRUE(record.IsValid());
    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    ASSERT_TRUE(json["Values"].HasMember("Gadget_MyID") && json["Values"]["Gadget_MyID"].IsString());
    ASSERT_STREQ(formattedVariesStr.c_str(), json["Values"]["Gadget_MyID"].GetString());
    ASSERT_TRUE(json["Values"].HasMember("Widget_MyID") && json["Values"]["Widget_MyID"].IsString());
    ASSERT_STREQ(formattedVariesStr.c_str(), json["Values"]["Widget_MyID"].GetString());
    ASSERT_TRUE(json["Values"].HasMember("Description") && json["Values"]["Description"].IsString());
    ASSERT_STREQ(formattedVariesStr.c_str(), json["Values"]["Description"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryExecutorTests, HandlesStructProperties)
    {
    ECEntityClassCP classI = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassI")->GetEntityClassCP();
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classI, [](IECInstanceR instance)
        {
        instance.SetValue("StringProperty", ECValue("1"));
        instance.SetValue("StructProperty.IntProperty", ECValue(2));
        instance.SetValue("StructProperty.StructProperty.IntProperty", ECValue(3));
        instance.SetValue("StructProperty.StructProperty.StringProperty", ECValue("4"));
        });

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, *classI, ContentDescriptor::Property("this", *classI, *classI->GetPropertyP("StringProperty")));
    AddField(*descriptor, *classI, ContentDescriptor::Property("this", *classI, *classI->GetPropertyP("StructProperty")));

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, classI, *query, bvector<RelatedClass>(), false), "this");
    query->From(*classI, false, "this");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryExecutor executor(*m_connection, *query);
    executor.ReadRecords();

    ASSERT_EQ(1, executor.GetRecordsCount());
    ContentSetItemPtr record = executor.GetRecord(0);
    ASSERT_TRUE(record.IsValid());

    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "ClassI_StringProperty": "1",
        "ClassI_StructProperty": {
           "IntProperty": 2,
           "StructProperty": {
               "IntProperty": 3,
               "StringProperty": "4"
               }
           }
        })");

    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryExecutorTests, HandlesArrayProperties)
    {
    ECClassCP classStruct1 = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Struct1");
    ECEntityClassCP classR = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassR")->GetEntityClassCP();
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classR, [&classStruct1](IECInstanceR instance)
        {
        instance.AddArrayElements("IntsArray", 2);
        instance.SetValue("IntsArray", ECValue(2), 0);
        instance.SetValue("IntsArray", ECValue(1), 1);

        instance.AddArrayElements("StructsArray", 2);

        IECInstancePtr struct1 = classStruct1->GetDefaultStandaloneEnabler()->CreateInstance();
        struct1->SetValue("StringProperty", ECValue("a"));
        ECValue structValue1;
        structValue1.SetStruct(struct1.get());
        instance.SetValue("StructsArray", structValue1, 0);

        IECInstancePtr struct2 = classStruct1->GetDefaultStandaloneEnabler()->CreateInstance();
        struct2->SetValue("StringProperty", ECValue("b"));
        ECValue structValue2;
        structValue2.SetStruct(struct2.get());
        instance.SetValue("StructsArray", structValue2, 1);
        });

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, *classR, ContentDescriptor::Property("this", *classR, *classR->GetPropertyP("IntsArray")));
    AddField(*descriptor, *classR, ContentDescriptor::Property("this", *classR, *classR->GetPropertyP("StructsArray")));

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, classR, *query, bvector<RelatedClass>(), false), "this");
    query->From(*classR, false, "this");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryExecutor executor(*m_connection, *query);
    executor.ReadRecords();

    ASSERT_EQ(1, executor.GetRecordsCount());
    ContentSetItemPtr record = executor.GetRecord(0);
    ASSERT_TRUE(record.IsValid());

    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "ClassR_IntsArray": [2, 1],
        "ClassR_StructsArray": [{
           "IntProperty": null,
           "StringProperty": "a"
           },{
           "IntProperty": null,
           "StringProperty": "b"
           }]
        })");

    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryExecutorTests, SelectsRelatedProperties)
    {
    ECRelationshipClassCP widgetHasGadgetsRelationship = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test Gadget"));});
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test Widget"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetsRelationship, *widget, *gadget);

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, *m_gadgetClass, ContentDescriptor::Property("this", *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_gadgetClass, ContentDescriptor::Property("rel_RET_Widget_0", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    const_cast<ContentDescriptor::Property&>(descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back()).SetIsRelated(RelatedClass(*m_widgetClass, *m_gadgetClass, *widgetHasGadgetsRelationship, true), RelationshipMeaning::RelatedInstance);

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_gadgetClass, *query), "this");
    query->From(*m_gadgetClass, false, "this");
    query->Join(RelatedClass(*m_gadgetClass, *m_widgetClass, *widgetHasGadgetsRelationship, false, "rel_RET_Widget_0"));

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryExecutor executor(*m_connection, *query);
    executor.ReadRecords();

    ASSERT_EQ(1, executor.GetRecordsCount());
    ContentSetItemPtr record = executor.GetRecord(0);
    ASSERT_TRUE(record.IsValid());

    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "Gadget_MyID": "Test Gadget",
        "Widget_MyID": "Test Widget"
        })");

    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryExecutorTests, SelectsRelatedPropertiesFromOnlySingleClassWhenSelectingFromMultipleClasses)
    {
    ECEntityClassCR classD = *s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassD")->GetEntityClassCP();
    ECEntityClassCR classE = *s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassE")->GetEntityClassCP();
    ECEntityClassCR classF = *s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassF")->GetEntityClassCP();
    ECRelationshipClassCR classDHasClassERelationship = *s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "ClassDHasClassE")->GetRelationshipClassCP();

    IECInstancePtr dInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), classD, [](IECInstanceR instance){instance.SetValue("StringProperty", ECValue("D Property"));});
    IECInstancePtr eInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), classE, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(11));});
    IECInstancePtr fInstance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), classF, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(22));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), classDHasClassERelationship, *dInstance, *eInstance);

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, classE, ContentDescriptor::Property("this", classE, *classE.GetPropertyP("IntProperty")->GetAsPrimitiveProperty()));
    AddField(*descriptor, classE, ContentDescriptor::Property("rel_RET_ClassD_0", classD, *classD.GetPropertyP("StringProperty")->GetAsPrimitiveProperty()));
    const_cast<ContentDescriptor::Property&>(descriptor->GetAllFields().back()->AsPropertiesField()->GetProperties().back()).SetIsRelated(RelatedClass(classD, classE, classDHasClassERelationship, true), RelationshipMeaning::RelatedInstance);

    ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
    query1->SelectContract(*ContentQueryContract::Create(1, *descriptor, &classE, *query1), "this");
    query1->From(classE, false, "this");
    query1->Join(RelatedClass(classE, classD, classDHasClassERelationship, false, "rel_RET_ClassD_0"));

    ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
    query2->SelectContract(*ContentQueryContract::Create(2, *descriptor, &classF, *query2), "this");
    query2->From(classF, false, "this");

    UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    ContentQueryExecutor executor(*m_connection, *query);
    executor.ReadRecords();

    ASSERT_EQ(2, executor.GetRecordsCount());

    // validate the first record
    ContentSetItemPtr record = executor.GetRecord(0);
    ASSERT_TRUE(record.IsValid());

    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({
        "ClassE_IntProperty": 11,
        "ClassD_StringProperty": "D Property"
        })");

    rapidjson::Document json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);

    // validate the second record
    record = executor.GetRecord(1);
    ASSERT_TRUE(record.IsValid());

    expectedValues.Parse(R"({
        "ClassE_IntProperty": 22,
        "ClassD_StringProperty": null
        })");

    json = record->AsJson();
    ASSERT_TRUE(json.IsObject());
    ASSERT_TRUE(json.HasMember("Values") && json["Values"].IsObject());
    EXPECT_EQ(expectedValues, json["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryExecutorTests, UsesSuppliedECPropertyFormatterToFormatPrimitiveECPropertyValues)
    {
    TestPropertyFormatter formatter;
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance)
        {
        instance.SetValue("MyID", ECValue("Test 1"));
        instance.SetValue("Description", ECValue("Test 2"));
        instance.SetValue("IntProperty", ECValue(3));
        instance.SetValue("BoolProperty", ECValue(true));
        instance.SetValue("DoubleProperty", ECValue(4.0));
        });

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("Description")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("IntProperty")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("BoolProperty")->GetAsPrimitiveProperty()));
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("DoubleProperty")->GetAsPrimitiveProperty()));

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query), "widget");
    query->From(*m_widgetClass, false, "widget");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData(), m_propertyFormatter);
    ContentQueryExecutor executor(*m_connection, *query);
    executor.SetPropertyFormatter(formatter);
    executor.ReadRecords();

    ASSERT_EQ(1, executor.GetRecordsCount());
    ContentSetItemPtr record = executor.GetRecord(0);
    ASSERT_TRUE(record.IsValid());

    rapidjson::Document expectedDisplayValues;
    expectedDisplayValues.Parse(R"({
        "Widget_MyID": "_Test 1_",
        "Widget_Description": "_Test 2_",
        "Widget_IntProperty": "_3_",
        "Widget_BoolProperty": "_True_",
        "Widget_DoubleProperty": "_4_"
        })");

    rapidjson::Document json = record->AsJson();
    EXPECT_EQ(expectedDisplayValues, json["DisplayValues"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedDisplayValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(json["DisplayValues"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorTests, GetDistinctStringValuesFromPropertyField)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test1"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test1"));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test2"));});

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    descriptor->AddContentFlag(ContentFlags::DistinctValues);

    ComplexContentQueryPtr nestedQuery = ComplexContentQuery::Create();
    ContentQueryContractPtr contract = ContentQueryContract::Create(1, *descriptor, m_widgetClass, *nestedQuery);
    nestedQuery->SelectContract(*contract, "widget");
    nestedQuery->From(*m_widgetClass, false, "widget");

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectAll();
    query->From(*nestedQuery);
    query->GroupByContract(*contract);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData(), m_propertyFormatter);
    ContentQueryExecutor executor(*m_connection, *query);
    executor.ReadRecords();

    ASSERT_EQ(2, executor.GetRecordsCount());

    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({"Widget_MyID": "Test1"})");
    ContentSetItemPtr record1 = executor.GetRecord(0);
    ASSERT_TRUE(record1.IsValid());
    EXPECT_EQ(expectedValues, record1->AsJson()["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record1->AsJson()["Values"]);

    expectedValues.Parse(R"({"Widget_MyID": "Test2"})");
    ContentSetItemPtr record2 = executor.GetRecord(1);
    ASSERT_TRUE(record2.IsValid());
    EXPECT_EQ(expectedValues, record2->AsJson()["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record2->AsJson()["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryExecutorTests, GetDistinctValuesFromCalculatedPropertyField)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test1"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test1"));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Test2"));});

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());

    descriptor->AddField(new ContentDescriptor::CalculatedPropertyField("label", "MyID", "this.MyID", nullptr));
    descriptor->AddContentFlag(ContentFlags::DistinctValues);

    ComplexContentQueryPtr nestedQuery = ComplexContentQuery::Create();
    ContentQueryContractPtr contract = ContentQueryContract::Create(1, *descriptor, m_widgetClass, *nestedQuery);
    nestedQuery->SelectContract(*contract, "widget");
    nestedQuery->From(*m_widgetClass, false, "widget");

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectAll();
    query->From(*nestedQuery);
    query->GroupByContract(*contract);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData(), m_propertyFormatter);
    ContentQueryExecutor executor(*m_connection, *query);
    executor.ReadRecords();

    ASSERT_EQ(2, executor.GetRecordsCount());

    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({"MyID": "Test1"})");
    ContentSetItemPtr record1 = executor.GetRecord(0);
    ASSERT_TRUE(record1.IsValid());
    EXPECT_EQ(expectedValues, record1->AsJson()["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record1->AsJson()["Values"]);

    expectedValues.Parse(R"({"MyID": "Test2"})");
    ContentSetItemPtr record2 = executor.GetRecord(1);
    ASSERT_TRUE(record2.IsValid());
    EXPECT_EQ(expectedValues, record2->AsJson()["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record2->AsJson()["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorTests, GetDistinctStringValuesFromMergedECPropertyField)
    {
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Gadget1"));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Widget1"));});
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [](IECInstanceR instance){instance.SetValue("MyID", ECValue("Gadget1"));});

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    ContentDescriptor::Field* field = new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category("Misc.", "Misc.", 0, false), "MyID", "MyID");
    field->AsPropertiesField()->AddProperty(ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    field->AsPropertiesField()->AddProperty(ContentDescriptor::Property("gadget", *m_gadgetClass, *m_gadgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));
    field->SetName("MyID");
    descriptor->AddField(field);
    descriptor->AddContentFlag(ContentFlags::DistinctValues);

    ComplexContentQueryPtr nestedQuery1 = ComplexContentQuery::Create();
    ContentQueryContractPtr contract1 = ContentQueryContract::Create(1, *descriptor, m_gadgetClass, *nestedQuery1);
    nestedQuery1->SelectContract(*contract1, "gadget");
    nestedQuery1->From(*m_gadgetClass, false, "gadget");

    ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
    query1->SelectAll();
    query1->From(*nestedQuery1);
    query1->GroupByContract(*contract1);

    ComplexContentQueryPtr nestedQuery2 = ComplexContentQuery::Create();
    ContentQueryContractPtr contract2 = ContentQueryContract::Create(2, *descriptor, m_widgetClass, *nestedQuery2);
    nestedQuery2->SelectContract(*contract2, "widget");
    nestedQuery2->From(*m_widgetClass, false, "widget");

    ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
    query2->SelectAll();
    query2->From(*nestedQuery2);
    query2->GroupByContract(*contract2);

    UnionContentQueryPtr query = UnionContentQuery::Create(*query1, *query2);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData(), m_propertyFormatter);
    ContentQueryExecutor executor(*m_connection, *query);
    executor.ReadRecords();

    ASSERT_EQ(2, executor.GetRecordsCount());

    rapidjson::Document expectedValues;
    expectedValues.Parse(R"({"MyID": "Gadget1"})");
    ContentSetItemPtr record1 = executor.GetRecord(0);
    ASSERT_TRUE(record1.IsValid());
    EXPECT_EQ(expectedValues, record1->AsJson()["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record1->AsJson()["Values"]);

    expectedValues.Parse(R"({"MyID": "Widget1"})");
    ContentSetItemPtr record2 = executor.GetRecord(1);
    ASSERT_TRUE(record2.IsValid());
    EXPECT_EQ(expectedValues, record2->AsJson()["Values"])
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expectedValues) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(record2->AsJson()["Values"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryExecutorTests, GetDistinctPointValuesFromPropertiesField)
    {
    ECEntityClassCP classH = s_project->GetECDbCR().Schemas().GetClass("RulesEngineTest", "ClassH")->GetEntityClassCP();
    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance){instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.1234566666000001, 2.0, 3.0)));});
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classH, [](IECInstanceR instance){instance.SetValue("PointProperty", ECValue(DPoint3d::From(1.1234566666000002, 2.0, 3.0)));});

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    descriptor->AddContentFlag(ContentFlags::DistinctValues);
    AddField(*descriptor, *classH, ContentDescriptor::Property("h", *classH, *classH->GetPropertyP("PointProperty")->GetAsPrimitiveProperty()));

    ComplexContentQueryPtr nestedQuery = ComplexContentQuery::Create();
    ContentQueryContractPtr contract = ContentQueryContract::Create(1, *descriptor, classH, *nestedQuery);
    nestedQuery->SelectContract(*contract, "h");
    nestedQuery->From(*classH, false, "h");

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectAll();
    query->From(*nestedQuery);
    query->GroupByContract(*contract);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData(), m_propertyFormatter);
    ContentQueryExecutor executor(*m_connection, *query);
    executor.ReadRecords();

    ASSERT_EQ(1, executor.GetRecordsCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryExecutorTests, DoesntIncludeFieldPropertyValueInstanceKeysWhenDescriptorContainsExcludeEditingDataFlag)
    {
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    RulesDrivenECPresentationManager::ContentOptions options(m_ruleset->GetRuleSetId());
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    descriptor->SetContentFlags(descriptor->GetContentFlags() | (int)ContentFlags::ExcludeEditingData);
    AddField(*descriptor, *m_widgetClass, ContentDescriptor::Property("widget", *m_widgetClass, *m_widgetClass->GetPropertyP("MyID")->GetAsPrimitiveProperty()));

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(1, *descriptor, m_widgetClass, *query), "widget");
    query->From(*m_widgetClass, false, "widget");

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_userSettings, nullptr, m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData(), m_propertyFormatter);
    ContentQueryExecutor executor(*m_connection, *query);
    executor.ReadRecords();

    ASSERT_EQ(1, executor.GetRecordsCount());
    ContentSetItemPtr record = executor.GetRecord(0);
    ASSERT_TRUE(record.IsValid());
    EXPECT_TRUE(record->GetFieldInstanceKeys().empty());
    }
