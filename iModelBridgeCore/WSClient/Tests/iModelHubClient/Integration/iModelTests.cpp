/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/iModelTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"
#include <WebServices\iModelHub\Client\ClientHelper.h>
#include "../Helpers/Oidc/OidcSignInManager.h"

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

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Vilius.Kazlauskas               09/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    iModelResult CreateiModel(Utf8StringCR name, Utf8StringCR description = "")
        {
        TestsProgressCallback callback;
        iModelHubHelpers::DeleteiModelByName(s_client, name);
        iModelResult createResult = s_client->CreateNewiModel(s_projectId, *m_db, name, description, true, callback.Get())->GetResult();        
        callback.Verify(true);
        return createResult;
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
    EXPECT_TRUE(getResult.GetValue()->IsInitialized());
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
* @bsimethod                                    Karolis.Dziedzelis              03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, FilterGetiModels)
    {
    Utf8String imodelName = GetTestiModelName();
    iModelResult createResult;
    iModelHubHelpers::CreateUninitializediModel(createResult, s_client, s_projectId, imodelName);

    iModelsResult filteredResult = s_client->GetiModels(s_projectId, nullptr, true)->GetResult();
    ASSERT_SUCCESS(filteredResult);
    iModelsResult unfilteredResult = s_client->GetiModels(s_projectId, nullptr, false)->GetResult();
    ASSERT_SUCCESS(unfilteredResult);
    ASSERT_GT(unfilteredResult.GetValue().size(), filteredResult.GetValue().size());
    for (iModelInfoPtr info : filteredResult.GetValue())
        EXPECT_NE(imodelName, info->GetName());
    bool found = false;
    for (iModelInfoPtr info : unfilteredResult.GetValue())
        if (imodelName == info->GetName())
            found = true;
    EXPECT_TRUE(found);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, SuccessfulCreateiModelWithASpaceInName)
    {
    m_imodelName = "iModel With A Space In Name";
    Utf8String description = m_imodelName + " is created by iModelHubHost";
    ASSERT_SUCCESS(CreateiModel(m_imodelName, description));

    iModelResult getResult = s_client->GetiModelByName(s_projectId, m_imodelName)->GetResult(); //Query by name fails
    ASSERT_SUCCESS(getResult);
    EXPECT_EQ(getResult.GetValue()->GetUserCreated(), getResult.GetValue()->GetOwnerInfo()->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, UnsuccessfulCreateiModelFromBriefcase)
    {
    TestsProgressCallback callback;
    m_imodelName = GetTestiModelName();
    Utf8String description = m_imodelName + " is created by iModelHubHost";
    m_db->SetAsBriefcase(BeSQLite::BeBriefcaseId(2));

    iModelResult createResult = s_client->CreateNewiModel(s_projectId, *m_db, m_imodelName, description, true, callback.Get())->GetResult();
    ASSERT_FAILURE(createResult);
    EXPECT_EQ(Error::Id::FileIsBriefcase, createResult.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, CreateiModelUsingOidcLogin)
    {
    ClientPtr oidcClient;
    iModelHubHelpers::CreateOidcClient(oidcClient, IntegrationTestsSettings::Instance().GetValidAdminCredentials());
    
    iModelResult createResult = iModelHubHelpers::CreateNewiModel(oidcClient, m_db, s_projectId, true);
    ASSERT_SUCCESS(createResult) << "iModel create using Oidc login failed";
    auto creatediModelInfo = createResult.GetValue();
    auto creatediModelId = creatediModelInfo->GetId();

    iModelResult getResult = oidcClient->GetiModelById(s_projectId, creatediModelId)->GetResult();
    ASSERT_SUCCESS(getResult) << "iModel creation using Oidc login failed";
    EXPECT_TRUE(getResult.GetValue()->IsInitialized());
    EXPECT_EQ(getResult.GetValue()->GetUserCreated(), getResult.GetValue()->GetOwnerInfo()->GetId());

    EXPECT_SUCCESS (oidcClient->DeleteiModel(s_projectId, *creatediModelInfo)->GetResult());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vilius.Kazlauskas               09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, UpdateiModel)
    {
    iModelResult createResult = CreateiModel();
    ASSERT_SUCCESS(createResult);

    Utf8String name2 = "TestiModel2";
    ASSERT_SUCCESS(CreateiModel(name2));
    
    iModelResult imodelResult = s_client->GetiModelById(s_projectId, createResult.GetValue()->GetId())->GetResult();
    ASSERT_SUCCESS(imodelResult);

    // Rename to existing name
    iModelInfoPtr imodel = imodelResult.GetValue();
    imodel->SetName(name2);
    StatusResult updateResult1 = s_client->UpdateiModel(s_projectId, *imodel)->GetResult();
    ASSERT_FAILURE(updateResult1);
    EXPECT_EQ(Error::Id::iModelAlreadyExists, updateResult1.GetError().GetId());
    // Clean up
    iModelHubHelpers::DeleteiModelByName(s_client, name2);
    
    // Rename to invalid name
    imodel->SetName("");
    StatusResult updateResult2 = s_client->UpdateiModel(s_projectId, *imodel)->GetResult();
    ASSERT_FAILURE(updateResult2);
    EXPECT_EQ(Error::Id::MissingRequiredProperties, updateResult2.GetError().GetId());

    // Rename with a valid data
    m_imodelName = GetTestiModelName() + "_Renamed";
    Utf8String description = m_imodelName + "_Description";
    ASSERT_SUCCESS(iModelHubHelpers::DeleteiModelByName(s_client, m_imodelName));
    imodel->SetName(m_imodelName);
    imodel->SetDescription(description);
    ASSERT_SUCCESS(s_client->UpdateiModel(s_projectId, *imodel)->GetResult());

    // Check if renamed properly and other properites have not changed
    iModelResult updatediModelResult = s_client->GetiModelById(s_projectId, imodel->GetId())->GetResult();
    ASSERT_SUCCESS(updatediModelResult);
    iModelInfoPtr updatediModel = updatediModelResult.GetValue();
    EXPECT_EQ(m_imodelName, updatediModel->GetName());
    EXPECT_EQ(description, updatediModel->GetDescription());
    EXPECT_EQ(imodel->GetId(), updatediModel->GetId());
    EXPECT_EQ(imodel->GetCreatedDate(), updatediModel->GetCreatedDate());
    EXPECT_EQ(imodel->GetUserCreated(), updatediModel->GetUserCreated());
    EXPECT_EQ(imodel->GetServerURL(), updatediModel->GetServerURL());
    }
