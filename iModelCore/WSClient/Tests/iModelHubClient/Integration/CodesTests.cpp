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
* @bsiclass                                     Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct CodesTests : public iModelTestsBase
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

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Utf8String GetParentRevisionId(BriefcaseR briefcase)
        {
        return briefcase.GetDgnDb().Revisions().GetParentRevisionId();
        }
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, ReserveCodeMultipleBriefcases)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireAndOpenBriefcase();
    auto briefcase2 = AcquireAndOpenBriefcase();
    auto manager1 = IntegrationTestsBase::_GetRepositoryManager(briefcase1->GetDgnDb());
    auto manager2 = IntegrationTestsBase::_GetRepositoryManager(briefcase2->GetDgnDb());

    iModelHubHelpers::ExpectCodesCount(briefcase1, 0);
    iModelHubHelpers::ExpectCodesCount(briefcase2, 0);

    DgnCode code = CreateElementCode(briefcase1->GetDgnDb(), TestCodeName().c_str(), "");
    DgnCodeSet codesSet;
    codesSet.insert(code);

    // Firstly code is not reserved
    RepositoryStatus status;
    EXPECT_FALSE(briefcase1->GetDgnDb().BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::CodeNotReserved, status);
    EXPECT_FALSE(briefcase2->GetDgnDb().BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::CodeNotReserved, status);

    // Reserve code
    EXPECT_EQ(RepositoryStatus::Success, briefcase1->GetDgnDb().BriefcaseManager().ReserveCode(code));
    EXPECT_EQ(RepositoryStatus::CodeUnavailable, briefcase2->GetDgnDb().BriefcaseManager().ReserveCode(code));

    // Checks how api behaves when trying to reserve same code multiple times
    EXPECT_EQ(RepositoryStatus::Success, briefcase1->GetDgnDb().BriefcaseManager().ReserveCode(code));

    iModelHubHelpers::ExpectCodesCount(briefcase1, 1);
    iModelHubHelpers::ExpectCodesCount(briefcase2, 0);
    ExpectCodeState(CreateCodeReserved(code, briefcase1->GetDgnDb()), manager1);

    // Check if code is reserved by first briefcase
    EXPECT_TRUE(briefcase1->GetDgnDb().BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::Success, status);

    codesSet.insert(code);
    EXPECT_FALSE(briefcase2->GetDgnDb().BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::CodeNotReserved, status);

    // Check if code is not released using wrong briefcase
    EXPECT_EQ(RepositoryStatus::CodeUnavailable, briefcase2->GetDgnDb().BriefcaseManager().ReleaseCodes(codesSet));
    EXPECT_TRUE(briefcase1->GetDgnDb().BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::Success, status);

    // Release code
    codesSet.insert(code);
    EXPECT_EQ(RepositoryStatus::Success, briefcase1->GetDgnDb().BriefcaseManager().ReleaseCodes(codesSet));
    EXPECT_FALSE(briefcase1->GetDgnDb().BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::CodeNotReserved, status);

    ExpectNoCodeWithState(CreateCodeAvailable(code), manager1);

    // Check if now other briefcase is able to reserve code
    EXPECT_EQ(RepositoryStatus::Success, briefcase2->GetDgnDb().BriefcaseManager().ReserveCode(code));
    ExpectCodeState(CreateCodeReserved(code, briefcase2->GetDgnDb()), manager2);

    // Relinquish
    EXPECT_EQ(RepositoryStatus::Success, briefcase2->GetDgnDb().BriefcaseManager().RelinquishCodes());
    ExpectNoCodeWithState(CreateCodeDiscarded(code, GetParentRevisionId(*briefcase2)), manager2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, ReserveMultipleCodes)
    {
    //Prapare imodel and acquire briefcases
    BriefcasePtr briefcase = AcquireAndOpenBriefcase();
    DgnDbR db = briefcase->GetDgnDb();
    auto manager = IntegrationTestsBase::_GetRepositoryManager(db);

    iModelHubHelpers::ExpectCodesCount(briefcase, 0);

    DgnCode code = CreateElementCode(db, TestCodeName(1).c_str(), TestCodeName().c_str());
    DgnCode code2 = CreateElementCode(db, TestCodeName(2).c_str(), TestCodeName().c_str());
    DgnCodeSet codesSet;
    codesSet.insert(code);
    codesSet.insert(code2);

    // Firstly codes are not reserved
    RepositoryStatus status;
    EXPECT_FALSE(db.BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::CodeNotReserved, status);

    // Reserve codes
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCodes(codesSet).Result());
    iModelHubHelpers::ExpectCodesCount(briefcase, 2);
    ExpectCodeState(CreateCodeReserved(code, db), manager);
    ExpectCodeState(CreateCodeReserved(code2, db), manager);

    // Release one code
    DgnCodeSet codes;
    codes.insert(code);
    EXPECT_STATUS(Success, db.BriefcaseManager().ReleaseCodes(codes));
    ExpectNoCodeWithState(CreateCodeAvailable(code), manager);
    ExpectCodeState(CreateCodeReserved(code2, db), manager);

    // Relinquish
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishCodes());
    ExpectNoCodeWithState(CreateCodeAvailable(code), manager);
    ExpectNoCodeWithState(CreateCodeAvailable(code2), manager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, ReserveStyleCodes)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase = AcquireAndOpenBriefcase();
    DgnDbR db = briefcase->GetDgnDb();
    auto manager = IntegrationTestsBase::_GetRepositoryManager(db);

    // When we insert an element without having explicitly reserved its code, an attempt to reserve it will automatically occur
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle(TestCodeName().c_str(), db));
    iModelHubHelpers::ExpectCodesCount(briefcase, 1);
    ExpectCodeState(CreateCodeReserved(MakeStyleCode(TestCodeName().c_str(), db), db), manager);

    // An attempt to insert an element with the same code as an already-used code will fail
    EXPECT_EQ(DgnDbStatus::DuplicateCode, InsertStyle(TestCodeName().c_str(), db, false));

    // Updating an element and changing its code will NOT reserve the new code if we haven't done so already
    auto pStyle = AnnotationTextStyle::Get(db.GetDictionaryModel(), TestCodeName().c_str())->CreateCopy();
    DgnDbStatus status;
    pStyle->SetName(TestCodeName(1).c_str());
    EXPECT_FALSE(pStyle->DgnElement::Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, status);

    // Explicitly reserve the code
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(pStyle->GetCode()));
    EXPECT_TRUE(pStyle->Update().IsValid());
    iModelHubHelpers::ExpectCodesCount(briefcase, 2);
    ExpectCodeState(CreateCodeReserved(pStyle->GetCode(), db), manager);
    pStyle = nullptr;

    db.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));
    ExpectCodeState(CreateCodeUsed(MakeStyleCode(TestCodeName(1).c_str(), db), GetParentRevisionId(*briefcase)), manager);

    pStyle = nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, ReserveModelCode)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase = AcquireAndOpenBriefcase();
    DgnDbR db = briefcase->GetDgnDb();
    auto manager = IntegrationTestsBase::_GetRepositoryManager(db);

    auto modeledElement = CreateModeledElement(TestCodeName().c_str(), db);
    auto persistentModeledElement = modeledElement->Insert();
    ASSERT_TRUE(persistentModeledElement.IsValid());
    DgnCode modeledElemCode = modeledElement->GetCode();
    iModelHubHelpers::ExpectCodesCount(briefcase, 1);
    ExpectCodeState(CreateCodeReserved(modeledElemCode, db), manager);

    db.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));
    iModelHubHelpers::ExpectCodesCount(briefcase, 0);
    ExpectCodeState(CreateCodeUsed(modeledElemCode, GetParentRevisionId(*briefcase)), manager);

    // Same code reservation for the second time should pass
    auto reserveResult = db.BriefcaseManager().ReserveCode(modeledElemCode);
    EXPECT_EQ(RepositoryStatus::Success, reserveResult);

    // Changing model code to not reserved one should fail
    EXPECT_EQ(DgnDbStatus::Success, modeledElement->SetCode(MakeModelCode(TestCodeName(1).c_str(), db)));
    EXPECT_FALSE(modeledElement->Update().IsValid());

    // Creating new model with the same code should fail
    auto partition = PhysicalPartition::Create(*db.Elements().GetRootSubject(), TestCodeName().c_str());
    ASSERT_TRUE(partition.IsValid());
    EXPECT_EQ(RepositoryStatus::CodeUsed, db.BriefcaseManager().AcquireForElementInsert(*partition));
    EXPECT_FALSE(partition->Insert().IsValid());

    EXPECT_EQ(DgnDbStatus::Success, persistentModeledElement->Delete());
    briefcase->GetDgnDb().SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));
    ExpectNoCodeWithState(CreateCodeDiscarded(MakeModelCode(TestCodeName().c_str(), db), GetParentRevisionId(*briefcase)), manager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, CodesWithChangeSets)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase = AcquireAndOpenBriefcase();
    DgnDbR db = briefcase->GetDgnDb();
    auto manager = IntegrationTestsBase::_GetRepositoryManager(db);

    DgnCode unusedCode = MakeStyleCode(TestCodeName().c_str(), db);
    DgnCode usedCode = MakeStyleCode(TestCodeName(1).c_str(), db);
    DgnCodeSet req;
    req.insert(unusedCode);
    req.insert(usedCode);
    EXPECT_STATUS(Success, db.BriefcaseManager().ReserveCodes(req).Result());

    // Use one of the codes
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle(TestCodeName(1).c_str(), db));
    ExpectCodeState(CreateCodeReserved(unusedCode, db), manager);
    ExpectCodeState(CreateCodeReserved(usedCode, db), manager);

    // Commit the change as a changeSet
    briefcase->GetDgnDb().SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));

    // The used code should not be marked as such
    ExpectCodeState(CreateCodeUsed(usedCode, GetParentRevisionId(*briefcase)), manager);
    ExpectCodeState(CreateCodeReserved(unusedCode, db), manager);

    // Swap the code so that "Used" becomes "Unused"
    auto pStyle = AnnotationTextStyle::GetForEdit(db.GetDictionaryModel(), TestCodeName(1).c_str());
    pStyle->SetName(TestCodeName().c_str());
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;

    // Commit the changeSet
    briefcase->GetDgnDb().SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));
    Utf8String cs2 = GetParentRevisionId(*briefcase);

    // "Used" is now discarded; "Unused" is now used; both in the same changeSet
    ExpectCodeState(CreateCodeUsed(unusedCode, cs2), manager);
    ExpectNoCodeWithState(CreateCodeDiscarded(usedCode, cs2), manager);

    // Delete the style => its code becomes discarded
    // Ugh except you are not allowed to delete text styles...rename it again instead
    pStyle = AnnotationTextStyle::GetForEdit(db.GetDictionaryModel(), TestCodeName().c_str());
    pStyle->SetName(TestCodeName(2).c_str());

    // Will fail because we haven't reserved code...
    DgnDbStatus status;
    EXPECT_FALSE(pStyle->DgnElement::Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, status);
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(MakeStyleCode(TestCodeName(2).c_str(), db)));
    EXPECT_TRUE(pStyle->DgnElement::Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::Success, status);
    pStyle = nullptr;

    // Cannot release codes if transactions are pending
    iModelHubHelpers::ExpectCodesCount(briefcase, 1);
    DgnCodeSet codes;
    codes.insert(MakeStyleCode(TestCodeName(2).c_str(), db));
    EXPECT_STATUS(PendingTransactions, db.BriefcaseManager().ReleaseCodes(codes));
    EXPECT_STATUS(PendingTransactions, db.BriefcaseManager().RelinquishCodes());

    // Cannot release a code which is used locally
    db.SaveChanges();
    iModelHubHelpers::ExpectCodesCount(briefcase, 1);
    EXPECT_STATUS(CodeUsed, db.BriefcaseManager().ReleaseCodes(codes));
    EXPECT_STATUS(CodeUsed, db.BriefcaseManager().RelinquishCodes());

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));
    Utf8String cs3 = GetParentRevisionId(*briefcase);
    ExpectNoCodeWithState(CreateCodeDiscarded(unusedCode, cs3), manager);
    ExpectNoCodeWithState(CreateCodeDiscarded(usedCode, cs2), manager);

    // We can reserve either code, since they are discarded and we have the latest changeSet
    EXPECT_STATUS(Success, db.BriefcaseManager().ReserveCode(usedCode));
    EXPECT_STATUS(Success, db.BriefcaseManager().ReserveCode(unusedCode));
    ExpectCodeState(CreateCodeReserved(usedCode, db), manager);
    ExpectCodeState(CreateCodeReserved(unusedCode, db), manager);

    // If we release these codes, they should return to "Discarded" and retain the most recent changeSet ID in which they were discarded.
    EXPECT_STATUS(Success, db.BriefcaseManager().RelinquishCodes());
    ExpectNoCodeWithState(CreateCodeDiscarded(unusedCode, cs3), manager);
    ExpectNoCodeWithState(CreateCodeDiscarded(usedCode, cs2), manager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, CodesWithSpecialSymbols)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase = AcquireAndOpenBriefcase();
    DgnDbR db = briefcase->GetDgnDb();
    iModelManager* manager = (iModelManager*)_GetRepositoryManager(db);
    manager->GetiModelConnectionPtr()->SetCodesLocksPageSize(1);

    DgnCode code1 = MakeStyleCode("1*1", db);
    DgnCode code2 = MakeStyleCode("\t*\n", db);
    DgnCodeSet req;
    req.insert(code1);
    req.insert(code2);
    EXPECT_STATUS(Success, db.BriefcaseManager().ReserveCodes(req).Result());

    // Use one of the codes
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("1*1", db));
    ExpectCodeState(CreateCodeReserved(code1, db), manager);
    ExpectCodeState(CreateCodeReserved(code2, db), manager);

    // Commit the change as a changeSet
    briefcase->GetDgnDb().SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));

    ExpectCodeState(CreateCodeUsed(code1, GetParentRevisionId(*briefcase)), manager);
    ExpectCodeState(CreateCodeReserved(code2, db), manager);

    // Discard first code
    auto pStyle = AnnotationTextStyle::GetForEdit(db.GetDictionaryModel(), "1*1");
    pStyle->SetName("\t*\n");
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;

    // Commit the changeSet
    briefcase->GetDgnDb().SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false, false));

    Utf8String cs = GetParentRevisionId(*briefcase);
    ExpectCodeState(CreateCodeUsed(code2, cs), manager);
    ExpectNoCodeWithState(CreateCodeDiscarded(code1, cs), manager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Dziedzelis             06/2016
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, QueryUnavailableCodesTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireAndOpenBriefcase();
    auto briefcase2 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();
    auto manager = IntegrationTestsBase::_GetRepositoryManager(db2);

    int codesCountInSeedFile = GetCodesCount(db1);
    iModelHubHelpers::ExpectUnavailableCodesCount(briefcase1, codesCountInSeedFile);
    iModelHubHelpers::ExpectUnavailableCodesCount(briefcase2, codesCountInSeedFile);

    //Reserve the code
    DgnCode used = MakeStyleCode(TestCodeName(1).c_str(), db1);
    DgnCode unused = MakeStyleCode(TestCodeName().c_str(), db1);
    DgnCodeSet req;
    req.insert(used);
    req.insert(unused);
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(req).Result());

    //Reserved code should be unavailable
    ExpectCodeState(CreateCodeReserved(used, db1), manager);
    ExpectCodeState(CreateCodeReserved(unused, db1), manager);
    iModelHubHelpers::ExpectUnavailableCodesCount(briefcase2, codesCountInSeedFile + 2);

    //Use the code
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle(TestCodeName(1).c_str(), db1));
    db1.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));

    //Used code should be unavailable
    Utf8String cs1 = GetParentRevisionId(*briefcase1);
    ExpectCodeState(CreateCodeUsed(used, cs1), manager);
    ExpectCodeState(CreateCodeReserved(unused, db1), manager);
    iModelHubHelpers::ExpectUnavailableCodesCount(briefcase2, codesCountInSeedFile + 2);

    //Swap the name, to discard the first code
    auto pStyle = AnnotationTextStyle::GetForEdit(db1.GetDictionaryModel(), TestCodeName(1).c_str());
    pStyle->SetName(TestCodeName().c_str());
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;

    db1.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));

    //Discarded code should be available
    Utf8String cs2 = GetParentRevisionId(*briefcase1);
    ExpectNoCodeWithState(CreateCodeDiscarded(used, cs2), manager);
    ExpectCodeState(CreateCodeUsed(unused, cs2), manager);
    iModelHubHelpers::ExpectUnavailableCodesCount(briefcase2, codesCountInSeedFile + 1);
    {
    //Merge the second briefcase
    auto pullResult = briefcase2->PullAndMerge()->GetResult();
    EXPECT_SUCCESS(pullResult);
    }

    //Discared code should be available
    ExpectNoCodeWithState(CreateCodeDiscarded(used, cs2), manager);
    ExpectCodeState(CreateCodeUsed(unused, cs2), manager);
    iModelHubHelpers::ExpectUnavailableCodesCount(briefcase2, codesCountInSeedFile + 1);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Dziedzelis             07/2016
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, QueryAvailableCodesTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireAndOpenBriefcase();
    auto briefcase2 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    DgnCode code1 = MakeStyleCode(TestCodeName().c_str(), db1);
    DgnCode code2 = MakeStyleCode(TestCodeName(1).c_str(), db1);

    //Reserve codes
    DgnCodeSet codes;
    codes.insert(code1);
    codes.insert(code2);
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());

    //Reserved codes are unavailable
    IBriefcaseManager::Request req(IBriefcaseManager::ResponseOptions::None);
    req.Codes() = codes;
    IBriefcaseManager::Response response(IBriefcaseManager::RequestPurpose::Query, req.Options());
    EXPECT_FALSE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Use a code
    InsertStyle(TestCodeName().c_str(), db1, true);
    db1.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));

    //Used code is available
    EXPECT_SUCCESS(briefcase2->PullAndMerge()->GetResult());
    req.Reset();
    req.SetOptions(IBriefcaseManager::ResponseOptions::CodeState);
    req.Codes().insert(code1);  // reserved
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //A set of available codes should be available
    DgnCode code3 = MakeStyleCode(TestCodeName(2).c_str(), db1);
    req.Reset();
    req.SetOptions(IBriefcaseManager::ResponseOptions::CodeState);
    req.Codes().insert(code3);  // new code
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Make a code available
    DgnLockSet locks;
    codes.clear();
    codes.insert(code2);
    EXPECT_STATUS(Success, db1.BriefcaseManager().Demote(locks, codes));

    //If all codes are available it should return true
    req.Reset();
    req.SetOptions(IBriefcaseManager::ResponseOptions::CodeState);
    req.Codes().insert(code2);  // available
    req.Codes().insert(code3);  // new code
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Discard a code
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());
    auto pStyle = AnnotationTextStyle::GetForEdit(db1.GetDictionaryModel(), TestCodeName().c_str());
    pStyle->SetName(TestCodeName(1).c_str());
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;
    db1.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));

    //Discarded codes should be available
    EXPECT_SUCCESS(briefcase2->PullAndMerge()->GetResult());
    req.Reset();
    req.SetOptions(IBriefcaseManager::ResponseOptions::CodeState);
    req.Codes().insert(code1);  // discarded
    req.Codes().insert(code3);  // new code
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             08/2016
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, GetCodeMaximumIndex)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();

    // First code spec
    CodeSpecPtr codeSpec1 = CodeSpec::Create(db1, TestCodeName().c_str());
    ASSERT_TRUE(codeSpec1.IsValid());
    ASSERT_EQ(codeSpec1->GetScope().GetType(), CodeScopeSpec::Type::Repository);
    codeSpec1->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("PMP-"));
    auto fragmentSpec = CodeFragmentSpec::FromSequence();
    fragmentSpec.SetMinChars(4);
    codeSpec1->GetFragmentSpecsR().push_back(fragmentSpec);
    db1.CodeSpecs().Insert(*codeSpec1);

    // Second code spec
    CodeSpecPtr codeSpec2 = CodeSpec::Create(db1, TestCodeName(1).c_str());
    ASSERT_TRUE(codeSpec2.IsValid());
    ASSERT_EQ(codeSpec2->GetScope().GetType(), CodeScopeSpec::Type::Repository);
    codeSpec2->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("P-"));
    auto fragmentSpec2 = CodeFragmentSpec::FromSequence();
    fragmentSpec2.SetMinChars(4);
    codeSpec2->GetFragmentSpecsR().push_back(fragmentSpec2);
    db1.CodeSpecs().Insert(*codeSpec2);

    auto partition1_1 = CreateAndInsertModeledElement("Model1-1", db1);
    db1.SaveChanges();

    //Reserve codes
    DgnCode code1 = DgnCode(codeSpec1->GetCodeSpecId(), codeSpec1->GetScopeElementId(*partition1_1), "PMP-0010");
    DgnCode code2 = DgnCode(codeSpec1->GetCodeSpecId(), codeSpec1->GetScopeElementId(*partition1_1), "PMP-0020");
    DgnCodeSet codes;
    codes.insert(code1);
    codes.insert(code2);
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());

    // Try with CodeSequence class
    auto codeSequence = CodeSequence(codeSpec1->GetCodeSpecId(), GetScopeString(codeSpec1, *partition1_1), "PMP-####");
    auto codeResult = briefcase1->GetiModelConnection().QueryCodeMaximumIndex(codeSequence)->GetResult();
    auto resultTemplate = codeResult.GetValue();
    EXPECT_EQ("0020", resultTemplate.GetValue());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             08/2016
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, GetCodeNextAvailable)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();

    // First code spec
    CodeSpecPtr codeSpec1 = CodeSpec::Create(db1, TestCodeName(0).c_str());
    ASSERT_TRUE(codeSpec1.IsValid());
    ASSERT_EQ(codeSpec1->GetScope().GetType(), CodeScopeSpec::Type::Repository);
    codeSpec1->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("PMQ-"));
    auto fragmentSpec = CodeFragmentSpec::FromSequence();
    fragmentSpec.SetMinChars(4);
    fragmentSpec.SetStartNumber(10);
    fragmentSpec.SetNumberGap(5);
    codeSpec1->GetFragmentSpecsR().push_back(fragmentSpec);
    db1.CodeSpecs().Insert(*codeSpec1);

    // Second code spec
    CodeSpecPtr codeSpec2 = CodeSpec::Create(db1, TestCodeName(1).c_str());
    ASSERT_TRUE(codeSpec2.IsValid());
    ASSERT_EQ(codeSpec2->GetScope().GetType(), CodeScopeSpec::Type::Repository);
    codeSpec2->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("P-"));
    auto fragmentSpec2 = CodeFragmentSpec::FromSequence();
    fragmentSpec2.SetMinChars(4);
    fragmentSpec2.SetStartNumber(10);
    fragmentSpec2.SetNumberGap(5);
    codeSpec2->GetFragmentSpecsR().push_back(fragmentSpec2);
    db1.CodeSpecs().Insert(*codeSpec2);

    auto partition1_1 = CreateAndInsertModeledElement("Model1-2", db1);
    db1.SaveChanges();

    //Reserve codes
    DgnCode code1 = DgnCode(codeSpec1->GetCodeSpecId(), codeSpec1->GetScopeElementId(*partition1_1), "PMQ-0010");
    DgnCode code2 = DgnCode(codeSpec1->GetCodeSpecId(), codeSpec1->GetScopeElementId(*partition1_1), "PMQ-0020");
    DgnCodeSet codes;
    codes.insert(code1);
    codes.insert(code2);
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());

    // Try with CodeSequence class
    auto codeSequence = CodeSequence(codeSpec1->GetCodeSpecId(), GetScopeString(codeSpec1, *partition1_1), "PMQ-####");
    auto templatesResult = briefcase1->GetiModelConnection().QueryCodeNextAvailable(codeSequence, 10, 5)->GetResult();
    auto resultTemplate = templatesResult.GetValue();
    EXPECT_EQ("0015", resultTemplate.GetValue());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
void InsertPhysicalElement(PhysicalElementPtr& element, PhysicalModelR model, DgnCategoryId categoryId, BeSQLite::BeGuidCR federationGuid = BeSQLite::BeGuid(), DgnCodeCR code = DgnCode())
    {
    element = GenericPhysicalObject::Create(model, categoryId);
    ASSERT_TRUE(element.IsValid());
    element->SetFederationGuid(federationGuid);
    element->SetCode(code);
    ASSERT_TRUE(element->Insert().IsValid());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, PlantScenario)
    {
    auto briefcase1 = AcquireAndOpenBriefcase();
    DgnDbR db = briefcase1->GetDgnDb();

    EXPECT_FALSE(db.BriefcaseManager().IsBulkOperation());
    db.BriefcaseManager().StartBulkOperation();
    EXPECT_TRUE(db.BriefcaseManager().IsBulkOperation());

    PhysicalModelPtr physicalModel = CreateModel(TestCodeName().c_str(), db);
    ASSERT_TRUE(physicalModel.IsValid());
    DgnCategoryId categoryId = GetOrCreateCategory(db);
    ASSERT_TRUE(categoryId.IsValid());

    CodeSpecPtr unitCodeSpec = CodeSpec::Create(db, "CodesManagerTest.Unit", CodeScopeSpec::CreateModelScope());
    ASSERT_TRUE(unitCodeSpec.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, unitCodeSpec->Insert());

    // Create related element scope codeSpec
    Utf8CP fakeRelationship = "Fake.EquipmentScopedByUnit"; // relationship name not validated by CodeScopeSpec yet
    CodeSpecPtr equipmentCodeSpec = CodeSpec::Create(db, "CodesManagerTest.Equipment", CodeScopeSpec::CreateRelatedElementScope(fakeRelationship, CodeScopeSpec::ScopeRequirement::FederationGuid));
    ASSERT_TRUE(equipmentCodeSpec.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, equipmentCodeSpec->Insert());
    EXPECT_TRUE(equipmentCodeSpec->IsRelatedElementScope());
    EXPECT_STREQ(equipmentCodeSpec->GetScope().GetRelationship().c_str(), fakeRelationship);
    EXPECT_TRUE(equipmentCodeSpec->GetScope().IsFederationGuidRequired());

    db.SaveChanges("1");
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));

    BeSQLite::BeGuid unitGuid(true);
    BeSQLite::BeGuid equipment1Guid(true);

    // Check what is next available code
    auto codeSequence = CodeSequence(equipmentCodeSpec->GetCodeSpecId(), unitGuid.ToString(), "P-#");
    auto templatesResult = briefcase1->GetiModelConnection().QueryCodeNextAvailable(codeSequence, 1, 1)->GetResult();
    auto resultTemplate = templatesResult.GetValue();
    EXPECT_EQ("1", resultTemplate.GetValue());

    // Create codes to reserve
    DgnCode unitCode = unitCodeSpec->CreateCode(*physicalModel, "U1");
    DgnCode equipment1Code = equipmentCodeSpec->CreateCode(unitGuid, "P-1");

    DgnCodeSet codesToReserve;
    codesToReserve.insert(unitCode);
    codesToReserve.insert(equipment1Code);
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCodes(codesToReserve).Result());
    EXPECT_TRUE(db.BriefcaseManager().AreCodesReserved(codesToReserve));

    // Use codes
    PhysicalElementPtr unitElement;
    InsertPhysicalElement(unitElement, *physicalModel, categoryId, unitGuid, unitCode);
    PhysicalElementPtr equipment1Element;
    InsertPhysicalElement(equipment1Element, *physicalModel, categoryId, equipment1Guid, equipment1Code);

    EXPECT_EQ(unitCode, unitElement->GetCode());
    EXPECT_EQ(equipment1Code, equipment1Element->GetCode());

    //ReserveCode "P-2"
    DgnCodeSet codesToReserve1;
    DgnCode equipment2Code = equipmentCodeSpec->CreateCode(unitGuid, "P-2");
    codesToReserve1.insert(equipment2Code);
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCodes(codesToReserve1).Result());
    EXPECT_TRUE(db.BriefcaseManager().AreCodesReserved(codesToReserve1));

    db.SaveChanges("2");
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));

    //RelinquishLocks – this should not affect ReservedCodes
    db.BriefcaseManager().RelinquishLocks();

    // Check what is next available code
    codeSequence = CodeSequence(equipmentCodeSpec->GetCodeSpecId(), unitGuid.ToString(), "P-#");
    templatesResult = briefcase1->GetiModelConnection().QueryCodeNextAvailable(codeSequence, 1, 1)->GetResult();
    resultTemplate = templatesResult.GetValue();
    EXPECT_EQ("3", resultTemplate.GetValue());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Andrius.Zonys                   09/2016
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, RelinquishOtherUserCodes)
    {
    ClientPtr nonAdminClient = CreateNonAdminClient();

    // Prapare imodel and acquire briefcases.
    BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(nonAdminClient, s_info);
    ASSERT_SUCCESS(briefcaseResult);
    auto briefcase1 = briefcaseResult.GetValue();
    auto briefcase2 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();
    iModelHubHost::Instance().SetRepositoryAdmin(nonAdminClient->GetiModelAdmin());
    auto imodelManager1 = IntegrationTestsBase::_GetRepositoryManager(db1);
    iModelHubHost::Instance().SetRepositoryAdmin(s_client->GetiModelAdmin());

    DgnCode modelCode1 = MakeModelCode(TestCodeName(1).c_str(), db1);
    DgnCode modelCode2 = MakeModelCode(TestCodeName(2).c_str(), db1);
    DgnCode modelCode3 = MakeModelCode(TestCodeName(3).c_str(), db1);
    DgnCode modelCode4 = MakeModelCode(TestCodeName(4).c_str(), db1);

    // Briefcase1 acquires three model codes.
    DgnCodeSet codes;
    codes.insert(modelCode1);
    codes.insert(modelCode2);
    codes.insert(modelCode3);
    iModelHubHost::Instance().SetRepositoryAdmin(nonAdminClient->GetiModelAdmin());
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());
    iModelHubHost::Instance().SetRepositoryAdmin(s_client->GetiModelAdmin());
    iModelHubHelpers::ExpectCodesCount(briefcase1, 3);
    ExpectCodeState(CreateCodeReserved(modelCode1, db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelCode2, db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelCode3, db1), imodelManager1);

    // Briefcase2 relinquishes other briefcase codes.
    EXPECT_SUCCESS(briefcase2->GetiModelConnection().RelinquishCodesLocks(briefcase1->GetBriefcaseId())->GetResult());
    iModelHubHelpers::ExpectCodesCount(briefcase1, 0);

    // Briefcase1 should be able to aquire codes again.
    iModelHubHost::Instance().SetRepositoryAdmin(nonAdminClient->GetiModelAdmin());
    db1.BriefcaseManager().RefreshFromRepository(); // BriefcaseManager says modelCode1 is already reserved if we don't call refresh.
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());
    iModelHubHelpers::ExpectCodesCount(briefcase1, 3);

    // Briefcase1 acquires modelCode4 and makes it used.
    auto model4 = CreateModel(TestCodeName(4).c_str(), db1);
    db1.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));
    Utf8String changeSet1 = briefcase1->GetLastChangeSetPulled();
    iModelHubHelpers::ExpectCodesCount(briefcase1, 3);
    ExpectCodeState(CreateCodeReserved(modelCode1, db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelCode2, db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelCode3, db1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelCode4, changeSet1), imodelManager1);
    iModelHubHost::Instance().SetRepositoryAdmin(s_client->GetiModelAdmin());

    // Briefcase2 should not be able to discard reserved code.
    DgnLockSet locks;
    codes.clear();
    codes.insert(modelCode3);
    auto result = briefcase2->GetiModelConnection().DemoteCodesLocks(locks, codes, briefcase2->GetBriefcaseId(), db2.GetDbGuid())->GetResult();
    EXPECT_EQ(Error::Id::CodeReservedByAnotherBriefcase, result.GetError().GetId());

    // Briefcase2 relinquishes specific codes.
    codes.clear();
    codes.insert(modelCode1);
    codes.insert(modelCode2);
    result = briefcase2->GetiModelConnection().DemoteCodesLocks(locks, codes, briefcase1->GetBriefcaseId(), db2.GetDbGuid())->GetResult();
    EXPECT_SUCCESS(result);
    iModelHubHelpers::ExpectCodesCount(briefcase1, 1);
    ExpectCodeState(CreateCodeReserved(modelCode3, db1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelCode4, changeSet1), imodelManager1);

    // Briefcase2 aquires modelCode1.
    auto model1 = CreateModel(TestCodeName(1).c_str(), db2);
    db2.SaveChanges();

    // Briefcase1 should not be able to release all other briefcase codes.
    result = briefcase1->GetiModelConnection().RelinquishCodesLocks(briefcase2->GetBriefcaseId())->GetResult();
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase1 should not be able to release specific other briefcase codes.
    codes.clear();
    codes.insert(modelCode1);
    result = briefcase1->GetiModelConnection().DemoteCodesLocks(locks, codes, briefcase2->GetBriefcaseId(), db1.GetDbGuid())->GetResult();
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase2 pushes his changes.
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, true, true));
    Utf8String changeSet2 = briefcase2->GetLastChangeSetPulled();
    iModelHubHelpers::ExpectCodesCount(briefcase1, 1);
    iModelHubHelpers::ExpectCodesCount(briefcase2, 0);
    ExpectCodeState(CreateCodeUsed(modelCode1, changeSet2), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelCode3, db1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelCode4, changeSet1), imodelManager1);

    // Briefcase2 relinquishes other briefcase codes while some used codes exists.
    result = briefcase2->GetiModelConnection().RelinquishCodesLocks(briefcase1->GetBriefcaseId())->GetResult();
    EXPECT_SUCCESS(result);
    iModelHubHelpers::ExpectCodesCount(briefcase1, 0);
    iModelHubHelpers::ExpectCodesCount(briefcase2, 0);
    ExpectCodeState(CreateCodeUsed(modelCode1, changeSet2), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelCode4, changeSet1), imodelManager1);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas        04/2017
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, CodesStatesResponseTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireAndOpenBriefcase();
    auto briefcase2 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    //Create a models and push them.
    auto model1 = CreateModel(TestCodeName().c_str(), briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true));

    auto model2 = CreateModel(TestCodeName(1).c_str(), briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true));

    DgnCodeSet codeSet;
    DgnCode code1 = MakeStyleCode(TestCodeName(2).c_str(), db1);
    codeSet.insert(code1);

    auto response = db1.BriefcaseManager().ReserveCodes(codeSet, IBriefcaseManager::ResponseOptions::CodeState);
    EXPECT_EQ(RepositoryStatus::Success, response.Result());

    response = db2.BriefcaseManager().ReserveCodes(codeSet, IBriefcaseManager::ResponseOptions::CodeState);
    EXPECT_EQ(RepositoryStatus::CodeUnavailable, response.Result());
    EXPECT_EQ(1, response.CodeStates().size());
    auto codeState = *response.CodeStates().begin();
    EXPECT_EQ(briefcase1->GetBriefcaseId(), codeState.GetReservedBy());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             07/2018
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, ExternalCodesRequestTest)
    {
    //Prapare imodel and acquire briefcases
    BriefcasePtr briefcase1 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();

    //Create an external code spec
    CodeSpecPtr externalCodeSpec = CodeSpec::Create(db1, TestCodeName().c_str());
    ASSERT_TRUE(externalCodeSpec.IsValid());
    externalCodeSpec->SetIsManagedWithDgnDb(false);
    db1.CodeSpecs().Insert(*externalCodeSpec);
    db1.SaveChanges();

    //Try to acquire code
    DgnCode testCode = externalCodeSpec->CreateCode(TestCodeName(1));
    IRepositoryManagerP manager = s_client->GetiModelAdmin()->_GetRepositoryManager(db1);
    IBriefcaseManager::Request req;
    req.Codes().insert(testCode);
    IRepositoryManager::Response response = manager->Acquire(req, db1);

    EXPECT_EQ(RepositoryStatus::InvalidRequest, response.Result());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             07/2018
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, ExternalCodesPushTest)
    {
    //Prapare imodel and acquire briefcases
    BriefcasePtr briefcase1 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();

    //Create an external code spec
    CodeSpecPtr internalCodeSpec = CodeSpec::Create(db1, TestCodeName(1).c_str());
    ASSERT_TRUE(internalCodeSpec.IsValid());
    internalCodeSpec->SetIsManagedWithDgnDb(false);

    //Create an element with an external code.
    auto model = CreateModel(TestCodeName().c_str(), db1);
    ASSERT_TRUE(model.IsValid());
    db1.CodeSpecs().Insert(*internalCodeSpec);
    DgnElementCPtr element = CreateElement(*model.get(), internalCodeSpec->CreateCode(TestCodeName(2)));
    ASSERT_TRUE(element.IsValid());
    db1.SaveChanges();

    //Push changes
    DgnCodeSet externalAssignedCodes, externalDiscardedCodes;
    CodeCallbackFunction callback = [&] (DgnCodeSet const& assignedCodes, DgnCodeSet const& discardedCodes)
        {
        externalAssignedCodes = assignedCodes;
        externalDiscardedCodes = discardedCodes;
        return CreateCompletedAsyncTask();
        };

    //There should be 1 external assigned code and 1 imodelhub assigned code for model
    ChangeSetsResult pushResult = briefcase1->PullMergeAndPush(nullptr, false, nullptr, nullptr, nullptr, 1, nullptr, &callback)->GetResult();
    ASSERT_SUCCESS(pushResult);
    EXPECT_EQ(1, externalAssignedCodes.size());
    EXPECT_EQ(0, externalDiscardedCodes.size());
    externalAssignedCodes.clear();
    externalDiscardedCodes.clear();

    //Reserve a new code
    auto updatedCode = internalCodeSpec->CreateCode(TestCodeName(3));
    db1.BriefcaseManager().ReserveCode(updatedCode);

    //Update element with the new code
    DgnElementPtr mutableElement = db1.Elements().GetForEdit<DgnElement>(element->GetElementId());
    ASSERT_TRUE(mutableElement.IsValid());
    mutableElement->SetCode(updatedCode);
    DgnDbStatus status;
    mutableElement->Update(&status);
    ASSERT_EQ(DgnDbStatus::Success, status);
    db1.SaveChanges();

    //There should be 1 assigned and 1 discarded external codes, no new codes in iModelHub
    pushResult = briefcase1->PullMergeAndPush(nullptr, false, nullptr, nullptr, nullptr, 1, nullptr, &callback)->GetResult();
    ASSERT_SUCCESS(pushResult);
    EXPECT_EQ(1, externalAssignedCodes.size());
    EXPECT_EQ(1, externalDiscardedCodes.size());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas        07/2017
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, CodeIdsTest)
    {
    //Prapare imodel and acquire briefcases
    BriefcasePtr briefcase1 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();

    CodeSpecPtr codeSpec1 = CodeSpec::Create(db1, TestCodeName().c_str());
    ASSERT_TRUE(codeSpec1.IsValid());
    db1.CodeSpecs().Insert(*codeSpec1);
    auto partition1_1 = CreateAndInsertModeledElement(TestCodeName(1).c_str(), db1);
    db1.SaveChanges();

    // Create two codes
    DgnCodeSet codeSet;
    DgnCode code1 = DgnCode(codeSpec1->GetCodeSpecId(), codeSpec1->GetScopeElementId(*partition1_1), "PMR-0010");
    codeSet.insert(code1);
    BeSQLite::BeGuid codeGuid;
    codeGuid.Create();
    DgnCode code2 = DgnCode(codeSpec1->GetCodeSpecId(), codeGuid, "PMR-0020");
    codeSet.insert(code2);

    // Reserve codes
    auto response = db1.BriefcaseManager().ReserveCodes(codeSet, IBriefcaseManager::ResponseOptions::CodeState);
    EXPECT_EQ(RepositoryStatus::Success, response.Result());

    DgnCodeSet codes;
    codes.insert(code1);
    codes.insert(code2);
    auto result = briefcase1->GetiModelConnection().QueryCodesByIds(codes)->GetResult();
    EXPECT_SUCCESS(result);

    auto codeStatesIterator = result.GetValue().begin();
    auto code1Result = *codeStatesIterator;
    codeStatesIterator++;
    auto code2Result = *codeStatesIterator;

    // Check if scope requirements were properly parsed
    EXPECT_EQ(code1.GetScopeString(), code1Result.GetCode().GetScopeString());
    EXPECT_EQ(code2.GetScopeString(), code2Result.GetCode().GetScopeString());

    //Query empty array of codes
    DgnCodeSet emptyCodes;
    iModelHubHelpers::ExpectCodesCountByIds(*briefcase1, 0, false, emptyCodes);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Gintare.Grazulyte                12/2017
//---------------------------------------------------------------------------------------
TEST_F(CodesTests, FailingCodesResponseOptions)
    {
    BriefcasePtr briefcase1 = AcquireAndOpenBriefcase();
    BriefcasePtr briefcase2 = AcquireAndOpenBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    iModelHubHelpers::ExpectCodesCount(briefcase1, 0);
    iModelHubHelpers::ExpectCodesCount(briefcase2, 0);

    PhysicalPartitionPtr partition1_1 = CreateModeledElement(TestCodeName().c_str(), db1);
    auto partition1_1Element = partition1_1->Insert();
    ASSERT_TRUE(partition1_1Element.IsValid());

    DgnElementCPtr partition1_2 = CreateAndInsertModeledElement(TestCodeName(1).c_str(), db1);
    iModelHubHelpers::ExpectCodesCount(briefcase1, 2);
    ASSERT_SUCCESS(briefcase1->GetiModelConnection().RelinquishCodesLocks(briefcase1->GetBriefcaseId())->GetResult());
    iModelHubHelpers::ExpectCodesCount(briefcase1, 0);

    DgnElementCPtr partition2_1 = CreateAndInsertModeledElement(TestCodeName().c_str(), db2);
    iModelHubHelpers::ExpectCodesCount(briefcase2, 1);
    db2.SaveChanges();

    db1.SaveChanges();
    ConflictsInfoPtr conflictsInfo = std::make_shared<ConflictsInfo>();
    PushChangeSetArgumentsPtr pushChangeSetArguments = PushChangeSetArguments::Create(nullptr, ChangeSetInfo::ContainingChanges::NotSpecified,
        nullptr, false, nullptr, IBriefcaseManager::ResponseOptions::All, nullptr, conflictsInfo);
    StatusResult result1 = briefcase1->Push(pushChangeSetArguments)->GetResult();
    ASSERT_SUCCESS(result1);
    EXPECT_TRUE(conflictsInfo->Any());
    EXPECT_EQ(1, conflictsInfo->GetCodesConflicts().size());
    
    DgnCodeSet codes;
    codes.insert(partition1_1->GetCode());
    db1.BriefcaseManager().ClearUserHeldCodesLocks();
    IBriefcaseManager::Response result2 = db1.BriefcaseManager().ReserveCodes(codes, IBriefcaseManager::ResponseOptions::None);
    ASSERT_EQ(RepositoryStatus::CodeUnavailable, result2.Result());
    EXPECT_TRUE(result2.CodeStates().empty());

    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::Success, db1.BriefcaseManager().PrepareForElementDelete(req, *partition1_1Element, IBriefcaseManager::PrepareAction::Acquire));
    EXPECT_EQ(DgnDbStatus::Success, partition1_1Element->Delete());
    briefcase1->GetDgnDb().SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false, false));
    }
