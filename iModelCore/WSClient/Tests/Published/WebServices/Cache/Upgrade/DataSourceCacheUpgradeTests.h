/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Upgrade/DataSourceCacheUpgradeTests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../../WebServicesTestsHelper.h"
#include "../BaseCachingDataSourceTest.h"
#include <WebServices/Cache/Persistence/IDataSourceCache.h>
#include <WebServices/Cache/ICachingDataSource.h>

class DataSourceCacheUpgradeTests : public BaseCachingDataSourceTest
    {
    public:
        void ValidateV5SeedData(ICachingDataSourcePtr ds, BeFileNameCR path, CacheEnvironmentCR environment);
        void ValidateV5SeedData(IDataSourceCache& cache, BeFileNameCR path, CacheEnvironmentCR environment);
        virtual void SetUp();
    };
