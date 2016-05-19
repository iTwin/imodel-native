/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/BaseCachingDataSourceTest.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "CachingTestsHelper.h"
#include "../Client/MockWSRepositoryClient.h"

#include <WebServices/Cache/CachingDataSource.h>

#include "Persistence/BaseCacheTest.h"
#include "Persistence/MockDataSourceCache.h"
#include "Persistence/MockRepositoryInfoStore.h"
#include "Persistence/StubRepositoryInfoStore.h"
#include "Transactions/StubCacheTransactionManager.h"

#ifdef USE_GTEST
class BaseCachingDataSourceTest : public BaseCacheTest
    {
    private:
        static std::shared_ptr<MockWSRepositoryClient> s_mockClient;
        static ICachingDataSourcePtr s_lastCachingDataSource;
        static IDataSourceCache* s_lastDataSourceCache;
        static BeFileName s_seedCacheFolderPath;
        static BeFileName s_targetCacheFolderPath;

    private:
        void SetupTestDataSource(CachingDataSourcePtr& reusable, WSInfoCR info);
        std::shared_ptr<MockWSRepositoryClient> GetMockClientPtr();

    protected:
        // Create CachingDataSource for testing that has stubbed internals
        CachingDataSourcePtr CreateMockedCachingDataSource
            (
            std::shared_ptr<IWSRepositoryClient> client = nullptr,
            std::shared_ptr<IDataSourceCache> cache = nullptr,
            std::shared_ptr<IRepositoryInfoStore> store = nullptr,
            WorkerThreadPtr thread = nullptr,
            BeFileName temporaryDir = BeFileName()
            );

        // Get reusable WebApi 1.3 CachingDataSource created with GetMockClient() MockWSRepositoryClient;
        CachingDataSourcePtr GetTestDataSourceV1();
        // Get reusable WebApi 2.0 CachingDataSource created with GetMockClient() MockWSRepositoryClient;
        CachingDataSourcePtr GetTestDataSourceV2();
        // Get reusable WebApi version CachingDataSource created with GetMockClient() MockWSRepositoryClient;
        CachingDataSourcePtr GetTestDataSource(BeVersion webApiVersion);

        // Use for CachingDataSource created with GetTestDataSourceV1 ();
        MockWSRepositoryClient& GetMockClient();

        // Create new CachingDataSource for testing
        CachingDataSourcePtr CreateNewTestDataSource
            (
            std::shared_ptr<MockWSRepositoryClient> customClient = nullptr,
            ECSchemaPtr customSchema = nullptr,
            WSInfoCR info = StubWSInfoWebApi({1, 3})
            );

        // Create new CachingDataSource for testing
        CachingDataSourcePtr CreateNewTestDataSource
            (
            BeFileName path,
            CacheEnvironment environment = StubCacheEnvironemnt(),
            std::shared_ptr<MockWSRepositoryClient> customClient = nullptr,
            ECSchemaPtr customSchema = nullptr,
            WSInfoCR info = StubWSInfoWebApi({1, 3})
            );

    public:
        virtual void TearDown();
        static void SetUpTestCase();
        static void TearDownTestCase();
    };
#endif