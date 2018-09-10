/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NavigationQueryBuilderTests_GroupingRule.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"
#include "TestHelpers.h"

static JsonNavNodesFactory s_nodesFactory;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_ClassFilterIsPolymorphic)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "Basic4", "ClassA", "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup());
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic4:ClassB", false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_ClassFilterIsPolymorphic", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_IgnoresGroupingRulesWithInvalidSchemas)
    {
    IGNORE_BE_ASSERT();

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "InvalidSchemaName", "InvalidClassName", "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup());
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic4:ClassA", false);
    
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_IgnoresGroupingRulesWithInvalidSchemas", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_IgnoresGroupingRulesWithInvalidClasses)
    {
    IGNORE_BE_ASSERT();

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "Basic4", "InvalidClassName", "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup());
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic4:ClassA", false);
    
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_IgnoresGroupingRulesWithInvalidClasses", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_SameLabelInstanceGroup_OnlyGroupedInstanceNodes)
    {
    GroupingRuleP groupingRule1 = new GroupingRule("", 1, false, "Basic1", "Class1A", "", "", "");
    groupingRule1->AddGroup(*new SameLabelInstanceGroup());
    GroupingRuleP groupingRule2 = new GroupingRule("", 1, false, "Basic1", "Class1B", "", "", "");
    groupingRule2->AddGroup(*new SameLabelInstanceGroup());
    m_ruleset->AddPresentationRule(*groupingRule1);
    m_ruleset->AddPresentationRule(*groupingRule2);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic1:Class1A,Class1B", true);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_SameLabelInstanceGroup_OnlyGroupedInstanceNodes", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_SameLabelInstanceGroup_MultipleGroupedAndUngroupedInstanceNodes)
    {
    GroupingRuleP groupingRule1 = new GroupingRule("", 1, false, "Basic1", "Class1A", "", "", "");
    groupingRule1->AddGroup(*new SameLabelInstanceGroup());
    GroupingRuleP groupingRule2 = new GroupingRule("", 1, false, "Basic1", "Class1B", "", "", "");
    groupingRule2->AddGroup(*new SameLabelInstanceGroup());
    m_ruleset->AddPresentationRule(*groupingRule1);
    m_ruleset->AddPresentationRule(*groupingRule2);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic1:Class1A,Class1B;Basic4:ClassA", true);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_SameLabelInstanceGroup_MultipleGroupedAndUngroupedInstanceNodes", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_SameLabelInstanceGroup_SetsValidQueryResultParameters)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "Basic4", "ClassA", "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup());
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic4:ClassA", false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    EXPECT_EQ(NavigationQueryResultType::ECInstanceNodes, query->GetResultParameters().GetResultType());
    EXPECT_EQ((int)GroupingType::SameLabelInstance, query->GetResultParameters().GetNavNodeExtendedData().GetGroupingType());
    EXPECT_TRUE(query->GetResultParameters().HasInstanceGroups());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_ClassGroup_GroupsByClass)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "Basic2", "Class2", "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", true, "Basic2", "Class2"));
    m_ruleset->AddPresentationRule(*groupingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic2");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_ClassGroup_GroupsByClass", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_ClassGroup_GroupsByBaseClass)
    {
    GroupingRuleP groupingRule1 = new GroupingRule("", 1, false, "Basic4", "ClassB", "", "", "");
    groupingRule1->AddGroup(*new ClassGroup("", true, "Basic4", "ClassA"));
    GroupingRuleP groupingRule2 = new GroupingRule("", 1, false, "Basic4", "ClassC", "", "", "");
    groupingRule2->AddGroup(*new ClassGroup("", true, "Basic4", "ClassA"));
    m_ruleset->AddPresentationRule(*groupingRule1);
    m_ruleset->AddPresentationRule(*groupingRule2);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic4:ClassB,ClassC", false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_ClassGroup_GroupsByBaseClass", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_ClassGroup_BaseClassGroupingAndGroupByClass)
    {
    GroupingRuleP groupingRule1 = new GroupingRule("", 1, false, "Basic4", "ClassB", "", "", "");
    groupingRule1->AddGroup(*new ClassGroup("", true, "Basic4", "ClassA"));
    GroupingRuleP groupingRule2 = new GroupingRule("", 1, false, "Basic4", "ClassC", "", "", "");
    groupingRule2->AddGroup(*new ClassGroup("", true, "Basic4", "ClassA"));
    m_ruleset->AddPresentationRule(*groupingRule1);
    m_ruleset->AddPresentationRule(*groupingRule2);

    AllInstanceNodesSpecification spec(1, false, false, false, true, false, "Basic4");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(2, queries.size());

    NavigationQueryPtr baseClassQuery = queries[1];
    ASSERT_TRUE(baseClassQuery.IsValid());
    
    NavigationQueryCPtr expectedBaseClassQuery = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_ClassGroup_BaseClassGroupingAndGroupByClass_BaseClass", spec);
    EXPECT_TRUE(expectedBaseClassQuery->IsEqual(*baseClassQuery)) 
        << "Expected: " << expectedBaseClassQuery->ToString() << "\r\n"
        << "Actual:   " << baseClassQuery->ToString();
    
    NavigationQueryPtr classQuery = queries[0];
    ASSERT_TRUE(classQuery.IsValid());

    NavigationQueryCPtr expectedClassQuery = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_ClassGroup_BaseClassGroupingAndGroupByClass_Class", spec);
    EXPECT_TRUE(expectedClassQuery->IsEqual(*classQuery)) 
        << "Expected: " << expectedClassQuery->ToString() << "\r\n"
        << "Actual:   " << classQuery->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_ClassGroup_ECInstanceNodesChildrenQuery)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "Basic2", "Class2", "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", true, "Basic2", "Class2"));
    m_ruleset->AddPresentationRule(*groupingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic2");
    spec.SetDoNotSort(true);

    TestNavNodePtr parentNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *GetECClass("Basic2", "Class2"), "MyLabel");
    m_nodesCache.Cache(*parentNode, false);
    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::BaseClass);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_ClassGroup_ECInstanceNodesChildrenQuery", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_GroupsByProperty)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "Basic2", "Class2", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Name"));
    m_ruleset->AddPresentationRule(*groupingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic2");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_GroupsByProperty", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_GroupsMultipleClassesByProperty)
    {
    GroupingRuleP groupingRule1 = new GroupingRule("", 1, false, "Basic1", "Class1A", "", "", "");
    groupingRule1->AddGroup(*new PropertyGroup("", "", true, "Name"));
    GroupingRuleP groupingRule2 = new GroupingRule("", 1, false, "Basic1", "Class1B", "", "", "");
    groupingRule2->AddGroup(*new PropertyGroup("", "", true, "Name"));
    m_ruleset->AddPresentationRule(*groupingRule1);
    m_ruleset->AddPresentationRule(*groupingRule2);
    
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic1:Class1A,Class1B", true);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_GroupsMultipleClassesByProperty", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_GroupsByRange)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "Basic1", "Class1A", "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "Name");
    groupingSpec->AddRange(*new PropertyRangeGroupSpecification("", "", "0", "5"));
    groupingSpec->AddRange(*new PropertyRangeGroupSpecification("", "", "6", "10"));
    groupingSpec->AddRange(*new PropertyRangeGroupSpecification("", "", "11", "20"));
    groupingRule->AddGroup(*groupingSpec);

    m_ruleset->AddPresentationRule(*groupingRule);
    
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic1:Class1A,Class1B", true);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(2, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_GroupsByRange", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_OverridesImageId)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "Basic2", "Class2", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "TestImageId", true, "Name"));
    m_ruleset->AddPresentationRule(*groupingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic2");

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_OverridesImageId", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_ValueFiltering)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "Basic1", "Class1A", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Name"));
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic1:Class1A", false);

    TestNavNodePtr propertyGroupingNode = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *ecClass, *ecClass->GetPropertyP("Name"), "MyCustomLabel", rapidjson::Value(9), false);
    m_nodesCache.Cache(*propertyGroupingNode, false);
    NavNodeExtendedData extendedData(*propertyGroupingNode);
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Property);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *propertyGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_ValueFiltering", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_RangeFiltering)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "Basic1", "Class1A", "", "", "");
    m_ruleset->AddPresentationRule(*groupingRule);

    PropertyGroup* groupingSpecification = new PropertyGroup("", "", true, "Name");
    groupingSpecification->AddRange(*new PropertyRangeGroupSpecification("", "", "1", "5"));
    groupingRule->AddGroup(*groupingSpecification);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic1:Class1A", false);

    TestNavNodePtr propertyGroupingNode = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *ecClass, *ecClass->GetPropertyP("Name"), "MyCustomLabel", rapidjson::Value(0), true);
    m_nodesCache.Cache(*propertyGroupingNode, false);
    NavNodeExtendedData extendedData(*propertyGroupingNode);
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Property);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *propertyGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_RangeFiltering", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_OtherRangeFiltering)
    {
    ECClassCP ecClass = GetECClass("Basic1", "Class1A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "Basic1", "Class1A", "", "", "");
    m_ruleset->AddPresentationRule(*groupingRule);

    PropertyGroup* groupingSpecification = new PropertyGroup("", "", true, "Name");
    groupingSpecification->AddRange(*new PropertyRangeGroupSpecification("", "", "1", "5"));
    groupingSpecification->AddRange(*new PropertyRangeGroupSpecification("", "", "7", "9"));
    groupingSpecification->AddRange(*new PropertyRangeGroupSpecification("", "", "10", "15"));
    groupingRule->AddGroup(*groupingSpecification);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "Basic1:Class1A", false);

    TestNavNodePtr propertyGroupingNode = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *ecClass, *ecClass->GetPropertyP("Name"), "MyCustomLabel", rapidjson::Value(-1), true);
    m_nodesCache.Cache(*propertyGroupingNode, false);
    NavNodeExtendedData extendedData(*propertyGroupingNode);
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Property);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *propertyGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_OtherRangeFiltering", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_CreatesGroupingQueryWhenRelationshipWithNavigationPropertyIsUsed)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "SchemaComplex3", "ChildClassWithNavigationProperty", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Group"));
    m_ruleset->AddPresentationRule(*groupingRule);
    
    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false,
        "", "SchemaComplex3:ChildClassWithNavigationProperty", false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_CreatesGroupingQueryWhenRelationshipWithNavigationPropertyIsUsed", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_GroupsSubclassNodesWhenPolymorphicallyReturningParentNodes)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassF", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty"));
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(2, queries.size());

    NavigationQueryPtr propertyGroupingNodesQuery = queries[0];
    ASSERT_TRUE(propertyGroupingNodesQuery.IsValid());
    
    NavigationQueryPtr instanceNodesQuery = queries[1];
    ASSERT_TRUE(instanceNodesQuery.IsValid());

    NavigationQueryCPtr expectedPropertyGroupingNodesQuery = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_GroupsSubclassNodesWhenPolymorphicallyReturningParentNodes_1", spec);
    EXPECT_TRUE(expectedPropertyGroupingNodesQuery->IsEqual(*propertyGroupingNodesQuery)) 
        << "Expected: " << expectedPropertyGroupingNodesQuery->ToString() << "\r\n"
        << "Actual:   " << propertyGroupingNodesQuery->ToString();
        
    NavigationQueryCPtr expectedInstanceNodesQuery = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_GroupsSubclassNodesWhenPolymorphicallyReturningParentNodes_2", spec);
    EXPECT_TRUE(expectedInstanceNodesQuery->IsEqual(*instanceNodesQuery)) 
        << "Expected: " << expectedInstanceNodesQuery->ToString() << "\r\n"
        << "Actual:   " << instanceNodesQuery->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_GroupsSubclassNodesWhenPolymorphicallyReturningParentNodesWithRelatedInstances)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassF", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty"));
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassE", true);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:ClassDHasClassE", "RulesEngineTest:ClassD", "d"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(2, queries.size());

    NavigationQueryPtr propertyGroupingNodesQuery = queries[0];
    ASSERT_TRUE(propertyGroupingNodesQuery.IsValid());
    
    NavigationQueryPtr instanceNodesQuery = queries[1];
    ASSERT_TRUE(instanceNodesQuery.IsValid());

    NavigationQueryCPtr expectedPropertyGroupingNodesQuery = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_GroupsSubclassNodesWhenPolymorphicallyReturningParentNodesWithRelatedInstances_1", spec);
    EXPECT_TRUE(expectedPropertyGroupingNodesQuery->IsEqual(*propertyGroupingNodesQuery)) 
        << "Expected: " << expectedPropertyGroupingNodesQuery->ToString() << "\r\n"
        << "Actual:   " << propertyGroupingNodesQuery->ToString();
        
    NavigationQueryCPtr expectedInstanceNodesQuery = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_GroupsSubclassNodesWhenPolymorphicallyReturningParentNodesWithRelatedInstances_2", spec);
    EXPECT_TRUE(expectedInstanceNodesQuery->IsEqual(*instanceNodesQuery)) 
        << "Expected: " << expectedInstanceNodesQuery->ToString() << "\r\n"
        << "Actual:   " << instanceNodesQuery->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_GroupsRelatedSubclassNodesWhenPolymorphicallyReturningParentNodes)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassF", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty"));
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassD", true);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, "RulesEngineTest:ClassDHasClassE", "RulesEngineTest:ClassE", "e"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(2, queries.size());

    NavigationQueryPtr propertyGroupingNodesQuery = queries[0];
    ASSERT_TRUE(propertyGroupingNodesQuery.IsValid());
    
    NavigationQueryPtr instanceNodesQuery = queries[1];
    ASSERT_TRUE(instanceNodesQuery.IsValid());

    NavigationQueryCPtr expectedPropertyGroupingNodesQuery = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_GroupsRelatedSubclassNodesWhenPolymorphicallyReturningParentNodes_1", spec);
    EXPECT_TRUE(expectedPropertyGroupingNodesQuery->IsEqual(*propertyGroupingNodesQuery)) 
        << "Expected: " << expectedPropertyGroupingNodesQuery->ToString() << "\r\n"
        << "Actual:   " << propertyGroupingNodesQuery->ToString();
        
    NavigationQueryCPtr expectedInstanceNodesQuery = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_GroupsRelatedSubclassNodesWhenPolymorphicallyReturningParentNodes_2", spec);
    EXPECT_TRUE(expectedInstanceNodesQuery->IsEqual(*instanceNodesQuery)) 
        << "Expected: " << expectedInstanceNodesQuery->ToString() << "\r\n"
        << "Actual:   " << instanceNodesQuery->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_GroupsByBaseClassPropertyWhenReturningDerivedClassInstances)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassE", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty"));
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "RulesEngineTest:ClassF,ClassG", false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_GroupsByBaseClassPropertyWhenReturningDerivedClassInstances", spec);
    EXPECT_TRUE(expected->IsEqual(*queries[0])) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << queries[0]->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PicksActiveGroupingSpecification)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "Basic2", "Class2", "", "", "TestSettingsId");
    groupingRule->AddGroup(*new ClassGroup("", true, "Basic2", "Class2"));
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Name"));
    m_ruleset->AddPresentationRule(*groupingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic2");

    bool accessedLocalState = false;
    TestLocalState localState;
    GetBuilder().GetParameters().SetLocalState(localState);
    localState.SetGetHandler([&accessedLocalState](Utf8CP ns, Utf8CP key) -> Json::Value
        {
        EXPECT_TRUE(0 == strcmp(RULES_ENGINE_ACTIVE_GROUPS_LOCAL_STATE_NAMESPACE, ns));
        EXPECT_TRUE(0 == strcmp("TestSettingsId", key));
        accessedLocalState = true;
        return 1;
        });

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    EXPECT_TRUE(accessedLocalState);

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_GroupsByProperty", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_GroupsByRelatedInstanceProperty)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "IntProperty"));
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "RulesEngineTest:Gadget", false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Backward, "RulesEngineTest:WidgetHasGadgets", "RulesEngineTest:Widget", "widget"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_GroupsByRelatedInstanceProperty", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_GroupsByRelationshipPropertyWhenUsedWithRelatedInstancesSpecification)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "Widget"));
    m_nodesCache.Cache(*rootInstanceNode, false);
    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "WidgetHasGadget", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Priority"));
    m_ruleset->AddPresentationRule(*groupingRule);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Forward,
        "RulesEngineTest", "RulesEngineTest:WidgetHasGadget", "RulesEngineTest:Gadget");
    
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *rootInstanceNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_GroupsByRelationshipPropertyWhenUsedWithRelatedInstancesSpecification", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }


/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryBuilderTests, Grouping_GroupsMultipleClassesByTheSameRelationshipProperty)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("RulesEngineTest", "ClassD"));
    m_nodesCache.Cache(*rootInstanceNode, false);

    // create the rules    
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "ClassDReferencesClassE", "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Priority", ""));
    m_ruleset->AddPresentationRule(*groupingRule);
    
    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "RulesEngineTest", "RulesEngineTest:ClassDReferencesClassE", "RulesEngineTest:ClassF,ClassG");

    // request for root nodes
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *rootInstanceNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_GroupsMultipleClassesByTheSameRelationshipProperty", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_GroupsByPropertyValue)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "IntProperty");
    groupingSpec->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_GroupsByPropertyValue", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_SortsByDisplayLabelWhenTryingToSortByPropertyValueAndGroupByLabel)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "IntProperty");
    groupingSpec->SetPropertyGroupingValue(PropertyGroupingValue::DisplayLabel);
    groupingSpec->SetSortingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_SortsByDisplayLabelWhenTryingToSortByPropertyValueAndGroupByLabel", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_GroupsAndSortsByPropertyValue)
    {
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "RulesEngineTest", "Widget", "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "IntProperty");
    groupingSpec->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    groupingSpec->SetSortingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", "RulesEngineTest:Widget", false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("Grouping_PropertyGroup_GroupsAndSortsByPropertyValue", spec);
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavigationQueryBuilder_MultiLevelGroupingTests::SetUp()
    {
    T_Super::SetUp();

    GroupingRuleP baseClassGroupingRule = new GroupingRule("", 1, false, "Basic4", "ClassA", "", "", "");
    baseClassGroupingRule->AddGroup(*new ClassGroup("", true, "Basic4", "ClassA"));
    m_ruleset->AddPresentationRule(*baseClassGroupingRule);

    GroupingRuleP propertyGroupingRule1 = new GroupingRule("", 2, false, "Basic4", "ClassA", "", "", "");
    propertyGroupingRule1->AddGroup(*new PropertyGroup("", "", true, "SomeProperty"));
    m_ruleset->AddPresentationRule(*propertyGroupingRule1);
    
    GroupingRuleP propertyGroupingRule2 = new GroupingRule("", 1, false, "Basic4", "ClassA", "", "", "");
    propertyGroupingRule2->AddGroup(*new PropertyGroup("", "", true, "Description"));
    m_ruleset->AddPresentationRule(*propertyGroupingRule2);
    
    GroupingRuleP propertyGroupingRule3 = new GroupingRule("", 1, false, "Basic4", "ClassC", "", "", "");
    propertyGroupingRule3->AddGroup(*new PropertyGroup("", "", true, "SomeProperty"));
    m_ruleset->AddPresentationRule(*propertyGroupingRule3);

    GroupingRuleP sameLabelInstanceGroupingRule = new GroupingRule("", 1, false, "Basic4", "ClassA", "", "", "");
    sameLabelInstanceGroupingRule->AddGroup(*new SameLabelInstanceGroup(""));
    m_ruleset->AddPresentationRule(*sameLabelInstanceGroupingRule);

    m_specification1 = new AllInstanceNodesSpecification(1, false, false, false, true, true, "Basic4");
    m_specification2 = new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, true, true, true, "", "Basic4:ClassA", true);
    m_specification2->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, "Basic4:ClassBHasClassC", "Basic4:ClassC", "c"));
    m_specification3 = new AllRelatedInstanceNodesSpecification(1, false, false, false, true, true, true, 0, "Basic4");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavigationQueryBuilder_MultiLevelGroupingTests::TearDown()
    {
    if (nullptr != m_specification1)
        delete m_specification1;
    
    if (nullptr != m_specification2)
        delete m_specification2;
    
    if (nullptr != m_specification3)
        delete m_specification3;

    T_Super::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* Uses AllInstanceNodes specification
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilder_MultiLevelGroupingTests, RootNodesQueryReturnsBaseClassGroupingNodes_1)
    {
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, *m_specification1);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RootNodesQueryReturnsBaseClassGroupingNodes_1", *m_specification1);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* Uses InstanceNodesOfSpecificClasses specification
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilder_MultiLevelGroupingTests, RootNodesQueryReturnsBaseClassGroupingNodes_2)
    {
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, *m_specification2);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RootNodesQueryReturnsBaseClassGroupingNodes_2", *m_specification2);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* Uses AllRelatedInstanceNodes specification
* WIP: need support for metadata queries to allow relationship grouping nodes
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilder_MultiLevelGroupingTests, RootNodesQueryReturnsRelationshipGroupingNodes)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("Basic4", "ClassB"));
    m_nodesCache.Cache(*rootInstanceNode, false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification3, *rootInstanceNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RootNodesQueryReturnsRelationshipGroupingNodes", *m_specification3);
    ASSERT_TRUE(expected.IsValid());
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* Uses AllRelatedInstanceNodes specification
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilder_MultiLevelGroupingTests, RelationshipGroupingNodeChildrenQueryReturnsBaseClassGroupingNodes)
    {
    TestNavNodePtr rootInstanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *GetECClass("Basic4", "ClassB"));
    TestNavNodePtr relationshipGroupingNode = TestNodesHelper::CreateRelationshipGroupingNode(*m_connection, *GetECClass("Basic4", "ClassBHasClassC")->GetRelationshipClassCP(), "test");
    m_nodesCache.Cache(*rootInstanceNode, false);
    relationshipGroupingNode->SetParentNode(*rootInstanceNode);
    m_nodesCache.Cache(*relationshipGroupingNode, false);
    NavNodeExtendedData extendedData(*relationshipGroupingNode);
    extendedData.SetSpecificationHash(m_specification3->GetHash());
    extendedData.SetGroupingType((int)GroupingType::Relationship);
    extendedData.SetParentECClassId(GetECClass("Basic4", "ClassB")->GetId());
    extendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
    
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification3, *relationshipGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("RelationshipGroupingNodeChildrenQueryReturnsBaseClassGroupingNodes", *m_specification3);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* Uses AllInstanceNodes specification
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilder_MultiLevelGroupingTests, BaseClassNodeChildrenQueryReturnsClassGroupingNodes_1)
    {
    ECClassCP ecClassA = GetECClass("Basic4", "ClassA");

    TestNavNodePtr baseClassGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClassA, "test");
    m_nodesCache.Cache(*baseClassGroupingNode, false);
    NavNodeExtendedData extendedData(*baseClassGroupingNode);
    extendedData.SetSpecificationHash(m_specification1->GetHash());
    extendedData.SetGroupingType((int)GroupingType::BaseClass);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification1, *baseClassGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("BaseClassNodeChildrenQueryReturnsClassGroupingNodes_1", *m_specification1);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* Uses InstanceNodesOfSpecificClasses specification
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilder_MultiLevelGroupingTests, BaseClassNodeChildrenQueryReturnsClassGroupingNodes_2)
    {
    ECClassCP ecClassA = GetECClass("Basic4", "ClassA");

    TestNavNodePtr baseClassGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClassA, "test");
    m_nodesCache.Cache(*baseClassGroupingNode, false);
    NavNodeExtendedData extendedData(*baseClassGroupingNode);
    extendedData.SetSpecificationHash(m_specification2->GetHash());
    extendedData.SetGroupingType((int)GroupingType::BaseClass);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification2, *baseClassGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("BaseClassNodeChildrenQueryReturnsClassGroupingNodes_2", *m_specification2);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* Uses InstanceNodesOfSpecificClasses specification
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilder_MultiLevelGroupingTests, ClassNodeChildrenQueryReturnsFirstPropertyGroupingNodes)
    {
    ECClassCP ecClassA = GetECClass("Basic4", "ClassA");
    ECClassCP ecClassB = GetECClass("Basic4", "ClassB");

    TestNavNodePtr baseClassGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClassA, "test");
    NavNodeExtendedData baseClassGroupingNodeExtendedData(*baseClassGroupingNode);
    baseClassGroupingNodeExtendedData.SetSpecificationHash(m_specification2->GetHash());
    baseClassGroupingNodeExtendedData.SetGroupingType((int)GroupingType::BaseClass);

    TestNavNodePtr classGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClassB, "test");
    m_nodesCache.Cache(*baseClassGroupingNode, false);
    classGroupingNode->SetParentNode(*baseClassGroupingNode);
    m_nodesCache.Cache(*classGroupingNode, false);
    NavNodeExtendedData classGroupingNodeExtendedData(*classGroupingNode);
    classGroupingNodeExtendedData.SetSpecificationHash(m_specification2->GetHash());
    classGroupingNodeExtendedData.SetGroupingType((int)GroupingType::Class);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification2, *classGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("ClassNodeChildrenQueryReturnsFirstPropertyGroupingNodes", *m_specification2);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* Uses InstanceNodesOfSpecificClasses specification
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilder_MultiLevelGroupingTests, FirstPropertyNodeChildrenQueryReturnsSecondPropertyGroupingNodes)
    {
    ECClassCP ecClassA = GetECClass("Basic4", "ClassA");
    ECClassCP ecClassB = GetECClass("Basic4", "ClassB");

    TestNavNodePtr baseClassGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClassA, "test");
    m_nodesCache.Cache(*baseClassGroupingNode, false);
    NavNodeExtendedData baseClassGroupingNodeExtendedData(*baseClassGroupingNode);
    baseClassGroupingNodeExtendedData.SetSpecificationHash(m_specification2->GetHash());
    baseClassGroupingNodeExtendedData.SetGroupingType((int)GroupingType::BaseClass);

    TestNavNodePtr classGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClassB, "test");
    classGroupingNode->SetParentNode(*baseClassGroupingNode);
    m_nodesCache.Cache(*classGroupingNode, false);
    NavNodeExtendedData classGroupingNodeExtendedData(*classGroupingNode);
    classGroupingNodeExtendedData.SetSpecificationHash(m_specification2->GetHash());
    classGroupingNodeExtendedData.SetGroupingType((int)GroupingType::Class);
    
    TestNavNodePtr propertyGroupingNode = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *ecClassB, *ecClassB->GetPropertyP("SomeProperty"), "test1", rapidjson::Value((uint64_t)10000000000), false);
    propertyGroupingNode->SetParentNode(*classGroupingNode);
    m_nodesCache.Cache(*propertyGroupingNode, false);
    NavNodeExtendedData propertyGroupingNodeExtendedData(*propertyGroupingNode);
    propertyGroupingNodeExtendedData.SetSpecificationHash(m_specification2->GetHash());
    propertyGroupingNodeExtendedData.SetGroupingType((int)GroupingType::Property);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification2, *propertyGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("FirstPropertyNodeChildrenQueryReturnsSecondPropertyGroupingNodes", *m_specification2);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* Uses InstanceNodesOfSpecificClasses specification
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilder_MultiLevelGroupingTests, SecondPropertyGroupingNodeChildrenQueryReturnsThirdPropertyGroupingNodes)
    {
    ECClassCP ecClassA = GetECClass("Basic4", "ClassA");
    ECClassCP ecClassB = GetECClass("Basic4", "ClassB");

    TestNavNodePtr baseClassGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClassA, "test");
    m_nodesCache.Cache(*baseClassGroupingNode, false);
    NavNodeExtendedData baseClassGroupingNodeExtendedData(*baseClassGroupingNode);
    baseClassGroupingNodeExtendedData.SetSpecificationHash(m_specification2->GetHash());
    baseClassGroupingNodeExtendedData.SetGroupingType((int)GroupingType::BaseClass);

    TestNavNodePtr classGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClassB, "test");
    classGroupingNode->SetParentNode(*baseClassGroupingNode);
    m_nodesCache.Cache(*classGroupingNode, false);
    NavNodeExtendedData classGroupingNodeExtendedData(*classGroupingNode);
    classGroupingNodeExtendedData.SetSpecificationHash(m_specification2->GetHash());
    classGroupingNodeExtendedData.SetGroupingType((int)GroupingType::Class);
    
    TestNavNodePtr propertyGroupingNode1 = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *ecClassB, *ecClassB->GetPropertyP("SomeProperty"), "test1", rapidjson::Value(2.5), false);
    propertyGroupingNode1->SetParentNode(*classGroupingNode);
    m_nodesCache.Cache(*propertyGroupingNode1, false);
    NavNodeExtendedData propertyGroupingNodeExtendedData1(*propertyGroupingNode1);
    propertyGroupingNodeExtendedData1.SetSpecificationHash(m_specification2->GetHash());
    propertyGroupingNodeExtendedData1.SetGroupingType((int)GroupingType::Property);
    
    TestNavNodePtr propertyGroupingNode2 = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *ecClassB, *ecClassB->GetPropertyP("Description"), "test2", rapidjson::Value("TestGroupingDescription"), false);
    propertyGroupingNode2->SetParentNode(*propertyGroupingNode1);
    m_nodesCache.Cache(*propertyGroupingNode2, false);
    NavNodeExtendedData propertyGroupingNodeExtendedData2(*propertyGroupingNode2);
    propertyGroupingNodeExtendedData2.SetSpecificationHash(m_specification2->GetHash());
    propertyGroupingNodeExtendedData2.SetGroupingType((int)GroupingType::Property);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification2, *propertyGroupingNode2);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("SecondPropertyGroupingNodeChildrenQueryReturnsThirdPropertyGroupingNodes", *m_specification2);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* Uses InstanceNodesOfSpecificClasses specification
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilder_MultiLevelGroupingTests, ThirdPropertyGroupingNodeChildrenQueryReturnsLabelGroupingNodes)
    {
    ECClassCP ecClassA = GetECClass("Basic4", "ClassA");
    ECClassCP ecClassB = GetECClass("Basic4", "ClassB");
    ECClassCP ecClassC = GetECClass("Basic4", "ClassC");

    TestNavNodePtr baseClassGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClassA, "test");
    m_nodesCache.Cache(*baseClassGroupingNode, false);
    NavNodeExtendedData baseClassGroupingNodeExtendedData(*baseClassGroupingNode);
    baseClassGroupingNodeExtendedData.SetSpecificationHash(m_specification2->GetHash());
    baseClassGroupingNodeExtendedData.SetGroupingType((int)GroupingType::BaseClass);

    TestNavNodePtr classGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClassB, "test");
    classGroupingNode->SetParentNode(*baseClassGroupingNode);
    m_nodesCache.Cache(*classGroupingNode, false);
    NavNodeExtendedData classGroupingNodeExtendedData(*classGroupingNode);
    classGroupingNodeExtendedData.SetSpecificationHash(m_specification2->GetHash());
    classGroupingNodeExtendedData.SetGroupingType((int)GroupingType::Class);
    
    TestNavNodePtr propertyGroupingNode1 = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *ecClassB, *ecClassB->GetPropertyP("SomeProperty"), "test1", rapidjson::Value(9), false);
    propertyGroupingNode1->SetParentNode(*classGroupingNode);
    m_nodesCache.Cache(*propertyGroupingNode1, false);
    NavNodeExtendedData propertyGroupingNodeExtendedData1(*propertyGroupingNode1);
    propertyGroupingNodeExtendedData1.SetSpecificationHash(m_specification2->GetHash());
    propertyGroupingNodeExtendedData1.SetGroupingType((int)GroupingType::Property);
    
    TestNavNodePtr propertyGroupingNode2 = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *ecClassB, *ecClassB->GetPropertyP("Description"), "test2", rapidjson::Value("TestGroupingDescription"), false);
    propertyGroupingNode2->SetParentNode(*propertyGroupingNode1);
    m_nodesCache.Cache(*propertyGroupingNode2, false);
    NavNodeExtendedData propertyGroupingNodeExtendedData2(*propertyGroupingNode2);
    propertyGroupingNodeExtendedData2.SetSpecificationHash(m_specification2->GetHash());
    propertyGroupingNodeExtendedData2.SetGroupingType((int)GroupingType::Property);
    
    TestNavNodePtr propertyGroupingNode3 = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *ecClassC, *ecClassC->GetPropertyP("SomeProperty"), "test3", rapidjson::Value(true), false);
    propertyGroupingNode3->SetParentNode(*propertyGroupingNode2);
    m_nodesCache.Cache(*propertyGroupingNode3, false);
    NavNodeExtendedData propertyGroupingNodeExtendedData3(*propertyGroupingNode3);
    propertyGroupingNodeExtendedData3.SetSpecificationHash(m_specification2->GetHash());
    propertyGroupingNodeExtendedData3.SetGroupingType((int)GroupingType::Property);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification2, *propertyGroupingNode3);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("ThirdPropertyGroupingNodeChildrenQueryReturnsLabelGroupingNodes", *m_specification2);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* Uses InstanceNodesOfSpecificClasses specification
* @bsitest                                      Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilder_MultiLevelGroupingTests, LabelGroupingNodeChildrenQueryReturnsInstanceNodes)
    {
    ECClassCP ecClassA = GetECClass("Basic4", "ClassA");
    ECClassCP ecClassB = GetECClass("Basic4", "ClassB");
    ECClassCP ecClassC = GetECClass("Basic4", "ClassC");

    TestNavNodePtr baseClassGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClassA, "test");
    m_nodesCache.Cache(*baseClassGroupingNode, false);
    NavNodeExtendedData baseClassGroupingNodeExtendedData(*baseClassGroupingNode);
    baseClassGroupingNodeExtendedData.SetSpecificationHash(m_specification2->GetHash());
    baseClassGroupingNodeExtendedData.SetGroupingType((int)GroupingType::BaseClass);

    TestNavNodePtr classGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClassB, "test");
    classGroupingNode->SetParentNode(*baseClassGroupingNode);
    m_nodesCache.Cache(*classGroupingNode, false);
    NavNodeExtendedData classGroupingNodeExtendedData(*classGroupingNode);
    classGroupingNodeExtendedData.SetSpecificationHash(m_specification2->GetHash());
    classGroupingNodeExtendedData.SetGroupingType((int)GroupingType::Class);
    
    TestNavNodePtr propertyGroupingNode1 = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *ecClassB, *ecClassB->GetPropertyP("SomeProperty"), "test1", rapidjson::Value(9), false);
    propertyGroupingNode1->SetParentNode(*classGroupingNode);
    m_nodesCache.Cache(*propertyGroupingNode1, false);
    NavNodeExtendedData propertyGroupingNodeExtendedData1(*propertyGroupingNode1);
    propertyGroupingNodeExtendedData1.SetSpecificationHash(m_specification2->GetHash());
    propertyGroupingNodeExtendedData1.SetGroupingType((int)GroupingType::Property);
    
    TestNavNodePtr propertyGroupingNode2 = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *ecClassB, *ecClassB->GetPropertyP("Description"), "test2", rapidjson::Value("TestGroupingDescription"), false);
    propertyGroupingNode2->SetParentNode(*propertyGroupingNode1);
    m_nodesCache.Cache(*propertyGroupingNode2, false);
    NavNodeExtendedData propertyGroupingNodeExtendedData2(*propertyGroupingNode2);
    propertyGroupingNodeExtendedData2.SetSpecificationHash(m_specification2->GetHash());
    propertyGroupingNodeExtendedData2.SetGroupingType((int)GroupingType::Property);
    
    TestNavNodePtr propertyGroupingNode3 = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *ecClassC, *ecClassC->GetPropertyP("SomeProperty"), "test3", rapidjson::Value(99), false);
    propertyGroupingNode3->SetParentNode(*propertyGroupingNode2);
    m_nodesCache.Cache(*propertyGroupingNode3, false);
    NavNodeExtendedData propertyGroupingNodeExtendedData3(*propertyGroupingNode3);
    propertyGroupingNodeExtendedData3.SetSpecificationHash(m_specification2->GetHash());
    propertyGroupingNodeExtendedData3.SetGroupingType((int)GroupingType::Property);
    
    JsonNavNodePtr labelGroupingNode = TestNodesHelper::CreateLabelGroupingNode(*m_connection, "test");
    labelGroupingNode->SetParentNode(*propertyGroupingNode3);
    m_nodesCache.Cache(*labelGroupingNode, false);
    NavNodeExtendedData labelGroupingNodeExtendedData(*labelGroupingNode);
    labelGroupingNodeExtendedData.SetSpecificationHash(m_specification2->GetHash());
    labelGroupingNodeExtendedData.SetGroupingType((int)GroupingType::DisplayLabel);
    labelGroupingNodeExtendedData.SetGroupedInstanceKeys({ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1))});
    
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification2, *labelGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());

    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("LabelGroupingNodeChildrenQueryReturnsInstanceNodes", *m_specification2);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }
