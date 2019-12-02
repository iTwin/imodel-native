/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    Utf8String excludeSchemasStr("E:");
    bvector<ECSchemaCP> schemas = ExpectedQueries::GetInstance(BeTest::GetHost()).GetDb().Schemas().GetSchemas(false);
    for (size_t i = 0; i < schemas.size(); ++i)
        {
        if (schemas[i]->GetName().Equals("Basic2"))
            continue;
        if (i > 0)
            excludeSchemasStr.append(",");
        excludeSchemasStr.append(schemas[i]->GetName());
        }

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, excludeSchemasStr);
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
* @bsitest                                      Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryBuilderTests, AllInstanceNodes_HideExpression)
    {
    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "");
    spec.SetHideExpression("test");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    EXPECT_TRUE(query->GetResultParameters().GetNavNodeExtendedData().HasHideExpression());
    EXPECT_STREQ("test", query->GetResultParameters().GetNavNodeExtendedData().GetHideExpression());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigationQueryBuilderTests, AllInstanceNodes_HideExpression_NotSetIfHiddenInHierarchy)
    {
    AllInstanceNodesSpecification spec(1, false, true, false, false, false, "");
    spec.SetHideExpression("test");
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    EXPECT_FALSE(query->GetResultParameters().GetNavNodeExtendedData().HasHideExpression());
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
    TestNavNodePtr parentNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClass, "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);
    
    AllInstanceNodesSpecification spec(1, false, false, false, true, false, "Basic1");

    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationHash(spec.GetHash());
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
    JsonNavNodePtr parentNode = TestNodesHelper::CreateLabelGroupingNode(*m_connection, "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    AllInstanceNodesSpecification spec(1, false, false, false, false, true, "Basic1");
    
    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::DisplayLabel);
    extendedData.SetInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));

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
    TestNavNodePtr parentNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClass, "MyLabel");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);
    
    AllInstanceNodesSpecification spec(1, false, false, false, true, true, "Basic1");

    NavNodeExtendedData extendedData(*parentNode);
    extendedData.SetSpecificationHash(spec.GetHash());
    extendedData.SetGroupingType((int)GroupingType::Class);
    extendedData.SetInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));

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
    TestNavNodePtr classGroupingNode = TestNodesHelper::CreateClassGroupingNode(*m_connection, *ecClass, "Class Grouping Node");
    JsonNavNodePtr labelGroupingNode = TestNodesHelper::CreateLabelGroupingNode(*m_connection, "Label Grouping Node");
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *classGroupingNode);
    labelGroupingNode->SetParentNode(*classGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *labelGroupingNode);

    AllInstanceNodesSpecification spec(1, false, false, false, true, true, "Basic1");

    NavNodeExtendedData classNodeExtendedData(*classGroupingNode);
    NavNodeExtendedData labelNodeExtendedData(*labelGroupingNode);
    classNodeExtendedData.SetSpecificationHash(spec.GetHash());
    classNodeExtendedData.SetGroupingType((int)GroupingType::Class);
    labelNodeExtendedData.SetSpecificationHash(spec.GetHash());
    labelNodeExtendedData.SetGroupingType((int)GroupingType::DisplayLabel);
    labelNodeExtendedData.SetInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));

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
    TestNavNodePtr instanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *instance);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *instanceNode);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, "Basic3");
        
    NavNodeExtendedData instanceNodeExtendedData(*instanceNode);
    instanceNodeExtendedData.SetSpecificationHash(spec.GetHash());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *instanceNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_RecursiveNodeRelationships", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_InstanceLabelOverride_AppliedByPriority, R"*(
    <ECEntityClass typeName="Class1">
        <ECProperty propertyName="Code" typeName="string" />
        <ECProperty propertyName="Description" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_InstanceLabelOverride_AppliedByPriority)
    {
    Utf8String schemaName = BeTest::GetNameOfCurrentTest();
    ECClassCP ecClass = GetECClass(schemaName.c_str(), "Class1");
    IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    TestNavNodePtr instanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *instance);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *instanceNode);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, schemaName);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, ecClass->GetFullName(), "Code"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, true, ecClass->GetFullName(), "Description"));

    NavNodeExtendedData instanceNodeExtendedData(*instanceNode);
    instanceNodeExtendedData.SetSpecificationHash(spec.GetHash());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *instanceNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_InstanceLabelOverride_AppliedByPriority", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AllInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels, R"*(
    <ECEntityClass typeName="Class1">
        <ECProperty propertyName="Code" typeName="string" />
        <ECProperty propertyName="Description" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Class2">
        <ECProperty propertyName="Code" typeName="string" />
        <ECProperty propertyName="Description" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, AllInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels)
    {
    Utf8String schemaName = BeTest::GetNameOfCurrentTest();
    ECClassCP ecClass = GetECClass(schemaName.c_str(), "Class1");
    IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    TestNavNodePtr instanceNode = TestNodesHelper::CreateInstanceNode(*m_connection, *instance);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *instanceNode);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, schemaName);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, ecClass->GetFullName(), "Code"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, true, ecClass->GetFullName(), "Description"));

    NavNodeExtendedData instanceNodeExtendedData(*instanceNode);
    instanceNodeExtendedData.SetSpecificationHash(spec.GetHash());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *instanceNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ASSERT_TRUE(query.IsValid());
    
    NavigationQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetNavigationQuery("AllInstanceNodes_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels", spec);
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    }