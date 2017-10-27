/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ContentQueryBuilderTests_ContentInstancesOfSpecificClasses.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClass)
    {
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "Basic1:Class1A", false);
    
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClass");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClassPolymorphically)
    {
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "Basic1:Class1A", true);
    
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClassPolymorphically");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Tautvydas.Zinys                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentInstacesOfSpecificClasses_ReturnsQueryWithNotIncludedHiddenProperties)
    {
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "Basic2:Class2", true);
    
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentInstacesOfSpecificClasses_ReturnsQueryWithNotIncludedHiddenProperty");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Tautvydas.Zinys                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryBuilderTests, ContentInstacesOfSpecificClasses_ReturnsQueryWithCalculatedProperties)
    {
    m_localizationProvider.SetHandler([](Utf8StringCR key, Utf8StringR localized)
        {
        EXPECT_TRUE(key.Equals("LocalizationId"));
        localized = "Test";
        return true;
        });

    ContentInstancesOfSpecificClassesSpecification spec(1, "", "Basic2:Class2", true);
    spec.AddCalculatedProperty(*new CalculatedPropertiesSpecification("Label@LocalizationId@_1", 1200, "\"Value\" & 1"));
    spec.AddCalculatedProperty(*new CalculatedPropertiesSpecification("Label@LocalizationId@_2", 1500, "this.Name & \"Test\""));
    
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentInstacesOfSpecificClasses_ReturnsQueryWithCalculatedProperties");
    EXPECT_TRUE(expected->IsEqual(*query))
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_EQ(expected->GetContract()->GetDescriptor(), query->GetContract()->GetDescriptor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Tautvydas.Zinys                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentInstacesOfSpecificClasses_ReturnsQueryWithPropertyPriority)
    {
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "Basic2:Class2", true);
    
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    int priority = query->GetContract()->GetDescriptor().GetVisibleFields()[0]->GetPriority();
    EXPECT_EQ(1200, priority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Tautvydas.Zinys                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentQueryBuilderTests, ContentInstacesOfSpecificClasses_ReturnsQueryWithDefaultPropertyPriority)
    {
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "Basic3:Class3", true);
    
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    int priority = query->GetContract()->GetDescriptor().GetVisibleFields()[0]->GetPriority();
    EXPECT_EQ(ContentDescriptor::Property::DEFAULT_PRIORITY, priority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleClasses)
    {
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "Basic1:Class1A,Class1B", false);
    
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleClasses");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleSchemaClasses)
    {
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "Basic1:Class1A;Basic2:Class2", false);
    
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleSchemaClasses");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_AppliesInstanceFilter)
    {
    ContentInstancesOfSpecificClassesSpecification spec(1, "this.DisplayLabel = 10", "Basic1:Class1A", false);
    
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentInstancesOfSpecificClasses_AppliesInstanceFilter");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_CategorizesFields)
    {
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "Basic1:Class2", false);

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(2, fields.size());

    EXPECT_STREQ("Class2_DisplayLabel", fields[0]->GetName().c_str());
    EXPECT_STREQ("Miscellaneous", fields[0]->GetCategory().GetName().c_str());

    EXPECT_STREQ("Class2_CategorizedProperty", fields[1]->GetName().c_str());
    EXPECT_STREQ("CategoryName", fields[1]->GetCategory().GetName().c_str());
    EXPECT_STREQ("Category Label", fields[1]->GetCategory().GetLabel().c_str());
    EXPECT_STREQ("Category description", fields[1]->GetCategory().GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* Reproduces the case when the spec results in 1 query and we can set the flag immediately 
  on that query.
* @bsitest                                      Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_SetsMergeResultsFlagForPropertyPaneContentType1)
    {
    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::PropertyPane);

    ContentInstancesOfSpecificClassesSpecification spec(1, "", "Basic1:Class1A", false);
    
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    ContentQueryCPtr expected = ExpectedQueries::GetInstance(BeTest::GetHost()).GetContentQuery("ContentInstancesOfSpecificClasses_SetsMergeResultsFlagForPropertyPaneContentType1");
    EXPECT_TRUE(expected->IsEqual(*query)) 
        << "Expected: " << expected->ToString() << "\r\n"
        << "Actual:   " << query->ToString();
    EXPECT_TRUE(expected->GetContract()->GetDescriptor().Equals(query->GetContract()->GetDescriptor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_MergesSimilarPropertiesIntoOneField)
    {
    ContentInstancesOfSpecificClassesSpecification spec(1, "", "Basic1:Class1A,Class2", false);

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    ContentQueryPtr query = GetQueryBuilder().CreateQuery(spec, *descriptor);
    ASSERT_TRUE(query.IsValid());

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(2, fields.size());

    EXPECT_STREQ("Class1A_Class2_DisplayLabel", fields[0]->GetName().c_str());
    EXPECT_EQ(2, fields[0]->AsPropertiesField()->GetProperties().size());

    EXPECT_STREQ("Class2_CategorizedProperty", fields[1]->GetName().c_str());
    EXPECT_EQ(1, fields[1]->AsPropertiesField()->GetProperties().size());
    }