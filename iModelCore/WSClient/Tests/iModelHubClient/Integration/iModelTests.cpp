/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Vilius.Kazlauskas               09/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    iModelResult CreateiModel(iModelCreateInfoPtr imodelCreateInfo, bool expectSuccess = true)
        {
        return IntegrationTestsBase::CreateiModel(m_db, imodelCreateInfo, expectSuccess);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             10/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    iModelInfoPtr GetOldestiModel()
        {
        iModelsResult iModelsList = s_client->GetiModels(s_projectId, nullptr, false)->GetResult();
        iModelInfoPtr oldestiModel = nullptr;
        for (auto imodel : iModelsList.GetValue())
            {
            if (oldestiModel.IsNull())
                {
                oldestiModel = imodel;
                continue;
                }

            if (DateTime::Compare(imodel->GetCreatedDate(), oldestiModel->GetCreatedDate()) == DateTime::CompareResult::EarlierThan)
                {
                oldestiModel = imodel;
                }
            }

        return oldestiModel;
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, SuccessfulCreateiModel)
    {
    iModelsResult iModelsList = s_client->GetiModels(s_projectId)->GetResult();
    ASSERT_SUCCESS(iModelsList);

    // If there are no iModels in the project, primary iModel query should return error
    iModelResult primaryiModel;
    if (iModelsList.GetValue().size() == 0)
        {
        primaryiModel = s_client->GetiModel(s_projectId)->GetResult();
        ASSERT_FAILURE(primaryiModel);
        EXPECT_EQ(Error::Id::iModelDoesNotExist, primaryiModel.GetError().GetId());
        }

    // Create first iModel
    m_imodelName = GetTestiModelName();
    iModelCreateInfoPtr imodelCreateInfo = iModelCreateInfo::Create(m_imodelName, "", bvector<double> { 1, 2, 3, 4 });
    iModelResult createResult = CreateiModel(imodelCreateInfo);
    auto creatediModelId = createResult.GetValue()->GetId();

    iModelResult getResult = s_client->GetiModelById(s_projectId, creatediModelId)->GetResult();
    ASSERT_SUCCESS(getResult) << "Needs defect filed";
    EXPECT_TRUE(getResult.GetValue()->IsInitialized());
    EXPECT_EQ(getResult.GetValue()->GetUserCreated(), getResult.GetValue()->GetOwnerInfo()->GetId());
    EXPECT_EQ(getResult.GetValue()->GetExtent(), imodelCreateInfo->GetExtent());

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

    // First iModel should be primary
    auto oldestiModelId = GetOldestiModel()->GetId();
    primaryiModel = s_client->GetiModel(s_projectId)->GetResult();
    auto primaryiModelId = primaryiModel.GetValue()->GetId();
    ASSERT_SUCCESS(primaryiModel);
    EXPECT_EQ(oldestiModelId, primaryiModelId);

    // Create second iModel
    iModelResult createResult2 = CreateiModel("Primary2", "Primary2Description");
    ASSERT_SUCCESS(createResult2);

    // First iModel should be primary
    primaryiModel = s_client->GetiModel(s_projectId)->GetResult();
    ASSERT_SUCCESS(primaryiModel);
    EXPECT_EQ(oldestiModelId, primaryiModelId);

    s_client->DeleteiModel(s_projectId, *createResult2.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, CreateEmptyiModel)
    {
    // Create with invalid name
    iModelCreateInfoPtr imodelCreateInfo = iModelCreateInfo::Create("", "", bvector<double> { 1, 2, 3, 4 });
    iModelResult createResult = CreateEmptyiModel(imodelCreateInfo, false);
    ASSERT_FAILURE(createResult);
    EXPECT_EQ(Error::Id::InvalidiModelName, createResult.GetError().GetId());

    // Create with invalid extent
    m_imodelName = GetTestiModelName();
    imodelCreateInfo = iModelCreateInfo::Create(m_imodelName, "", bvector<double> { 1 });
    createResult = CreateEmptyiModel(imodelCreateInfo, false);
    ASSERT_FAILURE(createResult);
    EXPECT_EQ(Error::Id::InvalidiModelExtentCount, createResult.GetError().GetId());

    // Create with invalid extent coordinate
    imodelCreateInfo = iModelCreateInfo::Create(m_imodelName, "", bvector<double> { 1, -200, 3, 4 });
    createResult = CreateEmptyiModel(imodelCreateInfo, false);
    ASSERT_FAILURE(createResult);
    EXPECT_EQ(Error::Id::InvalidiModelExtentCoordinate, createResult.GetError().GetId());

    m_imodelName = GetTestiModelName();
    imodelCreateInfo = iModelCreateInfo::Create(m_imodelName, "", bvector<double> { 1, 2, 3, 4 });
    createResult = CreateEmptyiModel(imodelCreateInfo);
    iModelInfoPtr creatediModel = createResult.GetValue();

    iModelResult getResult = s_client->GetiModelByName(s_projectId, m_imodelName)->GetResult();
    ASSERT_SUCCESS(getResult);

    EXPECT_TRUE(creatediModel->IsInitialized());
    EXPECT_EQ(creatediModel->GetUserCreated(), getResult.GetValue()->GetOwnerInfo()->GetId());
    DateTime compareDate(DateTime::Kind::Utc, 2019, 1, 1, 0, 0, 0, 0);
    EXPECT_EQ((int)DateTime::CompareResult::EarlierThan, (int)DateTime::Compare(compareDate, creatediModel->GetCreatedDate()));
    EXPECT_EQ(creatediModel->GetExtent(), imodelCreateInfo->GetExtent());

    creatediModel->SetName(GetTestiModelName() + "_Renamed");
    ASSERT_SUCCESS(iModelHubHelpers::DeleteiModelByName(s_client, creatediModel->GetName()));
    creatediModel->SetDescription("Description_Renamed");
    creatediModel->SetExtent(bvector<double>{ 5, 6, 7, 8 });
    ASSERT_SUCCESS(s_client->UpdateiModel(s_projectId, *creatediModel)->GetResult());

    getResult = s_client->GetiModelByName(s_projectId, creatediModel->GetName())->GetResult();
    ASSERT_SUCCESS(getResult);
    EXPECT_EQ(creatediModel->GetName(), getResult.GetValue()->GetName());
    EXPECT_EQ(creatediModel->GetDescription(), getResult.GetValue()->GetDescription());
    EXPECT_EQ(creatediModel->GetExtent(), getResult.GetValue()->GetExtent());

    ASSERT_SUCCESS(iModelHubHelpers::DeleteiModelByName(s_client, creatediModel->GetName()));
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
    iModelResult result = iModelHubHelpers::CreateNewiModel(s_client, m_db, BeSQLite::BeGuid(true).ToString(), false);
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
    iModelsResult result = s_client->GetiModels(BeSQLite::BeGuid(true).ToString())->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::FailedToGetProjectPermissions, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, GetiModelsInvalidContextId)
    {
    iModelsResult result = s_client->GetiModels("badContextId")->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::InvalidContextId, result.GetError().GetId());
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
* @bsimethod                                    Karolis.Dziedzelis              09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, CloneEmptyiModel)
    {
    m_imodelName = GetTestiModelName();
    iModelCreateInfoPtr imodelCreateInfo = iModelCreateInfo::Create(m_imodelName, "", bvector<double> { 1, 2, 3, 4 });
    iModelResult createResult = CreateEmptyiModel(imodelCreateInfo, true);
    iModelInfoPtr info = createResult.GetValue();

    Utf8String imodel2Name = Utf8PrintfString("%s-%d", m_imodelName.c_str(), 2);
    iModelHubHelpers::DeleteiModelByName(s_client, imodel2Name);
    iModelCreateInfoPtr imodelCreateInfo2 = iModelCreateInfo::Create(imodel2Name, "", bvector<double> { 1, 2, 3, 4 });
    iModelResult create2Result = s_client->CloneiModel(s_projectId, info->GetId(), "", imodelCreateInfo2)->GetResult();
    ASSERT_SUCCESS(create2Result);

    iModelResult imodelResult = s_client->GetiModelById(s_projectId, create2Result.GetValue()->GetId())->GetResult();
    ASSERT_SUCCESS(imodelResult);
    EXPECT_EQ(Utf8PrintfString("%s:", info->GetId().c_str()), imodelResult.GetValue()->GetTemplate());
    iModelHubHelpers::DeleteiModelByName(s_client, imodel2Name);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, CloneWithChangeSets)
    {
    m_imodelName = GetTestiModelName();
    iModelCreateInfoPtr imodelCreateInfo = iModelCreateInfo::Create(m_imodelName, "", bvector<double> { 1, 2, 3, 4 });
    iModelResult createResult = CreateEmptyiModel(imodelCreateInfo, true);
    iModelInfoPtr info = createResult.GetValue();

    BriefcasePtr briefcase = iModelHubHelpers::AcquireAndAddChangeSets(s_client, info, 1);
    Utf8String changeSetId = briefcase->GetLastChangeSetPulled();

    Utf8String imodel2Name = Utf8PrintfString("%s-%d", m_imodelName.c_str(), 2);
    iModelHubHelpers::DeleteiModelByName(s_client, imodel2Name);
    iModelCreateInfoPtr imodelCreateInfo2 = iModelCreateInfo::Create(imodel2Name, "", bvector<double> { 1, 2, 3, 4 });
    iModelResult create2Result = s_client->CloneiModel(s_projectId, info->GetId(), changeSetId, imodelCreateInfo2)->GetResult();
    ASSERT_SUCCESS(create2Result);

    iModelResult imodelResult = s_client->GetiModelById(s_projectId, create2Result.GetValue()->GetId())->GetResult();
    ASSERT_SUCCESS(imodelResult);
    EXPECT_EQ(Utf8PrintfString("%s:%s", info->GetId().c_str(), changeSetId.c_str()), imodelResult.GetValue()->GetTemplate());
    iModelHubHelpers::DeleteiModelByName(s_client, imodel2Name);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vilius.Kazlauskas               09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelTests, UpdateiModel)
    {
    m_imodelName = GetTestiModelName();
    iModelCreateInfoPtr imodelCreateInfo = iModelCreateInfo::Create(m_imodelName, "", bvector<double> { 1, 2, 3, 4 });
    iModelResult createResult = CreateiModel(imodelCreateInfo);
    ASSERT_SUCCESS(createResult);

    Utf8String name2 = "TestiModel2";
    ASSERT_SUCCESS(CreateiModel(name2));
    
    iModelResult imodelResult = s_client->GetiModelById(s_projectId, createResult.GetValue()->GetId())->GetResult();
    ASSERT_SUCCESS(imodelResult);

    // Rename to existing name
    iModelInfoPtr imodel = imodelResult.GetValue();
    imodel->SetName(name2);
    StatusResult updateResult = s_client->UpdateiModel(s_projectId, *imodel)->GetResult();
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::iModelAlreadyExists, updateResult.GetError().GetId());
    // Clean up
    iModelHubHelpers::DeleteiModelByName(s_client, name2);
    
    // Rename to invalid name
    imodel->SetName("");
    updateResult = s_client->UpdateiModel(s_projectId, *imodel)->GetResult();
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::MissingRequiredProperties, updateResult.GetError().GetId());

    // Change extent to one with invalid count
    imodel->SetName(m_imodelName);
    imodel->SetExtent(bvector<double> { 1 });
    updateResult = s_client->UpdateiModel(s_projectId, *imodel)->GetResult();
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::InvalidPropertiesValues, updateResult.GetError().GetId());

    // Change extent with invalid coordiante
    imodel->SetExtent(bvector<double> { 1, -200, 3, 4 });
    updateResult = s_client->UpdateiModel(s_projectId, *imodel)->GetResult();
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::InvalidPropertiesValues, updateResult.GetError().GetId());

    // Edit iModel with a valid data
    m_imodelName = GetTestiModelName() + "_Renamed";
    Utf8String description = m_imodelName + "_Description";
    ASSERT_SUCCESS(iModelHubHelpers::DeleteiModelByName(s_client, m_imodelName));
    imodel->SetName(m_imodelName);
    imodel->SetDescription(description);
    imodel->SetExtent(bvector<double> { 5, 6, 7, 8 });
    ASSERT_SUCCESS(s_client->UpdateiModel(s_projectId, *imodel)->GetResult());

    // Check if renamed properly and other properites have not changed
    iModelResult updatediModelResult = s_client->GetiModelById(s_projectId, imodel->GetId())->GetResult();
    ASSERT_SUCCESS(updatediModelResult);
    iModelInfoPtr updatediModel = updatediModelResult.GetValue();
    EXPECT_EQ(imodel->GetName(), updatediModel->GetName());
    EXPECT_EQ(imodel->GetDescription(), updatediModel->GetDescription());
    EXPECT_EQ(imodel->GetExtent(), updatediModel->GetExtent());
    EXPECT_EQ(imodel->GetId(), updatediModel->GetId());
    EXPECT_EQ(imodel->GetCreatedDate(), updatediModel->GetCreatedDate());
    EXPECT_EQ(imodel->GetUserCreated(), updatediModel->GetUserCreated());
    EXPECT_EQ(imodel->GetServerURL(), updatediModel->GetServerURL());
    }
