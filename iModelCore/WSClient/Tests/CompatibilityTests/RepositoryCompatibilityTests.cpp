/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/RepositoryCompatibilityTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "RepositoryCompatibilityTests.h"

#include <WebServices/Cache/CachingDataSource.h>
#include <BeHttp/ProxyHttpHandler.h>

#include "../UnitTests/Published/WebServices/Connect/StubLocalState.h"

#include "TestsHelper.h"
#include "TestsHost.h"
#include "Parser/ArgumentParser.h"
#include "Disk/DiskRepositoryClient.h"

bvector<TestRepositories> s_createTestData;
bvector<TestRepositories> s_upgradeTestData;

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
        else
            {
            s_createTestData.push_back(repositories);
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
    }

void CreateDateStampFile(Utf8StringCR testName, BeFileName path)
    {
    Utf8String name = testName + "-" + DateTime::GetCurrentTimeUtc().ToString() + ".stamp";
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

BeFileName GetOutputPath(Utf8StringCR testName, Utf8StringCR repositoryId, Utf8StringCR dateStr)
    {
    BeFileName path = GetOutputPath();
    path.AppendToPath(BeFileName(dateStr));
    path.AppendToPath(BeFileName(testName));
    path.AppendToPath(BeFileName(repositoryId));
    return path;
    }

BeFileName GetNewOutputPath(Utf8StringCR testName, Utf8StringCR repositoryId)
    {
    auto todayStr = GetDateStr(DateTime::GetCurrentTimeUtc());
    auto path = GetOutputPath(testName, repositoryId, todayStr);

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

    auto client = WSRepositoryClient::Create(repository.serverUrl, repository.id, StubValidClientInfo(), nullptr, httpHandler);

    if (!repository.environment && !repository.credentials.IsEmpty())
        {
        client->SetCredentials(repository.credentials);
        }

    return client;
    }

void CreateTestPaths(IWSRepositoryClientPtr client, Utf8CP testName, BeFileName& cachePathOut, CacheEnvironment& envOut)
    {
    cachePathOut = GetNewOutputPath(testName, client->GetRepositoryId());

    CreateDateStampFile(testName, cachePathOut);

    envOut = CacheEnvironment();
    envOut.persistentFileCacheDir = cachePathOut;
    envOut.temporaryFileCacheDir = cachePathOut;

    cachePathOut.AppendToPath(L"WSCache.ecdb");
    }

struct RepositoryCompatibilityTests_Create : RepositoryCompatibilityTests {};
INSTANTIATE_TEST_CASE_P(, RepositoryCompatibilityTests_Create, ::testing::ValuesIn(s_createTestData));
TEST_P(RepositoryCompatibilityTests_Create, Create)
    {
    auto repository = GetParam().create;
    auto client = CreateClient(repository);

    BeFileName path;
    CacheEnvironment env;
    CreateTestPaths(client, "Create", path, env);
    ASSERT_FALSE(path.DoesPathExist());

    auto createResult = CachingDataSource::OpenOrCreate(client, path, env)->GetResult();
    ASSERT_TRUE(createResult.IsSuccess());
    }

struct RepositoryCompatibilityTests_Upgrade : RepositoryCompatibilityTests {};
INSTANTIATE_TEST_CASE_P(, RepositoryCompatibilityTests_Upgrade, ::testing::ValuesIn(s_upgradeTestData));
TEST_P(RepositoryCompatibilityTests_Upgrade, Upgrade)
    {
    // Create base cache
    auto repository = GetParam().create;
    auto client = CreateClient(repository);

    BeFileName path;
    CacheEnvironment env;
    CreateTestPaths(client, "Upgrade", path, env);
    ASSERT_FALSE(path.DoesPathExist());

    auto createResult = CachingDataSource::OpenOrCreate(client, path, env)->GetResult();
    ASSERT_TRUE(createResult.IsSuccess());

    // Force close
    IDataSourceCache* cache = nullptr;
    auto task = createResult.GetValue()->GetCacheAccessThread()->ExecuteAsync([&]
        {
        cache = &createResult.GetValue()->StartCacheTransaction().GetCache();
        });
    task->Wait();
    cache->Close();
    createResult = CachingDataSource::OpenResult();

    // Open connection for upgrade
    repository = GetParam().upgrade;
    client = CreateClient(repository);
    auto openResult = CachingDataSource::OpenOrCreate(client, path, env)->GetResult();
    ASSERT_TRUE(openResult.IsSuccess());

    // Pull new schemas if any
    auto updateResult = openResult.GetValue()->UpdateSchemas(nullptr)->GetResult();
    ASSERT_TRUE(updateResult.IsSuccess());

    // Latest schemas pulled, second updates should do nothing and succeed
    for (int i = 0; i < 2; i++)
        {
        updateResult = openResult.GetValue()->UpdateSchemas(nullptr)->GetResult();
        ASSERT_TRUE(updateResult.IsSuccess());
        }
    }
