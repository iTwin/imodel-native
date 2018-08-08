/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/IMHSClientExe/IMHSClientExe.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IMHSClientExe.h"
#include <WebServices/iModelHub/Client/Client.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeThread.h>
#include "../Helpers/IntegrationTestsSettings.h"
#include "../Helpers/DgnPlatformHelpers.h"
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/iModelHub/Client/ClientHelper.h>
#include <BeHttp/ProxyHttpHandler.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_TASKS
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Algirdas.Mikoliunas                12/16
+---------------+---------------+---------------+---------------+---------------+------*/
IMHSClientExe::IMHSClientExe()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Algirdas.Mikoliunas                12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IMHSClientExe::Initialize(Utf8String exePath)
    {
    m_exePath = exePath;
    DgnPlatformLib::Initialize(*this, true);

    WebServices::ClientInfoPtr clientInfo = IntegrationTestsSettings::Instance().GetClientInfo();

    auto clientHelper = ClientHelper::Initialize(clientInfo);
    clientHelper->SetUrl(IntegrationTestsSettings::Instance().GetServerUrl());

    auto manager = ConnectSignInManager::Create(clientInfo, ProxyHttpHandler::GetFiddlerProxyIfReachable());
    SignInResult signInResult = manager->SignInWithCredentials(IntegrationTestsSettings::Instance().GetValidAdminCredentials())->GetResult();
    if (!signInResult.IsSuccess())
        return BSIERROR;

    ClientPtr client = clientHelper->SignInWithManager(manager, IntegrationTestsSettings::Instance().GetEnvironment());

    WString logFileName = L"iModelHubIntgerationTests.log";
    WString path = _wgetenv(L"LOCALAPPDATA") + WString(L"\\Bentley\\LogsThread\\");
    WString fullPath = path + logFileName;

    BeFileName::CreateNewDirectory(path.c_str());

    NativeLogging::LoggingConfig::DeactivateProvider();
    NativeLogging::LoggingConfig::SetOption(L"OUTPUT_FILE", fullPath.c_str());
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::SIMPLEFILE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_IMODELHUB, NativeLogging::SEVERITY::LOG_INFO);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_HTTP, NativeLogging::LOG_INFO);

    return BSISUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Algirdas.Mikoliunas                12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BriefcasePtr IMHSClientExe::AcquireBriefcase(iModelConnectionPtr connection, Utf8String guid)
    {
    BeFileName briefcaseLocation(GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName());
    briefcaseLocation.AppendToPath(BeFileName(guid));
    briefcaseLocation.AppendSeparator();
    BeFileName::CreateNewDirectory(briefcaseLocation.c_str());
    auto acquireResult = m_client->AcquireBriefcase(connection->GetiModelInfo(), briefcaseLocation, false)->GetResult();
    if (!acquireResult.IsSuccess())
        return nullptr;

    BeFileName dbPath = acquireResult.GetValue()->GetLocalPath();
    if (!dbPath.DoesPathExist())
        return nullptr;
    DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, dbPath, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    if (!db.IsValid())
        return nullptr;

    auto briefcaseResult = m_client->OpenBriefcase(db, false)->GetResult();
    if (!briefcaseResult.IsSuccess())
        return nullptr;

    return briefcaseResult.GetValue();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Algirdas.Mikoliunas                12/16
+---------------+---------------+---------------+---------------+---------------+------*/
int IMHSClientExe::CreateNewModelAndPush(Utf8String projectId, Utf8String imodelId)
    {
    // Connect to imodel
    auto connectionResult = m_client->ConnectToiModel(*m_client->GetiModelById(projectId, imodelId)->GetResult().GetValue())->GetResult();
    if (!connectionResult.IsSuccess())
        return BSIERROR;
    iModelConnectionPtr connection = connectionResult.GetValue();
    m_repositoryAdmin = m_client->GetiModelAdmin();

    // Acquire briefcase
    Utf8String guid = BeSQLite::BeGuid(true).ToString();
    auto briefcase = AcquireBriefcase(connection, guid);
    if (briefcase.IsNull())
        return 1;
    DgnDbR briefcaseDb = briefcase->GetDgnDb();

    // Create model in the briefcase
    Utf8String name;
    name.Sprintf("TestModeltpt%s", guid);
    CreateModel(name.c_str(), briefcaseDb);
    BeSQLite::DbResult saveResult = briefcaseDb.SaveChanges();
    if(BE_SQLITE_OK != saveResult)
        return 2;

    // Sleep before trying to push
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    // Push!
    StopWatch timer(true);
    auto result = briefcase->PullMergeAndPush(nullptr, false, nullptr, nullptr, nullptr, 100)->GetResult();
    timer.Stop();
    std::cout << "Client finished in: " << timer.GetElapsedSeconds() << "\n";

    if (!result.IsSuccess())
        {
        std::cout << "Push failed with error: " << result.GetError().GetMessage() << "\n\n";
        return 3;
        }

    briefcase->GetDgnDb().CloseDb();
    return 0;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas              12/16
* This exe is used in the integration test PerformanceTests.PullMergeAndPush_PerformanceTests
* This test acquires briefcase from imodel, adds one model, waits 5 seconds and pushes changes to the server
* This exe could be run in following format: IMHSClientExe.exe {ProjectId} {iModelId}
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, wchar_t const* argv[])
    {
    IMHSClientExe imhsClient;
    Utf8String exePath (argv[0]);
    if (BSISUCCESS != imhsClient.Initialize(exePath))
        return 1;

    int result = imhsClient.CreateNewModelAndPush(Utf8String(argv[1]), Utf8String(argv[2]));
    if (0 != result)
        std::cout << "Thread failed: " << result << "\n\n";

    return result;
    }
