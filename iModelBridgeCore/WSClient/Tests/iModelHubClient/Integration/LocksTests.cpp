/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/LocksTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE

static const Utf8CP s_iModelName = "LocksTests";

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocksTests : public iModelTestsBase
    {
    iModelInfoPtr               m_info;

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

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void CreateTestiModel()
        {
        DgnDbPtr db = CreateTestDb();
        iModelResult result = IntegrationTestsBase::CreateiModel(db);
        ASSERT_SUCCESS(result);
        m_info = result.GetValue();
        }
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, QueryLocksTest)
    {
    //Prapare imodel and acquire briefcases
    BriefcasePtr briefcase1 = AcquireAndOpenBriefcase();
    BriefcasePtr briefcase2 = AcquireAndOpenBriefcase();

     iModelHubHelpers::ExpectLocksCount(briefcase1, 0);
     iModelHubHelpers::ExpectLocksCount(briefcase2, 0);

    //Create two models in different briefcases. This should also acquire locks automatically.
    PhysicalModelPtr model1 = CreateModel(TestCodeName().c_str(), briefcase1->GetDgnDb());
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false));
    PhysicalModelPtr model2 = CreateModel(TestCodeName(1).c_str(), briefcase2->GetDgnDb());
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, true, false));

     iModelHubHelpers::ExpectLocksCount(briefcase1, 4);
     iModelHubHelpers::ExpectLocksCount(briefcase2, 4);

    //Check if we can access locks by Id
    iModelHubHelpers::ExpectLocksCountById(briefcase1, 2, true, LockableId(*model1), LockableId(model1->GetDgnDb()));
    iModelHubHelpers::ExpectLocksCountById(briefcase1, 2, true, LockableId(*model1), LockableId(*model2), LockableId(model1->GetDgnDb()));
    iModelHubHelpers::ExpectLocksCountById(briefcase1, 3, false, LockableId(*model1), LockableId(model1->GetDgnDb()));
    iModelHubHelpers::ExpectLocksCountById(briefcase1, 4, false, LockableId(*model1), LockableId(*model2), LockableId(model1->GetDgnDb()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Dziedzelis             06/2016
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, QueryUnavailableLocksTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireAndOpenBriefcase();
    auto briefcase2 = AcquireAndOpenBriefcase();

    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase1, 0);
    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase2, 0);

    //Create a model and push it.
    auto model1 = CreateModel(TestCodeName().c_str(), briefcase1->GetDgnDb());
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false));

    //Both model and file locks should be unavailable to the second briefcase.
    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase2, 4);

    //After releasing model lock, it should still be unavailable until merge.
    EXPECT_EQ(RepositoryStatus::Success, DemoteLock(*model1));
    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase2, 4);

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, false, false));
    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase2, 2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Dziedzelis             07/2016
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, QueryAvailableLocksTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireAndOpenBriefcase();
    auto briefcase2 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase1, 0);
    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase2, 0);

    //Create a model and push it.
    auto model1 = CreateModel(TestCodeName().c_str(), briefcase1->GetDgnDb());
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false));

    //Locks are available for the briefcase that owns them
    IBriefcaseManager::Request req(IBriefcaseManager::ResponseOptions::None);
    IBriefcaseManager::Response response(IBriefcaseManager::RequestPurpose::Query, req.Options());
    req.Locks().GetLockSet().insert(DgnLock(LockableId(briefcase1->GetDgnDb()), LockLevel::Shared));
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Exclusive));
    EXPECT_TRUE(db1.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Demote should be possible
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Shared));
    EXPECT_TRUE(db1.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Locks are unavailable without pull
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Shared));
    EXPECT_FALSE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Shared locks are available for shared acquire
    EXPECT_SUCCESS(briefcase2->PullAndMerge()->GetResult());
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(briefcase1->GetDgnDb()), LockLevel::Shared));
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Shared locks are unavailable for exclusive acquire
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(briefcase1->GetDgnDb()), LockLevel::Exclusive));
    EXPECT_FALSE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Exclusive locks are unavailable for both exclusvie and shared acquire
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Exclusive));
    EXPECT_FALSE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Shared));
    EXPECT_FALSE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Promotion should be possible
    DemoteLock(*model1, LockLevel::Shared);
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Exclusive));
    EXPECT_TRUE(db1.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));


    //Released lock should be available for both exclusive and shared acquire
    DemoteLock(*model1);
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Exclusive));
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Shared));
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite        03/2017
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, LocksStatesResponseTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireAndOpenBriefcase();
    auto briefcase2 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase1, 0);
    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase2, 0);

    //Create a models and push them.
    auto model1 = CreateModel(TestCodeName().c_str(), briefcase1->GetDgnDb());
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false));

    auto model2 = CreateModel(TestCodeName(1).c_str(), briefcase1->GetDgnDb());
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false));

    LockRequest req;
    req.Insert(*model1, LockLevel::Exclusive);
    req.Insert(*model2, LockLevel::Shared);
    auto response = db1.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::LockState);
    EXPECT_EQ(RepositoryStatus::Success, response.Result());

    //chek if LockAlreadyHeld error returns locks correctly
    req.Clear();
    req.Insert(*model1, LockLevel::Exclusive);
    response = db2.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::LockState);
    EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());
    EXPECT_EQ(1, response.LockStates().size());
    auto lock = *response.LockStates().begin();
    EXPECT_EQ(LockLevel::Exclusive, lock.GetOwnership().GetLockLevel());
    EXPECT_EQ(LockableId(*model1), lock.GetLockableId());

    //check if all conflicting locks are returned
    req.Clear();
    req.Insert(*model1, LockLevel::Exclusive);
    req.Insert(*model2, LockLevel::Exclusive);
    response = db2.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::LockState);
    EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());
    EXPECT_EQ(2, response.LockStates().size());

    //make sure that no locks are returned then ResponseOptions is None
    response = db2.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::None);
    EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());
    EXPECT_EQ(0, response.LockStates().size());

    EXPECT_EQ(RepositoryStatus::Success, briefcase1->GetDgnDb().BriefcaseManager().RelinquishLocks());

    //check if ChangeSetRequired error returns locks correctly
    req.Clear();
    req.Insert(*model1, LockLevel::Exclusive);
    response = db2.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::LockState);
    EXPECT_EQ(RepositoryStatus::RevisionRequired, response.Result());
    EXPECT_EQ(1, response.LockStates().size());
    lock = *response.LockStates().begin();
    EXPECT_EQ(LockableId(*model1), lock.GetLockableId());

    //make sure that no locks are returned then ResponseOptions is None
    response = db2.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::None);
    EXPECT_EQ(RepositoryStatus::RevisionRequired, response.Result());
    EXPECT_EQ(0, response.LockStates().size());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, ReleasedWithChangeSetLocksTest)
    {
    //Prapare imodel and acquire a briefcase
    auto briefcase = AcquireAndOpenBriefcase();
    Utf8String originalChangeSetId = briefcase->GetLastChangeSetPulled();

    //Create a model, this will automatically acquire neccessary locks
    auto model = CreateModel(TestCodeName().c_str(), briefcase->GetDgnDb());
     iModelHubHelpers::ExpectLocksCount(briefcase, 2);
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false));
     iModelHubHelpers::ExpectLocksCount(briefcase, 4);
    Utf8String lastChangeSetId = briefcase->GetLastChangeSetPulled();

    //Release model lock
    EXPECT_EQ(RepositoryStatus::Success, DemoteLock(*model));
     iModelHubHelpers::ExpectLocksCount(briefcase, 2);

    //Release all locks
    EXPECT_EQ(RepositoryStatus::Success, briefcase->GetDgnDb().BriefcaseManager().RelinquishLocks());
     iModelHubHelpers::ExpectLocksCount(briefcase, 0);

    //We should not be able to acquire locks if we haven't pulled a changeSet.
    iModelHubHelpers::SetLastPulledChangeSetId(briefcase, originalChangeSetId);
    EXPECT_EQ(RepositoryStatus::RevisionRequired, AcquireLock(*model, LockLevel::Exclusive));
    EXPECT_EQ(RepositoryStatus::RevisionRequired, AcquireLock(*model, LockLevel::Shared));
     iModelHubHelpers::ExpectLocksCount(briefcase, 0);

    //We should be able to acquire db lock because it shared locks should not be marked with ChangeSetId during push.
    EXPECT_EQ(RepositoryStatus::Success, AcquireLock(briefcase->GetDgnDb()));
     iModelHubHelpers::ExpectLocksCount(briefcase, 1);
    EXPECT_EQ(RepositoryStatus::Success, briefcase->GetDgnDb().BriefcaseManager().RelinquishLocks());
     iModelHubHelpers::ExpectLocksCount(briefcase, 0);

    //We should not be able to acquire locks if changeSet id is invalid.
    iModelHubHelpers::SetLastPulledChangeSetId(briefcase, "BadChangeSetId");
    EXPECT_EQ(RepositoryStatus::InvalidRequest, AcquireLock(*model, LockLevel::Exclusive));
     iModelHubHelpers::ExpectLocksCount(briefcase, 0);

    //We should be able to acquire locks if changeSet id is set correctly.
    iModelHubHelpers::SetLastPulledChangeSetId(briefcase, lastChangeSetId);
    EXPECT_EQ(RepositoryStatus::Success, AcquireLock(*model, LockLevel::Exclusive));
     iModelHubHelpers::ExpectLocksCount(briefcase, 2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, ShouldNotBeAbleDeletingModelLockedByAnotherBriefcase)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireAndOpenBriefcase();
    auto briefcase2 = AcquireAndOpenBriefcase();

     iModelHubHelpers::ExpectLocksCount(briefcase1, 0);
     iModelHubHelpers::ExpectLocksCount(briefcase2, 0);

    //Create two models in different briefcases. This should also acquire locks automatically.
    auto model1 = CreateModel(TestCodeName(1).c_str(), briefcase1->GetDgnDb());
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false));
    auto model2 = CreateModel(TestCodeName(2).c_str(), briefcase2->GetDgnDb());
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, true, false));

    iModelHubHelpers::ExpectLocksCount(briefcase1, 4);
    iModelHubHelpers::ExpectLocksCount(briefcase2, 4);

    //Try delete Model1 from briefcase2
    DgnModelPtr model1_2 = briefcase2->GetDgnDb().Models().GetModel(model1->GetModelId());
    EXPECT_NE(DgnDbStatus::Success, model1_2->Delete());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Gintare.Grazulyte                12/2017
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, FailingLocksResponseOptions)
    {
    CreateTestiModel();
    BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, m_info);
    ASSERT_SUCCESS(briefcaseResult);
    BriefcasePtr briefcase1 = briefcaseResult.GetValue();
    briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, m_info);
    ASSERT_SUCCESS(briefcaseResult);
    BriefcasePtr briefcase2 = briefcaseResult.GetValue();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

     iModelHubHelpers::ExpectLocksCount(briefcase1, 0);
     iModelHubHelpers::ExpectLocksCount(briefcase2, 0);

    DgnModelPtr model1_1 = CreateModel(TestCodeName().c_str(), db1);
    DgnModelPtr model1_2 = CreateModel(TestCodeName(1).c_str(), db1);
     iModelHubHelpers::ExpectLocksCount(briefcase1, 2);
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false));
     iModelHubHelpers::ExpectLocksCount(briefcase1, 6);

    EXPECT_EQ(DgnDbStatus::Success, model1_1->Delete());
    EXPECT_EQ(DgnDbStatus::Success, model1_2->Delete());
    db1.SaveChanges();

    ASSERT_SUCCESS(briefcase2->PullAndMerge()->GetResult());
    DgnModelPtr model2_1 = db2.Models().GetModel(model1_1->GetModelId());

    DgnLockSet locks;
    locks.insert(DgnLock(LockableId(model2_1->GetModelId()), LockLevel::None));
    DgnCodeSet codes;
    StatusResult result = briefcase2->GetiModelConnection().DemoteCodesLocks(locks, codes, briefcase1->GetBriefcaseId(), db2.GetDbGuid())->GetResult();
    ASSERT_SUCCESS(result);
	
    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::Success, db2.BriefcaseManager().PrepareForModelDelete(req, *model2_1, IBriefcaseManager::PrepareAction::Acquire)); 
    EXPECT_EQ(DgnDbStatus::Success, model2_1->Delete());
    db2.SaveChanges();
     iModelHubHelpers::ExpectLocksCount(briefcase1, 5);
     iModelHubHelpers::ExpectLocksCount(briefcase2, 2);

    StatusResult result1 = briefcase1->Push(nullptr, false, nullptr, IBriefcaseManager::ResponseOptions::All)->GetResult();
    ASSERT_FAILURE(result1);
    EXPECT_EQ(Error::Id::LockOwnedByAnotherBriefcase, result1.GetError().GetId());
    JsonValueCR error1 = result1.GetError().GetExtendedData();
    EXPECT_EQ(1, error1["ConflictingLocks"].size());

    StatusResult result2 = briefcase1->Push(nullptr, false, nullptr, IBriefcaseManager::ResponseOptions::None)->GetResult();
    ASSERT_FAILURE(result2);
    EXPECT_EQ(Error::Id::LockOwnedByAnotherBriefcase, result1.GetError().GetId());
    JsonValueCR error2 = result2.GetError().GetExtendedData();
    EXPECT_EQ(Json::Value::GetNull(), error2["ConflictingLocks"]);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Andrius.Zonys                   09/2016
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, RelinquishOtherUserLocks)
    {
    CreateTestiModel();
    ClientPtr nonAdminClient = CreateNonAdminClient();

    // Prapare imodel and acquire briefcases.
    BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(nonAdminClient, m_info);
    ASSERT_SUCCESS(briefcaseResult);
    auto briefcase1 = briefcaseResult.GetValue();
    briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, m_info);
    ASSERT_SUCCESS(briefcaseResult);
    auto briefcase2 = briefcaseResult.GetValue();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    // Briefcase1 creates three models. This should also acquire locks automatically.
    iModelHubHost::Instance().SetRepositoryAdmin(nonAdminClient->GetiModelAdmin());
    auto model1 = CreateModel(TestCodeName(1).c_str(), db1);
    auto model2 = CreateModel(TestCodeName(2).c_str(), db1);
    auto model3 = CreateModel(TestCodeName(3).c_str(), db1);
    iModelHubHost::Instance().SetRepositoryAdmin(s_client->GetiModelAdmin());
    db1.SaveChanges();
    iModelHubHelpers::ExpectLocksCount(briefcase1, 2);

    // Briefcase2 relinquishes other briefcase codes and locks.
    EXPECT_SUCCESS(briefcase2->GetiModelConnection().RelinquishCodesLocks(briefcase1->GetBriefcaseId())->GetResult());
    iModelHubHelpers::ExpectLocksCount(briefcase1, 0);

    // Briefcase1 should be able to push changes since nobody owns them.
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false));
    Utf8String changeSet1 = briefcase1->GetLastChangeSetPulled();
     iModelHubHelpers::ExpectLocksCount(briefcase1, 8);

    // Briefcase1 deletes two models.
    EXPECT_EQ(DgnDbStatus::Success, model1->Delete());
    EXPECT_EQ(DgnDbStatus::Success, model2->Delete());
    db1.SaveChanges();

    // Briefcase2 relinquishes specific locks and acquires its own lock.
    DgnLockSet locks;
    locks.insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::None));
    locks.insert(DgnLock(LockableId(model3->GetModelId()), LockLevel::None));
    DgnCodeSet codes;
    auto result = briefcase2->GetiModelConnection().DemoteCodesLocks(locks, codes, briefcase1->GetBriefcaseId(), db2.GetDbGuid())->GetResult();
    EXPECT_SUCCESS(result);

    EXPECT_SUCCESS(briefcase2->PullAndMerge()->GetResult());
    DgnModelPtr model1_2 = briefcase2->GetDgnDb().Models().GetModel(model1->GetModelId());
    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::Success, db2.BriefcaseManager().PrepareForModelDelete(req, *model1_2, IBriefcaseManager::PrepareAction::Acquire));
    EXPECT_EQ(DgnDbStatus::Success, model1_2->Delete());
    db2.SaveChanges();
     iModelHubHelpers::ExpectLocksCount(briefcase1, 6);
     iModelHubHelpers::ExpectLocksCount(briefcase2, 2);

    // Briefcase1 should not be able to push his changes since one lock is owned.
    iModelHubHost::Instance().SetRepositoryAdmin(nonAdminClient->GetiModelAdmin());
    auto pushResult = briefcase1->PullMergeAndPush()->GetResult();
    EXPECT_EQ(Error::Id::LockOwnedByAnotherBriefcase, pushResult.GetError().GetId());
     iModelHubHelpers::ExpectLocksCount(briefcase1, 6);
     iModelHubHelpers::ExpectLocksCount(briefcase2, 2);

    // Briefcase1 should not be able to release all other briefcase locks.
    result = briefcase1->GetiModelConnection().RelinquishCodesLocks(briefcase2->GetBriefcaseId())->GetResult();
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase1 should not be able to release specific other briefcase locks.
    result = briefcase1->GetiModelConnection().DemoteCodesLocks(locks, codes, briefcase2->GetBriefcaseId(), db1.GetDbGuid())->GetResult();
    iModelHubHost::Instance().SetRepositoryAdmin(s_client->GetiModelAdmin());
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase2 should be able to push changes but he will need to wait.
    pushResult = briefcase2->PullMergeAndPush()->GetResult();
    EXPECT_EQ(Error::Id::AnotherUserPushing, pushResult.GetError().GetId());

    iModelHubHelpers::ExpectLocksCountById(briefcase1, 2, false, LockableId(*model3), LockableId(model1->GetDgnDb()));
    iModelHubHelpers::ExpectLocksCountById(briefcase1, 4, false, LockableId(*model1), LockableId(*model2), LockableId(model1->GetDgnDb()));
    }


