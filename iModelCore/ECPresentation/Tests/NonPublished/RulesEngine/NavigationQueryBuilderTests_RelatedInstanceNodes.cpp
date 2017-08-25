﻿/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NavigationQueryBuilderTests_RelatedInstanceNodes.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"

// note: some of the common specification properties of this specification are tested
//       QueryBuilderTests_AllRelatedInstanceNodes.cpp as they're common between the two.

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_SkipOneRelatedLevel_WidgetToSprocket)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Widget"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, false, 1, "", RequiredRelationDirection_Forward, "RulesEngineTest", "", "RulesEngineTest:Widget,Gadget,Sprocket");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_SkipOneRelatedLevel_WidgetToSprocket", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_SkipTooManyRelatedLevels_ReturnsNoQueries)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Widget"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);
    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, false, 2, "", RequiredRelationDirection_Forward, "RulesEngineTest", "", "");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_SkipRelatedLevel_GroupsByECInstanceKey)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Widget"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, false, 1, "", 
        RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_SkipRelatedLevel_GroupsByECInstanceKey", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RequiresRelatedClassesOrRelationshipClassNames)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Gadget"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);
    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both, "Basic1", "", "");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelationshipClassNames_OnlyIncluded)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Gadget"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);
    
    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both, "", "RulesEngineTest:WidgetHasGadget,GadgetHasSprockets", "");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_RelationshipClassNames_OnlyIncluded", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelationshipAndClassNamesOverrideSupportedSchemas)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Gadget"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);
    
    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both, "Basic1", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Widget");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_RelationshipClassNamesOverridesSupportedSchemas", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClasses_OnlyIncluded)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Gadget"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both, "", "", "RulesEngineTest:Sprocket");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_RelatedClasses_OnlyIncluded", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClassesAndRelationships_OnlyIncluded)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Gadget"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both, "", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Widget");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_RelatedClassesAndRelationships_OnlyIncluded", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClasses_AllDerivedClasses)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Sprocket"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both, "", "", "RulesEngineTest:Gadget;E:RulesEngineTest:Gadget");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_RelatedClasses_AllDerivedClasses", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClasses_PolymorphicExcludeInludedClass)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Sprocket"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both, "", "", "RulesEngineTest:Gadget;PE:RulesEngineTest:Gadget");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClasses_GroupByClassWithExcluded)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Sprocket"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    RelatedInstanceNodesSpecification spec(1, false, false, false, true, false, false, true, 0, "", RequiredRelationDirection_Both, "", "", "RulesEngineTest:Gadget;E:RulesEngineTest:Gadget");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_RelatedClasses_GroupByClassWithExcluded", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_InstanceFilter)
    {    
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Sprocket"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "this.Description = \"2\"", RequiredRelationDirection_Both, "", "", "RulesEngineTest:Gadget");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_InstanceFilter", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_InstanceFilter_LikeOperator)
    {    
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Sprocket"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "this.Description ~ \"Test\"", RequiredRelationDirection_Both, "", "", "RulesEngineTest:Gadget");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_InstanceFilter_LikeOperator", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_InstanceFilter_IsOfClassFunction)
    {    
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Sprocket"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "this.IsOfClass(\"ClassName\", \"SchemaName\")", RequiredRelationDirection_Both, "", "", "RulesEngineTest:Gadget");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_InstanceFilter_IsOfClassFunction", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_ShowEmptyGroups)
    {
    // WIP - need to support grouping by class first.
    FAIL();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClasses_MultipleExcludedClasses)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "ClassD"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both, 
        "", "RulesEngineTest:ClassDHasClassE", "RulesEngineTest:ClassE;PE:RulesEngineTest:ClassF;E:RulesEngineTest:ClassG");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_RelatedClasses_MultipleExcludedClasses", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_ExcludedWithDifferentSorting)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "ClassD"));
    parentNode->SetParentNodeId(NavNode::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);
    
    SortingRuleP sortingRule1 = new SortingRule("", 1, "RulesEngineTest", "ClassE", "IntProperty", true, false, false);
    m_ruleset->AddPresentationRule(*sortingRule1);

    SortingRuleP sortingRule2 = new SortingRule("", 1, "RulesEngineTest", "ClassG", "", true, true, false);
    m_ruleset->AddPresentationRule(*sortingRule2);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both, "", 
        "RulesEngineTest:ClassDHasClassE", "RulesEngineTest:ClassE;PE:RulesEngineTest:ClassF");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_ExcludedWithDifferentSorting", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }
