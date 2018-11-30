/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/IntegrationTestsBase.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"
#include "RequestBehaviorOptions.h"
#include <WebServices/iModelHub/Client/ClientHelper.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <FakeServer/MockIMHubHttpHandler.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

#define DEFAULT_LANGUAGE_CODE "en"

BeFileName IntegrationTestsBase::s_seed;
DgnCategoryId IntegrationTestsBase::s_defaultCategoryId;
ClientPtr IntegrationTestsBase::s_client;
Utf8String IntegrationTestsBase::s_projectId;

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
::testing::TestInfo const& GetTestInfo()
    {
    return *::testing::UnitTest::GetInstance()->current_test_info();
    }

void InitLogging(BeFileNameCR assets)
    {
    BeFileName configFile(assets);
    configFile.AppendToPath(L"iModelHubIntegrationTests.logging.config.xml");

    NativeLogging::LoggingConfig::DeactivateProvider();
    NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, configFile);
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
    }

BeSQLite::L10N::SqlangFiles GetSqlangFiles(BeFileNameCR assets)
    {
    BeFileName sdkSqlangPath(assets);
    sdkSqlangPath.AppendToPath(L"sqlang");
    WPrintfString sdkLangFileName(L"DgnClientFx_%ls.sqlang.db3", WString(DEFAULT_LANGUAGE_CODE, true).c_str());
    sdkSqlangPath.AppendToPath(sdkLangFileName.c_str());
    BeSQLite::L10N::SqlangFiles sdkSqlangFiles(sdkSqlangPath, BeFileName(nullptr), BeFileName(nullptr));
    return sdkSqlangFiles;
    }

void InitializeTests()
    {
    static bool s_initialized = false;
    if (s_initialized)
        {
        Http::HttpClient::Reinitialize();
        return;
        }


    iModelHubHost& host = iModelHubHost::Instance();
    BeFileName temp = host.GetTempDirectory();
    BeFileName assets = host.GetDgnPlatformAssetsDirectory();
    InitLogging(assets);
    BeSQLite::BeSQLiteLib::Initialize(temp);
    BeSQLite::EC::ECDb::Initialize(temp, &assets);
    Http::HttpClient::Initialize(assets);
    UrlProvider::Initialize(IntegrationTestsSettings::ReadEnvironment(), UrlProvider::DefaultTimeout, StubLocalState::Instance());
    //auto mockHandler = std::make_shared<MockIMSHttpHandler>();
    //UrlProvider::SetHttpHandler(mockHandler);
    //ClientHelper::Initialize(IntegrationTestsSettings::Instance().GetClientInfo(), StubLocalState::Instance(), mockHandler);
    ClientHelper::Initialize(IntegrationTestsSettings::Instance().GetClientInfo(), StubLocalState::Instance(), ProxyHttpHandler::GetFiddlerProxyIfReachable());
    ClientHelper::GetInstance()->SetUrl(IntegrationTestsSettings::Instance().GetServerUrl());

    s_initialized = true;
    }

void IntegrationTestsBase::CreateSeedDb()
    {
    BeFileName seedFileName = iModelHubHost::Instance().BuildDbFileName("Test_Seed");
    if (seedFileName.DoesPathExist())
        {
        s_seed = seedFileName;
        return;
        }
    DgnDbPtr seedDb = iModelHubHost::Instance().CreateTestDb("Test_Seed");
    ASSERT_TRUE(seedDb.IsValid());
    s_seed = seedDb->GetFileName();
    s_defaultCategoryId = CreateCategory("DefaultCategory", *seedDb);
    BeSQLite::DbResult result = seedDb->SaveChanges();
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, result);
    seedDb->CloseDb();
    }

void IntegrationTestsBase::SetUpTestCase(RequestBehaviorOptions behaviourOptions)
    {
    InitializeTests();
    CreateSeedDb();
    iModelHubHelpers::CreateClient(s_client, IntegrationTestsSettings::Instance().GetValidAdminCredentials());
    iModelHubHost::Instance().SetRepositoryAdmin(s_client->GetiModelAdmin());
    s_projectId = IntegrationTestsSettings::Instance().GetProjectId();

    bmap<Utf8String, Utf8String> requestOptions = bmap<Utf8String, Utf8String>();
    requestOptions.insert(behaviourOptions.GetBehaviorOptionsResultPair());
    s_client->GlobalRequestOptions()->SetRequestOptions(requestOptions);
    }

void IntegrationTestsBase::TearDownTestCase()
    {
    Http::HttpClient::Uninitialize();
    }

void IntegrationTestsBase::SetUp()
    {
    ASSERT_TRUE(s_client.IsValid());
    bmap<Utf8String, Utf8String> requestOptions = bmap<Utf8String, Utf8String>();
    requestOptions.insert(RequestBehaviorOptions().GetBehaviorOptionsResultPair());
    s_client->GlobalRequestOptions()->SetRequestOptions(requestOptions);
    }

void IntegrationTestsBase::TearDown()
    {
    if (!Utf8String::IsNullOrEmpty(m_imodelName.c_str()))
        {
        iModelHubHelpers::DeleteiModelByName(s_client, m_imodelName);
        m_imodelName = "";
        }
    }

void CopyAndOpen(DgnDbPtr& db, BeFileName fileLocation, Utf8StringCR dbName)
    {
    BeFileNameStatus copyStatus = BeFileName::BeCopyFile(fileLocation, iModelHubHost::Instance().BuildDbFileName(dbName));
    EXPECT_EQ(BeFileNameStatus::Success, copyStatus);
    db = DgnDb::OpenDgnDb(nullptr, iModelHubHost::Instance().BuildDbFileName(dbName), DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    ASSERT_TRUE(db.IsValid());
    }

DgnDbPtr IntegrationTestsBase::CreateTestDb(Utf8StringCR dbName)
    {
    BeSQLite::BeGuid dbGuid;
    dbGuid.Create();

    DgnDbPtr db;
    CopyAndOpen(db, s_seed, dbName);

    // Create model and insert view
    PhysicalModelPtr model = CreateModel("DefaultModel", *db);
    InsertSpatialView(*model, "DefaultView");

    db->ChangeDbGuid(dbGuid);
    BeSQLite::DbResult result = db->SavePropertyString(PropertySpec("Name", "dgn_Proj"), dbName.c_str());
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, result);
    result = db->SavePropertyString(PropertySpec("Description", "dgn_Proj"), (dbName + " is created by iModelHubHost").c_str());
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, result);
    result = db->SaveChanges();
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, result);

    return db;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
iModelResult IntegrationTestsBase::CreateiModel(DgnDbPtr db, bool expectSuccess)
    {
    /*auto mockHandler = std::make_shared<MockIMSHttpHandler>();
    s_client->SetHttpHandler(mockHandler);*/
    return iModelHubHelpers::CreateNewiModel(s_client, db, s_projectId, expectSuccess);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
iModelConnectionPtr IntegrationTestsBase::CreateiModelConnection(iModelInfoPtr info)
    {
    auto connectionResult = s_client->ConnectToiModel(*info)->GetResult();
    EXPECT_SUCCESS(connectionResult);
    return connectionResult.GetValue();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr IntegrationTestsBase::CreateTestDb()
    {
    return CreateTestDb(GetTestiModelName());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IntegrationTestsBase::GetTestiModelName()
    {
    return Utf8PrintfString("%s-%s", GetTestInfo().test_case_name(), GetTestInfo().name());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName IntegrationTestsBase::OutputDir()
    {
    iModelHubHost& host = iModelHubHost::Instance();
    return host.GetOutputDirectory();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IntegrationTestsBase::TestCodeName(int number) const
    {
    return Utf8PrintfString("%s%d", GetTestInfo().name(), number);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr IntegrationTestsBase::CreateNonAdminClient() const
    {
    ClientPtr client;
    iModelHubHelpers::CreateClient(client, IntegrationTestsSettings::Instance().GetValidNonAdminCredentials());
    return client;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IRepositoryManagerP IntegrationTestsBase::_GetRepositoryManager(DgnDbR db)
    {
    return s_client->GetiModelAdmin()->_GetRepositoryManager(db);
    }
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
