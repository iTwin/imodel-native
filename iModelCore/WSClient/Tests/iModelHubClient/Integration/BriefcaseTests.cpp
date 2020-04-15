/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct BriefcaseTests : public iModelTestsBase
    {
    DgnDbPtr                    m_db;

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
    virtual void SetUp() override
        {
        iModelTestsBase::SetUp();
        ASSERT_SUCCESS(iModelHubHelpers::AbandonAllBriefcases(s_client, s_connection));
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void TearDown() override
        {
        if (m_db.IsValid())
            {
            if (m_db->IsDbOpen())
                m_db->CloseDb();
            m_db = nullptr;
            }
        iModelTestsBase::TearDown();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExpectFileIsReadOnly(BeFileNameCR path, Utf8String changesetId)
        {
        DgnDbPtr db;
        OpenReadOnlyDgnDb(db, path);

        BriefcaseResult briefcaseResult = s_client->OpenBriefcase(db, false)->GetResult();
        ASSERT_FAILURE(briefcaseResult);
        EXPECT_EQ(Error::Id::FileIsNotBriefcase, briefcaseResult.GetError().GetId());
        EXPECT_EQ(changesetId, db->Revisions().GetParentRevisionId());

        PhysicalPartitionPtr partition = CreateModeledElement(TestCodeName().c_str(), *db);
        EXPECT_FALSE(partition->Insert().IsValid());
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, SuccessfulAcquireBriefcase)
    {
    TestsProgressCallback callback;
    auto result = s_client->AcquireBriefcaseToDir(*s_info, OutputDir(), false, Client::DefaultFileNameCallback, callback.Get())->GetResult();

    EXPECT_SUCCESS(result);
    auto dbPath = result.GetValue()->GetLocalPath();
    EXPECT_TRUE(dbPath.DoesPathExist());
    callback.Verify();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, UnsuccessfulAcquireBriefcase)
    {
    //Attempt acquiring briefcase from a non-existing imodel
    iModelResult imodelResult = iModelHubHelpers::CreateEmptyiModel(*s_client, s_projectId, GetTestiModelName(), "", true);
    ASSERT_SUCCESS(imodelResult);
    iModelInfoPtr info = imodelResult.GetValue();
    auto deleteResult = s_client->DeleteiModel(s_projectId, *info)->GetResult();
    EXPECT_SUCCESS(deleteResult);

    TestsProgressCallback callback;
    auto result = s_client->AcquireBriefcase(*info, OutputDir(), false, callback.Get())->GetResult();

    ASSERT_FAILURE(result);
    callback.Verify(false);
    EXPECT_EQ(Error::Id::iModelDoesNotExist, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, AcquireAfterQueryingById)
    {
    auto getResult = s_client->GetiModelById(s_projectId, s_info->GetId())->GetResult();
    ASSERT_SUCCESS(getResult);
    EXPECT_EQ(getResult.GetValue()->GetUserCreated(), getResult.GetValue()->GetOwnerInfo()->GetId());

    TestsProgressCallback callback;
    BriefcaseInfoResult acquireResult = s_client->AcquireBriefcaseToDir(*getResult.GetValue(), OutputDir(), false, Client::DefaultFileNameCallback, callback.Get())->GetResult();
    EXPECT_SUCCESS(acquireResult);
    callback.Verify();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, AcquireAfterQueryingByName)
    {
    auto getResult = s_client->GetiModelByName(s_projectId, s_info->GetName())->GetResult();
    EXPECT_SUCCESS(getResult);
    EXPECT_EQ(getResult.GetValue()->GetUserCreated(), getResult.GetValue()->GetOwnerInfo()->GetId());

    TestsProgressCallback callback;
    auto acquireResult = s_client->AcquireBriefcaseToDir(*getResult.GetValue(), OutputDir(), false, Client::DefaultFileNameCallback, callback.Get())->GetResult();
    EXPECT_SUCCESS(acquireResult);
    callback.Verify();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, AcquireBriefcaseId)
    {
    auto briefcaseResult = s_connection->AcquireNewBriefcase()->GetResult();
    ASSERT_SUCCESS(briefcaseResult);
    BeBriefcaseId firstId = briefcaseResult.GetValue()->GetId();

    briefcaseResult = s_connection->AcquireNewBriefcase()->GetResult();
    ASSERT_SUCCESS(briefcaseResult);
    EXPECT_EQ(BeSQLite::BeBriefcaseId(firstId.GetValue() + 1), briefcaseResult.GetValue()->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, AbandonExistingBriefcase)
    {
    BriefcaseInfoResult briefcaseResult = s_connection->AcquireNewBriefcase()->GetResult();
    ASSERT_SUCCESS(briefcaseResult);

    StatusResult abandonResult = s_client->AbandonBriefcase(*s_info, briefcaseResult.GetValue()->GetId())->GetResult();
    EXPECT_SUCCESS(abandonResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, AbandonNonExistentBriefcase)
    {
    StatusResult abandonResult = s_client->AbandonBriefcase(*s_info, BeSQLite::BeBriefcaseId(100))->GetResult();
    ASSERT_FAILURE(abandonResult);
    EXPECT_EQ(Error::Id::BriefcaseDoesNotExist, abandonResult.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, AbandonSingleBriefcase)
    {
    //Acquire first briefcase
    BriefcaseInfoResult briefcaseResult = s_connection->AcquireNewBriefcase()->GetResult();
    ASSERT_SUCCESS(briefcaseResult);

    //Acquire second briefcase
    BriefcaseInfoResult briefcaseResult2 = s_connection->AcquireNewBriefcase()->GetResult();
    ASSERT_SUCCESS(briefcaseResult2);

    //Abandon the first briefcase
    StatusResult abandonResult = s_client->AbandonBriefcase(*s_info, briefcaseResult.GetValue()->GetId())->GetResult();
    ASSERT_SUCCESS(abandonResult);

    //Check remaining briefcases
    BriefcasesInfoResult queryResult = s_connection->QueryAllBriefcasesInfo()->GetResult();
    ASSERT_SUCCESS(queryResult);
    ASSERT_EQ(1, queryResult.GetValue().size());
    EXPECT_EQ(briefcaseResult2.GetValue()->GetId(), queryResult.GetValue()[0]->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, AdminAbandonOtherUserBriefcase)
    {
    ClientPtr nonAdminClient;
    iModelHubHelpers::CreateClient(nonAdminClient, IntegrationTestsSettings::Instance().GetValidNonAdminCredentials());

    iModelConnectionResult connectionResult = nonAdminClient->ConnectToiModel(*s_info)->GetResult();
    ASSERT_SUCCESS(connectionResult);
    iModelConnectionPtr connection = connectionResult.GetValue();

    BriefcaseInfoResult briefcaseResult = connection->AcquireNewBriefcase()->GetResult();
    ASSERT_SUCCESS(briefcaseResult);

    EXPECT_SUCCESS(s_client->AbandonBriefcase(*s_info, briefcaseResult.GetValue()->GetId())->GetResult());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, NonAdminAbandonOtherUserBriefcase)
    {
    ClientPtr nonAdminClient;
    iModelHubHelpers::CreateClient(nonAdminClient, IntegrationTestsSettings::Instance().GetValidNonAdminCredentials());

    BriefcaseInfoResult briefcaseResult = s_connection->AcquireNewBriefcase()->GetResult();
    ASSERT_SUCCESS(briefcaseResult);

    StatusResult abandonResult = nonAdminClient->AbandonBriefcase(*s_info, briefcaseResult.GetValue()->GetId())->GetResult();
    ASSERT_FAILURE(abandonResult);
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, abandonResult.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, SuccessfulOpenBriefcase)
    {
    BriefcaseInfoResult acquireResult = iModelHubHelpers::AcquireBriefcase(s_client, s_info);
    ASSERT_SUCCESS(acquireResult);
    DgnDbPtr db;
    OpenDgnDb(db, acquireResult.GetValue()->GetLocalPath());
    EXPECT_TRUE(db->GetBriefcaseId().IsValid());
    EXPECT_FALSE(db->GetBriefcaseId().IsStandAloneId());
    EXPECT_FALSE(db->GetBriefcaseId().IsSnapshot());

    iModelHubHelpers::OpenBriefcase(s_client, db);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, OpenSeedFileAsBriefcase)
    {
    DgnDbPtr db = CreateTestDb();
    ASSERT_TRUE(db.IsValid());

    iModelResult createResult = CreateiModel(db);
    ASSERT_SUCCESS(createResult);

    BriefcaseResult openResult = iModelHubHelpers::OpenBriefcase(s_client, db, false, false);
    EXPECT_EQ(Error::Id::FileIsNotBriefcase, openResult.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, SuccessfulRestoreBriefcase)
    {
    BriefcaseInfoResult briefcaseResult = s_connection->AcquireNewBriefcase()->GetResult();
    ASSERT_SUCCESS(briefcaseResult);

    TestsProgressCallback callback;
    auto result = s_client->RestoreBriefcase(*s_info, briefcaseResult.GetValue()->GetId(), OutputDir(), false, Client::DefaultFileNameCallback, callback.Get())->GetResult();
    ASSERT_SUCCESS(result);
    callback.Verify();

    auto dbPath = result.GetValue()->GetLocalPath();
    EXPECT_TRUE(dbPath.DoesPathExist());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, UnsuccessfulRestoreBriefcase)
    {
    TestsProgressCallback callback;
    auto result = s_client->RestoreBriefcase(*s_info, BeSQLite::BeBriefcaseId(100), OutputDir(), false, Client::DefaultFileNameCallback, callback.Get())->GetResult();

    ASSERT_FAILURE(result);
    callback.Verify(false);
    EXPECT_EQ(Error::Id::BriefcaseDoesNotExist, result.GetError().GetId()) << "TFS#804283";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    julius.cepukenas                 08/2016
//---------------------------------------------------------------------------------------
TEST_F(BriefcaseTests, QueryInformationAboutSpecificBriefcase)
    {
    BriefcaseInfoResult acquireResult = s_connection->AcquireNewBriefcase()->GetResult();
    ASSERT_SUCCESS(acquireResult);
    BriefcaseInfoPtr info = acquireResult.GetValue();

    //Check if only one Briefcase is retrieved
    BriefcaseInfoResult result = s_connection->QueryBriefcaseInfo(info->GetId())->GetResult();
    EXPECT_SUCCESS(result);

    //Check if Briefcase information contains correct data
    BriefcaseInfoPtr briefcaseInfo = result.GetValue();
    EXPECT_EQ(info->GetId(), briefcaseInfo->GetId());

    // Previously UserOwned was an email. Now it is UserId, we don't have it here
    EXPECT_GT(briefcaseInfo->GetUserOwned().size(), 0);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    julius.cepukenas                 08/2016
//---------------------------------------------------------------------------------------
TEST_F(BriefcaseTests, QueryInformationAboutInvalidBriefcase)
    {
    BriefcaseInfoResult result = s_connection->QueryBriefcaseInfo(BeSQLite::BeBriefcaseId())->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::InvalidBriefcase, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, QueryInformationAboutCheckpointSnapshot)
    {
    BriefcaseInfoResult result = s_connection->QueryBriefcaseInfo(BeBriefcaseId(BeBriefcaseId::CheckpointSnapshot()))->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::InvalidBriefcase, result.GetError().GetId()) << "TFS#804283";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Shaun.Sewall                    03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, QueryInformationAboutStandAlone)
    {
    BriefcaseInfoResult result = s_connection->QueryBriefcaseInfo(BeBriefcaseId(BeBriefcaseId::StandAlone()))->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::InvalidBriefcase, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Shaun.Sewall                    03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, QueryInformationAboutSnapshot)
    {
    BriefcaseInfoResult result = s_connection->QueryBriefcaseInfo(BeBriefcaseId(BeBriefcaseId::Snapshot()))->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::InvalidBriefcase, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseTests, QueryInformationAboutNonexistentBriefcase)
    {
    BriefcaseInfoResult result = s_connection->QueryBriefcaseInfo(BeSQLite::BeBriefcaseId(100))->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::BriefcaseDoesNotExist, result.GetError().GetId()) << "TFS#804283";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    julius.cepukenas                 08/2016
//---------------------------------------------------------------------------------------
TEST_F(BriefcaseTests, QueryInformationAboutAllAvailableBriefcases)
    {
    //For this test, the information contained in briefcases information is not tested
    //Such test is performed in QueryInformationAboutSpecificBriefcase
    //This test is in order to verify the functionality of vorious briefcase information query option

    //Query briefcases info when no briefcases are acquired
    auto result = s_connection->QueryAllBriefcasesInfo()->GetResult();
    EXPECT_SUCCESS(result);
    EXPECT_EQ(0, result.GetValue().size());

    ClientPtr nonAdminClient = CreateNonAdminClient();
    iModelConnectionPtr connection = nonAdminClient->ConnectToiModel(*s_info)->GetResult().GetValue();
    //Acquire three briefcases
    EXPECT_SUCCESS(s_connection->AcquireNewBriefcase()->GetResult());
    EXPECT_SUCCESS(s_connection->AcquireNewBriefcase()->GetResult());
    EXPECT_SUCCESS(connection->AcquireNewBriefcase()->GetResult());

    //Query information about all acquired briefcases
    result = s_connection->QueryAllBriefcasesInfo()->GetResult();
    EXPECT_SUCCESS(result);
    EXPECT_EQ(3, result.GetValue().size());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    julius.cepukenas                 08/2016
//---------------------------------------------------------------------------------------
TEST_F(BriefcaseTests, QueryInformationAboutASubSetOfAvailableBriefcases)
    {
    //Setup connection
    ClientPtr nonAdminClient = CreateNonAdminClient();
    iModelConnectionPtr connection = nonAdminClient->ConnectToiModel(*s_info)->GetResult().GetValue();

    //Acquire three briefcases
    BriefcaseInfoResult briefcaseResult1 = s_connection->AcquireNewBriefcase()->GetResult();
    ASSERT_SUCCESS(briefcaseResult1);
    EXPECT_SUCCESS(s_connection->AcquireNewBriefcase()->GetResult());
    BriefcaseInfoResult briefcaseResult3 = connection->AcquireNewBriefcase()->GetResult();
    EXPECT_SUCCESS(briefcaseResult3);

    bvector<BeSQLite::BeBriefcaseId> queryBriefcases;
    //Query information with empty set of briefcases
    auto result = s_connection->QueryBriefcasesInfo(queryBriefcases)->GetResult();
    EXPECT_SUCCESS(result);
    EXPECT_EQ(0, result.GetValue().size());

    //Query information about subset of available briefcases
    queryBriefcases.push_back(briefcaseResult1.GetValue()->GetId());
    queryBriefcases.push_back(briefcaseResult3.GetValue()->GetId());

    result = s_connection->QueryBriefcasesInfo(queryBriefcases)->GetResult();
    EXPECT_SUCCESS(result);
    EXPECT_EQ(2, result.GetValue().size());
    }
