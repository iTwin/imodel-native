/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/NotPublished/ConnectC/ConnectWebServicesClientCTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ConnectWebServicesClientCTests.h"

using namespace ::testing;
using namespace ::std;
USING_NAMESPACE_BENTLEY_WEBSERVICES

const WString m_username = L"testUser";
const WString m_password = L"drowssap";
const WString m_ccProductId = L"9999";
const WString m_applicationName = L"Bentley-Test";
const WString m_applicationVersion = L"1.0.0.0";
const WString m_applicationGuid = L"TestAppGUID";

static BeFileName s_temporaryDirectory, s_assetsRootDirectory;

void ConnectWebServicesClientCTests::SetUpTestCase()
    {
    BaseMockHttpHandlerTest::SetUpTestCase();
    BeTest::GetHost().GetTempDir(s_temporaryDirectory);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(s_assetsRootDirectory);
    }

//////////////////////////////////////////
// --------------  TODO  -------------- //
// ------ 3. ProjectMRU CRUD tests ---- //
// ------ 4. ProjectFav CRUD tests ---- //
//////////////////////////////////////////

#define BUFF_TYPE_PROJECTFAVORITE_V4 4
#define BUFF_TYPE_PROJECTFAVORITE_V4_V2 5
#define BUFF_TYPE_PROJECTMRU 6
#define BUFF_TYPE_PROJECTMRUDETAIL 7
#define BUFF_TYPE_PROJECTMRUDETAIL_V2 8
//////////////////////////////////////////
// -------- Constructor  Tests -------- //
//////////////////////////////////////////
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, InitalizeApiWithCredentials_ValidParameters_InitializedApiProperly)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(2);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    ASSERT_STREQ("Login successful.", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("Login using credentials performed successfully.", ConnectWebServicesClientC_GetLastStatusDescription(api));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, InitalizeApiWithCredentials_UnsuccessfulLogin_InvalidApiInitialization)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(2);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubHttpResponse());         //For connectSignInManager->SignInWithCredentials

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api == nullptr);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, InitalizeApiWithToken_ValidParameters_InitializedApiProperly)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(2);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithToken
    
    WString samlToken;
    samlToken.AssignUtf8(StubSamlToken()->AsString().c_str());

    auto api = ConnectWebServicesClientC_InitializeApiWithToken
        (samlToken.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    ASSERT_STREQ("Login successful.", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("Login using credentials performed successfully.", ConnectWebServicesClientC_GetLastStatusDescription(api));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, InitalizeApiWithToken_UnsuccessfulLoginFromInvalidToken_InvalidApiInitialization)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(0);

    auto api = ConnectWebServicesClientC_InitializeApiWithToken
        (L"invalidToken",
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api == nullptr);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, InitalizeApiWithToken_UnsuccessfulLoginFromInvalidResponse_InitializedApiProperly)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(2);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubHttpResponse());         //For connectSignInManager->SignInWithToken

    WString samlToken;
    samlToken.AssignUtf8(StubSamlToken()->AsString().c_str());

    auto api = ConnectWebServicesClientC_InitializeApiWithToken
        (samlToken.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api == nullptr);
    }

//////////////////////////////////////////
// ------ Organization Tests ---------- //
//////////////////////////////////////////
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadOrganization_V2List_ThreeOrganizationsReturned_ValidBufferPropertiesSetAndParsed)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Organization_V2", request.GetUrl().c_str());
        Utf8String objectResponse(R"({"instances":[{"instanceId":"1001389117","schemaName":"GlobalSchema","className":"Organization_V2","properties":{"OrganizationGuid":"OrgGuid","OrganizationName":"Bentley Systems Inc"},"eTag":"\"1CWiEjIPqtgVWIirMVEBagMIr1E=\""},{"instanceId":"1001389117","schemaName":"GlobalSchema","className":"Organization_V2","properties":{"OrganizationGuid":"OrgGuid","OrganizationName":"Bentley Systems Inc"},"eTag":"\"1CWiEjIPqtgVWIirMVEBagMIr1E=\""},{"instanceId":"1001389117","schemaName":"GlobalSchema","className":"Organization_V2","properties":{"OrganizationGuid":"OrgGuid","OrganizationName":"Bentley Systems Inc"},"eTag":"\"1CWiEjIPqtgVWIirMVEBagMIr1E=\""}]})");
        return Response(HttpResponseContent::Create(HttpStringBody::Create(objectResponse)), "", ConnectionStatus::OK, HttpStatus::OK);
        });

    CWSCCDATABUFHANDLE organizations;
    CallStatus status = ConnectWebServicesClientC_ReadOrganization_V2List(api, &organizations);
    ASSERT_TRUE(status == SUCCESS);

    auto bufferCount = ConnectWebServicesClientC_DataBufferGetCount(organizations);
    ASSERT_TRUE(3 == bufferCount);

    for (int i = 0; i < bufferCount; i++)
        {
        wchar_t stringBuf[4096];
        status = ConnectWebServicesClientC_DataBufferGetGuidProperty(api, organizations, ORGANIZATION_V2_BUFF_ORGANIZATIONGUID, i, 4096, stringBuf);
        ASSERT_STREQ(L"OrgGuid", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, organizations, ORGANIZATION_V2_BUFF_ORGANIZATIONNAME, i, 4096, stringBuf);
        ASSERT_STREQ(L"Bentley Systems Inc", stringBuf);
        ASSERT_TRUE(SUCCESS == status);
        }

    status = ConnectWebServicesClientC_DataBufferFree(api, organizations);
    ASSERT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, CreateOrganization_ValidOrganizationParameters_OrganizationSuccessfullyCreated)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Organization_V2", request.GetUrl().c_str());
        Utf8String objectResponse(R"({"changedInstance":{"change":"Created","instanceAfterChange":{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"Organization_V2","properties":{"OrganizationGuid":"CWSCCTest","OrganizationName":"CWSCCTest"}}}})");
        return Response(HttpResponseContent::Create(HttpStringBody::Create(objectResponse)), "", ConnectionStatus::OK, HttpStatus::Created);
        });

    WString Name(L"CWSCCTest");
    WString Number(L"CWSCCTest");
    CallStatus status = ConnectWebServicesClientC_CreateOrganization_V2(api, Name.c_str(), Number.c_str());
    ASSERT_TRUE(status == SUCCESS);

    Utf8String instanceId;
    ConnectWebServicesClientC_GetLastCreatedObjectInstanceId (api, instanceId);
    EXPECT_STREQ("testInstanceId", instanceId.c_str());

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadOrganization_OneOrganizationReturned_ValidBufferPropertiesSetAndParsed)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Organization_V2/testInstanceId", request.GetUrl().c_str());
        Utf8String objectResponse(R"({"instances":[{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"Organization_V2","properties":{"OrganizationGuid":"OrgGuid","OrganizationName":"OrgName"},"eTag":"\"1CWiEjIPqtgVWIirMVEBagMIr1E=\""}]})");
        return Response(HttpResponseContent::Create(HttpStringBody::Create(objectResponse)), "", ConnectionStatus::OK, HttpStatus::OK);
        });

    CWSCCDATABUFHANDLE organization;
    CallStatus status = ConnectWebServicesClientC_ReadOrganization_V2(api, L"testInstanceId", &organization);
    ASSERT_TRUE(status == SUCCESS);

    auto bufferCount = ConnectWebServicesClientC_DataBufferGetCount(organization);
    ASSERT_TRUE(1 == bufferCount);

    wchar_t stringBuf[4096];
    int i = 0;
    status = ConnectWebServicesClientC_DataBufferGetGuidProperty(api, organization, ORGANIZATION_V2_BUFF_ORGANIZATIONGUID, i, 4096, stringBuf);
    ASSERT_STREQ(L"OrgGuid", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, organization, ORGANIZATION_V2_BUFF_ORGANIZATIONNAME, i, 4096, stringBuf);
    ASSERT_STREQ(L"OrgName", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferFree(api, organization);
    ASSERT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

//////////////////////////////////////////
// --------- PROJECT_V4 Tests --------- //
//////////////////////////////////////////
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProjectV4List_FiveProjectsReturned_ValidBufferPropertiesSetAndParsed)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String objectResponse(R"({"instances":[{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"Project_V4","properties":{"UltimateRefId":"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1","LinkToAssetType":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8/Asset/11","LinkToIndustry":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8","LinkToIndustryAssets":"https://qa-recommendation-eus.cloudapp.net/api/v1/IndustryAssets","IsRbacEnabled":"true","Name":"CWSCCTest","Number":"CWSCCTest","Industry":"8","AssetType":"11","LastModified":"2016-06-03T17:19:48.97","Location":"Huntsville","Latitude":null,"Longitude":null,"LocationIsUsingLatLong":false,"RegisteredDate":"2016-06-03T17:19:48.97","TimeZoneLocation":null,"Status":1,"Data_Location_Guid":"99999999-9999-9999-9999-999999999999","Country_Code":"ZZ"},"eTag":"\"ebajNAaaq0kUZvzhmESp1mK5KCI=\""},{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"Project_V4","properties":{"UltimateRefId":"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1","LinkToAssetType":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8/Asset/11","LinkToIndustry":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8","LinkToIndustryAssets":"https://qa-recommendation-eus.cloudapp.net/api/v1/IndustryAssets","IsRbacEnabled":"true","Name":"CWSCCTest","Number":"CWSCCTest","Industry":"8","AssetType":"11","LastModified":"2016-06-03T17:19:48.97","Location":"Huntsville","Latitude":null,"Longitude":null,"LocationIsUsingLatLong":false,"RegisteredDate":"2016-06-03T17:19:48.97","TimeZoneLocation":null,"Status":1,"Data_Location_Guid":"99999999-9999-9999-9999-999999999999","Country_Code":"ZZ"},"eTag":"\"ebajNAaaq0kUZvzhmESp1mK5KCI=\""},{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"Project_V4","properties":{"UltimateRefId":"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1","LinkToAssetType":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8/Asset/11","LinkToIndustry":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8","LinkToIndustryAssets":"https://qa-recommendation-eus.cloudapp.net/api/v1/IndustryAssets","IsRbacEnabled":"true","Name":"CWSCCTest","Number":"CWSCCTest","Industry":"8","AssetType":"11","LastModified":"2016-06-03T17:19:48.97","Location":"Huntsville","Latitude":null,"Longitude":null,"LocationIsUsingLatLong":false,"RegisteredDate":"2016-06-03T17:19:48.97","TimeZoneLocation":null,"Status":1,"Data_Location_Guid":"99999999-9999-9999-9999-999999999999","Country_Code":"ZZ"},"eTag":"\"ebajNAaaq0kUZvzhmESp1mK5KCI=\""},{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"Project_V4","properties":{"UltimateRefId":"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1","LinkToAssetType":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8/Asset/11","LinkToIndustry":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8","LinkToIndustryAssets":"https://qa-recommendation-eus.cloudapp.net/api/v1/IndustryAssets","IsRbacEnabled":"true","Name":"CWSCCTest","Number":"CWSCCTest","Industry":"8","AssetType":"11","LastModified":"2016-06-03T17:19:48.97","Location":"Huntsville","Latitude":null,"Longitude":null,"LocationIsUsingLatLong":false,"RegisteredDate":"2016-06-03T17:19:48.97","TimeZoneLocation":null,"Status":1,"Data_Location_Guid":"99999999-9999-9999-9999-999999999999","Country_Code":"ZZ"},"eTag":"\"ebajNAaaq0kUZvzhmESp1mK5KCI=\""},{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"Project_V4","properties":{"UltimateRefId":"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1","LinkToAssetType":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8/Asset/11","LinkToIndustry":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8","LinkToIndustryAssets":"https://qa-recommendation-eus.cloudapp.net/api/v1/IndustryAssets","IsRbacEnabled":"true","Name":"CWSCCTest","Number":"CWSCCTest","Industry":"8","AssetType":"11","LastModified":"2016-06-03T17:19:48.97","Location":"Huntsville","Latitude":null,"Longitude":null,"LocationIsUsingLatLong":false,"RegisteredDate":"2016-06-03T17:19:48.97","TimeZoneLocation":null,"Status":1,"Data_Location_Guid":"99999999-9999-9999-9999-999999999999","Country_Code":"ZZ"},"eTag":"\"ebajNAaaq0kUZvzhmESp1mK5KCI=\""}]})");
        return Response(HttpResponseContent::Create(HttpStringBody::Create(objectResponse)), "", ConnectionStatus::OK, HttpStatus::OK);
        });

    CWSCCDATABUFHANDLE projects;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4List(api, &projects);
    ASSERT_TRUE(status == SUCCESS);

    auto bufferCount = ConnectWebServicesClientC_DataBufferGetCount(projects);
    ASSERT_TRUE(5 == bufferCount);

    for (int i = 0; i < bufferCount; i++)
        {
        wchar_t stringBuf[4096];
        bool boolProperty;
        double doubleProperty;
        int32_t intProperty;

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECT_V4_BUFF_ULTIMATEREFID, i, 4096, stringBuf);
        ASSERT_STREQ(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECT_V4_BUFF_NAME, i, 4096, stringBuf);
        ASSERT_STREQ(L"CWSCCTest", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECT_V4_BUFF_NUMBER, i, 4096, stringBuf);
        ASSERT_STREQ(L"CWSCCTest", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECT_V4_BUFF_INDUSTRY, i, 4096, stringBuf);
        ASSERT_STREQ(L"8", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECT_V4_BUFF_ASSETTYPE, i, 4096, stringBuf);
        ASSERT_STREQ(L"11", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetDatetimeProperty(api, projects, PROJECT_V4_BUFF_LASTMODIFIED, i, 4096, stringBuf);
        ASSERT_STREQ(L"2016-06-03T17:19:48.97", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECT_V4_BUFF_LOCATION, i, 4096, stringBuf);
        ASSERT_STREQ(L"Huntsville", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(api, projects, PROJECT_V4_BUFF_LATITUDE, i, &doubleProperty);
        ASSERT_TRUE(PROPERTY_HAS_NOT_BEEN_SET == status);

        status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(api, projects, PROJECT_V4_BUFF_LONGITUDE, i, &doubleProperty);
        ASSERT_TRUE(PROPERTY_HAS_NOT_BEEN_SET == status);

        status = ConnectWebServicesClientC_DataBufferGetBooleanProperty(api, projects, PROJECT_V4_BUFF_LOCATIONISUSINGLATLONG, i, &boolProperty);
        ASSERT_FALSE(boolProperty);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetDatetimeProperty(api, projects, PROJECT_V4_BUFF_REGISTEREDDATE, i, 4096, stringBuf);
        ASSERT_STREQ(L"2016-06-03T17:19:48.97", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECT_V4_BUFF_TIMEZONELOCATION, i, 4096, stringBuf);
        ASSERT_TRUE(PROPERTY_HAS_NOT_BEEN_SET == status);

        status = ConnectWebServicesClientC_DataBufferGetIntProperty(api, projects, PROJECT_V4_BUFF_STATUS, i, &intProperty);
        ASSERT_TRUE(1 == intProperty);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECT_V4_BUFF_COUNTRY_CODE, i, 4096, stringBuf);
        ASSERT_STREQ(L"ZZ", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECT_V4_BUFF_DATA_LOCATION_GUID, i, 4096, stringBuf);
        ASSERT_STREQ(L"99999999-9999-9999-9999-999999999999", stringBuf);
        ASSERT_TRUE(SUCCESS == status);
        }

    status = ConnectWebServicesClientC_DataBufferFree(api, projects);
    ASSERT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, CreateProjectV4_ValidProjectParameters_ProjectSuccessfullyCreated)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String objectResponse(R"({"changedInstance":{"change":"Created","instanceAfterChange":{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"Project_V4","properties":{"UltimateRefId":"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1","LinkToAssetType":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8/Asset/11","LinkToIndustry":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8","LinkToIndustryAssets":"https://qa-recommendation-eus.cloudapp.net/api/v1/IndustryAssets","IsRbacEnabled":"true","Name":"CWSCCTest","Number":"CWSCCTest","Industry":"8","AssetType":"11","LastModified":"2016-06-03T17:19:48.9552644Z","Location":"Huntsville","Latitude":null,"Longitude":null,"LocationIsUsingLatLong":false,"RegisteredDate":"2016-06-03T17:19:48.9552644Z","TimeZoneLocation":null,"Status":1,"Data_Location_Guid":"99999999-9999-9999-9999-999999999999","Country_Code":"ZZ"}}}})");
        return Response(HttpResponseContent::Create(HttpStringBody::Create(objectResponse)), "", ConnectionStatus::OK, HttpStatus::Created);
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString Name(L"CWSCCTest");
    WString Number(L"CWSCCTest");
    WString OrgId = L"1001389117";
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_CreateProject_V4(api, UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),
                                                               nullptr, Location.c_str(), nullptr, nullptr, nullptr, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == SUCCESS);

    Utf8String instanceId;
    ConnectWebServicesClientC_GetLastCreatedObjectInstanceId (api, instanceId);
    EXPECT_STREQ("testInstanceId", instanceId.c_str());

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProjectV4_OneProjectReturned_ValidBufferPropertiesSetAndParsed)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/testInstanceId", request.GetUrl().c_str());
        Utf8String objectResponse(R"({"instances":[{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"Project_V4","properties":{"UltimateRefId":"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1","LinkToAssetType":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8/Asset/11","LinkToIndustry":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8","LinkToIndustryAssets":"https://qa-recommendation-eus.cloudapp.net/api/v1/IndustryAssets","IsRbacEnabled":"true","Name":"CWSCCTest","Number":"CWSCCTest","Industry":"8","AssetType":"11","LastModified":"2016-06-03T17:19:48.97","Location":"Huntsville","Latitude":null,"Longitude":null,"LocationIsUsingLatLong":false,"RegisteredDate":"2016-06-03T17:19:48.97","TimeZoneLocation":null,"Status":1,"Data_Location_Guid":"99999999-9999-9999-9999-999999999999","Country_Code":"ZZ"},"eTag":"\"ebajNAaaq0kUZvzhmESp1mK5KCI=\""}]})");
        return Response(HttpResponseContent::Create(HttpStringBody::Create(objectResponse)), "", ConnectionStatus::OK, HttpStatus::OK);
        });

    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4(api, L"testInstanceId", &project);
    ASSERT_TRUE(status == SUCCESS);

    auto bufferCount = ConnectWebServicesClientC_DataBufferGetCount(project);
    ASSERT_TRUE(1 == bufferCount);


    wchar_t stringBuf[4096];
    bool boolProperty;
    double doubleProperty;
    int32_t intProperty;
    int i = 0;

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECT_V4_BUFF_ULTIMATEREFID, i, 4096, stringBuf);
    ASSERT_STREQ(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECT_V4_BUFF_NAME, i, 4096, stringBuf);
    ASSERT_STREQ(L"CWSCCTest", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECT_V4_BUFF_NUMBER, i, 4096, stringBuf);
    ASSERT_STREQ(L"CWSCCTest", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECT_V4_BUFF_INDUSTRY, i, 4096, stringBuf);
    ASSERT_STREQ(L"8", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECT_V4_BUFF_ASSETTYPE, i, 4096, stringBuf);
    ASSERT_STREQ(L"11", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetDatetimeProperty(api, project, PROJECT_V4_BUFF_LASTMODIFIED, i, 4096, stringBuf);
    ASSERT_STREQ(L"2016-06-03T17:19:48.97", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECT_V4_BUFF_LOCATION, i, 4096, stringBuf);
    ASSERT_STREQ(L"Huntsville", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(api, project, PROJECT_V4_BUFF_LATITUDE, i, &doubleProperty);
    ASSERT_TRUE(PROPERTY_HAS_NOT_BEEN_SET == status);

    status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(api, project, PROJECT_V4_BUFF_LONGITUDE, i, &doubleProperty);
    ASSERT_TRUE(PROPERTY_HAS_NOT_BEEN_SET == status);

    status = ConnectWebServicesClientC_DataBufferGetBooleanProperty(api, project, PROJECT_V4_BUFF_LOCATIONISUSINGLATLONG, i, &boolProperty);
    ASSERT_FALSE(boolProperty);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetDatetimeProperty(api, project, PROJECT_V4_BUFF_REGISTEREDDATE, i, 4096, stringBuf);
    ASSERT_STREQ(L"2016-06-03T17:19:48.97", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECT_V4_BUFF_TIMEZONELOCATION, i, 4096, stringBuf);
    ASSERT_TRUE(PROPERTY_HAS_NOT_BEEN_SET == status);

    status = ConnectWebServicesClientC_DataBufferGetIntProperty(api, project, PROJECT_V4_BUFF_STATUS, i, &intProperty);
    ASSERT_TRUE(1 == intProperty);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECT_V4_BUFF_COUNTRY_CODE, i, 4096, stringBuf);
    ASSERT_STREQ(L"ZZ", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECT_V4_BUFF_DATA_LOCATION_GUID, i, 4096, stringBuf);
    ASSERT_STREQ(L"99999999-9999-9999-9999-999999999999", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferFree(api, project);
    ASSERT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, UpdateProjectV4_OneProjectReturned_ProjectSuccessfullyUpdated)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/testInstanceId", request.GetUrl().c_str());
        Utf8String objectResponse(R"({"changedInstance":{"change":"Modified","instanceAfterChange":{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"Project_V4","properties":{"UltimateRefId":"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1","LinkToAssetType":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8/Asset/11","LinkToIndustry":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8","LinkToIndustryAssets":"https://qa-recommendation-eus.cloudapp.net/api/v1/IndustryAssets","IsRbacEnabled":"true","Name":"CWSCCTest","Number":"CWSCCTest","Industry":"8","AssetType":"11","LastModified":"0001-01-01T00:00:00","Location":null,"Latitude":null,"Longitude":null,"LocationIsUsingLatLong":false,"RegisteredDate":"0001-01-01T00:00:00","TimeZoneLocation":null,"Status":1,"Data_Location_Guid":"99999999-9999-9999-9999-999999999999","Country_Code":"ZZ"}}}})");
        return Response(HttpResponseContent::Create(HttpStringBody::Create(objectResponse)), "", ConnectionStatus::OK, HttpStatus::OK);
        });

    WString instanceId(L"testInstanceId");
    WString UltimateRefId(L"3c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString Name(L"CWSCCTest");
    WString Number(L"CWSCCTest");
    WString Industry = L"9";
    WString AssetType = L"12";
    WString Location = L"Huntsville";
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_UpdateProject_V4(api, instanceId.c_str(), UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),
                                                                nullptr, Location.c_str(), nullptr, nullptr, nullptr, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, DeleteProjectV4_ValidDeletionParameters_ProjectSuccessfullyDeleted)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("DELETE", request.GetMethod().c_str());
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/testInstanceId", request.GetUrl().c_str());
        Utf8String objectResponse(R"({"changedInstance":{"change":"Deleted","instanceAfterChange":{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"Project_V4","properties":{}}}})");
        return Response(HttpResponseContent::Create(HttpStringBody::Create(objectResponse)), "", ConnectionStatus::OK, HttpStatus::OK);
        });

    WString instanceId(L"testInstanceId");
    CallStatus status = ConnectWebServicesClientC_DeleteProject_V4(api, instanceId.c_str());
    ASSERT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

//////////////////////////////////////////
// ------ PROJECTFAVORITE_V4 Tests ------- //
//////////////////////////////////////////
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProjectFavorite_V4List_ThreeProjectsReturned_ValidBufferPropertiesSetAndParsed)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
    (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
    );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=](RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/ProjectFavorite_V4", request.GetUrl().c_str());
        Utf8String objectResponse(R"({"instances":[{"instanceId":"blahId","schemaName":"GlobalSchema","className":"ProjectFavorite_V4","properties":{"UltimateRefId":"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1","LinkToAssetType":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8/Asset/11","LinkToIndustry":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8","LinkToIndustryAssets":"https://qa-recommendation-eus.cloudapp.net/api/v1/IndustryAssets","IsRbacEnabled":"true","Name":"Davids New QA Project","Number":"-1234567890-0987654321","Industry":"8","AssetType":"11","LastModified":"2016-04-06","Location":"Huntsville","Latitude":null,"Longitude":null,"LocationIsUsingLatLong":false,"RegisteredDate":"2016-04-06","TimeZoneLocation":null,"Status":1,"Data_Location_Guid":"99999999-9999-9999-9999-999999999999","Country_Code":"ZZ"},"eTag":"\"hchy/9kFbShmIZOdRTYoTeSW9+0=\""},{"instanceId":"blahId","schemaName":"GlobalSchema","className":"ProjectFavorite_V4","properties":{"UltimateRefId":"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1","LinkToAssetType":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8/Asset/11","LinkToIndustry":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8","LinkToIndustryAssets":"https://qa-recommendation-eus.cloudapp.net/api/v1/IndustryAssets","IsRbacEnabled":"true","Name":"Davids New QA Project","Number":"-1234567890-0987654321","Industry":"8","AssetType":"11","LastModified":"2016-04-06","Location":"Huntsville","Latitude":null,"Longitude":null,"LocationIsUsingLatLong":false,"RegisteredDate":"2016-04-06","TimeZoneLocation":null,"Status":1,"Data_Location_Guid":"99999999-9999-9999-9999-999999999999","Country_Code":"ZZ"},"eTag":"\"hchy/9kFbShmIZOdRTYoTeSW9+0=\""},{"instanceId":"blahId","schemaName":"GlobalSchema","className":"ProjectFavorite_V4","properties":{"UltimateRefId":"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1","LinkToAssetType":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8/Asset/11","LinkToIndustry":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8","LinkToIndustryAssets":"https://qa-recommendation-eus.cloudapp.net/api/v1/IndustryAssets","IsRbacEnabled":"true","Name":"Davids New QA Project","Number":"-1234567890-0987654321","Industry":"8","AssetType":"11","LastModified":"2016-04-06","Location":"Huntsville","Latitude":null,"Longitude":null,"LocationIsUsingLatLong":false,"RegisteredDate":"2016-04-06","TimeZoneLocation":null,"Status":1,"Data_Location_Guid":"99999999-9999-9999-9999-999999999999","Country_Code":"ZZ"},"eTag":"\"hchy/9kFbShmIZOdRTYoTeSW9+0=\""}]})");
        return Response(HttpResponseContent::Create(HttpStringBody::Create(objectResponse)), "", ConnectionStatus::OK, HttpStatus::OK);
        });

    CWSCCDATABUFHANDLE projects;
    CallStatus status = ConnectWebServicesClientC_ReadProjectFavorite_V4List(api, &projects);
    ASSERT_TRUE(status == SUCCESS);

    auto bufferCount = ConnectWebServicesClientC_DataBufferGetCount(projects);
    ASSERT_TRUE(3 == bufferCount);

    for (int i = 0; i < bufferCount; i++)
        {
        wchar_t stringBuf[4096];
        bool boolProperty;
        double doubleProperty;
        int32_t intProperty;

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECTFAVORITE_V4_BUFF_ULTIMATEREFID, i, 4096, stringBuf);
        ASSERT_STREQ(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECTFAVORITE_V4_BUFF_NAME, i, 4096, stringBuf);
        ASSERT_STREQ(L"Davids New QA Project", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECTFAVORITE_V4_BUFF_NUMBER, i, 4096, stringBuf);
        ASSERT_STREQ(L"-1234567890-0987654321", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECTFAVORITE_V4_BUFF_INDUSTRY, i, 4096, stringBuf);
        ASSERT_STREQ(L"8", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECTFAVORITE_V4_BUFF_ASSETTYPE, i, 4096, stringBuf);
        ASSERT_STREQ(L"11", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetDatetimeProperty(api, projects, PROJECTFAVORITE_V4_BUFF_LASTMODIFIED, i, 4096, stringBuf);
        ASSERT_STREQ(L"2016-04-06", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECTFAVORITE_V4_BUFF_LOCATION, i, 4096, stringBuf);
        ASSERT_STREQ(L"Huntsville", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(api, projects, PROJECTFAVORITE_V4_BUFF_LATITUDE, i, &doubleProperty);
        ASSERT_TRUE(PROPERTY_HAS_NOT_BEEN_SET == status);

        status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(api, projects, PROJECTFAVORITE_V4_BUFF_LONGITUDE, i, &doubleProperty);
        ASSERT_TRUE(PROPERTY_HAS_NOT_BEEN_SET == status);

        status = ConnectWebServicesClientC_DataBufferGetBooleanProperty(api, projects, PROJECTFAVORITE_V4_BUFF_LOCATIONISUSINGLATLONG, i, &boolProperty);
        ASSERT_FALSE(boolProperty);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetDatetimeProperty(api, projects, PROJECTFAVORITE_V4_BUFF_REGISTEREDDATE, i, 4096, stringBuf);
        ASSERT_STREQ(L"2016-04-06", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECTFAVORITE_V4_BUFF_TIMEZONELOCATION, i, 4096, stringBuf);
        ASSERT_TRUE(PROPERTY_HAS_NOT_BEEN_SET == status);

        status = ConnectWebServicesClientC_DataBufferGetIntProperty(api, projects, PROJECTFAVORITE_V4_BUFF_STATUS, i, &intProperty);
        ASSERT_TRUE(1 == intProperty);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECTFAVORITE_V4_BUFF_DATA_LOCATION_GUID, i, 4096, stringBuf);
        ASSERT_STREQ(L"99999999-9999-9999-9999-999999999999", stringBuf);
        ASSERT_TRUE(SUCCESS == status);

        status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, projects, PROJECTFAVORITE_V4_BUFF_COUNTRY_CODE, i, 4096, stringBuf);
        ASSERT_STREQ(L"ZZ", stringBuf);
        ASSERT_TRUE(SUCCESS == status);
        }

    status = ConnectWebServicesClientC_DataBufferFree(api, projects);
    ASSERT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, CreateProjectFavorite_V4_ValidProjectParameters_ProjectSuccessfullyCreated)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
    (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
    );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=](RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/ProjectFavorite_V4", request.GetUrl().c_str());
        Utf8String objectResponse(R"({"changedInstance":{"change":"Created","instanceAfterChange":{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"ProjectFavorite_V4","properties":{"UltimateRefId":"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1","LinkToAssetType":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8/Asset/11","LinkToIndustry":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8","LinkToIndustryAssets":"https://qa-recommendation-eus.cloudapp.net/api/v1/IndustryAssets","IsRbacEnabled":"true","Name":"CWSCCTest","Number":"CWSCCTest","AssetType":"11","Industry":"8","Latitude":null,"Location":"Huntsville","LocationIsUsingLatLong":false,"Longitude":null,"LastModified":"6/3/2016 1:31:03 PM","RegisteredDate":"6/3/2016 1:31:03 PM","TimeZoneLocation":null,"Status":1,"Data_Location_Guid":"99999999-9999-9999-9999-999999999999","Country_Code":"ZZ"}}}})");
        return Response(HttpResponseContent::Create(HttpStringBody::Create(objectResponse)), "", ConnectionStatus::OK, HttpStatus::Created);
        });

    CallStatus status = ConnectWebServicesClientC_CreateProjectFavorite_V4(api, L"testInstanceId");
    ASSERT_TRUE(status == SUCCESS);

    Utf8String instanceId;
    ConnectWebServicesClientC_GetLastCreatedObjectInstanceId (api, instanceId);
    EXPECT_STREQ("testInstanceId", instanceId.c_str());

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProjectFavorite_V4_OneProjectReturned_ValidBufferPropertiesSetAndParsed)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
    (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
    );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=](RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/ProjectFavorite_V4/testInstanceId", request.GetUrl().c_str());
        Utf8String objectResponse(R"({"instances":[{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"ProjectFavorite_V4","properties":{"UltimateRefId":"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1","LinkToAssetType":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8/Asset/11","LinkToIndustry":"https://qa-recommendation-eus.cloudapp.net/api/v1/Industry/8","LinkToIndustryAssets":"https://qa-recommendation-eus.cloudapp.net/api/v1/IndustryAssets","IsRbacEnabled":"true","Name":"Davids New QA Project","Number":"-1234567890-0987654321","Industry":"8","AssetType":"11","LastModified":"2016-04-06","Location":"Huntsville","Latitude":null,"Longitude":null,"LocationIsUsingLatLong":false,"RegisteredDate":"2016-04-06","TimeZoneLocation":null,"Status":1,"Data_Location_Guid":"99999999-9999-9999-9999-999999999999","Country_Code":"ZZ"},"eTag":"\"hchy/9kFbShmIZOdRTYoTeSW9+0=\""}]})");
        return Response(HttpResponseContent::Create(HttpStringBody::Create(objectResponse)), "", ConnectionStatus::OK, HttpStatus::OK);
        });

    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProjectFavorite_V4(api, L"testInstanceId", &project);
    ASSERT_TRUE(status == SUCCESS);

    auto bufferCount = ConnectWebServicesClientC_DataBufferGetCount(project);
    ASSERT_TRUE(1 == bufferCount);

    wchar_t stringBuf[4096];
    bool boolProperty;
    double doubleProperty;
    int32_t intProperty;
    int i = 0;

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECTFAVORITE_V4_BUFF_ULTIMATEREFID, i, 4096, stringBuf);
    ASSERT_STREQ(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECTFAVORITE_V4_BUFF_NAME, i, 4096, stringBuf);
    ASSERT_STREQ(L"Davids New QA Project", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECTFAVORITE_V4_BUFF_NUMBER, i, 4096, stringBuf);
    ASSERT_STREQ(L"-1234567890-0987654321", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECTFAVORITE_V4_BUFF_INDUSTRY, i, 4096, stringBuf);
    ASSERT_STREQ(L"8", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECTFAVORITE_V4_BUFF_ASSETTYPE, i, 4096, stringBuf);
    ASSERT_STREQ(L"11", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetDatetimeProperty(api, project, PROJECTFAVORITE_V4_BUFF_LASTMODIFIED, i, 4096, stringBuf);
    ASSERT_STREQ(L"2016-04-06", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECTFAVORITE_V4_BUFF_LOCATION, i, 4096, stringBuf);
    ASSERT_STREQ(L"Huntsville", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(api, project, PROJECTFAVORITE_V4_BUFF_LATITUDE, i, &doubleProperty);
    ASSERT_TRUE(PROPERTY_HAS_NOT_BEEN_SET == status);

    status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(api, project, PROJECTFAVORITE_V4_BUFF_LONGITUDE, i, &doubleProperty);
    ASSERT_TRUE(PROPERTY_HAS_NOT_BEEN_SET == status);

    status = ConnectWebServicesClientC_DataBufferGetBooleanProperty(api, project, PROJECTFAVORITE_V4_BUFF_LOCATIONISUSINGLATLONG, i, &boolProperty);
    ASSERT_FALSE(boolProperty);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetDatetimeProperty(api, project, PROJECTFAVORITE_V4_BUFF_REGISTEREDDATE, i, 4096, stringBuf);
    ASSERT_STREQ(L"2016-04-06", stringBuf);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECTFAVORITE_V4_BUFF_TIMEZONELOCATION, i, 4096, stringBuf);
    ASSERT_TRUE(PROPERTY_HAS_NOT_BEEN_SET == status);

    status = ConnectWebServicesClientC_DataBufferGetIntProperty(api, project, PROJECTFAVORITE_V4_BUFF_STATUS, i, &intProperty);
    ASSERT_TRUE(1 == intProperty);
    ASSERT_TRUE(SUCCESS == status);

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECTFAVORITE_V4_BUFF_DATA_LOCATION_GUID, i, 4096, stringBuf);
    ASSERT_STREQ(L"99999999-9999-9999-9999-999999999999", stringBuf);
    ASSERT_TRUE(SUCCESS == status);
    
    status = ConnectWebServicesClientC_DataBufferGetStringProperty(api, project, PROJECTFAVORITE_V4_BUFF_COUNTRY_CODE, i, 4096, stringBuf);
    ASSERT_STREQ(L"ZZ", stringBuf);
    ASSERT_TRUE(SUCCESS == status);


    status = ConnectWebServicesClientC_DataBufferFree(api, project);
    ASSERT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, DeleteProjectFavorite_V4_ValidDeletionParameters_ProjectSuccessfullyDeleted)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
    (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
    );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=](RequestCR request)
        {
        EXPECT_STREQ("DELETE", request.GetMethod().c_str());
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/ProjectFavorite_V4/testInstanceId", request.GetUrl().c_str());
        Utf8String objectResponse(R"({"changedInstance":{"change":"Deleted","instanceAfterChange":{"instanceId":"testInstanceId","schemaName":"GlobalSchema","className":"Project","properties":{}}}})");
        return Response(HttpResponseContent::Create(HttpStringBody::Create(objectResponse)), "", ConnectionStatus::OK, HttpStatus::OK);
        });

    WString instanceId(L"testInstanceId");
    CallStatus status = ConnectWebServicesClientC_DeleteProjectFavorite_V4(api, instanceId.c_str());
    ASSERT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

//////////////////////////////////////////
// ---- Testing Error Code Returns ---- //
// This is the same code flow for every //
//   ECClass CRUD function, so testing  //
//   just for Project is sufficient     //
//////////////////////////////////////////
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4List_ServerReturnsClassNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>ClassNotFound</errorId><errorMessage>TestMessage</errorMessage><errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });


    CWSCCDATABUFHANDLE Project_V4s;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4List(api, &Project_V4s);
    ASSERT_TRUE(status == CLASS_NOT_FOUND);

    ASSERT_STREQ("Class not found on server. Please contact your server administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4List_ServerReturnsSchemaNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>SchemaNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });


    CWSCCDATABUFHANDLE Project_V4s;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4List(api, &Project_V4s);
    ASSERT_TRUE(status == SCHEMA_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4List_ServerReturnsRepositoryNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>RepositoryNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });


    CWSCCDATABUFHANDLE Project_V4s;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4List(api, &Project_V4s);
    ASSERT_TRUE(status == REPOSITORY_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4List_ServerReturnsPropertyNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>PropertyNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });


    CWSCCDATABUFHANDLE Project_V4s;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4List(api, &Project_V4s);
    ASSERT_TRUE(status == PROPERTY_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4List_ServerReturnsInstanceNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>InstanceNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });


    CWSCCDATABUFHANDLE Project_V4s;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4List(api, &Project_V4s);
    ASSERT_TRUE(status == INSTANCE_NOT_FOUND);

    ASSERT_STREQ("Item not found on server. Please contact your server administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4List_ServerReturnsNotEnoughRights_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>NotEnoughRights</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });


    CWSCCDATABUFHANDLE Project_V4s;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4List(api, &Project_V4s);
    ASSERT_TRUE(status == NOT_ENOUGH_RIGHTS);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4List_ServerReturnsSslRequired_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>SslRequired</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });


    CWSCCDATABUFHANDLE Project_V4s;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4List(api, &Project_V4s);
    ASSERT_TRUE(status == SSL_REQUIRED);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4List_ServerReturnsServerError_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::InternalServerError, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });


    CWSCCDATABUFHANDLE Project_V4s;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4List(api, &Project_V4s);
    ASSERT_TRUE(status == ERROR500);

    ASSERT_STREQ("Server error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4List_ServerReturnsConflict_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());

        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::Conflict, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });


    CWSCCDATABUFHANDLE Project_V4s;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4List(api, &Project_V4s);
    ASSERT_TRUE(status == ERROR409);

    ASSERT_STREQ("Unknown error. Please contact your server administrator.", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("Unexpected error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4List_ServerReturnsBadRequest_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::BadRequest, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });


    CWSCCDATABUFHANDLE Project_V4s;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4List(api, &Project_V4s);
    ASSERT_TRUE(status == ERROR400);

    ASSERT_STREQ("Unknown error. Please contact your server administrator.", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("Unexpected error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, CreateProject_V4_ServerReturnsClassNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>ClassNotFound</errorId><errorMessage>TestMessage</errorMessage><errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString OrgId = L"1001389117";
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    WString CountryCode = L"US";
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_CreateProject_V4(api, UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),
                                                                nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, CountryCode.c_str());
    ASSERT_TRUE(status == CLASS_NOT_FOUND);

    ASSERT_STREQ("Class not found on server. Please contact your server administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, CreateProject_V4_ServerReturnsSchemaNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>SchemaNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString OrgId = L"1001389117";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_CreateProject_V4(api, UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),
                                                                nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == SCHEMA_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, CreateProject_V4_ServerReturnsRepositoryNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>RepositoryNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString OrgId = L"1001389117";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_CreateProject_V4(api, UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),
                                                                nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == REPOSITORY_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, CreateProject_V4_ServerReturnsPropertyNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>PropertyNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString OrgId = L"1001389117";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_CreateProject_V4(api, UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),
                                                                nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == PROPERTY_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, CreateProject_V4_ServerReturnsNotEnoughRights_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>NotEnoughRights</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString OrgId = L"1001389117";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_CreateProject_V4(api, UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),
                                                                nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == NOT_ENOUGH_RIGHTS);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, CreateProject_V4_ServerReturnsSslRequired_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>SslRequired</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString OrgId = L"1001389117";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_CreateProject_V4(api, UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(),  Industry.c_str(), AssetType.c_str(),  
                                                                nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == SSL_REQUIRED);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, CreateProject_V4_ServerReturnsServerError_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::InternalServerError, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_CreateProject_V4(api, UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == ERROR500);

    ASSERT_STREQ("Server error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, CreateProject_V4_ServerReturnsConflict_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());

        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::Conflict, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString OrgId = L"1001389117";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    WString DataLocationGUID = L"99999999-9999-9999-9999-999999999999";
    CallStatus status = ConnectWebServicesClientC_CreateProject_V4(api, UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),
                                                                nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, DataLocationGUID.c_str(), nullptr);
    ASSERT_TRUE(status == ERROR409);

    ASSERT_STREQ("Unknown error. Please contact your server administrator.", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("Unexpected error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, CreateProject_V4_ServerReturnsBadRequest_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4", request.GetUrl().c_str());
        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::BadRequest, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_CreateProject_V4(api, UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == ERROR400);

    ASSERT_STREQ("Unknown error. Please contact your server administrator.", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("Unexpected error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4_ServerReturnsClassNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>ClassNotFound</errorId><errorMessage>TestMessage</errorMessage><errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CWSCCDATABUFHANDLE Project_V4;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4(api, instanceId, &Project_V4);
    ASSERT_TRUE(status == CLASS_NOT_FOUND);

    ASSERT_STREQ("Class not found on server. Please contact your server administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4_ServerReturnsSchemaNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>SchemaNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CWSCCDATABUFHANDLE Project_V4;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4(api, instanceId, &Project_V4);
    ASSERT_TRUE(status == SCHEMA_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4_ServerReturnsRepositoryNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>RepositoryNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CWSCCDATABUFHANDLE Project_V4;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4(api, instanceId, &Project_V4);
    ASSERT_TRUE(status == REPOSITORY_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4_ServerReturnsPropertyNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>PropertyNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CWSCCDATABUFHANDLE Project_V4;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4(api, instanceId, &Project_V4);
    ASSERT_TRUE(status == PROPERTY_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4_ServerReturnsInstanceNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>InstanceNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CWSCCDATABUFHANDLE Project_V4;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4(api, instanceId, &Project_V4);
    ASSERT_TRUE(status == INSTANCE_NOT_FOUND);

    ASSERT_STREQ("Item not found on server. Please contact your server administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4_ServerReturnsNotEnoughRights_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>NotEnoughRights</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CWSCCDATABUFHANDLE Project_V4;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4(api, instanceId, &Project_V4);
    ASSERT_TRUE(status == NOT_ENOUGH_RIGHTS);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4_ServerReturnsSslRequired_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>SslRequired</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CWSCCDATABUFHANDLE Project_V4;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4(api, instanceId, &Project_V4);
    ASSERT_TRUE(status == SSL_REQUIRED);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4_ServerReturnsServerError_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::InternalServerError, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });

    WCharP instanceId = L"blahId";
    CWSCCDATABUFHANDLE Project_V4;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4(api, instanceId, &Project_V4);
    ASSERT_TRUE(status == ERROR500);

    ASSERT_STREQ("Server error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4_ServerReturnsConflict_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());

        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::Conflict, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });

    WCharP instanceId = L"blahId";
    CWSCCDATABUFHANDLE Project_V4;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4(api, instanceId, &Project_V4);
    ASSERT_TRUE(status == ERROR409);

    ASSERT_STREQ("Unknown error. Please contact your server administrator.", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("Unexpected error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, ReadProject_V4_ServerReturnsBadRequest_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::BadRequest, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });

    WCharP instanceId = L"blahId";
    CWSCCDATABUFHANDLE Project_V4;
    CallStatus status = ConnectWebServicesClientC_ReadProject_V4(api, instanceId, &Project_V4);
    ASSERT_TRUE(status == ERROR400);

    ASSERT_STREQ("Unknown error. Please contact your server administrator.", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("Unexpected error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, UpdateProject_V4_ServerReturnsClassNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/instanceId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>ClassNotFound</errorId><errorMessage>TestMessage</errorMessage><errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString instanceId(L"instanceId");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString OrgId = L"1001389117";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_UpdateProject_V4(api, instanceId.c_str(), UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),
                                                                nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == CLASS_NOT_FOUND);

    ASSERT_STREQ("Class not found on server. Please contact your server administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, UpdateProject_V4_ServerReturnsSchemaNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/instanceId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>SchemaNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString instanceId(L"instanceId");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_UpdateProject_V4(api, instanceId.c_str(), UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(), nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == SCHEMA_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, UpdateProject_V4_ServerReturnsRepositoryNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/instanceId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>RepositoryNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString instanceId(L"instanceId");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_UpdateProject_V4(api, instanceId.c_str(), UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(), nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == REPOSITORY_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, UpdateProject_V4_ServerReturnsPropertyNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/instanceId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>PropertyNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString instanceId(L"instanceId");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_UpdateProject_V4(api, instanceId.c_str(), UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(), nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == PROPERTY_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, UpdateProject_V4_ServerReturnsInstanceNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/instanceId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>InstanceNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString instanceId(L"instanceId");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString OrgId = L"1001389117";
    WString DataLocationGUID = L"99999999-9999-9999-9999-999999999999";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_UpdateProject_V4(api, instanceId.c_str(), UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(), nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, DataLocationGUID.c_str(), nullptr);
    ASSERT_TRUE(status == INSTANCE_NOT_FOUND);

    ASSERT_STREQ("Item not found on server. Please contact your server administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, UpdateProject_V4_ServerReturnsNotEnoughRights_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/instanceId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>NotEnoughRights</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString instanceId(L"instanceId");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_UpdateProject_V4(api, instanceId.c_str(), UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),
                                                                nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == NOT_ENOUGH_RIGHTS);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, UpdateProject_V4_ServerReturnsSslRequired_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/instanceId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>SslRequired</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString instanceId(L"instanceId");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString OrgId = L"1001389117";
    bool Active = true;
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_UpdateProject_V4(api, instanceId.c_str(), UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),
                                                                nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == SSL_REQUIRED);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, UpdateProject_V4_ServerReturnsServerError_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/instanceId", request.GetUrl().c_str());
        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::InternalServerError, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString instanceId(L"instanceId");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_UpdateProject_V4(api, instanceId.c_str(), UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(),
                                                                nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == ERROR500);

    ASSERT_STREQ("Server error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, UpdateProject_V4_ServerReturnsConflict_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/instanceId", request.GetUrl().c_str());

        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::Conflict, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString instanceId(L"instanceId");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_UpdateProject_V4(api, instanceId.c_str(), UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(), nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == ERROR409);

    ASSERT_STREQ("Unknown error. Please contact your server administrator.", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("Unexpected error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, UpdateProject_V4_ServerReturnsBadRequest_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/instanceId", request.GetUrl().c_str());
        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::BadRequest, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });

    WString UltimateRefId(L"2c6f18fc-1ce8-4baf-a2eb-2f99b738eda1");
    WString instanceId(L"instanceId");
    WString Name(L"CWSCCTest%s");
    WString Number(L"CWSCCTest%s");
    WString Industry = L"8";
    WString AssetType = L"11";
    WString Location = L"Huntsville";
    double lat = 48.1231232;
    double lon = -25.12315411;
    bool LocationIsUsingLatLong = false;
    bool isRbacEnabled = true;
    CallStatus status = ConnectWebServicesClientC_UpdateProject_V4(api, instanceId.c_str(), UltimateRefId.c_str(), &isRbacEnabled, Name.c_str(), Number.c_str(), Industry.c_str(), AssetType.c_str(), nullptr, Location.c_str(), &lat, &lon, &LocationIsUsingLatLong, nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_TRUE(status == ERROR400);

    ASSERT_STREQ("Unknown error. Please contact your server administrator.", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("Unexpected error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, DeleteProject_V4_ServerReturnsClassNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>ClassNotFound</errorId><errorMessage>TestMessage</errorMessage><errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CallStatus status = ConnectWebServicesClientC_DeleteProject_V4(api, instanceId);
    ASSERT_TRUE(status == CLASS_NOT_FOUND);

    ASSERT_STREQ("Class not found on server. Please contact your server administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, DeleteProject_V4_ServerReturnsSchemaNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>SchemaNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CallStatus status = ConnectWebServicesClientC_DeleteProject_V4(api, instanceId);
    ASSERT_TRUE(status == SCHEMA_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, DeleteProject_V4_ServerReturnsRepositoryNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>RepositoryNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CallStatus status = ConnectWebServicesClientC_DeleteProject_V4(api, instanceId);
    ASSERT_TRUE(status == REPOSITORY_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, DeleteProject_V4_ServerReturnsPropertyNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>PropertyNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CallStatus status = ConnectWebServicesClientC_DeleteProject_V4(api, instanceId);
    ASSERT_TRUE(status == PROPERTY_NOT_FOUND);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, DeleteProject_V4_ServerReturnsInstanceNotFound_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>InstanceNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CallStatus status = ConnectWebServicesClientC_DeleteProject_V4(api, instanceId);
    ASSERT_TRUE(status == INSTANCE_NOT_FOUND);

    ASSERT_STREQ("Item not found on server. Please contact your server administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, DeleteProject_V4_ServerReturnsNotEnoughRights_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>NotEnoughRights</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CallStatus status = ConnectWebServicesClientC_DeleteProject_V4(api, instanceId);
    ASSERT_TRUE(status == NOT_ENOUGH_RIGHTS);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, DeleteProject_V4_ServerReturnsSslRequired_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        Utf8String body(R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>SslRequired</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)");
        return StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}});
        });

    WCharP instanceId = L"blahId";
    CallStatus status = ConnectWebServicesClientC_DeleteProject_V4(api, instanceId);
    ASSERT_TRUE(status == SSL_REQUIRED);

    ASSERT_STREQ("Resource not found", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("TestMessage\nTestDescription", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, DeleteProject_V4_ServerReturnsServerError_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::InternalServerError, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });

    WCharP instanceId = L"blahId";
    CallStatus status = ConnectWebServicesClientC_DeleteProject_V4(api, instanceId);
    ASSERT_TRUE(status == ERROR500);

    ASSERT_STREQ("Server error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, DeleteProject_V4_ServerReturnsConflict_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());

        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::Conflict, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });

    WCharP instanceId = L"blahId";
    CallStatus status = ConnectWebServicesClientC_DeleteProject_V4(api, instanceId);
    ASSERT_TRUE(status == ERROR409);

    ASSERT_STREQ("Unknown error. Please contact your server administrator.", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("Unexpected error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectWebServicesClientCTests, DeleteProject_V4_ServerReturnsBadRequest_APIReturnsProperStatus)
    {
    shared_ptr<MockHttpHandler> mockHttpHandler = GetHandlerPtr();
    mockHttpHandler->ExpectRequests(7);
    mockHttpHandler->ForRequest(1, StubHttpResponse());         //Buddi.Get() - For connectSignInManager
    mockHttpHandler->ForRequest(2, StubImsTokenHttpResponse()); //For connectSignInManager->SignInWithCredentials
    mockHttpHandler->ForRequest(3, StubHttpResponse());         //Buddi.Get() - For WSRepositoryClient::Create
    mockHttpHandler->ForRequest(4, StubHttpResponse());         //Buddi.Get() - For RequestAuthentication
    mockHttpHandler->ForRequest(5, StubImsTokenHttpResponse()); //For token->UpdateToken() in Update
    mockHttpHandler->ForRequest(6, StubWSInfoHttpResponseWebApi20());

    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials
        (m_username.c_str(),
        m_password.c_str(),
        s_temporaryDirectory.c_str(),
        s_assetsRootDirectory.c_str(),
        m_applicationName.c_str(),
        m_applicationVersion.c_str(),
        m_applicationGuid.c_str(),
        m_ccProductId.c_str(),
        nullptr,
        nullptr,
        nullptr,
        reinterpret_cast<void *>(&mockHttpHandler),
		nullptr
        );
    ASSERT_TRUE(api != nullptr);

    mockHttpHandler->ForRequest(7, [=] (RequestCR request)
        {
        EXPECT_STREQ("https://qa-wsg20-eus.cloudapp.net/v2.0/Repositories/BentleyCONNECT.Global--CONNECT.GLOBAL/GlobalSchema/Project_V4/blahId", request.GetUrl().c_str());
        auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
        return StubHttpResponse(HttpStatus::BadRequest, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });

    WCharP instanceId = L"blahId";
    CallStatus status = ConnectWebServicesClientC_DeleteProject_V4(api, instanceId);
    ASSERT_TRUE(status == ERROR400);

    ASSERT_STREQ("Unknown error. Please contact your server administrator.", ConnectWebServicesClientC_GetLastStatusMessage(api));
    ASSERT_STREQ("Unexpected error happened. Please contact your administrator", ConnectWebServicesClientC_GetLastStatusDescription(api));

    status = ConnectWebServicesClientC_FreeApi(api);
    ASSERT_TRUE(status == SUCCESS);
    }
//////////////////////////////////////////
// -- End Testing Error Code Returns--- //
//////////////////////////////////////////
