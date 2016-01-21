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
        static std::shared_ptr<DataSourceCache> s_reusableCache;

    protected:
        std::shared_ptr<DataSourceCache> GetTestCache();
        std::shared_ptr<DataSourceCache> CreateTestCache(Utf8StringCR fileName = "newTestCache.ecdb");

        virtual ECSchemaPtr GetTestSchema();
        virtual ECSchemaPtr GetTestSchema2();
        BeFileName GetTestSchemaPath();

    public:
        static void SetUpTestCase();
    };

