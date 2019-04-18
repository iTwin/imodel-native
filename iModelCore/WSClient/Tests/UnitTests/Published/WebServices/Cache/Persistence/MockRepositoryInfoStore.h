/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Persistence/IRepositoryInfoStore.h>

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockRepositoryInfoStore : public IRepositoryInfoStore
    {
    MOCK_METHOD1(SetCacheInitialized, BentleyStatus(IDataSourceCache&));
    MOCK_METHOD1(IsCacheInitialized, bool(IDataSourceCache&));
    MOCK_METHOD2(CacheServerInfo, BentleyStatus(IDataSourceCache&, WSInfoCR));
    MOCK_METHOD2(CacheRepositoryInfo, BentleyStatus(IDataSourceCache&, WSRepositoryCR));
    MOCK_METHOD1(PrepareInfo, BentleyStatus(IDataSourceCache&));
    MOCK_METHOD0(GetServerInfo, WSInfo());
    MOCK_METHOD0(GetRepositoryInfo, WSRepository());
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
