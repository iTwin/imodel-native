/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/IntegrationTestsBase.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsHelper.h"
#include "IntegrationTestsBase.h"
#include <WebServices/iModelHub/Client/Client.h>
#include <WebServices/iModelHub/Client/Configuration.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <Bentley/bmap.h>
#include <BeHttp/HttpClient.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
#define DEFAULT_LANGUAGE_CODE "en"
#define EXPECT_STATUS(STAT, EXPR) EXPECT_EQ(RepositoryStatus:: STAT, (EXPR))

L10N::SqlangFiles GetSqlangFiles(BeFileNameCR assets)
    {
    BeFileName sdkSqlangPath(assets);
    sdkSqlangPath.AppendToPath(L"sqlang");
    WPrintfString sdkLangFileName(L"DgnClientFx_%ls.sqlang.db3", WString(DEFAULT_LANGUAGE_CODE, true).c_str());
    sdkSqlangPath.AppendToPath(sdkLangFileName.c_str());
    L10N::SqlangFiles sdkSqlangFiles(sdkSqlangPath, BeFileName(nullptr), BeFileName(nullptr));
    return sdkSqlangFiles;
    }

void Initialize(ScopediModelHubHost* host)
    {
    static bool initialized = false;
    if (!initialized)
        {
        host->CleanOutputDirectory();
        BeFileName temp = host->GetTempDirectory();
        BeFileName assets = host->GetDgnPlatformAssetsDirectory();
        BeSQLiteLib::Initialize(temp);
        BeSQLite::EC::ECDb::Initialize(temp, &assets);
        Http::HttpClient::Initialize(assets);
        initialized = true;
        }
    }

void IntegrationTestsBase::SetUp()
    {
    InitLogging();
    m_pHost = new ScopediModelHubHost();
    Initialize(m_pHost);
    CreateInitialSeedDb();
    Configuration::SetPredownloadChangeSetsEnabled(false);
    }

void IntegrationTestsBase::TearDown()
    {
    if (m_pHost)
        delete m_pHost;
    }

void IntegrationTestsBase::SetUpTestCase()
    {
    }

void IntegrationTestsBase::TearDownTestCase()
    {
    }

void IntegrationTestsBase::InitLogging()
    {
    WString logFileName = L"iModelHubIntgerationTests.log";
    WString path        = _wgetenv(L"LOCALAPPDATA") + WString(L"\\Bentley\\Logs\\");
    WString fullPath    = path + logFileName;

    BeFileName::CreateNewDirectory(path.c_str());

    NativeLogging::LoggingConfig::DeactivateProvider();
    NativeLogging::LoggingConfig::SetOption(L"OUTPUT_FILE", fullPath.c_str());
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::SIMPLEFILE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_IMODELHUB, NativeLogging::SEVERITY::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_HTTP, NativeLogging::LOG_TRACE);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis            06/2016
//---------------------------------------------------------------------------------------
void IntegrationTestsBase::CreateInitialSeedDb()
    {
    BeFileName seedFileName = m_pHost->BuildDbFileName("Test_Seed");
    if (seedFileName.DoesPathExist())
        {
        m_seed = seedFileName;
        return;
        }
    DgnDbPtr seedDb = m_pHost->CreateTestDb("Test_Seed");
    EXPECT_TRUE(seedDb.IsValid());
    m_seed = seedDb->GetFileName();
    CreateCategory("DefaultCategory", *seedDb);
    BeSQLite::DbResult result = seedDb->SaveChanges();
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, result);
    seedDb->CloseDb();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis            06/2016
//---------------------------------------------------------------------------------------
DgnDbPtr IntegrationTestsBase::CreateTestDb(Utf8StringCR baseName)
    {
    BeSQLite::BeGuid dbGuid;
    dbGuid.Create();
    Utf8String dbName;
    dbName.Sprintf("%s_%s", 0 != baseName.size() ? baseName : "Test", dbGuid.ToString());

    BeFileNameStatus copyStatus = BeFileName::BeCopyFile(m_seed, LocalPath(dbName));
    EXPECT_EQ(BeFileNameStatus::Success, copyStatus);
    DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, LocalPath(dbName), DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(db.IsValid());

    db->ChangeDbGuid(dbGuid);
    BeSQLite::DbResult result = db->SavePropertyString(PropertySpec("Name", "dgn_Proj"), dbName.c_str());
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, result);
    result = db->SavePropertyString(PropertySpec("Description", "dgn_Proj"), (dbName + " is created by ScopediModelHubHost").c_str());
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, result);
    result = db->SaveChanges();
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, result);

    return db;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis            06/2016
//---------------------------------------------------------------------------------------
void IntegrationTestsBase::DeleteiModel(ClientCR client, iModelInfoCR imodel)
    {
    auto result = client.DeleteiModel(imodel)->GetResult();
    EXPECT_SUCCESS(result);
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
BeSQLite::BeGuid IntegrationTestsBase::ReplaceSeedFile(iModelConnectionPtr imodelConnection, DgnDbR db, bool changeGuid)
    {
    if (changeGuid)
        {
        BeSQLite::BeGuid newGuid;
        newGuid.Create();
        db.ChangeDbGuid(newGuid);
        db.SaveChanges();
        }

    auto fileName = db.GetFileName();
    ICancellationTokenPtr canc = SimpleCancellationToken::Create();
    FileInfoPtr fileInfo = FileInfo::Create(db, "Replacement description1");
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    EXPECT_SUCCESS(imodelConnection->UploadNewSeedFile(fileName, *fileInfo, true, nullptr, canc)->GetResult());

    return db.GetDbGuid();
    }


BeFileName IntegrationTestsBase::LocalPath(Utf8StringCR baseName)
    {
    return m_pHost->BuildDbFileName(baseName);
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubLocalState : public IJsonLocalState
    {
    private:
        bmap<Utf8String, Utf8String> m_map;

    public:
        static StubLocalState* Instance()
            {
            static StubLocalState* s_instance = nullptr;
            if (nullptr == s_instance)
                s_instance = new StubLocalState();
            return s_instance;
            }

        bmap<Utf8String, Utf8String> GetStubMap()
            {
            return m_map;
            }

        void _SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) override
            {
            Utf8PrintfString identifier("%s/%s", nameSpace, key);
            if (value == "null")
                {
                m_map.erase(identifier);
                }
            else
                {
                m_map[identifier] = value;
                }
            };

        Utf8String _GetValue(Utf8CP nameSpace, Utf8CP key) const override
            {
            Utf8PrintfString identifier("%s/%s", nameSpace, key);
            auto iterator = m_map.find(identifier);
            if (iterator == m_map.end())
                {
                return "";
                }
            else
                {
                return iterator->second;
                }
            };
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas           05/2017
//---------------------------------------------------------------------------------------
bool SignInWithRetry(ConnectSignInManagerPtr manager, Credentials credentials, int retryCount)
    {
    SignInResult signInResult;

    for (int i = 0; i <= retryCount; i++)
        {
        signInResult = manager->SignInWithCredentials(credentials)->GetResult();
        if (signInResult.IsSuccess())
            return true;
        }

    Utf8String errorMessage;
    errorMessage.Sprintf("IMS signIn failed: %s", signInResult.GetError().GetMessage().c_str());
    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_IMODELHUB)->error(errorMessage.c_str());
    return false;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis            07/2016
//---------------------------------------------------------------------------------------
Utf8String QueryProjectId(Utf8String projectNr, Credentials credentials)
    {
    UrlProvider::Initialize(IntegrationTestSettings::Instance().GetEnvironment(), UrlProvider::DefaultTimeout, StubLocalState::Instance());
    Utf8String imodelId = "BentleyCONNECT.Global--CONNECT.GLOBAL";

    WebServices::ClientInfoPtr clientInfo = IntegrationTestSettings::Instance().GetClientInfo();
    auto manager = ConnectSignInManager::Create(clientInfo, nullptr, StubLocalState::Instance());
    if (!SignInWithRetry(manager, credentials, 1))
        return nullptr;

    auto authHandler = manager->GetAuthenticationHandler(UrlProvider::Urls::ConnectWsgGlobal.Get());
    auto client = WSRepositoryClient::Create(UrlProvider::Urls::ConnectWsgGlobal.Get(), imodelId, clientInfo, nullptr, authHandler);

    WSQuery query("GlobalSchema", "Project");
    query.SetSelect("$id");
    query.SetFilter(Utf8PrintfString("Active+eq+true+and+Number+eq+'%s'", projectNr.c_str()));

    auto result = client->SendQueryRequest(query)->GetResult();
    if (!result.IsSuccess())
        return nullptr;

    JsonValueCR instance = result.GetValue().GetJsonValue()["instances"][0];
    return instance["instanceId"].asString();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
ClientPtr IntegrationTestsBase::SetUpClient (Utf8StringCR host, Credentials credentials, IHttpHandlerPtr customHandler)
    {
    WebServices::ClientInfoPtr clientInfo = IntegrationTestSettings::Instance().GetClientInfo();
    ClientPtr client = nullptr;

    if (IntegrationTestSettings::Instance().IsIms())
        {
        AuthenticationHandlerPtr authHandler = nullptr;
        Utf8String projectId;
        UrlProvider::Initialize(IntegrationTestSettings::Instance().GetEnvironment(), UrlProvider::DefaultTimeout, StubLocalState::Instance());
        auto manager = ConnectSignInManager::Create(clientInfo, customHandler, StubLocalState::Instance());
        
        if (SignInWithRetry(manager, credentials, 1))
            authHandler = manager->GetAuthenticationHandler(host);
        else
            return nullptr;

        projectId = QueryProjectId(IntegrationTestSettings::Instance().GetProjectNr(), credentials);
        client = Client::Create(clientInfo, authHandler);
        client->SetProject(projectId);
        }
    else
        {
        client = Client::Create(clientInfo, customHandler);
        }

    client->SetServerURL(host);
    client->SetCredentials(credentials);

    return client;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
iModelInfoPtr IntegrationTestsBase::CreateNewiModel (ClientCR client, Utf8StringCR imodelName)
    {
    //Create the seed file
    DgnDbPtr db = CreateTestDb(imodelName);
    EXPECT_TRUE(db.IsValid());

    //Upload the seed file to the server
    auto createResult = client.CreateNewiModel(*db)->GetResult();
    EXPECT_SUCCESS(createResult);
    if (!createResult.IsSuccess())
        throw createResult.GetError().GetId();
    iModelInfoPtr imodelInfo = createResult.GetValue();

    //Delete the seed file
    BeFileName dbPath = db->GetFileName();
    db->CloseDb();
    dbPath.BeDeleteFile();

    return imodelInfo;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
iModelInfoPtr IntegrationTestsBase::CreateNewiModelFromDb(ClientCR client, DgnDbR db)
    {
    //Upload the seed file to the server
    auto createResult = client.CreateNewiModel(db)->GetResult();
    EXPECT_SUCCESS(createResult);
    return createResult.GetValue();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
iModelConnectionPtr IntegrationTestsBase::ConnectToiModel(ClientCR client, iModelInfoPtr imodelInfo)
    {
    auto connectionResult = client.ConnectToiModel(imodelInfo->GetId())->GetResult();
    EXPECT_SUCCESS(connectionResult);
    auto connection = connectionResult.GetValue();
    return connection;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
BriefcasePtr IntegrationTestsBase::AcquireBriefcase (ClientCR client, iModelInfoCR imodelInfo, bool pull)
    {
    auto acquireResult = client.AcquireBriefcaseToDir (imodelInfo, m_pHost->GetOutputDirectory(), pull)->GetResult ();
    EXPECT_SUCCESS(acquireResult);

    BeFileName dbPath = acquireResult.GetValue ()->GetLocalPath();
    EXPECT_TRUE (dbPath.DoesPathExist ());

    DgnDbPtr db = DgnDb::OpenDgnDb (nullptr, dbPath, DgnDb::OpenParams (DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(db.IsValid());

    auto briefcaseResult = client.OpenBriefcase(db, false)->GetResult();
    EXPECT_SUCCESS(briefcaseResult);

    return briefcaseResult.GetValue();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis            06/2016
//---------------------------------------------------------------------------------------
void IntegrationTestsBase::InitializeWithChangeSets(ClientCR client, iModelInfoCR imodel, uint32_t changeSetCount)
    {
    BriefcasePtr briefcase = AcquireBriefcase(client, imodel);

    Utf8String modelName;
    BeSQLite::BeGuid guid;
    guid.Create();
    modelName.Sprintf("AddChangeSetsModel%s", guid.ToString());
    DgnDbR db = briefcase->GetDgnDb();
    DgnModelPtr model = CreateModel(modelName.c_str(), db);
    for (uint32_t i = 0; i < changeSetCount; ++i)
        {
        CreateElement(*model, false);
        BeSQLite::DbResult saveResult = db.SaveChanges();
        EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, saveResult);
        auto result = briefcase->PullMergeAndPush()->GetResult();
        EXPECT_SUCCESS(result);
        }
    db.BriefcaseManager().Relinquish();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas            08/2016
//---------------------------------------------------------------------------------------
void IntegrationTestsBase::CreateNonAdminUser()
    {
    if (IntegrationTestSettings::Instance().IsIms())
        return;

    auto client = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidAdminCredentials());
    client->CreateBasicUser(IntegrationTestSettings::Instance().GetValidNonAdminCredentials())->GetResult();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas            08/2016
//---------------------------------------------------------------------------------------
void IntegrationTestsBase::RemoveNonAdminUser()
    {
    if (IntegrationTestSettings::Instance().IsIms())
        return;

    auto client = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidAdminCredentials());
    auto result = client->RemoveBasicUser(IntegrationTestSettings::Instance().GetValidNonAdminCredentials())->GetResult();
    EXPECT_SUCCESS(result);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
Utf8String IntegrationTestsBase::PushPendingChanges(Briefcase& briefcase, bool relinquishLocksCodes)
    {
    auto pushResult = briefcase.PullMergeAndPush(nullptr, relinquishLocksCodes)->GetResult();
    EXPECT_SUCCESS(pushResult);
    return briefcase.GetLastChangeSetPulled();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
void IntegrationTestsBase::ExpectLocksCount(Briefcase& briefcase, int expectedCount)
    {
    auto result = briefcase.GetiModelConnection().QueryCodesLocks(briefcase.GetBriefcaseId())->GetResult();
    EXPECT_SUCCESS(result);
    auto actualCount = result.GetValue().GetLocks().size();
    EXPECT_EQ(expectedCount, actualCount);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void IntegrationTestsBase::ExpectCodesEqual(DgnCodeStateCR exp, DgnCodeStateCR act)
    {
    if (exp.IsAvailable())
        {
        EXPECT_TRUE(act.IsAvailable());
        }
    else if (exp.IsReserved())
        {
        EXPECT_TRUE(act.IsReserved());
        EXPECT_EQ(exp.GetReservedBy().GetValue(), act.GetReservedBy().GetValue());
        }
    else
        {
        EXPECT_EQ(exp.IsDiscarded(), act.IsDiscarded());
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void IntegrationTestsBase::ExpectCodesEqual(DgnCodeInfoCR exp, DgnCodeInfoCR act)
    {
    EXPECT_EQ(exp.GetCode(), act.GetCode());
    ExpectCodesEqual(static_cast<DgnCodeState>(exp), static_cast<DgnCodeState>(act));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void IntegrationTestsBase::ExpectCodesEqual(DgnCodeInfoSet const& expected, DgnCodeInfoSet const& actual)
    {
    EXPECT_EQ(expected.size(), actual.size());
    for (auto expIter = expected.begin(), actIter = actual.begin(); expIter != expected.end() && actIter != actual.end(); ++expIter, ++actIter)
        ExpectCodesEqual(*expIter, *actIter);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void IntegrationTestsBase::ExpectCodeState(DgnCodeInfoSet const& expect, IRepositoryManagerP imodelManager)
    {
    DgnCodeInfoSet actual;
    DgnCodeSet codes;
    for (auto const& info : expect)
        codes.insert(info.GetCode());

    EXPECT_STATUS(Success, imodelManager->QueryCodeStates(actual, codes));
    ExpectCodesEqual(expect, actual);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void IntegrationTestsBase::ExpectCodeState(DgnCodeInfoCR expect, IRepositoryManagerP imodelManager)
    {
    DgnCodeInfoSet expectInfos;
    expectInfos.insert(expect);
    ExpectCodeState(expectInfos, imodelManager);
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnCodeInfo IntegrationTestsBase::CreateCodeUsed(DgnCodeCR code, Utf8StringCR changeSetId)
    {
    DgnCodeInfo info(code);
    info.SetUsed(changeSetId);
    return info;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnCode IntegrationTestsBase::MakeModelCode(Utf8CP name, DgnDbR db)
    {
    return PhysicalPartition::CreateCode(*db.Elements().GetRootSubject(), name);
    }
