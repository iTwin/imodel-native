/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Persistence/DataReadOptionsTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DataReadOptionsTests.h"

#include <WebServices/Cache/Persistence/DataReadOptions.h>
#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>

#include "../CachingTestsHelper.h"

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

#ifdef USE_GTEST
TEST_F(DataReadOptionsTests, GetSelected_NoChanges_ReturnsNull)
    {
    EXPECT_TRUE(DataReadOptions().AreAllClassesSelected());
    }

TEST_F(DataReadOptionsTests, SelectAllClasses_Called_GetSelectedReturnsNull)
    {
    DataReadOptions options;

    options.SelectAllClasses();

    EXPECT_TRUE(options.AreAllClassesSelected());
    }

TEST_F(DataReadOptionsTests, SelectAllClasses_SelectClassBefore_AllClassesSelected)
    {
    DataReadOptions options;

    options.SelectClass("Schema.Foo");
    options.SelectAllClasses();

    EXPECT_TRUE(options.AreAllClassesSelected());
    }

TEST_F(DataReadOptionsTests, SelectClass_CalledOnce_ClassAddedToSelected)
    {
    DataReadOptions options;

    options.SelectClass("Schema.Foo");

    EXPECT_EQ(1, options.GetSelected().size());
    EXPECT_EQ("Schema.Foo", options.GetSelected()[0].GetClassKey());
    }

TEST_F(DataReadOptionsTests, SelectClass_CalledOnceWithECClass_ClassAddedToSelected)
    {
    DataReadOptions options;

    auto schema = StubSchema();
    options.SelectClass(*schema->GetClassCP("TestClass"));

    EXPECT_EQ(1, options.GetSelected().size());
    EXPECT_EQ("TestSchema.TestClass", options.GetSelected()[0].GetClassKey());
    }

TEST_F(DataReadOptionsTests, SelectClass_CalledWithTwoClasses_BothClassesAddedToSelected)
    {
    DataReadOptions options;

    options.SelectClass("Schema.Foo");
    options.SelectClass("Schema.Boo");

    EXPECT_EQ(2, options.GetSelected().size());
    EXPECT_EQ("Schema.Foo", options.GetSelected()[0].GetClassKey());
    EXPECT_EQ("Schema.Boo", options.GetSelected()[1].GetClassKey());
    }

TEST_F(DataReadOptionsTests, SelectClass_CalledOnce_AllPropertiesSelectedForClass)
    {
    DataReadOptions options;

    options.SelectClass("Schema.Foo");

    EXPECT_TRUE(options.GetSelected()[0].AreAllPropertiesSelected());
    }

TEST_F(DataReadOptionsTests, SelectClass_SelectAllClassesCalledBefore_OnlyOneClassIsSelected)
    {
    DataReadOptions options;

    options.SelectAllClasses();
    options.SelectClass("Schema.Foo");

    EXPECT_EQ(1, options.GetSelected().size());
    }

TEST_F(DataReadOptionsTests, SelectClassAndProperty_CalledOnce_ClassAndPropertyAddedToSelected)
    {
    DataReadOptions options;

    options.SelectClassAndProperty("Schema.Foo", "Property");

    EXPECT_EQ(1, options.GetSelected().size());
    EXPECT_EQ("Schema.Foo", options.GetSelected()[0].GetClassKey());
    EXPECT_EQ("Property", *options.GetSelected()[0].GetSelectedProperties().begin());
    }

TEST_F(DataReadOptionsTests, SelectClassAndProperty_CalledOnceWithECProperty_ClassAndPropertyAddedToSelected)
    {
    DataReadOptions options;

    auto schema = StubSchema();
    options.SelectClassAndProperty(*schema->GetClassCP("TestClass")->GetPropertyP("TestProperty"));

    EXPECT_EQ(1, options.GetSelected().size());
    EXPECT_EQ("TestSchema.TestClass", options.GetSelected()[0].GetClassKey());
    EXPECT_EQ("TestProperty", *options.GetSelected()[0].GetSelectedProperties().begin());
    }

TEST_F(DataReadOptionsTests, SelectClassAndProperty_CalledWithDifferentProperties_ClassAndPropertiesAddedToSelected)
    {
    DataReadOptions options;

    options.SelectClassAndProperty("Schema.Foo", "PropertyA");
    options.SelectClassAndProperty("Schema.Foo", "PropertyB");

    EXPECT_EQ(1, options.GetSelected().size());
    auto& selectedClass = options.GetSelected()[0];
    EXPECT_EQ(2, selectedClass.GetSelectedProperties().size());
    EXPECT_CONTAINS(selectedClass.GetSelectedProperties(), "PropertyA");
    EXPECT_CONTAINS(selectedClass.GetSelectedProperties(), "PropertyB");
    }

TEST_F(DataReadOptionsTests, SelectClassWithAllProperties_CalledOnEmptyOptios_AllPropertiesSelectedForClass)
    {
    DataReadOptions options;

    options.SelectClassWithAllProperties("Schema.Foo");

    auto& selectedClass = options.GetSelected()[0];
    EXPECT_TRUE(selectedClass.AreAllPropertiesSelected());
    EXPECT_EQ(0, selectedClass.GetSelectedProperties().size());
    }

TEST_F(DataReadOptionsTests, SelectClassWithAllProperties_CalledWithECClass_AllPropertiesSelectedForClass)
    {
    DataReadOptions options;

    auto schema = StubSchema();
    options.SelectClassWithAllProperties(*schema->GetClassCP("TestClass"));

    auto& selectedClass = options.GetSelected()[0];
    EXPECT_EQ("TestSchema.TestClass", selectedClass.GetClassKey());
    EXPECT_TRUE(selectedClass.AreAllPropertiesSelected());
    EXPECT_EQ(0, selectedClass.GetSelectedProperties().size());
    }

TEST_F(DataReadOptionsTests, SelectClassWithAllProperties_PreviouslySelectedProperty_ClearsOldSelection)
    {
    DataReadOptions options;

    options.SelectClassAndProperty("Schema.Foo", "Property");
    options.SelectClassWithAllProperties("Schema.Foo");

    auto& selectedClass = options.GetSelected()[0];
    EXPECT_TRUE(selectedClass.AreAllPropertiesSelected());
    EXPECT_EQ(0, selectedClass.GetSelectedProperties().size());
    }

TEST_F(DataReadOptionsTests, SelectClassWithNoProperties_CalledOnEmptyOptios_NoPropertiesSelectedForClass)
    {
    DataReadOptions options;

    options.SelectClassWithNoProperties("Schema.Foo");

    auto& selectedClass = options.GetSelected()[0];
    EXPECT_EQ("Schema.Foo", selectedClass.GetClassKey());
    EXPECT_FALSE(selectedClass.AreAllPropertiesSelected());
    EXPECT_EQ(0, selectedClass.GetSelectedProperties().size());
    }

TEST_F(DataReadOptionsTests, SelectClassWithNoProperties_CalledOnEmptyOptiosWithECClass_NoPropertiesSelectedForClass)
    {
    DataReadOptions options;

    auto schema = StubSchema();
    options.SelectClassWithNoProperties(*schema->GetClassCP("TestClass"));

    auto& selectedClass = options.GetSelected()[0];
    EXPECT_EQ("TestSchema.TestClass", selectedClass.GetClassKey());
    EXPECT_FALSE(selectedClass.AreAllPropertiesSelected());
    EXPECT_EQ(0, selectedClass.GetSelectedProperties().size());
    }

TEST_F(DataReadOptionsTests, SelectClassWithNoProperties_PreviouslySelectedProperty_ClearsOldSelection)
    {
    DataReadOptions options;

    options.SelectClassAndProperty("Schema.Foo", "Property");
    options.SelectClassWithNoProperties("Schema.Foo");

    auto& selectedClass = options.GetSelected()[0];
    EXPECT_FALSE(selectedClass.AreAllPropertiesSelected());
    EXPECT_EQ(0, selectedClass.GetSelectedProperties().size());
    }

TEST_F(DataReadOptionsTests, SelectClassAndProperty_SelectAllClassesCalledBefore_OnlyOneClassIsSelected)
    {
    DataReadOptions options;

    options.SelectAllClasses();
    options.SelectClassAndProperty("Schema.Foo", "Property");

    EXPECT_EQ(1, options.GetSelected().size());
    }

TEST_F(DataReadOptionsTests, AddOrderByClassAndProperties_NoOrdering_Empty)
    {
    EXPECT_TRUE(DataReadOptions().GetOrderBy().empty());
    }

TEST_F(DataReadOptionsTests, AddOrderByClassAndProperties_MultipleOrdering_AllAdded)
    {
    bvector<DataReadOptions::OrderedProperty> properties;
    properties.push_back(Utf8String("C"));
    properties.push_back(Utf8String("A"));
    DataReadOptions options;
    options.AddOrderByClassAndProperties("Schema.Foo", properties);

    EXPECT_EQ(1, options.GetOrderBy().size());
    EXPECT_EQ("Schema.Foo", options.GetOrderBy()[0].GetClassKey());
    EXPECT_EQ(2, options.GetOrderBy()[0].GetProperties().size());
    EXPECT_EQ("C", options.GetOrderBy()[0].GetProperties()[0].GetName());
    EXPECT_EQ("A", options.GetOrderBy()[0].GetProperties()[1].GetName());
    }

TEST_F(DataReadOptionsTests, AddOrderByClassAndProperties_OrderingWithECClass_Added)
    {
    auto schema = StubSchema();

    bvector<DataReadOptions::OrderedProperty> properties;
    properties.push_back(Utf8String("TestProperty"));

    DataReadOptions options;
    options.AddOrderByClassAndProperties(*schema->GetClassCP("TestClass"), properties);

    EXPECT_EQ(1, options.GetOrderBy().size());
    EXPECT_EQ("TestSchema.TestClass", options.GetOrderBy()[0].GetClassKey());
    EXPECT_EQ(1, options.GetOrderBy()[0].GetProperties().size());
    EXPECT_EQ("TestProperty", options.GetOrderBy()[0].GetProperties()[0].GetName());
    }

TEST_F(DataReadOptionsTests, AddOrderByClassAndProperties_PropertiesAdded_CorrectDirection)
    {
    bvector<DataReadOptions::OrderedProperty> properties;
    properties.push_back(DataReadOptions::OrderedProperty("A", false));
    properties.push_back(DataReadOptions::OrderedProperty("B", true));
    DataReadOptions options;
    options.AddOrderByClassAndProperties("Schema.Foo", properties);

    EXPECT_EQ(false, options.GetOrderBy()[0].GetProperties()[0].IsOrderAscending());
    EXPECT_EQ(true, options.GetOrderBy()[0].GetProperties()[1].IsOrderAscending());
    }

TEST_F(DataReadOptionsTests, GetSelectedClass_NoSelectedClassesAndECClassPassed_ReturnsNull)
    {
    DataReadOptions options;

    auto schema = StubSchema("TestSchema");
    auto selectedClass = options.GetSelectedClass(*schema->GetClassCP("TestClass"));
    ASSERT_EQ(nullptr, selectedClass);
    }

TEST_F(DataReadOptionsTests, GetSelectedClass_SelectedECClassPassed_ReturnsSelectedClass)
    {
    DataReadOptions options;
    options.SelectClass("TestSchema.TestClass");

    auto schema = StubSchema("TestSchema");
    auto selectedClass = options.GetSelectedClass(*schema->GetClassCP("TestClass"));
    ASSERT_NE(nullptr, selectedClass);
    EXPECT_EQ("TestSchema.TestClass", selectedClass->GetClassKey());
    }

TEST_F(DataReadOptionsTests, GetSelectProperties_ClassNotSelected_ReturnsNull)
    {
    DataReadOptions options;
    options.SelectClass("SomeSchema.SomeClass");

    auto schema = StubSchema("TestSchema");
    auto properties = options.GetSelectProperties(*schema->GetClassCP("TestClass"));

    EXPECT_EQ(nullptr, properties);
    }

TEST_F(DataReadOptionsTests, GetSelectProperties_SelectingAllClasses_ReturnsSelectAllProperties)
    {
    DataReadOptions options;
    options.SelectAllClasses();

    auto schema = StubSchema("TestSchema");
    auto properties = options.GetSelectProperties(*schema->GetClassCP("TestClass"));

    EXPECT_NE(nullptr, properties);
    EXPECT_TRUE(properties->GetSelectAll());
    EXPECT_TRUE(properties->GetSelectInstanceId());
    EXPECT_THAT(properties->GetProperties(), IsEmpty());
    EXPECT_THAT(properties->GetExtendedProperties(), IsEmpty());
    }

TEST_F(DataReadOptionsTests, GetSelectProperties_SelectingAllClassesAndSpecificPropertyForClass_ReturnsSelectSpecificProperty)
    {
    DataReadOptions options;
    options.SelectAllClasses();
    options.SelectClassAndProperty("TestSchema.TestClass", "TestProperty");

    auto schema = StubSchema("TestSchema");
    ECClassCR ecClass = *schema->GetClassCP("TestClass");
    auto properties = options.GetSelectProperties(ecClass);

    EXPECT_NE(nullptr, properties);
    EXPECT_FALSE(properties->GetSelectAll());
    EXPECT_TRUE(properties->GetSelectInstanceId());
    EXPECT_THAT(properties->GetProperties(), SizeIs(1));
    EXPECT_EQ(ecClass.GetPropertyP("TestProperty"), *properties->GetProperties().begin());
    EXPECT_THAT(properties->GetExtendedProperties(), IsEmpty());
    }

TEST_F(DataReadOptionsTests, GetSelectProperties_SelectingPropertyThatDoesNotExistInClass_AddsPropertyToExtendedProperties)
    {
    DataReadOptions options;
    options.SelectClassAndProperty("TestSchema.TestClass", "SomeProperty");

    auto schema = StubSchema("TestSchema");
    auto properties = options.GetSelectProperties(*schema->GetClassCP("TestClass"));

    EXPECT_NE(nullptr, properties);
    EXPECT_FALSE(properties->GetSelectAll());
    EXPECT_TRUE(properties->GetSelectInstanceId());
    EXPECT_THAT(properties->GetProperties(), IsEmpty());
    EXPECT_THAT(properties->GetExtendedProperties(), SizeIs(1));
    EXPECT_EQ("SomeProperty", *properties->GetExtendedProperties().begin());
    }

TEST_F(DataReadOptionsTests, GetSortPriority_TwoClassesInOrdering_ReturnsCorrectPriorities)
    {
    DataReadOptions options;
    options.AddOrderByClassAndProperties("TestSchema1.TestClass", bvector<DataReadOptions::OrderedProperty>());
    options.AddOrderByClassAndProperties("TestSchema2.TestClass", bvector<DataReadOptions::OrderedProperty>());

    auto schema1 = StubSchema("TestSchema1");
    auto schema2 = StubSchema("TestSchema2");

    auto priority1 = options.GetSortPriority(*schema1->GetClassCP("TestClass"));
    auto priority2 = options.GetSortPriority(*schema2->GetClassCP("TestClass"));

    EXPECT_LT(priority2, priority1);
    }

TEST_F(DataReadOptionsTests, GetSortProperties_NoClassInOrdering_ReturnsEmpty)
    {
    DataReadOptions options;
    auto schema = StubSchema("TestSchema");

    auto properties = options.GetSortProperties(*schema->GetClassCP("TestClass"));
    EXPECT_THAT(properties, IsEmpty());
    }

TEST_F(DataReadOptionsTests, GetSortProperties_ClassWithOrderProperties_ReturnsSortProperties)
    {
    DataReadOptions options;
    bvector<DataReadOptions::OrderedProperty> orderProperties;
    orderProperties.push_back(DataReadOptions::OrderedProperty("TestProperty", false));
    options.AddOrderByClassAndProperties("TestSchema.TestClass", orderProperties);

    auto schema = StubSchema("TestSchema");
    ECClassCR ecClass = *schema->GetClassCP("TestClass");
    auto properties = options.GetSortProperties(ecClass);

    EXPECT_THAT(properties, SizeIs(1));
    EXPECT_EQ(ecClass.GetPropertyP("TestProperty"), &properties[0].GetProperty());
    EXPECT_EQ(false, properties[0].GetSortAscending());
    }
#endif