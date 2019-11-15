/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

// Example of how to initialize the Cross Platform License Client, start the licensing heartbeat, stop the licensing heartbeat, and post feature data
// this assumes that you have either the DgnClientFxSdk, or iModelCore libraries

// ----------- in header file ------------
#include <Licensing/Client.h>

Licensing::Client m_licensingClient;
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
    StopLicensing();

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
    Http::HttpClient::Initialize(assetsDir); // need certs at /assets/http/cabundle.pem

    // init L10N sqlite lib for logging
    BeFileName sqlangPath(assetsDir);
    sqlangPath.AppendToPath(L"sqlang/DgnClientFx_en.sqlang.db3"); // db file found in iModelCore source
    BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(path));

    // initialize BeSQLiteLib for db
    BeFileName tempDir;
    Desktop::FileSystem::BeGetTempPath(tempDir);
    BeSQLite::BeSQLiteLib::Initialize(tempDir);

    // initialize local json state
    m_localState = new RuntimeJsonLocalState();

    // initialize UrlProvider
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, m_localState);

    // initialize client info
    WebServices::ClientInfoPtr clientInfo = std::make_shared<WebServices::ClientInfo>
        (
        "Bentley-Test", // human readable string with company and application name. Format: "Bentley-TestApplication"
        BeVersion(1, 0), // application verison to identify application server side
        "00000000-0000-0000-0000-000000000000", // application GUID used for registering WSG usage
        "TestDeviceId", // unique device ID for licensing. Should be different for different devices
        "TestSystem", // system description for User-Agent header
        "0000" // product ID used for IMS sign-in
        );

    // OPTIONAL: initialize proxy http handler if desired, such as a fiddler proxy for debugging
    auto proxyHttpHandler = ProxyHttpHandler::GetFiddlerProxyIfReachable(); // nullptr uses the default http handler

    // initialize ConnectSignInManager
    auto manager = ConnectSignInManager::Create(clientInfo, proxyHttpHandler, m_localState); // change auto

    // sign in to authenticate the manager
    Credentials credentials("username", "password");
    auto signInResult = manager->SignInWithCredentials(credentials)->GetResult();
    if (!signInResult->IsSuccess())
        {
        // log sign in error if desired
        // AsyncError(signInResult->GetError().GetMessage(), signInResult->GetError().GetDescription());
        return nullptr;
        }

    // initialize database path
    BeFileName dbPath;
    BeTest::GetHost().GetTempDir(path); // some method to get temporary directory for the Licensing database
    path.AppendToPath(L"License.db");

    // create the client
    m_licensingClient = Client::Create
        (
        manager->GetUserInfo(),
        clientInfo, // can also pass a Licensing::ApplicationInfo here
        manager,
        dbPath,
        true, // offline mode
        "", // projectId
        "", // featureString
        proxy // optional custom httpHandler, pass nullptr for default
        );
    }

// call StartApplication to start the licensing heartbeats
void StartLicensing()
    {
    if (nullptr != m_licensingClient && !m_licensingActive)
        {
        // you may wish to add logic to handle StartApplication not returning Ok
        if( m_licensingClient->StartApplication() == Licensing::LicenseStatus::Ok)
            {
            m_licensingActive = true;
            }
        }
    }

// call StopApplication to stop licensing heartbeats
void StopLicensing()
    {
    if (nullptr != m_licensingClient && m_licensingActive)
        {
        // you may wish to add logic to handle StopApplication failing
        if( m_connectLicensingClient->StopApplication() == BentleyStatus::SUCCESS)
            {
            m_connectLicensingActive = false;
            }
        }
    }

// call MarkFeature to post a feature log in real time
void MarkFeature()
    {
    if (nullptr != m_licensingClient)
        {
        Utf8String featureId = "TestFeatureId";
        FeatureUserDataMapPtr featureData = std::make_shared<FeatureUserDataMap>();
        featureData->AddAttribute("Manufacturer", "Bentley Systems, Inc.");

        // you may wish to add logic to handle StartApplication not returning Success
        m_licensingClient->MarkFeature(featureId, featureData);
        }
    }
