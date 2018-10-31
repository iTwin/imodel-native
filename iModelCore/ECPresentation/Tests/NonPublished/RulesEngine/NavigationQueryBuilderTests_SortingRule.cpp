/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NavigationQueryBuilderTests_SortingRule.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SortingRule_SortByRulesAndLabelAndNotSorted)
    {
    SortingRuleP sortingRule1 = new SortingRule("", 1, "Basic4", "ClassA", "SomeProperty", true, false, false);
    m_ruleset->AddPresentationRule(*sortingRule1);

    SortingRuleP sortingRule2 = new SortingRule("", 1, "Basic4", "ClassB", "", true, true, false);
    m_ruleset->AddPresentationRule(*sortingRule2);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic4");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SortingRule_SortByRulesAndLabelAndNotSorted", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SortingRule_SortSingleECClassByRule)
    {
    SortingRuleP sortingRule1 = new SortingRule("", 1, "Basic3", "Class3", "SomeProperty", true, false, false);
    m_ruleset->AddPresentationRule(*sortingRule1);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic3");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SortingRule_SortSingleECClassByRule", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SortingRule_SortSingleECClassByDoNotSortRule)
    {
    SortingRuleP sortingRule1 = new SortingRule("", 1, "Basic3", "Class3", "SomeProperty", true, true, false);
    m_ruleset->AddPresentationRule(*sortingRule1);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic3");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SortingRule_SortSingleECClassByDoNotSortRule", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SortingRule_SortDescending)
    {
    SortingRuleP sortingRule1 = new SortingRule("", 1, "Basic3", "Class3", "SomeProperty", false, false, false);
    m_ruleset->AddPresentationRule(*sortingRule1);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic3");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SortingRule_SortDescending", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SortingRule_AppliedToAllSchemaClasses)
    {
    SortingRuleP sortingRule1 = new SortingRule("", 1, "Basic3", "", "SomeProperty", true, false, false);
    m_ruleset->AddPresentationRule(*sortingRule1);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic2,Basic3");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SortingRule_AppliedToAllSchemaClasses", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SortingRule_AppliedToClassesOfAllSchemas)
    {
    SortingRuleP sortingRule1 = new SortingRule("", 1, "", "", "SomeProperty", true, false, false);
    m_ruleset->AddPresentationRule(*sortingRule1);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic3,Basic4");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SortingRule_AppliedToClassesOfAllSchemas", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SortingRule_SortByInvalidProperty)
    {
    SortingRuleP sortingRule = new SortingRule("", 1, "Basic3", "Class3", "InvalidProperty", true, false, false);
    m_ruleset->AddPresentationRule(*sortingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic3");

    IGNORE_BE_ASSERT();

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SortingRule_SortByInvalidProperty", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SortingRule_OverridenBySpecificationsDoNotSortFlag)
    {
    SortingRuleP sortingRule = new SortingRule("", 1, "Basic3", "Class3", "SomeProperty", true, false, false);
    m_ruleset->AddPresentationRule(*sortingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic3");
    spec.SetDoNotSort(true);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SortingRule_OverridenBySpecificationsDoNotSortFlag", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SortingRule_DoesntSortWhenGroupingByClass)
    {
    SortingRuleP sortingRule = new SortingRule("", 1, "Basic3", "Class3", "SomeProperty", true, false, false);
    m_ruleset->AddPresentationRule(*sortingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, true, false, "Basic3");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SortingRule_DoesntSortWhenGroupingByClass", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SortingRule_DoesntSortWhenGroupingByLabel)
    {
    SortingRuleP sortingRule = new SortingRule("", 1, "Basic3", "Class3", "SomeProperty", true, false, false);
    m_ruleset->AddPresentationRule(*sortingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, true, "Basic3");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SortingRule_DoesntSortWhenGroupingByLabel", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SortingRule_SortsAlphanumerically)
    {
    SortingRuleP sortingRule = new SortingRule("", 1, "Basic3", "Class3", "SomeProperty", true, false, false);
    m_ruleset->AddPresentationRule(*sortingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, true, "Basic3");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SortingRule_DoesntSortWhenGroupingByLabel", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryBuilderTests, SortingRule_SortsByProperty_WhenUsingParentInstanceOfTheSameClass)
    {
    SortingRuleP sortingRule = new SortingRule("", 1, "Basic3", "Class3", "SomeProperty", true, false, false);
    m_ruleset->AddPresentationRule(*sortingRule);

    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("Basic3", "Class3"));
    Cache(*parentNode);
   
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "this.SomeProperty = parent.SomeProperty", "Basic3:Class3", false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SortingRule_SortsByProperty_WhenUsingParentInstanceOfTheSameClass", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryBuilderTests, SortingRule_SortsUsingRelatedInstanceProperty)
    {
    SortingRuleP sortingRule = new SortingRule("", 1, "RulesEngineTest", "Widget", "IntProperty", true, false, false);
    m_ruleset->AddPresentationRule(*sortingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SortingRule_SortsUsingRelatedInstanceProperty", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }