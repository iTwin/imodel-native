/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Persistence/BaseCacheTest.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../CachingTestsHelper.h"
#include <memory>
#include <ostream>

class BaseCacheTest : public WSClientBaseTest
    {
    private:
        static BeFileName s_seedCacheFolderPath;
        static BeFileName s_targetCacheFolderPath;
        static BeFileName s_seedCachePath;
        static BeFileName s_targetCachePath;
        static CacheEnvironment s_seedEnvironment;
        static CacheEnvironment s_targetEnvironment;

    protected:
        std::shared_ptr<DataSourceCache> GetTestCache(CacheEnvironment environment = GetTestCacheEnvironment());
        std::shared_ptr<DataSourceCache> CreateTestCache(Utf8StringCR fileName = "newTestCache.ecdb");
        std::shared_ptr<DataSourceCache> CreateTestCache(BeFileName filePath, CacheEnvironment environment);

        virtual ECSchemaPtr GetTestSchema();
        virtual ECSchemaPtr GetTestSchema2();
        BeFileName GetTestSchemaPath();
        static CacheEnvironment GetTestCacheEnvironment();

    public:
        static void SetUpTestCase();
    };

