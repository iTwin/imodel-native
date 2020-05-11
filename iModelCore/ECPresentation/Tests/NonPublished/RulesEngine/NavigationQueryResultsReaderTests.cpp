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

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void VerifyNodeInstance(NavNodeCR node, IECInstanceCR instance)
    {
    bvector<ECInstanceKey> nodeInstanceKeys = NavNodeExtendedData(node).GetInstanceKeys();
    ASSERT_EQ(1, nodeInstanceKeys.size());
    EXPECT_STREQ(instance.GetInstanceId().c_str(), nodeInstanceKeys[0].GetInstanceId().ToString().c_str());
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
static void AssertNodeGroupsECInstances(NavNodeCR node, bvector<IECInstancePtr> const& instances)
    {
    NavNodeExtendedData extendedData(node);
    bvector<ECInstanceKey> groupedKeys = extendedData.GetInstanceKeys();
    EXPECT_EQ(instances.size(), groupedKeys.size());
    for (IECInstancePtr const& instance : instances)
        {
        auto iter = std::find(groupedKeys.begin(), groupedKeys.end(), CreateECInstanceKey(instance->GetClass(), instance->GetInstanceId()));
        EXPECT_NE(groupedKeys.end(), iter);
        }
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryResultsReaderTests : QueryExecutorTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, GetInstanceNodes)
    {
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    ECClassSet classes;
    classes[m_widgetClass] = true;
    classes[m_gadgetClass] = true;

    NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(classes, "this");
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::MultiECInstanceNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *query->GetContract(),
        query->GetResultParameters().GetResultType(), &query->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyNodeInstance(*node, *gadget);

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyNodeInstance(*node, *widget);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#711486
* @bsitest                                      Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, GetInstanceNodes_GroupsByInstanceKey)
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
    contract->SetPathFromSelectToParentClass(relationshipPath);
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

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *query->GetContract(),
        query->GetResultParameters().GetResultType(), &query->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyNodeInstance(*node, *widget2);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, GetClassGroupingNodes)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    ECInstanceId widgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [&widgetId](IECInstanceR gadget){gadget.SetValue("Widget", ECValue((int64_t)widgetId.GetValue()));});
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [&widgetId](IECInstanceR gadget){gadget.SetValue("Widget", ECValue((int64_t)widgetId.GetValue()));});

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

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *query->GetContract(),
        query->GetResultParameters().GetResultType(), &query->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr gadgetNode;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(gadgetNode, *reader));
    EXPECT_STREQ(Utf8String(m_gadgetClass->GetDisplayLabel().c_str()).c_str(), gadgetNode->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, gadgetNode->GetType().c_str());
    EXPECT_EQ(2, NavNodeExtendedData(*gadgetNode).GetInstanceKeys().size());
    AssertNodeGroupsECInstances(*gadgetNode, {gadget1, gadget2});

    JsonNavNodePtr widgetNode;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(widgetNode, *reader));
    EXPECT_STREQ(Utf8String(m_widgetClass->GetDisplayLabel().c_str()).c_str(), widgetNode->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, widgetNode->GetType().c_str());
    EXPECT_EQ(1, NavNodeExtendedData(*widgetNode).GetInstanceKeys().size());
    AssertNodeGroupsECInstances(*widgetNode, {widget});

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, GetDisplayLabelGroupingNodes)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass, [](IECInstanceR widget){widget.SetValue("MyID", ECValue("WidgetID"));});

    ECInstanceId widgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [&widgetId](IECInstanceR gadget)
        {
        gadget.SetValue("MyID", ECValue("GadgetID"));
        gadget.SetValue("Widget", ECValue((int64_t)widgetId.GetValue()));
        });
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass, [&widgetId](IECInstanceR gadget)
        {
        gadget.SetValue("MyID", ECValue("GadgetID"));
        gadget.SetValue("Widget", ECValue((int64_t)widgetId.GetValue()));
        });

    RefCountedPtr<DisplayLabelGroupingNodesQueryContract> gadgetContract = DisplayLabelGroupingNodesQueryContract::Create(m_gadgetClass, bvector<RelatedClassPath>(), {new InstanceLabelOverridePropertyValueSpecification("MyID")});
    ComplexNavigationQueryPtr gadgetInstancesQuery = ComplexNavigationQuery::Create();
    gadgetInstancesQuery->SelectContract(*gadgetContract, "this");
    gadgetInstancesQuery->From(*m_gadgetClass, false, "this");

    RefCountedPtr<DisplayLabelGroupingNodesQueryContract> widgetContract = DisplayLabelGroupingNodesQueryContract::Create(m_widgetClass, bvector<RelatedClassPath>(), {new InstanceLabelOverridePropertyValueSpecification("MyID")});
    ComplexNavigationQueryPtr widgetInstancesQuery = ComplexNavigationQuery::Create();
    widgetInstancesQuery->SelectContract(*widgetContract, "this");
    widgetInstancesQuery->From(*m_widgetClass, false, "this");

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll();
    query->From(*UnionNavigationQuery::Create({gadgetInstancesQuery, widgetInstancesQuery}));
    query->GroupByContract(*DisplayLabelGroupingNodesQueryContract::Create(nullptr));
    query->OrderBy(Utf8String("[").append(ECClassGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::DisplayLabelGroupingNodes);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::DisplayLabel);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *query->GetContract(),
        query->GetResultParameters().GetResultType(), &query->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr gadgetNode;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(gadgetNode, *reader));
    EXPECT_STREQ("GadgetID", gadgetNode->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, gadgetNode->GetType().c_str());
    AssertNodeGroupsECInstances(*gadgetNode, {gadget1, gadget2});

    JsonNavNodePtr widgetNode;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(widgetNode, *reader));
    EXPECT_STREQ("WidgetID", widgetNode->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, widgetNode->GetType().c_str());
    AssertNodeGroupsECInstances(*widgetNode, {widget});

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, OverridesLabel)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.IsInstanceNode AND ThisNode.ClassName=\"Widget\"", 1, "\"NavigationQueryResultsReaderTests.OverridesLabel\"", ""));

    ECClassSet classes;
    classes[m_widgetClass] = true;
    classes[m_gadgetClass] = true;

    NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(classes, "this");
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::MultiECInstanceNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *query->GetContract(),
        query->GetResultParameters().GetResultType(), &query->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyNodeInstance(*node, *gadget);
    EXPECT_STRNE("NavigationQueryResultsReaderTests.OverridesLabel", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyNodeInstance(*node, *widget);
    EXPECT_STREQ("NavigationQueryResultsReaderTests.OverridesLabel", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryResultsReaderTests, PropertyGroupingByBooleanProperty)
    {
    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("BoolProperty", ECValue(true)); });
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("BoolProperty", ECValue(false)); });
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("BoolProperty", ECValue(false)); });

    PropertyGroup propertyGroup("", "TestImageId", false, "BoolProperty", "");

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*m_widgetClass, *m_widgetClass->GetPropertyP("BoolProperty"), nullptr, propertyGroup, nullptr);
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract);
    nested->From(*m_widgetClass, false);
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    QueryExecutor executor(*m_connection, *grouped);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *grouped->GetContract(),
        grouped->GetResultParameters().GetResultType(), &grouped->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("False", node->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(NavNodeExtendedData(*node).HasPropertyValue());
    EXPECT_EQ(false, NavNodeExtendedData(*node).GetPropertyValue()->GetBool());
    AssertNodeGroupsECInstances(*node, {widget2, widget3});

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("True", node->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(NavNodeExtendedData(*node).HasPropertyValue());
    EXPECT_EQ(true, NavNodeExtendedData(*node).GetPropertyValue()->GetBool());
    AssertNodeGroupsECInstances(*node, {widget1});

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, PropertyGroupingByIntProperty)
    {
    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    IECInstancePtr widget4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    IECInstancePtr widget5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(10));});

    PropertyGroup propertyGroup("", "TestImageId", false, "IntProperty", "");

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create(*m_widgetClass, *m_widgetClass->GetPropertyP("IntProperty"), nullptr, propertyGroup, nullptr);
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectContract(*contract);
    nested->From(*m_widgetClass, false);
    ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
    grouped->SelectAll().From(*nested).GroupByContract(*contract).OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    grouped->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GroupingType::Property);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    QueryExecutor executor(*m_connection, *grouped);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *grouped->GetContract(),
        grouped->GetResultParameters().GetResultType(), &grouped->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("-1", node->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(NavNodeExtendedData(*node).HasPropertyValue());
    EXPECT_EQ(-1, NavNodeExtendedData(*node).GetPropertyValue()->GetInt());
    AssertNodeGroupsECInstances(*node, {widget1, widget4});

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("10", node->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(NavNodeExtendedData(*node).HasPropertyValue());
    EXPECT_EQ(10, NavNodeExtendedData(*node).GetPropertyValue()->GetInt());
    AssertNodeGroupsECInstances(*node, {widget5});

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("5", node->GetLabelDefinition().GetDisplayValue().c_str());
    ASSERT_TRUE(NavNodeExtendedData(*node).HasPropertyValue());
    EXPECT_EQ(5, NavNodeExtendedData(*node).GetPropertyValue()->GetInt());
    AssertNodeGroupsECInstances(*node, {widget2, widget3});

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, PropertyRangeGrouping)
    {
    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(-1));});
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(5));});
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(6));});
    IECInstancePtr widget4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(8));});
    IECInstancePtr widget5 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(10));});
    IECInstancePtr widget6 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(21));});
    IECInstancePtr widget7 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance){instance.SetValue("IntProperty", ECValue(100));});

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
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    QueryExecutor executor(*m_connection, *grouped);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *grouped->GetContract(),
        grouped->GetResultParameters().GetResultType(), &grouped->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("0 - 5", node->GetLabelDefinition().GetDisplayValue().c_str());
    AssertNodeGroupsECInstances(*node, {widget2});

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("6 - 10", node->GetLabelDefinition().GetDisplayValue().c_str());
    AssertNodeGroupsECInstances(*node, {widget3, widget4, widget5});

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ(RULESENGINE_LOCALIZEDSTRING_Other.c_str(), node->GetLabelDefinition().GetDisplayValue().c_str());
    AssertNodeGroupsECInstances(*node, {widget1, widget6, widget7});

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, PropertyRangeGroupingWithCustomLabelsAndImageIds)
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
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    QueryExecutor executor(*m_connection, *grouped);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *grouped->GetContract(),
        grouped->GetResultParameters().GetResultType(), &grouped->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ(RULESENGINE_LOCALIZEDSTRING_Other.c_str(), node->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("", node->GetCollapsedImageId().c_str());
    EXPECT_STREQ("", node->GetExpandedImageId().c_str());

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("CustomLabel1", node->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("CustomImage1", node->GetCollapsedImageId().c_str());
    EXPECT_STREQ("CustomImage1", node->GetExpandedImageId().c_str());

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("CustomLabel2", node->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("", node->GetCollapsedImageId().c_str());
    EXPECT_STREQ("", node->GetExpandedImageId().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, PropertyGroupingByForeignKeyWithoutDisplayLabel)
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

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    QueryExecutor executor(*m_connection, *grouped);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *grouped->GetContract(),
        grouped->GetResultParameters().GetResultType(), &grouped->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    ASSERT_TRUE(node.IsValid());
    EXPECT_STREQ("GadgetID", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, PropertyGroupingByForeignKeyWithDisplayLabelOverridenByLabelOverride)
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

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    QueryExecutor executor(*m_connection, *grouped);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *grouped->GetContract(),
        grouped->GetResultParameters().GetResultType(), &grouped->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("Widget1", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("Widget2", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, PropertyGroupingByForeignKeyWithDisplayLabelOveridenByInstanceLabelOverride)
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

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    QueryExecutor executor(*m_connection, *grouped);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *grouped->GetContract(),
        grouped->GetResultParameters().GetResultType(), &grouped->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("Widget1", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("Widget2", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, PropertyGroupingByForeignKeyWithDisplayLabel_UsingNavigationProperty)
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

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    QueryExecutor executor(*m_connection, *grouped);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *grouped->GetContract(),
        grouped->GetResultParameters().GetResultType(), &grouped->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("ClassD_Label", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, PropertyGroupingByForeignKeyWithLabelOverride)
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

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    QueryExecutor executor(*m_connection, *grouped);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *grouped->GetContract(),
        grouped->GetResultParameters().GetResultType(), &grouped->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("MyLabel", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, PropertyGroupingByForeignKeyWithInstanceLabelOverride)
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

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    QueryExecutor executor(*m_connection, *grouped);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *grouped->GetContract(),
        grouped->GetResultParameters().GetResultType(), &grouped->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("ClassD_StringProperty", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, PropertyGroupingByForeignKeyWithDisplayLabel_DifferentInstancesWithSameLabelGetMerged)
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

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    QueryExecutor executor(*m_connection, *grouped);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *grouped->GetContract(),
        grouped->GetResultParameters().GetResultType(), &grouped->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("WidgetName", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, PropertyGroupingByForeignKeyWithDisplayLabel_DifferentInstancesWithSameInstanceLabelGetMerged)
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

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, *m_ruleset, "locale", m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &grouped->GetExtendedData());
    QueryExecutor executor(*m_connection, *grouped);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, "locale", *grouped->GetContract(),
        grouped->GetResultParameters().GetResultType(), &grouped->GetResultParameters().GetNavNodeExtendedData());

    JsonNavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("WidgetName", node->GetLabelDefinition().GetDisplayValue().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }
