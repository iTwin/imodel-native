/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ClientIntegrationTests.h"
#include "TestsHelper.h"
#include "DummyPolicyHelper.h"

#include <Licensing/Utils/DateHelper.h>
#include <Licensing/Utils/SCVWritter.h>
#include "../../../Licensing/LicensingDb.h"

//#include "../../../Licensing/Providers/IAuthHandlerProvider.h"
//#include "../../../Licensing/Providers/IBuddiProvider.h"
//#include "../../../Licensing/Providers/IPolicyProvider.h"
//#include "../../../Licensing/Providers/IUlasProvider.h"

#include "../../../Licensing/Providers/AuthHandlerProvider.h"
#include "../../../Licensing/Providers/BuddiProvider.h"
#include "../../../Licensing/Providers/PolicyProvider.h"
#include "../../../Licensing/Providers/UlasProvider.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>
#include <fstream>
#include <Licensing/Utils/InMemoryJsonLocalState.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/SecurityToken.h>
#include <OidcNativeClient/OidcNative.h>


#define TEST_PRODUCT_ID      "2545"
#define TEST_PROJECT_ID      "77000000-0000-0000-0000-000000000077" 
#define OIDC_CLIENT_ID "cplc-integration-tests-guwkmv6sxwn4ro05bvei7l1r2"
#define OIDC_SCOPES "openid email profile entitlement-policy-service-2576 ulas-log-location-2728 ulas-realtime-log-posting-2733"

#define FIELD_APPLICATION_NAME  "Field"
#define FIELD_PRODUCT_ID        "2862"

#define TOKENPROVIDER_OIDC_Prefix       "Bearer"

// Project name: license_usage_test2
#define FIELD_PROJECT_ID_CoverExternalUsageCost     "2C0133ED-691B-4D16-80CE-79462BE90543"
// Project name: license_usage_test
#define FIELD_PROJECT_ID_NotCoverExternalUsageCost  "8d944112-752c-4a6e-a911-276e6fbe4c24"

// Project name: TES (covers external usage cost and registred by bentley account (bentley organization))
//#define FIELD_PROJECT_ID_CoverCostRegisteredByBentley  "dfc4d9bd-b5dd-4a49-96fd-58ae669be9ef"

// User info
#define FIELD_USER_UserName         "qa2_devuser2@mailinator.com"
#define FIELD_USER_Password         "bentley"
#define FIELD_USER_FirstName        "Bender"
#define FIELD_USER_LastName         "Benderson"
#define FIELD_USER_UserId           "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa"
#define FIELD_USER_OrganizationId   "A0DC5A35-7A81-47A5-9380-5F54BF040C43"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_INTEGRATION_TESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE

using namespace OidcInterop;

BeFileName ClientIntegrationTests::GetLicensingDbPathIntegration() const
    {
    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    path.AppendToPath(L"License.db");

    return path;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientIntegrationTests::ClientIntegrationTests() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientIntegrationTests::TearDown() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientIntegrationTests::SetUpTestCase()
    {
    // This is only an example of how to set logging severity and see info logs. Usually should be set more globally than in TestCase SetUp
    //NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_INFO);
    //NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_DEBUG);
    //NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_TRACE);

    BeFileName asssetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(asssetsDir);
    HttpClient::Initialize(asssetsDir);

    BeFileName tmpDir;
    BeTest::GetHost().GetTempDir(tmpDir);
    BeSQLiteLib::Initialize(tmpDir);

    BeFileName path;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(path);
    path.AppendToPath(L"TestAssets/sqlang/DgnClientFx_en.sqlang.db3");

    ASSERT_EQ(SUCCESS, L10N::Initialize(BeSQLite::L10N::SqlangFiles(path)));
    }

ClientPtr ClientIntegrationTests::CreateTestClient(bool signIn, Utf8StringCR productId) const
    {
    //InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    RuntimeJsonLocalState* localState = new RuntimeJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);

    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    // NOTE (7/30/19): all of these tests fail because this policy is expired... see if we can renew this
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto manager = ConnectSignInManager::Create(clientInfo, proxy, localState);
    if (signIn)
        {
        Credentials credentials("qa2_devuser2@mailinator.com", "bentley");
        auto result = manager->SignInWithCredentials(credentials)->GetResult();
        if (!result.IsSuccess())
            {
            LOG.infov("error: %s, %s", result.GetError().GetMessage().c_str(), result.GetError().GetDescription().c_str());
            return nullptr;
            }
        }

    BeFileName dbPath = GetLicensingDbPathIntegration();

    return Client::Create
        (
        manager->GetUserInfo(),
        clientInfo,
        manager,
        dbPath,
        true,
        "",
        "",
        proxy
        );
    }

ClientImplPtr ClientIntegrationTests::CreateTestClientImpl(bool signIn, Utf8StringCR productId) const
    {
    //InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    RuntimeJsonLocalState* localState = new RuntimeJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);

    //NOTE (5/28/19): the policy for this test user and product ID seems to be trial now so some integration tests fail. Will take a look at this...
    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto manager = ConnectSignInManager::Create(clientInfo, proxy, localState);
    if (signIn)
        {
        Credentials credentials("qa2_devuser2@mailinator.com", "bentley");
        auto result = manager->SignInWithCredentials(credentials)->GetResult();
        if (!result.IsSuccess())
            {
            LOG.infov("error: %s, %s", result.GetError().GetMessage().c_str(), result.GetError().GetDescription().c_str());
            return nullptr;
            }
        }
    BeFileName dbPath = GetLicensingDbPathIntegration();

    AuthType authType = AuthType::SAML;

    ApplicationInfoPtr applicationInfo = std::make_shared<ApplicationInfo>(clientInfo->GetApplicationVersion(), clientInfo->GetDeviceId(), clientInfo->GetApplicationProductId());
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IAuthHandlerProviderPtr authHandlerProvider = std::make_shared<AuthHandlerProvider>(manager, proxy);
    IPolicyProviderPtr policyProvider = std::make_shared<PolicyProvider>(buddiProvider, applicationInfo, proxy, authType, authHandlerProvider);
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, proxy);

    return std::make_shared<ClientImpl>
        (
        manager->GetUserInfo(),
        applicationInfo,
        dbPath,
        true,
        policyProvider,
        ulasProvider,
        "",
        "",
        nullptr
        );
    }

struct OidcTokenProvider : public IConnectTokenProvider
    {
    private:
        ISecurityTokenPtr m_token;

        ISecurityTokenPtr GetOidcToken()
            {
            //auto oidcToken = "eyJhbGciOiJSUzI1NiIsImtpZCI6IkJlbnRsZXlRQSIsInBpLmF0bSI6ImE4bWUifQ.eyJzY29wZSI6WyJlbWFpbCIsImltb2RlbGh1YiIsIm9mZmxpbmVfYWNjZXNzIiwib3BlbmlkIiwib3JnYW5pemF0aW9uIiwicHJvZmlsZSIsInByb2plY3R3aXNlLXNoYXJlIiwiYWdlbmN5LWluc3BlY3Rpb24tZGF0YTpyZWFkIiwiY29udGV4dC1yZWdpc3RyeS1zZXJ2aWNlIiwiZW50aXRsZW1lbnQtcG9saWN5LXNlcnZpY2UtMjU3NiIsImZlYXR1cmVfdHJhY2tpbmciLCJmb3Jtcy1kYXRhLWFjY2Vzcy0yMzQ2IiwibWFwLWxheWVyIiwicGRmbWFya3VwLXNlcnZpY2UtMjc1NyIsInJlcG9zaXRvcnktZmVkZXJhdGlvbi0yNTY3IiwidWxhcy1sb2ctbG9jYXRpb24tMjcyOCIsIndlYXRoZXItc2VydmljZSJdLCJjbGllbnRfaWQiOiJzeW5jaHJvLWZpZWxkLWlvcyIsImF1ZCI6WyJodHRwczovL3FhLWltcy5iZW50bGV5LmNvbS9hcy90b2tlbi5vYXV0aDIiLCJodHRwczovL3FhLWltc29pZGMuYmVudGxleS5jb20vYXMvdG9rZW4ub2F1dGgyIiwiaHR0cHM6Ly9xYTItaW1zLmJlbnRsZXkuY29tL2FzL3Rva2VuLm9hdXRoMiIsImh0dHBzOi8vcWEyLWltc29pZGMuYmVudGxleS5jb20vYXMvdG9rZW4ub2F1dGgyIiwiaHR0cHM6Ly9xYS1pbXNvaWRjLmJlbnRsZXkuY29tL3Jlc291cmNlcyIsImh0dHBzOi8vcWEyLWltcy5iZW50bGV5LmNvbS9yZXNvdXJjZXMiLCJpbW9kZWwtaHViLXNlcnZpY2VzLTI0ODUiLCJwcm9qZWN0d2lzZS1zaGFyZS0yNTY3IiwiYWdlbmN5LWluc3BlY3Rpb24tc2VydmljZS0yNzU0IiwiY29udGV4dC1yZWdpc3RyeS0yNzc3IiwiZW50aXRsZW1lbnQtcG9saWN5LXNlcnZpY2UtMjU3NiIsImZvcm1zLWRhdGEtYWNjZXNzLTIzNDYiLCJtYXAtbGF5ZXItc2VydmljZSIsInBkZm1hcmt1cC1zZXJ2aWNlLTI3NTciLCJyZXBvc2l0b3J5LWZlZGVyYXRpb24tMjU2NyIsInVsYXMtbG9nLWxvY2F0aW9uLTI3MjgiLCJiZW50bGV5LXdlYXRoZXItc2VydmljZS0yOTIwIl0sInN1YiI6IjUwYzM2M2E4LWVmMzMtNDA5NS1hN2UwLTM4MjhjYjk5N2ViZiIsInJvbGUiOlsiU0lURV9BRE1JTklTVFJBVE9SIiwiQ09OTkVDVCBRdWFsaXR5IEFzc3VyYW5jZSJdLCJvcmciOiIxY2FlZjg3NS1mMjJlLTRiNmItOTFkMC1mZTliYjA5NWEwYWYiLCJzdWJqZWN0IjoiNTBjMzYzYTgtZWYzMy00MDk1LWE3ZTAtMzgyOGNiOTk3ZWJmIiwiaXNzIjoiaHR0cHM6Ly9xYS1pbXNvaWRjLmJlbnRsZXkuY29tIiwiZW50aXRsZW1lbnQiOiJlbnRpdGxlbWVudCIsInByZWZlcnJlZF91c2VybmFtZSI6ImJjY191c2VyNkBtYWlsaW5hdG9yLmNvbSIsImdpdmVuX25hbWUiOiJGcmFuayIsInNpZCI6IkY4SElaOXVFdE1sejQ4RFlXdVhRTWgzT3NuOC5VVUZKVFZNdFFtVnVkR3hsZVMxRVJRLnNxTTkuUjIyelU4TklEMjB1MER6VDBaRlVRVXJuMyIsIm5iZiI6MTU4ODc2NTIyNSwidWx0aW1hdGVfc2l0ZSI6IjEwMDEzODcxMTYiLCJ1c2FnZV9jb3VudHJ5X2lzbyI6IlVTIiwiYXV0aF90aW1lIjoxNTg4NzY1NTI1LCJuYW1lIjoiYmNjX3VzZXI2QG1haWxpbmF0b3IuY29tIiwib3JnX25hbWUiOiJDb3Jud2FsbCBDb3VuY2lsIiwiZmFtaWx5X25hbWUiOiJGZWxjaGVyIiwiZW1haWwiOiJiY2NfdXNlcjZAbWFpbGluYXRvci5jb20iLCJleHAiOjE1ODg5Mzk4OTV9.uayW6Xl7obGqLB9G38G5BgB8BmAqwi-E6_n989WCcYp120PWH2Viu9gNrT36hYfPAmoCtHgftOM8P1VRB7p63tUy7PWSr0_45ydg3Q8eZRqRaVYcMRhyGiWKbazzG_ojYHqpu8j6_NWYCbXRi935EPt2RFwe07ZiUMZTuSf0kKPRXwDBD627OUZbTN1_5u6x7YTsFMa8MSmMTL6Rk9i8-rLDKHDvi-F3646dJFlZVOyxyXVJfZ0Gp6GgqU8ZpeLd69NEVoTN7tq2oToW58hJkEt0BVUCUFf2JkOu_JglDdA7zVxJBmK0-7mUjJIFkAryyURqbaxqPr1_HS1LuMKCuQ";
            //auto oidcToken = OIDCNative::IssueToken(FIELD_USER_UserName, FIELD_USER_Password, UrlProvider::Urls::IMSOpenID.Get().c_str(), UrlProvider::Urls::IMSOpenID.Get().c_str(), OIDC_CLIENT_ID, OIDC_SCOPES);
            auto oidcToken = "0f1ccd6b5fdc75c9956dec242f746bfd0dd0c05b1e8be0f4516a5d1d7d57fe00";
            return std::make_shared<SecurityToken>(oidcToken, DateTime::DateTime(2021, 1, 1));
            }

    public:
        OidcTokenProvider()
            {
            m_token = GetOidcToken();
            }

        AsyncTaskPtr<ISecurityTokenResult> UpdateTokenWithResult() //override
            {
            if (m_token == nullptr)
                m_token = GetOidcToken();

            return CreateCompletedAsyncTask<ISecurityTokenResult>(ISecurityTokenResult::Success(m_token));
            }

        AsyncTaskPtr<ISecurityTokenPtr> UpdateToken()// override
            {
            return UpdateTokenWithResult()
                ->Then<ISecurityTokenPtr>([=] (ISecurityTokenResult result)
                {
                return result.GetValue();
                });
            }

        ISecurityTokenPtr GetToken() //override
            {
            return m_token;
            }
    };

struct OidcAuthenticationProvider : IConnectAuthenticationProvider
    {
    AuthenticationHandlerPtr GetAuthenticationHandler(Utf8StringCR serverUrl, IHttpHandlerPtr httpHandler = nullptr, HeaderPrefix prefix = HeaderPrefix::Token) const override
        {
        auto configurationHandler = UrlProvider::GetSecurityConfigurator(httpHandler);
        auto tokenProvider = OidcTokenProvider();
        auto connectHandler = ConnectAuthenticationHandler::Create
        (
            serverUrl,
            std::make_shared<OidcTokenProvider>(tokenProvider),
            configurationHandler,
            TOKENPROVIDER_OIDC_Prefix
        );

        return connectHandler;
        }
    };

ClientPtr ClientIntegrationTests::CreateOidcTestClient() const
    {

    ConnectSignInManager::UserInfo m_userInfo;
    m_userInfo.username = FIELD_USER_UserName;
    m_userInfo.firstName = FIELD_USER_FirstName;
    m_userInfo.lastName = FIELD_USER_LastName;
    m_userInfo.userId = FIELD_USER_UserId;
    //m_userInfo.userId = "00000000-0000-0000-0000-000000000000";
    m_userInfo.organizationId = FIELD_USER_OrganizationId;

    RuntimeJsonLocalState* localState = new RuntimeJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);
    auto clientInfo = std::make_shared<ClientInfo>(FIELD_APPLICATION_NAME, BeVersion(1, 0), FIELD_PROJECT_ID_CoverExternalUsageCost, "TestDeviceId", "TestSystem", FIELD_PRODUCT_ID);
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    BeFileName dbPath = GetLicensingDbPathIntegration();

    // Fake Auth Provider
    auto authProviderTest = OidcAuthenticationProvider();

    return Client::Create
    (
        m_userInfo,
        clientInfo,
        std::make_shared<OidcAuthenticationProvider>(authProviderTest),
        dbPath,
        true,
        "",
        "",
        proxy,
        AuthType::OIDC
    );
    }

TEST_F(ClientIntegrationTests, DISABLED_StartStopApplicationForProject_OidcSigninAndProjectCoverExternalUsageCosts_Success)
    {
    auto client = CreateOidcTestClient();
    if (nullptr == client)
        {
        FAIL() << "client is null";
        }

    auto startStatus = client->StartApplicationForProject(FIELD_PROJECT_ID_CoverExternalUsageCost);
    //auto startStatus = client->StartApplicationForProject(FIELD_PROJECT_ID_CoverExternalUsageCost);
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));

    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, DISABLED_StartStopApplicationForProject_OidcSigninAndProjectNotCoverExternalUsageCosts_Success)
    {
    auto client = CreateOidcTestClient();
    if (nullptr == client)
        {
        FAIL() << "client is null";
        }

    auto startStatus = client->StartApplicationForProject(FIELD_PROJECT_ID_NotCoverExternalUsageCost);
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));

    EXPECT_SUCCESS(client->StopApplication());
    }


TEST_F(ClientIntegrationTests, DISABLED_Equality_Test)
    {
    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    ASSERT_EQ((int)1, (int)1); // Mock policy should result in NotEntitled
    }

// Tests using the Client's Create method
TEST_F(ClientIntegrationTests, FactoryStartStopApplication_Success)
    {
    auto client = CreateTestClient(true, TEST_PRODUCT_ID);
    if (nullptr == client)
        {
        FAIL() << "client is null";
        }
    auto startStatus = client->StartApplication();
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, FactoryStartStopApplicationForProject_Success)
    {
    auto client = CreateTestClient(true, TEST_PRODUCT_ID);
    if (nullptr == client)
        {
        FAIL() << "client is null";
        }
    auto startStatus = client->StartApplicationForProject(TEST_PROJECT_ID);
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));

    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    EXPECT_SUCCESS(client->StopApplication());
    }

// Tests using the Clients' implementation
TEST_F(ClientIntegrationTests, StartStopApplication_Success)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);
    auto startStatus = client->StartApplication();
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));

    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, StartStopApplicationForProject_Success)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);
    auto startStatus = client->StartApplicationForProject(TEST_PROJECT_ID);

    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));

    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, MarkFeature_Success)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);

    FeatureUserDataMapPtr featureData = std::make_shared<FeatureUserDataMap>();

    featureData->AddAttribute("Manufacturer", "Bentley Systems, Inc.");
    featureData->AddAttribute("Website", "https://www.w3schools.com");
    featureData->AddAttribute("Title", "Mobile App");

    auto startStatus = client->StartApplication();
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));

    client->AddPolicyToDb(Policy::Create(jsonPolicyValid));

    EXPECT_EQ(static_cast<int>(client->MarkFeature("TestFeatureId", featureData)), static_cast<int>(BentleyStatus::SUCCESS));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, GetLicenseStatusValidTrialPolicy_Test)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValidTrial = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);   

    client->AddPolicyToDb(Policy::Create(jsonPolicyValidTrial));

    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Trial));

	auto startStatus = client->StartApplication();
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, GetLicenseStatusValidExpiredTrialPolicy_Test)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValidTrialExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(-1), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);

   

    client->AddPolicyToDb(Policy::Create(jsonPolicyValidTrialExpired));

    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Expired));

	auto startStatus = client->StartApplication();
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));

    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, GetLicenseStatusValidPolicyWithGracePeriod_Test)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);

    auto timestamp = DateHelper::GetCurrentTime();
    auto timestampPast = DateHelper::AddDaysToCurrentTime(-14); // Two weeks ago; default offline period allowed is only 1 week

    auto startStatus = client->StartApplication();
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));

    client->AddPolicyToDb(Policy::Create(jsonPolicyValid));

    client->GetLicensingDb().SetOfflineGracePeriodStart(timestamp);
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Offline)); // Valid status should be Offline now
    client->GetLicensingDb().ResetOfflineGracePeriod();
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Ok)); // Should be back to Ok
    client->GetLicensingDb().SetOfflineGracePeriodStart(timestampPast);
    EXPECT_EQ(client->GetLicensingDb().GetOfflineGracePeriodStart(), timestampPast);
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Expired)); // Valid status should be Expired now, since offline grace period has expired
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, GetLicenseStatusOfflineNotAllowedPolicy_Test)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyOfflineNotAllowed = DummyPolicyHelper::CreatePolicyOfflineNotAllowed(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);

    auto timestamp = DateHelper::GetCurrentTime();    

    client->AddPolicyToDb(Policy::Create(jsonPolicyOfflineNotAllowed));

    client->GetLicensingDb().SetOfflineGracePeriodStart(timestamp);
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::DisabledByPolicy)); // Grace Period started; should be disabled
    client->GetLicensingDb().ResetOfflineGracePeriod();
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Ok)); // Should be back to Ok
    
	auto startStatus = client->StartApplication();
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));
	
	EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, GetTrialDaysRemainingValidTrialPolicy_Test)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValidTrial = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);

    // NOTE: we don't need to start application here since we are not testing the heartbeat, and are manually adding a policy

    client->AddPolicyToDb(Policy::Create(jsonPolicyValidTrial));

    EXPECT_EQ(client->GetTrialDaysRemaining(), 6);
    }

TEST_F(ClientIntegrationTests, GetTrialDaysRemainingValidExpiredTrialPolicy_Test)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValidTrialExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(-1), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);

    // NOTE: we don't need to start application here since we are not testing the heartbeat, and are manually adding a policy

    client->AddPolicyToDb(Policy::Create(jsonPolicyValidTrialExpired));

    EXPECT_EQ(client->GetTrialDaysRemaining(), 0);
    }

TEST_F(ClientIntegrationTests, ImportBelicFile_UseIt_DeleteIt)
{
	auto client = CreateTestClientImpl(false, TEST_PRODUCT_ID); //works logged in or not, testing logged out. 
	BeFileName testbelic;
	BeTest::GetHost().GetDgnPlatformAssetsDirectory(testbelic);
	testbelic.AppendToPath(L"TestAssets/DA7CDA78-92D0-4C80-82C8-C35C479E5D0E_TestDeviceId.belic");
	auto result = client->ImportCheckout(testbelic);
	EXPECT_EQ(result, 0);
	auto startStatus = client->StartApplication(); 	
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));
	EXPECT_SUCCESS(client->StopApplication());
    client->DeleteLocalCheckout("2545");
    startStatus = client->StartApplication();
    ASSERT_EQ(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
    EXPECT_SUCCESS(client->StopApplication());
}

TEST_F(ClientIntegrationTests, DeleteLocalCheckout_InvalidParams)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);
    auto result = client->DeleteLocalCheckout("123");
    EXPECT_EQ(result, ERROR);
    result = client->DeleteLocalCheckout("1a3b");
    EXPECT_EQ(result, ERROR);
    result = client->DeleteLocalCheckout("12.12");
    EXPECT_EQ(result, ERROR);
    result = client->DeleteLocalCheckout("1$12");
    EXPECT_EQ(result, ERROR);
    result = client->DeleteLocalCheckout("9999");//Valid params not in DB should return success, nothing to delete is still a success. 
    EXPECT_EQ(result, SUCCESS);
    }

TEST_F(ClientIntegrationTests, ImportBelicFile_MismatchedDeviceID) 
{
	auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);
	BeFileName testbelic;
	BeTest::GetHost().GetDgnPlatformAssetsDirectory(testbelic);
	testbelic.AppendToPath(L"TestAssets/DA7CDA78-92D0-4C80-82C8-C35C479E5D0E_baddevicefortest.belic");
	auto result = client->ImportCheckout(testbelic);
	EXPECT_EQ(result, -2);
}

TEST_F(ClientIntegrationTests, ImportBelicFile_FileNonExistant)
{
	auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);
	BeFileName testbelic;
	BeTest::GetHost().GetDgnPlatformAssetsDirectory(testbelic);
	testbelic.AppendToPath(L"TestAssets/55555555-92D0-4C80-82C8-C35C479E5D0E_naou23106.belic");
	auto result = client->ImportCheckout(testbelic);
	EXPECT_EQ(result, -1);
}

TEST_F(ClientIntegrationTests, ImportBelicFile_WrongFileExtension)
{
	auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);
	BeFileName testbelic;
	BeTest::GetHost().GetDgnPlatformAssetsDirectory(testbelic);
	testbelic.AppendToPath(L"TestAssets/randomtextfile.txt");
	auto result = client->ImportCheckout(testbelic);
	EXPECT_EQ(result, -1);
}


// TODO: heartbeat tests, different LicenseStatus situations

