
#include "demo.h"

#include <iostream>
#include <sstream>

#include <Bentley/Desktop/FileSystem.h>
#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeHttp/HttpProxy.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

using namespace std;


const string commandList = "\n\
    - client [arg] - client commands, type client with no arguments, or \"client help\" to get a list of valid arguments\n\
    - help - get an explanation of this application and a list of commands\n\
    - log [arg] - set the logging level\n\
    - exit, quit - quit the program\n\
\n";

const string clientArgList = "\n\
    - client status - get the current status of the client (e.g. flavor of client, running, stopped, client not created)\n\
    - client create [arg] - create a client with the flavor of client as the argument. Call with no arguments to get a list of valid client flavors\n\
    - client start - for Client and AccessKeyClient, starts the licensing heartbeats. No effect for SaasClient.\n\
    - client stop - for Client and AccessKeyClient, stops the licensing heartbeats. No effect for SaasClient.\n\
    - client trackusage - for SaasClient, posts usage for the given product ID. No effect for Client and AccessKeyClient.\n\
    - client markfeature - for all client flavors, posts a feature log in realtime for the given feature.\n\
    - client licstatus - for AccessKeyClient get the license status of the current policy. No effect for Client and SaasClient.\n\
    - client help - get an explanation of the client and a list of valid client arguments.\n\
\n";

const string createArgList = "\n\
    - client create client - Create the Client. You will be prompted to provide all required information.\n\
    - client create accesskeyclient - Create the AccessKeyClient. You will be prompted to provide all required information.\n\
    - client create saasclient - Create the SaasClient. You will be prompted to provide all required information.\n\
\n";

const string logArgList = "\n\
    - log info\n\
    - log debug\n\
    - log error\n\
\n";

int main()
    {
    m_client = nullptr;
    m_accessKeyClient = nullptr;
    m_saasClient = nullptr;

    m_isClientCreated = false;
    m_heartbeatRunning = false;

    m_localState = nullptr;

    cout << "Type \"exit\" or \"(q)uit\" to exit the program.\n";

    Initialize();

    string inputString = "";
    while (true)
        {
        cout << ": ";
        getline(cin, inputString);
        //cout << "inputString: " << inputString << endl;
        vector<string> input = ParseInput(inputString);

        if(input.size() < 1)
            {
            cout << "Please enter one of the following valid commands:\n" + commandList;
            // list valid commands
            continue;
            }

        if(input.at(0) == "client" || input.at(0) == "c")
            {
            ProcessClientCommand(input);
            continue;            
            }
        else if(input.at(0) == "help" || input.at(0) == "h")
            {
            cout << "This command line application is intended to provide an interface for demonstrating and testing the Cross Platform License Client.\n\
Start by creating a client and then recording usage for the product ID you provided.\n\
The following is a full list of commands (all commands case insensitive):" + commandList;
            continue;
            }
        else if(input.at(0) == "log" || input.at(0) == "l")
            {
            if(input.size() < 2)
                {
                cout << "Enter the log level you desire:" + logArgList;
                continue;
                }
            else if(input.at(1) == "error" || input.at(1) == "e")
                {
                NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_ERROR);
                cout << "Set logging level to Error.\n";
                continue;
                }
            else if(input.at(1) == "info" || input.at(1) == "i")
                {
                NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_INFO);
                cout << "Set logging level to Info.\n";
                continue;
                }
            else if(input.at(1) == "debug" || input.at(1) == "d")
                {
                NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_DEBUG);
                cout << "Set logging level to Debug.\n";
                continue;
                }
            else if(input.at(1) == "trace" || input.at(1) == "t")
                {
                NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_TRACE);
                cout << "Set logging level to Trace.\n";
                continue;
                }
            else if(input.at(1) == "all" || input.at(1) == "a")
                {
                NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_ERROR);
                NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_DEBUG);
                NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_INFO);
                NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_TRACE);
                cout << "Set logging level to All.\n";
                continue;
                }
            else
                {
                cout << "Invalid input, please enter a valid argument for \"log\".\n";
                continue;
                }
            }
        else if(input.at(0) == "exit" || input.at(0) == "quit" || input.at(0) == "q")
            {
            if(m_client != nullptr && m_heartbeatRunning)
                {
                if(BentleyStatus::SUCCESS == m_client->StopApplication())
                    {
                    cout << "Licensing heartbeats stopped.\n";
                    m_heartbeatRunning = false;
                    }
                else
                    {
                    cout << "Failed to stop Licensing heartbeats. Exiting anyway.\n";
                    }
                }
            else if(m_accessKeyClient != nullptr && m_heartbeatRunning)
                {
                if(BentleyStatus::SUCCESS == m_accessKeyClient->StopApplication())
                    {
                    cout << "Licensing heartbeats stopped.\n";
                    m_heartbeatRunning = false;
                    m_accessKeyClient.reset();
                    }
                else
                    {
                    cout << "Failed to stop Licensing heartbeats. Exiting anyway.\n";
                    }
                }

            Http::HttpClient::Uninitialize();
            WebServices::UrlProvider::Uninitialize();
            BeSQLite::L10N::Shutdown();
            delete m_localState;
            break;
            }
        else
            {
            cout << "Invalid command. Please enter one of the following valid commands:\n" + commandList;
            continue;
            }
        }


    cout << "Exiting program\n";
    return 0;
    }

vector<string> ParseInput(string input)
    {
    // turn the cli input in to a vector of commands

    // make case not matter
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);

    vector<string> result;
    std::istringstream iss(input);
    for(std::string x; iss >> x;)
        {
        result.push_back(x);
        }

    return result;
    }

void Initialize()
    {
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_ERROR);

    BeFileName assetsDir(BeFileName::FileNameParts::Directory, L"/home/jason/Desktop/LicensingDemo/src/assets/");
    Http::HttpClient::Initialize(assetsDir);

    BeFileName tempDir;
    Desktop::FileSystem::BeGetTempPath(tempDir);
    if(BeSQLite::DbResult::BE_SQLITE_OK != BeSQLite::BeSQLiteLib::Initialize(tempDir))
        {
        cout << "BeSQLiteLib initialization failed. Try again.\n";
        return;
        }

    BeFileName path(assetsDir);
    path.AppendToPath(L"sqlang/DgnClientFx_en.sqlang.db3");
    if(BentleyStatus::SUCCESS != BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(path)))
        {
        cout << "L10N initialization failed. Try again.\n";
        return;
        }

    m_localState = new RuntimeJsonLocalState();
    WebServices::UrlProvider::Initialize(WebServices::UrlProvider::Environment::Qa, WebServices::UrlProvider::DefaultTimeout, m_localState);
    }

void ProcessClientCommand(vector<string> input)
    {
    if(input.size() < 2)
        {
        cout << "Please enter one of the following valid client arguments: \n" + clientArgList;
        return;
        }
    // client status
    else if(input.at(1) == "status")
        {
        if(m_client != nullptr)
            {
            cout << "License Client type: Client\n";
            if(m_heartbeatRunning)
                {
                cout << "Client heartbeat status: Running\n";
                }
            else
                {
                cout << "Client heartbeat status: Stopped\n";
                }
            }
        else if(m_accessKeyClient != nullptr)
            {
            cout << "License Client type: AccessKeyClient\n";
            if(m_heartbeatRunning)
                {
                cout << "Client heartbeat status: Running\n";
                }
            else
                {
                cout << "Client heartbeat status: Stopped\n";
                }
            }
        else if(m_saasClient != nullptr)
            {
            cout << "License Client type: SaasClient\n";
            }
        else
            {
            cout << "License Client: not created\n";
            }

        return;
        }

    // client create
    else if(input.at(1) == "create" || input.at(1) == "c")
        {
        if(input.size() < 3)
            {
            cout << "The following are the types of client you may create. You will be prompted to enter all required information." + createArgList;
            return;
            }

        if(m_client != nullptr)
            {
            cout << "Create failed. You have already created a Client.\n";
            return;
            }
        if(m_accessKeyClient != nullptr)
            {
            cout << "Create failed. You have already created an AccessKeyClient.\n";
            return;
            }
        if(m_saasClient != nullptr)
            {
            cout << "Create failed. You have already created a SaasClient.\n";
            return;
            }

        // create the client
        if(input.at(2) == "client" || input.at(2) == "c")
            {
            CreateClient();
            return;
            }
        else if(input.at(2) == "accesskeyclient" || input.at(2) == "accesskey" || input.at(2) == "a")
            {
            CreateAccessKeyClient();
            return;
            }
        else if(input.at(2) == "saasclient" || input.at(2) == "saas" || input.at(2) == "s")
            {
            CreateSaasClient();
            return;
            }
        else
            {
            cout << "Invalid client type. Please enter one of the valid client type:" + createArgList;
            return;
            }

        return;
        }
    
    // client start
    else if(input.at(1) == "start")
        {
        // TODO: move to a function
        if(m_client != nullptr)
            {
            auto startResult = m_client->StartApplication();
            cout << (int)startResult << endl;

            if(Licensing::LicenseStatus::Ok == startResult)
                {
                cout << "Started the Licensing heartbeats.\n";
                m_heartbeatRunning = true;
                return;
                }
            else if(Licensing::LicenseStatus::Offline == startResult)
                {
                cout << "Started the Licensing heartbeats. You are in offline mode.\n";
                m_heartbeatRunning = true;
                return;
                }
            else if(Licensing::LicenseStatus::Trial == startResult)
                {
                cout << "Started the Licensing heartbeats. You are in trial mode.\n";
                m_heartbeatRunning = true;
                return;
                }
            else if(Licensing::LicenseStatus::Expired == startResult)
                {
                cout << "Failed to start the Licensing heartbeats. License is expired.\n";
                m_heartbeatRunning = false;
                return;
                }
            else if(Licensing::LicenseStatus::AccessDenied == startResult)
                {
                cout << "Failed to start the Licensing heartbeats. Access denied.\n";
                m_heartbeatRunning = false;
                return;
                }
            else if(Licensing::LicenseStatus::DisabledByLogSend == startResult)
                {
                cout << "Failed to start the Licensing heartbeats. Disabled by log send.\n";
                m_heartbeatRunning = false;
                return;
                }
            else if(Licensing::LicenseStatus::DisabledByPolicy == startResult)
                {
                cout << "Failed to start the Licensing heartbeats. Disabled by policy.\n";
                m_heartbeatRunning = false;
                return;
                }
            else if(Licensing::LicenseStatus::NotEntitled == startResult)
                {
                cout << "Failed to start the Licensing heartbeats. Not entitled.\n";
                m_heartbeatRunning = false;
                return;
                }
            else
                {
                cout << "StartApplication failed.\n";
                m_heartbeatRunning = false;
                return;
                }
            }
        else if(m_accessKeyClient != nullptr)
            {
            auto startResult = m_accessKeyClient->StartApplication();
            cout << (int)startResult << endl;

            if(Licensing::LicenseStatus::Ok == startResult)
                {
                cout << "Started the Licensing policy heartbeat.\n";
                m_heartbeatRunning = true;
                return;
                }
            else if(Licensing::LicenseStatus::Offline == startResult)
                {
                cout << "Started the Licensing policy heartbeat. You are in offline mode.\n";
                m_heartbeatRunning = true;
                return;
                }
            else if(Licensing::LicenseStatus::Trial == startResult)
                {
                cout << "Started the Licensing policy heartbeat. You are in trial mode.\n";
                m_heartbeatRunning = true;
                return;
                }
            else if(Licensing::LicenseStatus::Expired == startResult)
                {
                cout << "Failed to start the Licensing policy heartbeat. License is expired.\n";
                m_heartbeatRunning = false;
                return;
                }
            else if(Licensing::LicenseStatus::AccessDenied == startResult)
                {
                cout << "Failed to start the Licensing policy heartbeat. Access denied.\n";
                m_heartbeatRunning = false;
                return;
                }
            else if(Licensing::LicenseStatus::DisabledByLogSend == startResult)
                {
                cout << "Failed to start the Licensing policy heartbeat. Disabled by log send.\n";
                m_heartbeatRunning = false;
                return;
                }
            else if(Licensing::LicenseStatus::DisabledByPolicy == startResult)
                {
                cout << "Failed to start the Licensing policy heartbeat. Disabled by policy.\n";
                m_heartbeatRunning = false;
                return;
                }
            else if(Licensing::LicenseStatus::NotEntitled == startResult)
                {
                cout << "Failed to start the Licensing policy heartbeat. Not entitled.\n";
                m_heartbeatRunning = false;
                return;
                }
            else
                {
                cout << "StartApplication failed.\n";
                m_heartbeatRunning = false;
                return;
                }
            }
        else if(m_saasClient != nullptr)
            {
            cout << "The SaasClient does not have Licensing heartbeats. Use client \"trackusage\" instead.\n";
            return;
            }
        else
            {
            cout << "The Licensing Client has not been created. You must create a client with \"client create [arg]\" to start Licensing heartbeats\n";
            return;
            }

        return;
        }
    
    // client stop
    else if(input.at(1) == "stop")
        {
        if(m_client != nullptr)
            {
            if(BentleyStatus::SUCCESS == m_client->StopApplication())
                {
                cout << "Licensing heartbeats stopped.\n";
                m_heartbeatRunning = false;
                return;
                }
            else
                {
                cout << "Failed to stop Licensing heartbeats. Please try again.\n";
                return;
                }
            }
        else if(m_accessKeyClient != nullptr)
            {
            if(BentleyStatus::SUCCESS == m_accessKeyClient->StopApplication())
                {
                cout << "Licensing heartbeats stopped.\n";
                m_heartbeatRunning = false;
                return;
                }
            else
                {
                cout << "Failed to stop Licensing heartbeats. Please try again.\n";
                return;
                }
            }
        else if(m_saasClient != nullptr)
            {
            cout << "SaasClient does not have Licensing heartbeats.\n";
            return;
            }
        else
            {
            cout << "The Licensing Client has not been created. You must create a client with \"client create [arg]\" to start Licensing heartbeats\n";
            return;
            }

        return;
        }
    
    // client trackusage
    // TODO: finish testing with OIDC token
    else if(input.at(1) == "trackusage" || input.at(1) == "t")
        {
        if(m_saasClient != nullptr)
            {
            TrackUsage();
            return;
            }
        else if(m_client != nullptr || m_accessKeyClient != nullptr)
            {
            cout << "Only the SaasClient has the TrackUsage function, use client start with the other client to record usage.\n";
            return;
            }

        return;
        }
    
    // client markfeature
    else if(input.at(1) == "markfeature" || input.at(1) == "m")
        {
        if(m_client != nullptr)
            {
            MarkFeature();
            return;
            }
        else if(m_accessKeyClient != nullptr)
            {
            AccessKeyMarkFeature();
            return;
            }
        else if(m_saasClient != nullptr)
            {
            SaasMarkFeature();
            return;
            }
        else
            {
            cout << "The Licensing Client has not been created. You must create a client with \"client create [arg]\" to mark features\n";
            return;
            }

        return;
        }
    
    // client licstatus
    else if(input.at(1) == "licstatus" || input.at(1) == "l")
        {
        if(m_accessKeyClient != nullptr)
            {
            LicenseStatus();
            }
        else if (m_client != nullptr || m_saasClient != nullptr)
            {
            cout << "\"licstatus\" is only available for the AccessKeyClient.\n";
            return;
            }
        return;
        }
    
    // client help
    else if(input.at(1) == "help" || input.at(1) == "h")
        {
        cout << "The \"client\" command allows you to create and interact with the license client flavor of your choosing.\n\
    Please enter one of the following valid arguments:" + clientArgList;
        return;
        }
    else
        {
        cout << "Invalid client argument. Please enter one of the following valid arguments:\n" + clientArgList;
        return;
        }
    }

// initialize required libraries and information and create the client
void CreateClient()
    {
    // TODO: option for full info (including ClientInfo) or "test" info prompts

    // prompt for required values
    string productIdInput;
    string signInInput;
    string offlineModeInput;
    string projectIdInput;
    string featureStringInput;

    cout << "Enter a product ID: ";
    getline(cin, productIdInput);
    cout << "Enter a projectId: ";
    getline(cin, projectIdInput);
    cout << "Enter a featureString: ";
    getline(cin, featureStringInput);

    // prompt for offlineMode
    bool offlineMode;
    while (true)
        {
        cout << "Offline mode? (true or false): ";
        getline(cin, offlineModeInput);
        std::transform(offlineModeInput.begin(), offlineModeInput.end(), offlineModeInput.begin(), ::tolower);
        if(offlineModeInput == "true" || offlineModeInput == "t")
            {
            offlineMode = true;
            break;
            }
        else if(offlineModeInput == "false" || offlineModeInput == "f")
            {
            offlineMode = false;
            break;
            }

        cout << "Invalid input, please enter \"true\" or \"false\"\n";
        }

    Utf8String productId = Utf8String(productIdInput.c_str());
    Utf8String projectId = Utf8String(projectIdInput.c_str());
    Utf8String featureString = Utf8String(featureStringInput.c_str());

    // TODO: option to prompt for ClientInfo values, or default to "test" values
    auto clientInfo = std::make_shared<WebServices::ClientInfo>("Bentley-Test", BeVersion(1,0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);
    if(clientInfo == nullptr)
        {
        cout << "failed to create ClientInfo\n";
        return;
        }

    // prompt for sign in
    bool signIn;
    while (true)
        {
        cout << "Do you want to sign in? (yes or no): ";
        getline(cin, signInInput);
        std::transform(signInInput.begin(), signInInput.end(), signInInput.begin(), ::tolower);
        if(signInInput == "yes" || signInInput == "y")
            {
            signIn = true;
            break;
            }
        else if(signInInput == "no" || signInInput == "n")
            {
            signIn = false;
            break;
            }

        cout << "Invalid input, please enter \"yes\" or \"no\"\n";
        }

    auto manager = WebServices::ConnectSignInManager::Create(clientInfo, nullptr, m_localState);
    if(signIn)
        {
        // TODO: prompt for user/pass

        // NOTE: qa2_devuser2@mailinator.com only works for product ID 2223 (2545 on desktop)
        Http::Credentials credentials("qa2_devuser2@mailinator.com", "bentley");
        auto signInResult = manager->SignInWithCredentials(credentials)->GetResult();
        if(!signInResult.IsSuccess())
            {
            cout << "Error: " << signInResult.GetError().GetMessage() << ". " << signInResult.GetError().GetDescription() << endl;
            // TODO: re-prompt for user/password
            cout << "Failed to create Client, please try again.";
            return;
            }
        else 
            cout << "Signed in\n";
        }

    BeFileName dbPath;
    if(Desktop::FileSystem::BeGetTempPath(dbPath) != BeFileNameStatus::Success)
        {
        cout << "Failed to find temporary directory.";
        return;
        }

    dbPath.AppendToPath(L"License.db");

    m_client = Licensing::Client::Create
        (
        manager->GetUserInfo(),
        clientInfo,
        manager,
        dbPath,
        offlineMode,
        projectId,
        featureString,
        nullptr
        );
    cout << "after create\n";

    if(m_client == nullptr)
        {
        cout << "Failed to create Client, please try again.\n";
        return;
        }
    else
        {
        cout << "Client created successfully!\n";
        return;
        }
    }

// initialize required libraries and information and create the client
void CreateAccessKeyClient()
    {
    // TODO: option for full info or "test" info prompts? - client info values?

    // prompt for required values
    string productIdInput;
    string accessKeyInput;
    string offlineModeInput;
    string projectIdInput;
    string featureStringInput;

    cout << "Enter a product ID: ";
    getline(cin, productIdInput);
    cout << "Enter an AccessKey: ";
    getline(cin, accessKeyInput);
    cout << "Enter a projectId: ";
    getline(cin, projectIdInput);
    cout << "Enter a featureString: ";
    getline(cin, featureStringInput);

    bool offlineMode;
    while (true)
        {
        cout << "Offline mode? (true or false): ";
        getline(cin, offlineModeInput);
        std::transform(offlineModeInput.begin(), offlineModeInput.end(), offlineModeInput.begin(), ::tolower);
        if(offlineModeInput == "true" || offlineModeInput == "t")
            {
            offlineMode = true;
            break;
            }
        else if(offlineModeInput == "false" || offlineModeInput == "f")
            {
            offlineMode = false;
            break;
            }

        cout << "Invalid input, please enter \"true\" or \"false\"\n";
        }

    Utf8String productId = Utf8String(productIdInput.c_str());
    Utf8String accessKey = Utf8String(accessKeyInput.c_str());
    Utf8String projectId = Utf8String(projectIdInput.c_str());
    Utf8String featureString = Utf8String(featureStringInput.c_str());

    // create the rest of the required info
    auto clientInfo = std::make_shared<WebServices::ClientInfo>("Bentley-Test", BeVersion(1,0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    if(clientInfo == nullptr)
        {
        cout << "failed to create ClientInfo\n";
        return;
        }

    BeFileName dbPath;
    if(Desktop::FileSystem::BeGetTempPath(dbPath) != BeFileNameStatus::Success)
        {
        cout << "Failed to find temporary directory.";
        return;
        }

    dbPath.AppendToPath(L"License.db");

    m_accessKeyClient = Licensing::AccessKeyClient::Create
        (
        accessKey, // 3469AD8D095A53F3CBC9A905A8FF8926 -> valid with deviceId (aka machineName) = "TestDeviceId" and product ID 1000
        clientInfo,
        dbPath,
        offlineMode,
        projectId,
        featureString,
        nullptr
        );

    if(m_accessKeyClient == nullptr)
        {
        cout << "Failed to create AccessKeyClient, please try again.\n";
        return;
        }
    else
        {
        cout << "AccessKeyClient created successfully!\n";
        return;
        }
    }

// initialize required libraries and information and create the client
void CreateSaasClient()
    {
    // prompt for productId, featureString
    string productIdInput = "";
    int productId = 0;
    string featureString = "";

    cout << "Enter a product ID: ";
    getline(cin, productIdInput);
    productId = std::atoi(productIdInput.c_str());

    cout << "Enter a featureString: ";
    getline(cin, featureString);

    m_saasClient = Licensing::SaasClient::Create(productId, featureString.c_str(), nullptr);

    if(m_saasClient == nullptr)
        {
        cout << "Failed to create SaasClient, please try again.\n";
        return;
        }
    else
        {
        cout << "SaasClient created successfully!\n";
        return;
        }
    }

// initialize required info and call TrackUsage function
void TrackUsage()
    {
    string tokenInput;
    string versionInput;
    string projectIdInput;

    cout << "Enter an OIDC token: ";
    getline(cin, tokenInput);
    cout << "Enter the version of the product you are posting usage for (format: 1.2.3.4): ";
    getline(cin, versionInput);
    cout << "Enter a project ID: ";
    getline(cin, projectIdInput);

    BeVersion version(versionInput.c_str());
    Utf8String token = Utf8String(tokenInput.c_str());
    Utf8String projectId = projectIdInput == "" ? "00000000-0000-0000-0000-000000000000" : Utf8String(projectIdInput.c_str()); // TODO: move this logic to the license client

    auto resultStatus = m_saasClient->TrackUsage(token, version, projectId).get();
    if(resultStatus != BentleyStatus::SUCCESS)
        {
        cout << "Failed to post usage.\n";
        return;
        }
    else
        {   
        cout << "Posted usage successfully!\n";
        return;
        }
    }

// initialize required info and call MarkFeature
void MarkFeature()
    {
    string token;
    string featureId;
    string version;
    string projectId;
    string attributeKey;
    string attributeValue;

    cout << "FOR TESTING PURPOSES: enter an OIDC token: ";
    getline(cin, token);
    cout << "Enter the Feature ID (must be a guid): ";
    getline(cin, featureId);
    cout << "Enter the version of the product (format: 1.2.3.4): ";
    getline(cin, version);
    //TODO: make beversion here, reprompt if not successfull
    cout << "Enter a project ID: ";
    getline(cin, projectId);

    Licensing::FeatureUserDataMapPtr featureData = std::make_shared<Licensing::FeatureUserDataMap>();

    string another = "y";
    while(another == "yes" || another == "y")
        {
        cout << "Enter a feature attribute key: ";
        getline(cin, attributeKey);
        cout << "Enter a feature attribute value: ";
        getline(cin, attributeValue);

        if(BentleyStatus::SUCCESS == featureData->AddAttribute(Utf8String(attributeKey.c_str()), Utf8String(attributeValue.c_str())))
            {
            cout << "Attribute added successfully.\n";
            }
        else
            {
            cout << "Failed to add attribute.\n";
            }

        // prompt for continue adding
        while (true)
            {
            cout << "Would you like to add another attribute? (\"yes\" or \"no\"): ";
            getline(cin, another);
            std::transform(another.begin(), another.end(), another.begin(), ::tolower);
            if(another == "yes" || another == "y" || another == "no" || another == "n")
                {
                break;
                }

            cout << "Invalid input, please enter \"yes\" or \"no\"\n";
            }
        }

    auto resultStatus = m_client->MarkFeature(Utf8String(featureId.c_str()), featureData);
    if(resultStatus != BentleyStatus::SUCCESS)
        {
        cout << "Failed to mark feature.\n";
        return;
        }
    else
        {   
        cout << "Marked feature successfully!\n";
        return;
        }
    }

// initialize required info and call MarkFeature
void AccessKeyMarkFeature()
    {
    string token;
    string featureId;
    string version;
    string projectId;
    string attributeKey;
    string attributeValue;

    cout << "Enter the Feature ID: ";
    getline(cin, featureId);
    cout << "Enter the version of the product (format: 1.2.3.4): ";
    getline(cin, version);
    //TODO: make beversion here, reprompt if not successfull
    cout << "Enter a project ID: ";
    getline(cin, projectId);

    Licensing::FeatureUserDataMapPtr featureData = std::make_shared<Licensing::FeatureUserDataMap>();

    string another = "y";
    while(another == "yes" || another == "y")
        {
        cout << "Enter a feature attribute key: ";
        getline(cin, attributeKey);
        cout << "Enter a feature attribute value: ";
        getline(cin, attributeValue);

        if(BentleyStatus::SUCCESS == featureData->AddAttribute(Utf8String(attributeKey.c_str()), Utf8String(attributeValue.c_str())))
            {
            cout << "Attribute added successfully.\n";
            }
        else
            {
            cout << "Failed to add attribute.\n";
            }

        // prompt for continue adding
        while (true)
            {
            cout << "Would you like to add another attribute? (\"yes\" or \"no\"): ";
            getline(cin, another);
            std::transform(another.begin(), another.end(), another.begin(), ::tolower);
            if(another == "yes" || another == "y" || another == "no" || another == "n")
                {
                break;
                }

            cout << "Invalid input, please enter \"yes\" or \"no\"\n";
            }
        }

    auto resultStatus = m_accessKeyClient->MarkFeature(Utf8String(featureId.c_str()), featureData);
    if(resultStatus != BentleyStatus::SUCCESS)
        {
        cout << "Failed to mark feature.\n";
        return;
        }
    else
        {   
        cout << "Marked feature successfully!\n";
        return;
        }
    }

// initialize required info and call MarkFeature
void SaasMarkFeature()
    {
    string token;
    string featureId;
    string version;
    string projectIdInput;
    string attributeKey;
    string attributeValue;

    cout << "Enter an OIDC token: ";
    getline(cin, token);
    cout << "Enter the Feature ID: ";
    getline(cin, featureId);
    cout << "Enter the version of the product: (format: 1.2.3.4): ";
    getline(cin, version);
    //TODO: make beversion here, reprompt if not successfull
    cout << "Enter a project ID: ";
    getline(cin, projectIdInput);

    Licensing::FeatureUserDataMapPtr featureData = std::make_shared<Licensing::FeatureUserDataMap>();

    string another = "y";
    while(another == "yes" || another == "y")
        {
        cout << "Enter a feature attribute key: ";
        getline(cin, attributeKey);
        cout << "Enter a feature attribute value: ";
        getline(cin, attributeValue);

        if(BentleyStatus::SUCCESS == featureData->AddAttribute(Utf8String(attributeKey.c_str()), Utf8String(attributeValue.c_str())))
            {
            cout << "Attribute added successfully.\n";
            }
        else
            {
            cout << "Failed to add attribute.\n";
            }

        // prompt for continue adding
        while (true)
            {
            cout << "Would you like to add another attribute? (\"yes\" or \"no\"): ";
            getline(cin, another);
            std::transform(another.begin(), another.end(), another.begin(), ::tolower);
            if(another == "yes" || another == "y" || another == "no" || another == "n")
                {
                break;
                }

            cout << "Invalid input, please enter \"yes\" or \"no\"\n";
            }
        }

    Utf8String projectId = projectIdInput == "" ? "00000000-0000-0000-0000-000000000000" : Utf8String(projectIdInput.c_str()); // TODO: move this logic to the license client

    Licensing::FeatureEvent featureEvent = Licensing::FeatureEvent(Utf8String(featureId.c_str()), BeVersion(version.c_str()), projectId, featureData);

    // use token from windows Integration Tests (OIDC native)

    auto resultStatus = m_saasClient->MarkFeature(Utf8String(token.c_str()), featureEvent).get();
    if(resultStatus != BentleyStatus::SUCCESS)
        {
        cout << "Failed to mark feature.\n";
        return;
        }
    else
        {   
        cout << "Marked feature successfully!\n";
        return;
        }
    }

// call GetLicenseStatus
void LicenseStatus()
    {
    auto licStatus = m_accessKeyClient->GetLicenseStatus();
    cout << (int)licStatus << endl;

    if(Licensing::LicenseStatus::Ok == licStatus)
        {
        cout << "Valid License.\n";
        return;
        }
    else if(Licensing::LicenseStatus::Offline == licStatus)
        {
        cout << "Valid License. You are in offline mode.\n";
        return;
        }
    else if(Licensing::LicenseStatus::Trial == licStatus)
        {
        cout << "Valid License. You are in trial mode.\n";
        return;
        }
    else if(Licensing::LicenseStatus::Expired == licStatus)
        {
        cout << "Invalid License. License is expired.\n";
        return;
        }
    else if(Licensing::LicenseStatus::AccessDenied == licStatus)
        {
        cout << "Invalid License. Access denied.\n";
        return;
        }
    else if(Licensing::LicenseStatus::DisabledByLogSend == licStatus)
        {
        cout << "Invalid License. Disabled by log send.\n";
        return;
        }
    else if(Licensing::LicenseStatus::DisabledByPolicy == licStatus)
        {
        cout << "Invalid License. Disabled by policy.\n";
        return;
        }
    else if(Licensing::LicenseStatus::NotEntitled == licStatus)
        {
        cout << "Invalid License. Not entitled.\n";
        return;
        }
    else
        {
        cout << "GetLicenseStatus failed.\n";
        return;
        }
    }