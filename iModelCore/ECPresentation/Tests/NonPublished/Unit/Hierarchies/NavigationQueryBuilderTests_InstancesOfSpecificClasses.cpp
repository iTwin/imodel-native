/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "NavigationQueryBuilderTests.h"

// note: some of the common specification properties of this specification are tested
//       QueryBuilderTests_AllInstanceNodes.cpp as they're common between the two.

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_Classes_NotPolymorphic, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(NavigationQueryBuilderTests, InstancesOfSpecificClasses_Classes_NotPolymorphic)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    InstanceNodesOfSpecificClassesSpecification spec(1, ChildrenHint::Unknown, false, false, false, false, "",
        bvector<MultiSchemaClass*>{new MultiSchemaClass(GetECSchema()->GetName(), false, bvector<Utf8String>{ classA->GetName(), classB->GetName() })},
        bvector<MultiSchemaClass*>());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", false);
        ComplexNavigationQueryPtr nestedQuery1 = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA)), "this")
            .From(selectClassA));

        SelectClass<ECClass> selectClassB(*classB, "this", false);
        ComplexNavigationQueryPtr nestedQuery2 = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB)), "this")
            .From(selectClassB));

        UnionNavigationQueryPtr query = UnionNavigationQuery::Create({ nestedQuery1, nestedQuery2 });
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_Classes_Polymorphic, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(NavigationQueryBuilderTests, InstancesOfSpecificClasses_Classes_Polymorphic)
    {
    ECClassCP classA = GetECClass("A");

    InstanceNodesOfSpecificClassesSpecification spec(1, ChildrenHint::Unknown, false, false, false, false, "",
        bvector<MultiSchemaClass*>{new MultiSchemaClass(GetECSchema()->GetName(), true, bvector<Utf8String>{ classA->GetName() })},
        bvector<MultiSchemaClass*>());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(selectClass));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_ExcludedClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
)*");
TEST_F(NavigationQueryBuilderTests, InstancesOfSpecificClasses_ExcludedClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");

    InstanceNodesOfSpecificClassesSpecification spec(1, ChildrenHint::Unknown, false, false, false, false, "",
        bvector<MultiSchemaClass*>{
            new MultiSchemaClass(GetECSchema()->GetName(), true, bvector<Utf8String>{ classA->GetName() })
        },
        bvector<MultiSchemaClass*>{
            new MultiSchemaClass(GetECSchema()->GetName(), false, bvector<Utf8String>{ classB->GetName() }),
            new MultiSchemaClass(GetECSchema()->GetName(), true, bvector<Utf8String>{ classC->GetName() })
        });

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClassWithExcludes<ECClass> selectClassA(*classA, "this", true);
        selectClassA.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classB, "", false));
        selectClassA.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classC, "", true));
        ComplexNavigationQueryPtr nestedQuery = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA)), "this")
            .From(selectClassA));
        nestedQuery->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return nestedQuery;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_ClassNames_NotPolymorphic, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_ClassNames_NotPolymorphic)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", RulesEngineTestHelpers::CreateClassNamesList({ classA, classB }), false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", false);
        ComplexNavigationQueryPtr nestedQueryA = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA)), "this")
            .From(selectClassA));

        SelectClass<ECClass> selectClassB(*classB, "this", false);
        ComplexNavigationQueryPtr nestedQueryB = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB)), "this")
            .From(selectClassB));

        UnionNavigationQueryPtr query = UnionNavigationQuery::Create({ nestedQueryA, nestedQueryB });
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_ClassNames_Polymorphic, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_ClassNames_Polymorphic)
    {
    ECClassCP classA = GetECClass("A");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), true);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(selectClass));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_AllowsStandardSchemasIfExplicitylySpecified)
    {
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "ECDbMeta:ECSchemaDef", false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_TRUE(queries.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_GroupByClass, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_GroupByClass)
    {
    ECClassCP classA = GetECClass("A");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, true, false, false, "", classA->GetFullName(), false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create("", nullptr);

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(*classA, false, "this"));
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectAll();
        query->From(*grouped);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetSelectInstanceClasses().clear();
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_GroupByClass_ChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_GroupByClass_ChildrenQuery)
    {
    ECClassCP classA = GetECClass("A");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, true, false, false, "", classA->GetFullName(), false);

    auto parentNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECClassGroupingNode(nullptr, *classA, false, "MyLabel", {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(*classA, false, "this"));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_GroupByLabel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_GroupByLabel)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, true, false, "", RulesEngineTestHelpers::CreateClassNamesList({ classA, classB }), false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass1(*classA, "this", false);
        ComplexNavigationQueryPtr nestedQuery1 = ComplexNavigationQuery::Create();
        nestedQuery1->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create("", nullptr, classA, CreateDisplayLabelField(selectClass1)), "this");
        nestedQuery1->From(selectClass1);

        SelectClass<ECClass> selectClass2(*classB, "this", false);
        ComplexNavigationQueryPtr nestedQuery2 = ComplexNavigationQuery::Create();
        nestedQuery2->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create("", nullptr, classB, CreateDisplayLabelField(selectClass2)), "this");
        nestedQuery2->From(selectClass2);

        auto contract = DisplayLabelGroupingNodesQueryContract::Create("", nullptr, nullptr, CreateGroupingDisplayLabelField());
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, "this");
        query->From(*UnionNavigationQuery::Create({ nestedQuery1, nestedQuery2 }));
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
DEFINE_SCHEMA(InstancesOfSpecificClasses_GroupByLabel_ChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_GroupByLabel_ChildrenQuery)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, true, false, "", RulesEngineTestHelpers::CreateClassNamesList({ classA, classB }), false);

    NavNodePtr parentNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateDisplayLabelGroupingNode(nullptr, "MyLabel", 1);
    parentNode->SetInstanceKeysSelectQuery(StringGenericQuery::Create(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass1(*classA, "this", false);
        ComplexNavigationQueryPtr nestedQuery1 = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            SetLabelGroupingNodeChildrenWhereClause(
                ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass1)), "this").From(selectClass1)
            ));

        SelectClass<ECClass> selectClass2(*classB, "this", false);
        ComplexNavigationQueryPtr nestedQuery2 = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            SetLabelGroupingNodeChildrenWhereClause(
                ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass2)), "this").From(selectClass2)
            ));

        UnionNavigationQueryPtr query = UnionNavigationQuery::Create({ nestedQuery1, nestedQuery2 });
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_GroupByClassAndLabel_ClassNodeChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_GroupByClassAndLabel_ClassNodeChildrenQuery)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, true, true, false, "", RulesEngineTestHelpers::CreateClassNamesList({ classA, classB }), false);

    auto parentNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECClassGroupingNode(nullptr, *classA, false, "MyLabel", {});
    parentNode->SetInstanceKeysSelectQuery(StringGenericQuery::Create(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = DisplayLabelGroupingNodesQueryContract::Create("", nullptr, classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, "this")
            .From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(selectClass))
            .GroupByContract(*contract)
            .OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetSelectInstanceClasses().clear();
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_GroupByClassAndLabel_LabelNodeChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_GroupByClassAndLabel_LabelNodeChildrenQuery)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, true, true, false, "", RulesEngineTestHelpers::CreateClassNamesList({ classA, classB }), false);

    auto classGroupingNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECClassGroupingNode(nullptr, *classA, false, "Class Grouping Node", {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *classGroupingNode);

    auto labelGroupingNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateDisplayLabelGroupingNode(classGroupingNode->GetKey().get(), "Label Grouping Node", 1);
    labelGroupingNode->SetParentNode(*classGroupingNode);
    labelGroupingNode->SetInstanceKeysSelectQuery(StringGenericQuery::Create(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *labelGroupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *labelGroupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));

        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(selectClass);
        SetLabelGroupingNodeChildrenWhereClause(*nestedQuery);

        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *nestedQuery);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_InstanceFilter, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Name" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceFilter)
    {
    ECClassCP classA = GetECClass("A");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "this.Name = 2", classA->GetFullName(), false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass)), "this")
            .From(selectClass)
            .Where("[this].[Name] = 2", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_InstanceFilter_LikeOperator, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Name" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceFilter_LikeOperator)
    {
    ECClassCP classA = GetECClass("A");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "this.Name ~ \"Test\"", classA->GetFullName(), false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where("CAST([this].[Name] AS TEXT) LIKE 'Test' ESCAPE \'\\\'", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_InstanceFilter_ReferencingParentInstanceWithoutParentNodeDoesntApplyTheFilter, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceFilter_ReferencingParentInstanceWithoutParentNodeDoesntApplyTheFilter)
    {
    ECClassCP classA = GetECClass("A");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "parent.Name = 999", classA->GetFullName(), false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(selectClass));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_InstanceFilter_ReferencingParentInstance_WithParentInstanceNode, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Name" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceFilter_ReferencingParentInstance_WithParentInstanceNode)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "parent.Name = 999", classA->GetFullName(), false);

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classB);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .From(*classB, false, "parent")
            .Where("[parent].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) })
            .Where("[parent].[Name] = 999", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_InstanceFilter_ReferencingGrandParentInstance, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Name" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceFilter_ReferencingGrandParentInstance)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");

    auto grandparentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classC, ECInstanceId((uint64_t)123));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *grandparentNode);

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classB, ECInstanceId((uint64_t)456));
    parentNode->SetParentNodeId(grandparentNode->GetNodeId());
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "parent.parent.Name = 999", classA->GetFullName(), false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .From(*classC, false, "parent_parent")
            .Where("[parent_parent].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) })
            .Where("[parent_parent].[Name] = 999", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_InstanceFilter_ReferencingGrandParentAndParentInstance, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="NameB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="NameC" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceFilter_ReferencingGrandParentAndParentInstance)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");

    auto grandparentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classC, ECInstanceId((uint64_t)123));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *grandparentNode);

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classB, ECInstanceId((uint64_t)456));
    parentNode->SetParentNodeId(grandparentNode->GetNodeId());
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false,
        "parent.parent.NameC = parent.NameB", classA->GetFullName(), false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .From(*classB, false, "parent")
            .From(*classC, false, "parent_parent")
            .Where("[parent].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)456)) })
            .Where("[parent_parent].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) })
            .Where("[parent_parent].[NameC] = [parent].[NameB]", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", RulesEngineTestHelpers::CreateClassNamesList({ classA, classB }), false);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "PropB"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", false);
        NavigationQueryContractPtr contractA = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA));
        ComplexNavigationQueryPtr queryA = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*contractA, "this").From(selectClassA));

        SelectClass<ECClass> selectClassB(*classB, "this", false);
        NavigationQueryContractPtr contractB = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB, {}, { &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("PropB")) }));
        ComplexNavigationQueryPtr queryB = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexNavigationQuery::Create()->SelectContract(*contractB, "this").From(selectClassB));

        UnionNavigationQueryPtr sorted = UnionNavigationQuery::Create({ queryA, queryB });
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_InstanceLabelOverride_AppliedByPriority, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop1" typeName="string" />
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceLabelOverride_AppliedByPriority)
    {
    ECClassCP classA = GetECClass("A");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), "Prop1"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, false, classA->GetFullName(), "Prop2"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        auto labelField = CreateDisplayLabelField(selectClass, {},
            {
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Prop2")),
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Prop1")),
            });
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, labelField);
        ComplexNavigationQueryPtr sorted = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(selectClass));
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return sorted;
        });
    }
