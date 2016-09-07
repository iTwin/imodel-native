/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Cache/RepositorySchemaUpgradeTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "RepositorySchemaUpgradeTests.h"

#include <WebServices/Cache/CachingDataSource.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <Bentley/BeFileListIterator.h>

#include <prg.h>
static Utf8CP BUILD_VERSION = REL_V "." MAJ_V "." MIN_V "." SUBMIN_V;

StubLocalState s_localState;
IHttpHandlerPtr s_proxy;
ConnectSignInManagerPtr s_signInManager;

Credentials s_credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
BeFileName s_outRootPath("C:/test/RepositorySchemaUpgradeTests/"); // Keep historic records for "Upgrade" test cases

bvector<RepositoryDef> GetTestData()
    {
    bvector<UrlProvider::Environment> testEnvironments =
        {
        //UrlProvider::Environment::Release,
        UrlProvider::Environment::Qa,
        //UrlProvider::Environment::Dev,
        };

    bmap<UrlProvider::Environment, bvector<RepositoryDef>> all;

    all[UrlProvider::Release] = {
            {UrlProvider::Release, UrlProvider::Urls::ConnectWsgGlobal, "BentleyCONNECT.Global--CONNECT.GLOBAL"},
            {UrlProvider::Release, UrlProvider::Urls::ConnectWsgSharedContent, "BentleyCONNECT.SharedContent--CONNECT.SharedContent"},
            {UrlProvider::Release, UrlProvider::Urls::ConnectWsgPersonalPublishing, "BentleyCONNECT.PersonalPublishing--CONNECT.PersonalPublishing"},
            {UrlProvider::Release, UrlProvider::Urls::ConnectWsgPunchList, "IssuePlugin--default"},
            {UrlProvider::Release, UrlProvider::Urls::ConnectWsgProjectContent, "BentleyCONNECT.ProjectContent--e577dbb3-07d4-413d-af06-4b0d6abf5a55"}
        };

    all[UrlProvider::Qa] = {
            {UrlProvider::Qa, UrlProvider::Urls::ConnectWsgGlobal, "BentleyCONNECT.Global--CONNECT.GLOBAL"},
            {UrlProvider::Qa, UrlProvider::Urls::ConnectWsgSharedContent, "BentleyCONNECT.SharedContent--CONNECT.SharedContent"},
            {UrlProvider::Qa, UrlProvider::Urls::ConnectWsgPersonalPublishing, "BentleyCONNECT.PersonalPublishing--CONNECT.PersonalPublishing"},
            {UrlProvider::Qa, UrlProvider::Urls::ConnectWsgPunchList, "IssuePlugin--default"},
            {UrlProvider::Qa, UrlProvider::Urls::ConnectWsgProjectContent, "BentleyCONNECT.ProjectContent--b89f0fe5-5313-4995-84c0-731723fb1590"},
        };

    all[UrlProvider::Dev] = {
            {UrlProvider::Dev, UrlProvider::Urls::ConnectWsgGlobal, "BentleyCONNECT.Global--CONNECT.GLOBAL"},
            {UrlProvider::Dev, UrlProvider::Urls::ConnectWsgSharedContent, "BentleyCONNECT.SharedContent--CONNECT.SharedContent"},
            {UrlProvider::Dev, UrlProvider::Urls::ConnectWsgPersonalPublishing, "BentleyCONNECT.PersonalPublishing--CONNECT.PersonalPublishing"},
            {UrlProvider::Dev, UrlProvider::Urls::ConnectWsgPunchList, "IssuePlugin--default"},
            {UrlProvider::Dev, UrlProvider::Urls::ConnectWsgProjectContent, "BentleyCONNECT.ProjectContent--ddd97671-790e-432b-bfd9-772bf8715b09"}
        };

    bvector<RepositoryDef> testRepositories;
    for (auto env : testEnvironments)
        {
        auto& allForEnv = all[env];
        testRepositories.insert(testRepositories.end(), allForEnv.begin(), allForEnv.end());
        }
    return testRepositories;
    };

INSTANTIATE_TEST_CASE_P(, RepositorySchemaUpgradeTests, ::testing::ValuesIn(GetTestData()));

void RepositorySchemaUpgradeTests::SetUpTestCase()
    {
    WSClientBaseTest::SetUpTestCase();
    }

void RepositorySchemaUpgradeTests::SetUp()
    {
    s_localState = StubLocalState();
    s_proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    UrlProvider::Initialize(GetParam().env, UrlProvider::DefaultTimeout, &s_localState, nullptr, s_proxy);

    s_signInManager = ConnectSignInManager::Create(StubValidClientInfo(), s_proxy, &s_localState);
    ASSERT_TRUE(s_signInManager->SignInWithCredentials(s_credentials)->GetResult().IsSuccess());

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCLIENT, NativeLogging::LOG_INFO);
    }

void CreateDateStampFile(Utf8StringCR testName, BeFileName path)
    {
    Utf8String name = testName + "-" + DateTime::GetCurrentTimeUtc().ToUtf8String() + ".stamp";
    name.ReplaceAll(":", ".");
    path.AppendToPath(BeFileName(name));
    BeFile file;
    EXPECT_EQ(BeFileStatus::Success, file.Create(path));
    file.Close();
    }

Utf8String RepositorySchemaUpgradeTests::GetEnvStr(UrlProvider::Environment env)
    {
    if (UrlProvider::Dev == env)
        return "DEV";
    if (UrlProvider::Qa == env)
        return "QA";
    if (UrlProvider::Release == env)
        return "PROD";
    return nullptr;
    }

Utf8String RepositorySchemaUpgradeTests::GetDateStr(DateTimeCR dateTime)
    {
    return Utf8PrintfString("%04d-%02d-%02d", dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay());
    }

BeFileName RepositorySchemaUpgradeTests::GetOutputPath()
    {
    BeFileName path = s_outRootPath;
    path.AppendToPath(BeFileName(BUILD_VERSION));
    path.AppendToPath(BeFileName(GetEnvStr(GetParam().env)));
    return path;
    }

BeFileName RepositorySchemaUpgradeTests::GetOutputPath(Utf8StringCR testName, Utf8StringCR repositoryId, Utf8StringCR dateStr)
    {
    BeFileName path = GetOutputPath();
    path.AppendToPath(BeFileName(dateStr));
    path.AppendToPath(BeFileName(testName));
    path.AppendToPath(BeFileName(repositoryId));
    return path;
    }

BeFileName RepositorySchemaUpgradeTests::GetNewOutputPath(Utf8StringCR testName, Utf8StringCR repositoryId)
    {
    auto todayStr = GetDateStr(DateTime::GetCurrentTimeUtc());
    auto path = GetOutputPath(testName, repositoryId, todayStr);

    if (path.DoesPathExist())
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(path));

    EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(path));

    return path;
    }

BeFileName RepositorySchemaUpgradeTests::GetPreviousOutputPath(Utf8StringCR testName, Utf8StringCR repositoryId)
    {
    BeFileName path = GetOutputPath();
    path.AppendToPath(L"*");

    auto todayStr = GetDateStr(DateTime::GetCurrentTimeUtc());

    BeFileListIterator iterator(path, false);
    bset<Utf8String> folders;
    BeFileName folderPath;
    while (SUCCESS == iterator.GetNextFileName(folderPath))
        {
        Utf8String folder(folderPath.GetFileNameAndExtension());
        if (folder == todayStr)
            continue;
        folders.insert(folder);
        }

    if (folders.empty())
        return BeFileName();

    Utf8String previousFolder = *folders.rbegin();
    path = GetOutputPath(testName, repositoryId, previousFolder);
    EXPECT_TRUE(path.DoesPathExist());
    return path;
    }

TEST_P(RepositorySchemaUpgradeTests, Create)
    {
    Utf8String serverUrl = GetParam().url->Get();
    Utf8String repositoryId = GetParam().id;

    BeFileName path = GetNewOutputPath("Create", repositoryId);

    CreateDateStampFile("Create", path);

    CacheEnvironment env;
    env.persistentFileCacheDir = path;
    env.temporaryFileCacheDir = path;

    path.AppendToPath(L"WSCache.ecdb");
    ASSERT_FALSE(path.DoesPathExist());

    auto authHandler = s_signInManager->GetAuthenticationHandler(serverUrl, s_proxy);
    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);
    auto createResult = CachingDataSource::OpenOrCreate(client, path, env)->GetResult();
    ASSERT_TRUE(createResult.IsSuccess());
    }

TEST_P(RepositorySchemaUpgradeTests, Upgrade)
    {
    Utf8String serverUrl = GetParam().url->Get();
    Utf8String repositoryId = GetParam().id;

    BeFileName oldPath = GetPreviousOutputPath("Create", repositoryId);
    BeFileName path = GetNewOutputPath("Upgrade", repositoryId);

    ASSERT_TRUE(oldPath.DoesPathExist());
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(path));
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CloneDirectory(oldPath, path));

    CreateDateStampFile("Upgrade", path);

    CacheEnvironment env;
    env.persistentFileCacheDir = path;
    env.temporaryFileCacheDir = path;

    path.AppendToPath(L"WSCache.ecdb");
    ASSERT_TRUE(path.DoesPathExist());

    auto authHandler = s_signInManager->GetAuthenticationHandler(serverUrl, s_proxy);
    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);
    auto createResult = CachingDataSource::OpenOrCreate(client, path, env)->GetResult();
    ASSERT_TRUE(createResult.IsSuccess());

    // Pull new schemas if any
    auto updateResult = createResult.GetValue()->UpdateSchemas(nullptr)->GetResult();
    ASSERT_TRUE(updateResult.IsSuccess());

    // Latest schemas pulled, second updates should do nothing and succeed
    for (int i = 0; i < 2; i++)
        {
        updateResult = createResult.GetValue()->UpdateSchemas(nullptr)->GetResult();
        ASSERT_TRUE(updateResult.IsSuccess());
        }
    }
