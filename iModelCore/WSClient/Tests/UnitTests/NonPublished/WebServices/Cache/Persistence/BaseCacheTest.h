/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
        std::shared_ptr<DataSourceCache> GetTestCache
            (
            CacheEnvironment environment = GetTestCacheEnvironment(), 
            IFileManagerPtr fileManager = nullptr
            );
        std::shared_ptr<DataSourceCache> CreateTestCache
            (
            Utf8StringCR fileName = "newTestCache.ecdb",
            IFileManagerPtr fileManager = nullptr
            );
        std::shared_ptr<DataSourceCache> CreateTestCache
            (
            BeFileName filePath,
            CacheEnvironment environment,
            IFileManagerPtr fileManager = nullptr
            );

        virtual ECSchemaPtr GetTestSchema();
        virtual ECSchemaPtr GetTestSchema2();
        virtual ECSchemaPtr GetNavigationSchema();
        BeFileName GetTestSchemaPath();
        static CacheEnvironment GetTestCacheEnvironment();

    public:
        static void SetUpTestCase();
    };

