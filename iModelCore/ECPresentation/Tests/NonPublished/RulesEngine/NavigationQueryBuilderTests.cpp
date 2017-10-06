/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NavigationQueryBuilderTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                06/2015
//---------------------------------------------------------------------------------------
void NavigationQueryBuilderTests::SetUp()
    {
    Localization::Init();

    m_ruleset = PresentationRuleSet::CreateInstance("NavigationQueryBuilderTests", 1, 0, false, "", "", "", false);
    m_schemaHelper = new ECSchemaHelper(ExpectedQueries::GetInstance(BeTest::GetHost()).GetDb(), m_relatedPathsCache, nullptr);
    m_builder = new NavigationQueryBuilder(NavigationQueryBuilderParameters(*m_schemaHelper, 
        *m_ruleset, m_settings, nullptr, m_expressionsCache, m_nodesCache));

    m_rootNodeRule = new RootNodeRule();
    m_childNodeRule = new ChildNodeRule();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                06/2015
//---------------------------------------------------------------------------------------
void NavigationQueryBuilderTests::TearDown()
    {
    delete m_builder;
    delete m_rootNodeRule;
    delete m_childNodeRule;
    Localization::Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP NavigationQueryBuilderTests::GetECClass(Utf8CP schemaName, Utf8CP className)
    {
    return ExpectedQueries::GetInstance(BeTest::GetHost()).GetECClass(schemaName, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECClassCP> NavigationQueryBuilderTests::GetECClasses(Utf8CP schemaName)
    {
    return ExpectedQueries::GetInstance(BeTest::GetHost()).GetECClasses(schemaName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String NavigationQueryBuilderTests::GetInstanceIdWithFormat(Utf8CP format, NavNodeCR node)
    {
    Utf8String instanceId(node.GetInstance()->GetInstanceId().c_str());
    return Utf8PrintfString(format, instanceId.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, NotifiesAboutUsedClassesInFromClause)
    {
    TestUsedClassesListener listener;
    GetBuilder().GetParameters().SetUsedClassesListener(&listener);

    RootNodeRule rule("", 1000, false, TargetTree_MainTree, false);
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", false);

    GetBuilder().GetQueries(rule, spec);
    ASSERT_EQ(1, listener.GetUsedClasses().size());
    auto iter = listener.GetUsedClasses().find(GetECClass("RulesEngineTest", "Widget"));
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_FALSE(iter->second); // not polymorphic
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, NotifiesAboutUsedPolymorphicClassesInFromClause)
    {
    TestUsedClassesListener listener;
    GetBuilder().GetParameters().SetUsedClassesListener(&listener);

    RootNodeRule rule("", 1000, false, TargetTree_MainTree, false);
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, 
        "", "RulesEngineTest:Widget", true);

    GetBuilder().GetQueries(rule, spec);
    ASSERT_EQ(1, listener.GetUsedClasses().size());
    auto iter = listener.GetUsedClasses().find(GetECClass("RulesEngineTest", "Widget"));
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, NotifiesAboutUsedClassesInJoins)
    {
    TestUsedClassesListener listener;
    GetBuilder().GetParameters().SetUsedClassesListener(&listener);
    
    TestNavNodePtr parentNode = TestNodesHelper::CreateInstanceNode(*GetECClass("RulesEngineTest", "Gadget"));
    parentNode->SetParentNodeId(TestNodesHelper::CreateNodeId());
    m_nodesCache.Cache(*parentNode, false);

    ChildNodeRule rule("", 1000, false, TargetTree_MainTree);
    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, false,
        0, "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:GadgetHasSprockets", "RulesEngineTest:Sprocket");

    GetBuilder().GetQueries(rule, spec, *parentNode);
    ASSERT_EQ(3, listener.GetUsedClasses().size());

    auto iter = listener.GetUsedClasses().find(GetECClass("RulesEngineTest", "Gadget"));
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic

    iter = listener.GetUsedClasses().find(GetECClass("RulesEngineTest", "GadgetHasSprockets"));
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic

    iter = listener.GetUsedClasses().find(GetECClass("RulesEngineTest", "Sprocket"));
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, NotifiesAboutUsedRelatedClassesInInstanceFilter)
    {
    TestUsedClassesListener listener;
    GetBuilder().GetParameters().SetUsedClassesListener(&listener);
    
    RootNodeRule rule("", 1000, false, TargetTree_MainTree, false);
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, 
        "this.GetRelatedValue(\"RulesEngineTest:WidgetHasGadgets\", \"Forward\", \"RulesEngineTest:Gadget\", \"MyID\") = \"test\"", "RulesEngineTest:Widget", false);

    GetBuilder().GetQueries(rule, spec);
    ASSERT_EQ(3, listener.GetUsedClasses().size());

    auto iter = listener.GetUsedClasses().find(GetECClass("RulesEngineTest", "Widget"));
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_FALSE(iter->second); // not polymorphic

    iter = listener.GetUsedClasses().find(GetECClass("RulesEngineTest", "WidgetHasGadgets"));
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic

    iter = listener.GetUsedClasses().find(GetECClass("RulesEngineTest", "Gadget"));
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, NotifiesAboutUsedRelatedInstanceClasses)
    {
    TestUsedClassesListener listener;
    GetBuilder().GetParameters().SetUsedClassesListener(&listener);
    
    RootNodeRule rule("", 1000, false, TargetTree_MainTree, false);
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Gadget", "g"));

    GetBuilder().GetQueries(rule, spec);
    ASSERT_EQ(3, listener.GetUsedClasses().size());

    auto iter = listener.GetUsedClasses().find(GetECClass("RulesEngineTest", "Widget"));
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_FALSE(iter->second); // not polymorphic

    iter = listener.GetUsedClasses().find(GetECClass("RulesEngineTest", "WidgetHasGadgets"));
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic

    iter = listener.GetUsedClasses().find(GetECClass("RulesEngineTest", "Gadget"));
    ASSERT_TRUE(listener.GetUsedClasses().end() != iter);
    EXPECT_TRUE(iter->second); // polymorphic
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, JoinsWithAdditionalRelatedInstances)
    {
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("JoinsWithAdditionalRelatedInstances", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, FiltersByRelatedInstanceProperties)
    {
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "widget.IntProperty > 5 AND widget.MyID <> this.MyID", "RulesEngineTest:Gadget", false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("FiltersByRelatedInstanceProperties", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }
