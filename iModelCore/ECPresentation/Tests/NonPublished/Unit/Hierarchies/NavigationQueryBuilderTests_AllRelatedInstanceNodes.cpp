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
DEFINE_SCHEMA(AllRelatedInstanceNodes_NullParentNode_ReturnsNoQuery, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_NullParentNode_ReturnsNoQuery)
    {
    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllRelatedInstanceNodes_NonInstanceParentNode_ReturnsNoQuery, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_NonInstanceParentNode_ReturnsNoQuery)
    {
    auto parentNode = TestNodesHelper::CreateCustomNode(GetConnection(), "TestType", "label", "");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryBuilderTests, AllRelatedInstanceNodes_AllowsStandardSchemasIfExplicitylySpecified)
    {
    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *GetECClass("ECDbMeta", "ECSchemaDef"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, "ECDbMeta");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllRelatedInstanceNodes_SupportedSchemas_DifferentThanParentInstanceNodeSchema, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_SupportedSchemas_DifferentThanParentInstanceNodeSchema)
    {
    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *GetECClass("ECDbMeta", "ECClassDef"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllRelatedInstanceNodes_NoGrouping_ForwardRelationDirection, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_NoGrouping_ForwardRelationDirection)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, GetECSchema()->GetName());
    spec.SetRequiredRelationDirection(RequiredRelationDirection_Forward);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllRelatedInstanceNodes_NoGrouping_BackwardRelationDirection, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_NoGrouping_BackwardRelationDirection)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classB);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, GetECSchema()->GetName());
    spec.SetRequiredRelationDirection(RequiredRelationDirection_Backward);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (%s)",
                ComplexGenericQuery::Create()->SelectContract(*CreateRelatedInstancesQueryContract(), "relatedInstances")
                .From(*classA, true, "relatedInstances")
                .Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, "related", true), false))
                .Where("[related].[ECInstanceId] IN (?)", BoundQueryValuesList())
                .ToString().c_str()
            ).c_str(), { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllRelatedInstanceNodes_NoGrouping_BothDirections, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_NoGrouping_BothDirections)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_C")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classB);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, GetECSchema()->GetName());
    spec.SetRequiredRelationDirection(RequiredRelationDirection_Both);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", true);
        ComplexNavigationQueryPtr nestedQueryA = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA)), "this")
            .From(selectClassA)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (%s)",
                ComplexGenericQuery::Create()->SelectContract(*CreateRelatedInstancesQueryContract(), "relatedInstances")
                .From(*classA, true, "relatedInstances")
                .Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, "related", true), false))
                .Where("[related].[ECInstanceId] IN (?)", BoundQueryValuesList())
                .ToString().c_str()
            ).c_str(), { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));

        SelectClass<ECClass> selectClassC(*classC, "this", true);
        ComplexNavigationQueryPtr nestedQueryC = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClassC)), "this")
            .From(selectClassC)
            .Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), false, SelectClass<ECClass>(*classB, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));

        UnionNavigationQueryPtr query = UnionNavigationQuery::Create({ nestedQueryA, nestedQueryC });
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relBC);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllRelatedInstanceNodes_GroupByClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByClass)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_C")->GetRelationshipClassCP();

    auto rootInstanceNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classB);

    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);
    AllRelatedInstanceNodesSpecification spec(1, false, false, false, true, false, false, 0, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *rootInstanceNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create("", nullptr);

        SelectClass<ECClass> selectClassA(*classA, "this", true);
        ComplexNavigationQueryPtr nestedQueryA = &ComplexNavigationQuery::Create()
            ->SelectContract(*contract, "this")
            .From(selectClassA)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (%s)",
                ComplexGenericQuery::Create()->SelectContract(*CreateRelatedInstancesQueryContract(), "relatedInstances")
                .From(*classA, true, "relatedInstances")
                .Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, "related", true), false))
                .Where("[related].[ECInstanceId] IN (?)", BoundQueryValuesList())
                .ToString().c_str()
            ).c_str(), { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) });

        SelectClass<ECClass> selectClassC(*classC, "this", true);
        ComplexNavigationQueryPtr nestedQueryC = &ComplexNavigationQuery::Create()
            ->SelectContract(*contract, "this")
            .From(selectClassC)
            .Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), false, SelectClass<ECClass>(*classB, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) });

        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectAll();
        query->From(ComplexNavigationQuery::Create()
            ->SelectAll()
            .From(*UnionNavigationQuery::Create({ nestedQueryA, nestedQueryC }))
            .GroupByContract(*contract));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetSelectInstanceClasses().clear();
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relBC);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllRelatedInstanceNodes_GroupByClass_ChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C_1" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C_2" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByClass_ChildrenQuery)
    {
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relBC1 = GetECClass("B_C_1")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC2 = GetECClass("B_C_2")->GetRelationshipClassCP();

    auto rootInstanceNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classB);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, true, false, false, 0, GetECSchema()->GetName());

    auto groupingNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECClassGroupingNode(rootInstanceNode->GetKey().get(), *classC, false, "MyLabel", {});
    groupingNode->SetParentNode(*rootInstanceNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *groupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *groupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classC, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClass));

        ComplexNavigationQueryPtr nestedQuery1 = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC1, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC1, 0)), false, SelectClass<ECClass>(*classB, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));

        ComplexNavigationQueryPtr nestedQuery2 = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC2, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC2, 0)), false, SelectClass<ECClass>(*classB, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));

        UnionNavigationQueryPtr query = UnionNavigationQuery::Create({ nestedQuery1, nestedQuery2 });
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relBC1);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relBC2);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllRelatedInstanceNodes_GroupByLabel, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByLabel)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_C")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classB);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, true, 0, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", true);
        ComplexNavigationQueryPtr nestedQueryA = &ComplexNavigationQuery::Create()
            ->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create("", nullptr, classA, CreateDisplayLabelField(selectClassA)), "this")
            .From(selectClassA)
            .Where(Utf8PrintfString("[this].[ECInstanceId] IN (%s)",
                ComplexGenericQuery::Create()->SelectContract(*CreateRelatedInstancesQueryContract(), "relatedInstances")
                .From(*classA, true, "relatedInstances")
                .Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, "related", true), false))
                .Where("[related].[ECInstanceId] IN (?)", BoundQueryValuesList())
                .ToString().c_str()
            ).c_str(), { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) });

        SelectClass<ECClass> selectClassC(*classC, "this", true);
        ComplexNavigationQueryPtr nestedQueryC = &ComplexNavigationQuery::Create()
            ->SelectContract(*DisplayLabelGroupingNodesQueryContract::Create("", nullptr, classC, CreateDisplayLabelField(selectClassC)), "this")
            .From(selectClassC)
            .Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), false, SelectClass<ECClass>(*classB, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) });

        auto contract = DisplayLabelGroupingNodesQueryContract::Create("", nullptr, nullptr, CreateGroupingDisplayLabelField());
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, "this");
        query->From(*UnionNavigationQuery::Create({ nestedQueryA, nestedQueryC }));
        query->GroupByContract(*contract);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetSelectInstanceClasses().clear();
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relBC);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllRelatedInstanceNodes_GroupByLabel_ChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByLabel_ChildrenQuery)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_C")->GetRelationshipClassCP();

    auto rootInstanceNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classB);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, true, 0, GetECSchema()->GetName());

    auto groupingNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateDisplayLabelGroupingNode(rootInstanceNode->GetKey().get(), "MyLabel", 1);
    groupingNode->SetParentNode(*rootInstanceNode);
    groupingNode->SetInstanceKeysSelectQuery(StringGenericQuery::Create(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *groupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *groupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", true);
        ComplexNavigationQueryPtr nestedQueryA = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(
            *classA,
            SetLabelGroupingNodeChildrenWhereClause(
                ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA)), "this")
                .From(selectClassA)
                .Where(Utf8PrintfString("[this].[ECInstanceId] IN (%s)",
                    ComplexGenericQuery::Create()->SelectContract(*CreateRelatedInstancesQueryContract(), "relatedInstances")
                    .From(*classA, true, "relatedInstances")
                    .Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, "related", true), false))
                    .Where("[related].[ECInstanceId] IN (?)", BoundQueryValuesList())
                    .ToString().c_str()
                ).c_str(), { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) })
            ));

        SelectClass<ECClass> selectClassC(*classC, "this", true);
        ComplexNavigationQueryPtr nestedQueryC = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(
            *classC,
            SetLabelGroupingNodeChildrenWhereClause(
                ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClassC)), "this")
                .From(selectClassC)
                .Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), false, SelectClass<ECClass>(*classB, "related", true), false))
                .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) })
            ));

        UnionNavigationQueryPtr query = UnionNavigationQuery::Create({ nestedQueryA, nestedQueryC });
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relBC);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllRelatedInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery)
    {
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relBC = GetECClass("B_C")->GetRelationshipClassCP();

    auto rootInstanceNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classB);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, true, false, true, 0, GetECSchema()->GetName());

    auto groupingNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECClassGroupingNode(rootInstanceNode->GetKey().get(), *classC, false, "MyLabel", {});
    groupingNode->SetParentNode(*rootInstanceNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *groupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *groupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classC, "this", true);
        auto contract = DisplayLabelGroupingNodesQueryContract::Create("", nullptr, classC, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, "this");
        query->From(ComplexNavigationQuery::Create()
            ->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), false, SelectClass<ECClass>(*classB, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));
        query->GroupByContract(*contract);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetSelectInstanceClasses().clear();
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relBC);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllRelatedInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery)
    {
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relBC = GetECClass("B_C")->GetRelationshipClassCP();

    auto rootInstanceNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classB);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, true, false, true, 0, GetECSchema()->GetName());

    auto classGroupingNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECClassGroupingNode(rootInstanceNode->GetKey().get(), *classC, false, "Class Grouping Node", {});
    classGroupingNode->SetParentNode(*rootInstanceNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *classGroupingNode);

    auto labelGroupingNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateDisplayLabelGroupingNode(classGroupingNode->GetKey().get(), "Label Grouping Node", 1);
    labelGroupingNode->SetParentNode(*classGroupingNode);
    labelGroupingNode->SetInstanceKeysSelectQuery(StringGenericQuery::Create(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *labelGroupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *labelGroupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classC, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            SetLabelGroupingNodeChildrenWhereClause(
                ComplexNavigationQuery::Create()->SelectContract(*contract, "this")
                .From(selectClass)
                .Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), false, SelectClass<ECClass>(*classB, "related", true), false))
                .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) })
            ));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relBC);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllRelatedInstanceNodes_InstanceLabelOverride_AppliedByPriority, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop1" typeName="string" />
        <ECProperty propertyName="Prop2" typeName="string" />
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
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_InstanceLabelOverride_AppliedByPriority)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, GetECSchema()->GetName());
    spec.SetRequiredRelationDirection(RequiredRelationDirection_Forward);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(), "Prop1"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, false, classB->GetFullName(), "Prop2"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass, {},
            {
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Prop2")),
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Prop1")),
            }));

        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllRelatedInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ac" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ca" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relAC = GetECClass("A_C")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, GetECSchema()->GetName());
    spec.SetRequiredRelationDirection(RequiredRelationDirection_Forward);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, classC->GetFullName(), "PropC"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassB(*classB, "this", true);
        ComplexNavigationQueryPtr nestedQueryB = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB)), "this")
            .From(selectClassB)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));

        SelectClass<ECClass> selectClassC(*classC, "this", true);
        ComplexNavigationQueryPtr nestedQueryC = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClassC, {}, { &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("PropC")) })), "this")
            .From(selectClassC)
            .Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relAC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAC, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));

        UnionNavigationQueryPtr query = UnionNavigationQuery::Create({ nestedQueryB, nestedQueryC });
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relAC);
        return query;
        });
    }
