//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/ExampleTestGroup.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_DPTEST

#define EXAMPLE_MODEL_NAME  "Example"

//=======================================================================================
// @bsiclass                                                 Sam.Wilson     01/2016
//=======================================================================================
struct ExampleTestGroup : ::testing::Test
{
    Dgn::ScopedDgnHost m_testHost;
    static DgnDbTestUtils::SeedDbInfo s_seedFileInfo;

    ExampleTestGroup() { }

    // Insert the following macros into the definition of your test group subclass to declare that you want to set up shared resources for all of the tests in your group.
    public: static void SetUpTestCase();
    public: static void TearDownTestCase();
};

DgnDbTestUtils::SeedDbInfo ExampleTestGroup::s_seedFileInfo;

//---------------------------------------------------------------------------------------
//  Do one-time setup for all tests in this group
//  This is an example of a test group that need to make a copy of the main seed file for use by tests in the group. 
//  This case only comes up when the tests in the group need some setup that is specific to (and shared by) 
//  tests in this group. There's no point in making a group copy if the tests can start with a root seed file just as well.
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void ExampleTestGroup::SetUpTestCase()
    {
    ScopedDgnHost tempHost;

    //  Request a root seed file.
    DgnDbTestUtils::SeedDbInfo rootSeedInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, true));

    //  The group's seed file is essentially the same as the root seed file, but with a different relative path.
    //  Note that we must put our group seed file in a group-specific sub-directory
    ExampleTestGroup::s_seedFileInfo = rootSeedInfo;
    ExampleTestGroup::s_seedFileInfo.fileName.SetName(L"ExampleTestGroup/Test.dgndb");

    DgnDbPtr db = DgnDbTestUtils::OpenSeedDbCopy(rootSeedInfo.fileName, ExampleTestGroup::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
    ASSERT_TRUE(db.IsValid());
    
    // Suppose that all of the tests in this group will work with a certain setup, such as working woth two spatial models. 
    // Do that setup now, one time for the whole group. 
    // That way, each individual test can just make a copy of this seed file and start working, knowing that it has the needed setup.
    // This custom setup is what makes our group seed different from the root seed file.
    ASSERT_TRUE(DgnDbTestUtils::InsertSpatialModel(*db, DgnModel::CreateModelCode(EXAMPLE_MODEL_NAME)).IsValid());

    db->SaveSettings();
    db->SaveChanges();
    }

//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void ExampleTestGroup::TearDownTestCase()
    {
    // Note: leave your subdirectory in place. Don't remove it. That allows the 
    // base class to detect and throw an error if two groups try to use a directory of the same name.
    // Don't worry about stale data. The test runner will clean out everything at the start of the program.
    // You can empty the directory, if you want to save space.
    DgnDbTestUtils::EmptySubDirectory(ExampleTestGroup::s_seedFileInfo.fileName.GetDirectoryName());
    }

//---------------------------------------------------------------------------------------
// An example of a test that opens the seed Db read-only and inspects it.
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
TEST_F(ExampleTestGroup, Test1)
    {
    if (true)
        {
        // Verify that the root seed file is there.
        DgnDbTestUtils::SeedDbInfo rootSeedInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, true));
        DgnDbPtr db = DgnDbTestUtils::OpenSeedDb(rootSeedInfo.fileName);
        ASSERT_TRUE(db.IsValid());
        ASSERT_TRUE(db->Models().QueryModelId(rootSeedInfo.modelCode).IsValid());
        ASSERT_TRUE(DgnCategory::QueryCategoryId(rootSeedInfo.categoryName, *db).IsValid());
        ASSERT_TRUE(ViewDefinition::QueryViewId(rootSeedInfo.viewName, *db).IsValid());
        ASSERT_FALSE(db->Models().QueryModelId(DgnModel::CreateModelCode(EXAMPLE_MODEL_NAME)).IsValid()) << "Root seed file does not have this group's special setup";
        }

    if (true)
        {
        // Verify that the seed file for just this group is there.
        DgnDbPtr db = DgnDbTestUtils::OpenSeedDb(s_seedFileInfo.fileName);
        ASSERT_TRUE(db.IsValid());
        ASSERT_TRUE(db->Models().QueryModelId(s_seedFileInfo.modelCode).IsValid());
        ASSERT_TRUE(DgnCategory::QueryCategoryId(s_seedFileInfo.categoryName, *db).IsValid());
        ASSERT_TRUE(ViewDefinition::QueryViewId(s_seedFileInfo.viewName, *db).IsValid());
        ASSERT_TRUE(db->Models().QueryModelId(DgnModel::CreateModelCode(EXAMPLE_MODEL_NAME)).IsValid()) << "Group seed file has special setup";
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
        DgnDbPtr db = DgnDbTestUtils::OpenSeedDbCopy(s_seedFileInfo.fileName);
        DgnModelId defaultModelId = db->Models().QueryModelId(s_seedFileInfo.modelCode);
        ASSERT_TRUE(defaultModelId.IsValid());

        ASSERT_TRUE(db->Models().QueryModelId(DgnModel::CreateModelCode(EXAMPLE_MODEL_NAME)).IsValid()) << "Group seed file has special setup";

        SpatialModelPtr model2 = DgnDbTestUtils::InsertSpatialModel(*db, DgnModel::CreateModelCode("Model2"));
        ASSERT_TRUE(model2.IsValid());
        ASSERT_TRUE(model2->GetModelId() != defaultModelId);

        db->SaveChanges();
        }
            
    if (true)
        {
        //  Verify that we can work with a read-write copy of the root seed file
        DgnDbTestUtils::SeedDbInfo info = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, true));
        DgnDbPtr db = DgnDbTestUtils::OpenSeedDbCopy(info.fileName);

        ASSERT_FALSE(db->Models().QueryModelId(DgnModel::CreateModelCode(EXAMPLE_MODEL_NAME)).IsValid()) << "Root seed file does not have this group's special setup";

        SpatialModelPtr model2 = DgnDbTestUtils::InsertSpatialModel(*db, DgnModel::CreateModelCode("Model2"));
        ASSERT_TRUE(model2.IsValid());
        db->SaveChanges();
        }

    if (true)
        {
        //  Verify that we can work with a read-write copy of the root seed file, where we assign a name
        DgnDbTestUtils::SeedDbInfo info = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, true));
        DgnDbPtr db = DgnDbTestUtils::OpenSeedDbCopy(info.fileName, L"Test2");

        SpatialModelPtr model2 = DgnDbTestUtils::InsertSpatialModel(*db, DgnModel::CreateModelCode("Model2"));
        ASSERT_TRUE(model2.IsValid());
        db->SaveChanges();
        }
    }
