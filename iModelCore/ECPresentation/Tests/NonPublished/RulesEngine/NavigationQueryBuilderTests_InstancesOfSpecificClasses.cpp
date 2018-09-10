/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NavigationQueryBuilderTests_InstancesOfSpecificClasses.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"
#include "ECDbTestProject.h"

// note: some of the common specification properties of this specification are tested
//       QueryBuilderTests_AllInstanceNodes.cpp as they're common between the two.

static JsonNavNodesFactory s_nodesFactory;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_ClassNames_NotPolymorphic)
    {
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic1:Class1A,Class1B", false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_ClassNames_NotPolymorphic", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_ClassNames_Polymorphic)
    {
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic1:Class1A", true);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_ClassNames_Polymorphic", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_AllowsStandardSchemasIfExplicitylySpecified)
    {
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "ECDbMeta:ECSchemaDef", false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_TRUE(queries.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_GroupByClass)
    {
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, true, false, false, "", "Basic1:Class1A", false);
    
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_GroupByClass", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_ShowEmptyGroups)
    {
    // WIP - need metadata queries to support selecting classes without instances
    FAIL();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_GroupByClass_ChildrenQuery)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    TestNavNodePtr parentNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClass, "MyLabel");
    m_nodesCache.Cache(*parentNode, false);
    
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, true, false, false, "", "Basic1:Class1A", false);

    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Class);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_GroupByClass_ChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_GroupByLabel)
    {
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, true, false, "", "Basic1:Class1A,Class1B", false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_GroupByLabel", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_GroupByLabel_ChildrenQuery)
    {
    JsonNavNodePtr parentNode = TestNodesHelper::CreateLabelGroupingNode(*m_connection, "MyLabel");
    m_nodesCache.Cache(*parentNode, false);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, true, false, "", "Basic1:Class1A,Class1B", false);
    
    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::DisplayLabel);
    extendedData.SetGroupedInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_GroupByLabel_ChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_GroupByClassAndLabel_ClassNodeChildrenQuery)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    TestNavNodePtr parentNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClass, "MyLabel");
    m_nodesCache.Cache(*parentNode, false);
    
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, true, true, false, "", "Basic1:Class1A,Class1B", false);

    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Class);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_GroupByClassAndLabel_ClassNodeChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_GroupByClassAndLabel_LabelNodeChildrenQuery)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");
    TestNavNodePtr classGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClass, "Class Grouping Node");
    JsonNavNodePtr labelGroupingNode = TestNodesHelper::CreateLabelGroupingNode(*m_connection, "Label Grouping Node");
    m_nodesCache.Cache(*classGroupingNode, false);
    labelGroupingNode->SetParentNode(*classGroupingNode);
    m_nodesCache.Cache(*labelGroupingNode, false);
    
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, true, true, false, "", "Basic1:Class1A,Class1B", false);

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
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_GroupByClassAndLabel_LabelNodeChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceFilter)
    {    
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "this.Name = 2", "Basic1:Class1A,Class1B", false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_InstanceFilter", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceFilter_LikeOperator)
    {    
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "this.Name ~ \"Test\"", "Basic2:Class2", false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_InstanceFilter_LikeOperator", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceFilter_ReferencingParentInstanceWithoutParentNodeDoesntApplyTheFilter)
    {
    IGNORE_BE_ASSERT();

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "this.Name = parent.Name", "Basic1:Class1A", false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_InstanceFilter_ReferencingParentInstanceWithoutParentNodeDoesntApplyTheFilter", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceFilter_ReferencingParentInstance_WithParentInstanceNode)
    {    
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("Basic2", "Class2"));
    m_nodesCache.Cache(*parentNode, false);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "this.Name = parent.Name", "Basic1:Class1A", false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_InstanceFilter_ReferencingParentInstance_WithParentInstanceNode", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceFilter_ReferencingGrandParentInstance)
    {    
    TestNavNodePtr grandparentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("Basic2", "Class2"), ECInstanceId((uint64_t)123));
    m_nodesCache.Cache(*grandparentNode, false);

    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("Basic2", "Class2"), ECInstanceId((uint64_t)456));
    parentNode->SetParentNodeId(grandparentNode->GetNodeId());
    m_nodesCache.Cache(*parentNode, false);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "this.Name = parent.parent.Name", "Basic1:Class1A", false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_InstanceFilter_ReferencingGrandParentInstance", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceFilter_ReferencingGrandParentAndParentInstance)
    {    
    TestNavNodePtr grandparentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("Basic2", "Class2"), ECInstanceId((uint64_t)123));
    m_nodesCache.Cache(*grandparentNode, false);

    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("Basic2", "Class2"), ECInstanceId((uint64_t)456));
    parentNode->SetParentNodeId(grandparentNode->GetNodeId());
    m_nodesCache.Cache(*parentNode, false);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, 
        "this.Name = parent.parent.Name OR this.Name = parent.Name", "Basic1:Class1A", false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_InstanceFilter_ReferencingGrandParentAndParentInstance", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels)
    {
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget,Gadget", false);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, InstancesOfSpecificClasses_InstanceLabelOverride_AppliedByPriority)
    {
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, true, "RulesEngineTest:Widget", "Description"));
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("InstancesOfSpecificClasses_InstanceLabelOverride_AppliedByPriority", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }