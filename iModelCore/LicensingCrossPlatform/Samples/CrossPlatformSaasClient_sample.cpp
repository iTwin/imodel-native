/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

// Example of how to initialize the Cross Platform License Client (SaasClient flavor), post usage data, and post feature data
// this assumes that you have either the DgnClientFxSdk, or iModelCore libraries

// ----------- in header file ------------
#include <Licensing/SaasClient.h>

Licensing::SaasClient m_licensingClient;
bool m_licensingActive = false;
JsonLocalState* m_localState;

// ----------- in .cpp file --------------
// start the application
void StartProduct()
    {
    InitLicensing();

    StartLicensing();
    }

// close/cleanup the application
void CleanUpProduct()
    {
    Http::HttpClient::Uninitialize();
    WebServices::UrlProvider::Uninitialize();
    BeSQLite::L10N::Shutdown();
    delete m_localState;
    }

// initialize required values, and create the client
// this assumes that none of these values were previously created
// in practice some or all of these required values may already be initialized in a given application,
// pass in or reference existing values as desired
void InitLicensing()
    {
    BeFileName assetsDir(BeFileName::FileNameParts::Directory, L"./assets/");

    // init HttpClient for http calls
    Http::HttpClient::Initialize(assetsDir); // need certs at /assets/http/cabundle.pem, can be found in iModelCore source

    // init L10N sqlite lib for string localization
    BeFileName sqlangPath(assetsDir);
    sqlangPath.AppendToPath(L"sqlang/DgnClientFx_en.sqlang.db3"); // db file found in iModelCore source
    BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(path));

    // OPTIONAL - initialize proxy http handler if desired, such as a fiddler proxy for debugging
    proxyHttpHandler = ProxyHttpHandler::GetFiddlerProxyIfReachable(); // nullptr uses the default http handler

    // initialize local state
    m_localState = new RuntimeJsonLocalState();

    // initialize UrlProvider
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, m_localState);

    // create the client
    m_licensingClient = SaasClient::Create
        (
        1000, // OPTIONAL - projectId to use for all usage/feature logs
        "", // OPTIONAL - featureString
        proxyHttpHandler // OPTIONAL - custom HttpHandler, pass nullptr for default HttpHandler
        );
    }

// call TrackUsage to post usage in real time
void TrackUsage()
    {
    if (nullptr != m_licensingClient)
        {
        Utf8String accessToken = "token"; // OIDC token of user
        BeVersion version = BeVersion(1, 0); // application version
        Utf8String projectId = "00000000-0000-0000-0000-000000000000"; // GUID

        // you may wish to add logic to handle StartApplication not returning Success
        // NB: optional parameters are passed here with their default values
        m_licensingClient->TrackUsage
            (
            accessToken,
            version,
            projectId,
            AuthType::OIDC, // OPTIONAL - pass AuthType::SAML if accessToken is a SAML token, defaults to OIDC
            -1, // OPTIONAL - pass a product ID for this usage, defaults to productId specified on create
            "", // OPTIONAL - pass a deviceId for this usage, defaults to system deviceId
            UsageType::Production, // OPTIONAL - pass a usage type for this usage, defaults to Production
            "" // OPTIONAL - pass a correlationId for this usage, defaults to a new GUID
            );
        }
    }

// call MarkFeature to post a feature log in real time
void MarkFeature()
    {
    if (nullptr != m_licensingClient)
        {
        Utf8String featureId = "00000000-0000-0000-0000-000000000000"; // must be a GUID
        Utf8String accessToken = "token"; // OIDC token of user
        BeVersion version = BeVersion(1, 0); // application version
        Utf8String projectId = "00000000-0000-0000-0000-000000000000"; // GUID
        FeatureUserDataMapPtr featureData = std::make_shared<FeatureUserDataMap>();
        featureData->AddAttribute("Manufacturer", "Bentley Systems, Inc.");

        Licensing::FeatureEvent event = Licensing::FeatureEvent(featureId, version, projectId, featureData);

        // you may wish to add logic to handle StartApplication not returning Success
        m_licensingClient->MarkFeature
            (
            accessToken,
            event,
            AuthType::OIDC, // OPTIONAL - pass AuthType::SAML if accessToken is a SAML token, defaults to OIDC
            -1, // OPTIONAL - pass a product ID for this usage, defaults to productId specified on create
            "", // OPTIONAL - pass a deviceId for this usage, defaults to system deviceId
            UsageType::Production, // OPTIONAL - pass a usage type for this usage, defaults to Production
            "" // OPTIONAL - pass a correlationId for this usage, defaults to a new GUID
            );
        }
    }
