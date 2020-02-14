/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"

// note: some of the common specification properties of this specification are tested
//       QueryBuilderTests_AllRelatedInstanceNodes.cpp as they're common between the two.

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_SkipOneRelatedLevel_WidgetToSprocket)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

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
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);
    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, false, 2, "", RequiredRelationDirection_Forward, "RulesEngineTest", "", "");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_SkipRelatedLevel_GroupsByECInstanceKey)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

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
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);
    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both, "Basic1", "", "");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelationshipClassNames_OnlyIncluded)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);
    
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
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);
    
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
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

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
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

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
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Sprocket"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

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
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Sprocket"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both, "", "", "RulesEngineTest:Gadget;PE:RulesEngineTest:Gadget");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_TRUE(queries.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClasses_GroupByClassWithExcluded)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Sprocket"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

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
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Sprocket"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

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
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Sprocket"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

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
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Sprocket"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

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
* @bsitest                                      Grigas.Petraitis                01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_InstanceFilter_AllowsAccessingParentNode)
    {    
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Sprocket"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, 
        "this.Description = parent.Description", RequiredRelationDirection_Backward, "", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Gadget");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery(BeTest::GetNameOfCurrentTest(), spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_RelatedClasses_MultipleExcludedClasses)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "ClassD"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

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
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "ClassD"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);
    
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

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_InstanceLabelOverride_AppliedByPriority)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Backward, "", "RulesEngineTest:WidgetHasGadget", "");
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, true, "RulesEngineTest:Widget", "Description"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_InstanceLabelOverride_AppliedByPriority", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, RelatedInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels)
    {
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Gadget"));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);
    
    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Both, "", "RulesEngineTest:WidgetHasGadget,GadgetHasSprockets", "");
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, "RulesEngineTest:Widget", "MyID"));
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelatedInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2019
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

    TestNavNodePtr parentNode = TestNodesHelper::CreateInstancesNode(*m_connection, 
        {
        ECInstanceKey(classA->GetId(), ECInstanceId((uint64_t)1)),
        ECInstanceKey(classB->GetId(), ECInstanceId((uint64_t)2)),
        });
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, true, 0, "", RequiredRelationDirection_Forward,
        "", Utf8PrintfString("%s:%s,%s", GetECSchema()->GetName().c_str(), rel1->GetName().c_str(), rel2->GetName().c_str()), classC->GetFullName());
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery(BeTest::GetNameOfCurrentTest(), spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                02/2020
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

    TestNavNodePtr parentNode = TestNodesHelper::CreateInstancesNode(*m_connection,
        {
        ECInstanceKey(classA->GetId(), ECInstanceId((uint64_t)1)),
        });
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, ChildrenHint::Unknown, false, false, false, false, "", { new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relAToB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, classB->GetFullName()),
        new RepeatableRelationshipStepSpecification(relBIsInC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, classC->GetFullName())
        }) 
    });
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery(BeTest::GetNameOfCurrentTest(), spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedInstanceNodes_CreatesInClauseForOneToManyToOnePathOnNavigationProperty, R"*(
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
TEST_F(NavigationQueryBuilderTests, RelatedInstanceNodes_CreatesInClauseForOneToManyToOnePathOnNavigationProperty)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAToB = GetECClass("AToB")->GetRelationshipClassCP();
    ECRelationshipClassCP relBIsInC = GetECClass("BIsInC")->GetRelationshipClassCP();

    TestNavNodePtr parentNode = TestNodesHelper::CreateInstancesNode(*m_connection,
        {
        ECInstanceKey(classA->GetId(), ECInstanceId((uint64_t)1)),
        });
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    RelatedInstanceNodesSpecification spec(1, ChildrenHint::Unknown, false, false, false, false, "", { new RepeatableRelationshipPathSpecification({
        new RepeatableRelationshipStepSpecification(relAToB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, classB->GetFullName()),
        new RepeatableRelationshipStepSpecification(relBIsInC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward, classC->GetFullName())
        })
        });
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery(BeTest::GetNameOfCurrentTest(), spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }
