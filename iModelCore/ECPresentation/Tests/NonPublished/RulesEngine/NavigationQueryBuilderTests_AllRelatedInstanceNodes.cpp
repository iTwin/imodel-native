/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"

// note: some of the common specification properties of this specification are tested
//       QueryBuilderTests_AllInstanceNodes.cpp as they're common between the two.

static JsonNavNodesFactory s_nodesFactory;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_NullParentNode_ReturnsNoQuery)
    {
    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, "RulesEngineTest");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_NonInstanceParentNode_ReturnsNoQuery)
    {
    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, "RulesEngineTest");
    TestNavNodePtr parentNode = TestNodesHelper::CreateCustomNode(*m_connection, "TestType", "label", "");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_SupportedSchemas_DifferentThanRootInstanceSchema)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);
    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, "Basic1");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_AllowsStandardSchemasIfExplicitylySpecified)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("ECDbMeta", "ECSchemaDef"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);
    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, "ECDbMeta");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_NoGrouping_ForwardRelationDirection)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, "RulesEngineTest");
    spec.SetRequiredRelationDirection(RequiredRelationDirection_Forward);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_NoGrouping_ForwardRelationDirection", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_NoGrouping_BackwardRelationDirection)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, "RulesEngineTest");
    spec.SetRequiredRelationDirection(RequiredRelationDirection_Backward);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_NoGrouping_BackwardRelationDirection", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_NoGrouping_BothDirections)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, "RulesEngineTest");
    spec.SetRequiredRelationDirection(RequiredRelationDirection_Both);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_NoGrouping_BothDirections", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByClass)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);
    AllRelatedInstanceNodesSpecification spec(1, false, false, false, true, false, false, 0, "RulesEngineTest");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *rootInstanceNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_GroupByClass", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByClass_ChildrenQuery)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    TestNavNodePtr groupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"), "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);
    groupingNode->SetParentNode(*rootInstanceNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *groupingNode);
    NavNodeExtendedData extendedData(*groupingNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, true, false, false, 0, "RulesEngineTest");
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Class);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *groupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_GroupByClass_ChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByRelationship_ChildrenQuery)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"));
    TestNavNodePtr relationshipGroupingNode = TestNodesHelper::CreateRelationshipGroupingNode(*m_connection, *GetECClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP(), "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);
    relationshipGroupingNode->SetParentNode(*rootInstanceNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *relationshipGroupingNode);
    NavNodeExtendedData extendedData(*relationshipGroupingNode);
    
    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, true, false, 0, "RulesEngineTest");
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Relationship);
    extendedData.SetParentECClassId(GetECClass("RulesEngineTest", "Widget")->GetId());
    extendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Forward);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *relationshipGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_GroupByRelationship_ChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByRelationship_MultipleClassesInRelationship)
    {
    ECClassCR classA = *GetECClass("RulesEngineTest", "ClassA");
    ECRelationshipClassCR classAHasBAndC = *GetECClass("RulesEngineTest", "ClassAHasBAndC")->GetRelationshipClassCP();
    
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, classA);
    TestNavNodePtr relationshipGroupingNode = TestNodesHelper::CreateRelationshipGroupingNode(*m_connection, classAHasBAndC, "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);
    relationshipGroupingNode->SetParentNode(*rootInstanceNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *relationshipGroupingNode);
    NavNodeExtendedData extendedData(*relationshipGroupingNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, true, false, 0, "RulesEngineTest");
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Relationship);
    extendedData.SetParentECClassId(GetECClass("RulesEngineTest", "ClassA")->GetId());
    extendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Forward);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *relationshipGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_GroupByRelationship_MultipleClassesInRelationship", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByLabel)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, true, 0, "RulesEngineTest");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_GroupByLabel", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByLabel_ChildrenQuery)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Sprocket"));
    JsonNavNodePtr groupingNode = TestNodesHelper::CreateLabelGroupingNode(*m_connection, "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);
    groupingNode->SetParentNode(*rootInstanceNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *groupingNode);
    NavNodeExtendedData extendedData(*groupingNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, true, 0, "RulesEngineTest");
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::DisplayLabel);
    extendedData.SetGroupedInstanceKeys({ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1))});

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *groupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_GroupByLabel_ChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    TestNavNodePtr groupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"), "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);
    groupingNode->SetParentNode(*rootInstanceNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *groupingNode);
    NavNodeExtendedData extendedData(*groupingNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, true, false, true, 0, "RulesEngineTest");
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Class);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *groupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_GroupByClassAndLabel_ClassNodeChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    TestNavNodePtr classGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"), "Class Grouping Node");
    JsonNavNodePtr labelGroupingNode = TestNodesHelper::CreateLabelGroupingNode(*m_connection, "Label Grouping Node");
    
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *classGroupingNode);
    labelGroupingNode->SetParentNode(*classGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);
    classGroupingNode->SetParentNode(*rootInstanceNode); 
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *labelGroupingNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, true, false, true, 0, "RulesEngineTest");
    
    NavNodeExtendedData classNodeExtendedData(*classGroupingNode);
    NavNodeExtendedData labelNodeExtendedData(*labelGroupingNode);
    classNodeExtendedData.SetSpecificationHash(spec.GetHash());
    classNodeExtendedData.SetGroupingType((int)GroupingType::Class);
    labelNodeExtendedData.SetSpecificationHash(spec.GetHash());
    labelNodeExtendedData.SetGroupingType((int)GroupingType::DisplayLabel);
    labelNodeExtendedData.SetGroupedInstanceKeys({ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1))});

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *labelGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_GroupByClassAndLabel_LabelNodeChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByRelationshipAndClass_RelationshipNodeChildrenQuery)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"));
    TestNavNodePtr relationshipGroupingNode = TestNodesHelper::CreateRelationshipGroupingNode(*m_connection, *GetECClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP(), "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);
    relationshipGroupingNode->SetParentNode(*rootInstanceNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *relationshipGroupingNode);
    NavNodeExtendedData extendedData(*relationshipGroupingNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, true, true, false, 0, "RulesEngineTest");
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Relationship);
    extendedData.SetParentECClassId(GetECClass("RulesEngineTest", "Widget")->GetId());
    extendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Forward);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *relationshipGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_GroupByRelationshipAndClass_RelationshipNodeChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByRelationshipAndClass_ClassNodeChildrenQuery)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"));
    TestNavNodePtr relationshipGroupingNode = TestNodesHelper::CreateRelationshipGroupingNode(*m_connection, *GetECClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP(), "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);
    relationshipGroupingNode->SetParentNode(*rootInstanceNode);
    TestNavNodePtr classGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"), "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *relationshipGroupingNode);
    classGroupingNode->SetParentNode(*relationshipGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *classGroupingNode);
    NavNodeExtendedData relationshipNodeExtendedData(*relationshipGroupingNode);
    NavNodeExtendedData classNodeExtendedData(*classGroupingNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, true, true, false, 0, "RulesEngineTest");
    relationshipNodeExtendedData.SetParentECClassId(GetECClass("RulesEngineTest", "Widget")->GetId());
    relationshipNodeExtendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
    relationshipNodeExtendedData.SetSpecificationHash(spec.GetHash());
    relationshipNodeExtendedData.SetGroupingType((int)GroupingType::Relationship);
    classNodeExtendedData.SetSpecificationHash(spec.GetHash());
    classNodeExtendedData.SetGroupingType((int)GroupingType::Class);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *classGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_GroupByRelationshipAndClass_ClassNodeChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByRelationshipAndLabel_RelationshipNodeChildrenQuery)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"));
    TestNavNodePtr relationshipGroupingNode = TestNodesHelper::CreateRelationshipGroupingNode(*m_connection, *GetECClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP(), "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);
    relationshipGroupingNode->SetParentNode(*rootInstanceNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *relationshipGroupingNode);
    NavNodeExtendedData extendedData(*relationshipGroupingNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, true, true, 0, "RulesEngineTest");
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Relationship);
    extendedData.SetParentECClassId(GetECClass("RulesEngineTest", "Widget")->GetId());
    extendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Forward);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *relationshipGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_GroupByRelationshipAndLabel_RelationshipNodeChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByRelationshipAndLabel_LabelNodeChildrenQuery)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"));
    TestNavNodePtr relationshipGroupingNode = TestNodesHelper::CreateRelationshipGroupingNode(*m_connection, *GetECClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP(), "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);
    relationshipGroupingNode->SetParentNode(*rootInstanceNode);
    NavNodeExtendedData relationshipNodeExtendedData(*relationshipGroupingNode);
    JsonNavNodePtr labelGroupingNode = TestNodesHelper::CreateLabelGroupingNode(*m_connection, "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *relationshipGroupingNode);
    labelGroupingNode->SetParentNode(*relationshipGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *labelGroupingNode);
    NavNodeExtendedData labelNodeExtendedData(*labelGroupingNode);
    
    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, true, true, 0, "RulesEngineTest");
    relationshipNodeExtendedData.SetSpecificationHash(spec.GetHash());
    relationshipNodeExtendedData.SetGroupingType((int)GroupingType::Relationship);
    relationshipNodeExtendedData.SetParentECClassId(GetECClass("RulesEngineTest", "Widget")->GetId());
    relationshipNodeExtendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
    labelNodeExtendedData.SetSpecificationHash(spec.GetHash());
    labelNodeExtendedData.SetGroupingType((int)GroupingType::DisplayLabel);
    labelNodeExtendedData.SetGroupedInstanceKeys({ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1))});

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *labelGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_GroupByRelationshipAndLabel_LabelNodeChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();

    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_GroupByRelationshipAndClassAndLabel_LabelNodeChildrenQuery)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"));
    TestNavNodePtr relationshipGroupingNode = TestNodesHelper::CreateRelationshipGroupingNode(*m_connection, *GetECClass("RulesEngineTest", "WidgetHasGadget")->GetRelationshipClassCP(), "Relationship Grouping Node");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);
    relationshipGroupingNode->SetParentNode(*rootInstanceNode);
    NavNodeExtendedData relationshipNodeExtendedData(*relationshipGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *relationshipGroupingNode);
    TestNavNodePtr classGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"), "Class Grouping Node");
    classGroupingNode->SetParentNode(*relationshipGroupingNode);
    NavNodeExtendedData classNodeExtendedData(*classGroupingNode);
    JsonNavNodePtr labelGroupingNode = TestNodesHelper::CreateLabelGroupingNode(*m_connection, "Label Grouping Node");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *classGroupingNode);
    labelGroupingNode->SetParentNode(*classGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *labelGroupingNode);
    NavNodeExtendedData labelNodeExtendedData(*labelGroupingNode);
    
    AllRelatedInstanceNodesSpecification spec(1, false, false, false, true, true, true, 0, "RulesEngineTest");
    relationshipNodeExtendedData.SetSpecificationHash(spec.GetHash());
    relationshipNodeExtendedData.SetGroupingType((int)GroupingType::Relationship);
    relationshipNodeExtendedData.SetParentECClassId(GetECClass("RulesEngineTest", "Widget")->GetId());
    relationshipNodeExtendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
    classNodeExtendedData.SetSpecificationHash(spec.GetHash());
    classNodeExtendedData.SetGroupingType((int)GroupingType::Class);
    labelNodeExtendedData.SetSpecificationHash(spec.GetHash());
    labelNodeExtendedData.SetGroupedInstanceKeys({ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1))});

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *labelGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_GroupByRelationshipAndClassAndLabel_LabelNodeChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_InstanceLabelOverride_AppliedByPriority)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, "RulesEngineTest");
    spec.SetRequiredRelationDirection(RequiredRelationDirection_Forward);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Gadget", "MyID"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, true, "RulesEngineTest:Gadget", "Description"));
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_InstanceLabelOverride_AppliedByPriority", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, AllRelatedInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllRelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, 0, "RulesEngineTest");
    spec.SetRequiredRelationDirection(RequiredRelationDirection_Both);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Sprocket", "MyID"));
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllRelatedInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }