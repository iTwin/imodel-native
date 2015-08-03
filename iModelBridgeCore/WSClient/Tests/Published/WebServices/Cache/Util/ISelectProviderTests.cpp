/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Util/ISelectProviderTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ISelectProviderTests.h"

#include <WebServices/Cache/Util/ISelectProvider.h>
#include "../CachingTestsHelper.h"

#ifdef USE_GTEST
using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(ISelectProviderTests, GetSelectProperties_AnyClass_SelectAll)
    {
    auto schema = StubSchema();

    auto select = ISelectProvider().GetSelectProperties(*schema->GetClassCP("TestClass"));

    ASSERT_THAT(select, Not(nullptr));
    EXPECT_THAT(select->GetSelectInstanceId(), true);
    EXPECT_THAT(select->GetSelectAll(), true);
    EXPECT_THAT(select->GetProperties(), IsEmpty());
    EXPECT_THAT(select->GetExtendedProperties(), IsEmpty());
    }

TEST_F(ISelectProviderTests, GetSortPriority_AnyClass_Zero)
    {
    auto schema = StubSchema();
    ASSERT_THAT(ISelectProvider().GetSortPriority(*schema->GetClassCP("TestClass")), 0);
    }

TEST_F(ISelectProviderTests, GetSortProperties_AnyClass_Empty)
    {
    auto schema = StubSchema();
    ASSERT_THAT(ISelectProvider().GetSortProperties(*schema->GetClassCP("TestClass")), IsEmpty());
    }

TEST_F(ISelectProviderTests, SelectProperties_Ctor_Default_SelectAllWithid)
    {
    ISelectProvider::SelectProperties select;

    EXPECT_THAT(select.GetSelectInstanceId(), true);
    EXPECT_THAT(select.GetSelectAll(), true);
    EXPECT_THAT(select.GetProperties(), IsEmpty());
    EXPECT_THAT(select.GetExtendedProperties(), IsEmpty());
    }

TEST_F(ISelectProviderTests, SelectProperties_SetSelectAll_True_RemovesProperties)
    {
    auto schema = StubSchema();

    ISelectProvider::SelectProperties select;

    select.AddProperty(schema->GetClassCP("TestClass")->GetPropertyP("TestProperty"));
    EXPECT_THAT(select.GetProperties(), SizeIs(1));

    select.SetSelectAll(true);
    EXPECT_THAT(select.GetProperties(), IsEmpty());
    EXPECT_THAT(select.GetSelectAll(), true);
    }

TEST_F(ISelectProviderTests, SelectProperties_SetSelectAll_False_LeavesProperties)
    {
    auto schema = StubSchema();

    ISelectProvider::SelectProperties select;

    select.AddProperty(schema->GetClassCP("TestClass")->GetPropertyP("TestProperty"));
    EXPECT_THAT(select.GetProperties(), SizeIs(1));

    select.SetSelectAll(false);
    EXPECT_THAT(select.GetProperties(), SizeIs(1));
    EXPECT_THAT(select.GetSelectAll(), false);
    }

TEST_F(ISelectProviderTests, SelectProperties_SetSelectAll_True_DoesNotRemoveExtendedProperties)
    {
    ISelectProvider::SelectProperties select;

    select.AddExtendedProperty("Foo");
    EXPECT_THAT(select.GetExtendedProperties(), SizeIs(1));

    select.SetSelectAll(true);
    EXPECT_THAT(select.GetExtendedProperties(), SizeIs(1));
    EXPECT_THAT(select.GetSelectAll(), true);
    }

TEST_F(ISelectProviderTests, SelectProperties_AddProperty_Null_SkipsProperty)
    {
    ISelectProvider::SelectProperties select;

    select.AddProperty(nullptr);
    EXPECT_THAT(select.GetExtendedProperties(), IsEmpty());
    }

TEST_F(ISelectProviderTests, SelectProperties_AddProperty_DuplicateProperties_DoesNotAddDuplicates)
    {
    auto schema = StubSchema();

    ISelectProvider::SelectProperties select;

    select.AddProperty(schema->GetClassCP("TestClass")->GetPropertyP("TestProperty"));
    select.AddProperty(schema->GetClassCP("TestClass")->GetPropertyP("TestProperty"));
    EXPECT_THAT(select.GetProperties(), SizeIs(1));
    }

TEST_F(ISelectProviderTests, SelectProperties_AddProperty_Null_SelectAllFalse)
    {
    ISelectProvider::SelectProperties select;
    EXPECT_THAT(select.GetSelectAll(), true);

    select.AddProperty(nullptr);
    EXPECT_THAT(select.GetSelectAll(), false);
    }

TEST_F(ISelectProviderTests, SelectProperties_AddProperty_ValidProperty_SelectAllFalse)
    {
    auto schema = StubSchema();

    ISelectProvider::SelectProperties select;
    EXPECT_THAT(select.GetSelectAll(), true);

    select.AddProperty(schema->GetClassCP("TestClass")->GetPropertyP("TestProperty"));
    EXPECT_THAT(select.GetSelectAll(), false);
    }

TEST_F(ISelectProviderTests, SelectProperties_AddExtendedProperty_Empty_DoesNotEmptyProperties)
    {
    ISelectProvider::SelectProperties select;

    select.AddExtendedProperty("");
    EXPECT_THAT(select.GetExtendedProperties(), IsEmpty());
    }

TEST_F(ISelectProviderTests, SelectProperties_AddExtendedProperty_PropertyAdded_DoesNotClearSelectAll)
    {
    ISelectProvider::SelectProperties select;

    select.AddExtendedProperty("SomeProperty");
    EXPECT_THAT(select.GetExtendedProperties(), SizeIs(1));
    EXPECT_THAT(select.GetSelectAll(), true);
    }

TEST_F(ISelectProviderTests, SelectProperties_AddExtendedProperty_DuplicateProperty_DoesNotAddDuplicates)
    {
    ISelectProvider::SelectProperties select;

    select.AddExtendedProperty("SomeProperty");
    select.AddExtendedProperty("SomeProperty");
    EXPECT_THAT(select.GetExtendedProperties(), SizeIs(1));
    }

TEST_F(ISelectProviderTests, SortProperty_Ctor_Default_ReturnsAscendingTrue)
    {
    auto schema = StubSchema();
    auto classProperty = schema->GetClassCP("TestClass")->GetPropertyP("TestProperty");

    ISelectProvider::SortProperty sort(*classProperty);
    EXPECT_THAT(sort.GetSortAscending(), true);
    }

TEST_F(ISelectProviderTests, SortProperty_Ctor_AscendingFalse_ReturnsAscendingFalse)
    {
    auto schema = StubSchema();
    auto classProperty = schema->GetClassCP("TestClass")->GetPropertyP("TestProperty");

    ISelectProvider::SortProperty sort(*classProperty, false);
    EXPECT_THAT(sort.GetSortAscending(), false);
    }

TEST_F(ISelectProviderTests, SortProperty_Ctor_AscendingTrue_ReturnsAscendingTrue)
    {
    auto schema = StubSchema();
    auto classProperty = schema->GetClassCP("TestClass")->GetPropertyP("TestProperty");

    ISelectProvider::SortProperty sort(*classProperty, true);
    EXPECT_THAT(sort.GetSortAscending(), true);
    }

TEST_F(ISelectProviderTests, SortProperty_Ctor_Default_ReturnsSameProperty)
    {
    auto schema = StubSchema();
    auto classProperty = schema->GetClassCP("TestClass")->GetPropertyP("TestProperty");

    ISelectProvider::SortProperty sort(*classProperty, true);
    EXPECT_THAT(&sort.GetProperty(), classProperty);
    }
#endif