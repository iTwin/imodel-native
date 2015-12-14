/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/BaseCachingDataSourceTest.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "BaseCachingDataSourceTest.h"
#include <WebServices/Cache/Transactions/CacheTransactionManager.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

#ifdef USE_GTEST

bmap<BeVersion, CachingDataSourcePtr> BaseCachingDataSourceTest::s_reusableDataSources;
std::shared_ptr<MockWSRepositoryClient> BaseCachingDataSourceTest::s_mockClient;

CachingDataSourcePtr BaseCachingDataSourceTest::GetTestDataSourceV1()
    {
    return GetTestDataSource({1, 3});
    }

CachingDataSourcePtr BaseCachingDataSourceTest::GetTestDataSourceV2()
    {
    return GetTestDataSource({2, 0});
    }

CachingDataSourcePtr BaseCachingDataSourceTest::GetTestDataSource(BeVersion webApiVersion)
    {
    SetupTestDataSource(s_reusableDataSources[webApiVersion], StubWSInfoWebApi(webApiVersion));
    return s_reusableDataSources[webApiVersion];
    }

void BaseCachingDataSourceTest::SetupTestDataSource(CachingDataSourcePtr& reusable, WSInfoCR info)
    {
    if (nullptr == reusable)
        {
        reusable = CreateNewTestDataSource(GetMockClientPtr(), nullptr, info);
        }

    auto txn = reusable->StartCacheTransaction();
    EXPECT_EQ(SUCCESS, txn.GetCache().Reset());
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
    if (client == nullptr)
        {
        client = GetMockClientPtr();
        }

    if (schema.IsNull())
        {
        schema = GetTestSchema();
        }

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
        EXPECT_EQ(SchemaWriteStatus::Success, status);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));

    auto ds = CachingDataSource::OpenOrCreate(client, BeFileName(":memory:"), StubCacheEnvironemnt())->GetResult().GetValue();
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

    s_reusableDataSources.clear();
    s_mockClient = nullptr;

    BaseCacheTest::SetUpTestCase();
    }

void BaseCachingDataSourceTest::TearDownTestCase()
    {
    CacheTransactionManager::SetAllowUnsafeAccess(false);

    s_reusableDataSources.clear();
    s_mockClient = nullptr;

    BaseCacheTest::TearDownTestCase();
    }
#endif