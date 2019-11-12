//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See COPYRIGHT.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include "../TestFixture/DgnPlatformSeedManager.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DPTEST

#define EXAMPLE_MODEL_NAME  "Example"

//=======================================================================================
// @bsiclass                                                 Sam.Wilson     01/2016
//=======================================================================================
struct ExampleTestGroup : ::testing::Test
{
    Dgn::ScopedDgnHost m_testHost;
    static DgnPlatformSeedManager::SeedDbInfo s_seedFileInfo;

    ExampleTestGroup() { }

    // Insert the following macros into the definition of your test group subclass to declare that you want to set up shared resources for all of the tests in your group.
    public: static void SetUpTestCase();
    public: static void TearDownTestCase();
};

DgnPlatformSeedManager::SeedDbInfo ExampleTestGroup::s_seedFileInfo;

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
    DgnPlatformSeedManager::SeedDbInfo rootSeedInfo = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, DgnPlatformSeedManager::SeedDbOptions(false, true));

    //  The group's seed file is essentially the same as the root seed file, but with a different relative path.
    //  Note that we must put our group seed file in a group-specific sub-directory
    ExampleTestGroup::s_seedFileInfo = rootSeedInfo;
    ExampleTestGroup::s_seedFileInfo.fileName.SetName(L"ExampleTestGroup/Test.bim");

    DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(rootSeedInfo.fileName, ExampleTestGroup::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
    ASSERT_TRUE(db.IsValid());
    
    // Suppose that all of the tests in this group will work with a certain setup, such as working woth two spatial models. 
    // Do that setup now, one time for the whole group. 
    // That way, each individual test can just make a copy of this seed file and start working, knowing that it has the needed setup.
    // This custom setup is what makes our group seed different from the root seed file.
    ASSERT_TRUE(DgnDbTestUtils::InsertPhysicalModel(*db, EXAMPLE_MODEL_NAME).IsValid());

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
    DgnPlatformSeedManager::EmptySubDirectory(ExampleTestGroup::s_seedFileInfo.fileName.GetDirectoryName());
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
        DgnPlatformSeedManager::SeedDbInfo rootSeedInfo = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, DgnPlatformSeedManager::SeedDbOptions(false, true));
        DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDb(rootSeedInfo.fileName);
        ASSERT_TRUE(db.IsValid());
        DgnCode physicalPartitionCode = PhysicalPartition::CreateCode(*db->Elements().GetRootSubject(), rootSeedInfo.physicalPartitionName);
        ASSERT_TRUE(db->Models().QuerySubModelId(physicalPartitionCode).IsValid());
        ASSERT_TRUE(SpatialCategory::QueryCategoryId(db->GetDictionaryModel(), rootSeedInfo.categoryName).IsValid());
        ASSERT_TRUE(ViewDefinition::QueryViewId(db->GetDictionaryModel(), rootSeedInfo.viewName).IsValid());
        DgnCode examplePartitionCode = PhysicalPartition::CreateCode(*db->Elements().GetRootSubject(), EXAMPLE_MODEL_NAME);
        ASSERT_FALSE(db->Models().QuerySubModelId(examplePartitionCode).IsValid()) << "Root seed file does not have this group's special setup";
        }

    if (true)
        {
        // Verify that the seed file for just this group is there.
        DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDb(s_seedFileInfo.fileName);
        ASSERT_TRUE(db.IsValid());
        DgnCode physicalPartitionCode = PhysicalPartition::CreateCode(*db->Elements().GetRootSubject(), s_seedFileInfo.physicalPartitionName);
        ASSERT_TRUE(db->Models().QuerySubModelId(physicalPartitionCode).IsValid());
        ASSERT_TRUE(SpatialCategory::QueryCategoryId(db->GetDictionaryModel(), s_seedFileInfo.categoryName).IsValid());
        ASSERT_TRUE(ViewDefinition::QueryViewId(db->GetDictionaryModel(), s_seedFileInfo.viewName).IsValid());
        DgnCode examplePartitionCode = PhysicalPartition::CreateCode(*db->Elements().GetRootSubject(), EXAMPLE_MODEL_NAME);
        ASSERT_TRUE(db->Models().QuerySubModelId(examplePartitionCode).IsValid()) << "Group seed file has special setup";
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
        DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(s_seedFileInfo.fileName);
        DgnCode physicalPartitionCode = PhysicalPartition::CreateCode(*db->Elements().GetRootSubject(), s_seedFileInfo.physicalPartitionName);
        DgnModelId defaultModelId = db->Models().QuerySubModelId(physicalPartitionCode);
        ASSERT_TRUE(defaultModelId.IsValid());

        DgnCode examplePartitionCode = PhysicalPartition::CreateCode(*db->Elements().GetRootSubject(), EXAMPLE_MODEL_NAME);
        ASSERT_TRUE(db->Models().QuerySubModelId(examplePartitionCode).IsValid()) << "Group seed file has special setup";

        PhysicalModelPtr model2 = DgnDbTestUtils::InsertPhysicalModel(*db, "Model2");
        ASSERT_TRUE(model2.IsValid());
        ASSERT_TRUE(model2->GetModelId() != defaultModelId);

        db->SaveChanges();
        }
            
    if (true)
        {
        //  Verify that we can work with a read-write copy of the root seed file
        DgnPlatformSeedManager::SeedDbInfo info = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, DgnPlatformSeedManager::SeedDbOptions(false, true));
        DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(info.fileName);

        DgnCode examplePartitionCode = PhysicalPartition::CreateCode(*db->Elements().GetRootSubject(), EXAMPLE_MODEL_NAME);
        ASSERT_FALSE(db->Models().QuerySubModelId(examplePartitionCode).IsValid()) << "Root seed file does not have this group's special setup";

        PhysicalModelPtr model2 = DgnDbTestUtils::InsertPhysicalModel(*db, "Model2");
        ASSERT_TRUE(model2.IsValid());
        db->SaveChanges();
        }

    if (true)
        {
        //  Verify that we can work with a read-write copy of the root seed file, where we assign a name
        DgnPlatformSeedManager::SeedDbInfo info = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, DgnPlatformSeedManager::SeedDbOptions(false, true));
        DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(info.fileName, L"Test2");

        PhysicalModelPtr model2 = DgnDbTestUtils::InsertPhysicalModel(*db, "Model2");
        ASSERT_TRUE(model2.IsValid());
        db->SaveChanges();
        }
    }
