/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Published/SaasClientIntegrationTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SaasClientIntegrationTests.h"
#include "TestsHelper.h"
#include "DummyPolicyHelper.h"

#include <Licensing/Utils/DateHelper.h>
#include <Licensing/Utils/SCVWritter.h>
#include "../../../Licensing/LicensingDb.h"
//#include "../Helpers/OIDC/OidcSignInManager.h"

//#include "../../../Licensing/Providers/IAuthHandlerProvider.h"
//#include "../../Licensing/Providers/IBuddiProvider.h"
//#include "../../Licensing/Providers/IPolicyProvider.h"
//#include "../../Licensing/Providers/IUlasProvider.h"

#include "../../../Licensing/Providers/BuddiProvider.h"
#include "../../../Licensing/Providers/UlasProvider.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>
#include <fstream>
#include <Licensing/Utils/InMemoryJsonLocalState.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

#include  <OidcNativeClient/OidcNative.h>

#define TEST_PRODUCT_ID      "2545"
#define OIDC_CLIENT_ID "cplc-integration-tests-guwkmv6sxwn4ro05bvei7l1r2"
#define OIDC_SCOPES "openid email profile entitlement-policy-service-2576 ulas-log-location-2728 ulas-realtime-log-posting-2733"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_INTEGRATION_TESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE

using namespace OidcInterop;

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SaasClientIntegrationTests::SaasClientIntegrationTests() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SaasClientIntegrationTests::TearDown() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SaasClientIntegrationTests::SetUpTestCase()
    {
    // This is only an example of how to set logging severity and see info logs. Usually should be set more globally than in TestCase SetUp
    // NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_DEBUG);

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

SaasClientPtr SaasClientIntegrationTests::CreateTestSaasClient(int productId) const
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    return SaasClient::Create
        (
        productId,
        "",
        proxy
        );
    }

SaasClientImplPtr SaasClientIntegrationTests::CreateTestSaasClientImpl(int productId) const
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, proxy);

    //auto oidcManager = OidcSignInManager();

    return std::make_shared<SaasClientImpl>
        (
        productId,
        "",
        ulasProvider
        );
    }

TEST_F(SaasClientIntegrationTests, DISABLED_Equality_Test)
    {
    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    ASSERT_EQ((int)1, (int)1); // Mock policy should result in NotEntitled
    }

// Tests using the Client's Create method
TEST_F(SaasClientIntegrationTests, FactoryTrackUsage_Success)
    {
    auto client = CreateTestSaasClient(std::atoi(TEST_PRODUCT_ID));

    auto oidcToken = OIDCNative::IssueToken("qa2_devuser2@mailinator.com", "bentley", UrlProvider::Urls::IMSOpenID.Get().c_str(), OIDC_CLIENT_ID, OIDC_SCOPES);
    LOG.debugv("token issued: %s", Utf8CP(oidcToken));
    auto version = BeVersion(1, 0);
    Utf8String projectId = "00000000-0000-0000-0000-000000000000";

    EXPECT_NE(static_cast<int>(client->TrackUsage(oidcToken, version, projectId).get()), static_cast<int>(BentleyStatus::ERROR));
    }

TEST_F(SaasClientIntegrationTests, FactoryMarkFeature_Success)
    {
    auto client = CreateTestSaasClient(std::atoi(TEST_PRODUCT_ID));

    auto oidcToken = OIDCNative::IssueToken("qa2_devuser2@mailinator.com", "bentley", UrlProvider::Urls::IMSOpenID.Get().c_str(), OIDC_CLIENT_ID, OIDC_SCOPES);
    LOG.debugv("token issued: %s", Utf8CP(oidcToken));

    auto version = BeVersion(1, 0);
    Utf8String projectId = "00000000-0000-0000-0000-000000000000";
    Utf8String featureId = "e49af0e9-1d2b-4385-b5c5-ce50b07693d3"; // featureID must be a GUID

    FeatureUserDataMapPtr featureData = std::make_shared<FeatureUserDataMap>();

    featureData->AddAttribute("Manufacturer", "Bentley Systems, Inc.");
    featureData->AddAttribute("Website", "https://www.w3schools.com");
    featureData->AddAttribute("Title", "Mobile App");

    FeatureEvent featureEvent = FeatureEvent(featureId, version, projectId, featureData);

    EXPECT_NE(static_cast<int>(client->MarkFeature(oidcToken, featureEvent).get()), static_cast<int>(BentleyStatus::ERROR));
    }

// Tests using the Client's implementation
TEST_F(SaasClientIntegrationTests, TrackUsage_Success)
    {
    auto client = CreateTestSaasClientImpl(std::atoi(TEST_PRODUCT_ID));

    auto oidcToken = OIDCNative::IssueToken("qa2_devuser2@mailinator.com", "bentley", UrlProvider::Urls::IMSOpenID.Get().c_str(), OIDC_CLIENT_ID, OIDC_SCOPES);
    LOG.debugv("token issued: %s", Utf8CP(oidcToken));

    auto version = BeVersion(1, 0);
    Utf8String projectId = "00000000-0000-0000-0000-000000000000";

    EXPECT_NE(static_cast<int>(client->TrackUsage(oidcToken, version, projectId, std::atoi(TEST_PRODUCT_ID), "", UsageType::Production, "").get()), static_cast<int>(BentleyStatus::ERROR));
    }

TEST_F(SaasClientIntegrationTests, MarkFeature_Success)
    {
    auto client = CreateTestSaasClientImpl(std::atoi(TEST_PRODUCT_ID));

    auto oidcToken = OIDCNative::IssueToken("qa2_devuser2@mailinator.com", "bentley", UrlProvider::Urls::IMSOpenID.Get().c_str(), OIDC_CLIENT_ID, OIDC_SCOPES);
    LOG.debugv("token issued: %s", Utf8CP(oidcToken));

    auto version = BeVersion(1, 0);
    Utf8String projectId = "00000000-0000-0000-0000-000000000000";
    Utf8String featureId = "e49af0e9-1d2b-4385-b5c5-ce50b07693d3"; // featureID must be a GUID

    FeatureUserDataMapPtr featureData = std::make_shared<FeatureUserDataMap>();

    featureData->AddAttribute("Manufacturer", "Bentley Systems, Inc.");
    featureData->AddAttribute("Website", "https://www.w3schools.com");
    featureData->AddAttribute("Title", "Mobile App");

    FeatureEvent featureEvent = FeatureEvent(featureId, version, projectId, featureData);

    EXPECT_NE(static_cast<int>(client->MarkFeature(oidcToken, featureEvent, std::atoi(TEST_PRODUCT_ID), "", UsageType::Production, "").get()), static_cast<int>(BentleyStatus::ERROR));
    }
