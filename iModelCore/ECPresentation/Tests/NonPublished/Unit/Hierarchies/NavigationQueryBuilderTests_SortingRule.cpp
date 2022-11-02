/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "NavigationQueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortByRulesAndLabelAndNotSorted, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F (NavigationQueryBuilderTests, SortingRule_SortByRulesAndLabelAndNotSorted)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");

    m_ruleset->AddPresentationRule(*new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "PropA", true, false, false));
    m_ruleset->AddPresentationRule(*new SortingRule("", 1, classB->GetSchema().GetName(), classB->GetName(), "", true, true, false));
    // note: "C" doesn't have a sorting rule and is going to be sorted by label
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", true);
        ComplexNavigationQueryPtr rulesSortedQuery = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA)), "this")
            .From(selectClassA)
            .OrderBy("[this].[PropA]"));

        SelectClass<ECClass> selectClassC(*classC, "this", true);
        ComplexNavigationQueryPtr labelSortedQuery = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classC, CreateDisplayLabelField(selectClassC)), "this")
            .From(selectClassC));
        labelSortedQuery->OrderBy(GetECInstanceNodesOrderByClause().c_str());

        ComplexNavigationQueryPtr labelSortedQueryWrapper = ComplexNavigationQuery::Create();
        labelSortedQueryWrapper->SelectAll().From(*labelSortedQuery);

        SelectClass<ECClass> selectClassB(*classB, "this", true);
        ComplexNavigationQueryPtr notSortedQuery = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB)), "this")
            .From(selectClassB));

        return UnionNavigationQuery::Create({ rulesSortedQuery, labelSortedQueryWrapper, notSortedQuery });
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortSingleECClassByRule, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, SortingRule_SortSingleECClassByRule)
    {
    ECClassCP classA = GetECClass("A");

    m_ruleset->AddPresentationRule(*new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "PropA", true, false, false));
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr rulesSortedQuery = &ComplexNavigationQuery::Create()->SelectContract(*contract, "this")
            .From(selectClass)
            .OrderBy(Utf8PrintfString("%s([this].[%s])", FUNCTION_NAME_GetSortingValue, "PropA").c_str());
        return RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *rulesSortedQuery);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortSingleECClassByDoNotSortRule, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, SortingRule_SortSingleECClassByDoNotSortRule)
    {
    ECClassCP classA = GetECClass("A");

    m_ruleset->AddPresentationRule(*new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "", true, true, false));
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr notSortedQuery = ComplexNavigationQuery::Create();
        notSortedQuery->SelectContract(*contract, "this");
        notSortedQuery->From(selectClass);
        return RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *notSortedQuery);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortDescending, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, SortingRule_SortDescending)
    {
    ECClassCP classA = GetECClass("A");

    m_ruleset->AddPresentationRule(*new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "PropA", false, false, false));
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr rulesSortedQuery = ComplexNavigationQuery::Create();
        rulesSortedQuery->SelectContract(*contract, "this");
        rulesSortedQuery->From(selectClass);
        rulesSortedQuery->OrderBy(Utf8PrintfString("%s([this].[%s]) DESC", FUNCTION_NAME_GetSortingValue, "PropA").c_str());
        return RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *rulesSortedQuery);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_AppliedToAllSchemaClasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, SortingRule_AppliedToAllSchemaClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    m_ruleset->AddPresentationRule(*new SortingRule("", 1, GetECSchema()->GetName(), "", "Prop", true, false, false));
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", true);
        ComplexNavigationQueryPtr queryA = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA)), "this")
            .From(selectClassA)
            .OrderBy(Utf8PrintfString("%s([this].[%s])", FUNCTION_NAME_GetSortingValue, "Prop").c_str()));

        SelectClass<ECClass> selectClassB(*classB, "this", true);
        ComplexNavigationQueryPtr queryB = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB)), "this")
            .From(selectClassB)
            .OrderBy(Utf8PrintfString("%s([this].[%s])", FUNCTION_NAME_GetSortingValue, "Prop").c_str()));

        return UnionNavigationQuery::Create({ queryA, queryB });
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_AppliedToClassesOfAllSchemas, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, SortingRule_AppliedToClassesOfAllSchemas)
    {
    ECClassCP classA = GetECClass("A");

    m_ruleset->AddPresentationRule(*new SortingRule("", 1, "", "", "Prop", true, false, false));

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass)), "this")
            .From(selectClass)
            .OrderBy(Utf8PrintfString("%s([this].[%s])", FUNCTION_NAME_GetSortingValue, "Prop").c_str()));
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_IgnoresRulesWithInvalidProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, SortingRule_IgnoresRulesWithInvalidProperty)
    {
    ECClassCP classA = GetECClass("A");

    m_ruleset->AddPresentationRule(*new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "InvalidProperty", true, false, false));
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));

        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, "this");
        query->From(selectClass);

        ComplexNavigationQueryPtr labelSorted = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *query);
        labelSorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());

        return labelSorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_OverridenBySpecificationsDoNotSortFlag, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, SortingRule_OverridenBySpecificationsDoNotSortFlag)
    {
    ECClassCP classA = GetECClass("A");

    m_ruleset->AddPresentationRule(*new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "Prop", true, false, false));

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());
    spec.SetDoNotSort(true);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));

        ComplexNavigationQueryPtr notSortedQuery = ComplexNavigationQuery::Create();
        notSortedQuery->SelectContract(*contract, "this");
        notSortedQuery->From(selectClass);

        return RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *notSortedQuery);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_DoesntSortWhenGroupingByClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, SortingRule_DoesntSortWhenGroupingByClass)
    {
    ECClassCP classA = GetECClass("A");

    m_ruleset->AddPresentationRule(*new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "Prop", true, false, false));

    AllInstanceNodesSpecification spec(1, false, false, false, true, false, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create("", nullptr);

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(*classA, true, "this"));
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortsByProperty_WhenUsingParentInstanceOfTheSameClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F(NavigationQueryBuilderTests, SortingRule_SortsByProperty_WhenUsingParentInstanceOfTheSameClass)
    {
    ECClassCP classA = GetECClass("A");

    m_ruleset->AddPresentationRule(*new SortingRule("", 1, classA->GetSchema().GetName(), classA->GetName(), "Prop", true, false, false));

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "this.Prop = parent.Prop", classA->GetFullName(), false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, "this")
            .From(selectClass)
            .From(*classA, false, "parent")
            .Where("[parent].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) })
            .Where("[this].[Prop] = [parent].[Prop]", BoundQueryValuesList())
            .OrderBy(Utf8PrintfString("%s([this].[%s])", FUNCTION_NAME_GetSortingValue, "Prop").c_str());
        return RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *query);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SortingRule_SortsUsingRelatedInstanceProperty, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(NavigationQueryBuilderTests, SortingRule_SortsUsingRelatedInstanceProperty)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    m_ruleset->AddPresentationRule(*new SortingRule("", 1, classB->GetSchema().GetName(), classB->GetName(), "Prop", true, false, false));

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, relAB->GetFullName(), classB->GetFullName(), "b"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        RelatedClass relatedInstanceClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, "b", true));
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass, { RelatedClassPath{relatedInstanceClass} }), { RelatedClassPath{relatedInstanceClass} });
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(relatedInstanceClass)
            .OrderBy("[b].[Prop]");
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        return RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *query);
        });
    }
