//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/BaseTestGroupExample.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "BaseTestGroup.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//=======================================================================================
// @bsiclass                                                 Sam.Wilson     01/2016
//=======================================================================================
struct ExampleTestGroup : BaseTestGroup
{
    // Insert the following macros into the definition of your test group subclass to declare that you want to set up shared resources for all of the tests in your group.
    BETEST_DECLARE_TC_SETUP
    BETEST_DECLARE_TC_TEARDOWN
};

//---------------------------------------------------------------------------------------
// Do one-time setup for all tests in this group
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
BETEST_TC_SETUP(ExampleTestGroup) 
    {
    ScopedDgnHost tempHost;
    ExampleTestGroup::CreateSubDirectory(L"ExampleTestGroup");
    DgnDbPtr db = ExampleTestGroup::CreateDgnDb(L"ExampleTestGroup/Test.dgndb");
    SpatialModelPtr model = ExampleTestGroup::InsertSpatialModel(*db, "Default");
    ExampleTestGroup::InsertCameraView(*model);
    ExampleTestGroup::InsertCategory(*db, "DefaultCategory");
    }

//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
BETEST_TC_TEARDOWN(ExampleTestGroup)
    {
    // Note: leave your subdirectory in place. Don't remove it. That allows the 
    // base class to detect and throw an error if two groups try to use a directory of the same name.
    // Don't worry about stale data. The test runner will clean out everything before starting.
    // You can empty the directory, if you want to save space.
    ExampleTestGroup::EmptySubDirectory(L"ExampleTestGroup");
    }

//---------------------------------------------------------------------------------------
// An example of a test that opens the seed Db read-only and inspects it.
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
TEST_F(ExampleTestGroup, Test1)
    {
    DgnDbPtr db = OpenDgnDb(L"ExampleTestGroup/Test.dgndb");

    ASSERT_TRUE(db->Models().QueryModelId(DgnModel::CreateModelCode("Default")).IsValid());
    ASSERT_TRUE(DgnCategory::QueryCategoryId("DefaultCategory", *db).IsValid());
    ASSERT_TRUE(ViewDefinition::QueryViewId("Default", *db).IsValid());
    }

//---------------------------------------------------------------------------------------
// An example of a test that makes a copy of the seed Db and then modifies the copy.
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
TEST_F(ExampleTestGroup, Test2)
    {
    DgnDbPtr db = CopyDgnDb(L"ExampleTestGroup/Test.dgndb", L"Test2.dgndb");

    ASSERT_TRUE(db->Models().QueryModelId(DgnModel::CreateModelCode("Default")).IsValid());
    SpatialModelPtr model2 = InsertSpatialModel(*db, "Model2");
    ASSERT_TRUE(model2.IsValid());
    }
