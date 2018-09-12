/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/MockCachingDataSource.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/ICachingDataSource.h>

#include "Persistence/MockDataSourceCache.h"
#include "Transactions/MockTransactionHandler.h"

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockCachingDataSource : public ICachingDataSource
    {
    private:
        NiceMock<MockDataSourceCache> m_cache;
        WorkerThreadPtr m_thread;
        NiceMock<MockTransactionHandler> m_txnManager;

    public:
        MockCachingDataSource() :
            m_thread(WorkerThread::Create("MockCachingDataSource"))
            {
            DefaultValue<AsyncTaskPtr<ObjectsResult>>::Set(CreateCompletedAsyncTask(ObjectsResult()));
            DefaultValue<AsyncTaskPtr<KeysResult>>::Set(CreateCompletedAsyncTask(KeysResult()));
            DefaultValue<AsyncTaskPtr<FileResult>>::Set(CreateCompletedAsyncTask(FileResult()));
            DefaultValue<AsyncTaskPtr<BatchResult>>::Set(CreateCompletedAsyncTask(BatchResult()));
            DefaultValue<AsyncTaskPtr<Result>>::Set(CreateCompletedAsyncTask(Result()));

            ON_CALL(*this, GetCacheAccessThread()).WillByDefault(Return(m_thread));

            ON_CALL(m_txnManager, CommitTransaction()).WillByDefault(Return(SUCCESS));
            ON_CALL(m_txnManager, RollbackTransaction()).WillByDefault(Return(SUCCESS));

            ON_CALL(*this, StartCacheTransactionProxy()).WillByDefault(Invoke([=]
                {
                auto stubTransactionPtr = std::make_shared<CacheTransaction>(m_cache, &m_txnManager);
                return stubTransactionPtr;
                }));
            }

        virtual ~MockCachingDataSource()
            {}

        MockDataSourceCache& GetCacheMock()
            {
            return m_cache;
            }

        MOCK_CONST_METHOD0(GetClient, IWSRepositoryClientPtr());
        MOCK_METHOD1(SetClient,
            void(IWSRepositoryClientPtr client));
        MOCK_METHOD0(CancelAllTasks,
            AsyncTaskPtr<void>());
        MOCK_METHOD1(EnableSkipTokens,void(bool));
        MOCK_METHOD1(UpdateSchemas,
            AsyncTaskPtr<Result>(ICancellationTokenPtr ct));

        CacheTransaction StartCacheTransaction() override
            {
            return std::move(*StartCacheTransactionProxy());
            }

        //! Use this method to mock StartCacheTransaction that returns non-copyable CacheTransaction
        MOCK_METHOD0(StartCacheTransactionProxy,
            std::shared_ptr<CacheTransaction>());

        MOCK_METHOD0(GetCacheAccessThread,
            WorkerThreadPtr());
        MOCK_METHOD1(GetRepositorySchemas,
            ECSchemaList(CacheTransactionCR));
        MOCK_METHOD1(GetRepositorySchemaKeys,
            bvector<SchemaKey>(CacheTransactionCR txn));
        MOCK_METHOD1(GetServerInfo,
            WSInfo(CacheTransactionCR));
        MOCK_METHOD0(GetServerInfo,
            WSInfo());
        MOCK_METHOD0(GetRepositoryInfo,
            WSRepository());
        MOCK_METHOD1(SetClassesToAlwaysCacheChildren,
            void(const bset<Utf8String>& classesToAlwaysCacheChildren));
        MOCK_METHOD2(GetNavigationResponseKey,
            CachedResponseKey(CacheTransactionCR, ObjectIdCR parentId));
        MOCK_METHOD2(GetNavigationResponseKey,
            CachedResponseKey(CacheTransactionCR, ECInstanceKeyCR parentKey));
        MOCK_METHOD3(GetNavigationQuery,
            WSQueryPtr(CacheTransactionCR, ObjectIdCR parentId, ISelectProviderPtr selectProvider));
        MOCK_METHOD4(GetObject,
            AsyncTaskPtr<ObjectsResult>(ObjectIdCR, RetrieveOptions, IDataSourceCache::JsonFormat, ICancellationTokenPtr));
        MOCK_METHOD5(GetObjects,
            AsyncTaskPtr<ObjectsResult>(CachedResponseKeyCR responseKey, WSQueryCR query, DataOrigin origin, std::shared_ptr<const ISelectProvider>, ICancellationTokenPtr ct));
        MOCK_METHOD4(GetObjectsKeys,
            AsyncTaskPtr<KeysResult>(CachedResponseKeyCR responseKey, WSQueryCR query, RetrieveOptions retrieveOptions, ICancellationTokenPtr ct));
        MOCK_METHOD4(GetNavigationChildren,
            AsyncTaskPtr<ObjectsResult>(ObjectIdCR, DataOrigin, std::shared_ptr<const SelectProvider>, ICancellationTokenPtr));
        MOCK_METHOD4(GetNavigationChildrenKeys,
            AsyncTaskPtr<KeysResult>(ObjectIdCR, DataOrigin, std::shared_ptr<const ISelectProvider>, ICancellationTokenPtr));
        MOCK_METHOD4(GetFile,
            AsyncTaskPtr<FileResult>(ObjectIdCR fileId, DataOrigin origin, ProgressCallback onProgress, ICancellationTokenPtr ct));
        MOCK_METHOD5(CacheFiles,
            AsyncTaskPtr<BatchResult>(const bvector<ObjectId>& filesIds, bool skipCachedFiles, FileCache fileCacheLocation, ProgressCallback onProgress, ICancellationTokenPtr ct));
        MOCK_METHOD2(DownloadAndCacheChildren,
            AsyncTaskPtr<Result>(const bvector<ObjectId>& parentIds, ICancellationTokenPtr ct));
        MOCK_METHOD3(SyncLocalChanges,
            AsyncTaskPtr<BatchResult>(ProgressCallback, ICancellationTokenPtr, SyncOptions));
        MOCK_METHOD4(SyncLocalChanges,
            AsyncTaskPtr<BatchResult>(const bset<ECInstanceKey>&, ProgressCallback, ICancellationTokenPtr, SyncOptions));
        MOCK_METHOD5(SyncCachedData,
            AsyncTaskPtr<BatchResult>(bvector<ECInstanceKey>, bvector<IQueryProvider::Query>, bvector<IQueryProviderPtr>, ProgressHandler, ICancellationTokenPtr));
        MOCK_METHOD5(CacheNavigation,
            AsyncTaskPtr<BatchResult>(const bvector<ObjectId>&, const bvector<ObjectId>&, std::shared_ptr<const ISelectProvider>, ProgressCallback, ICancellationTokenPtr));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif