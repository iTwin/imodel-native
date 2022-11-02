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

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedClasses;
        expectedClasses[classA->GetEntityClassCP()] = true;
        expectedClasses[classB->GetEntityClassCP()] = true;

        NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedClasses, "this");
        query->AsUnionQuery()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
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
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet classes;
        classes[classA->GetEntityClassCP()] = true;

        NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), classes, "this");
        query->AsComplexQuery()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
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
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this");
        query->AsComplexQuery()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_AllowsStandardSchemasIfExplicitylySpecified)
    {
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "ECDbMeta");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
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

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this");
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

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this");
        query->AsComplexQuery()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideNodesInHierarchy(true);
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

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this");
        query->AsComplexQuery()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfNoChildren(true);
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

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this");
        query->AsComplexQuery()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideExpression("test");
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
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create("", nullptr);
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectAll();
        nested->From(*RulesEngineTestHelpers::CreateQuery(*contract, { classA }, true, "this"));
        nested->GroupByContract(*contract);

        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectAll();
        query->From(*nested);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetSelectInstanceClasses().clear();
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
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(selectClass.GetClass(),
            ComplexNavigationQuery::Create()->SelectContract(*contract, selectClass.GetAlias().c_str()).From(selectClass));
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

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        auto contract = DisplayLabelGroupingNodesQueryContract::Create("", nullptr, nullptr, CreateGroupingDisplayLabelField());
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract);
        query->From(
            ComplexNavigationQuery::Create()
            ->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create("", nullptr, &selectClass.GetClass(), CreateDisplayLabelField(selectClass)), "this")
            .From(selectClass));
        query->GroupByContract(*contract);
        query->OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        query->GetResultParametersR().GetSelectInstanceClasses().clear();
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
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
    parentNode->SetInstanceKeysSelectQuery(StringGenericQuery::Create(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this", [&](ComplexNavigationQuery& query)
            {
            SetLabelGroupingNodeChildrenWhereClause(query);
            });
        query->AsComplexQuery()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
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
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        ECClassSet expectedQueryClasses;
        expectedQueryClasses[classA->GetEntityClassCP()] = true;

        NavigationQueryPtr query = RulesEngineTestHelpers::CreateECInstanceNodesQueryForClasses(GetSchemaHelper(), expectedQueryClasses, "this", [](ComplexNavigationQuery& query)
            {
            query.Where(ValuesFilteringHelper(bvector<ECInstanceId>{ ECInstanceId((uint64_t)456) }).Create("[this].[ECInstanceId]"));
            });
        query->AsComplexQuery()->OrderBy(GetECInstanceNodesOrderByClause().c_str());
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

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = DisplayLabelGroupingNodesQueryContract::Create("", nullptr, &selectClass.GetClass(), CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, "this");
        query->From(ComplexNavigationQuery::Create()->SelectContract(*contract, selectClass.GetAlias().c_str()).From(selectClass));
        query->GroupByContract(*contract);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetSelectInstanceClasses().clear();
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
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
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *classGroupingNode);

    auto labelGroupingNode = factory.CreateDisplayLabelGroupingNode(classGroupingNode->GetKey().get(), "Label Grouping Node", 1);
    labelGroupingNode->SetParentNode(*classGroupingNode);
    labelGroupingNode->SetInstanceKeysSelectQuery(StringGenericQuery::Create(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *labelGroupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *labelGroupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(*classA, false, "this");
        SetLabelGroupingNodeChildrenWhereClause(*nested);

        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *nested);
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
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *instanceNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *instanceNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ValidateQuery(spec, query, [&]()
        {
        SelectClass<ECClass> selectClass(*class1, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", class1, CreateDisplayLabelField(selectClass, {},
            {
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Description")),
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Code")),
            }));
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, "this");
        query->From(selectClass);

        ComplexNavigationQueryPtr sorted = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*class1, *query);
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
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *instanceNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *instanceNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ValidateQuery(spec, query, [&]()
        {
        ECClassCP class2 = GetECClass("Class2");
        SelectClass<ECClass> selectClass1(*class1, "this", true);
        NavigationQueryContractPtr class1Contract = ECInstanceNodesQueryContract::Create("", class1, CreateDisplayLabelField(selectClass1, {},
            {
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Description")),
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Code")),
            }));
        ComplexNavigationQueryPtr class1Query = ComplexNavigationQuery::Create();
        class1Query->SelectContract(*class1Contract, "this");
        class1Query->From(selectClass1);

        SelectClass<ECClass> selectClass2(*class2, "this", true);
        NavigationQueryContractPtr class2Contract = ECInstanceNodesQueryContract::Create("", class2, CreateDisplayLabelField(selectClass2));
        ComplexNavigationQueryPtr class2Query = ComplexNavigationQuery::Create();
        class2Query->SelectContract(*class2Contract, "this");
        class2Query->From(selectClass2);

        UnionNavigationQueryPtr sorted = UnionNavigationQuery::Create({
            RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*class1, *class1Query),
            RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*class2, *class2Query)
            });
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return sorted;
        });
    }
