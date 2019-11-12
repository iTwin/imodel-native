/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"
#include "../Helpers/Domains/DgnPlatformTestDomain.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DPTEST

static const Utf8CP s_iModelName = "LocksTests";

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocksTests : public iModelTestsBase 
#ifdef NO_IMPLEMENTATION_IN_IMODEL02_PLATFORM
    , TestElementDrivesElementHandler::Callback 
#endif
    {
    iModelInfoPtr   m_info;
    DgnElementCPtr  m_onRootChangedElement = nullptr;
    bool            m_indirectElementUpdated = false;

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
        iModelResult result = IntegrationTestsBase::CreateEmptyiModel(GetTestiModelName());
        ASSERT_SUCCESS(result);
        m_info = result.GetValue();
        }
#ifdef NO_IMPLEMENTATION_IN_IMODEL02_PLATFORM
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Diego.Pinate    04/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _OnRootChanged(DgnDbR db, ECInstanceId relationshipId, DgnElementId source, DgnElementId target) override
        {
        if (m_onRootChangedElement.IsNull())
            return;

        // Update an element to test dependency changes don't generate locks in ExtractLocks call
        EXPECT_TRUE(m_onRootChangedElement.IsValid());
        DgnElementPtr elementEdit = db.Elements().GetForEdit<DgnElement>(m_onRootChangedElement->GetElementId());

        elementEdit->SetUserLabel("Updated label OnRootChanged");
        EXPECT_TRUE(elementEdit->Update().IsValid());
        m_indirectElementUpdated = true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Diego.Pinate    04/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _ProcessDeletedDependency(DgnDbR db, dgn_TxnTable::ElementDep::DepRelData const& relData) override
        {
        }
#endif
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, QueryLocksTest)
    {
    //Prapare imodel and acquire briefcases
    BriefcasePtr briefcase1 = AcquireAndOpenBriefcase();
    BriefcasePtr briefcase2 = AcquireAndOpenBriefcase();

    briefcase1->GetiModelConnectionPtr()->SetCodesLocksPageSize(1);
    briefcase2->GetiModelConnectionPtr()->SetCodesLocksPageSize(3);

    iModelHubHelpers::ExpectLocksCount(briefcase1, 0);
    iModelHubHelpers::ExpectLocksCount(briefcase2, 0);

    //Create two models in different briefcases. This should also acquire locks automatically.
    PhysicalModelPtr model1 = CreateModel(TestCodeName().c_str(), briefcase1->GetDgnDb());
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));
    PhysicalModelPtr model2 = CreateModel(TestCodeName(1).c_str(), briefcase2->GetDgnDb());
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, true, true, false));

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
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));

    //Both model and file locks should be unavailable to the second briefcase.
    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase2, 4);

    //After releasing model lock, it should still be unavailable until merge.
    EXPECT_EQ(RepositoryStatus::Success, DemoteLock(*model1));
    iModelHubHelpers::ExpectUnavailableLocksCount(briefcase2, 4);

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, false, true, false));
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
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));

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
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));

    auto model2 = CreateModel(TestCodeName(1).c_str(), briefcase1->GetDgnDb());
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));

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
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));
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
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));
    auto model2 = CreateModel(TestCodeName(2).c_str(), briefcase2->GetDgnDb());
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, true, true, false));

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
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));
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

    ConflictsInfoPtr conflictsInfo = std::make_shared<ConflictsInfo>();
    PushChangeSetArgumentsPtr pushChangeSetArguments = PushChangeSetArguments::Create(nullptr, ChangeSetInfo::ContainingChanges::NotSpecified,
        nullptr, false, nullptr, IBriefcaseManager::ResponseOptions::All, nullptr, conflictsInfo);
    StatusResult result1 = briefcase1->Push(pushChangeSetArguments)->GetResult();
    ASSERT_SUCCESS(result1);
    EXPECT_TRUE(conflictsInfo->Any());
    EXPECT_EQ(1, conflictsInfo->GetLocksConflicts().size());
    
    LockRequest lockRequest;
    lockRequest.Insert(*model2_1, LockLevel::Exclusive);
    db1.BriefcaseManager().ClearUserHeldCodesLocks();
    IBriefcaseManager::Response result2 = db1.BriefcaseManager().AcquireLocks(lockRequest, IBriefcaseManager::ResponseOptions::None);
    ASSERT_EQ(RepositoryStatus::LockAlreadyHeld, result2.Result());
    EXPECT_TRUE(result2.LockStates().empty());
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
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));
    Utf8String changeSet1 = briefcase1->GetLastChangeSetPulled();
     iModelHubHelpers::ExpectLocksCount(briefcase1, 6);

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
    iModelHubHelpers::ExpectLocksCount(briefcase1, 4);
    iModelHubHelpers::ExpectLocksCount(briefcase2, 2);

    // Briefcase1 should be able to push his changes, since conflict should be returned.
    iModelHubHost::Instance().SetRepositoryAdmin(nonAdminClient->GetiModelAdmin());
    ConflictsInfoPtr conflictsInfo = std::make_shared<ConflictsInfo>();
    auto pushResult = briefcase1->PullMergeAndPush(nullptr, false, nullptr, nullptr, nullptr, 1, conflictsInfo)->GetResult();
    ASSERT_SUCCESS(pushResult);
    EXPECT_TRUE(conflictsInfo->Any());
    EXPECT_EQ(1, conflictsInfo->GetLocksConflicts().size());

    iModelHubHelpers::ExpectLocksCount(briefcase1, 5);
    iModelHubHelpers::ExpectLocksCount(briefcase2, 2);

    // Briefcase1 should not be able to release all other briefcase locks.
    result = briefcase1->GetiModelConnection().RelinquishCodesLocks(briefcase2->GetBriefcaseId())->GetResult();
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase1 should not be able to release specific other briefcase locks.
    result = briefcase1->GetiModelConnection().DemoteCodesLocks(locks, codes, briefcase2->GetBriefcaseId(), db1.GetDbGuid())->GetResult();
    iModelHubHost::Instance().SetRepositoryAdmin(s_client->GetiModelAdmin());
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase2 can push its changes.
    pushResult = briefcase2->PullMergeAndPush()->GetResult();
    EXPECT_SUCCESS(pushResult);

    iModelHubHelpers::ExpectLocksCountById(briefcase1, 2, false, LockableId(*model3), LockableId(model1->GetDgnDb()));
    iModelHubHelpers::ExpectLocksCountById(briefcase1, 3, false, LockableId(*model1), LockableId(*model2), LockableId(model1->GetDgnDb()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas           12/2017
//---------------------------------------------------------------------------------------
void VerifyLocalAndServerLocks(BriefcasePtr briefcase, LockableId lock, LockLevel lockLevel, bool expectLocalLock, bool expectLockInServer)
    {
    LockableIdSet locks;
    locks.insert(lock);

    iModelHubHelpers::ExpectLocksCountByLevelAndId(briefcase, expectLockInServer ? 1 : 0, false, locks, lockLevel);
    iModelHubHelpers::ExpectLocksCountByLevelAndId(briefcase, expectLockInServer ? 1 : 0, true, locks, lockLevel);

    DgnDbR dgndb = briefcase->GetDgnDb();
    DgnLockSet locksSet;

    if (expectLocalLock)
        {
        locksSet.insert(DgnLock(lock, lockLevel));
        EXPECT_TRUE(dgndb.BriefcaseManager().AreLocksHeld(locksSet));

        if (lockLevel == LockLevel::Shared)
            {
            locksSet.clear();
            locksSet.insert(DgnLock(lock, LockLevel::Exclusive));
            EXPECT_FALSE(dgndb.BriefcaseManager().AreLocksHeld(locksSet));
            }
        }
    else
        {
        locksSet.insert(DgnLock(lock, LockLevel::Exclusive));
        EXPECT_FALSE(dgndb.BriefcaseManager().AreLocksHeld(locksSet));

        locksSet.clear();
        locksSet.insert(DgnLock(lock, LockLevel::Shared));
        EXPECT_FALSE(dgndb.BriefcaseManager().AreLocksHeld(locksSet));
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas           12/2017
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, WorkflowCreateNewElementReleaseLocksOnPush)
    {
    auto briefcase = AcquireAndOpenBriefcase();

    auto createdModel = CreateModel(TestCodeName().c_str(), briefcase->GetDgnDb());
    briefcase->GetDgnDb().SaveChanges();

    // After creation user should have local lock, but it should not be send to server
    auto lockDbId = LockableId(briefcase->GetDgnDb());
    auto lockModelId = LockableId(createdModel->GetModelId());
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, true, false);
    
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, true));

    // After push with release locks, no locks should be held
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, false, false);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, false, false);

    auto newElement = CreateElement(*createdModel);
    briefcase->GetDgnDb().SaveChanges();
    auto lockElementId = LockableId(newElement->GetElementId());

    // After new element is created user should get shared lock for model and exclusive for new element
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, true, false);

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, true));

    // After push no locks should be left
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, false, false);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Shared, false, false);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Shared, false, false);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas           12/2017
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, WorkflowCreateNewElementDoNotReleaseLocks)
    {
    auto briefcase = AcquireAndOpenBriefcase();

    auto createdModel = CreateModel(TestCodeName().c_str(), briefcase->GetDgnDb());
    briefcase->GetDgnDb().SaveChanges();

    // After creation user should have local lock, but it should not be send to server
    auto lockDbId = LockableId(briefcase->GetDgnDb());
    auto lockModelId = LockableId(createdModel->GetModelId());
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, true, false);

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));
    
    // Locks should be set after push
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, true, true);

    // Release model lock
    DgnLockSet lockSet;
    lockSet.insert(DgnLock(lockModelId, LockLevel::None));
    lockSet.insert(DgnLock(lockDbId, LockLevel::None));
    EXPECT_EQ(RepositoryStatus::Success, briefcase->GetDgnDb().BriefcaseManager().DemoteLocks(lockSet));
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, false, false);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Shared, false, false);

    // New element creation should acquire locks
    auto newElement = CreateElement(*createdModel);
    briefcase->GetDgnDb().SaveChanges();
    auto lockElementId = LockableId(newElement->GetElementId());

    // After new element is created user should get shared lock for model and exclusive for new element
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, true, false);

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));

    // After push locks should be set
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, true, true);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas           12/2017
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, WorkflowCreateNewElementModelExclusivelyLocked)
    {
    auto briefcase = AcquireAndOpenBriefcase();

    auto createdModel = CreateModel(TestCodeName().c_str(), briefcase->GetDgnDb());
    briefcase->GetDgnDb().SaveChanges();

    // After creation user should have local lock, but it should not be sent to server
    auto lockDbId = LockableId(briefcase->GetDgnDb());
    auto lockModelId = LockableId(createdModel->GetModelId());
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, true, false);

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, true));

    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, false, false);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, false, false);

    // Lock model exclusively
    LockRequest lockRequest;
    lockRequest.Insert(*createdModel, LockLevel::Exclusive);
    EXPECT_EQ(RepositoryStatus::Success, briefcase->GetDgnDb().BriefcaseManager().AcquireLocks(lockRequest).Result());
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, true, true);

    // New element creation should not acquire locks in the server
    auto newElement = CreateElement(*createdModel);
    briefcase->GetDgnDb().SaveChanges();
    auto lockElementId = LockableId(newElement->GetElementId());

    // After new element is created user only exclusive model lock should exist in the server
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, true, true);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, true, false);

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));

    // After new element is created user only exclusive model lock should exist in the server
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, true, true);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, true, false); // Lock should not be sent to the server, since user has model locked exclusively

    // Demote lock of model - should demote lock of element
    DgnLockSet lockSet;
    lockSet.insert(DgnLock(lockDbId, LockLevel::None));
    lockSet.insert(DgnLock(lockModelId, LockLevel::None));
    EXPECT_EQ(RepositoryStatus::Success, briefcase->GetDgnDb().BriefcaseManager().DemoteLocks(lockSet));
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, false, false);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Shared, false, false);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, false, false);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas           12/2017
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, WorkflowUpdateElement)
    {
    auto briefcase = AcquireAndOpenBriefcase();
    DgnDbR db = briefcase->GetDgnDb();

    auto createdModel = CreateModel(TestCodeName().c_str(), db);
    auto newElement = CreateElement(*createdModel);
    db.SaveChanges();

    // After creation user should have local lock, but it should not be send to server
    auto lockDbId = LockableId(briefcase->GetDgnDb());
    auto lockModelId = LockableId(createdModel->GetModelId());
    auto lockElementId = LockableId(newElement->GetElementId());
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, true, false);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, true, false);

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, true));

    // After push there should be no locks left
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, false, false);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, false, false);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, false, false);

    // Prepare for element update
    IBriefcaseManager::Request req;
    DgnElementPtr existingElement = briefcase->GetDgnDb().Elements().GetForEdit<PhysicalElement>(newElement->GetElementId());
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().PrepareForElementUpdate(req, *existingElement, IBriefcaseManager::PrepareAction::Acquire));
    existingElement->SetUserLabel("New label");
    existingElement->Update();
    briefcase->GetDgnDb().SaveChanges();

    // User should have shared lock for model and exclusive for element in both server and locally
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, true, true);

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, true));

    // After push locks should be cleared
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, false, false);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Shared, false, false);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, false, false);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas           12/2017
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, WorkflowUpdateElementWithExclusiveModelLock)
    {
    auto briefcase = AcquireAndOpenBriefcase();
    DgnDbR db = briefcase->GetDgnDb();

    auto createdModel = CreateModel(TestCodeName().c_str(), db);
    auto newElement = CreateElement(*createdModel);
    db.SaveChanges();

    // After creation user should have local lock, but it should not be send to server
    auto lockDbId = LockableId(briefcase->GetDgnDb());
    auto lockModelId = LockableId(createdModel->GetModelId());
    auto lockElementId = LockableId(newElement->GetElementId());
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, true, false);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, true, false);

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, true));

    // After push there should be no locks left
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, false, false);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, false, false);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, false, false);

    // Lock model exclusively
    LockRequest lockRequest;
    lockRequest.Insert(*createdModel, LockLevel::Exclusive);
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().AcquireLocks(lockRequest).Result());
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, true, true);

    // Prepare for element update
    IBriefcaseManager::Request req;
    DgnElementPtr existingElement = briefcase->GetDgnDb().Elements().GetForEdit<PhysicalElement>(newElement->GetElementId());
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().PrepareForElementUpdate(req, *existingElement, IBriefcaseManager::PrepareAction::Acquire));
    existingElement->SetUserLabel("New label");
    existingElement->Update();
    db.SaveChanges();

    // User should have exclusive lock for model and only local lock for element
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, true, true);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, true, false);

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));

    // After push locks should not be cleared
    VerifyLocalAndServerLocks(briefcase, lockDbId, LockLevel::Shared, true, true);
    VerifyLocalAndServerLocks(briefcase, lockModelId, LockLevel::Exclusive, true, true);
    VerifyLocalAndServerLocks(briefcase, lockElementId, LockLevel::Exclusive, true, false);
    }

#ifdef NO_IMPLEMENTATION_IN_IMODEL02_PLATFORM
//---------------------------------------------------------------------------------------
// In Component Modeling we have a "definition" and "instances" of the definition. 
// A user should be able to edit the "definition" while another user moves or interacts with the placed instances.
// When the "definition" is updated, a handler for my specific ElementDrivesElement relationship updates all 
// "instances" so that they mirror what the "definition" contains now.
// "Definition" modification should not require DgnLocks for "instances".
// http://tfs.bentley.com/tfs/ProductLine/Platform%20Technology/_workitems/edit/883714
//@bsimethod                                     Algirdas.Mikoliunas           04/2018
//---------------------------------------------------------------------------------------
TEST_F(LocksTests, IndirectChangesShouldNotBeExtractedAsRequiredLocks)
    {
    CreateTestiModel();
    auto briefcase = AcquireAndOpenBriefcase();
    DgnDbR db = briefcase->GetDgnDb();

    // Import domain and schema
    DgnDomains::RegisterDomain(DgnPlatformTestDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    EXPECT_EQ(SchemaStatus::Success, DgnPlatformTestDomain::GetDomain().ImportSchema(db));

    auto createdModel = CreateModel(TestCodeName().c_str(), db);

    // Create two elements and insert the relationship between them
    DgnElementCPtr root = CreateElement(*createdModel);
    DgnElementCPtr dependent = CreateElement(*createdModel);
    TestElementDrivesElementHandler::SetCallback(this);
    TestElementDrivesElementHandler::Insert(db, root->GetElementId(), dependent->GetElementId());
    db.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, true));

    // Set the dependent as the element that we will update in the _OnRootChanged call
    m_onRootChangedElement = dependent;

    // Update element. db.SaveChanges() triggers _OnRootChanged()
    DgnElementPtr rootEdit = root->CopyForEdit();
    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().PrepareForElementUpdate(req, *rootEdit, IBriefcaseManager::PrepareAction::Acquire));
    rootEdit->SetUserLabel("New Root Label");
    DgnDbStatus status;
    EXPECT_TRUE(db.Elements().Update<DgnElement>(*rootEdit).IsValid());
    db.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));
    
    // Check lock for dependent element was not acquired
    EXPECT_TRUE(m_indirectElementUpdated);
    LockableIdSet locks;
    locks.insert(LockableId(*dependent));
    iModelHubHelpers::ExpectLocksCountById(briefcase, 0, false, locks);

    // Check lock for root element is acquired
    locks.clear();
    locks.insert(LockableId(*rootEdit));
    iModelHubHelpers::ExpectLocksCountByLevelAndId(briefcase, 1, true, locks, LockLevel::Exclusive);

    // Get rid of the static callback pointer
    TestElementDrivesElementHandler::SetCallback(nullptr);
    m_onRootChangedElement = nullptr;
    }
#endif