/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "NavigationQueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_SupportedSchemas, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_SupportedSchemas)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedClasses;
        expectedClasses[classA->GetEntityClassCP()] = true;
        expectedClasses[classB->GetEntityClassCP()] = true;

        auto query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedClasses, "this");
        query->AsUnionQueryBuilder()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_SupportedSchemas_Excluded, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_SupportedSchemas_Excluded)
    {
    ECClassCP classA = GetECClass("A");

    Utf8String excludeSchemasStr("E:");
    bvector<ECSchemaCP> schemas = GetDb().Schemas().GetSchemas(false);
    for (size_t i = 0; i < schemas.size(); ++i)
        {
        if (schemas[i]->GetName().Equals(GetECSchema()->GetName()))
            continue;
        if (i > 0)
            excludeSchemasStr.append(",");
        excludeSchemasStr.append(schemas[i]->GetName());
        }

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, excludeSchemasStr);
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet classes;
        classes[classA->GetEntityClassCP()] = true;

        auto query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), classes, "this");
        query->AsComplexQueryBuilder()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_SupportedSchemas_UsedFromRulesetIfNotSpecified, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_SupportedSchemas_UsedFromRulesetIfNotSpecified)
    {
    ECClassCP classA = GetECClass("A");

    m_ruleset->SetSupportedSchemas(GetECSchema()->GetName());

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "");
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        auto query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this");
        query->AsComplexQueryBuilder()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_AllowsStandardSchemasIfExplicitylySpecified)
    {
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "ECDbMeta");
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_TRUE(queries.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_DoNotSort, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_DoNotSort)
    {
    ECClassCP classA = GetECClass("A");

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());
    spec.SetDoNotSort(true);

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        auto query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this");
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_HideNodesInHierarchy, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_HideNodesInHierarchy)
    {
    ECClassCP classA = GetECClass("A");

    AllInstanceNodesSpecification spec(1, false, true, false, false, false, GetECSchema()->GetName());

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        auto query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this");
        query->AsComplexQueryBuilder()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideNodesInHierarchy(true);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_HideIfNoChildren, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_HideIfNoChildren)
    {
    ECClassCP classA = GetECClass("A");

    AllInstanceNodesSpecification spec(1, false, false, true, false, false, GetECSchema()->GetName());

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        auto query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this");
        query->AsComplexQueryBuilder()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideIfNoChildren(true);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_HideExpression, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(NavigationQueryBuilderTests, AllInstanceNodes_HideExpression)
    {
    ECClassCP classA = GetECClass("A");

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());
    spec.SetHideExpression("test");

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        auto query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this");
        query->AsComplexQueryBuilder()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideExpression("test");
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_GroupByClass, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_GroupByClass)
    {
    ECClassCP classA = GetECClass("A");

    AllInstanceNodesSpecification spec(1, false, false, false, true, false, GetECSchema()->GetName());
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery());
        ComplexQueryBuilderPtr nested = ComplexQueryBuilder::Create();
        nested->SelectAll();
        nested->From(*RulesEngineTestHelpers::CreateQuery(*contract, { classA }, true, "this"));
        nested->GroupByContract(*contract);

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectAll();
        query->From(*nested);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetSelectInstanceClasses().clear();
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_GroupByClass_ChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_GroupByClass_ChildrenQuery)
    {
    ECClassCP classA = GetECClass("A");

    AllInstanceNodesSpecification spec(1, false, false, false, true, false, GetECSchema()->GetName());

    auto parentNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECClassGroupingNode(nullptr, *classA, false, "MyLabel", {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, m_connection->GetId(), m_ruleset->GetRuleSetId(), *parentNode);

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(selectClass.GetClass(),
            ComplexQueryBuilder::Create()->SelectContract(*contract, selectClass.GetAlias().c_str()).From(selectClass));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_GroupByLabel, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_GroupByLabel)
    {
    ECClassCP classA = GetECClass("A");

    AllInstanceNodesSpecification spec(1, false, false, false, false, true, GetECSchema()->GetName());

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        auto groupingContract = DisplayLabelGroupingNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), nullptr, CreateGroupingDisplayLabelField());
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*groupingContract);
        query->From(
            ComplexQueryBuilder::Create()
            ->SelectContract(*groupingContract)
            .From(
                ComplexQueryBuilder::Create()
                ->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(2, "", *CreateInstanceKeysSelectQuery(), &selectClass.GetClass(), CreateDisplayLabelField(selectClass)), selectClass.GetAlias().c_str())
                .From(selectClass)));
        query->GroupByContract(*groupingContract);
        query->OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetSelectInstanceClasses().clear();
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_GroupByLabel_ChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_GroupByLabel_ChildrenQuery)
    {
    ECClassCP classA = GetECClass("A");

    AllInstanceNodesSpecification spec(1, false, false, false, false, true, GetECSchema()->GetName());

    NavNodePtr parentNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateDisplayLabelGroupingNode(nullptr, "MyLabel", 1);
    parentNode->GetKey()->SetInstanceKeysSelectQuery(std::make_unique<PresentationQuery>(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, m_connection->GetId(), m_ruleset->GetRuleSetId(), *parentNode);

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        auto query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this", [&](ComplexQueryBuilder& query)
            {
            SetLabelGroupingNodeChildrenWhereClause(query);
            });
        query->AsComplexQueryBuilder()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_GroupByLabel_ChildrenQuery_WithKnownGroupedInstances, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(NavigationQueryBuilderTests, AllInstanceNodes_GroupByLabel_ChildrenQuery_WithKnownGroupedInstances)
    {
    ECClassCP classA = GetECClass("A");

    AllInstanceNodesSpecification spec(1, false, false, false, false, true, GetECSchema()->GetName());

    NavNodePtr parentNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateDisplayLabelGroupingNode(nullptr, "MyLabel", 1, { ECInstanceKey(ECClassId((uint64_t)123), ECInstanceId((uint64_t)456)) });
    RulesEngineTestHelpers::CacheNode(m_nodesCache, m_connection->GetId(), m_ruleset->GetRuleSetId(), *parentNode);

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        auto query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this", [](ComplexQueryBuilder& query)
            {
            query.Where(ValuesFilteringHelper(bvector<ECInstanceId>{ ECInstanceId((uint64_t)456) }).Create("[this].[ECInstanceId]"));
            });
        query->AsComplexQueryBuilder()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery)
    {
    ECClassCP classA = GetECClass("A");

    AllInstanceNodesSpecification spec(1, false, false, false, true, true, GetECSchema()->GetName());

    auto parentNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECClassGroupingNode(nullptr, *classA, false, "MyLabel", 1);

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr groupingContract = DisplayLabelGroupingNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), nullptr, CreateGroupingDisplayLabelField());
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*groupingContract);
        query->From(
            ComplexQueryBuilder::Create()
            ->SelectContract(*groupingContract)
            .From(
                ComplexQueryBuilder::Create()
                ->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create(2, "", *CreateInstanceKeysSelectQuery(), &selectClass.GetClass(), CreateDisplayLabelField(selectClass)), selectClass.GetAlias().c_str())
                .From(selectClass)));
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
DEFINE_SCHEMA(AllInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery)
    {
    ECClassCP classA = GetECClass("A");

    AllInstanceNodesSpecification spec(1, false, false, false, true, true, GetECSchema()->GetName());
    TestNodesFactory factory(GetConnection(), spec.GetHash(), "");

    auto classGroupingNode = factory.CreateECClassGroupingNode(nullptr, *classA, false, "Class Grouping Node", {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, m_connection->GetId(), m_ruleset->GetRuleSetId(), *classGroupingNode);

    auto labelGroupingNode = factory.CreateDisplayLabelGroupingNode(classGroupingNode->GetKey().get(), "Label Grouping Node", 1);
    labelGroupingNode->GetKey()->SetInstanceKeysSelectQuery(std::make_unique<PresentationQuery>(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, m_connection->GetId(), m_ruleset->GetRuleSetId(), *labelGroupingNode, classGroupingNode->GetNodeId());

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *labelGroupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), classA, CreateDisplayLabelField(selectClass));

        ComplexQueryBuilderPtr nested = ComplexQueryBuilder::Create();
        nested->SelectContract(*contract, "this");
        nested->From(*classA, false, "this");
        SetLabelGroupingNodeChildrenWhereClause(*nested);

        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *nested);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_InstanceLabelOverride_AppliedByPriority, R"*(
    <ECEntityClass typeName="Class1">
        <ECProperty propertyName="Code" typeName="string" />
        <ECProperty propertyName="Description" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_InstanceLabelOverride_AppliedByPriority)
    {
    ECClassCP class1 = GetECClass("Class1");

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, class1->GetSchema().GetName());
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, false, class1->GetFullName(), "Code"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, false, class1->GetFullName(), "Description"));

    IECInstancePtr instance = class1->GetDefaultStandaloneEnabler()->CreateInstance();
    auto instanceNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECInstanceNode(nullptr, *instance, "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, m_connection->GetId(), m_ruleset->GetRuleSetId(), *instanceNode);

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *instanceNode);
    ASSERT_EQ(1, queries.size());

    auto query = queries[0];
    ValidateQuery(spec, query, [&]()
        {
        SelectClass<ECClass> selectClass(*class1, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), class1, CreateDisplayLabelField(selectClass, {},
            {
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Description")),
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Code")),
            }));
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*contract, "this");
        query->From(selectClass);

        ComplexQueryBuilderPtr sorted = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*class1, *query);
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels, R"*(
    <ECEntityClass typeName="Class1">
        <ECProperty propertyName="Code" typeName="string" />
        <ECProperty propertyName="Description" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Class2">
        <ECProperty propertyName="Code" typeName="string" />
        <ECProperty propertyName="Description" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels)
    {
    ECClassCP class1 = GetECClass("Class1");

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, class1->GetSchema().GetName());
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, false, class1->GetFullName(), "Code"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, false, class1->GetFullName(), "Description"));

    IECInstancePtr instance = class1->GetDefaultStandaloneEnabler()->CreateInstance();
    auto instanceNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECInstanceNode(nullptr, *instance, "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, m_connection->GetId(), m_ruleset->GetRuleSetId(), *instanceNode);

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *instanceNode);
    ASSERT_EQ(1, queries.size());

    auto query = queries[0];
    ValidateQuery(spec, query, [&]()
        {
        ECClassCP class2 = GetECClass("Class2");
        SelectClass<ECClass> selectClass1(*class1, "this", true);
        NavigationQueryContractPtr class1Contract = ECInstanceNodesQueryContract::Create(1, "", *CreateInstanceKeysSelectQuery(), class1, CreateDisplayLabelField(selectClass1, {},
            {
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Description")),
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Code")),
            }));
        ComplexQueryBuilderPtr class1Query = ComplexQueryBuilder::Create();
        class1Query->SelectContract(*class1Contract, "this");
        class1Query->From(selectClass1);

        SelectClass<ECClass> selectClass2(*class2, "this", true);
        NavigationQueryContractPtr class2Contract = ECInstanceNodesQueryContract::Create(2, "", *CreateInstanceKeysSelectQuery(), class2, CreateDisplayLabelField(selectClass2));
        ComplexQueryBuilderPtr class2Query = ComplexQueryBuilder::Create();
        class2Query->SelectContract(*class2Contract, "this");
        class2Query->From(selectClass2);

        UnionQueryBuilderPtr sorted = UnionQueryBuilder::Create({
            RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*class1, *class1Query),
            RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*class2, *class2Query)
            });
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return sorted;
        });
    }
