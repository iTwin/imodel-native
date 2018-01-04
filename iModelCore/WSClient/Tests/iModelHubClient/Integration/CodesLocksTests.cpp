/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/CodesLocksTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct CodesLocksTests : public iModelTestsBase
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
//@bsimethod                                    Andrius.Zonys                   08/2016
//---------------------------------------------------------------------------------------
TEST_F(CodesLocksTests, PushAndRelinquishCodesLocks)
    {
    //Prapare imodel and acquire briefcases
    BriefcasePtr briefcase1 = AcquireAndOpenBriefcase();
    BriefcasePtr briefcase2 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();
    auto imodelManager1 = IntegrationTestsBase::_GetRepositoryManager(db1);
    auto imodelManager2 = IntegrationTestsBase::_GetRepositoryManager(db2);

    iModelHubHelpers::ExpectCodesCount(briefcase1, 0);
    iModelHubHelpers::ExpectCodesCount(briefcase2, 0);
    iModelHubHelpers::ExpectLocksCount(briefcase1, 0);
    iModelHubHelpers::ExpectLocksCount(briefcase2, 0);

    //Create two models in different briefcases. This should also acquire codes and locks automatically.
    auto partition1_1 = CreateAndInsertModeledElement(TestCodeName(1).c_str(), db1);
    auto partition2_1 = CreateAndInsertModeledElement(TestCodeName(2).c_str(), db2);

    iModelHubHelpers::ExpectCodesCount(briefcase1, 1);
    iModelHubHelpers::ExpectCodesCount(briefcase2, 1);
    ExpectCodeState(CreateCodeReserved(partition1_1->GetCode(), db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(partition2_1->GetCode(), db2), imodelManager2);

    //Push changes.
    db1.SaveChanges();
    db2.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false)); // Don't release codes and locks.
    Utf8String changeSet1 = briefcase1->GetLastChangeSetPulled();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, true, false)); // Don't release codes and locks.
    Utf8String changeSet2 = briefcase2->GetLastChangeSetPulled();
    ExpectCodeState(CreateCodeUsed(partition1_1->GetCode(), changeSet1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(partition2_1->GetCode(), changeSet2), imodelManager2);
    iModelHubHelpers::ExpectLocksCount(briefcase1, 3);
    iModelHubHelpers::ExpectLocksCount(briefcase2, 3);

    //Create additional two models in different briefcases. This should also acquire codes and locks automatically.
    auto partition1_2 = CreateAndInsertModeledElement(TestCodeName(3).c_str(), db1);
    auto partition2_2 = CreateAndInsertModeledElement(TestCodeName(4).c_str(), db2);

    //Reserve two codes without actual model.
    DgnCode modelCode1_3 = MakeModelCode(TestCodeName(5).c_str(), db1);
    DgnCode modelCode2_3 = MakeModelCode(TestCodeName(6).c_str(), db1);
    DgnCodeSet codes;
    codes.insert(modelCode1_3);
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());
    codes.clear();
    codes.insert(modelCode2_3);
    EXPECT_STATUS(Success, db2.BriefcaseManager().ReserveCodes(codes).Result());

    iModelHubHelpers::ExpectCodesCount(briefcase1, 2);
    iModelHubHelpers::ExpectCodesCount(briefcase2, 2);
    ExpectCodeState(CreateCodeReserved(partition1_2->GetCode(), db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelCode1_3, db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(partition2_2->GetCode(), db2), imodelManager2);
    ExpectCodeState(CreateCodeReserved(modelCode2_3, db2), imodelManager2);

    //Delete two models and push changes.
    EXPECT_EQ(DgnDbStatus::Success, partition1_1->Delete());
    EXPECT_EQ(DgnDbStatus::Success, partition2_1->Delete());
    db1.SaveChanges();
    db2.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false)); // Don't release codes and locks.
    Utf8String changeSet3 = briefcase1->GetLastChangeSetPulled();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, true, true)); // Release all codes and locks.
    Utf8String changeSet4 = briefcase2->GetLastChangeSetPulled();
    iModelHubHelpers::ExpectCodesCount(briefcase1, 1);
    iModelHubHelpers::ExpectCodesCount(briefcase2, 0);
    ExpectNoCodeWithState(CreateCodeDiscarded(partition1_1->GetCode(), changeSet3), imodelManager1);
    ExpectCodeState(CreateCodeUsed(partition1_2->GetCode(), changeSet3), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelCode1_3, db1), imodelManager1);
    ExpectNoCodeWithState(CreateCodeDiscarded(partition2_1->GetCode(), changeSet4), imodelManager2);
    ExpectCodeState(CreateCodeUsed(partition2_2->GetCode(), changeSet4), imodelManager2);
    ExpectNoCodeWithState(CreateCodeAvailable(modelCode2_3), imodelManager2);
    iModelHubHelpers::ExpectLocksCount(briefcase1, 4);
    iModelHubHelpers::ExpectLocksCount(briefcase2, 0);

    //Create one more model and release all locks and codes.
    auto partition1_4 = CreateAndInsertModeledElement(TestCodeName(7).c_str(), db1);
    iModelHubHelpers::ExpectCodesCount(briefcase1, 2);
    ExpectCodeState(CreateCodeReserved(partition1_4->GetCode(), db1), imodelManager1);
    db1.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, true)); // Release all codes and locks.
    Utf8String changeSet5 = briefcase1->GetLastChangeSetPulled();
    iModelHubHelpers::ExpectCodesCount(briefcase1, 0);
    ExpectNoCodeWithState(CreateCodeAvailable(modelCode1_3), imodelManager1);
    ExpectCodeState(CreateCodeUsed(partition1_4->GetCode(), changeSet5), imodelManager1);
    iModelHubHelpers::ExpectLocksCount(briefcase1, 0);

    LockableIdSet locks;
    codes.clear();
    codes.insert(partition1_1->GetCode());
    codes.insert(partition1_2->GetCode());
    codes.insert(partition1_4->GetCode());
    codes.insert(partition2_1->GetCode());
    codes.insert(partition2_2->GetCode());
    auto result = briefcase1->GetiModelConnection().QueryCodesLocksById(codes, locks)->GetResult();
    EXPECT_SUCCESS(result);

    //Check if we can reserve reserved and discarded codes without changeSet.
    iModelHubHelpers::SetLastPulledChangeSetId(briefcase1, "");
    iModelHubHelpers::SetLastPulledChangeSetId(briefcase2, "");
    codes.clear();
    codes.insert(modelCode2_3);
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());
    codes.clear();
    codes.insert(modelCode1_3);
    EXPECT_STATUS(Success, db2.BriefcaseManager().ReserveCodes(codes).Result());
    iModelHubHelpers::ExpectCodesCount(briefcase1, 1);
    iModelHubHelpers::ExpectCodesCount(briefcase2, 1);

    //We should not be able to acquire locks if we haven't pulled a required changeSet.
    iModelHubHelpers::SetLastPulledChangeSetId(briefcase1, changeSet2);
    iModelHubHelpers::SetLastPulledChangeSetId(briefcase2, changeSet1);
    EXPECT_EQ(RepositoryStatus::RevisionRequired, AcquireLock(db2, *partition1_1));
    EXPECT_EQ(RepositoryStatus::RevisionRequired, AcquireLock(db2, *partition1_2));
    EXPECT_EQ(RepositoryStatus::RevisionRequired, AcquireLock(db1, *partition2_1));
    EXPECT_EQ(RepositoryStatus::RevisionRequired, AcquireLock(db1, *partition2_2));
    iModelHubHelpers::ExpectLocksCount(briefcase1, 0);
    iModelHubHelpers::ExpectLocksCount(briefcase2, 0);

    //We should be able to acquire locks if we pulled a required changeSet.
    iModelHubHelpers::SetLastPulledChangeSetId(briefcase1, changeSet4);
    iModelHubHelpers::SetLastPulledChangeSetId(briefcase2, changeSet3);
    EXPECT_EQ(RepositoryStatus::Success, AcquireLock(db2, *partition1_1));
    EXPECT_EQ(RepositoryStatus::Success, AcquireLock(db2, *partition1_2));
    EXPECT_EQ(RepositoryStatus::RevisionRequired, AcquireLock(db2, *partition1_4));
    EXPECT_EQ(RepositoryStatus::Success, AcquireLock(db1, *partition2_1));
    EXPECT_EQ(RepositoryStatus::Success, AcquireLock(db1, *partition2_2));
    iModelHubHelpers::ExpectLocksCount(briefcase1, 2);
    iModelHubHelpers::ExpectLocksCount(briefcase2, 2);

    iModelHubHelpers::SetLastPulledChangeSetId(briefcase2, changeSet5);
    EXPECT_EQ(RepositoryStatus::Success, AcquireLock(db2, *partition1_4));
    iModelHubHelpers::ExpectLocksCount(briefcase2, 3);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Andrius.Zonys                   09/2016
//---------------------------------------------------------------------------------------
TEST_F(CodesLocksTests, RelinquishOtherUserCodesLocks)
    {
    ClientPtr nonAdminClient = CreateNonAdminClient();

    // Prapare imodel and acquire briefcases.
    BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(nonAdminClient, s_info);
    ASSERT_SUCCESS(briefcaseResult);
    BriefcasePtr briefcase1 = briefcaseResult.GetValue();
    BriefcasePtr briefcase2 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();
    iModelHubHost::Instance().SetRepositoryAdmin(nonAdminClient->GetiModelAdmin());
    auto imodelManager1 = IntegrationTestsBase::_GetRepositoryManager(db1);
    iModelHubHost::Instance().SetRepositoryAdmin(s_client->GetiModelAdmin());
    auto imodelManager2 = IntegrationTestsBase::_GetRepositoryManager(db2);

    // Briefcase1 creates two models. This should also acquire codes and locks automatically.
    iModelHubHost::Instance().SetRepositoryAdmin(nonAdminClient->GetiModelAdmin());
    auto modelElem1 = CreateAndInsertModeledElement(TestCodeName(1).c_str(), db1);
    auto modelElem2 = CreateAndInsertModeledElement(TestCodeName(2).c_str(), db1);
    iModelHubHost::Instance().SetRepositoryAdmin(s_client->GetiModelAdmin());
    db1.SaveChanges();
    iModelHubHelpers::ExpectCodesCount(briefcase1, 2);
    ExpectCodeState(CreateCodeReserved(modelElem1->GetCode(), db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelElem2->GetCode(), db1), imodelManager1);
    iModelHubHelpers::ExpectLocksCount(briefcase1, 2);

    // Briefcase2 relinquishes other briefcase codes and locks.
    auto result = briefcase2->GetiModelConnection().RelinquishCodesLocks(briefcase1->GetBriefcaseId())->GetResult();
    EXPECT_SUCCESS(result);
    iModelHubHelpers::ExpectCodesCount(briefcase1, 0);
    iModelHubHelpers::ExpectLocksCount(briefcase1, 0);

    // Briefcase1 should be able to push changes since nobody owns them.
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false));
    Utf8String changeSet1 = briefcase1->GetLastChangeSetPulled();
    iModelHubHelpers::ExpectCodesCount(briefcase1, 0);
    ExpectCodeState(CreateCodeUsed(modelElem1->GetCode(), changeSet1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelElem2->GetCode(), changeSet1), imodelManager1);
    iModelHubHelpers::ExpectLocksCount(briefcase1, 4);

    // Briefcase1 deletes one model and creates two new.
    EXPECT_EQ(DgnDbStatus::Success, modelElem2->Delete());
    iModelHubHost::Instance().SetRepositoryAdmin(nonAdminClient->GetiModelAdmin());
    auto modelElem3 = CreateAndInsertModeledElement(TestCodeName(3).c_str(), db1);
    auto modelElem4 = CreateAndInsertModeledElement(TestCodeName(4).c_str(), db1);
    iModelHubHost::Instance().SetRepositoryAdmin(s_client->GetiModelAdmin());
    db1.SaveChanges();

    // Briefcase2 should not be able to release reserved code.
    DgnLockSet locks;
    DgnCodeSet codes;
    codes.insert(modelElem1->GetCode());
    codes.insert(modelElem3->GetCode());
    result = briefcase2->GetiModelConnection().DemoteCodesLocks(locks, codes, briefcase2->GetBriefcaseId(), db2.GetDbGuid())->GetResult();
    EXPECT_EQ(Error::Id::CodeReservedByAnotherBriefcase, result.GetError().GetId());
    iModelHubHelpers::ExpectCodesCount(briefcase1, 2);
    ExpectCodeState(CreateCodeUsed(modelElem1->GetCode(), changeSet1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelElem2->GetCode(), changeSet1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelElem3->GetCode(), db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelElem4->GetCode(), db1), imodelManager1);
    iModelHubHelpers::ExpectLocksCount(briefcase1, 4);

    // Briefcase2 relinquishes specific code and lock.
    locks.clear();
    locks.insert(DgnLock(LockableId(modelElem1->GetElementId()), LockLevel::None));
    codes.clear();
    codes.insert(modelElem3->GetCode());
    result = briefcase2->GetiModelConnection().DemoteCodesLocks(locks, codes, briefcase1->GetBriefcaseId(), db2.GetDbGuid())->GetResult();
    EXPECT_SUCCESS(result);
    iModelHubHelpers::ExpectCodesCount(briefcase1, 1);
    ExpectCodeState(CreateCodeUsed(modelElem1->GetCode(), changeSet1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelElem2->GetCode(), changeSet1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelElem4->GetCode(), db1), imodelManager1);
    iModelHubHelpers::ExpectLocksCount(briefcase1, 3);

    // Briefcase2 deletes model1.
    EXPECT_SUCCESS(briefcase2->PullAndMerge()->GetResult());
    DgnElementCPtr model1_2 = briefcase2->GetDgnDb().Elements().GetElement(modelElem1->GetElementId());
    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::Success, db2.BriefcaseManager().PrepareForElementDelete(req, *model1_2, IBriefcaseManager::PrepareAction::Acquire));
    EXPECT_EQ(DgnDbStatus::Success, model1_2->Delete());
    db2.SaveChanges();
    iModelHubHelpers::ExpectCodesCount(briefcase1, 1);
    iModelHubHelpers::ExpectLocksCount(briefcase1, 3);
    iModelHubHelpers::ExpectCodesCount(briefcase2, 0);
    ExpectCodeState(CreateCodeUsed(modelElem1->GetCode(), changeSet1), imodelManager2);
    iModelHubHelpers::ExpectLocksCount(briefcase2, 3);

    // Briefcase2 acquires modelCode3.
    EXPECT_STATUS(Success, db2.BriefcaseManager().ReserveCodes(codes).Result());
    iModelHubHelpers::ExpectCodesCount(briefcase2, 1);
    ExpectCodeState(CreateCodeReserved(modelElem3->GetCode(), db2), imodelManager2);

    // Briefcase1 should not be able to release all other briefcase locks and codes.
    result = briefcase1->GetiModelConnection().RelinquishCodesLocks(briefcase2->GetBriefcaseId())->GetResult();
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase1 should not be able to release specific other briefcase locks and codes.
    locks.clear();
    locks.insert(DgnLock(LockableId(modelElem1->GetDgnDb()), LockLevel::None));
    result = briefcase1->GetiModelConnection().DemoteCodesLocks(locks, codes, briefcase2->GetBriefcaseId(), db1.GetDbGuid())->GetResult();
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase2 should be able to push changes.
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, true, false));
    Utf8String changeSet2 = briefcase2->GetLastChangeSetPulled();

    // Briefcase1 should not be able to push his changes since one code is owned.
    iModelHubHost::Instance().SetRepositoryAdmin(nonAdminClient->GetiModelAdmin());
    auto pushResult = briefcase1->PullMergeAndPush()->GetResult();
    iModelHubHost::Instance().SetRepositoryAdmin(s_client->GetiModelAdmin());
    ASSERT_FAILURE(pushResult);
    EXPECT_EQ(Error::Id::CodeReservedByAnotherBriefcase, pushResult.GetError().GetId());

    iModelHubHelpers::ExpectCodesCount(briefcase1, 1);
    iModelHubHelpers::ExpectLocksCount(briefcase1, 3);
    iModelHubHelpers::ExpectCodesCount(briefcase2, 1);
    ExpectNoCodeWithState(CreateCodeDiscarded(modelElem1->GetCode(), changeSet2), imodelManager2);
    ExpectCodeState(CreateCodeReserved(modelElem3->GetCode(), db2), imodelManager2);
    iModelHubHelpers::ExpectLocksCount(briefcase2, 3);
    }
