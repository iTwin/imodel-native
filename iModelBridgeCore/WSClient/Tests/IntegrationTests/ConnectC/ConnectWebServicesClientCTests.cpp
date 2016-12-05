/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/ConnectC/ConnectWebServicesClientCTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ConnectWebServicesClientCTests.h"

static BeFileName s_temporaryDirectory, s_assetsRootDirectory;

void ConnectWebServicesClientCTests::SetUpTestCase()
    {
    WSClientBaseTest::SetUpTestCase();
    BeTest::GetHost().GetTempDir(s_temporaryDirectory);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(s_assetsRootDirectory);
    }

void ConnectWebServicesClientCTests::SetUp ()
    {
    auto proxy = ProxyHttpHandler::GetProxyIfReachable ("http://127.0.0.1:8888");
    if (proxy)
        m_fiddlerProxyUrl = L"http://127.0.0.1:8888";
    else
        m_fiddlerProxyUrl = L"";
    }

void ConnectWebServicesClientCTests::TearDown ()
    {
    m_fiddlerProxyUrl = nullptr;
    }


TEST_F (ConnectWebServicesClientCTests, Ctor_InvalidProxyUrl_ApiIsNull)
    {
    //NOTE: If Fiddler is running, and has been running for previous tests, this test will fail.
    //      To successfully run this test, restart Fiddler, or close out Fiddler entirely.
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        L"http://0.0.0.0:80",
        nullptr,
        nullptr,
        nullptr
        );
    ASSERT_TRUE (api == nullptr);
    }

TEST_F (ConnectWebServicesClientCTests, Ctor_InvalidProxyCredentialsWhenProxyCredentialsAreRequired_ApiIsNull)
    {
    //NOTE: If Fiddler is running, and has been running for previous tests, this test will probably fail.
    //      To successfully run this test, restart Fiddler AND make sure to Require Proxy Authentication!
    //This test is meant to ensure that a proxy, requiring authentication, will only successfully complete if authentication is correct.
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        L"Invalid",
        L"Invalid",
        nullptr
        );
    if (WString::IsNullOrEmpty(m_fiddlerProxyUrl.c_str()))
        ASSERT_FALSE (api == nullptr);
    else
        ASSERT_TRUE (api == nullptr);
    }

TEST_F(ConnectWebServicesClientCTests, Ctor_ValidParameters_SuccessfulInitialization)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str(),
        m_pmPassword.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        m_fiddlerProxyUrl.c_str(),
        m_fiddlerProxyUsername.c_str(),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    CallStatus status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientCTests, Ctor_InvalidCredentialsAndValidProductId_ApiIsNull)
    {
    WCharP password = L"password";
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str(),
        password,
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        m_fiddlerProxyUrl.c_str(),
        m_fiddlerProxyUsername.c_str(),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE(api == nullptr);
    }

TEST_F(ConnectWebServicesClientCTests, Ctor_ValidCredentialsAndInvalidProductId_ApiIsNull)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str(),
        m_pmPassword.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        L"9999",
        m_fiddlerProxyUrl.c_str(),
        m_fiddlerProxyUsername.c_str(),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE(api == nullptr);
    }

TEST_F (ConnectWebServicesClientCTests, Ctor_NoProxyUrlOrCredentials_ApiIsNotNull)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        nullptr,
        nullptr,
        nullptr,
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    CallStatus status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F (ConnectWebServicesClientCTests, ReadProject_ProjectExists_SuccessfulRetreival)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    WCharP instanceId = L"a8835f81-3f33-4457-bdea-79795aeb09fe";
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject (api, instanceId, &project);
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_DataBufferFree (api, project);
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F (ConnectWebServicesClientCTests, ReadProject_InvalidDataBufHandle_ErrorCodeReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    WCharP instanceId = L"a8835f81-3f33-4457-bdea-79795aeb09fe";
    CallStatus status = ConnectWebServicesClientC_ReadProject (api, instanceId, nullptr);
    ASSERT_TRUE (status == INVALID_PARAMETER);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientCTests, DataBufferGetCount_Only1ProjectIsReturned_SuccessfulRetreival)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    WCharP instanceId = L"a8835f81-3f33-4457-bdea-79795aeb09fe";
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject(api, instanceId, &project);
    ASSERT_TRUE(status == SUCCESS);
    uint64_t bufferCount = ConnectWebServicesClientC_DataBufferGetCount(project);
    ASSERT_TRUE(bufferCount == 1);

    status = ConnectWebServicesClientC_DataBufferFree(api, project);
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientCTests, GetPropertyMethods_Only1ProjectIsReturnedWithFulfilledProjectProperties_SuccessfulRetreivalOfProperties)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    WCharP instanceId = L"a8835f81-3f33-4457-bdea-79795aeb09fe";
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject(api, instanceId, &project);
    ASSERT_TRUE(status == SUCCESS);
    uint64_t bufferCount = ConnectWebServicesClientC_DataBufferGetCount(project);
    ASSERT_TRUE(bufferCount == 1);

    wchar_t stringBuf[4096];
    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECT_BUFF_NAME, 0, 4096, stringBuf);
    ASSERT_TRUE(status == SUCCESS);
    ASSERT_STREQ(stringBuf, L"cwsccDEV_pm1_testproject");

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECT_BUFF_NUMBER, 0, 4096, stringBuf);
    ASSERT_TRUE(status == SUCCESS);
    ASSERT_STREQ(stringBuf, L"cwsccDEV_pm1_testproject");

    double longitude;
    status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(api, project, PROJECT_BUFF_LONGITUDE, 0, &longitude);
    ASSERT_TRUE(status == PROPERTY_HAS_NOT_BEEN_SET);

    status = ConnectWebServicesClientC_DataBufferFree(api, project);
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientCTests, GetPropertyMethods_NULLBuffer_AppropriateStatusCodeReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    wchar_t stringBuf[4096];
    CWSCCDATABUFHANDLE buf = NULL;
    CallStatus status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, buf, PROJECT_BUFF_NAME, 0, 4096, stringBuf);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    size_t outSize;
    status = ConnectWebServicesClientC_DataBufferGetStringLength(api, buf, PROJECT_BUFF_NAME, 0, &outSize);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    int integer;
    status = ConnectWebServicesClientC_DataBufferGetIntProperty(api, buf, PROJECT_BUFF_STATUS, 0, &integer);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    double pDouble;
    status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(api, buf, PROJECT_BUFF_LONGITUDE, 0, &pDouble);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    int64_t pLong;
    status = ConnectWebServicesClientC_DataBufferGetLongProperty(api, buf, PROJECTMRUDETAIL_BUFF_LASTACCESSEDBYUSER, 0, &pLong);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    status = ConnectWebServicesClientC_DataBufferFree(api, buf);
    ASSERT_TRUE (status == INVALID_PARAMETER);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientCTests, GetPropertyMethods_BufferWithProjectTypeButInvalidPropertyType_AppropriateStatusCodeReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    WCharP instanceId = L"a8835f81-3f33-4457-bdea-79795aeb09fe";
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject(api, instanceId, &project);
    ASSERT_TRUE(status == SUCCESS);
    uint64_t bufferCount = ConnectWebServicesClientC_DataBufferGetCount(project);
    ASSERT_TRUE(bufferCount == 1);

    int64_t pLong;
    status = ConnectWebServicesClientC_DataBufferGetLongProperty(api, project, PROJECTMRUDETAIL_BUFF_LASTACCESSEDBYUSER, 0, &pLong);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    status = ConnectWebServicesClientC_DataBufferFree(api, project);
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientCTests, GetPropertyMethods_BufferWithProjectTypeAndValidPropertyTypeButInvalidProperty_AppropriateStatusCodeReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    WCharP instanceId = L"a8835f81-3f33-4457-bdea-79795aeb09fe";
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject(api, instanceId, &project);
    ASSERT_TRUE(status == SUCCESS);
    uint64_t bufferCount = ConnectWebServicesClientC_DataBufferGetCount(project);
    ASSERT_TRUE(bufferCount == 1);

    int16_t INVALID_PROPERTY = 0;

    wchar_t stringBuf[4096];
    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, INVALID_PROPERTY, 0, 4096, stringBuf);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    size_t outSize;
    status = ConnectWebServicesClientC_DataBufferGetStringLength(api, project, INVALID_PROPERTY, 0, &outSize);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    int integer;
    status = ConnectWebServicesClientC_DataBufferGetIntProperty(api, project, INVALID_PROPERTY, 0, &integer);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    double pDouble;
    status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(api, project, INVALID_PROPERTY, 0, &pDouble);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    status = ConnectWebServicesClientC_DataBufferFree(api, project);
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F (ConnectWebServicesClientCTests, CRUDProjectFunctions_CRUDsSuccessful_SuccessfulCodesReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    /************************************************************************************//**
    * \brief Create a new project
    * \param[in] apiHandle API object
    * \param[in] Name
    * \param[in] Number
    * \param[in] OrganizationId
    * \param[in] Active
    * \param[in] Industry
    * \param[in] AssetType
    * \param[in] LastModified
    * \param[in] Location
    * \param[in] Latitude
    * \param[in] Longitude
    * \param[in] LocationIsUsingLatLong
    * \param[in] RegisteredDate
    * \param[in] TimeZoneLocation
    * \param[in] Status
    * \param[in] PWDMInvitationId
    * \return Success or error code. See \ref clientErrorCodes
    ****************************************************************************************/
    BeGuid guid(true);
    WPrintfString Name(L"CWSCCTest%s", guid.ToString().c_str());
    WPrintfString Number(L"CWSCCTest%s", guid.ToString().c_str());
    WString OrgId = L"1001389117";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    CallStatus status = ConnectWebServicesClientC_CreateProject(api, 
                                            Name.c_str(),
                                            Number.c_str (),
                                            OrgId.c_str (),
                                            &Active,
                                            Industry.c_str (),
                                            AssetType.c_str (),
                                            nullptr,
                                            Location.c_str (),
                                            &lat,
                                            &lon,
                                            &LocationIsUsingLatLong,
                                            nullptr,
                                            nullptr,
                                            0,
                                            nullptr);

    ASSERT_TRUE (status == SUCCESS);

    auto instanceId = ConnectWebServicesClientC_GetLastCreatedObjectInstanceId (api);
    EXPECT_FALSE (Utf8String::IsNullOrEmpty (instanceId));

    WString wInstanceId;
    wInstanceId.AssignUtf8 (instanceId);
    CWSCCDATABUFHANDLE project;
    status = ConnectWebServicesClientC_ReadProject (api, wInstanceId.c_str(), &project);
    EXPECT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_DataBufferFree (api, project);
    EXPECT_TRUE (status == SUCCESS);

    BeGuid newGuid(true);
    WPrintfString NewName(L"CWSCCTest%s", newGuid.ToString().c_str());
    status = ConnectWebServicesClientC_UpdateProject (api,
                                                      wInstanceId.c_str (),
                                                      NewName.c_str(),
                                                      Number.c_str(),
                                                      nullptr,
                                                      nullptr,
                                                      Industry.c_str(),
                                                      AssetType.c_str (),
                                                      nullptr,
                                                      nullptr,
                                                      nullptr,
                                                      nullptr,
                                                      nullptr,
                                                      nullptr,
                                                      nullptr,
                                                      nullptr,
                                                      nullptr);
    EXPECT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_DeleteProject (api, wInstanceId.c_str ());
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F (ConnectWebServicesClientCTests, CRUDProjectV2Functions_CRUDsSuccessful_SuccessfulCodesReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

/************************************************************************************//**
* \brief Create a new project_v2
* \param[in] apiHandle API object
* \param[in] Name
* \param[in] Number
* \param[in] OrganizationId
* \param[in] Industry
* \param[in] AssetType
* \param[in] LastModified
* \param[in] Location
* \param[in] Latitude
* \param[in] Longitude
* \param[in] LocationIsUsingLatLong
* \param[in] RegisteredDate
* \param[in] TimeZoneLocation
* \param[in] Status
* \param[in] Data_Location_Guid
* \param[in] Country_Code
* \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
****************************************************************************************/
    BeGuid guid(true);
    WPrintfString Name(L"CWSCCTest%s", guid.ToString().c_str());
    WPrintfString Number(L"CWSCCTest%s", guid.ToString().c_str());
    WString OrgId = L"1001389117";
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    WString DataLocationGUID = L"99999999-9999-9999-9999-999999999999";
    WString CountryCode = L"ZZ";
    CallStatus status = ConnectWebServicesClientC_CreateProject_V2(api, 
                                            Name.c_str(),
                                            Number.c_str (),
                                            OrgId.c_str (),
                                            Industry.c_str (),
                                            AssetType.c_str (),
                                            nullptr,
                                            Location.c_str(),
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            DataLocationGUID.c_str(),
                                            CountryCode.c_str());

    ASSERT_TRUE (status == SUCCESS);

    auto instanceId = ConnectWebServicesClientC_GetLastCreatedObjectInstanceId (api);
    EXPECT_FALSE (Utf8String::IsNullOrEmpty (instanceId));

    WString wInstanceId;
    wInstanceId.AssignUtf8 (instanceId);
    CWSCCDATABUFHANDLE project;
    status = ConnectWebServicesClientC_ReadProject_V2 (api, wInstanceId.c_str(), &project);
    EXPECT_TRUE (status == SUCCESS);

    CWSCCDATABUFHANDLE projects;
    status = ConnectWebServicesClientC_ReadProject_V2List(api, &projects);
    EXPECT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_DataBufferFree (api, project);
    EXPECT_TRUE (status == SUCCESS);

    BeGuid newGuid(true);
    WPrintfString NewName(L"CWSCCTest%s", newGuid.ToString().c_str());
    status = ConnectWebServicesClientC_UpdateProject_V2 (api,
                                                      wInstanceId.c_str (),
                                                      NewName.c_str(),
                                                      Number.c_str(),
                                                      nullptr,
                                                      Industry.c_str(),
                                                      AssetType.c_str (),
                                                      nullptr,
                                                      nullptr,
                                                      nullptr,
                                                      nullptr,
                                                      nullptr,
                                                      nullptr,
                                                      nullptr,
                                                      nullptr,
                                                      DataLocationGUID.c_str(),
                                                      CountryCode.c_str());
    EXPECT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_DeleteProject_V2 (api, wInstanceId.c_str ());
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F (ConnectWebServicesClientCTests, CRUDOrganizationFunctions_CRUDsSuccessful_SuccessfulCodesReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    /************************************************************************************//**
    * \brief Create a new organization
    * \param[in] apiHandle API object
    * \param[in] OrganizationGuid
    * \param[in] OrganizationName
    * \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
    ****************************************************************************************/
    //BeGuid newGuid(true);
    //WPrintfString Name(L"CWSCCTest%s", newGuid.ToString().c_str());
    //WString OrganizationGuid;
    //OrganizationGuid.AssignUtf8(newGuid.ToString().c_str());
    ////NOTE: Creation works fine, but deletion doesn't, so I don't want to create endless organizations
    //CallStatus status = ConnectWebServicesClientC_CreateOrganization(api, 
    //                                                                 OrganizationGuid.c_str (),
    //                                                                 Name.c_str ());
    //ASSERT_TRUE (status == SUCCESS);

    //auto instanceId = ConnectWebServicesClientC_GetLastCreatedObjectInstanceId (api);
    //ASSERT_FALSE (Utf8String::IsNullOrEmpty (instanceId));

    WString wInstanceId = L"1001389117";
    CWSCCDATABUFHANDLE organization;
    CallStatus status = ConnectWebServicesClientC_ReadOrganization (api, wInstanceId.c_str (), &organization);
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_DataBufferFree (api, organization);
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F (ConnectWebServicesClientCTests, CRUDProjectFavoriteFunctions_CRUDsSuccessful_SuccessfulCodesReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    /************************************************************************************//**
    * \brief Create a new project
    * \param[in] apiHandle API object
    * \param[in] Name
    * \param[in] Number
    * \param[in] OrganizationId
    * \param[in] Active
    * \param[in] Industry
    * \param[in] AssetType
    * \param[in] LastModified
    * \param[in] Location
    * \param[in] Latitude
    * \param[in] Longitude
    * \param[in] LocationIsUsingLatLong
    * \param[in] RegisteredDate
    * \param[in] TimeZoneLocation
    * \param[in] Status
    * \param[in] PWDMInvitationId
    * \return Success or error code. See \ref clientErrorCodes
    ****************************************************************************************/
    BeGuid guid(true);
    WPrintfString Name(L"CWSCCTest%s", guid.ToString().c_str());
    WPrintfString Number(L"CWSCCTest%s", guid.ToString().c_str());
    WString OrgId = L"1001389117";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    CallStatus status = ConnectWebServicesClientC_CreateProject(api, 
                                            Name.c_str(),
                                            Number.c_str (),
                                            OrgId.c_str (),
                                            &Active,
                                            Industry.c_str (),
                                            AssetType.c_str (),
                                            nullptr,
                                            Location.c_str (),
                                            &lat,
                                            &lon,
                                            &LocationIsUsingLatLong,
                                            nullptr,
                                            nullptr,
                                            0,
                                            nullptr);

    ASSERT_TRUE (status == SUCCESS);

    auto instanceId = ConnectWebServicesClientC_GetLastCreatedObjectInstanceId (api);
    ASSERT_FALSE (Utf8String::IsNullOrEmpty (instanceId));

    WString wInstanceId;
    wInstanceId.AssignUtf8 (instanceId);
    
    status = ConnectWebServicesClientC_CreateProjectFavorite(api, 
                                            wInstanceId.c_str());
    ASSERT_TRUE(status == SUCCESS);

    CWSCCDATABUFHANDLE projectFavorite;
    status = ConnectWebServicesClientC_ReadProjectFavorite (api, wInstanceId.c_str(), &projectFavorite);
    EXPECT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_DataBufferFree (api, projectFavorite);
    EXPECT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_DeleteProjectFavorite (api, wInstanceId.c_str ());
    EXPECT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_DeleteProject (api, wInstanceId.c_str ());
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F (ConnectWebServicesClientCTests, CRUDProjectFavoriteV2Functions_CRUDsSuccessful_SuccessfulCodesReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    /************************************************************************************//**
    * \brief Create a new project
    * \param[in] apiHandle API object
    * \param[in] Name
    * \param[in] Number
    * \param[in] OrganizationId
    * \param[in] Active
    * \param[in] Industry
    * \param[in] AssetType
    * \param[in] LastModified
    * \param[in] Location
    * \param[in] Latitude
    * \param[in] Longitude
    * \param[in] LocationIsUsingLatLong
    * \param[in] RegisteredDate
    * \param[in] TimeZoneLocation
    * \param[in] Status
    * \param[in] PWDMInvitationId
    * \return Success or error code. See \ref clientErrorCodes
    ****************************************************************************************/
    BeGuid guid(true);
    WPrintfString Name(L"CWSCCTest%s", guid.ToString().c_str());
    WPrintfString Number(L"CWSCCTest%s", guid.ToString().c_str());
    WString OrgId = L"1001389117";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    CallStatus status = ConnectWebServicesClientC_CreateProject(api, 
                                            Name.c_str(),
                                            Number.c_str (),
                                            OrgId.c_str (),
                                            &Active,
                                            Industry.c_str (),
                                            AssetType.c_str (),
                                            nullptr,
                                            Location.c_str (),
                                            &lat,
                                            &lon,
                                            &LocationIsUsingLatLong,
                                            nullptr,
                                            nullptr,
                                            0,
                                            nullptr);

    ASSERT_TRUE (status == SUCCESS);

    auto instanceId = ConnectWebServicesClientC_GetLastCreatedObjectInstanceId (api);
    ASSERT_FALSE (Utf8String::IsNullOrEmpty (instanceId));

    WString wInstanceId;
    wInstanceId.AssignUtf8 (instanceId);
    
    status = ConnectWebServicesClientC_CreateProjectFavorite_V2(api, 
                                            wInstanceId.c_str());
    ASSERT_TRUE(status == SUCCESS);

    CWSCCDATABUFHANDLE projectFavorite;
    status = ConnectWebServicesClientC_ReadProjectFavorite_V2(api, wInstanceId.c_str(), &projectFavorite);
    EXPECT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_DataBufferFree (api, projectFavorite);
    EXPECT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_DeleteProjectFavorite_V2(api, wInstanceId.c_str());
    EXPECT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_DeleteProject (api, wInstanceId.c_str ());
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F (ConnectWebServicesClientCTests, CRUDProjectMRUFunctions_CRUDsSuccessful_SuccessfulCodesReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    /************************************************************************************//**
    * \brief Create a new project
    * \param[in] apiHandle API object
    * \param[in] Name
    * \param[in] Number
    * \param[in] OrganizationId
    * \param[in] Active
    * \param[in] Industry
    * \param[in] AssetType
    * \param[in] LastModified
    * \param[in] Location
    * \param[in] Latitude
    * \param[in] Longitude
    * \param[in] LocationIsUsingLatLong
    * \param[in] RegisteredDate
    * \param[in] TimeZoneLocation
    * \param[in] Status
    * \param[in] PWDMInvitationId
    * \return Success or error code. See \ref clientErrorCodes
    ****************************************************************************************/
    BeGuid guid(true);
    WPrintfString Name(L"CWSCCTest%s", guid.ToString().c_str());
    WPrintfString Number(L"CWSCCTest%s", guid.ToString().c_str());
    WString OrgId = L"1001389117";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    CallStatus status = ConnectWebServicesClientC_CreateProject(api, 
                                            Name.c_str(),
                                            Number.c_str (),
                                            OrgId.c_str (),
                                            &Active,
                                            Industry.c_str (),
                                            AssetType.c_str (),
                                            nullptr,
                                            Location.c_str (),
                                            &lat,
                                            &lon,
                                            &LocationIsUsingLatLong,
                                            nullptr,
                                            nullptr,
                                            0,
                                            nullptr);
    ASSERT_TRUE (status == SUCCESS);

    auto instanceId = ConnectWebServicesClientC_GetLastCreatedObjectInstanceId (api);
    EXPECT_FALSE (Utf8String::IsNullOrEmpty (instanceId));

    WString wProjectInstanceId;
    wProjectInstanceId.AssignUtf8 (instanceId);

    /************************************************************************************//**
    * \brief Create a new projectmru
    * \param[in] apiHandle API object
    * \param[in] ProjectGuid
    * \param[in] ProjectName
    * \param[in] LastModified
    * \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
    ****************************************************************************************/
    status = ConnectWebServicesClientC_CreateProjectMRU(api, 
                                                        wProjectInstanceId.c_str (),
                                                        Name.c_str (),
                                                        nullptr);
    ASSERT_TRUE(status == SUCCESS);

    instanceId = ConnectWebServicesClientC_GetLastCreatedObjectInstanceId (api);
    EXPECT_FALSE (Utf8String::IsNullOrEmpty (instanceId));

    WString wProjectMRUInstanceId;
    wProjectMRUInstanceId.AssignUtf8 (instanceId);

    CWSCCDATABUFHANDLE projectMRU;
    status = ConnectWebServicesClientC_ReadProjectMRU (api, wProjectMRUInstanceId.c_str (), &projectMRU);
    EXPECT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_DataBufferFree (api, projectMRU);
    EXPECT_TRUE(status == SUCCESS);

    CWSCCDATABUFHANDLE projectMRUDetail;
    status = ConnectWebServicesClientC_ReadProjectMRUDetail (api, wProjectInstanceId.c_str (), &projectMRUDetail);
    EXPECT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_DataBufferFree (api, projectMRUDetail);
    EXPECT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_DeleteProject (api, wProjectInstanceId.c_str ());
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F (ConnectWebServicesClientCTests, CRUDProjectMRUV2Functions_CRUDsSuccessful_SuccessfulCodesReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_pmUsername.c_str (),
        m_pmPassword.c_str (),
        s_temporaryDirectory.c_str (),
        s_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str(),
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    /************************************************************************************//**
    * \brief Create a new project
    * \param[in] apiHandle API object
    * \param[in] Name
    * \param[in] Number
    * \param[in] OrganizationId
    * \param[in] Active
    * \param[in] Industry
    * \param[in] AssetType
    * \param[in] LastModified
    * \param[in] Location
    * \param[in] Latitude
    * \param[in] Longitude
    * \param[in] LocationIsUsingLatLong
    * \param[in] RegisteredDate
    * \param[in] TimeZoneLocation
    * \param[in] Status
    * \param[in] PWDMInvitationId
    * \return Success or error code. See \ref clientErrorCodes
    ****************************************************************************************/
    BeGuid guid(true);
    WPrintfString Name(L"CWSCCTest%s", guid.ToString().c_str());
    WPrintfString Number(L"CWSCCTest%s", guid.ToString().c_str());
    WString OrgId = L"1001389117";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    CallStatus status = ConnectWebServicesClientC_CreateProject(api, 
                                            Name.c_str(),
                                            Number.c_str (),
                                            OrgId.c_str (),
                                            &Active,
                                            Industry.c_str (),
                                            AssetType.c_str (),
                                            nullptr,
                                            Location.c_str (),
                                            &lat,
                                            &lon,
                                            &LocationIsUsingLatLong,
                                            nullptr,
                                            nullptr,
                                            0,
                                            nullptr);
    ASSERT_TRUE (status == SUCCESS);

    auto instanceId = ConnectWebServicesClientC_GetLastCreatedObjectInstanceId (api);
    EXPECT_FALSE (Utf8String::IsNullOrEmpty (instanceId));

    WString wProjectInstanceId;
    wProjectInstanceId.AssignUtf8 (instanceId);

    /************************************************************************************//**
    * \brief Create a new projectmru
    * \param[in] apiHandle API object
    * \param[in] ProjectGuid
    * \param[in] ProjectName
    * \param[in] LastModified
    * \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
    ****************************************************************************************/
    status = ConnectWebServicesClientC_CreateProjectMRU(api, 
                                                        wProjectInstanceId.c_str (),
                                                        Name.c_str (),
                                                        nullptr);
    ASSERT_TRUE(status == SUCCESS);

    CWSCCDATABUFHANDLE projectMRUDetail;
    status = ConnectWebServicesClientC_ReadProjectMRUDetail_V2 (api, wProjectInstanceId.c_str (), &projectMRUDetail);
    EXPECT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_DataBufferFree (api, projectMRUDetail);
    EXPECT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_DeleteProject (api, wProjectInstanceId.c_str ());
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

