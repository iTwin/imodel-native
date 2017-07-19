/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/LRP_Tests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"
#include "IntegrationTestsHelper.h"
#include <WebServices/iModelHub/Client/Client.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

struct LRPTests : public IntegrationTestsBase
    {
    ClientPtr    m_client;

    virtual void SetUp() override
        {
        IntegrationTestsBase::SetUp();
        auto proxy   = ProxyHttpHandler::GetFiddlerProxyIfReachable();
        m_client     = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidAdminCredentials(), proxy);
        m_pHost->SetRepositoryAdmin(m_client->GetiModelAdmin());
        }

    virtual void TearDown() override
        {
        m_client = nullptr;
        IntegrationTestsBase::TearDown();
        }

    DgnDbPtr CreateTestDb()
        {
        return IntegrationTestsBase::CreateTestDb("ClientTest");
        }

    void DeleteiModels()
        {
        auto result = m_client->GetiModels()->GetResult();
        EXPECT_SUCCESS(result);

        for (auto imodel : result.GetValue())
            {
            DeleteiModel(*m_client, *imodel);
            }
        }

    iModelInfoPtr CreateNewiModel()
        {
        return IntegrationTestsBase::CreateNewiModel(*m_client, "ClientTest");
        }
    };

TEST_F(LRPTests, InitializeFileJob)
    {
    auto db = CreateTestDb();
    int seedFileCodesCount = GetCodesCount(*db);

    auto modelPtr = CreateModel("InitializeModel1", *db);
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("InitializeStyle1", *db, true));
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db->SaveChanges());

    auto createResult = m_client->CreateNewiModel(*db)->GetResult();
    EXPECT_SUCCESS(createResult);

    auto imodelInfoPtr = createResult.GetValue();
    auto imodelConnection = ConnectToiModel(*m_client, imodelInfoPtr);
    auto briefcasePtr = AcquireBriefcase(*m_client, *imodelInfoPtr);

    //Check if codes are created and locks cleaned
    ExpectUnavailableCodesCount(*briefcasePtr, seedFileCodesCount + 2);
    ExpectLocksCount(*briefcasePtr, 0);

    // Change the file guid
    auto fileName = db->GetFileName();
    BeSQLite::BeGuid newGuid;
    newGuid.Create();
    db->ChangeDbGuid(newGuid);
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db->SaveChanges());

    modelPtr = CreateModel("ReplaceSeedFile1", *db);
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("ReplaceSeedFile2", *db, true));
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db->SaveChanges());

    // Replace seed file
    auto fileInfo = FileInfo::Create(*db, "Replacement description2");
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    EXPECT_SUCCESS(imodelConnection->UploadNewSeedFile(fileName, *fileInfo)->GetResult());

    //Check if codes are created and locks cleaned
    ExpectUnavailableCodesCount(*briefcasePtr, seedFileCodesCount + 4);
    ExpectLocksCount(*briefcasePtr, 0);

    DeleteiModel(*m_client, *imodelInfoPtr);
    }
