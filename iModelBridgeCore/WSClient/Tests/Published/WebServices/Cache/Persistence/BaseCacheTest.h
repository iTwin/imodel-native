/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Persistence/BaseCacheTest.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

        virtual ECSchemaPtr GetTestSchema();
        virtual ECSchemaPtr GetTestSchema2();
        BeFileName GetTestSchemaPath();

    public:
        static void SetUpTestCase();
    };

