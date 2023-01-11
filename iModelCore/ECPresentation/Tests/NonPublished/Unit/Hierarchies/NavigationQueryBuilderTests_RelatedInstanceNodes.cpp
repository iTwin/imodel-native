/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "NavigationQueryBuilderTests.h"

// note: some of the common specification properties of this specification are tested
//       QueryBuilderTests_AllRelatedInstanceNodes.cpp as they're common between the two.

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_SkipOneRelatedLevel_ForwardDirection, R"*(
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_SkipOneRelatedLevel_ForwardDirection)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_C")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, false, 1, "", RequiredRelationDirection_Forward,
        GetECSchema()->GetName(), "", RulesEngineTestHelpers::CreateClassNamesList({ classA, classB, classC }));
    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classC, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClass));
        RelatedClassPath relationshipPath
            {
            RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), false, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), false),
            RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false),
            };
        contract->SetPathFromSelectToParentClass(relationshipPath);
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(relationshipPath)
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));

        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relBC);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_SkipTooManyRelatedLevels_ReturnsNoQueries, R"*(
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_SkipTooManyRelatedLevels_ReturnsNoQueries)
    {
    ECClassCP classA = GetECClass("A");

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, false, 2, "",
        RequiredRelationDirection_Forward, GetECSchema()->GetName(), "", "");

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_SkipRelatedLevel_GroupsByECInstanceKey, R"*(
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_SkipRelatedLevel_GroupsByECInstanceKey)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_C")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, false, 1, "",
        RequiredRelationDirection_Forward, GetECSchema()->GetName(), relBC->GetFullName(), classC->GetFullName());
    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classC, "this", true);
        RelatedClassPath relationshipPath
            {
            RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), false, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), false),
            RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false),
            };
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClass));
        contract->SetPathFromSelectToParentClass(relationshipPath);
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(relationshipPath)
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relBC);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_RelationshipClassNames_OnlyIncluded, R"*(
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
    <ECRelationshipClass typeName="A_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ac" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ca" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelationshipClassNames_OnlyIncluded)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "",
        RequiredRelationDirection_Both, "", relAB->GetFullName(), "");
    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", true);
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass)), "this")
            .From(selectClass)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_RelationshipAndClassNamesOverrideSupportedSchemas, R"*(
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
    <ECRelationshipClass typeName="A_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ac" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ca" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelationshipAndClassNamesOverrideSupportedSchemas)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both,
        GetECSchema()->GetName(), relAB->GetFullName(), classB->GetFullName());

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass));

        ComplexQueryBuilderPtr nestedQuery = ComplexQueryBuilder::Create();
        nestedQuery->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) });

        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB, *nestedQuery);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_RelatedClasses_OnlyIncluded, R"*(
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
    <ECRelationshipClass typeName="A_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ac" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ca" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClasses_OnlyIncluded)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "",
        RequiredRelationDirection_Both, "", "", classB->GetFullName());

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_RelatedClassesAndRelationships_OnlyIncluded, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClassesAndRelationships_OnlyIncluded)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classD = GetECClass("D");
    ECRelationshipClassCP relAC = GetECClass("A_C")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "",
        RequiredRelationDirection_Both, "", relAC->GetFullName(), classD->GetFullName());

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classD, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classD, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr nestedQuery = ComplexQueryBuilder::Create();
        nestedQuery->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classD, SelectClass<ECRelationshipClass>(*relAC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAC, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) });
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classD, *nestedQuery);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAC);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_RelatedClasses_AllDerivedClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClasses_AllDerivedClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "",
        RequiredRelationDirection_Both, "", "", Utf8PrintfString("%s;E:%s", classB->GetFullName(), classB->GetFullName()));

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClassWithExcludes<ECClass> selectClass(*classB, "this", true);
        selectClass.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classB, "", false));
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_RelatedClasses_PolymorphicExcludeInludedClass, R"*(
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClasses_PolymorphicExcludeInludedClass)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "",
        RequiredRelationDirection_Both, "", "", Utf8PrintfString("%s;PE:%s", classB->GetFullName(), classB->GetFullName()));

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_RelatedClasses_GroupByClassWithExcluded, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClasses_GroupByClassWithExcluded)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, true, false, false, true, 0, "",
        RequiredRelationDirection_Both, "", "", Utf8PrintfString("%s;E:%s", classB->GetFullName(), classB->GetFullName()));

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create("", nullptr);
        SelectClassWithExcludes<ECClass> selectClass(*classB, "this", true);
        selectClass.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classB, "", false));

        ComplexQueryBuilderPtr nested = ComplexQueryBuilder::Create();
        nested->SelectContract(*contract, "this");
        nested->From(selectClass);
        nested->Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false));
        nested->Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) });

        ComplexQueryBuilderPtr grouped = ComplexQueryBuilder::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectAll();
        query->From(*grouped);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetSelectInstanceClasses().clear();
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_InstanceFilter, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_InstanceFilter)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "this.Prop = \"2\"",
        RequiredRelationDirection_Both, "", "", classB->GetFullName());

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) })
            .Where("[this].[Prop] = '2'", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_InstanceFilter_LikeOperator, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_InstanceFilter_LikeOperator)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "this.Prop ~ \"Test\"",
        RequiredRelationDirection_Both, "", "", classB->GetFullName());
    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) })
            .Where("CAST([this].[Prop] AS TEXT) LIKE 'Test' ESCAPE \'\\\'", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_InstanceFilter_IsOfClassFunction, R"*(
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_InstanceFilter_IsOfClassFunction)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "this.IsOfClass(\"ClassName\", \"SchemaName\")",
        RequiredRelationDirection_Both, "", "", classB->GetFullName());

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) })
            .Where("IsOfClass([this].[ECClassId], 'ClassName', 'SchemaName')", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_InstanceFilter_AllowsAccessingParentNode, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_InstanceFilter_AllowsAccessingParentNode)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0,
        "parent.Prop = 1", RequiredRelationDirection_Forward, "", relAB->GetFullName(), classB->GetFullName());

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr nested = ComplexQueryBuilder::Create();
        nested->SelectContract(*contract, "this")
            .From(selectClass)
            .From(*classA, false, "parent")
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) })
            .Where("[parent].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) })
            .Where("[parent].[Prop] = 1", BoundQueryValuesList());
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB, *nested);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_RelatedClasses_MultipleExcludedClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>B</BaseClass>
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClasses_MultipleExcludedClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECClassCP classD = GetECClass("D");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both,
        "", "", Utf8PrintfString("%s:%s;PE:%s:%s;E:%s:%s", classB->GetSchema().GetName().c_str(), classB->GetName().c_str(),
            classC->GetSchema().GetName().c_str(), classC->GetName().c_str(), classD->GetSchema().GetName().c_str(), classD->GetName().c_str()));

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClassWithExcludes<ECClass> selectClass(*classB, "this", true);
        selectClass.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classD, "", false));
        selectClass.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classC, "", true));
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr nested = ComplexQueryBuilder::Create();
        nested->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) });
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB, *nested);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_InstanceLabelOverride_AppliedByPriority, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop1" typeName="int" />
        <ECProperty propertyName="Prop2" typeName="int" />
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_InstanceLabelOverride_AppliedByPriority)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "",
        RequiredRelationDirection_Forward, "", relAB->GetFullName(), "");
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(), "Prop1"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, false, classB->GetFullName(), "Prop2"));

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", true);
        ComplexQueryBuilderPtr nested = ComplexQueryBuilder::Create();
        auto labelField = CreateDisplayLabelField(selectClass, {},
            {
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Prop2")),
            &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("Prop1")),
            });
        nested->SelectContract(*ECInstanceNodesQueryContract::Create("", classB, labelField), "this")
            .From(selectClass)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) });
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB, *nested);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="int" />
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
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relAC = GetECClass("A_C")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "",
        RequiredRelationDirection_Both, "", RulesEngineTestHelpers::CreateClassNamesList({ relAB, relAC }), "");
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "PropB"));

    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassB(*classB, "this", true);
        ComplexQueryBuilderPtr nestedQueryB = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB, {}, { &RegisterForDelete(*new InstanceLabelOverridePropertyValueSpecification("PropB")) })), "this")
            .From(selectClassB)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));

        SelectClass<ECClass> selectClassC(*classC, "this", true);
        ComplexQueryBuilderPtr nestedQueryC = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClassC)), "this")
            .From(selectClassC)
            .Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relAC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAC, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) }));

        UnionQueryBuilderPtr query = UnionQueryBuilder::Create({ nestedQueryB, nestedQueryC });
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAB);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAC);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_CreatesQueryForAllNodeECInstancesChildren, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="AToC" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BToC" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(NavigationQueryBuilderTests, RelatedInstanceNodes_CreatesQueryForAllNodeECInstancesChildren)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP rel1 = GetECClass("AToC")->GetRelationshipClassCP();
    ECRelationshipClassCP rel2 = GetECClass("BToC")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstancesNode(GetConnection(),
        {
        ECInstanceKey(classA->GetId(), ECInstanceId((uint64_t)1)),
        ECInstanceKey(classB->GetId(), ECInstanceId((uint64_t)2)),
        });
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Forward,
        "", Utf8PrintfString("%s:%s,%s", GetECSchema()->GetName().c_str(), rel1->GetName().c_str(), rel2->GetName().c_str()), classC->GetFullName());
    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    auto query = queries[0];
    ValidateQuery(spec, query, [&]()
        {
        SelectClass<ECClass> selectClass(*classC, "this", true);
        ComplexQueryBuilderPtr nestedQuery1 = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClass)), "this")
            .From(selectClass)
            .Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*rel1, RULES_ENGINE_RELATED_CLASS_ALIAS(*rel1, 0)), false, SelectClass<ECClass>(*classA, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)1)) }));

        ComplexQueryBuilderPtr nestedQuery2 = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClass)), "this")
            .From(selectClass)
            .Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*rel2, RULES_ENGINE_RELATED_CLASS_ALIAS(*rel2, 0)), false, SelectClass<ECClass>(*classB, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)2)) }));

        UnionQueryBuilderPtr query = UnionQueryBuilder::Create({ nestedQuery2, nestedQuery1 });
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());

        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(rel1);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(rel2);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_CreatesInClauseForOneToManyToOnePath, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="AToB" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BIsInC" strength="referencing" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns child" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(NavigationQueryBuilderTests, RelatedInstanceNodes_CreatesInClauseForOneToManyToOnePath)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAToB = GetECClass("AToB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBIsInC = GetECClass("BIsInC")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstancesNode(GetConnection(),
        {
        ECInstanceKey(classA->GetId(), ECInstanceId((uint64_t)1)),
        });
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, ChildrenHint::Unknown, false, false, false, false, "", { new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relAToB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, classB->GetFullName()),
        new RepeatableRelationshipStepSpecification(relBIsInC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, classC->GetFullName())
        })
    });
    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    auto query = queries[0];
    ValidateQuery(spec, query, [&]()
        {
        auto whereQuery = ComplexQueryBuilder::Create();
        RefCountedPtr<SimpleQueryContract> queryContract = SimpleQueryContract::Create({ PresentationQueryContractSimpleField::Create("/RelatedInstanceId/", "ECInstanceId", true, false, FieldVisibility::Inner) });
        whereQuery->SelectContract(*queryContract, "relatedInstances");
        whereQuery->From(SelectClass<ECClass>(*classC, "relatedInstances", true));
        whereQuery->Join({
            RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBIsInC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBIsInC, 0)), false, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), false),
            RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAToB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAToB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false)
            });
        whereQuery->Where(ValuesFilteringHelper(bvector<ECInstanceId>{ ECInstanceId((uint64_t)1) }).Create("[related].[ECInstanceId]"));
        Utf8String whereClause = Utf8String("[this].[ECInstanceId] IN (").append(whereQuery->GetQuery()->GetQueryString()).append(")");

        SelectClass<ECClass> selectClass(*classC, "this", true);
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClass)), "this")
            .From(selectClass)
            .Where(QueryClauseAndBindings(whereClause, whereQuery->GetQuery()->GetBindings())));

        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());

        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAToB);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relBIsInC);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_CreatesExistsClauseForOneToManyToOnePathOnNavigationProperty, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECNavigationProperty propertyName="A" relationshipName="AToB" direction="Backward" />
        <ECNavigationProperty propertyName="C" relationshipName="BIsInC" direction="Forward" />
    </ECEntityClass>
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="AToB" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BIsInC" strength="referencing" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns child" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(1..1)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(NavigationQueryBuilderTests, RelatedInstanceNodes_CreatesExistsClauseForOneToManyToOnePathOnNavigationProperty)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAToB = GetECClass("AToB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBIsInC = GetECClass("BIsInC")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstancesNode(GetConnection(),
        {
        ECInstanceKey(classA->GetId(), ECInstanceId((uint64_t)1)),
        });
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification(
            {
            new RepeatableRelationshipStepSpecification(relAToB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, classB->GetFullName()),
            new RepeatableRelationshipStepSpecification(relBIsInC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, classC->GetFullName())
            })
        });
    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    auto query = queries[0];
    ValidateQuery(spec, query, [&]()
        {
        auto whereQuery = ComplexQueryBuilder::Create();
        RefCountedPtr<SimpleQueryContract> queryContract = SimpleQueryContract::Create({ PresentationQueryContractSimpleField::Create("", "1", false, false, FieldVisibility::Inner) });
        whereQuery->SelectContract(*queryContract, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0).c_str());
        whereQuery->From(SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true, true));
        whereQuery->Join({ RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAToB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAToB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false) });
        whereQuery->Where(Utf8PrintfString("[this].[ECInstanceId] = [%s].[C].[Id]", RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0).c_str()).c_str(), BoundQueryValuesList());
        whereQuery->Where(ValuesFilteringHelper(bvector<ECInstanceId>{ ECInstanceId((uint64_t)1) }).Create("[related].[ECInstanceId]"));

        SelectClass<ECClass> selectClass(*classC, "this", true);
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            ComplexQueryBuilder::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClass)), "this")
            .From(selectClass)
            .Where(QueryClauseAndBindings(Utf8PrintfString("EXISTS (%s)", whereQuery->GetQuery()->GetQueryString().c_str()), whereQuery->GetQuery()->GetBindings())));

        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());

        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relAToB);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relBIsInC);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_HierarchyLevelInstanceFilter_OnDirectTargetClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="X" />
    <ECRelationshipClass typeName="XA" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="xa" polymorphic="true">
            <Class class="X"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ax" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(NavigationQueryBuilderTests, RelatedInstanceNodes_HierarchyLevelInstanceFilter_OnDirectTargetClass)
    {
    ECClassCP classX = GetECClass("X");
    ECClassCP classA = GetECClass("A");
    ECRelationshipClassCP relXA = GetECClass("XA")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstancesNode(GetConnection(),
        {
        ECInstanceKey(classX->GetId(), ECInstanceId((uint64_t)1)),
        });
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification(
            {
            new RepeatableRelationshipStepSpecification(
                relXA->GetFullName(),
                RequiredRelationDirection::RequiredRelationDirection_Forward),
            })
        });

    // first verify without instance filter
    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClassWithExcludes<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relXA, RULES_ENGINE_RELATED_CLASS_ALIAS(*relXA, 0)), false, SelectClass<ECClass>(*classX, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)1)) }));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relXA);
        return query;
        });

    // then with instance filter
    InstanceFilterDefinition instanceFilter("this.Prop = 123", *classA, {});
    GetBuilder().GetParameters().SetInstanceFilter(&instanceFilter);
    queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClassWithExcludes<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relXA, RULES_ENGINE_RELATED_CLASS_ALIAS(*relXA, 0)), false, SelectClass<ECClass>(*classX, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)1)) })
            .Where("[this].[Prop] = 123", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relXA);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_HierarchyLevelInstanceFilter_OnOneOfDirectTargetClasses, R"*(
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="X" />
    <ECRelationshipClass typeName="XA" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="xa" polymorphic="true">
            <Class class="X"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ax" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(NavigationQueryBuilderTests, RelatedInstanceNodes_HierarchyLevelInstanceFilter_OnOneOfDirectTargetClasses)
    {
    ECClassCP classX = GetECClass("X");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relXA = GetECClass("XA")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstancesNode(GetConnection(),
        {
        ECInstanceKey(classX->GetId(), ECInstanceId((uint64_t)1)),
        });
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
            {
            new RepeatableRelationshipStepSpecification(
                relXA->GetFullName(), 
                RequiredRelationDirection::RequiredRelationDirection_Forward, 
                RulesEngineTestHelpers::CreateClassNamesList({ classB, classC })),
            })
        });

    // first verify without instance filter
    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClassWithExcludes<ECClass> selectClassB(*classB, "this", true);
        NavigationQueryContractPtr contractB = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB));
        ComplexQueryBuilderPtr queryB = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*contractB, "this")
            .From(selectClassB)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relXA, RULES_ENGINE_RELATED_CLASS_ALIAS(*relXA, 0)), false, SelectClass<ECClass>(*classX, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)1)) }));

        SelectClassWithExcludes<ECClass> selectClassC(*classC, "this", true);
        NavigationQueryContractPtr contractC = ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClassC));
        ComplexQueryBuilderPtr queryC = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            ComplexQueryBuilder::Create()->SelectContract(*contractC, "this")
            .From(selectClassC)
            .Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relXA, RULES_ENGINE_RELATED_CLASS_ALIAS(*relXA, 1)), false, SelectClass<ECClass>(*classX, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)1)) }));

        UnionQueryBuilderPtr query = UnionQueryBuilder::Create({ queryB, queryC });
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relXA);
        return query;
        });

    // then with instance filter for B instances
    InstanceFilterDefinition instanceFilter("this.PropB = 123", *classB, {});
    GetBuilder().GetParameters().SetInstanceFilter(&instanceFilter);
    queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClassWithExcludes<ECClass> selectClass(*classB, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relXA, RULES_ENGINE_RELATED_CLASS_ALIAS(*relXA, 0)), false, SelectClass<ECClass>(*classX, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)1)) })
            .Where("[this].[PropB] = 123", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relXA);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_HierarchyLevelInstanceFilter_OnDerivedTargetClass, R"*(
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="X" />
    <ECRelationshipClass typeName="XA" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="xa" polymorphic="true">
            <Class class="X"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ax" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(NavigationQueryBuilderTests, RelatedInstanceNodes_HierarchyLevelInstanceFilter_OnDerivedTargetClass)
    {
    ECClassCP classX = GetECClass("X");
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relXA = GetECClass("XA")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstancesNode(GetConnection(),
        {
        ECInstanceKey(classX->GetId(), ECInstanceId((uint64_t)1)),
        });
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification(
            {
            new RepeatableRelationshipStepSpecification(
                relXA->GetFullName(),
                RequiredRelationDirection::RequiredRelationDirection_Forward),
            })
        });

    // first verify without instance filter
    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClassWithExcludes<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relXA, RULES_ENGINE_RELATED_CLASS_ALIAS(*relXA, 0)), false, SelectClass<ECClass>(*classX, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)1)) }));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relXA);
        return query;
        });

    // then with instance filter for B instances
    InstanceFilterDefinition instanceFilter("this.PropB = 123", *classB, {});
    GetBuilder().GetParameters().SetInstanceFilter(&instanceFilter);
    queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClassWithExcludes<ECClass> selectClass(*classB, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relXA, RULES_ENGINE_RELATED_CLASS_ALIAS(*relXA, 0)), false, SelectClass<ECClass>(*classX, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)1)) })
            .Where("[this].[PropB] = 123", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relXA);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_HierarchyLevelInstanceFilter_OnTargetRelatedClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="AB" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="b"/>
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="X" />
    <ECRelationshipClass typeName="XA" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="xa" polymorphic="true">
            <Class class="X"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ax" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(NavigationQueryBuilderTests, RelatedInstanceNodes_HierarchyLevelInstanceFilter_OnTargetRelatedClass)
    {
    ECClassCP classX = GetECClass("X");
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("AB")->GetRelationshipClassCP();
    ECRelationshipClassCP relXA = GetECClass("XA")->GetRelationshipClassCP();

    auto parentNode = TestNodesHelper::CreateInstancesNode(GetConnection(),
        {
        ECInstanceKey(classX->GetId(), ECInstanceId((uint64_t)1)),
        });
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, ChildrenHint::Unknown, false, false, false, false, "",
        {
        new RepeatableRelationshipPathSpecification(
            {
            new RepeatableRelationshipStepSpecification(
                relXA->GetFullName(),
                RequiredRelationDirection::RequiredRelationDirection_Forward),
            })
        });

    // first verify without instance filter
    auto queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClassWithExcludes<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relXA, RULES_ENGINE_RELATED_CLASS_ALIAS(*relXA, 0)), false, SelectClass<ECClass>(*classX, "related", true), false))
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)1)) }));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relXA);
        return query;
        });

    // then with instance filter using related B instance
    InstanceFilterDefinition instanceFilter("b.PropB = 123", *classA, 
        {
        RelatedClassPath{ RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "r"), true, SelectClass<>(*classB, "b"), true) }
        });
    GetBuilder().GetParameters().SetInstanceFilter(&instanceFilter);
    queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());
    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClassWithExcludes<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexQueryBuilderPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexQueryBuilder::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relXA, RULES_ENGINE_RELATED_CLASS_ALIAS(*relXA, 0)), false, SelectClass<ECClass>(*classX, "related", true), false))
            .Join(instanceFilter.GetRelatedInstances()[0])
            .Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)1)) })
            .Where("[b].[PropB] = 123", BoundQueryValuesList()));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(relXA);
        return query;
        });
    }
