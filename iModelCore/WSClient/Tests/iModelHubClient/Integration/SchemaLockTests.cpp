/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/SchemaLockTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"
#include "../Helpers/Domains/DgnPlatformTestDomain.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DPTEST

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaLockTests : public iModelTestsBase
    {
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();
        }
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaLockTests, LockSchemas)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireAndOpenBriefcase();
    auto briefcase2 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase1, 0);
    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase2, 0);

    // Lock schemas with the first briefcase
    auto response = db1.BriefcaseManager().LockSchemas().Result();
    EXPECT_EQ(RepositoryStatus::Success, response);

    // Second briefcase should not be able to lock schemas
    response = db2.BriefcaseManager().LockSchemas().Result();
    EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response);

    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase1, 0);
    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase2, 2);
    
    // First briefcase releases locks
    EXPECT_EQ(RepositoryStatus::Success, db1.BriefcaseManager().RelinquishLocks());

    // Now second briefcase sould be able to lock schemas
    response = db2.BriefcaseManager().LockSchemas().Result();
    EXPECT_EQ(RepositoryStatus::Success, response);

    IBriefcaseManager::Request req(IBriefcaseManager::ResponseOptions::LockState);
    IBriefcaseManager::Response resp(IBriefcaseManager::RequestPurpose::Query, req.Options());
    req.Locks().GetLockSet().insert(DgnLock(LockableId(LockableType::Schemas, BeInt64Id(1)), LockLevel::Exclusive));
    EXPECT_FALSE(db1.BriefcaseManager().AreResourcesAvailable(req, &resp, IBriefcaseManager::FastQuery::No));
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &resp, IBriefcaseManager::FastQuery::No));

    // Second briefcase releases all locks
    EXPECT_EQ(RepositoryStatus::Success, db2.BriefcaseManager().RelinquishLocks());

    // Locking schemas with id != 1 should not be available
    req.Locks().Clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(LockableType::Schemas, BeInt64Id(2)), LockLevel::Exclusive));
    EXPECT_FALSE(db1.BriefcaseManager().AreResourcesAvailable(req, &resp, IBriefcaseManager::FastQuery::No));
    EXPECT_FALSE(db2.BriefcaseManager().AreResourcesAvailable(req, &resp, IBriefcaseManager::FastQuery::No));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaLockTests, ModifySchema)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireAndOpenBriefcase();
    auto briefcase2 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    // Before changing schemas briefcase has to get Schemas lock
    EXPECT_EQ(RepositoryStatus::Success, db1.BriefcaseManager().LockSchemas().Result());

    // Second briefcase should not be able to get lock
    EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, db2.BriefcaseManager().LockSchemas().Result());

    // Modify schema
    db1.CreateTable("TestTable1", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    ASSERT_TRUE(db1.Txns().HasDbSchemaChanges());
    db1.SaveChanges("ChangeSet 1");
    ASSERT_FALSE(db1.Txns().HasDbSchemaChanges());

    // Push changeSet with schema changes
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false));
    Utf8String changeSet1 = briefcase1->GetLastChangeSetPulled();
    auto changeSetResult = briefcase1->GetiModelConnection().GetChangeSetById(changeSet1)->GetResult();
    ASSERT_SUCCESS(changeSetResult);
    ASSERT_EQ(1, changeSetResult.GetValue()->GetContainingChanges());

    // Second briefcase does not have pushed changeset, so it should not be able to get lock
    EXPECT_EQ(RepositoryStatus::RevisionRequired, db2.BriefcaseManager().LockSchemas().Result());

    auto model4 = CreateModel(TestCodeName().c_str(), db2);
    ASSERT_FALSE(db2.TableExists("TestTable1"));

    // Push changeSet without schema changes should fail
    auto pushResult = briefcase2->PullMergeAndPush(nullptr, true)->GetResult();
    EXPECT_EQ(Error::Id::MergeSchemaChangesOnOpen, pushResult.GetError().GetId());
    
    // Get changesets to merge
    TestsProgressCallback callback;
    ChangeSetsResult changeSetsResult = briefcase2->GetiModelConnection().DownloadChangeSetsAfterId(briefcase2->GetLastChangeSetPulled(), briefcase2->GetDgnDb().GetDbGuid(), callback.Get())->GetResult();
    ASSERT_SUCCESS(changeSetsResult);
    callback.Verify();
    ChangeSets changeSets = changeSetsResult.GetValue();
    auto filePath = db2.GetFileName();
    db2.CloseDb();

    // Reload db with changesets
    BeSQLite::DbResult status;
    auto db2Ptr = s_client->OpenWithSchemaUpgrade(&status, filePath, changeSets);
    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, status);
    
    auto briefcase2Result = s_client->OpenBriefcase(db2Ptr, false)->GetResult();
    ASSERT_SUCCESS(briefcase2Result);
    briefcase2 = briefcase2Result.GetValue();
    
    // Try to push again
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, true, false));
    Utf8String changeSet2 = briefcase2->GetLastChangeSetPulled();

    // Check ContainsSchemaChanges set to false
    changeSetResult = briefcase2->GetiModelConnection().GetChangeSetById(changeSet2)->GetResult();
    ASSERT_TRUE(changeSetResult.IsSuccess());
    EXPECT_EQ(0, changeSetResult.GetValue()->GetContainingChanges());
    EXPECT_TRUE(db2Ptr->TableExists("TestTable1"));

    // Second briefcase should be able to get lock
    EXPECT_EQ(RepositoryStatus::Success, db2Ptr->BriefcaseManager().LockSchemas().Result());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             07/2018
//---------------------------------------------------------------------------------------
TEST_F(SchemaLockTests, ImportSchemaAcquiresLock)
    {
    auto briefcase1 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    auto briefcase2 = AcquireAndOpenBriefcase();
    DgnDbR db2 = briefcase2->GetDgnDb();

    // Import domain and schema, schema lock should be acquired automatically
    DgnDomains::RegisterDomain(DgnPlatformTestDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    EXPECT_EQ(SchemaStatus::Success, DgnPlatformTestDomain::GetDomain().ImportSchema(db1));
    db1.SaveChanges();

    // Second briefcase should fail to get lock
    EXPECT_EQ(SchemaStatus::SchemaLockFailed, DgnPlatformTestDomain::GetDomain().ImportSchema(db2));

    // Push changes
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false));

    // Second briefcase should fail to get lock since it does not have changeset
    EXPECT_EQ(SchemaStatus::SchemaLockFailed, DgnPlatformTestDomain::GetDomain().ImportSchema(db2));
    }
