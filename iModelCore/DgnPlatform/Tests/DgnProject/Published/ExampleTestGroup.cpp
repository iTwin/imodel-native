//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/ExampleTestGroup.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

#define GROUP_SUBDIR L"ExampleTestGroup"
#define GROUP_SEED_FILENAME GROUP_SUBDIR L"/Test.dgndb"
#define DEFAULT_MODEL_NAME "Default"
#define DEFAULT_CATEGORY_NAME "DefaultCat"
#define DEFAULT_VIEW_NAME "DefaultView"

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//=======================================================================================
// @bsiclass                                                 Sam.Wilson     01/2016
//=======================================================================================
struct ExampleTestGroup : ::testing::Test
{
    Dgn::ScopedDgnHost m_testHost;

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

    DgnDbTestUtils::CreateSeedFiles();   // make sure that the base class has created the seed files that are used by all test groups

    DgnDbTestUtils::CreateSubDirectory(GROUP_SUBDIR);
    DgnDbPtr db = DgnDbTestUtils::CreateDgnDb(GROUP_SEED_FILENAME);
    SpatialModelPtr model = DgnDbTestUtils::InsertSpatialModel(*db, DgnModel::CreateModelCode(DEFAULT_MODEL_NAME));
    DgnDbTestUtils::InsertCameraView(*model, DEFAULT_VIEW_NAME);
    DgnDbTestUtils::InsertCategory(*db, DEFAULT_CATEGORY_NAME);
    }

//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
BETEST_TC_TEARDOWN(ExampleTestGroup)
    {
    // Note: leave your subdirectory in place. Don't remove it. That allows the 
    // base class to detect and throw an error if two groups try to use a directory of the same name.
    // Don't worry about stale data. The test runner will clean out everything at the start of the program.
    // You can empty the directory, if you want to save space.
    DgnDbTestUtils::EmptySubDirectory(GROUP_SUBDIR);
    }

//---------------------------------------------------------------------------------------
// An example of a test that opens the seed Db read-only and inspects it.
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
TEST_F(ExampleTestGroup, Test1)
    {
    if (true)
        {
        // Verify that the empty 3d seed file for the whole program is there.
        DgnDbPtr db = DgnDbTestUtils::OpenDgnDb(DgnDbTestUtils::GetEmpty3dSeedFileName());
        ASSERT_TRUE(db.IsValid());
        ASSERT_TRUE(db->Models().QueryModelId(DgnDbTestUtils::GetDefaultModelCode()).IsValid());
        ASSERT_TRUE(DgnCategory::QueryCategoryId(DgnDbTestUtils::GetDefaultCategoryName(), *db).IsValid());
        ASSERT_TRUE(ViewDefinition::QueryViewId(DgnDbTestUtils::GetDefaultCameraViewName(), *db).IsValid());
        }

    if (true)
        {
        // Verify that the seed file for just this group is there.
        DgnDbPtr db = DgnDbTestUtils::OpenDgnDb(GROUP_SEED_FILENAME);
        ASSERT_TRUE(db.IsValid());
        ASSERT_TRUE(db->Models().QueryModelId(DgnModel::CreateModelCode(DEFAULT_MODEL_NAME)).IsValid());
        ASSERT_TRUE(DgnCategory::QueryCategoryId(DEFAULT_CATEGORY_NAME, *db).IsValid());
        ASSERT_TRUE(ViewDefinition::QueryViewId(DEFAULT_VIEW_NAME, *db).IsValid());
        }
    }

//---------------------------------------------------------------------------------------
// An example of a test that makes a copy of the seed Db and then modifies the copy.
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
TEST_F(ExampleTestGroup, Test2)
    {
    if (true)
        {
        //  Verify that we can work with a read-write copy of the test group seed file
        DgnDbPtr db = DgnDbTestUtils::OpenDgnDbCopy(GROUP_SEED_FILENAME);
        db->Models().QueryModelId(DgnModel::CreateModelCode(DEFAULT_MODEL_NAME)).IsValid();

        SpatialModelPtr model2 = DgnDbTestUtils::InsertSpatialModel(*db, DgnModel::CreateModelCode("Model2"));
        ASSERT_TRUE(model2.IsValid());
        }
            
    if (true)
        {
        //  Verify that we can work with a read-write copy of the base seed file
        DgnDbPtr db = DgnDbTestUtils::OpenDgnDbCopy(DgnDbTestUtils::GetEmpty3dSeedFileName());
        db->Models().QueryModelId(DgnDbTestUtils::GetDefaultModelCode()).IsValid();

        SpatialModelPtr model2 = DgnDbTestUtils::InsertSpatialModel(*db, DgnModel::CreateModelCode("Model2"));
        ASSERT_TRUE(model2.IsValid());
        }

    if (true)
        {
        //  Verify that we can work with a read-write copy of the base seed file, where we assign a name
        DgnDbPtr db = DgnDbTestUtils::OpenDgnDbCopy(DgnDbTestUtils::GetEmpty3dSeedFileName(), L"Test2");
        db->Models().QueryModelId(DgnDbTestUtils::GetDefaultModelCode()).IsValid();

        SpatialModelPtr model2 = DgnDbTestUtils::InsertSpatialModel(*db, DgnModel::CreateModelCode("Model2"));
        ASSERT_TRUE(model2.IsValid());
        }
    }
