/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/ConnectC/ConnectWebServicesClientCTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma warning(disable: 4067) //suppress a compiler warning from using the boost li

#include "ConnectWebServicesClientCTests.h"

#include <Bentley/Bentley.h>

using namespace boost::uuids;

void ConnectWebServicesClientC::SetUp ()
    {
    auto proxy = ProxyHttpHandler::GetProxyIfReachable ("http://127.0.0.1:8888", Credentials ("1", "1"), nullptr);
    if (!Utf8String::IsNullOrEmpty (proxy->GetProxyUrl ().c_str ()))
        m_fiddlerProxyUrl.AssignUtf8 (proxy->GetProxyUrl ().c_str ());
    else
        m_fiddlerProxyUrl = L"";
    }

void ConnectWebServicesClientC::TearDown ()
    {
    m_fiddlerProxyUrl = nullptr;
    }


TEST_F (ConnectWebServicesClientC, Ctor_InvalidProxyUrl_ApiIsNull)
    {
    //NOTE: If Fiddler is running, and has been running for previous tests, this test will fail.
    //      To successfully run this test, restart Fiddler, or close out Fiddler entirely.
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str (),
        m_password.c_str (),
        m_temporaryDirectory.c_str (),
        m_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        L"http://0.0.0.0:80",
        nullptr,
        nullptr
        );
    ASSERT_TRUE (api == nullptr);
    }

TEST_F (ConnectWebServicesClientC, Ctor_InvalidProxyCredentialsWhenProxyCredentialsAreRequired_ApiIsNull)
    {
    //NOTE: If Fiddler is running, and has been running for previous tests, this test will probably fail.
    //      To successfully run this test, restart Fiddler AND make sure to Require Proxy Authentication!
    //This test is meant to ensure that a proxy, requiring authentication, will only successfully complete if authentication is correct.
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str (),
        m_password.c_str (),
        m_temporaryDirectory.c_str (),
        m_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        L"Invalid",
        L"Invalid"
        );
    if (WString::IsNullOrEmpty(m_fiddlerProxyUrl.c_str()))
        ASSERT_FALSE (api == nullptr);
    else
        ASSERT_TRUE (api == nullptr);
    }

TEST_F(ConnectWebServicesClientC, Ctor_ValidParameters_SuccessfulInitialization)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        m_temporaryDirectory.c_str(),
        m_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        m_fiddlerProxyUrl.c_str(),
        m_fiddlerProxyUsername.c_str(),
        m_fiddlerProxyPassword.c_str()
        );
    ASSERT_TRUE (api != nullptr);

    CallStatus status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientC, Ctor_InvalidCredentialsAndValidProductId_ApiIsNull)
    {
    WCharP password = L"password";
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        password,
        m_temporaryDirectory.c_str(),
        m_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        m_fiddlerProxyUrl.c_str(),
        m_fiddlerProxyUsername.c_str(),
        m_fiddlerProxyPassword.c_str()
        );
    ASSERT_TRUE(api == nullptr);
    }

TEST_F(ConnectWebServicesClientC, Ctor_ValidCredentialsAndInvalidProductId_ApiIsNull)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        m_temporaryDirectory.c_str(),
        m_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        L"9999",
        m_fiddlerProxyUrl.c_str(),
        m_fiddlerProxyUsername.c_str(),
        m_fiddlerProxyPassword.c_str()
        );
    ASSERT_TRUE(api == nullptr);
    }

TEST_F (ConnectWebServicesClientC, Ctor_NoProxyUrlOrCredentials_ApiIsNotNull)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str (),
        m_password.c_str (),
        m_temporaryDirectory.c_str (),
        m_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        nullptr,
        nullptr,
        nullptr
        );
    ASSERT_TRUE (api != nullptr);

    CallStatus status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F (ConnectWebServicesClientC, ReadProject_ProjectExists_SuccessfulRetreival)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str (),
        m_password.c_str (),
        m_temporaryDirectory.c_str (),
        m_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str ()
        );
    ASSERT_TRUE (api != nullptr);

    WCharP instanceId = L"8faf2677-4540-40d3-964d-252826089c7f";
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject (api, instanceId, &project);
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_DataBufferFree (api, project);
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F (ConnectWebServicesClientC, ReadProject_InvalidDataBufHandle_ErrorCodeReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str (),
        m_password.c_str (),
        m_temporaryDirectory.c_str (),
        m_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str ()
        );
    ASSERT_TRUE (api != nullptr);

    WCharP instanceId = L"8faf2677-4540-40d3-964d-252826089c7f";
    CallStatus status = ConnectWebServicesClientC_ReadProject (api, instanceId, nullptr);
    ASSERT_TRUE (status == INVALID_PARAMETER);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientC, DataBufferGetCount_Only1ProjectIsReturned_SuccessfulRetreival)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str (),
        m_password.c_str (),
        m_temporaryDirectory.c_str (),
        m_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str ()
        );
    ASSERT_TRUE (api != nullptr);

    WCharP instanceId = L"8faf2677-4540-40d3-964d-252826089c7f";
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

TEST_F(ConnectWebServicesClientC, GetPropertyMethods_Only1ProjectIsReturnedWithFulfilledProjectProperties_SuccessfulRetreivalOfProperties)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str (),
        m_password.c_str (),
        m_temporaryDirectory.c_str (),
        m_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str ()
        );
    ASSERT_TRUE (api != nullptr);

    WCharP instanceId = L"8faf2677-4540-40d3-964d-252826089c7f";
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject(api, instanceId, &project);
    ASSERT_TRUE(status == SUCCESS);
    uint64_t bufferCount = ConnectWebServicesClientC_DataBufferGetCount(project);
    ASSERT_TRUE(bufferCount == 1);

    wchar_t stringBuf[4096];
    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECT_BUFF_NAME, 0, 4096, stringBuf);
    ASSERT_TRUE(status == SUCCESS);
    ASSERT_STREQ(stringBuf, L"Davids New QA Project");

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECT_BUFF_NUMBER, 0, 4096, stringBuf);
    ASSERT_TRUE(status == SUCCESS);
    ASSERT_STREQ(stringBuf, L"-1234567890-0987654321");

    double longitude;
    status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(api, project, PROJECT_BUFF_LONGITUDE, 0, &longitude);
    ASSERT_TRUE(status == PROPERTY_HAS_NOT_BEEN_SET);

    status = ConnectWebServicesClientC_DataBufferFree(api, project);
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientC, GetPropertyMethods_NULLBuffer_AppropriateStatusCodeReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str (),
        m_password.c_str (),
        m_temporaryDirectory.c_str (),
        m_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str ()
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

    wchar_t guid[4096];
    status = ConnectWebServicesClientC_DataBufferGetGuidProperty(api, buf, CONNECTUSER_BUFF_CONNECTORGGUID, 0, 4096, guid);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    status = ConnectWebServicesClientC_DataBufferFree(api, buf);
    ASSERT_TRUE (status == INVALID_PARAMETER);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientC, GetPropertyMethods_BufferWithProjectTypeButInvalidPropertyType_AppropriateStatusCodeReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str (),
        m_password.c_str (),
        m_temporaryDirectory.c_str (),
        m_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str ()
        );
    ASSERT_TRUE (api != nullptr);

    WCharP instanceId = L"8faf2677-4540-40d3-964d-252826089c7f";
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject(api, instanceId, &project);
    ASSERT_TRUE(status == SUCCESS);
    uint64_t bufferCount = ConnectWebServicesClientC_DataBufferGetCount(project);
    ASSERT_TRUE(bufferCount == 1);

    int64_t pLong;
    status = ConnectWebServicesClientC_DataBufferGetLongProperty(api, project, PROJECTMRUDETAIL_BUFF_LASTACCESSEDBYUSER, 0, &pLong);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    wchar_t guid[4096];
    status = ConnectWebServicesClientC_DataBufferGetGuidProperty(api, project, CONNECTUSER_BUFF_CONNECTORGGUID, 0, 4096, guid);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    status = ConnectWebServicesClientC_DataBufferFree(api, project);
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientC, GetPropertyMethods_BufferWithProjectTypeAndValidPropertyTypeButInvalidProperty_AppropriateStatusCodeReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str (),
        m_password.c_str (),
        m_temporaryDirectory.c_str (),
        m_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str ()
        );
    ASSERT_TRUE (api != nullptr);

    WCharP instanceId = L"8faf2677-4540-40d3-964d-252826089c7f";
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

TEST_F (ConnectWebServicesClientC, CreateProjectAndDeleteCreateProject_ProjectCreated_SuccessfulCodeReturned)
    {
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str (),
        m_password.c_str (),
        m_temporaryDirectory.c_str (),
        m_assetsRootDirectory.c_str (),
        m_applicationName.c_str (),
        m_applicationVersion.c_str (),
        m_applicationGuid.c_str (),
        m_ccProductId.c_str (),
        m_fiddlerProxyUrl.c_str (),
        m_fiddlerProxyUsername.c_str (),
        m_fiddlerProxyPassword.c_str ()
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
    WPrintfString Name (L"CWSCCTest%s", to_wstring (random_generator ()()).c_str ());
    WString Number (to_wstring (random_generator()()).c_str ());
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
    ASSERT_FALSE (Utf8String::IsNullOrEmpty(instanceId));

    WString wInstanceId;
    wInstanceId.AssignUtf8 (instanceId);
    status = ConnectWebServicesClientC_DeleteProject (api, wInstanceId.c_str ());
    ASSERT_TRUE (status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi (api);
    ASSERT_TRUE (status == SUCCESS);
    }
