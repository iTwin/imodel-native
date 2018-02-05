/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/iModelTests.cpp $
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
struct iModelTests : public IntegrationTestsBase
    {
    DgnDbPtr     m_db;

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
* @bsimethod                                    Algirdas.Mikoliunas             07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, SuccessfulCreateiModel)
    {
    iModelResult createResult = CreateiModel();

    iModelResult getResult = s_client->GetiModelById(s_projectId, createResult.GetValue()->GetId())->GetResult();
    ASSERT_SUCCESS(getResult) << "Needs defect filed";
    EXPECT_EQ(getResult.GetValue()->GetUserCreated(), getResult.GetValue()->GetOwnerInfo()->GetId());

    auto queryResult = s_client->GetiModels(s_projectId)->GetResult();
    ASSERT_SUCCESS(queryResult);
    bvector<iModelInfoPtr>& imodels = queryResult.GetValue();
    DateTime compareDate (DateTime::Kind::Utc, 2018, 1, 1, 0, 0, 0, 0);
    for (iModelInfoPtr imodel : imodels)
        {
        EXPECT_FALSE(imodel->GetServerURL().empty());
        EXPECT_FALSE(imodel->GetId().empty());
        EXPECT_FALSE(imodel->GetName().empty());

        DateTimeCR createdDate = imodel->GetCreatedDate();
        EXPECT_TRUE(createdDate.IsValid());
        EXPECT_EQ((int)DateTime::CompareResult::EarlierThan, (int)DateTime::Compare(compareDate, createdDate));
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, UnauthorizedCreateiModel)
    {
    ClientPtr client;
    iModelHubHelpers::CreateClient(client, IntegrationTestsSettings::Instance().GetValidNonAdminCredentials());
    iModelResult result = iModelHubHelpers::CreateNewiModel(client, m_db, s_projectId, false);
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, RepeatedCreateiModel)
    {
    iModelResult result = CreateiModel();
    result = CreateiModel(false);
    EXPECT_EQ(Error::Id::iModelAlreadyExists, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, CancelCreateiModel)
    {
    // Act
    SimpleCancellationTokenPtr cancellationToken = SimpleCancellationToken::Create();
    iModelTaskPtr createiModelTask = s_client->CreateNewiModel(s_projectId, *m_db, true, nullptr, cancellationToken);
    createiModelTask->Execute();
    cancellationToken->SetCanceled();
    createiModelTask->Wait();

    // Assert
    iModelResult cancelledResult = createiModelTask->GetResult();
    EXPECT_FAILURE(cancelledResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, UnsuccessfulCreateiModel)
    {
    iModelResult result = iModelHubHelpers::CreateNewiModel(s_client, m_db, "bad project", false);
    EXPECT_EQ(Error::Id::FailedToGetProjectPermissions, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, QueryNonExistentiModelById)
    {
    iModelResult getResult = s_client->GetiModelById(s_projectId, BeSQLite::BeGuid(true).ToString())->GetResult();
    ASSERT_FAILURE(getResult);
    EXPECT_EQ(Error::Id::iModelDoesNotExist, getResult.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, QueryNonExistentiModelByName)
    {
    iModelResult getResult = s_client->GetiModelByName(s_projectId, "bad imodel")->GetResult();
    ASSERT_FAILURE(getResult);
    EXPECT_EQ(Error::Id::iModelDoesNotExist, getResult.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, UnsuccessfulGetiModels)
    {
    iModelsResult result = s_client->GetiModels("bad project")->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::FailedToGetProjectPermissions, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, SuccessfulCreateiModelWithASpaceInName)
    {
    TestsProgressCallback callback;
    m_imodelName = "iModel With A Space In Name";
    Utf8String description = m_imodelName + " is created by iModelHubHost";
    ASSERT_SUCCESS(iModelHubHelpers::DeleteiModelByName(s_client, m_imodelName));

    iModelResult createResult = s_client->CreateNewiModel(s_projectId, *m_db, m_imodelName, description, true, callback.Get())->GetResult();
    ASSERT_SUCCESS(createResult);
    callback.Verify(true);

    iModelResult getResult = s_client->GetiModelByName(s_projectId, m_imodelName)->GetResult(); //Query by name fails
    ASSERT_SUCCESS(getResult);
    EXPECT_EQ(getResult.GetValue()->GetUserCreated(), getResult.GetValue()->GetOwnerInfo()->GetId());
    }
