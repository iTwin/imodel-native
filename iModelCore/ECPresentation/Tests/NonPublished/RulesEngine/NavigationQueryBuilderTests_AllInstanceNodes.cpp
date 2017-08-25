/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NavigationQueryBuilderTests_AllInstanceNodes.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"

static JsonNavNodesFactory s_nodesFactory;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_SupportedSchemas_NoSpaces)
    {
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic1,Basic3");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_SupportedSchemas_NoSpaces", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_SupportedSchemas_WithSpaces)
    {
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic1, Basic3");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_SupportedSchemas_WithSpaces", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_SupportedSchemas_Excluded)
    {
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "E:Basic1,Basic3,SchemaComplex,SchemaComplex3,ECDbFileInfo,ECDbSystem,ECDbMeta");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_SupportedSchemas_Excluded", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_SupportedSchemas_UsedFromRulesetIfNotSpecified)
    {
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("NavigationQueryBuilderTests", 1, 0, false, "", "Basic1", "", false);
    GetBuilder().GetParameters().SetRuleset(*ruleset);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_SupportedSchemas_UsedFromRulesetIfNotSpecified", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_AllowsStandardSchemasIfExplicitylySpecified)
    {
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "ECDbMeta");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_TRUE(queries.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_DoNotSort)
    {
    AllInstanceNodesSpecification specification(1, false, false, false, false, false, "");
    specification.SetDoNotSort(true);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, specification);
    ASSERT_EQ(1, queries.size());
    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    EXPECT_FALSE(query->ToString().ContainsI("ORDER BY"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_HideNodesInHierarchy)
    {
    AllInstanceNodesSpecification spec(1, false, true, false, false, false, "");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    EXPECT_TRUE(query->GetResultParameters().GetNavNodeExtendedData().HideNodesInHierarchy());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_HideNodesInHierarchy_ElimintesAllGroupingStatements)
    {
    AllInstanceNodesSpecification spec(1, false, true, false, true, true, "");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    ASSERT_TRUE(nullptr != query->AsComplexQuery());
    EXPECT_TRUE(query->GetResultParameters().GetNavNodeExtendedData().HideNodesInHierarchy());
    EXPECT_FALSE(query->AsComplexQuery()->HasClause(CLAUSE_GroupBy));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_HideIfNoChildren)
    {
    AllInstanceNodesSpecification spec(1, false, false, true, false, false, "");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    EXPECT_TRUE(query->GetResultParameters().GetNavNodeExtendedData().HideIfNoChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_HideIfNoChildren_NotSetIfHiddenInHierarchy)
    {
    AllInstanceNodesSpecification spec(1, false, true, true, false, false, "");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    EXPECT_FALSE(query->GetResultParameters().GetNavNodeExtendedData().HideIfNoChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_CheckNotPolymorphic)
    {
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic4");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_CheckNotPolymorphic", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_GroupByClass)
    {
    AllInstanceNodesSpecification spec(1, false, false, false, true, false, "Basic1");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_GroupByClass", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_GroupByClass_ChildrenQuery)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    TestNavNodePtr parentNode = TestNodesHelper::CreateClassGroupingNode(*ecClass, "MyLabel");
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);
    
    AllInstanceNodesSpecification spec(1, false, false, false, true, false, "Basic1");

    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationId(spec.GetId());
    extendedData.SetGroupingType((int)GroupingType::Class);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_GroupByClass_ChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_GroupByLabel)
    {
    AllInstanceNodesSpecification spec(1, false, false, false, false, true, "Basic1");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_GroupByLabel", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_GroupByLabel_ChildrenQuery)
    {
    JsonNavNodePtr parentNode = TestNodesHelper::CreateLabelGroupingNode("MyLabel");
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    AllInstanceNodesSpecification spec(1, false, false, false, false, true, "Basic1");
    
    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationId(spec.GetId());
    extendedData.SetGroupingType((int)GroupingType::DisplayLabel);
    extendedData.SetGroupedInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_GroupByLabel_ChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    TestNavNodePtr parentNode = TestNodesHelper::CreateClassGroupingNode(*ecClass, "MyLabel");
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);
    
    AllInstanceNodesSpecification spec(1, false, false, false, true, true, "Basic1");

    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationId(spec.GetId());
    extendedData.SetGroupingType((int)GroupingType::Class);
    extendedData.SetGroupedInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    TestNavNodePtr classGroupingNode = TestNodesHelper::CreateClassGroupingNode(*ecClass, "Class Grouping Node");
    JsonNavNodePtr labelGroupingNode = TestNodesHelper::CreateLabelGroupingNode("Label Grouping Node");
    classGroupingNode->SetParentNodeId(NavNode::CreateNodeId());
    labelGroupingNode->SetParentNode(*classGroupingNode);
    m_nodesCache.Cache(*classGroupingNode, false);
    m_nodesCache.Cache(*labelGroupingNode, false);

    AllInstanceNodesSpecification spec(1, false, false, false, true, true, "Basic1");

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
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_RecursiveNodeRelationships)
    {
    ECClassCP ecClass = GetECClass("Basic3", "Class3");
    
    IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    TestNavNodePtr instanceNode = TestNodesHelper::CreateInstanceNode(*instance);
    instanceNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*instanceNode, false);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic3");
        
    NavNodeExtendedData instanceNodeExtendedData(*instanceNode);
    instanceNodeExtendedData.SetSpecificationId(spec.GetId());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *instanceNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_RecursiveNodeRelationships", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }
