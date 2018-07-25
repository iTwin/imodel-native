/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/LRPTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct LRPTests : public IntegrationTestsBase
    {
    DgnDbPtr     m_db;

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             07/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        IntegrationTestsBase::SetUpTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void SetUp() override
        {
        IntegrationTestsBase::SetUp();
        ASSERT_SUCCESS(iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName()));
        m_db = CreateTestDb();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void TearDown() override
        {
        if (m_db.IsValid())
            m_db = nullptr;
        iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName());
        IntegrationTestsBase::TearDown();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    iModelResult CreateiModel(bool expectSuccess = true)
        {
        return IntegrationTestsBase::CreateiModel(m_db, expectSuccess);
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LRPTests, InitializeFileJob)
    {
    int seedFileCodesCount = GetCodesCount(*m_db);

    // Check projectGuid is not set
    auto savedGuid = m_db->QueryProjectGuid();
    EXPECT_FALSE(savedGuid.IsValid());

    PhysicalModelPtr model = CreateModel(TestCodeName().c_str(), *m_db);
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle(TestCodeName(1).c_str(), *m_db, true));
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, m_db->SaveChanges());

    iModelResult createResult = s_client->CreateNewiModel(s_projectId, *m_db)->GetResult();
    ASSERT_SUCCESS(createResult);

    iModelInfoPtr info = createResult.GetValue();
    iModelConnectionPtr imodelConnection = CreateiModelConnection(info);
    BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, info);
    ASSERT_SUCCESS(briefcaseResult);
    BriefcasePtr briefcase = briefcaseResult.GetValue();

    // Check InitializeFileJob set ProjectGuid
    savedGuid = briefcase->GetDgnDb().QueryProjectGuid();
    EXPECT_EQ(s_projectId, savedGuid.ToString());

    //Check if codes are created and locks cleaned
    iModelHubHelpers::ExpectUnavailableCodesCount(briefcase, seedFileCodesCount + 2);
     iModelHubHelpers::ExpectLocksCount(briefcase, 0);

    // Change the file guid
    auto fileName = m_db->GetFileName();
    m_db->ChangeDbGuid(BeSQLite::BeGuid(true));
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, m_db->SaveChanges());

    model = CreateModel(TestCodeName(2).c_str(), *m_db);
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle(TestCodeName(3).c_str(), *m_db, true));
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, m_db->SaveChanges());

    // Replace seed file
    auto fileInfo = FileInfo::Create(*m_db, TestCodeName(3).c_str());
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    EXPECT_SUCCESS(imodelConnection->UploadNewSeedFile(fileName, *fileInfo)->GetResult());

    //Check if codes are created and locks cleaned
    iModelHubHelpers::ExpectUnavailableCodesCount(briefcase, seedFileCodesCount + 4);
     iModelHubHelpers::ExpectLocksCount(briefcase, 0);
    }
