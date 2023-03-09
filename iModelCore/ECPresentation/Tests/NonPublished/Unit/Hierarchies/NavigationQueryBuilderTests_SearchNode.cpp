/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "NavigationQueryBuilderTests.h"

// note: some of the common specification properties of this specification are tested
//       QueryBuilderTests_AllInstanceNodes.cpp as they're common between the two.

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_NoQuery)
    {
    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_AllowsStandardSchemasIfExplicitylySpecified)
    {
    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    Utf8CP query = "SELECT * FROM [ECDbMap].[SchemaMap]";
    spec.AddQuerySpecification(*new StringQuerySpecification(query, "ECDbMeta", "ECSchemaDef"));
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_TRUE(queries.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_NoGrouping, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_NoGrouping)
    {
    ECClassCP classA = GetECClass("A");

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classA), classA->GetSchema().GetName(), classA->GetName()));

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_GroupByClass, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_GroupByClass)
    {
    ECClassCP classA = GetECClass("A");

    SearchResultInstanceNodesSpecification spec(1, false, false, false, true, false);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classA), classA->GetSchema().GetName(), classA->GetName()));

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        RefCountedPtr<ECClassGroupingNodesQueryContract> contract = ECClassGroupingNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery());
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*contract, "this");
        query->From(*classA, true, "this");
        query->Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList());

        ComplexQueryBuilderPtr grouped = ComplexQueryBuilder::Create();
        grouped->SelectAll();
        grouped->From(*query);
        grouped->GroupByContract(*contract);

        ComplexQueryBuilderPtr ordered = ComplexQueryBuilder::Create();
        ordered->SelectAll();
        ordered->From(*grouped);
        ordered->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        ordered->GetNavigationResultParameters().GetSelectInstanceClasses().clear();
        return ordered;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_GroupByClass_ChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_GroupByClass_ChildrenQuery)
    {
    ECClassCP classA = GetECClass("A");

    SearchResultInstanceNodesSpecification spec(1, false, false, false, true, false);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classA), classA->GetSchema().GetName(), classA->GetName()));

    auto parentNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECClassGroupingNode(nullptr, *classA, false, "MyLabel", {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr ordered = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList()));
        ordered->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return ordered;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_GroupByLabel, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_GroupByLabel)
    {
    ECClassCP classA = GetECClass("A");

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, true);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classA), classA->GetSchema().GetName(), classA->GetName()));

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        auto contract = DisplayLabelGroupingNodesQueryContract::Create(2, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));
        auto groupingContract = DisplayLabelGroupingNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), nullptr, CreateGroupingDisplayLabelField());
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*groupingContract)
            .From(
                ComplexQueryBuilder::Create()
                ->SelectContract(*groupingContract)
                .From(
                    ComplexQueryBuilder::Create()
                    ->SelectContract(*contract, "this")
                    .From(selectClass)
                    .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList())
                    )
                )
            .GroupByContract(*groupingContract)
            .OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetSelectInstanceClasses().clear();
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_GroupByLabel_ChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_GroupByLabel_ChildrenQuery)
    {
    ECClassCP classA = GetECClass("A");

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, true);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classA), classA->GetSchema().GetName(), classA->GetName()));

    auto parentNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateDisplayLabelGroupingNode(nullptr, "MyLabel", 1);
    parentNode->GetKey()->SetInstanceKeysSelectQuery(std::make_unique<PresentationQuery>(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            SetLabelGroupingNodeChildrenWhereClause(
                ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
                .From(selectClass))
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery)
    {
    ECClassCP classA = GetECClass("A");

    SearchResultInstanceNodesSpecification spec(1, false, false, false, true, true);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classA), classA->GetSchema().GetName(), classA->GetName()));

    auto parentNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECClassGroupingNode(nullptr, *classA, false, "MyLabel", {});
    parentNode->GetKey()->SetInstanceKeysSelectQuery(std::make_unique<PresentationQuery>(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        auto contract = DisplayLabelGroupingNodesQueryContract::Create(2, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));
        auto groupingContract = DisplayLabelGroupingNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), nullptr, CreateGroupingDisplayLabelField());
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*groupingContract);
        query->From(
            ComplexQueryBuilder::Create()
            ->SelectContract(*groupingContract)
            .From(
                ComplexQueryBuilder::Create()
                ->SelectContract(*contract, "this")
                .From(selectClass)
                .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList()))
            );
        query->GroupByContract(*groupingContract);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetSelectInstanceClasses().clear();
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery)
    {
    ECClassCP classA = GetECClass("A");

    SearchResultInstanceNodesSpecification spec(1, false, false, false, true, true);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classA), classA->GetSchema().GetName(), classA->GetName()));

    TestNodesFactory nodesFactory(GetConnection(), spec.GetHash(), "");

    auto classGroupingNode = nodesFactory.CreateECClassGroupingNode(nullptr, *classA, false, "Class Grouping Node", {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *classGroupingNode);

    auto labelGroupingNode = nodesFactory.CreateDisplayLabelGroupingNode(classGroupingNode->GetKey().get(), "Label Grouping Node", 1);
    labelGroupingNode->GetKey()->SetInstanceKeysSelectQuery(std::make_unique<PresentationQuery>(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *labelGroupingNode, classGroupingNode->GetNodeId());

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *labelGroupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            SetLabelGroupingNodeChildrenWhereClause(
                ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
                .From(selectClass))
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_UsesParentPropertyValueQuery, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="TestQuery" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_UsesParentPropertyValueQuery)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    auto b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB, [&](IECInstanceR instance)
        {
        instance.SetValue("TestQuery", ECValue(SEARCH_NODE_QUERY(*classA).c_str()));
        });
    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *b);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    spec.AddQuerySpecification(*new ECPropertyValueQuerySpecification(classA->GetSchema().GetName(), classA->GetName(), "TestQuery"));

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        RefCountedPtr<ECInstanceNodesQueryContract> contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_InstanceLabelOverride_AppliedByPriority, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop1" typeName="string" />
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_InstanceLabelOverride_AppliedByPriority)
    {
    ECClassCP classA = GetECClass("A");

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, true);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classA), classA->GetSchema().GetName(), classA->GetName()));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), "Prop1"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, false, classA->GetFullName(), "Prop2"));

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        auto contract = DisplayLabelGroupingNodesQueryContract::Create(2, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass, {},
            {
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Prop2")),
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Prop1")),
            }));
        auto groupingContract = DisplayLabelGroupingNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), nullptr, CreateGroupingDisplayLabelField());
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*groupingContract);
        query->From(
            ComplexQueryBuilder::Create()
            ->SelectContract(*groupingContract)
            .From(
                ComplexQueryBuilder::Create()
                ->SelectContract(*contract, "this")
                .From(selectClass)
                .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList()))
            );
        query->GroupByContract(*groupingContract);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetSelectInstanceClasses().clear();
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, true);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classA), classA->GetSchema().GetName(), classA->GetName()));
    spec.AddQuerySpecification(*new StringQuerySpecification(Utf8PrintfString("SELECT * FROM %s WHERE [ECInstanceId] > 0", classB->GetFullName()), classB->GetSchema().GetName(), classB->GetName()));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "PropA"));

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", true);
        auto contractA = DisplayLabelGroupingNodesQueryContract::Create(2, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClassA, {}, { &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("PropA")) }));
        ComplexQueryBuilderPtr queryA = ComplexQueryBuilder::Create();
        queryA->SelectContract(*contractA, "this")
            .From(selectClassA)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList());

        SelectClass<ECClass> selectClassB(*classB, "this", true);
        auto contractB = DisplayLabelGroupingNodesQueryContract::Create(3, "", *CreateInstanceKeysSelectQuery(), classB, CreateDisplayLabelField(selectClassB));
        ComplexQueryBuilderPtr queryB = ComplexQueryBuilder::Create();
        queryB->SelectContract(*contractB, "this")
            .From(selectClassB)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (SELECT *, ECInstanceId AS [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM %s WHERE [ECInstanceId] > 0))", classB->GetFullName()).c_str(), BoundQueryValuesList());

        auto groupingContract = DisplayLabelGroupingNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), nullptr, CreateGroupingDisplayLabelField());
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*groupingContract);
        query->From(
            ComplexQueryBuilder::Create()
            ->SelectContract(*groupingContract)
            .From(*UnionQueryBuilder::Create({queryA, queryB})));
        query->GroupByContract(*groupingContract);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetSelectInstanceClasses().clear();
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_HierarchyLevelInstanceFilter_OnDirectClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(NavigationQueryBuilderTests, SearchResultInstanceNodes_HierarchyLevelInstanceFilter_OnDirectClass)
    {
    ECClassCP classA = GetECClass("A");

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classA), classA->GetSchema().GetName(), classA->GetName()));

    // first verify without instance filter
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        auto contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));
        auto query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });

    // then with instance filter
    InstanceFilterDefinition instanceFilter("this.Prop = 123", *classA, {});
    GetBuilder().GetParameters().SetInstanceFilter(&instanceFilter);
    queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList())
            .Where("[this].[Prop] = 123", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_HierarchyLevelInstanceFilter_OnOneOfDirectClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(NavigationQueryBuilderTests, SearchResultInstanceNodes_HierarchyLevelInstanceFilter_OnOneOfDirectClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classA), classA->GetSchema().GetName(), classA->GetName()));
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classB), classB->GetSchema().GetName(), classB->GetName()));

    // first verify without instance filter
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", true);
        ComplexQueryBuilderPtr nestedQueryA = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClassA)), "this")
            .From(selectClassA)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList()));

        SelectClass<ECClass> selectClassB(*classB, "this", true);
        ComplexQueryBuilderPtr nestedQueryB = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create(2, "", *CreateInstanceKeysSelectQuery(), classB, CreateDisplayLabelField(selectClassB)), "this")
            .From(selectClassB)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classB).c_str()).c_str(), BoundQueryValuesList()));

        UnionQueryBuilderPtr query = UnionQueryBuilder::Create({ nestedQueryA, nestedQueryB });
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });

    // then with instance filter for B instances
    InstanceFilterDefinition instanceFilter("this.PropB = 123", *classB, {});
    GetBuilder().GetParameters().SetInstanceFilter(&instanceFilter);
    queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classB, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classB).c_str()).c_str(), BoundQueryValuesList())
            .Where("[this].[PropB] = 123", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_HierarchyLevelInstanceFilter_OnDerivedClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(NavigationQueryBuilderTests, SearchResultInstanceNodes_HierarchyLevelInstanceFilter_OnDerivedClass)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classA), classA->GetSchema().GetName(), classA->GetName()));

    // first verify without instance filter
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        auto contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));
        auto query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });

    // then with instance filter for B instances
    InstanceFilterDefinition instanceFilter("this.PropB = 123", *classB, {});
    GetBuilder().GetParameters().SetInstanceFilter(&instanceFilter);
    queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classB, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList())
            .Where("[this].[PropB] = 123", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SearchResultInstanceNodes_HierarchyLevelInstanceFilter_OnRelatedClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(NavigationQueryBuilderTests, SearchResultInstanceNodes_HierarchyLevelInstanceFilter_OnRelatedClass)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY(*classA), classA->GetSchema().GetName(), classA->GetName()));

    // first verify without instance filter
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        auto contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));
        auto query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });

    // then with instance filter using related B instance
    InstanceFilterDefinition instanceFilter("b.PropB = 123", *classA,
        {
        RelatedClassPath{ RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "rel"), true, SelectClass<ECClass>(*classB, "b", true)) }
        });
    GetBuilder().GetParameters().SetInstanceFilter(&instanceFilter);
    queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        auto contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));
        auto query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(instanceFilter.GetRelatedInstances()[0])
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", SEARCH_NODE_QUERY_PROCESSED(*classA).c_str()).c_str(), BoundQueryValuesList())
            .Where("[b].[PropB] = 123", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }
