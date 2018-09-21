/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/RepositoryCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "RepositoryCompatibilityTests.h"

#include <WebServices/Cache/CachingDataSource.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeHttp/HttpConfigurationHandler.h>

#include "../UnitTests/Published/WebServices/Connect/StubLocalState.h"

#include "TestsHelper.h"
#include "TestsHost.h"
#include "Parser/ArgumentParser.h"
#include "Disk/DiskRepositoryClient.h"
#include "Logging/Logging.h"

bvector<TestRepositories> s_createTestData;
bvector<TestRepositories> s_upgradeTestData;
bvector<TestRepositories> s_downloadSchemasTestData;

StubLocalState s_localState;
IHttpHandlerPtr s_proxy;
ConnectSignInManagerPtr s_signInManager;

void RepositoryCompatibilityTests::SetTestData(const bvector<TestRepositories>& testData)
    {
    for (auto& repositories : testData)
        {
        if (repositories.upgrade.IsValid())
            {
            s_upgradeTestData.push_back(repositories);
            }
        else if (repositories.create.IsValid())
            {
            s_createTestData.push_back(repositories);
            }
        else if (repositories.downloadSchemas.IsValid())
            {
            s_downloadSchemasTestData.push_back(repositories);
            }
        }
    }

void RepositoryCompatibilityTests::SetUp()
    {
    TestsHost::GetErrorLog().clear();
    s_localState = StubLocalState();
    s_proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    UrlProvider::Uninitialize();

    s_signInManager = ConnectSignInManager::Create(StubValidClientInfo(), s_proxy, &s_localState);
    }

void RepositoryCompatibilityTests::TearDown()
    {
    UrlProvider::Uninitialize();

    if (!TestsHost::GetErrorLog().empty())
        ADD_FAILURE() << TestsHost::GetErrorLog();

    if (::testing::Test::HasFailure())
        {
        if (!GetParam().create.comment.empty())
            ADD_FAILURE() << "Known issue: " << GetParam().create.comment;

        if (!GetParam().upgrade.comment.empty())
            ADD_FAILURE() << "Known issue: " << GetParam().create.comment;

        if (!GetParam().downloadSchemas.comment.empty())
            ADD_FAILURE() << "Known issue: " << GetParam().create.comment;
        }
    }

void CreateDateStampFile(Utf8StringCR testName, BeFileName path)
    {
    Utf8String name = testName + "-" + Utf8String(DateTime::GetCurrentTimeUtc().ToString()) + ".stamp";
    name.ReplaceAll(":", ".");
    path.AppendToPath(BeFileName(name));
    BeFile file;
    EXPECT_EQ(BeFileStatus::Success, file.Create(path));
    file.Close();
    }

Utf8String GetDateStr(DateTimeCR dateTime)
    {
    return Utf8PrintfString("%04d-%02d-%02d", dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay());
    }

BeFileName GetOutputPath()
    {
    BeFileName path;
    BeTest::GetHost().GetOutputRoot(path);
    path.AppendToPath(BeFileName(BUILD_VERSION));
    return path;
    }

BeFileName GetOutputPath(Utf8StringCR testName, Utf8StringCR customLabel, Utf8String repositoryId, Utf8StringCR dateStr)
    {
    if (!customLabel.empty())
        repositoryId = customLabel;

    BeFileName path = GetOutputPath();
    path.AppendToPath(BeFileName(dateStr));
    path.AppendToPath(BeFileName(testName));
    path.AppendToPath(BeFileName(repositoryId));

    return path;
    }

BeFileName GetNewOutputPath(Utf8StringCR testName, Utf8StringCR customLabel, Utf8StringCR repositoryId)
    {
    auto todayStr = GetDateStr(DateTime::GetCurrentTimeUtc());
    auto path = GetOutputPath(testName, customLabel, repositoryId, todayStr);

    if (path.DoesPathExist())
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(path));

    EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(path));

    return path;
    }

IWSRepositoryClientPtr CreateClient(TestRepository& repository)
    {
    if (!repository.schemasDir.empty())
        return std::make_shared<DiskRepositoryClient>(repository.schemasDir);

    auto httpHandler = s_proxy;

    if (repository.validateCertificate == false)
        {
        httpHandler = std::make_shared<HttpConfigurationHandler>([] (Http::Request& request)
            {
            request.SetValidateCertificate(false);
            }, httpHandler);
        }

    if (repository.environment)
        {
        UrlProvider::Initialize(*repository.environment, UrlProvider::DefaultTimeout, &s_localState, nullptr, s_proxy);
        if (repository.credentials.IsValid())
            {
            EXPECT_TRUE(s_signInManager->SignInWithCredentials(repository.credentials)->GetResult().IsSuccess());
            }
        else if (repository.token)
            {
            EXPECT_TRUE(s_signInManager->SignInWithToken(repository.token)->GetResult().IsSuccess());
            }
        httpHandler = s_signInManager->GetAuthenticationHandler(repository.serverUrl, httpHandler);
        }

    auto client = WSRepositoryClient::Create(repository.serverUrl, repository.serviceVersion, repository.id, StubValidClientInfo(), nullptr, httpHandler);

    if (!repository.environment && !repository.credentials.IsEmpty())
        {
        client->SetCredentials(repository.credentials);
        }

    return client;
    }

void CreateTestPaths(IWSRepositoryClientPtr client, Utf8StringCR testName, Utf8StringCR customLabel, BeFileName& cachePathOut, CacheEnvironment& envOut)
    {
    cachePathOut = GetNewOutputPath(testName, customLabel, client->GetRepositoryId());

    CreateDateStampFile(testName, cachePathOut);

    envOut = CacheEnvironment();
    envOut.persistentFileCacheDir = cachePathOut;
    envOut.temporaryFileCacheDir = cachePathOut;

    cachePathOut.AppendToPath(L"WSCache.ecdb");
    }

struct RepositoryCompatibilityTests_DownloadSchemas : RepositoryCompatibilityTests {};
INSTANTIATE_TEST_CASE_P(, RepositoryCompatibilityTests_DownloadSchemas, ::testing::ValuesIn(s_downloadSchemasTestData));
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(RepositoryCompatibilityTests_DownloadSchemas, Download)
    {
    auto repository = GetParam().downloadSchemas;
    auto client = CreateClient(repository);

    BeFileName path;
    CacheEnvironment env;
    CreateTestPaths(client, "DownloadSchemas", repository.label, path, env);
    BeFileName schemasFolder = env.persistentFileCacheDir;
    schemasFolder.AppendToPath(L"Schemas").AppendSeparator();
    BeFileName::CreateNewDirectory(schemasFolder);
    ASSERT_TRUE(schemasFolder.DoesPathExist());

    auto schemas = client->SendGetSchemasRequest()->GetResult().GetValue();
    ASSERT_TRUE(schemas.GetInstances().IsValid());

    for (auto schema : schemas.GetInstances())
        {
        const rapidjson::Value& properties = schema.GetProperties();

        SchemaKey key(
            properties["Name"].GetString(),
            properties["VersionMajor"].GetInt(),
            properties.HasMember("VersionWrite") ? properties["VersionWrite"].GetInt() : 0, // EC2 compatibility
            properties["VersionMinor"].GetInt());

        BeFileName schemaFilePath(schemasFolder);
        Utf8PrintfString fullSchemaName("%s.%2d.%2d", key.m_schemaName.c_str(), key.m_versionRead, key.m_versionMinor);
        schemaFilePath.AppendToPath(BeFileName(fullSchemaName + ".ecschema.xml"));

        EXPECT_TRUE(client->SendGetFileRequest(schema.GetObjectId(), schemaFilePath)->GetResult().IsSuccess());
        }

    LOG.infov("Downloaded schemas to: %s", schemasFolder.GetNameUtf8().c_str());
    }

struct RepositoryCompatibilityTests_Create : RepositoryCompatibilityTests {};
INSTANTIATE_TEST_CASE_P(, RepositoryCompatibilityTests_Create, ::testing::ValuesIn(s_createTestData));
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(RepositoryCompatibilityTests_Create, Create)
    {
    auto repository = GetParam().create;
    auto client = CreateClient(repository);

    BeFileName path;
    CacheEnvironment env;
    CreateTestPaths(client, "Create", repository.label, path, env);
    ASSERT_FALSE(path.DoesPathExist());

    auto createResult = CachingDataSource::OpenOrCreate(client, path, env)->GetResult();
    ASSERT_TRUE(createResult.IsSuccess());
    auto ds = createResult.GetValue();

    ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        auto txn = ds->StartCacheTransaction();
        EXPECT_NE(0, ds->GetRepositorySchemas(txn).size());
        })->Wait();
    }

struct RepositoryCompatibilityTests_Upgrade : RepositoryCompatibilityTests {};
INSTANTIATE_TEST_CASE_P(, RepositoryCompatibilityTests_Upgrade, ::testing::ValuesIn(s_upgradeTestData));
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(RepositoryCompatibilityTests_Upgrade, Upgrade)
    {
    // Create base cache
    auto repository = GetParam().create;
    auto client = CreateClient(repository);

    BeFileName path;
    CacheEnvironment env;
    CreateTestPaths(client, "Upgrade", repository.label, path, env);
    ASSERT_FALSE(path.DoesPathExist());

    auto createResult = CachingDataSource::OpenOrCreate(client, path, env)->GetResult();
    ASSERT_TRUE(createResult.IsSuccess());

    // Force close
    createResult.GetValue()->Close();
    createResult = CachingDataSource::OpenResult();

    // Open connection for upgrade
    repository = GetParam().upgrade;
    client = CreateClient(repository);
    auto openResult = CachingDataSource::OpenOrCreate(client, path, env)->GetResult();
    ASSERT_TRUE(openResult.IsSuccess());
    auto ds = openResult.GetValue();

    // Pull new schemas if any
    auto updateResult = ds->UpdateSchemas(nullptr)->GetResult();
    ASSERT_TRUE(updateResult.IsSuccess());

    // Latest schemas pulled, second updates should do nothing and succeed
    for (int i = 0; i < 2; i++)
        {
        updateResult = ds->UpdateSchemas(nullptr)->GetResult();
        ASSERT_TRUE(updateResult.IsSuccess());
        }

    ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        auto txn = ds->StartCacheTransaction();
        EXPECT_NE(0, ds->GetRepositorySchemas(txn).size());
        })->Wait();
    }
