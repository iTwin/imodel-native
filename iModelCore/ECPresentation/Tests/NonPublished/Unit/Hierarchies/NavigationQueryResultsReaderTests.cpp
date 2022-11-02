/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../../../../Source/Hierarchies/NavigationQueryContracts.h"
#include "../../Helpers/TestHelpers.h"
#include "../Queries/QueryExecutorTests.h"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryResultsReaderTests : QueryExecutorTests
    {
    void VerifyNodeInstance(NavNodeCR node, IECInstanceCR instance)
        {
        RulesEngineTestHelpers::ValidateNodeInstances(s_project->GetECDb(), node, { &instance });
        }
    void VerifyClassGroupingNode(NavNodeCR node, Utf8CP label, bvector<RefCountedPtr<IECInstance const>> const& groupedInstances)
        {
        ASSERT_STREQ(NAVNODE_TYPE_ECClassGroupingNode, node.GetType().c_str());
        ASSERT_STREQ(label, node.GetLabelDefinition().GetDisplayValue().c_str());
        ASSERT_TRUE(nullptr != node.GetKey()->AsECClassGroupingNodeKey());
        RulesEngineTestHelpers::ValidateNodeInstances(s_project->GetECDb(), node, groupedInstances);
        }
    void VerifyLabelGroupingNode(NavNodeCR node, Utf8CP label, bvector<RefCountedPtr<IECInstance const>> const& groupedInstances)
        {
        ASSERT_STREQ(NAVNODE_TYPE_DisplayLabelGroupingNode, node.GetType().c_str());
        ASSERT_STREQ(label, node.GetLabelDefinition().GetDisplayValue().c_str());
        ASSERT_TRUE(nullptr != node.GetKey()->AsLabelGroupingNodeKey());
        if (groupedInstances.size() <= MAX_LABEL_GROUPED_INSTANCE_KEYS)
            {
            ASSERT_TRUE(nullptr != node.GetKey()->AsLabelGroupingNodeKey()->GetGroupedInstanceKeys());
            auto groupedInstanceKeys = ContainerHelpers::TransformContainer<bvector<ECInstanceKey>>(groupedInstances, [](auto const& instance)
                {
                ECInstanceId instanceId;
                ECInstanceId::FromString(instanceId, instance->GetInstanceId().c_str());
                return ECInstanceKey(instance->GetClass().GetId(), instanceId);
                });
            ASSERT_EQ(groupedInstanceKeys, *node.GetKey()->AsLabelGroupingNodeKey()->GetGroupedInstanceKeys());
            }
        RulesEngineTestHelpers::ValidateNodeInstances(s_project->GetECDb(), node, groupedInstances);
        }
    void VerifyPropertyGroupingNode(NavNodeCR node, Utf8CP label, bvector<RefCountedPtr<IECInstance const>> const& groupedInstances)
        {
        ASSERT_STREQ(NAVNODE_TYPE_ECPropertyGroupingNode, node.GetType().c_str());
        ASSERT_STREQ(label, node.GetLabelDefinition().GetDisplayValue().c_str());
        ASSERT_TRUE(nullptr != node.GetKey()->AsECPropertyGroupingNodeKey());
        RulesEngineTestHelpers::ValidateNodeInstances(s_project->GetECDb(), node, groupedInstances);
        }
    void VerifyPropertyGroupingNode(NavNodeCR node, Utf8CP label, bvector<RefCountedPtr<IECInstance const>> const& groupedInstances, bvector<ECValue> const& groupedValues)
        {
        VerifyPropertyGroupingNode(node, label, groupedInstances);
        RulesEngineTestHelpers::ValidateNodeGroupedValues(node, groupedValues);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, GetInstanceNodes)
    {
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    ECClassSet classes;
    classes[m_widgetClass] = true;
    classes[m_gadgetClass] = true;

    NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(*m_schemaHelper, classes, "this");
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::MultiECInstanceNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyNodeInstance(*node, *gadget);

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyNodeInstance(*node, *widget);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* TFS#711486
* @bsitest
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

    SelectClass<ECClass> selectClass(*m_widgetClass, "widget2", false);
    RelatedClassPath relationshipPath = {
        RelatedClass(*m_widgetClass, SelectClass<ECRelationshipClass>(*widgetHasGadgetsRelationship, "r_WidgetHasGadgets"), true, SelectClass<ECClass>(*m_gadgetClass, "gadget")),
        RelatedClass(*m_gadgetClass, SelectClass<ECRelationshipClass>(*widgetsHaveGadgetsRelationship, "r_WidgetsHaveGadgets"), false, SelectClass<ECClass>(*m_widgetClass, "widget1"))
        };
    RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create("", m_widgetClass, CreateDisplayLabelField(selectClass));
    contract->SetPathFromSelectToParentClass(relationshipPath);
    ComplexNavigationQueryPtr inner = ComplexNavigationQuery::Create();
    inner->SelectContract(*contract, "widget2")
        .From(selectClass)
        .Join(relationshipPath)
        .Where("widget1.ECInstanceId = ?", {std::make_shared<BoundQueryId>(widget1->GetInstanceId())});
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectContract(*contract)
        .From(*inner)
        .GroupByContract(*contract);
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyNodeInstance(*node, *widget2);

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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

    NavigationQueryPtr groupedInstanceKeysQuery = RulesEngineTestHelpers::CreateQuery(*ECClassGroupedInstancesQueryContract::Create(), classes, false, "keys");
    NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery);
    ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
    nested->SelectAll();
    nested->From(*RulesEngineTestHelpers::CreateQuery(*contract, classes, false, "this"));
    nested->GroupByContract(*contract);

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectAll();
    query->From(*nested);
    query->OrderBy(Utf8String("[").append(ECClassGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::ClassGroupingNodes);
    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr gadgetNode;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(gadgetNode, *reader));
    VerifyClassGroupingNode(*gadgetNode, m_gadgetClass->GetDisplayLabel().c_str(), { gadget1, gadget2 });

    NavNodePtr widgetNode;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(widgetNode, *reader));
    VerifyClassGroupingNode(*widgetNode, m_widgetClass->GetDisplayLabel().c_str(), { widget });

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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

    InstanceLabelOverridePropertyValueSpecification labelOverrideSpec("MyID");

    SelectClass<ECClass> gadgetSelectClass(*m_gadgetClass, "this", false);
    auto gadgetLabelField = CreateDisplayLabelField(gadgetSelectClass, {}, { &labelOverrideSpec });

    SelectClass<ECClass> widgetSelectClass(*m_widgetClass, "this", false);
    auto widgetLabelField = CreateDisplayLabelField(widgetSelectClass, {}, { &labelOverrideSpec });

    NavigationQueryPtr groupedInstanceKeysQuery = UnionNavigationQuery::Create({
        &ComplexNavigationQuery::Create()
            ->SelectContract(*DisplayLabelGroupedInstancesQueryContract::Create(gadgetLabelField))
            .From(gadgetSelectClass),
        &ComplexNavigationQuery::Create()
            ->SelectContract(*DisplayLabelGroupedInstancesQueryContract::Create(widgetLabelField))
            .From(widgetSelectClass),
        });

    auto contract = DisplayLabelGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery, nullptr, RulesEngineTestHelpers::CreateNullDisplayLabelField());
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectContract(*contract);
    query->From(*UnionNavigationQuery::Create({
        &ComplexNavigationQuery::Create()
            ->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create("", nullptr, m_gadgetClass, gadgetLabelField))
            .From(gadgetSelectClass),
        &ComplexNavigationQuery::Create()
            ->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create("", nullptr, m_widgetClass, widgetLabelField))
            .From(widgetSelectClass),
        }));
    query->GroupByContract(*contract);
    query->OrderBy(Utf8String("[").append(DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::DisplayLabelGroupingNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr gadgetNode;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(gadgetNode, *reader));
    VerifyLabelGroupingNode(*gadgetNode, "GadgetID", { gadget1, gadget2 });

    NavNodePtr widgetNode;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(widgetNode, *reader));
    VerifyLabelGroupingNode(*widgetNode, "WidgetID", { widget });

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, OverridesLabel)
    {
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_gadgetClass);

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.IsInstanceNode AND ThisNode.ClassName=\"Widget\"", 1, "\"NavigationQueryResultsReaderTests.OverridesLabel\"", ""));

    ECClassSet classes;
    classes[m_widgetClass] = true;
    classes[m_gadgetClass] = true;

    NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(*m_schemaHelper, classes, "this");
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::MultiECInstanceNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
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
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryResultsReaderTests, PropertyGroupingByBooleanProperty)
    {
    // create our own instances
    ECInstanceInserter inserter(s_project->GetECDb(), *m_widgetClass, nullptr);
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("BoolProperty", ECValue(true)); });
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("BoolProperty", ECValue(false)); });
    IECInstancePtr widget3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), inserter, *m_widgetClass, [](IECInstanceR instance) {instance.SetValue("BoolProperty", ECValue(false)); });

    PropertyGroup propertyGroup("", "TestImageId", false, "BoolProperty", "");
    SelectClass<ECClass> selectClass(*m_widgetClass, "alias", false);

    NavigationQueryPtr groupedInstanceKeysQuery = &ComplexNavigationQuery::Create()
        ->SelectContract(*ECPropertyGroupedInstancesQueryContract::Create("BoolProperty"))
        .From(selectClass);

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery, selectClass, *m_widgetClass->GetPropertyP("BoolProperty"), propertyGroup, nullptr);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()
        ->SelectContract(*contract)
        .From(ComplexNavigationQuery::Create()->SelectContract(*contract, "alias").From(selectClass))
        .GroupByContract(*contract)
        .OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "False", { widget2, widget3 }, { ECValue(false) });

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "True", { widget1 }, { ECValue(true) });

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    SelectClass<ECClass> selectClass(*m_widgetClass, "alias", false);

    NavigationQueryPtr groupedInstanceKeysQuery = &ComplexNavigationQuery::Create()
        ->SelectContract(*ECPropertyGroupedInstancesQueryContract::Create("IntProperty"))
        .From(selectClass);

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery, selectClass, *m_widgetClass->GetPropertyP("IntProperty"), propertyGroup, nullptr);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()
        ->SelectContract(*contract)
        .From(ComplexNavigationQuery::Create()->SelectContract(*contract, "alias").From(selectClass))
        .GroupByContract(*contract)
        .OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "-1", { widget1, widget4 }, { ECValue(-1) });

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "10", { widget5 }, { ECValue(10) });

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "5", { widget2, widget3  }, { ECValue(5) });

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    SelectClass<ECClass> selectClass(*m_widgetClass, "alias", false);

    NavigationQueryPtr groupedInstanceKeysQuery = &ComplexNavigationQuery::Create()
        ->SelectContract(*ECPropertyGroupedInstancesQueryContract::Create("IntProperty"))
        .From(selectClass);

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery, selectClass, groupingProperty, propertyGroup, nullptr);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()
        ->SelectAll()
        .From(ComplexNavigationQuery::Create()->SelectContract(*contract, "alias").From(selectClass))
        .GroupByContract(*contract)
        .OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    NavigationQueryExtendedData(*query).AddRangesData(groupingProperty, propertyGroup);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "0 - 5", { widget2 });

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "6 - 10", { widget3, widget4, widget5 });

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, CommonStrings::RULESENGINE_OTHER, { widget1, widget6, widget7 });

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
    SelectClass<ECClass> selectClass(*m_widgetClass, "alias", false);

    NavigationQueryPtr groupedInstanceKeysQuery = &ComplexNavigationQuery::Create()
        ->SelectContract(*ECPropertyGroupedInstancesQueryContract::Create("IntProperty"))
        .From(selectClass);

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery, selectClass, groupingProperty, propertyGroup, nullptr);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()
        ->SelectAll()
        .From(ComplexNavigationQuery::Create()->SelectContract(*contract, "alias").From(selectClass))
        .GroupByContract(*contract)
        .OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);
    NavigationQueryExtendedData(*query).AddRangesData(groupingProperty, propertyGroup);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ(CommonStrings::RULESENGINE_OTHER, node->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("", node->GetImageId().c_str());

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("CustomLabel1", node->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("CustomImage1", node->GetImageId().c_str());

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    EXPECT_STREQ("CustomLabel2", node->GetLabelDefinition().GetDisplayValue().c_str());
    EXPECT_STREQ("", node->GetImageId().c_str());

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryResultsReaderTests, PropertyGroupingByForeignKeyWithoutDisplayLabel)
    {
    ECRelationshipClassCP gadgetHasSprockets = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "GadgetHasSprockets")->GetRelationshipClassCP();

    // create our own instances
    ECInstanceInserter gadgetInserter(s_project->GetECDb(), *m_gadgetClass, nullptr);
    IECInstancePtr gadget1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
    IECInstancePtr gadget2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), gadgetInserter, *m_gadgetClass);
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
    ECPropertyCR groupingProperty = *m_sprocketClass->GetPropertyP("Gadget");
    SelectClass<ECClass> selectClass(*m_sprocketClass, "alias", false);
    SelectClass<ECClass> navPropSelectClass(*m_gadgetClass, "parentInstance", true);
    RelatedClass navRelatedClass(selectClass.GetClass(), SelectClass<ECRelationshipClass>(*gadgetHasSprockets, "rel"), false, navPropSelectClass, true);

    NavigationQueryPtr groupedInstanceKeysQuery = &ComplexNavigationQuery::Create()
        ->SelectContract(*ECPropertyGroupedInstancesQueryContract::Create("[Gadget].[Id]"))
        .From(selectClass);

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery, selectClass, groupingProperty, propertyGroup, &navPropSelectClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()
        ->SelectAll()
        .From(ComplexNavigationQuery::Create()
            ->SelectContract(*contract, "alias")
            .From(selectClass)
            .Join(navRelatedClass))
        .GroupByContract(*contract)
        .OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, CommonStrings::RULESENGINE_NOTSPECIFIED, { sprocket1, sprocket2, sprocket3, sprocket4 });

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    PropertyGroup propertyGroup("", "", false, "Widget", "");
    ECPropertyCR groupingProperty = *m_gadgetClass->GetPropertyP("Widget");
    SelectClass<ECClass> selectClass(*m_gadgetClass, "alias", false);
    SelectClass<ECClass> navPropSelectClass(*m_widgetClass, "parentInstance", true);
    RelatedClass navRelatedClass(selectClass.GetClass(), SelectClass<ECRelationshipClass>(*widgetHasGadgets, "rel"), false, navPropSelectClass, true);

    NavigationQueryPtr groupedInstanceKeysQuery = &ComplexNavigationQuery::Create()
        ->SelectContract(*ECPropertyGroupedInstancesQueryContract::Create("[Widget].[Id]"))
        .From(selectClass);

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery, selectClass, groupingProperty, propertyGroup, &navPropSelectClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()
        ->SelectAll()
        .From(ComplexNavigationQuery::Create()
            ->SelectContract(*contract, "alias")
            .From(selectClass)
            .Join(navRelatedClass))
        .GroupByContract(*contract)
        .OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "Widget1", { gadget1, gadget2 });

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "Widget2", { gadget3, gadget4 });

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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

    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    PropertyGroup propertyGroup("", "", false, "Widget", "");
    ECPropertyCR groupingProperty = *m_gadgetClass->GetPropertyP("Widget");
    SelectClass<ECClass> selectClass(*m_gadgetClass, "alias", false);
    SelectClass<ECClass> navPropSelectClass(*m_widgetClass, "parentInstance", true);
    RelatedClass navRelatedClass(selectClass.GetClass(), SelectClass<ECRelationshipClass>(*widgetHasGadgets, "rel"), false, navPropSelectClass, true);

    NavigationQueryPtr groupedInstanceKeysQuery = &ComplexNavigationQuery::Create()
        ->SelectContract(*ECPropertyGroupedInstancesQueryContract::Create("[Widget].[Id]"))
        .From(selectClass);

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery, selectClass, groupingProperty, propertyGroup, &navPropSelectClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()
        ->SelectAll()
        .From(ComplexNavigationQuery::Create()
            ->SelectContract(*contract, "alias")
            .From(selectClass)
            .Join(navRelatedClass))
        .GroupByContract(*contract)
        .OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "Widget1", { gadget1, gadget2 });

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "Widget2", { gadget3, gadget4 });

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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

    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:ClassD", "StringProperty"));

    PropertyGroup propertyGroup("", "", false, "ClassD", "");
    ECPropertyCR groupingProperty = *classE->GetPropertyP("ClassD");
    SelectClass<ECClass> selectClass(*classE, "alias", false);
    SelectClass<ECClass> navPropSelectClass(*classD, "parentInstance", true);
    RelatedClass navRelatedClass(selectClass.GetClass(), SelectClass<ECRelationshipClass>(*classDHasClassE, "rel"), false, navPropSelectClass, true);

    NavigationQueryPtr groupedInstanceKeysQuery = &ComplexNavigationQuery::Create()
        ->SelectContract(*ECPropertyGroupedInstancesQueryContract::Create("[ClassD].[Id]"))
        .From(selectClass);

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery, selectClass, groupingProperty, propertyGroup, &navPropSelectClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()
        ->SelectAll()
        .From(ComplexNavigationQuery::Create()
            ->SelectContract(*contract, "alias")
            .From(selectClass)
            .Join(navRelatedClass))
        .GroupByContract(*contract)
        .OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node,  CommonStrings::RULESENGINE_NOTSPECIFIED, { instanceE2 });

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "ClassD_Label", { instanceE1 });

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.IsInstanceNode ANDALSO ThisNode.ClassName=\"ClassD\"", 1, "\"MyLabel\"", ""));

    PropertyGroup propertyGroup("", "", false, "ClassD", "");
    ECPropertyCR groupingProperty = *classE->GetPropertyP("ClassD");
    SelectClass<ECClass> selectClass(*classE, "alias", false);
    SelectClass<ECClass> navPropSelectClass(*classD, "parentInstance", true);
    RelatedClass navRelatedClass(selectClass.GetClass(), SelectClass<ECRelationshipClass>(*classDHasClassE, "rel"), false, navPropSelectClass, true);

    NavigationQueryPtr groupedInstanceKeysQuery = &ComplexNavigationQuery::Create()
        ->SelectContract(*ECPropertyGroupedInstancesQueryContract::Create("[ClassD].[Id]"))
        .From(selectClass);

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery, selectClass, groupingProperty, propertyGroup, &navPropSelectClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()
        ->SelectAll()
        .From(ComplexNavigationQuery::Create()
            ->SelectContract(*contract, "alias")
            .From(selectClass)
            .Join(navRelatedClass))
        .GroupByContract(*contract)
        .OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, CommonStrings::RULESENGINE_NOTSPECIFIED, { instanceE2 });

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "MyLabel", { instanceE1 });

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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

    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:ClassD", "StringProperty"));

    PropertyGroup propertyGroup("", "", false, "ClassD", "");
    ECPropertyCR groupingProperty = *classE->GetPropertyP("ClassD");
    SelectClass<ECClass> selectClass(*classE, "alias", false);
    SelectClass<ECClass> navPropSelectClass(*classD, "parentInstance", true);
    RelatedClass navRelatedClass(selectClass.GetClass(), SelectClass<ECRelationshipClass>(*classDHasClassE, "rel"), false, navPropSelectClass, true);

    NavigationQueryPtr groupedInstanceKeysQuery = &ComplexNavigationQuery::Create()
        ->SelectContract(*ECPropertyGroupedInstancesQueryContract::Create("[ClassD].[Id]"))
        .From(selectClass);

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery, selectClass, groupingProperty, propertyGroup, &navPropSelectClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()
        ->SelectAll()
        .From(ComplexNavigationQuery::Create()
            ->SelectContract(*contract, "alias")
            .From(selectClass)
            .Join(navRelatedClass))
        .GroupByContract(*contract)
        .OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, CommonStrings::RULESENGINE_NOTSPECIFIED, { instanceE2 });

    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "ClassD_StringProperty", { instanceE1 });

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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

    m_ruleset->AddPresentationRule(*new LabelOverride("ThisNode.ClassName = \"Widget\"", 1, "this.MyID", ""));

    PropertyGroup propertyGroup("", "", false, "Widget", "");
    ECPropertyCR groupingProperty = *m_gadgetClass->GetPropertyP("Widget");
    SelectClass<ECClass> selectClass(*m_gadgetClass, "alias", false);
    SelectClass<ECClass> navPropSelectClass(*m_widgetClass, "parentInstance", true);
    RelatedClass navRelatedClass(selectClass.GetClass(), SelectClass<ECRelationshipClass>(*widgetHasGadgets, "rel"), false, navPropSelectClass, true);

    NavigationQueryPtr groupedInstanceKeysQuery = &ComplexNavigationQuery::Create()
        ->SelectContract(*ECPropertyGroupedInstancesQueryContract::Create("[Widget].[Id]"))
        .From(selectClass);

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery, selectClass, groupingProperty, propertyGroup, &navPropSelectClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()
        ->SelectAll()
        .From(ComplexNavigationQuery::Create()
            ->SelectContract(*contract, "alias")
            .From(selectClass)
            .Join(navRelatedClass))
        .GroupByContract(*contract)
        .OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "WidgetName", { gadget1, gadget2, gadget3, gadget4 });

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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

    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    PropertyGroup propertyGroup("", "", false, "Widget", "");
    ECPropertyCR groupingProperty = *m_gadgetClass->GetPropertyP("Widget");
    SelectClass<ECClass> selectClass(*m_gadgetClass, "alias", false);
    SelectClass<ECClass> navPropSelectClass(*m_widgetClass, "parentInstance", true);
    RelatedClass navRelatedClass(selectClass.GetClass(), SelectClass<ECRelationshipClass>(*widgetHasGadgets, "rel"), false, navPropSelectClass, true);

    NavigationQueryPtr groupedInstanceKeysQuery = &ComplexNavigationQuery::Create()
        ->SelectContract(*ECPropertyGroupedInstancesQueryContract::Create("[Widget].[Id]"))
        .From(selectClass);

    NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", groupedInstanceKeysQuery, selectClass, groupingProperty, propertyGroup, &navPropSelectClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()
        ->SelectAll()
        .From(ComplexNavigationQuery::Create()
            ->SelectContract(*contract, "alias")
            .From(selectClass)
            .Join(navRelatedClass))
        .GroupByContract(*contract)
        .OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName).append("]").c_str());
    query->GetResultParametersR().SetResultType(NavigationQueryResultType::PropertyGroupingNodes);

    CustomFunctionsContext ctx(*m_schemaHelper, m_connections, *m_connection, m_ruleset->GetRuleSetId(), *m_rulesPreprocessor, m_rulesetVariables, nullptr,
        m_schemaHelper->GetECExpressionsCache(), m_nodesFactory, nullptr, nullptr, &query->GetExtendedData());
    QueryExecutor executor(*m_connection, *query);

    auto reader = NavNodesReader::Create(m_nodesFactory, *m_connection, *query->GetContract(),
        query->GetResultParameters().GetResultType(), query->GetResultParameters().GetNavNodeExtendedData(), nullptr);

    NavNodePtr node;
    EXPECT_EQ(QueryExecutorStatus::Row, executor.ReadNext(node, *reader));
    VerifyPropertyGroupingNode(*node, "WidgetName", { gadget1, gadget2, gadget3, gadget4 });

    EXPECT_EQ(QueryExecutorStatus::Done, executor.ReadNext(node, *reader));
    EXPECT_TRUE(node.IsNull());
    }
