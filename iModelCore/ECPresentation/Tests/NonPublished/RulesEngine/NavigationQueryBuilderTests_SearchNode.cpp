/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NavigationQueryBuilderTests_SearchNode.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"

// note: some of the common specification properties of this specification are tested
//       QueryBuilderTests_AllInstanceNodes.cpp as they're common between the two.

static JsonNavNodesFactory s_nodesFactory;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_NoQuery)
    {
    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_AllowsStandardSchemasIfExplicitylySpecified)
    {
    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    Utf8CP query = "SELECT * FROM [ECDbMap].[SchemaMap]";
    spec.AddQuerySpecification(*new StringQuerySpecification(query, "ECDbMeta", "ECSchemaDef"));
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_TRUE(queries.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_NoGrouping)
    {
    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SearchResultInstanceNodes_NoGrouping", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_GroupByClass)
    {
    SearchResultInstanceNodesSpecification spec(1, false, false, false, true, false);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SearchResultInstanceNodes_GroupByClass", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_GroupByClass_ChildrenQuery)
    {
    ECClassCP widget = GetECClass("RulesEngineTest", "Widget");
    TestNavNodePtr parentNode = TestNodesHelper::CreateClassGroupingNode(*widget, "MyLabel");
    m_nodesCache.Cache(*parentNode, false);
    
    SearchResultInstanceNodesSpecification spec(1, false, false, false, true, false);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));

    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Class);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SearchResultInstanceNodes_GroupByClass_ChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_GroupByLabel)
    {
    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, true);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SearchResultInstanceNodes_GroupByLabel", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_GroupByLabel_ChildrenQuery)
    {
    JsonNavNodePtr parentNode = TestNodesHelper::CreateLabelGroupingNode("MyLabel");
    m_nodesCache.Cache(*parentNode, false);

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, true);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));
    
    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::DisplayLabel);
    extendedData.SetGroupedInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SearchResultInstanceNodes_GroupByLabel_ChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery)
    {
    ECClassCP widget = GetECClass("RulesEngineTest", "Widget");
    TestNavNodePtr parentNode = TestNodesHelper::CreateClassGroupingNode(*widget, "MyLabel");
    m_nodesCache.Cache(*parentNode, false);
    
    SearchResultInstanceNodesSpecification spec(1, false, false, false, true, true);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));

    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Class);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SearchResultInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery)
    {
    ECClassCP widget = GetECClass("RulesEngineTest", "Widget");
    TestNavNodePtr classGroupingNode = TestNodesHelper::CreateClassGroupingNode(*widget, "Class Grouping Node");
    JsonNavNodePtr labelGroupingNode = TestNodesHelper::CreateLabelGroupingNode("Label Grouping Node");
    m_nodesCache.Cache(*classGroupingNode, false);
    labelGroupingNode->SetParentNode(*classGroupingNode);
    m_nodesCache.Cache(*labelGroupingNode, false);

    SearchResultInstanceNodesSpecification spec(1, false, false, false, true, true);
    spec.AddQuerySpecification(*new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));

    NavNodeExtendedData classNodeExtendedData(*classGroupingNode);
    NavNodeExtendedData labelNodeExtendedData(*labelGroupingNode);
    classNodeExtendedData.SetSpecificationHash(spec.GetHash());
    classNodeExtendedData.SetGroupingType((int)GroupingType::Class);
    labelNodeExtendedData.SetSpecificationHash(spec.GetHash());
    labelNodeExtendedData.SetGroupingType((int)GroupingType::DisplayLabel);
    labelNodeExtendedData.SetGroupedInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
        
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *labelGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SearchResultInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_UsesParentPropertyValueQuery)
    {
    ECDbR db = ExpectedQueries::GetInstance(BeTest::GetHost()).GetDb();
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(db, *GetECClass("RulesEngineTest", "Gadget"), [](IECInstanceR instance)
        {
        instance.SetValue("Description", ECValue(SEARCH_NODE_QUERY));
        });
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*instance);
    m_nodesCache.Cache(*parentNode, false);

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    spec.AddQuerySpecification(*new ECPropertyValueQuerySpecification("RulesEngineTest", "Widget", "Description"));
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SearchResultInstanceNodes_UsesParentPropertyValueQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }
