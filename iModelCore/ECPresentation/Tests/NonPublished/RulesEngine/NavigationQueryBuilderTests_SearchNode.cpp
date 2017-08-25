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
    spec.GetQuerySpecificationsR().push_back(new StringQuerySpecification(query, "ECDbMeta", "ECSchemaDef"));
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_TRUE(queries.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, SearchResultInstanceNodes_NoGrouping)
    {
    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    spec.GetQuerySpecificationsR().push_back(new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));
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
    spec.GetQuerySpecificationsR().push_back(new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));
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
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);
    
    SearchResultInstanceNodesSpecification spec(1, false, false, false, true, false);
    spec.GetQuerySpecificationsR().push_back(new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));

    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationId(spec.GetId());
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
    spec.GetQuerySpecificationsR().push_back(new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));
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
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, true);
    spec.GetQuerySpecificationsR().push_back(new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));
    
    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationId(spec.GetId());
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
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);
    
    SearchResultInstanceNodesSpecification spec(1, false, false, false, true, true);
    spec.GetQuerySpecificationsR().push_back(new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));

    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationId(spec.GetId());
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
    classGroupingNode->SetParentNodeId(NavNode::CreateNodeId());
    labelGroupingNode->SetParentNode(*classGroupingNode);
    m_nodesCache.Cache(*classGroupingNode, false);
    m_nodesCache.Cache(*labelGroupingNode, false);

    SearchResultInstanceNodesSpecification spec(1, false, false, false, true, true);
    spec.GetQuerySpecificationsR().push_back(new StringQuerySpecification(SEARCH_NODE_QUERY, "RulesEngineTest", "Widget"));

    NavNodeExtendedData classNodeExtendedData(*classGroupingNode);
    NavNodeExtendedData labelNodeExtendedData(*labelGroupingNode);
    classNodeExtendedData.SetSpecificationId(spec.GetId());
    classNodeExtendedData.SetGroupingType((int)GroupingType::Class);
    labelNodeExtendedData.SetSpecificationId(spec.GetId());
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
    IECInstancePtr instance = GetECClass("RulesEngineTest", "Gadget")->GetDefaultStandaloneEnabler()->CreateInstance();
    instance->SetValue("Description", ECValue(SEARCH_NODE_QUERY));
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*instance);
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    SearchResultInstanceNodesSpecification spec(1, false, false, false, false, false);
    spec.GetQuerySpecificationsR().push_back(new ECPropertyValueQuerySpecification("RulesEngineTest", "Widget", "Description"));
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SearchResultInstanceNodes_UsesParentPropertyValueQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }
