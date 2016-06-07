/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/BaseCachingDataSourceTest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "BaseCachingDataSourceTest.h"
#include <WebServices/Cache/Transactions/CacheTransactionManager.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

#ifdef USE_GTEST

std::shared_ptr<MockWSRepositoryClient> BaseCachingDataSourceTest::s_mockClient;
ICachingDataSourcePtr BaseCachingDataSourceTest::s_lastCachingDataSource;
IDataSourceCache* BaseCachingDataSourceTest::s_lastDataSourceCache;
BeFileName BaseCachingDataSourceTest::s_seedCacheFolderPath;
BeFileName BaseCachingDataSourceTest::s_targetCacheFolderPath;

CachingDataSourcePtr BaseCachingDataSourceTest::GetTestDataSourceV1()
    {
    return GetTestDataSource({1, 3});
    }

CachingDataSourcePtr BaseCachingDataSourceTest::GetTestDataSourceV2()
    {
    return GetTestDataSource({2, 0});
    }

CachingDataSourcePtr BaseCachingDataSourceTest::GetTestDataSourceV24()
    {
    return GetTestDataSource({2, 4});
    }

CachingDataSourcePtr BaseCachingDataSourceTest::GetTestDataSource(BeVersion webApiVersion)
    {
    // AsyncTask API delays destruction of CachingDataSource, thus DB is not always closed, force close here
    if (s_lastCachingDataSource && s_lastDataSourceCache->GetECDb().IsDbOpen())
        s_lastDataSourceCache->Close();
    s_lastCachingDataSource = nullptr;

    BeFileName seedCacheFolderName(webApiVersion.ToString());
    BeFileName seedCacheFolderPath = BeFileName(s_seedCacheFolderPath).AppendToPath(seedCacheFolderName);
    BeFileName targetCacheFolderPath = BeFileName(s_targetCacheFolderPath).AppendToPath(seedCacheFolderName);

    // Prepare source files
    if (!seedCacheFolderPath.DoesPathExist())
        {
        auto client = MockWSRepositoryClient::Create();
        BeFileName seedCachePath = BeFileName(seedCacheFolderPath).AppendToPath(L"testcache.ecdb");
        CacheEnvironment seedEnvironment;
        seedEnvironment.persistentFileCacheDir = BeFileName(seedCacheFolderPath).AppendToPath(L"persistent");
        seedEnvironment.temporaryFileCacheDir = BeFileName(seedCacheFolderPath).AppendToPath(L"temporary");
        auto ds = CreateNewTestDataSource(seedCachePath, seedEnvironment, client, nullptr, StubWSInfoWebApi(webApiVersion));
        EXPECT_FALSE(nullptr == ds);
        }

    // Prepare target files
    if (targetCacheFolderPath.DoesPathExist())
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(targetCacheFolderPath));
    EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CloneDirectory(seedCacheFolderPath, targetCacheFolderPath));

    // Open target cache
    auto client = GetMockClientPtr();
    BeFileName targetCachePath = BeFileName(targetCacheFolderPath).AppendToPath(L"testcache.ecdb");
    CacheEnvironment targetEnvironment;
    targetEnvironment.persistentFileCacheDir = BeFileName(targetCacheFolderPath).AppendToPath(L"persistent");
    targetEnvironment.temporaryFileCacheDir = BeFileName(targetCacheFolderPath).AppendToPath(L"temporary");

    auto ds = CachingDataSource::OpenOrCreate(client, targetCachePath, targetEnvironment)->GetResult().GetValue();
    EXPECT_FALSE(nullptr == ds);

    // Store for next call
    s_lastCachingDataSource = ds;
    if (ds != nullptr)
        s_lastDataSourceCache = &ds->StartCacheTransaction().GetCache();

    return ds;
    }

void BaseCachingDataSourceTest::SetupTestDataSource(CachingDataSourcePtr& reusable, WSInfoCR info)
    {
    if (nullptr == reusable)
        {
        reusable = CreateNewTestDataSource(GetMockClientPtr(), nullptr, info);
        }

    auto txn = reusable->StartCacheTransaction();
    // EXPECT_EQ(SUCCESS, txn.GetCache().Reset());
    EXPECT_EQ(SUCCESS, txn.Commit());

    reusable->GetCacheAccessThread()->OnEmpty()->Wait();
    }

CachingDataSourcePtr BaseCachingDataSourceTest::CreateMockedCachingDataSource
(
std::shared_ptr<IWSRepositoryClient> client,
std::shared_ptr<IDataSourceCache> cache,
std::shared_ptr<IRepositoryInfoStore> store,
WorkerThreadPtr thread,
BeFileName temporaryDir
)
    {
    if (client == nullptr)
        {
        client = GetMockClientPtr();
        }
    if (cache == nullptr)
        {
        cache = std::make_shared<NiceMock<MockDataSourceCache>>();
        }
    if (store == nullptr)
        {
        store = std::make_shared<StubRepositoryInfoStore>(WSInfo({2, 0}, {2, 0}, WSInfo::Type::BentleyWSG));
        }
    if (thread == nullptr)
        {
        thread = WorkerThread::Create("Test CachingDataSource");
        }
    if (temporaryDir.empty())
        {
        temporaryDir = GetTestsTempDir();
        }

    auto txnManager = std::make_shared<StubCacheTransactionManager>(cache);

    return CachingDataSource::Create(client, txnManager, store, thread, temporaryDir);
    }

std::shared_ptr<MockWSRepositoryClient> BaseCachingDataSourceTest::GetMockClientPtr()
    {
    if (nullptr == s_mockClient)
        {
        s_mockClient = MockWSRepositoryClient::Create();
        }
    return s_mockClient;
    }

MockWSRepositoryClient& BaseCachingDataSourceTest::GetMockClient()
    {
    return *GetMockClientPtr();
    }

CachingDataSourcePtr BaseCachingDataSourceTest::CreateNewTestDataSource
(
std::shared_ptr<MockWSRepositoryClient> client,
ECSchemaPtr schema,
WSInfoCR info
)
    {
    return CreateNewTestDataSource(BeFileName(L":memory:"), StubCacheEnvironemnt(), client, schema, info);
    }

CachingDataSourcePtr BaseCachingDataSourceTest::CreateNewTestDataSource
(
BeFileName path,
CacheEnvironment environment,
std::shared_ptr<MockWSRepositoryClient> client,
ECSchemaPtr schema,
WSInfoCR info
)
    {
    if (path.empty())
        path = BeFileName(":memory:");

    if (client == nullptr)
        client = GetMockClientPtr();

    if (schema.IsNull())
        schema = GetTestSchema();

    StubInstances schemaDefs;
    schemaDefs.Add({"MetaSchema.ECSchemaDef", "TestSchema"}, {{"Name", "TestSchema"}, {"VersionMajor", 1}, {"VersionMinor", 0}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(info))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemaDefs.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "TestSchema"), _, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        SchemaWriteStatus status = schema->WriteToXmlFile(filePath);
        EXPECT_EQ(SchemaWriteStatus::SCHEMA_WRITE_STATUS_Success, status);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));

    auto ds = CachingDataSource::OpenOrCreate(client, path, environment)->GetResult().GetValue();
    EXPECT_FALSE(nullptr == ds);
    return ds;
    }

void BaseCachingDataSourceTest::TearDown()
    {
    if (s_mockClient)
        {
        Mock::VerifyAndClearExpectations(s_mockClient.get());
        }

    BaseCacheTest::TearDown();
    }

void BaseCachingDataSourceTest::SetUpTestCase()
    {
    CacheTransactionManager::SetAllowUnsafeAccess(true);

    s_mockClient = nullptr;
    s_lastCachingDataSource = nullptr;
    s_lastDataSourceCache = nullptr;

    s_seedCacheFolderPath = GetTestsOutputDir().AppendToPath(L"BaseCachingDataSourceTest-Seeds");
    s_targetCacheFolderPath = GetTestsTempDir().AppendToPath(L"BaseCachingDataSourceTest-TestCaches");

    if (s_seedCacheFolderPath.DoesPathExist())
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(s_seedCacheFolderPath));

    BaseCacheTest::SetUpTestCase();
    }

void BaseCachingDataSourceTest::TearDownTestCase()
    {
    CacheTransactionManager::SetAllowUnsafeAccess(false);

    s_mockClient = nullptr;

    BaseCacheTest::TearDownTestCase();
    }
#endif