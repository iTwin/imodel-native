/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../../../Utils/WebServicesTestsHelper.h"
#include "../BaseCachingDataSourceTest.h"
#include <WebServices/Cache/Persistence/IDataSourceCache.h>
#include <WebServices/Cache/ICachingDataSource.h>

#ifdef USE_GTEST
class DataSourceCacheUpgradeTests : public BaseCachingDataSourceTest
    {
    public:
        void ValidateV5SeedData(ICachingDataSourcePtr ds, BeFileNameCR path, CacheEnvironmentCR environment);
        void ValidateV5SeedData(IDataSourceCache& cache, BeFileNameCR path, CacheEnvironmentCR environment, bool checkETags = true);
        virtual void SetUp();
    };
#endif
