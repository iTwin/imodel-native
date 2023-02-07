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

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", false);
        ComplexQueryBuilderPtr nestedQuery1 = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA)), "this")
            .From(selectClassA));

        SelectClass<ECClass> selectClassB(*classB, "this", false);
        ComplexQueryBuilderPtr nestedQuery2 = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB)), "this")
            .From(selectClassB));

        UnionQueryBuilderPtr query = UnionQueryBuilder::Create({ nestedQuery1, nestedQuery2 });
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

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this").From(selectClass));
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

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClassWithExcludes<ECClass> selectClassA(*classA, "this", true);
        selectClassA.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classB, "", false));
        selectClassA.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classC, "", true));
        ComplexQueryBuilderPtr nestedQuery = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA)), "this")
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
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", false);
        ComplexQueryBuilderPtr nestedQueryA = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA)), "this")
            .From(selectClassA));

        SelectClass<ECClass> selectClassB(*classB, "this", false);
        ComplexQueryBuilderPtr nestedQueryB = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB)), "this")
            .From(selectClassB));

        UnionQueryBuilderPtr query = UnionQueryBuilder::Create({ nestedQueryA, nestedQueryB });
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
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this").From(selectClass));
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
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
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

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create("", nullptr);

        ComplexQueryBuilderPtr grouped = ComplexQueryBuilder::Create();
        grouped->SelectAll();
        grouped->From(ComplexQueryBuilder::Create()->SelectContract(*contract, "this").From(*classA, false, "this"));
        grouped->GroupByContract(*contract);

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectAll();
        query->From(*grouped);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetSelectInstanceClasses().clear();
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

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this").From(*classA, false, "this"));
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
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass1(*classA, "this", false);
        ComplexQueryBuilderPtr nestedQuery1 = ComplexQueryBuilder::Create();
        nestedQuery1->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create("", nullptr, classA, CreateDisplayLabelField(selectClass1)), "this");
        nestedQuery1->From(selectClass1);

        SelectClass<ECClass> selectClass2(*classB, "this", false);
        ComplexQueryBuilderPtr nestedQuery2 = ComplexQueryBuilder::Create();
        nestedQuery2->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create("", nullptr, classB, CreateDisplayLabelField(selectClass2)), "this");
        nestedQuery2->From(selectClass2);

        auto contract = DisplayLabelGroupingNodesQueryContract::Create("", nullptr, nullptr, CreateGroupingDisplayLabelField());
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*contract, "this");
        query->From(*UnionQueryBuilder::Create({ nestedQuery1, nestedQuery2 }));
        query->GroupByContract(*contract);
        query->OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetSelectInstanceClasses().clear();
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
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
    parentNode->GetKey()->SetInstanceKeysSelectQuery(std::make_unique<PresentationQuery>(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass1(*classA, "this", false);
        ComplexQueryBuilderPtr nestedQuery1 = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            SetLabelGroupingNodeChildrenWhereClause(
                ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass1)), "this").From(selectClass1)
            ));

        SelectClass<ECClass> selectClass2(*classB, "this", false);
        ComplexQueryBuilderPtr nestedQuery2 = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            SetLabelGroupingNodeChildrenWhereClause(
                ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass2)), "this").From(selectClass2)
            ));

        UnionQueryBuilderPtr query = UnionQueryBuilder::Create({ nestedQuery1, nestedQuery2 });
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
    parentNode->GetKey()->SetInstanceKeysSelectQuery(std::make_unique<PresentationQuery>(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = DisplayLabelGroupingNodesQueryContract::Create("", nullptr, classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*contract, "this")
            .From(ComplexQueryBuilder::Create()->SelectContract(*contract, "this").From(selectClass))
            .GroupByContract(*contract)
            .OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetSelectInstanceClasses().clear();
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
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
    labelGroupingNode->GetKey()->SetInstanceKeysSelectQuery(std::make_unique<PresentationQuery>(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *labelGroupingNode, classGroupingNode->GetNodeId());

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *labelGroupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));

        ComplexQueryBuilderPtr nestedQuery = ComplexQueryBuilder::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(selectClass);
        SetLabelGroupingNodeChildrenWhereClause(*nestedQuery);

        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *nestedQuery);
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
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass)), "this")
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
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
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
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this").From(selectClass));
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

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
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
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode, grandparentNode->GetNodeId());

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "parent.parent.Name = 999", classA->GetFullName(), false);
    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
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
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode, grandparentNode->GetNodeId());

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false,
        "parent.parent.NameC = parent.NameB", classA->GetFullName(), false);
    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
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

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", false);
        NavigationQueryContractPtr contractA = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA));
        ComplexQueryBuilderPtr queryA = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contractA, "this").From(selectClassA));

        SelectClass<ECClass> selectClassB(*classB, "this", false);
        NavigationQueryContractPtr contractB = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB, {}, { &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("PropB")) }));
        ComplexQueryBuilderPtr queryB = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*contractB, "this").From(selectClassB));

        UnionQueryBuilderPtr sorted = UnionQueryBuilder::Create({ queryA, queryB });
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

    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
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
        ComplexQueryBuilderPtr sorted = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this").From(selectClass));
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_HierarchyLevelInstanceFilter_OnDirectClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(NavigationQueryBuilderTests, InstancesOfSpecificClasses_HierarchyLevelInstanceFilter_OnDirectClass)
    {
    ECClassCP classA = GetECClass("A");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false,
        "", classA->GetFullName(), true);

    // first verify without instance filter
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        auto contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        auto query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass));
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
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where("[this].[Prop] = 123", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_HierarchyLevelInstanceFilter_OnOneOfDirectClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(NavigationQueryBuilderTests, InstancesOfSpecificClasses_HierarchyLevelInstanceFilter_OnOneOfDirectClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false,
        "", RulesEngineTestHelpers::CreateClassNamesList({ classA, classB }), false);

    // first verify without instance filter
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", false);
        ComplexQueryBuilderPtr nestedQueryA = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA)), "this")
            .From(selectClassA));

        SelectClass<ECClass> selectClassB(*classB, "this", false);
        ComplexQueryBuilderPtr nestedQueryB = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB)), "this")
            .From(selectClassB));

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
        SelectClass<ECClass> selectClass(*classB, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where("[this].[PropB] = 123", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_HierarchyLevelInstanceFilter_OnDerivedClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(NavigationQueryBuilderTests, InstancesOfSpecificClasses_HierarchyLevelInstanceFilter_OnDerivedClass)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false,
        "", classA->GetFullName(), true);

    // first verify without instance filter
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        auto contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        auto query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass));
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
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where("[this].[PropB] = 123", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InstancesOfSpecificClasses_HierarchyLevelInstanceFilter_OnRelatedClass, R"*(
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
TEST_F(NavigationQueryBuilderTests, InstancesOfSpecificClasses_HierarchyLevelInstanceFilter_OnRelatedClass)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false,
        "", classA->GetFullName(), true);

    // first verify without instance filter
    auto queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        auto contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        auto query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass));
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
        auto contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        auto query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(instanceFilter.GetRelatedInstances()[0])
            .Where("[b].[PropB] = 123", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }
