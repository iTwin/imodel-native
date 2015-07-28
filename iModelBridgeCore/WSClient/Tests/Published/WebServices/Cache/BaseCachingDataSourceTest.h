/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/BaseCachingDataSourceTest.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../WebServicesTestsHelper.h"
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
        static CachingDataSourcePtr s_reusableDataSourceV1;
        static CachingDataSourcePtr s_reusableDataSourceV2;
        static std::shared_ptr<MockWSRepositoryClient> s_reusableClient;

    private:
        CachingDataSourcePtr GetTestDataSourceV1(CachingDataSourcePtr& reusable, WSInfoCR info);
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
        // Use for CachingDataSource created with GetTestDataSourceV1 ();
        MockWSRepositoryClient& GetMockClient();

        // Create new CachingDataSource for testing
        CachingDataSourcePtr CreateNewTestDataSource
            (
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