/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Persistence/MockRepositoryInfoStore.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    MOCK_METHOD2 (CacheServerInfo, BentleyStatus (IDataSourceCache& cache, WSInfoCR info));
    MOCK_METHOD1 (GetServerInfo, WSInfoCR (IDataSourceCache& cache));
    MOCK_METHOD1 (SetCacheInitialized, BentleyStatus (IDataSourceCache& cache));
    MOCK_METHOD1 (IsCacheInitialized, bool (IDataSourceCache& cache));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
