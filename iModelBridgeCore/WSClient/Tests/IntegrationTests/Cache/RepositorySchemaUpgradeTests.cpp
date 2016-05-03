/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Cache/RepositorySchemaUpgradeTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "RepositorySchemaUpgradeTests.h"

#include <WebServices/Cache/CachingDataSource.h>
#include <DgnClientFx/Utils/Http/ProxyHttpHandler.h>
#include <Bentley/BeFileListIterator.h>

StubLocalState s_localState;
IHttpHandlerPtr s_proxy;
ConnectSignInManagerPtr s_signInManager;

Credentials s_credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
UrlProvider::Environment s_env = UrlProvider::Qa;
BeFileName s_outRootPath("C:/test/RepositorySchemaUpgradeTests/"); // Keep historic records for "Upgrade" test cases

std::vector<RepositoryDef> s_repositories =
    {
        {UrlProvider::Urls::ConnectWsgGlobal, "BentleyCONNECT.Global--CONNECT.GLOBAL"},
        {UrlProvider::Urls::ConnectWsgSharedContent, "BentleyCONNECT.SharedContent--CONNECT.SharedContent"},
        {UrlProvider::Urls::ConnectWsgProjectContent, "BentleyCONNECT.ProjectContent--b89f0fe5-5313-4995-84c0-731723fb1590"}, // QA only
        {UrlProvider::Urls::ConnectWsgPersonalPublishing, "BentleyCONNECT.PersonalPublishing--CONNECT.PersonalPublishing"},
        {UrlProvider::Urls::ConnectWsgPunchList, "IssuePlugin--default"},
    };

INSTANTIATE_TEST_CASE_P(, RepositorySchemaUpgradeTests, ::testing::ValuesIn(s_repositories));

void RepositorySchemaUpgradeTests::SetUpTestCase()
    {
    WSClientBaseTest::SetUpTestCase();
    s_localState = StubLocalState();
    s_proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    UrlProvider::Initialize(s_env, UrlProvider::DefaultTimeout, &s_localState, nullptr, s_proxy);

    s_signInManager = ConnectSignInManager::Create(StubValidClientInfo(), s_proxy, &s_localState);
    ASSERT_TRUE(s_signInManager->SignInWithCredentials(s_credentials)->GetResult().IsSuccess());
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

Utf8String GetEnvStr(UrlProvider::Environment env)
    {
    if (UrlProvider::Dev == env)
        return "DEV";
    if (UrlProvider::Qa == env)
        return "QA";
    if (UrlProvider::Release == env)
        return "PROD";
    return nullptr;
    }

Utf8String GetDateStr(DateTimeCR dateTime)
    {
    return Utf8PrintfString("%04d-%02d-%02d", dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay());
    }

BeFileName GetOutputPath(Utf8StringCR testName, Utf8StringCR repositoryId, Utf8StringCR dateStr)
    {
    BeFileName path = s_outRootPath;
    path.AppendToPath(BeFileName(GetEnvStr(s_env)));
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

BeFileName GetPreviousOutputPath(Utf8StringCR testName, Utf8StringCR repositoryId)
    {
    BeFileName path = s_outRootPath;
    path.AppendToPath(BeFileName(GetEnvStr(s_env)));
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

TEST_P(RepositorySchemaUpgradeTests, DISABLED_Create)
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

TEST_P(RepositorySchemaUpgradeTests, DISABLED_Upgrade)
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

    auto updateResult = createResult.GetValue()->UpdateSchemas(nullptr)->GetResult();
    ASSERT_TRUE(updateResult.IsSuccess());
    }
