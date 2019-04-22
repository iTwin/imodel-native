/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"
#include <DgnPlatform/GenericDomain.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct CodeSpecLockTests : public iModelTestsBase
    {
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             07/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             07/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             07/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Utf8String GetParentRevisionId(BriefcaseR briefcase)
        {
        return briefcase.GetDgnDb().Revisions().GetParentRevisionId();
        }


    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             07/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool IsCodeSpecLockAvailable(DgnDbR db, IBriefcaseManager::Response* response)
        {
        IBriefcaseManager::Request req(IBriefcaseManager::ResponseOptions::None);
        req.Locks().GetLockSet().insert(DgnLock(LockableId(db.CodeSpecs()), LockLevel::Exclusive));
        return db.BriefcaseManager().AreResourcesAvailable(req, response, IBriefcaseManager::FastQuery::No);
        }
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             07/2018
//---------------------------------------------------------------------------------------
TEST_F(CodeSpecLockTests, TwoBriefcasesCreateCodeSpecs)
    {
    IBriefcaseManager::Response response(IBriefcaseManager::RequestPurpose::Query, IBriefcaseManager::ResponseOptions::None);

    auto briefcase1 = AcquireAndOpenBriefcase();
    auto briefcase2 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    // First briefcase creates first code spec
    CodeSpecPtr codeSpec11 = CodeSpec::Create(db1, TestCodeName().c_str());
    codeSpec11->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromSequence());
    EXPECT_EQ(DgnDbStatus::Success, db1.CodeSpecs().Insert(*codeSpec11));

    // First briefcase creates second code spec
    CodeSpecPtr codeSpec12 = CodeSpec::Create(db1, TestCodeName(1).c_str());
    codeSpec12->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromSequence());
    EXPECT_EQ(DgnDbStatus::Success, db1.CodeSpecs().Insert(*codeSpec12));

    // Second briefcase should fail to get lock for codespec
    EXPECT_FALSE(IsCodeSpecLockAvailable(db2, &response));

    CodeSpecPtr codeSpec21 = CodeSpec::Create(db2, TestCodeName(2).c_str());
    codeSpec21->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromSequence());
    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(DgnDbStatus::LockNotHeld, db2.CodeSpecs().Insert(*codeSpec21));
    BeTest::SetFailOnAssert(true);

    // First briefcase pushes his changes
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db1.SaveChanges());
    EXPECT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, true));

    // Second briefcase is not able to get lock until pulled
    EXPECT_FALSE(IsCodeSpecLockAvailable(db2, &response));
    EXPECT_EQ(RepositoryStatus::RevisionRequired, response.Result());

    EXPECT_SUCCESS(briefcase2->PullAndMerge()->GetResult());
    EXPECT_TRUE(IsCodeSpecLockAvailable(db2, &response));
    EXPECT_EQ(DgnDbStatus::Success, db2.CodeSpecs().Insert(*codeSpec21));

    // First briefcase should not be able to get lock
    EXPECT_FALSE(IsCodeSpecLockAvailable(db1, &response));
    EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());

    // Second briefcase pushes changes
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db2.SaveChanges());
    EXPECT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, true, false, true));

    // First briefcase should not be able to get lock until pulls
    EXPECT_FALSE(IsCodeSpecLockAvailable(db1, &response));
    EXPECT_EQ(RepositoryStatus::RevisionRequired, response.Result());

    EXPECT_SUCCESS(briefcase1->PullAndMerge()->GetResult());
    EXPECT_TRUE(IsCodeSpecLockAvailable(db1, &response));
    }
