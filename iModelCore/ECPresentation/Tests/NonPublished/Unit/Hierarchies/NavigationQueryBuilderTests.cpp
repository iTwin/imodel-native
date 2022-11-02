/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "NavigationQueryBuilderTests.h"
#include "../../../../Source/Shared/RulesPreprocessor.h"

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void NavigationQueryBuilderTests::SetUp()
    {
    QueryBuilderTest::SetUp();

    m_ruleset = PresentationRuleSet::CreateInstance("NavigationQueryBuilderTests");
    m_rulesPreprocessor = std::make_unique<RulesPreprocessor>(GetConnections(), GetConnection(), *m_ruleset,
        m_rulesetVariables, nullptr, GetSchemaHelper().GetECExpressionsCache());

    NavigationQueryBuilderParameters builderParams(GetSchemaHelper(), GetConnections(), GetConnection(), &GetCancellationToken(), 
        *m_rulesPreprocessor, *m_ruleset, m_rulesetVariables, nullptr, GetSchemaHelper().GetECExpressionsCache(), m_nodesCache);
    builderParams.SetUseSpecificationIdentifierInContracts(false);
    m_builder = std::make_unique<NavigationQueryBuilder>(builderParams);

    m_rootNodeRule = std::make_unique<RootNodeRule>();
    m_childNodeRule = std::make_unique<ChildNodeRule>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryPtr NavigationQueryBuilderTests::PrepareNavigationQuery(std::function<NavigationQueryPtr()> queryFactory)
    {
    NavigationQueryPtr query = queryFactory();
    Utf8String queryStr = query->ToString();
    PrepareQuery(queryStr);
    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavigationQueryBuilderTests::ValidateQuery(ChildNodeSpecificationCR spec, NavigationQueryPtr actualQuery, std::function<NavigationQueryPtr()> expectedQueryFactory)
    {
    auto expected = PrepareNavigationQuery(expectedQueryFactory);
    expected->GetResultParametersR().SetSpecification(&spec);
    expected->GetResultParametersR().GetNavNodeExtendedDataR().SetRulesetId(m_ruleset->GetRuleSetId().c_str());

    ASSERT_TRUE(actualQuery.IsValid());
    EXPECT_TRUE(expected->IsEqual(*actualQuery))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << actualQuery->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldPtr NavigationQueryBuilderTests::CreateGroupingDisplayLabelField()
    {
    auto field = PresentationQueryContractSimpleField::Create(DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName, DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName, false);
    field->SetGroupingClause(QueryBuilderHelpers::CreateDisplayLabelValueClause(field->GetName()));
    field->SetResultType(PresentationQueryFieldType::LabelDefinition);
    return field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<SimpleQueryContract> NavigationQueryBuilderTests::CreateRelatedInstancesQueryContract()
    {
    auto field = PresentationQueryContractSimpleField::Create("/RelatedInstanceId/", "ECInstanceId", true, false, FieldVisibility::Inner);
    return SimpleQueryContract::Create({ field });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<SimpleQueryContract> NavigationQueryBuilderTests::CreateSelect1QueryContract()
    {
    auto field = PresentationQueryContractSimpleField::Create("", "1", false, false, FieldVisibility::Inner);
    return SimpleQueryContract::Create({ field });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexNavigationQueryR NavigationQueryBuilderTests::SetLabelGroupingNodeChildrenWhereClause(ComplexNavigationQueryR query)
    {
    auto labelClause = query.GetContract()->GetField(ECInstanceNodesQueryContract::DisplayLabelFieldName)->GetSelectClause("this");
    Utf8PrintfString whereClause("%s = ?", labelClause.GetClause().c_str());
    BoundQueryValuesList whereClauseBindings(labelClause.GetBindings());
    whereClauseBindings.push_back(std::make_unique<BoundQueryECValue>(ECValue(LabelDefinition::Create(LABEL_GROUPING_NODE_LABEL)->ToJsonString().c_str())));
    query.Where(whereClause.c_str(), whereClauseBindings);
    return query;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String NavigationQueryBuilderTests::GetECInstanceNodesOrderByClause()
    {
    static Utf8PrintfString clause("%s([%s]) IS NULL, %s([%s])", FUNCTION_NAME_GetSortingValue, ECInstanceNodesQueryContract::DisplayLabelFieldName, FUNCTION_NAME_GetSortingValue, ECInstanceNodesQueryContract::DisplayLabelFieldName);
    return clause;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String NavigationQueryBuilderTests::GetLabelGroupingNodesOrderByClause()
    {
    static Utf8PrintfString clause("%s([%s]) IS NULL, %s([%s])", FUNCTION_NAME_GetSortingValue, DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName, FUNCTION_NAME_GetSortingValue, DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName);
    return clause;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String NavigationQueryBuilderTests::GetECPropertyGroupingNodesOrderByClause()
    {
    static Utf8PrintfString clause("%s([%s]) IS NULL, %s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName, FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName);
    return clause;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String NavigationQueryBuilderTests::GetECClassGroupingNodesOrderByClause()
    {
    static Utf8PrintfString clause("%s([%s]) IS NULL, %s([%s])", FUNCTION_NAME_GetSortingValue, ECClassGroupingNodesQueryContract::DisplayLabelFieldName, FUNCTION_NAME_GetSortingValue, ECClassGroupingNodesQueryContract::DisplayLabelFieldName);
    return clause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(NotifiesAboutUsedClassesInFromClause, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, NotifiesAboutUsedClassesInFromClause)
    {
    ECClassCP classA = GetECClass("A");

    TestUsedClassesListener listener;
    GetBuilder().GetParameters().SetUsedClassesListener(&listener);

    RootNodeRule rule("", 1000, false, TargetTree_MainTree, false);
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false);

    GetBuilder().GetQueries(rule, spec);
    ASSERT_EQ(1, listener.GetUsedClasses().size());
    auto iter = listener.GetUsedClasses().find(classA);
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_FALSE(iter->second); // not polymorphic
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(NotifiesAboutUsedPolymorphicClassesInFromClause, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, NotifiesAboutUsedPolymorphicClassesInFromClause)
    {
    ECClassCP classA = GetECClass("A");

    TestUsedClassesListener listener;
    GetBuilder().GetParameters().SetUsedClassesListener(&listener);

    RootNodeRule rule("", 1000, false, TargetTree_MainTree, false);
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false,
        "", classA->GetFullName(), true);

    GetBuilder().GetQueries(rule, spec);
    ASSERT_EQ(1, listener.GetUsedClasses().size());
    auto iter = listener.GetUsedClasses().find(classA);
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(NotifiesAboutUsedClassesInJoins, R"*(
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
TEST_F (NavigationQueryBuilderTests, NotifiesAboutUsedClassesInJoins)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    TestUsedClassesListener listener;
    GetBuilder().GetParameters().SetUsedClassesListener(&listener);

    auto parentNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    ChildNodeRule rule("", 1000, false, TargetTree_MainTree);
    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, false,
        0, "", RequiredRelationDirection_Forward, relAB->GetSchema().GetName(), relAB->GetFullName(), classB->GetFullName());

    GetBuilder().GetQueries(rule, spec, *parentNode);
    ASSERT_EQ(3, listener.GetUsedClasses().size());

    auto iter = listener.GetUsedClasses().find(classA);
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic

    iter = listener.GetUsedClasses().find(relAB);
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic

    iter = listener.GetUsedClasses().find(classB);
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(NotifiesAboutUsedRelatedClassesInInstanceFilter, R"*(
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
TEST_F (NavigationQueryBuilderTests, NotifiesAboutUsedRelatedClassesInInstanceFilter)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    TestUsedClassesListener listener;
    GetBuilder().GetParameters().SetUsedClassesListener(&listener);

    RootNodeRule rule("", 1000, false, TargetTree_MainTree, false);
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false,
        Utf8PrintfString("this.GetRelatedValue(\"%s\", \"Forward\", \"%s\", \"Prop\") = \"test\"", relAB->GetFullName(), classB->GetFullName()),
        classA->GetFullName(), false);

    GetBuilder().GetQueries(rule, spec);
    ASSERT_EQ(3, listener.GetUsedClasses().size());

    auto iter = listener.GetUsedClasses().find(classA);
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_FALSE(iter->second); // not polymorphic

    iter = listener.GetUsedClasses().find(relAB);
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic

    iter = listener.GetUsedClasses().find(classB);
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(NotifiesAboutUsedRelatedInstanceClasses, R"*(
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
TEST_F (NavigationQueryBuilderTests, NotifiesAboutUsedRelatedInstanceClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    TestUsedClassesListener listener;
    GetBuilder().GetParameters().SetUsedClassesListener(&listener);

    RootNodeRule rule("", 1000, false, TargetTree_MainTree, false);
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, relAB->GetFullName(), classB->GetFullName(), "b"));

    GetBuilder().GetQueries(rule, spec);
    ASSERT_EQ(3, listener.GetUsedClasses().size());

    auto iter = listener.GetUsedClasses().find(classA);
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_FALSE(iter->second); // not polymorphic

    iter = listener.GetUsedClasses().find(relAB);
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic

    iter = listener.GetUsedClasses().find(classB);
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(JoinsWithAdditionalRelatedInstances, R"*(
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
TEST_F (NavigationQueryBuilderTests, JoinsWithAdditionalRelatedInstances)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classB->GetFullName(), false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward, relAB->GetFullName(), classA->GetFullName(), "a"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", false);
        RelatedClass relatedInstanceClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "a", true));
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass, { RelatedClassPath{relatedInstanceClass} }), { RelatedClassPath{relatedInstanceClass} });

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(relatedInstanceClass);

        ComplexNavigationQueryPtr sorted = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB, *nested);
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        sorted->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FiltersByRelatedInstanceProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="int" />
    </ECEntityClass>
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
TEST_F (NavigationQueryBuilderTests, FiltersByRelatedInstanceProperties)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "a.PropA > 5 AND a.PropA <> this.PropB", classB->GetFullName(), false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward, relAB->GetFullName(), classA->GetFullName(), "a"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", false);
        RelatedClass relatedInstanceClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "a", true));
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass, { RelatedClassPath{relatedInstanceClass} }), { RelatedClassPath{relatedInstanceClass} });

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(relatedInstanceClass)
            .Where("[a].[PropA] > 5 AND [a].[PropA] <> [this].[PropB]", BoundQueryValuesList());

        ComplexNavigationQueryPtr sorted = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB, *nested);
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        sorted->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(InnerJoinsWithAdditionalRelatedInstances, R"*(
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
TEST_F (NavigationQueryBuilderTests, InnerJoinsWithAdditionalRelatedInstances)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classB->GetFullName(), false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward, relAB->GetFullName(), classA->GetFullName(), "a", true));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", false);
        RelatedClass relatedInstanceClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "a", true), false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass, { RelatedClassPath{relatedInstanceClass} }), { RelatedClassPath{relatedInstanceClass} });

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(relatedInstanceClass);

        ComplexNavigationQueryPtr sorted = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classB, *nested);
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        sorted->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(JoinsWithAdditionalRelatedInstances_ReusesSameRelationship, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B1">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="AToB" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(1..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(NavigationQueryBuilderTests, JoinsWithAdditionalRelatedInstances_ReusesSameRelationship)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB1 = GetECClass("B1");
    ECClassCP classB2 = GetECClass("B2");
    ECRelationshipClassCP relAToB = GetECClass("AToB")->GetRelationshipClassCP();

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName() , false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification({*new RelationshipStepSpecification(relAToB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, classB1->GetFullName())}, "relatedB1", true));
    spec.AddRelatedInstance(*new RelatedInstanceSpecification({*new RelationshipStepSpecification(relAToB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, classB2->GetFullName())}, "relatedB2", true));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        RelatedClass relatedB1(*classA, SelectClass<ECRelationshipClass>(*relAToB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAToB, 0)), true, SelectClass<ECClass>(*classB1, "relatedB1", true), false);
        RelatedClass relatedB2(*classA, SelectClass<ECRelationshipClass>(*relAToB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAToB, 0)), true, SelectClass<ECClass>(*classB2, "relatedB2", true), false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass, { RelatedClassPath{relatedB1, relatedB2} }), { RelatedClassPath{relatedB1, relatedB2} });

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(relatedB1)
            .Join(relatedB2);

        ComplexNavigationQueryPtr sorted = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *nested);
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        sorted->GetResultParametersR().GetUsedRelationshipClasses().insert(relAToB);
        return sorted;
        });
    }
